
 /**
  * @name WormholeMgr.cpp
  *     WH Spawn managment system for Alasiya EvEmu
  *
  * @Author:        Allan
  * @date:          12 December 2015
  *
  */


#include "eve-server.h"

#include "EVEServerConfig.h"
#include "PyServiceMgr.h"
#include "StaticDataMgr.h"
#include "inventory/InventoryItem.h"
#include "system/cosmicMgrs/WormholeMgr.h"
#include "system/SystemManager.h"

/*  this class will need to keep track of all WH in universe, what systems they connect to, and how long they last.
 *
 *  it will need access to the system manager, a list of systems, and a way to boot a new system when needed.
 *
 *  this class will also need access to the wspace system manager (which may be same sysmgr)
 * with a way to access/track/boot as needed.
 *
 *  when one WH collapses, it will be in charge of creating new WH in the place of the old
 * one, making approprate connections, and tracking any other changes in the WH itself
 *
 *  this class is also in charge of all dynamic WH data in the db
 *
 */

WormholeMgr::WormholeMgr()
:  m_updateTimer(0),    // arbitrary 2m default
    m_services(nullptr)
{
    m_initalized = false;
}

void WormholeMgr::Initialize(PyServiceMgr* svc) {
    m_services = svc;

    m_updateTimer.Start(120000);

    m_initalized = true;
    sLog.Blue(" Wormhole Manager", "Wormhole Manager Initialized.");

    /* load current data, start timers, process current data, and create new items, if needed */
}

void WormholeMgr::Process() {
    if (!m_initalized)
        return;
    if (m_updateTimer.Check(false)) {
        /* do something useful here */
    }
}

void WormholeMgr::Create(CosmicSignature& sig)
{
    sig.sigName = "WormHole K162 (deco only)";
    // create and spawn and save actual anomaly item  // typeID, ownerID, locationID, flag, name, &_position
    GPoint pos(sig.x, sig.y, sig.z);
    pos += 10000;   // add 10km
    ItemData iData(30831, sig.ownerID, sig.systemID, flagAutoFit, sig.sigName.c_str(), pos);

    /** @todo update this to use temp items */
    InventoryItemRef iRef = sItemFactory.SpawnItem(iData);  /* not sure how well generic spawn will work here. */
    if (iRef.get() == nullptr) // we'll survive...
        return;
    sig.sigItemID = iRef->itemID();
    // do this or create/add generic se here?
    DBSystemDynamicEntity entity;
        entity.categoryID = iRef->categoryID();
        entity.groupID = iRef->groupID();
        entity.itemID = iRef->itemID();
        entity.itemName = sig.sigName;
        entity.typeID = sig.sigTypeID;
        entity.x = pos.x;
        entity.y = pos.y;
        entity.z = pos.z;
        /** @todo  fix these... */
        entity.ownerID = sig.ownerID;
        entity.allianceID = 0;  /** @todo  may have to write a method to check and set this */
        entity.corporationID = sDataMgr.GetCorpID(entity.ownerID);
        // do the spawn using SystemManager's BuildEntity:
    /** @todo this is more shit that should NOT be in db */
    sEntityList.FindOrBootSystem(sig.systemID)->BuildDynamicEntity(entity);

    _log(COSMIC_MGR__MESSAGE, "WormholeMgr::Create() - Creating WormHole type %u in system %u", sig.dungeonType, sig.systemID);
}

/*
    AttrWormholeTargetSystemClass = 1381,
    AttrWormholeMaxStableTime = 1382,
    AttrWormholeMaxStableMass = 1383,
    AttrWormholeMassRegeneration = 1384,
    AttrWormholeMaxJumpMass = 1385,
    AttrWormholeTargetRegion1 = 1386,
    AttrWormholeTargetRegion2 = 1387,
    AttrWormholeTargetRegion3 = 1388,
    AttrWormholeTargetRegion4 = 1389,
    AttrWormholeTargetRegion5 = 1390,
    AttrWormholeTargetRegion6 = 1391,
    AttrWormholeTargetRegion7 = 1392,
    AttrWormholeTargetRegion8 = 1393,
    AttrWormholeTargetRegion9 = 1394,
    AttrWormholeTargetConstellation1 = 1395,
    AttrWormholeTargetConstellation2 = 1396,
    AttrWormholeTargetConstellation3 = 1397,
    AttrWormholeTargetConstellation4 = 1398,
    AttrWormholeTargetConstellation5 = 1399,
    AttrWormholeTargetConstellation6 = 1400,
    AttrWormholeTargetConstellation7 = 1401,
    AttrWormholeTargetConstellation8 = 1402,
    AttrWormholeTargetConstellation9 = 1403,
    AttrWormholeTargetSystem1 = 1404,
    AttrWormholeTargetSystem2 = 1405,
    AttrWormholeTargetSystem3 = 1406,
    AttrWormholeTargetSystem4 = 1407,
    AttrWormholeTargetSystem5 = 1408,
    AttrWormholeTargetSystem6 = 1409,
    AttrWormholeTargetSystem7 = 1410,
    AttrWormholeTargetSystem8 = 1411,
    AttrWormholeTargetSystem9 = 1412,
    */