
 /**
  * @name SpawnMgr.cpp
  *     NPC Spawn managment system for Alasiya EvEmu
  *
  * @Author:         Allan
  * @date:          15 July 2015
  *
  */

#include "EVEServerConfig.h"
#include "PyServiceMgr.h"
#include "StaticDataMgr.h"
#include "npc/NPC.h"
#include "system/DestinyManager.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"
#include "system/cosmicMgrs/SpawnMgr.h"


/*
 SPAWN__ERROR
 SPAWN__WARNING
 SPAWN__MESSAGE
 SPAWN__POP
 SPAWN__DEPOP
 SPAWN__TRACE
 */
/** @todo  adjust this class to manage anomaly spawns, too */
/** @todo this class needs a bit of tweaking to work as designed */
SpawnMgr::SpawnMgr(SystemManager* mgr, PyServiceMgr& svc)
: m_system(mgr),
  m_services(svc),
  m_mainTimer(60000),  // 60s ... just putting something here
  m_groupTimer(60000)
{
    m_spawnID = 1;

    m_enabled = false;
    m_initalized = false;

    m_mainTimer.Disable();
    m_groupTimer.Disable();

    m_spawns.clear();
    m_bubbles.clear();
    m_toSpawn.clear();
    m_ratSpawns.clear();
    m_spawnClass.clear();
    m_factionGroups.clear();
}

bool SpawnMgr::Init()
{
    if (!sConfig.npc.RoamingSpawns and !sConfig.npc.StaticSpawns) {
        _log(COSMIC_MGR__MESSAGE, "Spawn System Disabled.  Not Initalizing Spawn Manager for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
        return true;
    }

    _log(COSMIC_MGR__MESSAGE, "SpawnMgr Initialized for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
    return (m_initalized = true);
}

void SpawnMgr::Process() {
    if (!m_initalized)
        return;

    double profileStartTime = 0.0;
    if (sConfig.debug.UseProfiling)
        profileStartTime = GetTimeUSeconds();
    // called by SystemManager::Process() for each system.  this will need to be fast.
    //  check timers and call approprate functions as needed.

    // this will be initial spawn timer for system.
    //  while this is active, NO spawns will be made.
    if (m_mainTimer.Enabled())  {
        if (m_mainTimer.Check(false)) {
            m_mainTimer.Disable();
            m_enabled = true;
            _log(SPAWN__MESSAGE, "SpawnMgr::Process() - Main Timer called.  Spawn functions enabled for %s(%u).",
                 m_system->GetName().c_str(), m_system->GetID());
        }
        if (sConfig.debug.UseProfiling)
            sProfile.AddTime(_spawnProfile, GetTimeUSeconds() - profileStartTime);
        return;
    }

    if (!m_enabled)
        return;

    // will have to think about this one a bit to implement properly (and quickly)
    /*  current implentation uses a single timer for entire system
	 * timer goes off, do the following...
	 *   loop thru all spawned bubbles in system
	 *   spawn rats as needed to fill spawn groups
	 *   kill timer after all spawns are full
	 * this should be a single check for each iteration.  if multiple rats are killed, timer remains counting down from initial setting.
	 * if all rats are respawned, then a rat is killed again (in this system), the timer is reset and the system begins again.
	 *   each time the group timer is hit, it will check for missing rats in each spawn group.  if there is a missing rat, that rat is
	 *   respawned and the timer is reset, to check again.  this *should* enable chain-ratting.
     *
     *  this will need work for correct operation as intended.
     * process should look at all SpawnEntryDefs for each bubble, and spawn according to number/total for that bubble's spawnID
     * right now, that number isnt updated when rats are killed.  will have to work on coding that correctly
	 */
    if (m_groupTimer.Enabled()) {
        if (m_groupTimer.Check()) {
            bool killTimer = true;
            RatBubbleVec::iterator curBubbleItr = m_bubbles.begin();
            while (curBubbleItr != m_bubbles.end()) {
                auto curSpawnItr = m_spawns.equal_range((*curBubbleItr)->GetID());
                for (auto it = curSpawnItr.first; it != curSpawnItr.second; ++it) {
                    if (it->second.enabled) {
                        _log(SPAWN__TRACE, "Process() called, groupTimer hit, bubbleItr != end and spawnItr enabled.  SpawnEntryID %u is 0x%X", \
                                    it->second.spawnID, &it->second);
                        // this means check SpawnEntry for 'missing' SpawnGroup members and respawn as needed.
                        ReSpawn((*curBubbleItr), it->second);
                        killTimer = false;
                        /* we respawned one.
                         * should we return here and wait for next timer to hit,
                         * or spawn all missing entities at same time?
                         * for now, just spawn them all.
                        if (sConfig.debug.UseProfiling)
                            sProfile.AddTime(_spawnProfile, GetTimeUSeconds() - profileStartTime);
                        return; */
                    }
                }
                ++curBubbleItr;
            }

            if (killTimer) {
                m_groupTimer.Disable();
                _log(SPAWN__MESSAGE, "SpawnMgr::Process() - Spawn Groups full (or no spawns) for %s(%u).  Group Timer disabled.", \
                     m_system->GetName().c_str(), m_system->GetID());
            }
        }
    }

    if (sConfig.debug.UseProfiling)
        sProfile.AddTime(_spawnProfile, GetTimeUSeconds() - profileStartTime);
}

void SpawnMgr::MoveSpawn()
{
    /* this will be to move a spawn from one location to another (change bubbles) */
    _log(SPAWN__TRACE, "MoveSpawn() called.");
}

void SpawnMgr::StartMainTimer()
{
    uint32 time = sConfig.npc.RoamingTimer *60 *1000;
    if (sConfig.debug.SpawnTest)
        time = 5000; /* 5s for npc spawn testing */
    m_mainTimer.Start(time);

    _log(SPAWN__MESSAGE, "SpawnMgr::StartMainTimer() - Main Spawn Timer started for %s(%u) at %u ms.", \
         m_system->GetName().c_str(), m_system->GetID(), time);
}

void SpawnMgr::SpawnDepopped(SystemBubble* pSysBubble, uint32 itemID)
{
    // NOTE this DOES NOT remove entity from system or bubble.  user must do this BEFORE calling.
    if (pSysBubble == nullptr)
        return;
    _log(SPAWN__DEPOP, "SpawnMgr::SpawnDepoped - NPC %u removed from system.  DePop requested", itemID);
    // delete this spawn item from SpawnEntry in this bubble.
    RemoveSpawn(pSysBubble->GetID(), itemID);
    // if any SpawnEntry still exists for this bubble, reset group timer.
    // this enables chain ratting or creating a new spawn
    SpawnEntryDef::iterator itr = m_spawns.find(pSysBubble->GetID());
    if (itr != m_spawns.end()) {
        if (!m_groupTimer.Enabled())
            m_groupTimer.Start(itr->second.time);
        itr->second.enabled = true;
    } else {
        //there is no SpawnEntry for this bubble (no spawns left here).  delete from the spawned list and reset bubble checks.
        m_bubbles.erase(std::find(m_bubbles.begin(), m_bubbles.end(), pSysBubble));
        pSysBubble->ResetBubbleRatSpawn();
        m_system->DecRatSpawnCount();
    }
}

void SpawnMgr::SpawnPopped(uint32 itemID)
{
    _log(SPAWN__POP, "SpawnMgr::SpawnPopped - Pop called for NPC %u", itemID);
}

void SpawnMgr::DoSpawnForAnomaly(int32 spawnID)
{
    /*  this needs to deal with multiple things.
     * 1- rat types for anomaly....
     * 2- rat faction per dungeon template
     * 3- waves per template.  how to do this???
     * 4- unlocking warp gates when needed per wave
     * 5- dropping loot according to (wave/dungeon/template)?
     * 6- after last spawn, possible escelation per dungeon type?   this should signal anomaly mgr to create the escelation
     * 7- more/others?
     */

}

bool SpawnMgr::DoSpawnForBubble(SystemBubble* pSysBubble, uint32 regionID, double secRating)
{
    if (!m_enabled)
        return false;
    if (pSysBubble == nullptr)
        return false;
    double profileStartTime = 0.0;
    if (sConfig.debug.UseProfiling)
        profileStartTime = GetTimeUSeconds();
    if (FindSpawnForBubble(pSysBubble->GetID())) {
        _log(SPAWN__TRACE, "SpawnMgr::FindSpawnForBubble() returned true for bubble %u.", pSysBubble->GetID());
        pSysBubble->SetSpawned(true);  // bubble flag to avoid multiple spawns in same bubble.
        return false;
    } else {
        sLog.Green("SpawnMgr", "DoSpawnForBubble called for bubble %u(%u) in %s(%u)(%.4f).",
                     pSysBubble->GetID(), sBubbleMgr.GetBeltID(pSysBubble->GetID()), m_system->GetName().c_str(), m_system->GetID(), secRating);
        if (PrepSpawn(pSysBubble, regionID, secRating)) {
            pSysBubble->SetSpawned(true);  // bubble flag to avoid multiple spawns in same bubble.
        } else {
            _log(SPAWN__TRACE, "SpawnMgr::PrepSpawn() returned false for bubble %u.", pSysBubble->GetID());
            return false;
        }
    }

    m_system->IncRatSpawnCount();

    /* this will throw off the accuracy of the profile, as this and Process() use the same data container */
    if (sConfig.debug.UseProfiling)
        sProfile.AddTime(_spawnProfile, GetTimeUSeconds() - profileStartTime);
    return true;
}

bool SpawnMgr::FindSpawnForBubble(uint16 itemID) {
    SpawnEntryDef::iterator itr = m_spawns.find(itemID);
    if (itr != m_spawns.end())
        return true;

    return false;
}

/*
struct SpawnEntry { // notes for me while creating/writing/testing
    uint8 spawnType;// spawn type.  1 = roaming, 2 = static
    uint8 total;    // total number of this group spawned
    uint8 number;   // spawn number in this group
    uint8 sub;      // spawn data subtype
    uint8 type;     // spawn data class id (in case we have to look it up again)
    uint16 typeID;  // rat type id
    uint32 itemID;  // rat entity id
    uint32 groupID; // rat group id (may look into changing typeID within group later on respawn (for chaining))
    uint32 spawnID; // spawn id (if needed to match up with other spawns of this group for warp or w/e (multiple spawn types in this group))
    uint32 time;    // spawn group timer run time
};
*/
bool SpawnMgr::PrepSpawn(SystemBubble* pSysBubble, uint32 regionID, double secRating)
{
    if (pSysBubble == nullptr)
        return false;
    // get faction for this region
    uint32 factionID = factionRogueDrones; // default to rogue drones.  this is my internal rogue drone factionID.
    if (sConfig.npc.RatFaction)
        factionID = sConfig.npc.RatFaction;
    else if (MakeRandomFloat() > 0.15) // random chance for ANY beltspawn to be rogue drone...if chance < 0.15, rat = drone.
        factionID = sDataMgr.GetRegionRatFaction(regionID);

    _log(SPAWN__MESSAGE, "SpawnMgr::PrepSpawn() - factionID is %u for region %u. (config set %s)", factionID, regionID, (sConfig.npc.RatFaction?"true":"false"));

    // get faction's ship typeclass and groupID map
    if (sDataMgr.GetRatGroups(factionID, m_factionGroups)) {
        _log(SPAWN__MESSAGE, "SpawnMgr::PrepSpawn() - m_factionGroups size is %u.", m_factionGroups.size());    //should be 12
    } else {
        _log(SPAWN__ERROR, "SpawnMgr::PrepSpawn() - No RatFaction data for faction %u.  Cancelling spawn.", factionID);
        return false;
    }

    /*spawn class is type of spawn based on system security rating
     * 1-7 are 'normal' roid rat spawns
     * 8 is hauler spawns (convoy, carrier, trailer, transporter, bulker, trucker, loader)
     * 9 is commander spawns
     * 10 is officer spawns
     *sub is the type subgroup number.  nothing special here.
     */

    // get possible spawn groups for this secRating.
    uint8 type = 0;
    if ((secRating < 0) && pSysBubble->IsBelt()) {   // check for hauler, commander, officer spawn, but ONLY in a belt
        //NOTE  random checks here are for TESTING only....all rates are high.  make config option later
        double rand = MakeRandomFloat();
        if (rand < 0.1) { //check for officer spawn
            if (factionID != factionRogueDrones)   //but not for drones.  they dont have officers..make this the rare drone hauler spawn
                type = 10;
        } else if (rand < 0.15) { //check for commander spawn
            type = 9;
        } else if (rand < 0.25) { //check for hauler spawn   TODO this needs work.  haulers are subclassed by size in db under same groupID.
            if (factionID != factionRogueDrones)    // hauler spawn for drones already set above...negate this one.
                type = 8;
        }
    }
    if ((type == 0) && pSysBubble->IsBelt()) {  // gonna be a 'regular' trusec-based spawn in a belt.
        if (secRating < -0.8)  type = 7;
        else if (secRating < -0.5) type = 6;
        else if (secRating < -0.2) type = 5;
        else if (secRating < 0.1) type = 4;
        else if (secRating < 0.4) type = 3;
        else if (secRating < 0.7) type = 2;
        else type = 1;
    }

    /** @todo write code to spawn smaller groups on gates */

    RatSpawnClassVec spawnEntry;
    if (sDataMgr.GetRatClasses(type, spawnEntry)) {
        _log(SPAWN__MESSAGE, "SpawnMgr::PrepSpawn() - spawnEntry size is %u.", spawnEntry.size());    //variable
    } else {
        _log(SPAWN__ERROR, "SpawnMgr::PrepSpawn() - No RatClass data for class %u.  Cancelling spawn.", type);
        return false;
    }

    // get ship class data from spawnEntry.at(subtype)
    // and put this spawn's group information in class designators
    uint8 subtype = MakeRandomInt(0, spawnEntry.size());
    uint8 f = spawnEntry.at(subtype).f;
    uint8 d = spawnEntry.at(subtype).d;
    uint8 c = spawnEntry.at(subtype).c;
    uint8 bc = spawnEntry.at(subtype).bc;
    uint8 bs = spawnEntry.at(subtype).bs;
    uint8 h = spawnEntry.at(subtype).h;
    uint8 o = spawnEntry.at(subtype).o;
    uint8 cf = spawnEntry.at(subtype).cf;
    uint8 cd = spawnEntry.at(subtype).cd;
    uint8 cc = spawnEntry.at(subtype).cc;
    uint8 cbc = spawnEntry.at(subtype).cbc;
    uint8 cbs = spawnEntry.at(subtype).cbs;
    spawnEntry.clear();

    // get typeIDs to spawn based on info in m_factionGroups and ship designators and put into Spawn Vector
    SpawnGroup toSpawn;
    if (f > 0) {
        toSpawn.typeID = GetRandTypeID(1);
        toSpawn.quantity = f;
        m_toSpawn.push_back(toSpawn);
    }
    if (d > 0) {
        toSpawn.typeID = GetRandTypeID(2);
        toSpawn.quantity = d;
        m_toSpawn.push_back(toSpawn);
    }
    if (c > 0) {
        toSpawn.typeID = GetRandTypeID(3);
        toSpawn.quantity = c;
        m_toSpawn.push_back(toSpawn);
    }
    if (bc > 0) {
        toSpawn.typeID = GetRandTypeID(4);
        toSpawn.quantity = bc;
        m_toSpawn.push_back(toSpawn);
    }
    if (bs > 0) {
        toSpawn.typeID = GetRandTypeID(5);
        toSpawn.quantity = bs;
        m_toSpawn.push_back(toSpawn);
    }
    if (h > 0) {
        toSpawn.typeID = GetRandTypeID(6);
        toSpawn.quantity = h;
        m_toSpawn.push_back(toSpawn);
    }
    if ((o > 0) && (factionID != factionRogueDrones)) {    //drones do NOT have officers (internal type 7).
        toSpawn.typeID = GetRandTypeID(7);
        toSpawn.quantity = o;
        m_toSpawn.push_back(toSpawn);
    }
    if (cf > 0) {
        toSpawn.typeID = GetRandTypeID(8);
        toSpawn.quantity = cf;
        m_toSpawn.push_back(toSpawn);
    }
    if (cd > 0) {
        toSpawn.typeID = GetRandTypeID(9);
        toSpawn.quantity = cd;
        m_toSpawn.push_back(toSpawn);
    }
    if (cc > 0) {
        toSpawn.typeID = GetRandTypeID(10);
        toSpawn.quantity = cc;
        m_toSpawn.push_back(toSpawn);
    }
    if (cbc > 0) {
        toSpawn.typeID = GetRandTypeID(11);
        toSpawn.quantity = cbc;
        m_toSpawn.push_back(toSpawn);
    }
    if (cbs > 0) {
        toSpawn.typeID = GetRandTypeID(12);
        toSpawn.quantity = cbs;
        m_toSpawn.push_back(toSpawn);
    }

    if ((factionID == factionRogueDrones) and ((bc > 0) or (bs > 0))) {
        // spawn 4 drone swarm-class ships for each bc/bs
        toSpawn.typeID = GetRandTypeID(7);
        toSpawn.quantity = ((bs > 0 ? bs : bc) *4);
        m_toSpawn.push_back(toSpawn);
    }

    //cleanup
    m_factionGroups.clear();

    if (m_toSpawn.size() > 0) {
        _log(SPAWN__MESSAGE, "SpawnMgr::PrepSpawn() - toSpawn size is %u.", m_toSpawn.size());    //variable
        MakeSpawn(pSysBubble, factionID, type, subtype);
        return true;
    } else
        _log(SPAWN__ERROR, "SpawnMgr::PrepSpawn() - Nothing to spawn.");

    return false;
}

uint32 SpawnMgr::GetRandTypeID(uint32 shipClass)
{
    // get rat faction's groupID based on previously selected shipClass
    uint32 groupID = 0;
    RatFactionGroupsMap::iterator itr = m_factionGroups.find(shipClass);
    if (itr != m_factionGroups.end())
        groupID = itr->second;
    else {
        _log(SPAWN__ERROR, "SpawnMgr::GetRandTypeID() - Failed to find groupID for shipClass %u.", shipClass);
        return 0;
    }
   //get typeIDs for this groupID from m_types and return only one for spawning
   /** @todo  will need to check typeIDs here for higher-level ships in hi-sec systems */
    std::vector<uint32> typeVec;
    if (sDataMgr.GetRatTypes(groupID, typeVec))  //groupID is key, typeID is value
        return typeVec.at(MakeRandomInt(0, typeVec.size()));
    else
        return 0;
}

/*
struct SpawnEntry { // notes for me while creating/writing/testing
    uint8 spawnType;// spawn type.  1 = roaming, 2 = static
    uint8 total;    // total number of this group spawned
    uint8 number;   // spawn number in this group
    uint8 sub;      // spawn data subtype
    uint8 type;     // spawn data class id (in case we have to look it up again)
    uint16 typeID;  // rat type id
    uint32 itemID;  // rat entity id
    uint32 groupID; // rat group id (may look into changing typeID within group later on respawn (for chaining))
    uint32 spawnID; // spawn id (if needed to match up with other spawns of this group for warp or w/e (multiple spawn types in this group))
    uint32 time;    // spawn group timer run time
};
*/
void SpawnMgr::ReSpawn(SystemBubble* pSysBubble, SpawnEntry& spawnEntry)
{
    GPoint startPos(pSysBubble->GetCenter());
    startPos.MakeRandomPointOnSphere(MakeRandomInt(10, 15) *100000); //1-1m5 km from current bubble center
    const GPoint warpToPoint(pSysBubble->GetCenter());
    _log(SPAWN__TRACE, "ReSpawn()  data for spawnEntryID %u  0x%X is type:%u, corp:%u, faction:%u, #:%u of %u", \
                spawnEntry.spawnID, &spawnEntry, spawnEntry.typeID, spawnEntry.corpID, \
                spawnEntry.factionID, spawnEntry.number, spawnEntry.total);
    /*
     *        ItemData( uint32 _typeID, uint32 _ownerID, uint32 _locationID, EVEItemFlags _flag, const char *_name = "",
     *                  const GPoint &_position = NULL_ORIGIN, const char *_customInfo = "", bool _contraband = false);
     */
    ItemData idata(spawnEntry.typeID, spawnEntry.corpID, m_system->GetID(), flagAutoFit, "", startPos, "BeltRat");
    InventoryItemRef iRef = sItemFactory.SpawnItem(idata);      // will have to work on this to NOT save npc to db.
    if (iRef.get() == nullptr) {
        _log(SPAWN__ERROR, "Failed to spawn item type %u.", spawnEntry.typeID);
        return;
    }

    _log(SPAWN__POP, "SpawnMgr::ReSpawn - Spawning NPC %s(%u)", iRef->itemName().c_str(), iRef->itemID());

    FactionData data;
        data.allianceID = 0;
        data.corporationID = spawnEntry.corpID;
        data.factionID = spawnEntry.factionID;
        data.ownerID = spawnEntry.corpID;
    NPC* npc = new NPC(iRef, m_services, m_system, data, this);

    // NPC::Load() no longer does anything.  it is still here in case we find a new use for it.
    if (!npc->Load()) {
        _log(SPAWN__ERROR, "Failed to load NPC data for NPC %u with type %u, depoping.", npc->GetID(), npc->GetSelf()->typeID());
        SafeDelete(npc);
        return;
    }

    //drop this npc into system, and begin warp.  this may have to be looked into later for timing of large spawns (>6)
    m_system->AddNPC(npc);
    startPos.MakeRandomPointOnSphere(MakeRandomInt(5, 10) *1000);
    npc->DestinyMgr()->WarpTo(warpToPoint, (MakeRandomInt(-5, 10) *1000));

    spawnEntry.enabled = false;
    _log(SPAWN__TRACE, "ReSpawn() completed for spawnEntryID %u 0x%X in bubble %u.", spawnEntry.spawnID, &spawnEntry, pSysBubble->GetID());
}

void SpawnMgr::MakeSpawn(SystemBubble* pSysBubble, uint32 factionID, uint8 type, uint8 subtype)
{
    NPC* npc(nullptr);
    SpawnEntry se;

    /*  the point here is to have all belt rats spawn outside their belt's bubble.
     * to make it 'realistic', they will need the appearance of warping in from some random point,
     *  to somewhere around bubble center.  this will make their origin appear elsewhere,
     * but not from same place every time.  they're pirates, they got other shit to do, too.
     *  eventually, when other systems are working, npcs will appear to 'warp in' from a hideout in the current system.
     *  this particular bit is not general knowledge and will have to be thought out a bit more before coding.
     *
     */
    GPoint startPos(pSysBubble->GetCenter());
    const GPoint warpToPoint(startPos);
    startPos.MakeRandomPointOnSphere(MakeRandomInt(10, 15) *100000); //1-1m5 km from current bubble center

    uint32 corpID = sDataMgr.GetCorpID(factionID);

    RatSpawnGroupVec::iterator cur = m_toSpawn.begin();

    FactionData data;
        data.allianceID = factionID;
        data.corporationID = corpID;
        data.factionID = (factionID == factionRogueDrones ? 0 : factionID); // the faction of rogue drones is wrong....should be "0" for client to use it right.
        data.ownerID = corpID;

    while (cur != m_toSpawn.end()) {
        if (cur->typeID == 0)
            return; // this is an error.
        /*
        ItemData( uint32 _typeID, uint32 _ownerID, uint32 _locationID, EVEItemFlags _flag, const char *_name = "",
                  const GPoint &_position = NULL_ORIGIN, const char *_customInfo = "", bool _contraband = false);
        */
        ItemData idata(cur->typeID, corpID, m_system->GetID(), flagAutoFit, "", startPos, "BeltRat");

        for (uint8 x=0; x!=cur->quantity; ++x) {
            InventoryItemRef iRef = sItemFactory.SpawnItem(idata);      // will have to work on this to NOT save npc to db....or save ALL the spawn shit
            if (iRef.get() == nullptr) {
                _log(SPAWN__ERROR, "Failed to spawn item type %u.", cur->typeID);
                continue;
            }

            _log(SPAWN__POP, "SpawnMgr::MakeSpawn - Spawning NPC type %u (%u)", cur->typeID, iRef->itemID());

            npc = new NPC(iRef, m_services, m_system, data, this);

            // NPC::Load() no longer does anything.  it is still here in case we find a new use for it.
            if (!npc->Load()) {
                _log(SPAWN__ERROR, "Failed to load NPC data for NPC %u with type %u, depoping.", npc->GetID(), npc->GetSelf()->typeID());
                SafeDelete(npc);
                continue;
            }

            m_system->AddNPC(npc);

            //startPos.MakeRandomPointOnSphere(MakeRandomInt(5, 10) *1000);
            npc->DestinyMgr()->SetPosition(startPos);
            //  begin warp.  this may have to be looked into later for timing of large spawns (>6)
            npc->DestinyMgr()->WarpTo(warpToPoint, (MakeRandomInt(-5, 10) *1000));

            se.enabled = false;
            se.groupID = iRef->type().groupID();
            se.itemID = iRef->itemID();
            se.total = cur->quantity;
            se.number = x;
            se.typeID = cur->typeID;
            se.spawnID = m_spawnID;
            se.corpID = corpID;
            se.factionID = factionID;
            se.type = type;
            se.sub = subtype;
            se.time = (sConfig.npc.RoamingTimer *60 *1000);
            m_spawns.emplace(pSysBubble->GetID(), se);
            _log(SPAWN__TRACE, "MakeSpawn() adding SpawnEntry with ID %u to m_spawns.", se.spawnID);
        }
        ++cur;
    }

    ++m_spawnID;
    m_bubbles.push_back(pSysBubble);

    //cleanup
    m_toSpawn.clear();
    m_ratSpawns.clear();

    _log(SPAWN__TRACE, "MakeSpawn() completed. %u bubbles in m_bubbles. %u entities in m_spawns.", m_bubbles.size(), m_spawns.size());
}

void SpawnMgr::RemoveSpawn(uint32 bubbleID, uint32 itemID)
{
    auto range = m_spawns.equal_range(bubbleID);
    auto itr = range.first;
    while (itr != range.second) {
        if (itr->second.itemID == itemID) {
            _log(SPAWN__TRACE, "RemoveSpawn() found item %u in spawnID %u and removed it.", itemID, itr->second.spawnID);
            m_spawns.erase(itr);
            return;
        }
        ++itr;
    }

    _log(SPAWN__TRACE, "RemoveSpawn() did not find item %u in bubble %u, out of %u total spawns in the map.", itemID, bubbleID, m_spawns.size());
    return;
}
