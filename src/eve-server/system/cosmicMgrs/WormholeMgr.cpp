
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
:  m_updateTimer(0),
    m_services(nullptr)
{
    m_initalized = false;
}

WormholeMgr::~WormholeMgr()
{
    // remove created wormholes
}

void WormholeMgr::Initialize(PyServiceMgr* svc) {
    m_services = svc;

    m_updateTimer.Start(120000);    // arbitrary 2m default

    m_initalized = true;

    /* load current data, start timers, process current data, and create new items, if needed */

    sLog.Blue(" Wormhole Manager", "Wormhole Manager Initialized.");
}

// this is called on a minute timer from EntityList
void WormholeMgr::Process() {
    if (!m_initalized)
        return;
    if (m_updateTimer.Check(false)) {
        /* do something useful here */
    }
}

void WormholeMgr::Create(CosmicSignature& sig)
{
    /** @note  this creates a k162 for deco only at this time.
     * it is more POC than usable
     */
    sig.sigName = "WormHole K162 (deco only)";
    GPoint pos(sig.x, sig.y, sig.z);
    // create and spawn and save actual anomaly item
    // typeID, ownerID, locationID, flag, name, &_position
    ItemData aData(sig.sigTypeID, sig.ownerID, sig.systemID, flagAutoFit, sig.sigName.c_str(), pos);
    /** @todo update this to use temp items */
    InventoryItemRef iRef = sItemFactory.SpawnItem(aData);
    if (iRef.get() == nullptr) // make error and exit
        return;
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
        entity.ownerID = sig.ownerID;
        entity.allianceID = -1;
        entity.corporationID = sDataMgr.GetCorpID(entity.ownerID);
    /** @todo this is more shit that should NOT be in db */
    sEntityList.FindOrBootSystem(sig.systemID)->BuildDynamicEntity(entity);
    // set itemID to return to anomaly mgr
    sig.sigItemID = entity.itemID;
    // set sigStrenth based on wh type and location
    sig.sigStrength = 0.04;

    // create k162 here
    pos += 25000;   // move 25k for WormHole position
    ItemData wData(30831, sig.ownerID, sig.systemID, flagAutoFit, sig.sigName.c_str(), pos);
    /** @todo update this to use temp items */
    iRef = sItemFactory.SpawnItem(wData);
    if (iRef.get() == nullptr) // we'll survive...
        return;
    sig.sigItemID = iRef->itemID();
    // update entity data for k162
    entity.categoryID = iRef->categoryID();
    entity.groupID = iRef->groupID();
    entity.itemID = iRef->itemID();
    entity.itemName = "K162";
    entity.typeID = 30831;
    entity.x = pos.x;
    entity.y = pos.y;
    entity.z = pos.z;
    entity.ownerID = sig.ownerID;
    entity.allianceID = -1;
    entity.corporationID = sDataMgr.GetCorpID(entity.ownerID);
    // do the spawn using SystemManager's BuildEntity:
    sEntityList.FindOrBootSystem(sig.systemID)->BuildDynamicEntity(entity);
    m_wormholes.push_back(entity.itemID);

    _log(COSMIC_MGR__MESSAGE, "WormholeMgr::Create() - Creating WormHole %s in system %u", iRef->itemName().c_str(), sig.systemID);
}

void WormholeMgr::CreateExit(SystemManager* pFromSys, SystemManager* pToSys)
{
    CosmicSignature sig;


    _log(COSMIC_MGR__MESSAGE, "WormholeMgr::CreateExit() - Creating Exit from %s(%u) to %s(%u)", \
                pFromSys->GetName().c_str(), pFromSys->GetID(), pToSys->GetName().c_str(), pToSys->GetID());
}

/*
graphicID    graphicFile     description     obsolete    graphicType     collidable  explosionID     directoryID     graphicName
2907    res:/dx9/Model/WorldObject/Wormhole/SpatialRift.re...   A spatial rift effect which is used in dungeons.    0   NULL    NULL    NULL    NULL
3428    res:/Model/WorldObject/Warpgate/WormholeBig.blue    Large, angry version of Eve wormhole - WormholeBig  0   NULL    NULL    NULL    NULL
3715    res:/dx9/model/WorldObject/Wormhole/Wormhole.red    Wormhole    0   NULL    NULL    NULL    NULL
11781   res:/dx9/Scene/Wormholes/wormhole_class_01.red      0   NULL    NULL    NULL    NULL
11782   res:/dx9/Scene/Wormholes/wormhole_class_02.red      0   NULL    NULL    NULL    NULL
11783   res:/dx9/Scene/Wormholes/wormhole_class_03.red      0   NULL    NULL    NULL    NULL
11784   res:/dx9/Scene/Wormholes/wormhole_class_04.red      0   NULL    NULL    NULL    NULL
11785   res:/dx9/Scene/Wormholes/wormhole_class_05.red      0   NULL    NULL    NULL    NULL
11786   res:/dx9/Scene/Wormholes/wormhole_class_06.red      0   NULL    NULL    NULL    NULL
*/

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