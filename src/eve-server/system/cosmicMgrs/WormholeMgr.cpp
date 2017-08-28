
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
#include "inventory/InventoryItem.h"
#include "system/cosmicMgrs/WormholeMgr.h"

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
:  m_updateTimer(120000),    // arbitrary 2m default
    m_services(nullptr)
{
    m_updateTimer.Disable();
    m_initalized = false;
}

void WormholeMgr::Initialize(PyServiceMgr* svc) {
    if (!sConfig.cosmic.WormHoleEnabled) {
        sLog.Warning(" Wormhole Manager", "Wormhole Manager Disabled.");
        return;
    }

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
    GPoint pos = GPoint(sig.x, sig.y, sig.z);
    ItemData iData(30831, sig.ownerID, sig.systemID, flagAutoFit, sig.sigName.c_str(), pos);

    /** @todo update this to use temp items */
    InventoryItemRef iRef = m_services->item_factory->SpawnItem(iData);  /* not sure how well generic spawn will work here. */
    if (iRef.get() == nullptr) // we'll survive...
        return;
    sig.sigItemID = iRef->itemID();

    _log(COSMIC_MGR__MESSAGE, "WormholeMgr::Create() - Creating WormHole type %u in system %u", sig.dungeonType, sig.systemID);
}

