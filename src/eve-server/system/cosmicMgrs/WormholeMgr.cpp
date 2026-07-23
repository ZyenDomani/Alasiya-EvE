
 /**
  * @name WormholeMgr.cpp
  *     WH management system for Alasiya EvEmu
  *
  * @Author:        Allan
  * @date:          12 December 2015
  *
  * Updates:        James
  * Date:          13 November 2021
  */


#include "eve-server.h"

#include "EVEServerConfig.h"
#include "StaticDataMgr.h"
#include "inventory/InventoryItem.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/AnomalyMgr.h"
#include "system/cosmicMgrs/WormholeMgr.h"
#include "map/MapData.h"


/*  this class will need to keep track of all WH in universe, what systems they connect to, and how long they last.
 *
 *  it will need access to the system manager, a list of systems, and a way to boot a new system when needed.
 *
 *  this class will also need access to the wspace system manager (which may be same sysmgr)
 * with a way to access/track/boot as needed.
 *
 *  when one WH collapses, it will be in charge of creating new WH in the place of the old
 * one, making appropriate connections, and tracking any other changes in the WH itself
 *
 *  this class is also in charge of all dynamic WH data in the db
 *
 */

WormholeMgr::WormholeMgr()
: m_updateTimer(0),
m_initalized(false)
{
}

WormholeMgr::~WormholeMgr()
{
    /* nothing to do here */
}

void WormholeMgr::Initialize() {
    m_updateTimer.Start(5 * EvE::Timer::Minute);    // arbitrary 5m default

    m_initalized = true;

    /* load current data, start timers, process current data, and create new items, if needed */

    sLog.Blue(" Wormhole Manager", "Wormhole Manager Initialized.");
}

// this is called on a minute timer from EntityMgr
void WormholeMgr::Process() {
    if (!m_initalized)
        return;
    if (m_updateTimer.Check(false)) {
        /* do something useful here */
    }
}

void WormholeMgr::Create(CosmicSignature& sig, uint32 exitSystemID/*0*/, uint32 exitSourceItemID/*0*/)
{
    // this really isnt needed.  may need later
    if (sig.dungeonType != Dungeon::Type::Wormhole)
        return;

    /*
     * Band        1/5     1/10    1/15    1/20    1/25    1/40    1/45    1/60    1/80
     * Percentage  20.0%   10.0%   6.67%   5.0%    4.0%    2.5%    2.22%   1.67%   1.25%
     *    Base sigStrength (modified for Alasiya)
     * k-space
     * 1/10 = exit(k162), hi>hi(a641), lo>c3(x702), lo>lo(x944)
     * 1/15 = hi>c1(z971)
     * 1/20 = hi>c2(r943),lo>c1(r943)
     * 1/25 = hi>c3(x702),hi>lo(r051),null>c3(x702),null>lo(n944)
     * 1/40 = hi>c4(o128),lo>c4(o128),lo>hi(b449),hi>null(v283),null>c1(z971),null>hi(b449)
     * 1/45 = null>c2(r943),null>c4(o128)
     * 1/60 = hi>c5(m555),lo>c5(n432),lo>null(s199),null>c5(n432)
     * 1/80 = hi>c6(b041)
     *
     * w-space
     * 1/10 = c1*hi(n110),c1>c1(h121),c2*hi(b274),c2>c1(z647),c3*lo(u210),c3>c3(n968),c4*c3(c247),c5*c5(h296),c5>null(z142),c6*c5(v911),c6>null(z142)
     * 1/15 = c1>c2(c125),c2>c2(d382),c3>c43(t405),c4*c4(x877),c5*c6(v753),c6*c6(w237)
     * 1/20 = c1*lo(j244),c1>c3(o883),c2*lo(a239),c2>c3(o477),c3*hi(d845),c3>c1(v301),c4*c1(p060),c5>lo(c140),c6*c3(l477),c6>lo(c140)
     * 1/25 = c1>c4(m609),c2>c4(y683),c3>c2(i182),c4*c2(n766),c6*c4(z457)
     * 1/40 = c1*null(z060),c1>c5(l614),c2*null(e545),c2>c5(n062),c3*null(k346),c3>c5(n770),c4*c5(h900),c5*c1(y790),c5>hi(d792),c6*c1(q317),c6>hi(d792)
     * 1/45 = c2>c6(r474),c3>c6(a982)
     * 1/60 = c5*c3(m267),c5*c4(e175),
     * 1/80 = c1>c6(s804),c4*c6(u574),c5*c2(d364),c6*c2(g024)
     *
     * note: * instead of > means static
     * wh to class 1,2,3,4 have 16h lifetime
     * others have 24h lifetime
     */
    sig.sigStrength = 0.1;

    //Destination for non-exit wormholes
    uint32 destSystem = 0;
    CelestialObjectRef iRef;

    // For exit wormholes (k162)
    if (exitSystemID != 0) {
        Vector3d pos(sig.position);
        ItemData wData(Item::Wormhole::K162, sig.ownerID, sig.systemID, flagAutoFit, sig.sigName.c_str(), pos);
        iRef = sItemFactory.SpawnWormhole(wData);
        if (iRef.get() == nullptr)
            return;

        // Set the destination system attribute for the wormhole
        // In this case, exitSystemID is the origin wormhole
        iRef->SetAttribute(AttrWormholeTargetSystem1, exitSystemID);
    // For all other kinds of wormholes
    } else {
        // decide which type of wormhole to create here
        const ItemType* whType = GetRandomWormholeType(sig.systemID);
        if (whType == nullptr)
            _log(COSMIC_MGR__WARNING, "WormholeMgr::Create() - Create Failure, SystemID not in Database %u", sig.systemID);
            return;
        destSystem = GetRandomDestination(whType);
        // create wormhole here
        sig.sigName = whType->name();
        sig.sigTypeID = whType->id();

        Vector3d pos(sig.position);
        ItemData wData(whType->id(), sig.ownerID, sig.systemID, flagAutoFit, sig.sigName.c_str(), pos);

        iRef = sItemFactory.SpawnWormhole(wData);
        if (iRef.get() == nullptr)
            return;

        // Set the destination system attribute for the wormhole
        iRef->SetAttribute(AttrWormholeTargetSystem1, destSystem);
    }

    // verify system is loaded
    SystemManager* pSysMgr = sEntityMgr.FindOrBootSystem(sig.systemID);
    if (pSysMgr == nullptr) {
        _log(COSMIC_MGR__WARNING, "WormholeMgr::Create() - Boot failure for system %u", sig.systemID);
        return;
    }

    // set itemID to return to anomaly mgr after creation succeeds
    sig.sigItemID = iRef->itemID();
    if (exitSystemID != 0) {
        // Set exit wormhole's attributes so it can find its' entrance
        iRef->SetAttribute(AttrWormholeTargetSystem1, exitSourceItemID);
        // Set source wormhole's attributes so it can find its' exit
        CelestialObjectRef sRef;
        sRef = sItemFactory.GetCelestialRef(exitSourceItemID);
        sRef->SetAttribute(AttrWormholeTargetSystem2, sig.sigItemID);
        sRef->SaveItem();
        // Set exit wormhole's attributes based upon source wormhole's attributes
        iRef->SetAttribute(AttrWormholeMassRegeneration, sRef->GetAttribute(AttrWormholeMassRegeneration).get_int());
        iRef->SetAttribute(AttrWormholeTargetSystemClass, sDataMgr.GetWHSystemClass(exitSystemID));
        iRef->SetAttribute(AttrWormholeMaxStableTime, sRef->GetAttribute(AttrWormholeMaxStableTime).get_int());
        iRef->SetAttribute(AttrWormholeMaxStableMass, sRef->GetAttribute(AttrWormholeMaxStableMass).get_int());
        iRef->SetAttribute(AttrWormholeMaxJumpMass, sRef->GetAttribute(AttrWormholeMaxJumpMass).get_int());
    }

    iRef->SaveItem();

    DBSystemDynamicEntity entity = DBSystemDynamicEntity();
    entity.ownerID = ownerSystem;
    entity.factionID = 0;
    entity.allianceID = 0;
    entity.corporationID = 0;
    entity.itemID = iRef->itemID();
    entity.itemName = iRef->itemName();
    entity.typeID = iRef->typeID();
    entity.groupID = iRef->groupID();
    entity.categoryID = iRef->categoryID();
    entity.position = iRef->position();
    entity.planetID = 0;
    SystemEntity* pSE = DynamicEntityFactory::BuildEntity(*pSysMgr, entity);

    sig.bubbleID = pSE->SysBubble()->GetID();
    // add wormhole to vector
    m_wormholes.push_back(iRef->itemID());

    // Call CreateExit() to create an exit wormhole (only if Create() was not called for an exit already)
    if (exitSystemID == 0) {
        SystemManager* pToSys = sEntityMgr.FindOrBootSystem(destSystem);
        if (pSysMgr == nullptr) {
            _log(COSMIC_MGR__WARNING, "WormholeMgr::Create() - Boot failure for system %u", destSystem);
            return;
        }
        CreateExit(pSysMgr, pToSys, sig.sigItemID);
    }

    _log(COSMIC_MGR__WARNING, "WormholeMgr::Create() - Created %s in %s(%u) with %.3f%% sigStrength.", \
            iRef->name(), pSysMgr->GetName(), sig.systemID, sig.sigStrength *100);

}

// Create exit for loaded systems
void WormholeMgr::CreateExit(SystemManager* pFromSys, SystemManager* pToSys, uint32 sourceItemID)
{
    // compile data for exit
    CosmicSignature sig = CosmicSignature();

    sig.sigID = sEntityMgr.GetAnomalyID();
    sig.systemID = pToSys->GetID();
    sig.dungeonType = Dungeon::Type::Wormhole;

    sig.sigItemID = 0;
    sig.sigName = "Wormhole K162 ";
    //default to 1/80
    sig.sigStrength = 0.0125;
    sig.sigTypeID = Item::Wormhole::K162;
    sig.sigGroupID = EVEDB::invGroups::Wormhole;
    sig.scanGroupID = Scanning::Group::Signature;
    sig.scanAttributeID = AttrScanAllStrength;
    sig.ownerID = ownerSystem;

    sig.position = sMapData.GetAnomalyPoint(pToSys);

    // send data to Create() for processing
    Create(sig, pFromSys->GetID());

    // Register this exit wormhole with the destination's AnomalyMgr...let AnomalyMgr do this
   // pToSys->GetAnomMgr()->RegisterExitWH(sig);

    _log(COSMIC_MGR__WARNING, "WormholeMgr::CreateExit() - Creating Exit(loaded) from %s(%u) to %s(%u)", \
                pFromSys->GetName(), pFromSys->GetID(), pToSys->GetName(), pToSys->GetID());
}

// Create exit for unloaded systems
void WormholeMgr::CreateExit(SystemManager* pFromSys, uint32 exitSystemID, uint32 sourceItemID)
{
    // compile data for exit
    CosmicSignature sig = CosmicSignature();

    sig.sigID = sEntityMgr.GetAnomalyID();
    sig.systemID = exitSystemID;
    sig.dungeonType = Dungeon::Type::Wormhole;

    sig.sigName = "Wormhole K162 ";
    //default to 1/80
    sig.sigStrength = 0.0125;
    sig.sigTypeID = Item::Wormhole::K162;
    sig.sigGroupID = EVEDB::invGroups::Wormhole;
    sig.scanGroupID = Scanning::Group::Signature;
    sig.scanAttributeID = AttrScanAllStrength;
    sig.ownerID = 1;
    sig.position = sMapData.GetAnomalyPoint(exitSystemID);

    CelestialObjectRef iRef;
    CelestialObjectRef sRef;

    Vector3d pos(sig.position);
    ItemData wData(Item::Wormhole::K162, sig.ownerID, sig.systemID, flagAutoFit, sig.sigName.c_str(), pos);
    iRef = sItemFactory.SpawnWormhole(wData);
    if (iRef.get() == nullptr)
        return;
    iRef->SetAttribute(AttrWormholeTargetSystem1, pFromSys->GetID());

    // Set exit wormhole's attributes so it can find its' entrance
    iRef->SetAttribute(AttrWormholeTargetSystem2, sourceItemID);
    // Set exit wormhole's attributes based upon source wormhole's attributes
    sRef = sItemFactory.GetCelestialRef( sourceItemID );
    iRef->SetAttribute(AttrWormholeMassRegeneration, sRef->GetAttribute(AttrWormholeMassRegeneration).get_int());
    iRef->SetAttribute(AttrWormholeTargetSystemClass, sDataMgr.GetWHSystemClass(exitSystemID));
    iRef->SetAttribute(AttrWormholeMaxStableTime, sRef->GetAttribute(AttrWormholeMaxStableTime).get_int());
    iRef->SetAttribute(AttrWormholeMaxStableMass, sRef->GetAttribute(AttrWormholeMaxStableMass).get_int());
    iRef->SetAttribute(AttrWormholeMaxJumpMass, sRef->GetAttribute(AttrWormholeMaxJumpMass).get_int());
    sig.sigItemID = iRef->itemID();
    iRef->SaveItem();

    // Set source wormhole's attributes so it can find its' exit
    sRef->SetAttribute(AttrWormholeTargetSystem2, sig.sigItemID);
    sRef->SaveItem();

    // Save the exit wormhole signature to the database
    m_mdb->SaveAnomaly(sig);

    _log(COSMIC_MGR__WARNING, "WormholeMgr::CreateExit() - Creating Exit(unloaded) from %s(%u) to system(%u)", \
                pFromSys->GetName(), pFromSys->GetID(), exitSystemID);
}

// Pick a random type of wormhole to create based upon the class of the system in question
const ItemType* WormholeMgr::GetRandomWormholeType(uint32 systemID) {
    std::vector<uint32> destTypes = sDataMgr.GetWHDestinationTypes(sDataMgr.GetWHSystemClass(systemID));
    if(destTypes.size()<1) {
        return nullptr;
    }
    uint32 typeID = destTypes[MakeRandomInt(0,destTypes.size()-1)];
    return sItemFactory.GetType(typeID);
}

// Pick a random destination of wormhole based upon its typeID
uint32 WormholeMgr::GetRandomDestination(const ItemType* whType) {
    uint8 targetClass = whType->GetAttribute(AttrWormholeTargetSystemClass).get_uint32();
    std::vector<uint32> destSystems = sDataMgr.GetWHClassSystems(targetClass);
    return destSystems[MakeRandomInt(0,destSystems.size()-1)];
}


/* attributeID  attributeName       attCat attIdx    description     categoryID
1381    wormholeTargetSystemClass       4   0   Target System Class for wormholes   7   **this is list of w-space ids
1382    wormholeMaxStableTime           5   0   The maximum amount of time a wormhole will stay open    7
1383    wormholeMaxStableMass           5   0   The maximum amount of mass a wormhole can transit before collapsing     7
1384    wormholeMassRegeneration        5   0   The amount of mass a wormhole regenerates per cycle     7
1385    wormholeMaxJumpMass             5   0   The maximum amount of mass that can transit this wormhole in one go    7
*/

/*
30579   Wormhole Z971   1381    wormholeTargetSystemClass       1
30579   Wormhole Z971   1382    wormholeMaxStableTime           57600000
30579   Wormhole Z971   1383    wormholeMaxStableMass           100000
30579   Wormhole Z971   1384    wormholeMassRegeneration        0
30579   Wormhole Z971   1385    wormholeMaxJumpMass             62000
*/

/** @todo  our db is missing data for these.  search newer db files for updated data  ...none */

/* attributeID  attributeName   attributeCategory   attributeIdx    description     categoryID
1381    wormholeTargetSystemClass   4   0   Target System Class for wormholes   7
1382    wormholeMaxStableTime   5   0   The maximum amount of time a wormhole will stay open    7
1383    wormholeMaxStableMass   5   0   The maximum amount of mass a wormhole can transit before collapsing     7
1384    wormholeMassRegeneration    5   0   The amount of mass a wormhole regenerates per cycle     7
1385    wormholeMaxJumpMass     5   0   The maximum amount of mass that can transit a wormhole in one go    7
// these dont have any data...
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

/* WH groupID: 988
 *      70 items in db
 *
 * graphicIDs - 3715 (lt blue, red center)
 *              2017  10817     312     Brown quarter
 *              2013  10813     312     Brown hemisphere
 *              2010  10810     312     Blue faint
 *              2009  10809     312     Thick White  (dark, single point center with light to rbottom)
 *              2008  10795     15      Jovian Construct
 */
