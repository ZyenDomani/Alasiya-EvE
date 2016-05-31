/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2011 The EVEmu Team
 *    For the latest information visit http://evemu.org
 *    ------------------------------------------------------------------------------------
 *    This program is free software; you can redistribute it and/or modify it under
 *    the terms of the GNU Lesser General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option) any later
 *    version.
 *
 *    This program is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License along with
 *    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 *    http://www.gnu.org/copyleft/lesser.txt.
 *    ------------------------------------------------------------------------------------
 *    Author:   Allan
 */

#include "EVEServerConfig.h"
#include "PyServiceMgr.h"
#include "npc/NPC.h"
#include "ship/DestinyManager.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"
#include "system/cosmicMgrs/SpawnMgr.h"


SpawnDataMgr::SpawnDataMgr()
{
}

int SpawnDataMgr::Initialize()
{
    m_db.DeleteSpawnedRats();
    _Populate();
    return 1;
}

void SpawnDataMgr::_Populate()
{
    double start = GetTimeUSeconds();
    DBQueryResult* res = new DBQueryResult();
    DBQueryResult* res2 = new DBQueryResult();
    DBResultRow row, row2;

    m_db.GetRegionFactionInfo(*res);
    while (res->GetRow(row)) {
        //SELECT regionID, ratFactionID FROM mapRegions WHERE ratFactionID != 0
        m_regions.insert(std::pair<uint32, uint32>(row.GetInt(0), row.GetInt(1)));
    }

    res->Reset();
    m_db.GetFactionGroups(*res);
    RatFactionGroups factionGroup;
    while (res->GetRow(row)) {
        //SELECT shipClass, groupID, factionID FROM roidRatClassGroup
        factionGroup.shipClass = row.GetInt(0);
        factionGroup.groupID = row.GetInt(1);
        m_groups.emplace(row.GetInt(2), factionGroup);

        m_db.GetGroupTypeIDs(row.GetInt(1), *res2);
        while (res2->GetRow(row2)) {
            //SELECT typeID FROM invTypes WHERE groupID = %u
            m_types.emplace(row.GetInt(1), row2.GetInt(0));
        }
    }

    res->Reset();
    m_db.GetSpawnClasses(*res);
    RatSpawnClass spawnClass;
    while (res->GetRow(row)) {
        //SELECT type, sub, f, d, c, bc, bs, h, o, cf, cd, cc, cbc, cbs FROM roidRatSpawnClass
        spawnClass.type = row.GetInt(0);
        spawnClass.sub = row.GetInt(1);
        spawnClass.f = row.GetInt(2);
        spawnClass.d = row.GetInt(3);
        spawnClass.c = row.GetInt(4);
        spawnClass.bc = row.GetInt(5);
        spawnClass.bs = row.GetInt(6);
        spawnClass.h = row.GetInt(7);
        spawnClass.o = row.GetInt(8);
        spawnClass.cf = row.GetInt(9);
        spawnClass.cd = row.GetInt(10);
        spawnClass.cc = row.GetInt(11);
        spawnClass.cbc = row.GetInt(12);
        spawnClass.cbs = row.GetInt(13);
        m_classes.emplace(row.GetInt(0), spawnClass);
    }

    //cleanup
    SafeDelete(res);
    SafeDelete(res2);

    sLog.Log("     SpawnDataMgr", "%u asteroid rat groups in %u buckets, %u rat spawn classes in %u buckets, and %u rat spawn types for %u regions loaded in %.3fms.",
             m_groups.size(), m_groups.bucket_count(), m_classes.size(), m_classes.bucket_count(), m_types.size(), m_regions.size(), (GetTimeUSeconds() - start));
}

/*
 _ log(
 SPAWN__ERROR
 SPAWN__WARNING
 SPAWN__MESSAGE
 SPAWN__POP
 SPAWN__DEPOP
 */
/** @todo  adjust this class to manage anomaly spawns, too */
SpawnMgr::SpawnMgr(SystemManager* mgr, PyServiceMgr& svc)
: m_system(mgr),
  m_services(svc),
  m_mainTimer(60000),  // 60s ... just putting something here
  m_groupTimer(60000)
{
    m_mainTimer.Disable();
    m_groupTimer.Disable();
}

void SpawnMgr::Process() {
    double profileStartTime = 0.0;
    if (sConfig.server.UseProfiling)
        profileStartTime = GetTimeUSeconds();
    // called by SystemManager::Process() for each system.  this will need to be fast.
    //  check timers and call approprate functions as needed.

    // this will be initial spawn timer for system.
    //  while this is active, NO spawns will be made.
    if (m_mainTimer.Enabled())  {
        if (m_mainTimer.Check()) {
            m_mainTimer.Disable();
            m_enabled = true;
            _log(SPAWN__MESSAGE, "SpawnMgr::Process() - Main Timer called.  Spawn functions enabled for %s(%u).",
                 m_system->GetName().c_str(), m_system->GetID());
        }
        return;
    }

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
     * UPDATE:  this works as planned/intended...  26Aug15
            22:32:26 [SpawnPop] SpawnMgr::ReSpawn - Spawning NPC 140001690
            22:32:26 [Trace] NPC Strain Hunter Drone: Added to system manager for Ammold(30002547)
            22:33:33 [Message] SpawnMgr::Process() - Spawn Groups full (or no spawns) for Ammold(30002547).  Group Timer disabled.
	 */
    if (m_groupTimer.Enabled()) {
        if (m_groupTimer.Check()) {
            bool killTimer = true;
            RatBubbleVec::iterator curBubbleItr = m_bubbles.begin();
            while (curBubbleItr != m_bubbles.end()) {
                auto curSpawnItr = m_spawns.equal_range((*curBubbleItr)->GetID());
                for (auto it = curSpawnItr.first; it != curSpawnItr.second; it++) {
                    if (it->second->enabled) {
                        // this means check SpawnEntry for 'missing' SpawnGroup members and respawn as needed.
                        ReSpawn((*curBubbleItr), it->second);
                        killTimer = false;
                    }
                }
                ++curBubbleItr;
            }

            if (killTimer) {
                m_groupTimer.Disable();
                _log(SPAWN__MESSAGE, "SpawnMgr::Process() - Spawn Groups full (or no spawns) for %s(%u).  Group Timer disabled.",
                     m_system->GetName().c_str(), m_system->GetID());
            }
        }
    }

    if (sConfig.server.UseProfiling)
        sProfile.AddTime(_spawnProfile, GetTimeUSeconds() - profileStartTime);
}

void SpawnMgr::MoveSpawn()
{
    /* this will be to move a spawn from one location to another (change bubbles) */
}

void SpawnMgr::StartMainTimer()
{
    m_mainTimer.Start(sConfig.npc.RoamingTimer *60 *1000);
    _log(SPAWN__MESSAGE, "SpawnMgr::StartMainTimer() - Main Spawn Timer started for %s(%u) at %u mins.",
         m_system->GetName().c_str(), m_system->GetID(), sConfig.npc.RoamingTimer);
}

void SpawnMgr::SpawnDepopped(SystemBubble* pSysBubble, uint32 itemID)
{   // NOTE this DOES NOT remove entity from system or bubble.  user must do this BEFORE calling.
	if (!pSysBubble) return;	//hack for null sys bubble.
    _log(SPAWN__DEPOP, "SpawnMgr::SpawnDepoped - NPC %u removed from system.  DePop requested", itemID);
    // delete this spawn item from SpawnEntry in this bubble.
    RemoveSpawn(pSysBubble->GetID(), itemID);
    // if any SpawnEntry still exists for this bubble, reset group timer.
    // this enables chain ratting or creating a new spawn
    SpawnEntryDef::iterator itr = m_spawns.find(pSysBubble->GetID());
    if (itr != m_spawns.end()) {
        if (!m_groupTimer.Enabled())
            m_groupTimer.Start(itr->second->time);
        itr->second->enabled = true;
    } else {
        //there is no SpawnEntry for this bubble (no spawns left here).  delete from the spawned list and reset bubble checks.
        m_bubbles.erase(std::find(m_bubbles.begin(), m_bubbles.end(), pSysBubble));
        pSysBubble->ResetBubbleSpawn();
    }
}

void SpawnMgr::SpawnPopped(uint32 itemID)
{
    _log(SPAWN__POP, "SpawnMgr::SpawnPopped - Pop called for NPC %u", itemID);

}

void SpawnMgr::DoSpawnForBubble(SystemBubble* pSysBubble, uint32 regionID, double secRating)
{
    if (!m_enabled) return;
    double profileStartTime = 0.0;
    if (sConfig.server.UseProfiling)
        profileStartTime = GetTimeUSeconds();
    if (!_FindSpawnForBubble(pSysBubble->GetID())) {
        sLog.Success("SpawnMgr", "DoSpawnForBubble called for bubble %u in %s(%u)(%.4f). Main Timer enabled.",
                     pSysBubble->GetID(), m_system->GetName().c_str(), m_system->GetID(), secRating);
        PrepSpawn(pSysBubble, regionID, secRating);

        pSysBubble->SetSpawned(true);  // bubble flag to avoid multiple spawns in same bubble.
    }

    /* this will throw off the accuracy of the profile, as this and Process() use the same data container */
    if (sConfig.server.UseProfiling)
        sProfile.AddTime(_spawnProfile, GetTimeUSeconds() - profileStartTime);
}

bool SpawnMgr::_FindSpawnForBubble(uint32 bubbleID) {
    SpawnEntryDef::iterator itr = m_spawns.find(bubbleID);
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
void SpawnMgr::PrepSpawn(SystemBubble* pSysBubble, uint32 regionID, double secRating)
{
    // get faction for this region
    uint32 factionID = factionRogueDrones; // default to rogue drones.  this is my internal rogue drone factionID.
    if (MakeRandomFloat() > 0.15) { // random chance for ANY beltspawn to be rogue drone...if chance < 0.15, rat = drone.
        std::map<uint32, uint32>::iterator itr = sSpawnDataMgr.m_regions.find(regionID);
        if (itr != sSpawnDataMgr.m_regions.end())
            factionID = (*itr).second;
    }

    _log(SPAWN__MESSAGE, "SpawnMgr::PrepSpawn() - factionID is %u for region %u.", factionID, regionID);

    // get faction's ship typeclass and groupID map
    auto groupRange = sSpawnDataMgr.m_groups.equal_range(factionID);
    for (auto it = groupRange.first; it != groupRange.second; it++) {
        m_factionGroups.insert(std::pair<uint8, uint32>(it->second.shipClass, it->second.groupID));
    }
    _log(SPAWN__MESSAGE, "SpawnMgr::PrepSpawn() - m_factionGroups size is %u.", m_factionGroups.size());    //should be 12

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
        //NOTE  random checks here are for TESTING only....all rates are high.
        double rand = MakeRandomFloat();
        if (rand < 0.1)  //check for officer spawn
            if (factionID == factionRogueDrones)   //but not for drones.  they dont have officers..make this the rare drone hauler spawn
                type = 0; //8
            else
                type = 10;
        else if (rand < 0.15) //check for commander spawn
            type = 9;
        else if (rand < 0.2) //check for hauler spawn   TODO this needs work.  haulers are subclassed by size in db under same groupID.
            if (factionID == factionRogueDrones)    // hauler spawn for drones already set above...negate this one.
                type = 0;
            else
                type = 8;
    }
    if ((!type) && pSysBubble->IsBelt()) {  // gonna be a 'regular' trusec-based spawn in a belt.
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
    RatSpawnClass spawnClass;
    auto classRange = sSpawnDataMgr.m_classes.equal_range(type);
    for (auto it = classRange.first; it != classRange.second; it++) {
        spawnClass.type = it->second.type;
        spawnClass.sub = it->second.sub;
        spawnClass.f = it->second.f;
        spawnClass.d = it->second.d;
        spawnClass.c = it->second.c;
        spawnClass.bc = it->second.bc;
        spawnClass.bs = it->second.bs;
        spawnClass.h = it->second.h;
        spawnClass.o = it->second.o;
        spawnClass.cf = it->second.cf;
        spawnClass.cd = it->second.cd;
        spawnClass.cc = it->second.cc;
        spawnClass.cbc = it->second.cbc;
        spawnClass.cbs = it->second.cbs;
        spawnEntry.push_back(spawnClass);
    }
    if (spawnEntry.size() > 0)
        _log(SPAWN__MESSAGE, "SpawnMgr::PrepSpawn() - spawnEntry size is %u.", spawnEntry.size());    //variable
    else {
        _log(SPAWN__MESSAGE, "SpawnMgr::PrepSpawn() - spawnEntry size is 0.");
        return;
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
    if (f) {
        toSpawn.typeID = GetRandTypeID(1);
        toSpawn.quantity = f;
        m_toSpawn.push_back(toSpawn);
    }
    if (d) {
        toSpawn.typeID = GetRandTypeID(2);
        toSpawn.quantity = d;
        m_toSpawn.push_back(toSpawn);
    }
    if (c) {
        toSpawn.typeID = GetRandTypeID(3);
        toSpawn.quantity = c;
        m_toSpawn.push_back(toSpawn);
    }
    if (bc) {
        toSpawn.typeID = GetRandTypeID(4);
        toSpawn.quantity = bc;
        m_toSpawn.push_back(toSpawn);
    }
    if (bs) {
        toSpawn.typeID = GetRandTypeID(5);
        toSpawn.quantity = bs;
        m_toSpawn.push_back(toSpawn);
    }
    if (h) {
        toSpawn.typeID = GetRandTypeID(6);
        toSpawn.quantity = h;
        m_toSpawn.push_back(toSpawn);
    }
    if ((o) && (factionID != factionRogueDrones)) {    //drones do NOT have officers (internal type 7).
        toSpawn.typeID = GetRandTypeID(7);
        toSpawn.quantity = o;
        m_toSpawn.push_back(toSpawn);
    }
    if (cf) {
        toSpawn.typeID = GetRandTypeID(8);
        toSpawn.quantity = cf;
        m_toSpawn.push_back(toSpawn);
    }
    if (cd) {
        toSpawn.typeID = GetRandTypeID(9);
        toSpawn.quantity = cd;
        m_toSpawn.push_back(toSpawn);
    }
    if (cc) {
        toSpawn.typeID = GetRandTypeID(10);
        toSpawn.quantity = cc;
        m_toSpawn.push_back(toSpawn);
    }
    if (cbc) {
        toSpawn.typeID = GetRandTypeID(11);
        toSpawn.quantity = cbc;
        m_toSpawn.push_back(toSpawn);
    }
    if (cbs) {
        toSpawn.typeID = GetRandTypeID(12);
        toSpawn.quantity = cbs;
        m_toSpawn.push_back(toSpawn);
    }

    if ((factionID == factionRogueDrones) && ((bc) || (bs))) {
        // spawn 4 drone swarm-class ships for each bc/bs
        toSpawn.typeID = GetRandTypeID(7);
        toSpawn.quantity = ((bs ? bs : bc) *4);
        m_toSpawn.push_back(toSpawn);
    }

    _log(SPAWN__MESSAGE, "SpawnMgr::PrepSpawn() - m_toSpawn size is %u.", m_toSpawn.size());    //variable
    MakeSpawn(pSysBubble, factionID, type, subtype);
}

uint32 SpawnMgr::GetRandTypeID(uint32 shipClass)
{
    // get rat faction's groupID based on previously selected shipClass
    uint32 groupID = 0;
    RatFactionGroupsMap::iterator itr = m_factionGroups.find(shipClass);
    if (itr != m_factionGroups.end()) {
        groupID = itr->second;
    } else {
        _log(SPAWN__ERROR, "SpawnMgr::GetRandTypeID() - Failed to find groupID for shipClass %u.", shipClass);
        return 0;
    }
   //get typeIDs for this groupID from m_types and return only one for spawning
   /** @todo  will need to check typeIDs here for higher-level ships in hi-sec systems */
    std::vector<uint32> typeVec;
    auto typeRange = sSpawnDataMgr.m_types.equal_range(groupID); //groupID is key, typeID is value
    for (auto it = typeRange.first; it != typeRange.second; it++) {
        typeVec.push_back(it->second);
    }

    return typeVec.at(MakeRandomInt(0, typeVec.size()));
}

void SpawnMgr::ReSpawn(SystemBubble* pSysBubble, SpawnEntry* spawnEntry)
{
    GPoint startPos(pSysBubble->GetCenter());
    startPos.MakeRandomPointOnSphere(8000); // put them at random spot 8k off center
    //startPos.MakeRandomPointOnSphere(500000); //500km from bubble center
    //const GPoint warpToPoint = (pSysBubble->GetCenter() - (MakeRandomInt(-5, 15) *1000));
    ItemData idata(
        spawnEntry->typeID,
        spawnEntry->corpID,
        m_system->GetID(),
        flagAutoFit,
        spawnEntry->factionID,  // set ownerID to factionID for rats
        "BeltRat"
    );

    InventoryItemRef i = m_services.item_factory->SpawnItem(idata);      // will have to work on this to NOT save npc to db.
    if (!i) {
        _log(SPAWN__ERROR, "Failed to spawn item type %u.", spawnEntry->typeID);
        return;
    }

    _log(SPAWN__POP, "SpawnMgr::ReSpawn - Spawning NPC %u", i->itemID());

    i->Relocate(startPos);
    NPC* npc = new NPC(i, m_services, m_system, spawnEntry->corpID, spawnEntry->factionID, this);

    //drop this npc into system, and begin warp.  this may have to be looked into later for timing of large spawns (>6)
    m_system->AddNPC(npc);
    //npc->DestinyMgr()->WarpTo(warpToPoint, (MakeRandomInt(0, 5) *1000)); //simulate a formation, until i actually write them.

    spawnEntry->enabled = false;
}

void SpawnMgr::MakeSpawn(SystemBubble* pSysBubble, uint32 factionID, uint8 type, uint8 subtype)
{
    SpawnEntry se;
    NPC* npc;

    /*  the point here is to have all belt rats spawn outside their belt's bubble.
     * to make it 'realistic', they will need the appearance of warping in from some random point,
     *  to somewhere around bubble center.  this will make their origin appear elsewhere,
     * but not from same place every time.  they're pirates, they got other shit to do, too.
     *
     * however, warping in dont seem to be working.  will look into later.
     */
    GPoint startPos(pSysBubble->GetCenter());
    //startPos.MakeRandomPointOnSphere(500000); //500km from bubble center
    //const GPoint warpToPoint = (pSysBubble->GetCenter() - (MakeRandomInt(-5, 15) *1000));

    uint32 corpID = GetCorpID(factionID);

    RatSpawnGroupVec::iterator cur = m_toSpawn.begin();

    while (cur != m_toSpawn.end()) {
        ItemData idata(
            cur->typeID,
            corpID,
            m_system->GetID(),
            flagAutoFit,
            factionID,  // set ownerID to factionID for rats
            "BeltRat"
        );

        for (uint8 x=0; x!=cur->quantity; x++) {
            InventoryItemRef i = m_services.item_factory->SpawnItem(idata);      // will have to work on this to NOT save npc to db....or save ALL the spawn shit
            if (!i) {
                _log(SPAWN__ERROR, "Failed to spawn item type %u.", cur->typeID);
                continue;
            }

            _log(SPAWN__POP, "SpawnMgr::MakeSpawn - Spawning NPC %u", i->itemID());

            startPos.MakeRandomPointOnSphere(8000); // put them at random spot 8k off center
            i->Relocate(startPos);
            npc = new NPC(i, m_services, m_system, corpID, factionID, this);

            // NPC::Load() no longer does anything.  it is still here in case we find a new use for it.
            if (!npc->Load(m_services.serviceDB())) {
                _log(SPAWN__ERROR, "Failed to load NPC data for NPC %u with type %u, depoping.", npc->GetID(), npc->GetSelf()->typeID());
                SafeDelete(npc);
            }
            //drop this npc into system, and begin warp.  this may have to be looked into later for timing of large spawns (>6)
            npc->DestinyMgr()->SetPosition(startPos);
            m_system->AddNPC(npc);
            //npc->DestinyMgr()->WarpTo(warpToPoint, (MakeRandomInt(0, 5) *1000)); //simulate a formation, until i actually write them.

            se.enabled = 0;
            se.groupID = i->type().groupID();
            se.itemID = i->itemID();
            se.total = cur->quantity;
            se.number = x;
            se.typeID = cur->typeID;
            se.spawnID = m_spawnID;
            se.corpID = corpID;
            se.factionID = factionID;
            se.type = type;
            se.sub = subtype;

            m_spawns.insert(std::pair<uint32, SpawnEntry*>(pSysBubble->GetID(), &se));
        }
        ++cur;
    }

    ++m_spawnID;
    m_bubbles.push_back(pSysBubble);
    //cleanup
    m_ratSpawns.clear();
}

void SpawnMgr::RemoveSpawn(uint32 bubbleID, uint32 itemID)
{	/** @todo:  this isnt right... */
    auto itr = m_spawns.equal_range(bubbleID);
    for (auto cur = itr.first; cur != itr.second; cur++)
        if ((*cur).second)
            if ((*cur).second->itemID == itemID)
                m_spawns.erase(cur);

    return;
}

uint32 SpawnMgr::GetCorpID(uint32 factionID)
{
    // UGLY HACK for rat corp id from faction.
    // FIXME this will need more work later.  NONE of the npc types have corp/faction info.
    switch (factionID) {
        case factionAngel:          return corpAngel;
        case factionSanshas:        return corpSanshas;
        case factionBloodRaider:    return corpBloodRaider;
        case factionGuristas:       return corpGuristas;
        case factionSerpentis:      return corpSerpentis;
        case factionRogueDrones:    return corpRogueDrones;
    }
}

