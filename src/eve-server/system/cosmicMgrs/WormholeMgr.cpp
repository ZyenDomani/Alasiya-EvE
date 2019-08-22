
 /**
  * @name WormholeMgr.cpp
  *     WH management system for Alasiya EvEmu
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
 * WH groupID: 988
 *      70 items in db
 */

WormholeMgr::WormholeMgr()
:  m_updateTimer(0),
    m_services(nullptr)
{
    m_initalized = false;
}

WormholeMgr::~WormholeMgr()
{
    /* nothing to do here */
}

void WormholeMgr::Initialize(PyServiceMgr* svc) {
    m_services = svc;

    m_updateTimer.Start(300000);    // arbitrary 5m default

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
    InventoryItemRef iRef = InventoryItem::SpawnItem(sItemFactory.GetNextTempID(), aData);
    if (iRef.get() == nullptr) // make error and exit
        return;
    SystemManager* pSysMgr = sEntityList.FindOrBootSystem(sig.systemID);
    if (pSysMgr == nullptr) {
        _log(COSMIC_MGR__ERROR, "WormholeMgr::Create() - Boot failure for system %u", sig.systemID);
        return;
    }
    // do this or create/add generic se here?
    DBSystemDynamicEntity entity = DBSystemDynamicEntity();
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
    pSysMgr->BuildDynamicEntity(entity);
    // set itemID to return to anomaly mgr
    sig.sigItemID = entity.itemID;
    // set sigStrenth based on wh type and location
    sig.sigStrength = 0.04;

    // create k162 here
    pos += 25000;   // move 25k for WormHole position
    ItemData wData(30831, sig.ownerID, sig.systemID, flagAutoFit, sig.sigName.c_str(), pos);
    iRef = InventoryItem::SpawnItem(sItemFactory.GetNextTempID(), wData);
    if (iRef.get() == nullptr) // we'll survive...anomaly is temp item, so not worried about deleting it here.
        return;
    sig.sigItemID = iRef->itemID();
    // update entity data for k162
    //entity = DBSystemDynamicEntity();
    entity.categoryID = iRef->categoryID();
    entity.groupID = iRef->groupID();
    entity.itemID = iRef->itemID();
    entity.itemName = "K162";
    entity.typeID = 30831;
    //entity.x = pos.x;
    //entity.y = pos.y;
    //entity.z = pos.z;
    //entity.ownerID = sig.ownerID;
    //entity.allianceID = -1;
    //entity.corporationID = sDataMgr.GetCorpID(entity.ownerID);
    // do the spawn using SystemManager's BuildEntity:
    pSysMgr->BuildDynamicEntity(entity);
    m_wormholes.push_back(entity.itemID);

    _log(COSMIC_MGR__TRACE, "WormholeMgr::Create() - Created %s in %s(%u)", iRef->itemName().c_str(), pSysMgr->GetName(), sig.systemID);
}

void WormholeMgr::CreateExit(SystemManager* pFromSys, SystemManager* pToSys)
{
    CosmicSignature sig = CosmicSignature();


    _log(COSMIC_MGR__TRACE, "WormholeMgr::CreateExit() - Creating Exit from %s(%u) to %s(%u)", \
                pFromSys->GetName(), pFromSys->GetID(), pToSys->GetName(), pToSys->GetID());
}

/** @todo  our db is missing data for these.  search newer db files for updated data  */

/* attributeID  attributeName   attributeCategory   attributeIdx    description     categoryID
1381    wormholeTargetSystemClass   4   0   Target System Class for wormholes   7
1382    wormholeMaxStableTime   5   0   The maximum amount of time a wormhole will stay open    7
1383    wormholeMaxStableMass   5   0   The maximum amount of mass a wormhole can transit before collapsing     7
1384    wormholeMassRegeneration    5   0   The amount of mass a wormhole regenerates per cycle     7
1385    wormholeMaxJumpMass     5   0   The maximum amount of mass that can transit a wormhole in one go    7
1386    wormholeTargetRegion1   4   0   Specific target region 1 for wormholes  7
1387    wormholeTargetRegion2   4   0   Specific target region 2 for wormholes  7
1388    wormholeTargetRegion3   4   0   Specific target region 3 for wormholes  7
1389    wormholeTargetRegion4   4   0   Specific target region 4 for wormholes  7
1390    wormholeTargetRegion5   4   0   Specific target region 5 for wormholes  7
1391    wormholeTargetRegion6   4   0   Specific target region 6 for wormholes  7
1392    wormholeTargetRegion7   4   0   Specific target region 7 for wormholes  7
1393    wormholeTargetRegion8   4   0   Specific target region 8 for wormholes  7
1394    wormholeTargetRegion9   4   0   Specific target region 9 for wormholes  7
1395    wormholeTargetConstellation1    5   0   Specific target constellation 1 for wormholes   7
1396    wormholeTargetConstellation2    4   0   Specific target constellation 2 for wormholes   7
1397    wormholeTargetConstellation3    4   0   Specific target constellation 3 for wormholes   7
1398    wormholeTargetConstellation4    4   0   Specific target constellation 4 for wormholes   7
1399    wormholeTargetConstellation5    4   0   Specific target constellation 5 for wormholes   7
1400    wormholeTargetConstellation6    4   0   Specific target constellation 6 for wormholes   7
1401    wormholeTargetConstellation7    4   0   Specific target constellation 7 for wormholes   7
1402    wormholeTargetConstellation8    4   0   Specific target constellation 8 for wormholes   7
1403    wormholeTargetConstellation9    4   0   Specific target constellation 9 for wormholes   7
1404    wormholeTargetSystem1   4   0   Specific target system 1 for wormholes  7
1405    wormholeTargetSystem2   4   0   Specific target system 2 for wormholes  7
1406    wormholeTargetSystem3   4   0   Specific target system 3 for wormholes  7
1407    wormholeTargetSystem4   4   0   Specific target system 4 for wormholes  7
1408    wormholeTargetSystem5   4   0   Specific target system 5 for wormholes  7
1409    wormholeTargetSystem6   4   0   Specific target system 6 for wormholes  7
1410    wormholeTargetSystem7   4   0   Specific target system 7 for wormholes  7
1411    wormholeTargetSystem8   4   0   Specific target system 8 for wormholes  7
1412    wormholeTargetSystem9   4   0   Specific target system 9 for wormholes  7
1457   wormholeTargetDistribution  4   0   This is the distribution ID of the target wormhole distribution     7
    */

/* typeID   typeName    graphicID
30463   Test wormhole   2907
30579   Wormhole Z971   3715
30583   Wormhole R943   3715
30584   Wormhole X702   3715
30642   Wormhole O128   3715
30643   Wormhole N432   3715
30644   Wormhole M555   3715
30645   Wormhole B041   3715
30646   Wormhole U319   3715
30647   Wormhole B449   3715
30648   Wormhole N944   3715
30649   Wormhole S199   3715
30657   Wormhole A641   3715
30658   Wormhole R051   3715
30659   Wormhole V283   3715
30660   Wormhole H121   3715
30661   Wormhole C125   3715
30662   Wormhole O883   3715
30663   Wormhole M609   3715
30664   Wormhole L614   3715
30665   Wormhole S804   3715
30666   Wormhole N110   3715
30667   Wormhole J244   3715
30668   Wormhole Z060   3715
30671   Wormhole Z647   3715
30672   Wormhole D382   3715
30673   Wormhole O477   3715
30674   Wormhole Y683   3715
30675   Wormhole N062   3715
30676   Wormhole R474   3715
30677   Wormhole B274   3715
30678   Wormhole A239   3715
30679   Wormhole E545   3715
30680   Wormhole V301   3715
30681   Wormhole I182   3715
30682   Wormhole N968   3715
30683   Wormhole T405   3715
30684   Wormhole N770   3715
30685   Wormhole A982   3715
30686   Wormhole S047   3715
30687   Wormhole U210   3715
30688   Wormhole K346   3715
30689   Wormhole P060   3715
30690   Wormhole N766   3715
30691   Wormhole C247   3715
30692   Wormhole X877   3715
30693   Wormhole H900   3715
30694   Wormhole U574   3715
30695   Wormhole D845   3715
30696   Wormhole N290   3715
30697   Wormhole K329   3715
30698   Wormhole Y790   3715
30699   Wormhole D364   3715
30700   Wormhole M267   3715
30701   Wormhole E175   3715
30702   Wormhole H296   3715
30703   Wormhole V753   3715
30704   Wormhole D792   3715
30705   Wormhole C140   3715
30706   Wormhole Z142   3715
30707   Wormhole Q317   3715
30708   Wormhole G024   3715
30709   Wormhole L477   3715
30710   Wormhole Z457   3715
30711   Wormhole V911   3715
30712   Wormhole W237   3715
30713   Wormhole B520   3715
30714   Wormhole C391   3715
30715   Wormhole C248   3715
30831   Wormhole K162   3715
*/