
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
#include "npc/NPCAI.h"
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
  m_ratTimer(0),
  m_ratGroupTimer(0),
  m_missionTimer(0),
  m_incursionTimer(0),
  m_deadspaceTimer(0)
{
    m_spawnID = 1;

    m_ratEnabled = false;
    m_initalized = false;
    /*
    m_ratTimer.Disable();
    m_missionTimer.Disable();
    m_ratGroupTimer.Disable();
    m_incursionTimer.Disable();
    m_deadspaceTimer.Disable();
    */

    m_spawns.clear();
    m_bubbles.clear();
    m_toSpawn.clear();
    m_ratSpawns.clear();
    m_ratSpawnClass.clear();        // not used
    m_factionGroups.clear();
}

bool SpawnMgr::Init()
{
    if (!sConfig.npc.RoamingSpawns and !sConfig.npc.StaticSpawns) {
        _log(COSMIC_MGR__MESSAGE, "Spawn System Disabled.  Not Initalizing Spawn Manager for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
        return true;
    }

    m_groupTimerSetTime = 150;  // (in seconds) 2.5m default check time. this will allow a max wait time of 7.5m for respawn

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
    if (m_ratTimer.Enabled())  {
        if (m_ratTimer.Check(false)) {
            m_ratTimer.Disable();
            m_ratEnabled = true;
            _log(SPAWN__MESSAGE, "SpawnMgr::Process() - Main Timer called.  Spawn functions enabled for %s(%u).",
                 m_system->GetName().c_str(), m_system->GetID());
        }
        if (sConfig.debug.UseProfiling)
            sProfile.AddTime(_spawnProfile, GetTimeUSeconds() - profileStartTime);
        return;
    }

    if (!m_ratEnabled)
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
    if (m_ratGroupTimer.Enabled())
        if (m_ratGroupTimer.Check()) {
            bool killTimer = true;
                SpawnEntryDef::iterator itr = m_spawns.begin();
                while (itr != m_spawns.end()) {
                    if (itr->second.enabled) {
                        killTimer = false;
                        if (itr->second.stamp < sEntityList.GetStamp()) {
                            ++itr;
                            continue;
                        }
                        _log(SPAWN__TRACE, "Process() calling Respawn for SpawnEntryID %u (0x%X)", \
                                    itr->second.spawnID, &itr->second);
                        // this means check SpawnEntry for 'missing' SpawnGroup members and respawn as needed.
                        ReSpawn(sBubbleMgr.FindBubbleByID(m_system->GetID(), itr->first), itr->second);
                        itr->second.enabled = false;
                    }
                    ++itr;
                }

            if (killTimer) {
                m_ratGroupTimer.Disable();
                _log(SPAWN__MESSAGE, "SpawnMgr::Process() - Rat Spawn Groups full (or no spawns) for %s(%u).  RatGroup Timer disabled.", \
                            m_system->GetName().c_str(), m_system->GetID());
            } /* else {
                // this doesnt matter...we're checking stamp times for respawn time.
                uint32 cTime = m_ratGroupTimer.GetRemainingTime();
                m_ratGroupTimer.Disable();
                m_ratGroupTimer.Start(cTime /2);    // set timer to half remaining time.  config option?  do this different later?
            } */
        }

    if (sConfig.debug.UseProfiling)
        sProfile.AddTime(_spawnProfile, GetTimeUSeconds() - profileStartTime);
}

void SpawnMgr::MoveSpawn(NPC* pNPC, SystemBubble* pBubble)
{
    if (pNPC == nullptr)
        return;
    if (pBubble == nullptr)
        return;
    uint32 npcID =  pNPC->GetID();
    _log(SPAWN__TRACE, "MoveSpawn() called by %s(%u) from bubbleID %u to bubbleID %u", pNPC->GetName(), npcID, pNPC->SysBubble()->GetID(), pBubble->GetID() );

    // get npc spawn data and add to new location
    auto range = m_spawns.equal_range(pNPC->SysBubble()->GetID());
    auto itr = range.first;
    while (itr != range.second) {
        if (itr->second.itemID == npcID) {
            m_spawns.emplace(pBubble->GetID(), itr->second);
            m_spawns.erase(itr);
            break;
        }
        ++itr;
    }

    //find out if any rats remain before setting bubble spawned to false
    if (pNPC->SysBubble()->CountNPCs() < 2)
        pNPC->SysBubble()->SetSpawned(false);
    // this will NOT count as new bubble being spawned.
    //pBubble->SetSpawned(true);
}

void SpawnMgr::WarpOutSpawn(NPC* pNPC, SystemBubble* pBubble)
{
    if (pNPC == nullptr)
        return;
    if (pBubble == nullptr)
        return;
    _log(SPAWN__TRACE, "WarpOutSpawn() called by %s(%u) from bubbleID %u to bubbleID %u", pNPC->GetName(), pNPC->GetID(), pNPC->SysBubble()->GetID(), pBubble->GetID() );
    NPC* rNPC(nullptr);
    auto range = m_spawns.equal_range(pNPC->SysBubble()->GetID());
    auto itr = range.first;
    while (itr != range.second) {
        if (itr->second.enabled) {
            ++itr;
            continue;
        }
        rNPC = m_system->GetNPCSE(itr->second.itemID);
        if (rNPC == nullptr) {
            ++itr;
            continue;
        }
        rNPC->DestinyMgr()->WarpTo(pBubble->GetCenter(), MakeRandomFloat(10, 30) *100);
        rNPC->GetAIMgr()->DisableWarpOutTimer();
        m_spawns.emplace(pBubble->GetID(), itr->second);
        m_spawns.erase(itr);
        ++itr;
    }

    pNPC->SysBubble()->SetSpawned(false);
    pBubble->SetSpawned(true);
}


void SpawnMgr::StartRatTimer()
{
    if (m_ratTimer.Enabled())
        return;
    uint16 time = sConfig.npc.RoamingTimer *1000;  //  s to ms
    if (sConfig.debug.SpawnTest)
        time = 5000; /* 5s for npc spawn testing */
    m_ratTimer.Start(time);

    if (is_log_enabled(SPAWN__MESSAGE))
        _log(SPAWN__MESSAGE, "SpawnMgr::StartRatTimer() - Main Spawn Timer started for %s(%u) at %u ms.", \
            m_system->GetName().c_str(), m_system->GetID(), time);
}

void SpawnMgr::StartRatGroupTimer()
{
    if (m_ratGroupTimer.Enabled()) {
        if (is_log_enabled(SPAWN__MESSAGE))
            _log(SPAWN__MESSAGE, "SpawnMgr::StartRatGroupTimer() - Group Spawn Timer currently running.  Time left: %us", m_ratGroupTimer.GetRemainingTime() /1000);
        return;
    }
    m_ratGroupTimer.Start(m_groupTimerSetTime *1000);

    if (is_log_enabled(SPAWN__MESSAGE))
        _log(SPAWN__MESSAGE, "SpawnMgr::StartRatGroupTimer() - Group Spawn Timer started for %s(%u) at %us.", \
            m_system->GetName().c_str(), m_system->GetID(), m_groupTimerSetTime);
}

void SpawnMgr::SpawnKilled(SystemBubble* pBubble, uint32 itemID)
{
    if (pBubble == nullptr)
        return;
    if (pBubble->IsBelt()) {
        _log(SPAWN__DEPOP, "SpawnMgr::SpawnKilled::Belt - called by %u.", itemID);
        // if any SpawnEntry still exists for this bubble, reset group timer.
        // this enables chain ratting
        bool killed = true;
        auto range = m_spawns.equal_range(pBubble->GetID());
        auto itr = range.first;
        while (itr != range.second) {
            if (itr->second.itemID == itemID) {
                itr->second.stamp = sEntityList.GetStamp() + sConfig.npc.RespawnTimer; // in seconds
                itr->second.enabled = true;
            }
            if (!itr->second.enabled)
                killed = false;     // at least one rat left.
            ++itr;
        }
        if (killed) {
            _log(SPAWN__DEPOP, "SpawnMgr::SpawnKilled - Belt Spawn has been destoyed.  Resetting spawn checks for bubble %u.", pBubble->GetID());
            // spawn destroyed.  delete from list and reset bubble checks.
            m_spawns.erase(pBubble->GetID()); // just in case....may/may not be in here.
            m_bubbles.erase(std::find(m_bubbles.begin(), m_bubbles.end(), pBubble));
            pBubble->ResetBubbleRatSpawn();
            m_system->RemoveSpawnBubble(pBubble);
            return;
        } else {
            StartRatGroupTimer();
        }
    } else if (pBubble->IsGate()) {
        _log(SPAWN__DEPOP, "SpawnMgr::SpawnKilled::Gate - called by %u.", itemID);
        // we are not enabling rat chaining on gates.
        RemoveSpawn(pBubble->GetID(), itemID);
        if (pBubble->CountNPCs() < 2)
            pBubble->SetSpawned(false);
    } else if (pBubble->IsAnomaly()) {
        _log(SPAWN__DEPOP, "SpawnMgr::SpawnKilled::Anomaly - called by %u.", itemID);
        // placeholder - not coded yet.
        /*  this needs to deal with multiple things.
         * 1- unlocking warp gates when needed per wave
         * 2- dropping loot according to (wave/dungeon/template)?
         * 3- after last spawn, possible escelation per dungeon type?   this should signal anomaly mgr to create the escelation
         * 4- more/others?
         */
    } else if (pBubble->IsMission()) {
        _log(SPAWN__DEPOP, "SpawnMgr::SpawnKilled::Mission - called by %u.", itemID);
        // placeholder - not coded yet.
        /*  this needs to deal with multiple things.
         * 1- unlocking warp gates when needed per wave
         * 2- dropping loot according to (wave/mission/template)?
         * 3- setting mission completion status
         * 4- more/others?
         */
    } else if (pBubble->IsIncursion()) {
        _log(SPAWN__DEPOP, "SpawnMgr::SpawnKilled::Incursion - called by %u.", itemID);
        // placeholder - not coded yet.
    } else {
        _log(SPAWN__DEPOP, "SpawnMgr::SpawnKilled::Other - called by %u.", itemID);
        RemoveSpawn(pBubble->GetID(), itemID);
    }
}

void SpawnMgr::DoSpawnForAnomaly(SystemBubble* pBubble, int32 spawnID)
{
    if (!m_ratEnabled)
        return;
    if (pBubble == nullptr)
        return;
    /*  this needs to deal with multiple things.
     * 1- rat types for anomaly....
     * 2- rat faction per dungeon template
     * 3- waves per template.  how to do this???
     * 4- more/others?
     */
}

void SpawnMgr::DoSpawnForIncursion(SystemBubble* pBubble, uint32 regionID)
{
    if (!m_ratEnabled)
        return;
    if (pBubble == nullptr)
        return;
    if (!IsRegion(regionID))
        return;
    // unknown parameters at this time
}

void SpawnMgr::DoSpawnForMission(SystemBubble* pBubble, uint32 regionID)
{
    if (!m_ratEnabled)
        return;
    if (pBubble == nullptr)
        return;
    if (!IsRegion(regionID))
        return;
    // unknown parameters at this time

    /*  this needs to deal with multiple things.
     * 1- rat types for mission....
     * 2- rat faction per mission template
     * 3- waves per template.  how to do this???
     * 4- more/others?
     */
}

bool SpawnMgr::DoSpawnForBubble(SystemBubble* pBubble, uint32 regionID, double secRating)
{
    if (!m_ratEnabled)
        return false;
    if (pBubble == nullptr)
        return false;
    if (!IsRegion(regionID))
        return false;
    double profileStartTime = 0.0;
    if (sConfig.debug.UseProfiling)
        profileStartTime = GetTimeUSeconds();
    if (FindSpawnForBubble(pBubble->GetID())) {
        _log(SPAWN__TRACE, "SpawnMgr::FindSpawnForBubble() returned true for bubble %u.", pBubble->GetID());
        pBubble->SetSpawned(true);  // bubble flag to avoid multiple spawns in same bubble.
        return false;
    } else {
        if (PrepSpawn(pBubble, regionID, secRating)) {
            pBubble->SetSpawned(true);  // bubble flag to avoid multiple spawns in same bubble.
        } else {
            _log(SPAWN__TRACE, "SpawnMgr::PrepSpawn() returned false for bubble %u.", pBubble->GetID());
            return false;
        }
    }

    if (pBubble->IsBelt())
        m_system->IncRatSpawnCount();
    if (pBubble->IsGate())
        m_system->IncGateSpawnCount();

    /* this will throw off the accuracy of the profile, as this and SpawnMgr::Process() use the same data container */
    if (sConfig.debug.UseProfiling)
        sProfile.AddTime(_spawnProfile, GetTimeUSeconds() - profileStartTime);
    return true;
}

bool SpawnMgr::FindSpawnForBubble(uint16 bubbleID) {
    SpawnEntryDef::iterator itr = m_spawns.find(bubbleID);
    if (itr != m_spawns.end())
        return true;

    return false;
}

bool SpawnMgr::PrepSpawn(SystemBubble* pBubble, uint32 regionID, double secRating)
{
    if (pBubble == nullptr)
        return false;
    // get faction for this region
    uint32 factionID = factionRogueDrones;  // default to rogue drones.  this is my internal rogue drone factionID.
    if (sConfig.npc.RatFaction)             // is RatFaction set in config?
        factionID = sConfig.npc.RatFaction;
    else if (MakeRandomFloat() < 0.15)      // random chance for ANY beltspawn to be rogue drone...if chance < 0.15, rat = drone.
        factionID = sDataMgr.GetRegionRatFaction(regionID);

    if (is_log_enabled(SPAWN__MESSAGE))
        _log(SPAWN__MESSAGE, "SpawnMgr::PrepSpawn() - faction: %s, region %u. (config set %s)", \
                sDataMgr.GetFactionName(factionID).c_str(), regionID, (sConfig.npc.RatFaction?"true":"false"));

    // get faction's ship typeclass and groupID map...is this feasible?  we're getting ALL groups for this faction.
    if (sDataMgr.GetRatGroups(factionID, m_factionGroups)) {
        if (is_log_enabled(SPAWN__MESSAGE))
            _log(SPAWN__MESSAGE, "SpawnMgr::PrepSpawn() - m_factionGroups size is %u.", m_factionGroups.size());    //should be 14
    } else {
        _log(SPAWN__ERROR, "SpawnMgr::PrepSpawn() - No RatFaction data for faction %u.  Cancelling spawn.", factionID);
        return false;
    }

    // get possible spawn groups for this secRating.
    uint8 sClass = Spawn::Class::None;
    if ((secRating < 0.2) and pBubble->IsBelt()) {   // check for hauler, commander, officer spawn, but ONLY in a belt
        //NOTE  random checks here are for TESTING only....all rates are high.  make config option later?
        double rand = MakeRandomFloat();
        if (rand < 0.1) { // officer spawn
            if (factionID != factionRogueDrones)   //but not for drones.  they dont have officers..make this the rare drone hauler spawn (which isnt written yet)
                sClass = Spawn::Class::Officer;
            else
                sClass = Spawn::Class::Hauler;
        } else if (rand < 0.15) { // commander spawn
            sClass = Spawn::Class::Commander;
        } else if (rand < 0.25) { // hauler spawn
            if (factionID != factionRogueDrones)
                sClass = Spawn::Class::Hauler;
        }
    }
    if ((sClass == Spawn::Class::None) and pBubble->IsBelt()) {
        /** @todo make (small) random chance for 'hell' spawn in nullsec */
        if ((factionID != factionRogueDrones) and (MakeRandomFloat() < 0.08)) { // second chance for hauler spawn, but include ALL secRating in this one
            sClass = Spawn::Class::Hauler;
        } else { // gonna be a 'regular' trusec-based spawn in a belt.
            if (secRating < -0.7)  sClass = Spawn::Class::Insane;
            else if (secRating < -0.4) sClass = Spawn::Class::Crazy;
            else if (secRating < -0.1) sClass = Spawn::Class::Hard;
            else if (secRating < 0.3) sClass = Spawn::Class::Medium;
            else if (secRating < 0.6) sClass = Spawn::Class::Average;
            else if (secRating < 0.85) sClass = Spawn::Class::Fair;
            else sClass = Spawn::Class::Easy;
        }
    } else if (pBubble->IsGate()) { // gate spawns are smaller than roid spawns
        if (secRating < -0.7)  sClass = Spawn::Class::Hard;
        else if (secRating < -0.4) sClass = Spawn::Class::Medium;
        else if (secRating < -0.1) sClass = Spawn::Class::Average;
        else if (secRating < 0.3) sClass = Spawn::Class::Fair;
        else sClass = Spawn::Class::Easy;
    }

    if (secRating < 0) {
        if (pBubble->IsBelt()) {
            if (MakeRandomFloat() < 0.15)
                sClass = Spawn::Class::Hell;
        } else {
            if (MakeRandomFloat() < 0.08)
                sClass = Spawn::Class::Hell;
        }
    }

    RatSpawnClassVec spawnEntry;
    if (sDataMgr.GetRatClasses(sClass, spawnEntry)) {
        if (is_log_enabled(SPAWN__MESSAGE))
            _log(SPAWN__MESSAGE, "SpawnMgr::PrepSpawn() - spawnEntry - size: %u, class: '%s', ", spawnEntry.size(), GetSpawnClassName(sClass).c_str());
    } else {
        _log(SPAWN__ERROR, "SpawnMgr::PrepSpawn() - No RatClass data for class '%s'.  Cancelling spawn.", GetSpawnClassName(sClass).c_str());
        return false;
    }

    // get ship class data from spawnEntry.at(subtype)
    // and put this spawn's group information in class designators
    uint8 subtype = 0;
    if (sClass == Spawn::Class::Hauler) {
        // split hauler spawns based on trusec
        if (secRating < -0.8)       subtype = MakeRandomInt(4, 7);
        else if (secRating < -0.5)  subtype = MakeRandomInt(3, 6);
        else if (secRating < -0.2)  subtype = MakeRandomInt(2, 5);
        else if (secRating < 0.1)   subtype = MakeRandomInt(1, 4);
        else if (secRating < 0.4)   subtype = MakeRandomInt(1, 3);
        else if (secRating < 0.7)   subtype = 2;
        else                        subtype = 1;
    } else
        subtype = MakeRandomInt(0, spawnEntry.size());

    uint8 f = spawnEntry.at(subtype).f;
    uint8 af = spawnEntry.at(subtype).af;
    uint8 d = spawnEntry.at(subtype).d;
    uint8 c = spawnEntry.at(subtype).c;
    uint8 ac = spawnEntry.at(subtype).ac;
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
    if (af > 0) {
        toSpawn.typeID = GetRandTypeID(2);
        toSpawn.quantity = af;
        m_toSpawn.push_back(toSpawn);
    }
    if (d > 0) {
        toSpawn.typeID = GetRandTypeID(3);
        toSpawn.quantity = d;
        m_toSpawn.push_back(toSpawn);
    }
    if (c > 0) {
        toSpawn.typeID = GetRandTypeID(4);
        toSpawn.quantity = c;
        m_toSpawn.push_back(toSpawn);
    }
    if (ac > 0) {
        toSpawn.typeID = GetRandTypeID(5);
        toSpawn.quantity = ac;
        m_toSpawn.push_back(toSpawn);
    }
    if (bc > 0) {
        toSpawn.typeID = GetRandTypeID(6);
        toSpawn.quantity = bc;
        m_toSpawn.push_back(toSpawn);
    }
    if (bs > 0) {
        toSpawn.typeID = GetRandTypeID(7);
        toSpawn.quantity = bs;
        m_toSpawn.push_back(toSpawn);
    }
    if (h > 0) {
        toSpawn.typeID = GetRandTypeID(8);
        toSpawn.quantity = h;
        m_toSpawn.push_back(toSpawn);
    }
    if ((o > 0) and (factionID != factionRogueDrones)) {    //drones do NOT have officers (internal type 9).
        toSpawn.typeID = GetRandTypeID(9);
        toSpawn.quantity = o;
        m_toSpawn.push_back(toSpawn);
    }
    if (cf > 0) {
        toSpawn.typeID = GetRandTypeID(10);
        toSpawn.quantity = cf;
        m_toSpawn.push_back(toSpawn);
    }
    if (cd > 0) {
        toSpawn.typeID = GetRandTypeID(11);
        toSpawn.quantity = cd;
        m_toSpawn.push_back(toSpawn);
    }
    if (cc > 0) {
        toSpawn.typeID = GetRandTypeID(12);
        toSpawn.quantity = cc;
        m_toSpawn.push_back(toSpawn);
    }
    if (cbc > 0) {
        toSpawn.typeID = GetRandTypeID(13);
        toSpawn.quantity = cbc;
        m_toSpawn.push_back(toSpawn);
    }
    if (cbs > 0) {
        toSpawn.typeID = GetRandTypeID(14);
        toSpawn.quantity = cbs;
        m_toSpawn.push_back(toSpawn);
    }

    if (factionID == factionRogueDrones) {
        if ((bc > 0) or (bs > 0)) {
            // spawn 4 swarm ships for each bc/bs
            toSpawn.typeID = GetRandTypeID(9);
            toSpawn.quantity = ((bs > 0 ? bs : bc) *4);
            m_toSpawn.push_back(toSpawn);
        } else if (o > 0) {
            // drones dont have officers.  spawn swarm
            toSpawn.typeID = GetRandTypeID(9);
            toSpawn.quantity = o *10;
            m_toSpawn.push_back(toSpawn);
        }
    }

    //cleanup
    m_factionGroups.clear();

    if (m_toSpawn.size() > 0) {
        if (is_log_enabled(SPAWN__MESSAGE))
            _log(SPAWN__MESSAGE, "SpawnMgr::PrepSpawn() - toSpawn size is %u.", m_toSpawn.size());    //variable
        MakeSpawn(pBubble, factionID, sClass, subtype);
        return true;
    } else
        _log(SPAWN__ERROR, "SpawnMgr::PrepSpawn() - Nothing to spawn.");

    return false;
}

uint16 SpawnMgr::GetRandTypeID(uint8 sClass)
{
    uint16 groupID = 0;
    RatFactionGroupsMap::iterator itr = m_factionGroups.find(sClass);
    if (itr != m_factionGroups.end())
        groupID = itr->second;
    else {
        _log(SPAWN__ERROR, "SpawnMgr::GetRandTypeID() - Failed to find groupID for shipClass %s.", GetSpawnClassName(sClass).c_str());
        return 0;
    }

    return sDataMgr.GetRandRatType(sClass, groupID);
}

/*
struct SpawnEntry {     // notes for me while creating/writing/testing
    bool enabled;       // is respawn enabled for this entry?  also provides conditional test for SpawnMgr::IsChaining() method
    uint8 spawnClass;   // spawn class.  0 = none, 1-7 = easy to insane based on sysSec, 8 = hauler, 9 = commander, 10 = officer
    uint8 spawnGroup;   // spawn group.   1 = roid rat, 2 = roaming, 3 = static, 4 = anomaly, 5 = mission, 6 = incursion, 7 = deadspace, 8 = sleeper
    uint8 total;        // total number of this group spawned
    uint8 number;       // this rat's number in group (to match up with above total)
    uint8 sub;          // spawn data subtype
    uint8 classID;      // spawn data class id (in case we have to look it up again)
    uint16 typeID;      // rat type id
    uint16 groupID;     // rat group id (may look into changing typeID within group later on respawn (for chaining))
    uint16 spawnID;     // spawn id (if needed to match up with other spawns of this group (multiple spawn types in this group))
    uint32 itemID;      // rat entity id
    uint32 corpID;      // rat corp id
    uint32 factionID;   // rat faction id
    uint16 stamp;       // entry stamp time to respawn (process conditional to allow for common timer and multiple respawn times)
};
*/
void SpawnMgr::ReSpawn(SystemBubble* pBubble, SpawnEntry& spawnEntry)
{
    //  we are NOT enabling spawn chaining for officer, hauler, or commander spawns.
    if (spawnEntry.spawnClass > Spawn::Class::Insane)
        return;
    GPoint startPos(pBubble->GetCenter());
    const GPoint warpToPoint(startPos);
    startPos.MakeRandomPointOnSphere(MakeRandomInt(10, 15) *100000); //1-1m5 km from current bubble center
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
    NPC* pNPC = new NPC(iRef, m_services, m_system, data, this);

    if (!pNPC->Load()) {
        _log(SPAWN__ERROR, "Failed to load NPC data for NPC %u with type %u, depoping.", pNPC->GetID(), pNPC->GetSelf()->typeID());
        SafeDelete(pNPC);
        return;
    }

    m_system->AddNPC(pNPC);
    startPos.MakeRandomPointOnSphere(MakeRandomInt(5, 10) *1000);
    pNPC->DestinyMgr()->WarpTo(warpToPoint, (MakeRandomInt(-5, 10) *1000));

    spawnEntry.stamp = 0;
    spawnEntry.enabled = false;
    _log(SPAWN__TRACE, "ReSpawn() completed for spawnEntryID %u 0x%X in bubble %u.", spawnEntry.spawnID, &spawnEntry, pBubble->GetID());
}

void SpawnMgr::MakeSpawn(SystemBubble* pBubble, uint32 factionID, uint8 sClass, uint8 subClass)
{
    NPC* pNPC(nullptr);
    SpawnEntry se;

    /*  the point here is to have all belt rats spawn outside their belt's bubble.
     * to make it 'realistic', they will need the appearance of warping in from some random point,
     *  to somewhere around bubble center.  this will make their origin appear elsewhere,
     * but not from same place every time.  they're pirates, they got other shit to do, too.
     *  eventually, when other systems are working, npcs will appear to 'warp in' from a hideout in the current system.
     *  this particular bit is not general knowledge and will have to be thought out a bit more before coding.
     *
     */
    GPoint startPos(pBubble->GetCenter());
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
         *        ItemData( uint32 _typeID, uint32 _ownerID, uint32 _locationID, EVEItemFlags _flag, const char *_name = "",
         *                  const GPoint &_position = NULL_ORIGIN, const char *_customInfo = "", bool _contraband = false);
         */
        ItemData idata(cur->typeID, corpID, m_system->GetID(), flagAutoFit, "", startPos, "BeltRat");

        for (uint8 x=0; x!=cur->quantity; ++x) {
            InventoryItemRef iRef = sItemFactory.SpawnItem(idata);      // will have to work on this to NOT save npc to db....or save ALL the spawn shit
            if (iRef.get() == nullptr) {
                _log(SPAWN__ERROR, "Failed to spawn item type %u.", cur->typeID);
                continue;
            }

            _log(SPAWN__POP, "SpawnMgr::MakeSpawn - Spawning NPC type %u (%u)", cur->typeID, iRef->itemID());

            pNPC = new NPC(iRef, m_services, m_system, data, this);

            if (!pNPC->Load()) {
                _log(SPAWN__ERROR, "Failed to load NPC data for NPC %u with type %u, depoping.", pNPC->GetID(), pNPC->GetSelf()->typeID());
                SafeDelete(pNPC);
                continue;
            }

            m_system->AddNPC(pNPC);

            pNPC->DestinyMgr()->SetPosition(startPos);
            //  begin warp.  this may have to be looked into later for timing of large spawns (>6)
            //  actually looks kinda cool when larger ships come in later...
            pNPC->DestinyMgr()->WarpTo(warpToPoint, (MakeRandomInt(-5, 10) *1000));

            se.enabled = false;
            se.groupID = iRef->type().groupID();
            se.itemID = iRef->itemID();
            se.total = cur->quantity;
            se.number = x+1;
            se.typeID = cur->typeID;
            se.spawnID = m_spawnID;
            se.corpID = corpID;
            se.factionID = factionID;
            se.spawnClass = sClass;
            se.spawnGroup = Spawn::Group::Roid;
            se.sub = subClass;
            se.stamp = 0;   // this is for respawn time...do not set here.
            m_spawns.emplace(pBubble->GetID(), se);
            _log(SPAWN__TRACE, "MakeSpawn() adding SpawnEntry with ID %u to m_spawns. Class:%u, Group:%u.", se.spawnID, se.spawnClass, se.spawnGroup);
        }
        ++cur;
    }

    ++m_spawnID;
    m_bubbles.push_back(pBubble);

    //cleanup
    m_toSpawn.clear();
    m_ratSpawns.clear();

    _log(SPAWN__TRACE, "MakeSpawn() completed. %u bubbles in m_bubbles. %u entities in m_spawns.", m_bubbles.size(), m_spawns.size());
}

bool SpawnMgr::IsChaining(uint16 bubbleID)
{
    bool rsp = false;
    auto range = m_spawns.equal_range(bubbleID);
    auto itr = range.first;
    while (itr != range.second) {
        if (itr->second.enabled);
            rsp = true;
        ++itr;
    }

    return rsp;
}

void SpawnMgr::RemoveSpawn(uint16 bubbleID, uint32 itemID)
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

std::string SpawnMgr::GetSpawnClassName(int8 typeID)
{
    switch(typeID) {
        case Spawn::Class::None:        return "None";
        case Spawn::Class::Easy:        return "Easy";
        case Spawn::Class::Fair:        return "Fair";
        case Spawn::Class::Average:     return "Average";
        case Spawn::Class::Medium:      return "Medium";
        case Spawn::Class::Hard:        return "Hard";
        case Spawn::Class::Crazy:       return "Crazy";
        case Spawn::Class::Insane:      return "Insane";
        case Spawn::Class::Hauler:      return "Hauler";
        case Spawn::Class::Commander:   return "Commander";
        case Spawn::Class::Officer:     return "Officer";
        default:                        return "Invalid";
    }
}
