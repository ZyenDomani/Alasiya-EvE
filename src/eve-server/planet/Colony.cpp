/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Cometo (basic system idea)
    Updates:    Allan   (mostly complete and working - 27Dec16)
    Major Update:   Allan  process code rewrite/optimized 19Jul19
                    again 25Oct25
*/

/*
 * COLONY__ERROR
 * COLONY__WARNING
 * COLONY__MESSAGE
 * COLONY__DEBUG
 * COLONY__INFO
 * COLONY__INFO
 * COLONY__DUMP
 * COLONY__RES_DUMP
 * COLONY__GC_DUMP
 * COLONY__PKT_TRACE
 * COLONY__DB_ERROR
 * COLONY__DB_WARNING
 */

#include "../eve-server.h"

#include "PyServiceMgr.h"
#include "Client.h"
#include "inventory/ItemType.h"
#include "planet/Colony.h"
#include "planet/Planet.h"
#include "planet/PlanetMgr.h"
#include "planet/PlanetDataMgr.h"
#include "CustomsOffice.h"
 // fund xfer and journal logging methods
#include "account/AccountService.h"
// for launching shit...
#include "system/Container.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"

/** @todo:
 * items to work on...
 *  timers (launch, run, current, logistics)
 *  planet items attribs
 *  item->Move() logistics
 *  pin attributes?   noted in PlanetDataMgr.cpp:157
 */
/* The list of restricted (no PI) systems is:
    Amarr
    Arnon
    Aunia
    Auvergne
    Balginia
    Dodixie
    Fricoure
    Ichoriya
    Irjunen
    Isaziwa
    Isinokka
    Jita
    Lustrevik
    Motsu
    Oursulaert
    Rens
    Sankkasen
    Umokka
    */

/*
Base costs for each tier of products per unit.
level			Base Cost
P0                  5 ISK
P1                400 ISK
P2              7,200 ISK
P3             60,000 ISK
P4          1,200,000 ISK
*/

Colony::Colony(PyServiceMgr* mgr, Client* pClient, SystemEntity* pSE)
:m_svcMgr(mgr),
m_pSE(pSE->GetPlanetSE()),
ccPin(new PI_CCPin()),
m_client(pClient),
m_colonyTimer(0),
m_active(false),
m_loaded(false),
m_newHead(false),
m_toUpdate(false),
m_pLevel(0),
m_pg(0),	// not used
m_cpu(0),	// not used
m_colonyID(0),
m_procTime(0)			// process check.  init to zero and stores last proc time, which is lastRunTime in command center
{
	assert(m_pSE != nullptr);

    _log(COLONY__DEBUG, "Colony::Colony() c'tor called for %s(%u) by %s(%u)", pSE->GetName(), pSE->GetID(), pClient->GetName(), pClient->GetCharacterID());
}

Colony::~Colony()
{
    SafeDelete(ccPin);
}

/***********
 * NOTE:  seems that client tracks all PI production,
 *  so doing so here is just a waste of processing power.
 *  all processing is commented out until proven needed.
 */
// update....this will be needed for initial update as client only processes AFTER colony data is sent
void Colony::Init()
{
    if (m_loaded)
        return;

    // check for and load colony if the char has one on this planet
    if (m_db.LoadColony(m_client->GetCharacterID(), m_pSE->GetID(), ccPin)) {
        m_colonyID = ccPin->ccPinID;
        Load();
    }

    if (m_loaded)
        Update();
}

void Colony::Shutdown()
{
    Update();
    m_db.SaveContents(ccPin);
}

void Colony::Load()
{
    // redundant check
    if (m_loaded) {
        Update();
        return;
    }

    if (!m_pSE->HasCOSE())
        m_pSE->CreateCustomsOffice();

    m_db.LoadPins(m_colonyID, ccPin->pins);
    m_db.LoadLinks(m_colonyID, ccPin->links);
    m_db.LoadRoutes(m_colonyID, ccPin->routes);

    LoadPlants();

    for (auto &cur : ccPin->routes) {
        m_srcRoutes.emplace(cur.second.srcItrID, cur.second);
        m_destRoutes.emplace(cur.second.destItrID, cur.second);
    }

	// need more accurate check for this
    if (m_procTime < EvE::Time::Day)
        m_procTime = GetFileTimeNow();

    m_loaded = (!ccPin->pins.empty());
}

void Colony::Save() {
    m_db.SavePins(ccPin);
    m_db.SaveLinks(ccPin);
    m_db.SaveRoutes(ccPin);
    m_db.SaveContents(ccPin);
    m_db.UpdatePlanetPins(m_colonyID, ccPin->pins.size());
}

// called by PlanetSE::Process() for loaded colony.
//  NOTE: colony is loaded after client calls for it. (no preload)
void Colony::Process() {
    if (m_colonyTimer.Check()) { //  this will process colony data every 5 mins. (typical cycle time is 30m)
        if (ccPin->pins.empty()) {
            m_colonyTimer.Disable();
            return;
        }

        Update();
    }

    if (m_toUpdate) {
        //  this is part of clever code to avoid db hits on every update.
        //  this method will check for updated contents and save to db as needed.
        std::map<uint32, PI_Pin>::iterator itr;
        for (auto &cur : ccPin->pins) {
            // has this pin's data been updated since last save?
            if (cur.second.update) {
				// need to figure out how to update client with current contents
				//    maybe use onitemchange packet here?   storage has an itemRef
                cur.second.update = false;
            }
        }

		// we dont need to hit db on every update...only shutdown
        //m_db.UpdatePins(0, ccPin);
        m_toUpdate = false;
    }
}

uint32 Colony::GetOwner()
{
    return m_client->GetCharacterID();
}

void Colony::LoadPlants() {
    for (auto &cur: ccPin->pins) {
        // set proc time on load
        if (cur.second.isCommandCenter) {
            m_procTime = cur.second.lastRunTime;
            continue;
        }

        if (cur.second.isECU) {
            DBQueryResult res;
            DBResultRow row;
            m_db.LoadECU(cur.first, res);
            res.GetRow(row);
            PI_ECU ecu                  = PI_ECU();
            ecu.expiryTime              = row.GetInt64(0);
            ecu.headRadius              = row.GetDouble(1);
            ecu.headTypeID              = row.GetUInt16(2);
            ecu.programType             = row.GetUInt16(3);
			ecu.cycleCount		= ??

            m_db.LoadHeads(cur.first, ecu.heads);

            ccPin->ecus[cur.first] = ecu;
            continue;
        }

        // load plants in mem object
        if (cur.second.isProcess) {
            PI_Plant plant                  = PI_Plant();
            plant.data                      = PI_Schematic();
            plant.hasReceivedInputs         = false;
            plant.receivedInputsLastCycle   = false;

            if (cur.second.schematicID) {
                sPIDataMgr.GetSchematicData(cur.second.schematicID, plant.data);
                cur.second.cycleTime    = plant.data.cycleTime * EvE::Time::Second; // data.cycleTime is in seconds
                cur.second.qtyPerCycle  = plant.data.outputQty;     // this is not saved
                plant.pLevel        	= sPIDataMgr.GetProductLevel(plant.data.outputType);   // i am ordering plant processing by output's Plevel
            }

            if (m_pLevel < 1)
                m_pLevel = plant.pLevel;
            if (plant.pLevel < m_pLevel)
                m_pLevel = plant.pLevel;

            ccPin->plants[cur.first] = plant;
            m_plantMap.emplace(plant.pLevel, cur.first);
        }
    }

    // set process timer
    if (!m_colonyTimer.Enabled())
        m_colonyTimer.Start(sConfig.rates.ColonyTimer * EvE::Timer::Minute);
}

void Colony::AbandonColony()
{
    /** @todo  go thru entire pinMap and delete each itemRef to remove pin/link contents from db. */
    for (auto &cur : ccPin->pins) {
        m_db.RemovePin(cur.first);
        m_db.RemoveContents(cur.first);
        sItemFactory.RemoveItem(cur.first);
    }
    for (auto &cur : ccPin->links) {
        m_db.RemovePin(cur.first);
        sItemFactory.RemoveItem(cur.first);
    }
    InventoryItemRef iRef = sItemFactory.GetItemRef(m_colonyID);
    iRef->Delete();
    m_db.DeleteColony(m_colonyID, m_pSE->GetID(), m_client->GetCharacterID());
    SafeDelete(ccPin);
    ccPin = new PI_CCPin();
    m_colonyID = 0;
    m_colonyTimer.Disable();
}

void Colony::CreateCommandPin(uint32 itemID, uint32 typeID, double latitude, double longitude) {
    m_colonyID = itemID;
    ccPin->ccPinID = itemID;
    ccPin->level = PI::Pin::Level0;
    CreatePin(EVEDB::invGroups::Command_Centers, itemID, typeID, latitude, longitude);
    m_db.SaveCommandCenter(itemID, m_client->GetCharacterID(), m_pSE->GetID(), typeID, latitude, longitude);
    m_db.AddPlanetForChar(m_pSE->SystemMgr()->GetID(), m_pSE->GetID(), m_client->GetCharacterID(), m_colonyID, m_pSE->GetTypeID());
    //m_db.SavePins(ccPin);
}

void Colony::CreatePin(uint32 groupID, uint32 pinID, uint32 typeID, double latitude, double longitude) {
    /** @todo will have to write code for effects and checks for pg/cpu/m3/etc for all of these.  */
    // maybe not...client checks before sending command
    using namespace EVEDB::invGroups;
    PI_Pin pin = PI_Pin();
    InventoryItemRef iRef(nullptr);
    if (groupID == Command_Centers) {
        iRef = sItemFactory.GetItemRef(m_colonyID);
        if (iRef->quantity() > 1) {
            // check for stack of CC items, and split as needed
            ItemData data(typeID, m_client->GetCharacterID(), locTemp, flagAutoFit, iRef->quantity() -1);
            InventoryItemRef iRef2 = sItemFactory.SpawnItem(data);
            iRef2->Move(m_client->GetShipID(), flagCargoHold);
            iRef->SetQuantity(1);
        }
        m_client->GetShip()->RemoveItem(iRef);
    } else {
        // type, owner, location, flag, qty
        ItemData data(typeID, m_client->GetCharacterID(), m_pSE->GetID(), flagAutoFit, 1);
        iRef = sItemFactory.SpawnItem(data);

        /*  this shit doesnt work....changes arent sent to client.  not sure why
        m_pg = iRef->GetAttribute(AttrPowerLoad).get_int();
        m_cpu = iRef->GetAttribute(AttrCpuLoad).get_int();
        if (groupID != Planetary_Links) {
            // reset pg/cpu needs based on char skills for all modules (ex links)
            m_pg *= (1 - ( 0.05f * (m_client->GetChar()->GetSkillLevel(EvESkill::Engineering, true))));               // 5% decrease in need
            m_pg *= (1 - ( 0.01f * (m_client->GetChar()->GetSkillLevel(EvESkill::EnergyManagement, true))));          // 1% decrease in need
            m_pg *= (1 - ( 0.01f * (m_client->GetChar()->GetSkillLevel(EvESkill::CommandCenterUpgrades, true))));     // 1% decrease in need
            m_pg *= (1 - ( 0.01f * (m_client->GetChar()->GetSkillLevel(EvESkill::EnergySystemsOperation, true))));    // 1% decrease in need

            m_cpu *= (1 - ( 0.05f * (m_client->GetChar()->GetSkillLevel(EvESkill::Electronics, true))));              // 5% decrease in need
            m_cpu *= (1 - ( 0.01f * (m_client->GetChar()->GetSkillLevel(EvESkill::CommandCenterUpgrades, true))));    // 1% decrease in need
        } */
    }

    pin.typeID = typeID;
    pin.ownerID = m_client->GetCharacterID();
    pin.latitude = latitude;
    pin.longitude = longitude;
    pin.state = PI::Pin::State::Idle;
    pin.lastRunTime = 0;
    pin.installTime = GetFileTimeNow();

    switch(groupID) {
        case Command_Centers: {     // 1027
            pin.isStorage = true;
            pin.isLaunchable = true;
            pin.isCommandCenter = true;
        } break;
        case Processors: {         // 1028
            pin.isStorage = true;
            pin.isProcess = true;
            ccPin->plants[iRef->itemID()] = PI_Plant();
        } break;
        case Extractor_Control_Units: { // 1063
            pin.isECU = true;
            pin.qtyPerCycle = iRef->GetAttribute(AttrPinExtractionQuantity).get_uint32();
            ccPin->ecus[iRef->itemID()] = PI_ECU();
        } break;
        case Spaceports:{   // 1030
            pin.isStorage = true;
            pin.isLaunchable = true;
        } break;
        case Storage_Facilities: {  // 1029
            pin.isStorage = true;
        } break;
        case Mercenary_Bases:
        case Capsuleer_Bases: {
            pin.isBase = true;
            /* nothing to do yet */
        } break;
        case Planetary_Links:  // 1036
        case Extractors: {  // 1026
            /* make error.  should never get here.  these are NOT pins  */
            /*
            m_cpu = iRef->GetAttribute().get_int();
             1633    powerLoadPerKm          NULL    0.15
             1634    cpuLoadPerKm            NULL    0.2
             1635    cpuLoadLevelModifier    NULL    1.4
             1636    powerLoadLevelModifier  NULL    1.2
             */
        } break;
    }

    iRef->Move(m_pSE->GetID(), flagPlanetSurface, true);
    iRef->ChangeSingleton(true);
    // cannot change attributes on PI items.....  :(
    //iRef->SetAttribute(AttrCpuLoad, m_cpu);
    //iRef->SetAttribute(AttrPowerLoad, m_pg);

    m_db.CreatePin(ccPin->ccPinID, iRef->itemID(), pin);
    ccPin->pins[iRef->itemID()] = std::move(pin);

    // save map of tempID to itemID - this handles the stacked-calls from client to use real itemIDs
    if (groupID != Command_Centers)
        tempPinIDs.insert(std::pair<uint32, uint32>(pinID, iRef->itemID()));

    _log(COLONY__INFO, "Colony::CreatePin() - Created pin for %s(%u)", iRef->name(), iRef->itemID());
}

void Colony::CreateLink(uint32 src, uint32 dest, uint16 level) {
    if (IsTempPinID(src) and (tempPinIDs.size() > 0)) {
        std::map<uint32, uint32>::iterator itr = tempPinIDs.find(src);
        if (itr != tempPinIDs.end())
            src = itr->second;
    }
    if (IsTempPinID(dest) and (tempPinIDs.size() > 0)) {
        std::map<uint32, uint32>::iterator itr = tempPinIDs.find(dest);
        if (itr != tempPinIDs.end())
            dest = itr->second;
    }
    ItemData data(2280, m_client->GetCharacterID(), locTemp, flagAutoFit, 1);
    InventoryItemRef iRef = sItemFactory.SpawnItem(data);
    iRef->Move(m_pSE->GetID(), flagPlanetSurface, true);
    iRef->SaveItem();

    PI_Link link = PI_Link();
        link.state = PI::Pin::State::Idle;
        link.level = level;
        link.endpoint1 = src;
        link.endpoint2 = dest;
        link.typeID = 2280; // Only link type in the game.
    ccPin->links[iRef->itemID()] = link;

    m_db.SaveLinks(ccPin);
    _log(COLONY__INFO, "Colony::CreateLink() - Created link - id:%u,  src:%u, dest:%u, level:%u", iRef->itemID(), src, dest, level);
}

void Colony::CreateRoute(uint16 routeID, uint32 typeID, uint32 qty, PyList* path) {
    // routeID is sent as tempID like pins.
    std::list<uint32> list1;
    for (uint16 i(0); i < path->size(); ++i) {
        if (path->GetItem(i)->IsTuple()) {
            list1.push_back(PyRep::IntegerValue(path->GetItem(i)->AsTuple()->GetItem(1)));
        } else if (path->GetItem(i)->IsInt()) {
            list1.push_back(PyRep::IntegerValue(path->GetItem(i)));
        } else {
            _log(COLONY__ERROR, "Colony::CreateRoute() - List item type unrecognized: %s", path->GetItem(1)->TypeString());
        }
    }

    if (tempPinIDs.size() > 0) {
        std::list<uint32> list2;
        std::map<uint32, uint32>::iterator itr;
        for (auto &cur : list1) {
            if (IsTempPinID(cur)) {
                itr = tempPinIDs.find(cur);
                if (itr != tempPinIDs.end())
                    list2.push_back(itr->second);
            } else {
                list2.push_back(cur);
            }
        }
        list1.clear();
        list1 = std::move(list2);
    }

    PI_Route route = PI_Route();
        route.state = PI::Pin::State::Idle;
        route.priority = PI::Route::PriorityNorm;
        route.commodityTypeID = typeID;
        route.commodityQuantity = qty;
        route.srcItrID = list1.front();
        route.destItrID = list1.back();
        route.path = std::move(list1);

    routeID = m_db.SaveRoute(m_colonyID, route);
    ccPin->routes[routeID] = route;

    m_srcRoutes.emplace(route.srcItrID, route);
    m_destRoutes.emplace(route.destItrID, route);

    _log(COLONY__INFO, "Colony::CreateRoute() - Created route id %u for %u of typeID %u, making %u hops.", routeID, qty, typeID, (uint32)path->size() - 1);

    // route has been created and added to list.  check for materials being moved, and if source has the mat, remove qty and send to dest.
    std::map<uint32, PI_Pin>::iterator srcItr = ccPin->pins.find(route.srcItrID);
    if (srcItr == ccPin->pins.end())
        return;  // source not found.  make error here.
    std::map<uint32, PI_Pin>::iterator destItr = ccPin->pins.find(route.destItrID);
    if (destItr == ccPin->pins.end())
        return;  // destination not found.  make error here.
    std::map<uint16, uint32>::iterator itemItr = srcItr->second.contents.find(route.commodityTypeID);
    if (itemItr == srcItr->second.contents.end())
        return;  // this material wasnt found in source container....cant move what we aint got..

    uint16 amount = route.commodityQuantity;
    // remove contents from storage pin
    if (itemItr->second > amount) {
        itemItr->second -= amount;
    } else {
        amount = itemItr->second;
        srcItr->second.contents.erase(itemItr);
    }
    srcItr->second.update = true;

    // add contents to dest pin if we have any
    itemItr = destItr->second.contents.find(route.commodityTypeID);
    if (itemItr != destItr->second.contents.end()) {
        // should we verify pin capy?
        itemItr->second += amount;
    } else {
        destItr->second.contents[route.commodityTypeID] = amount;
    }
    destItr->second.update = true;
    m_toUpdate = true;
}

void Colony::UpgradeCommandCenter(uint32 pinID, int8 level) {
    ccPin->level = level;
    std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(pinID);
    if (itr != ccPin->pins.end()) {
        itr->second.level = level;
        m_db.SaveCCLevel(pinID, level);
        _log(COLONY__INFO, "Colony::UpgradeCommandCenter() - Upgraded Command Center %u to level:%u", pinID, level);
    } else {
        _log(COLONY__ERROR, "Colony::UpgradeCommandCenter() - pinID %u not found in ccPin.pins map", pinID);
    }
}

void Colony::UpgradeLink(uint32 src, uint32 dest, uint8 level)
{
	/** @todo is there a better way to do this instead of interating thru entire map? */
    std::map<uint32, PI_Link>::iterator itr = ccPin->links.begin();
    for (; itr != ccPin->links.end(); itr++)
        if (itr->second.endpoint1 == src)
            if (itr->second.endpoint2 == dest) {
                itr->second.level = level;
                m_db.SaveLinks(ccPin);
                return;
            }
}

void Colony::RemovePin(uint32 pinID)
{
    uint8 linkCount = 0, routeCount = 0, pathCount = 0;
    // check for and remove links to this pinID
    std::map<uint32, PI_Link>::iterator linkItr = ccPin->links.begin();
    while (linkItr != ccPin->links.end()) {
        if (linkItr->second.endpoint1 == pinID) {
            m_db.RemoveLink(linkItr->first);
            linkItr = ccPin->links.erase(linkItr);
            ++linkCount;
            continue;
        }
        if (linkItr->second.endpoint2 == pinID) {
            m_db.RemoveLink(linkItr->first);
            linkItr = ccPin->links.erase(linkItr);
            ++linkCount;
            continue;
        }
        ++linkItr;
    }

    // check for routes to, from, or thru this pin
    std::map<uint16, PI_Route>::iterator routeItr = ccPin->routes.begin();
    while (routeItr != ccPin->routes.end()) {
        /*
            uint32 srcItrID;
            uint32 destItrID;
            std::list<uint32> path;
         */
        if (routeItr->second.srcItrID == pinID) {
            m_db.RemoveRoute(routeItr->first);
            routeItr = ccPin->routes.erase(routeItr);
            ++routeCount;
            continue;
        }
        if (routeItr->second.destItrID == pinID) {
            m_db.RemoveRoute(routeItr->first);
            routeItr = ccPin->routes.erase(routeItr);
            ++routeCount;
            continue;
        }
        // at this point, the pin being removed isnt src or dest, so check for intermediate routing.
        std::list<uint32>::iterator pItr = routeItr->second.path.begin();
        while (pItr != routeItr->second.path.end()) {
            //  we may have to reroute from this pin removal...
            if (*pItr == pinID) {
                //reroute = true;
                pItr = routeItr->second.path.erase(pItr);
                ++pathCount;
                continue;
            }
            ++pItr;
        }
        ++routeItr;
    }

    m_srcRoutes.erase(pinID);
    m_destRoutes.erase(pinID);

    ccPin->pins.erase(pinID);
    ccPin->plants.erase(pinID);  // may or may not be here.
    //m_ECUs.erase(pinID);
    m_db.RemovePin(pinID);
    m_db.SaveLinks(ccPin);
    m_db.SaveRoutes(ccPin);
    m_db.UpdatePlanetPins(m_colonyID, ccPin->pins.size());
    _log(COLONY__INFO, "Colony::RemovePin() - Removed pin %u with %u routes and %u links.  Upset %u routes by removing this pin", \
                            pinID, routeCount, linkCount, pathCount);
}

void Colony::RemoveLink(uint32 src, uint32 dest) {
    std::map<uint32, PI_Link>::iterator linkItr = ccPin->links.begin();
    for (; linkItr != ccPin->links.end(); ++linkItr) {
        if (linkItr->second.endpoint1 == src) {
            if (linkItr->second.endpoint2 == dest) {
                _log(COLONY__INFO, "Colony::RemoveLink() - Removing linkID %u - src: %u, dest: %u", linkItr->first, src, dest);
                m_db.RemoveLink(linkItr->first);
                ccPin->links.erase(linkItr);
                return;
            }
        }
    }
}

void Colony::RemoveRoute(uint16 routeID) {
    std::map<uint16, PI_Route>::iterator routeItr = ccPin->routes.find(routeID);
    if (routeItr != ccPin->routes.end()) {
        m_srcRoutes.erase(routeItr->second.srcItrID);
        m_destRoutes.erase(routeItr->second.destItrID);
    }
    ccPin->routes.erase(routeID);
    m_db.RemoveRoute(routeID);
    _log(COLONY__INFO, "Colony::RemoveRoute() - Removed route: %u", routeID);
}

void Colony::AddExtractorHead(uint32 ecuID, uint16 headID, double latitude, double longitude) {
    std::map<uint32, PI_ECU>::iterator ecuItr = ccPin->ecus.find(ecuID);
    if (ecuItr == ccPin->ecus.end()) {
        _log(COLONY__ERROR, "Colony::MoveExtractorHead() - ecuID %u not found in ccPin.pins map", ecuID);
        return;
    }


    m_newHead = true;
    tempECUs.push_back(ecuID);
    PI_Heads head = PI_Heads();
        head.typeID = ecuItr->second.programType;
        head.ecuPinID = ecuID;
        head.latitude = latitude;
        head.longitude = longitude;
    ecuItr->second.heads[headID] = head;
}

void Colony::MoveExtractorHead(uint32 ecuID, uint16 headID, double latitude, double longitude) {
    std::map<uint32, PI_ECU>::iterator ecuItr = ccPin->ecus.find(ecuID);
    if (ecuItr == ccPin->ecus.end()) {
        _log(COLONY__ERROR, "Colony::MoveExtractorHead() - ecuID %u not found in ccPin.pins map", ecuID);
        return;
    }

    std::map<uint16, PI_Heads>::iterator headItr = ecuItr->second.heads.find(headID);
    if (headItr == ecuItr->second.heads.end()) {
        _log(COLONY__ERROR, "Colony::MoveExtractorHead() - headID %u not found in pin.heads map", headID);
        return;
    }

    m_newHead = true;
    tempECUs.push_back(ecuID);
    // find head and update....
    headItr->second.latitude = latitude;
    headItr->second.longitude = longitude;
}

void Colony::KillExtractorHead(uint32 ecuID, uint16 headID) {
    std::map<uint32, PI_ECU>::iterator ecuItr = ccPin->ecus.find(ecuID);
    if (ecuItr == ccPin->ecus.end()) {
        _log(COLONY__ERROR, "Colony::KillExtractorHead() - ecuID %u not found in ccPin.pins map", ecuID);
        return;
    }
    ecuItr->second.heads.erase(headID);
}

void Colony::SetSchematic(uint32 pinID, uint8 schematicID/*0*/) {
    if (IsTempPinID(pinID) and (tempPinIDs.size() > 0)) {
        std::map<uint32, uint32>::iterator itr = tempPinIDs.find(pinID);
        if (itr != tempPinIDs.end())
            pinID = itr->second;
    }

    std::map<uint32, PI_Plant>::iterator plantItr = ccPin->plants.find(pinID);
    if (plantItr == ccPin->plants.end()) {
        _log(COLONY__ERROR, "Colony::SetSchematic() - plantID %u not found in ccPin.plants map", pinID);
        return;
    }

    std::map<uint32, PI_Pin>::iterator pinItr = ccPin->pins.find(pinID);

    if (schematicID) {
        // install new schematic.  set lastRunTime to 0.  set installTime to now.   update
        sPIDataMgr.GetSchematicData(schematicID, plantItr->second.data);
        plantItr->second.pLevel                  = sPIDataMgr.GetProductLevel(plantItr->second.data.outputType);
        pinItr->second.cycleTime                 = plantItr->second.data.cycleTime * sConfig.rates.PlantCycleMod * EvE::Time::Second;
        pinItr->second.qtyPerCycle               = plantItr->second.data.outputQty;
        pinItr->second.schematicID               = schematicID;
        pinItr->second.lastRunTime               = 0;
        plantItr->second.hasReceivedInputs       = false;
        plantItr->second.receivedInputsLastCycle = false;

        if (m_pLevel < 1)
            m_pLevel = plantItr->second.pLevel;
        if (plantItr->second.pLevel < m_pLevel)
            m_pLevel = plantItr->second.pLevel;

        pinItr->second.state = PI::Pin::State::Active;

        // set process timer to 30m
        if (!m_colonyTimer.Enabled())
            m_colonyTimer.Start(sConfig.rates.ColonyTimer * EvE::Timer::Minute);
        _log(COLONY__INFO, "Colony::SetSchematic() - Set Schematic %u in plantID %u", schematicID, pinID);
    } else {
        plantItr->second = PI_Plant();
        plantItr->second.data = PI_Schematic();
        pinItr->second.state = PI::Pin::State::Idle;
        _log(COLONY__INFO, "Colony::SetSchematic() - Cleared Schematic from plantID %u", pinID);
    }
}

void Colony::InstallProgram(uint32 ecuID, uint16 typeID, double headRadius) {
    /*
     * 09:54:54 [PlanetCallDump]       [ 0]   [10]   [ 1]  Tuple: 3 elements
     * 09:54:54 [PlanetCallDump]       [ 0]   [10]   [ 1]   [ 0]    Integer: 140000565  ecuID
     * 09:54:54 [PlanetCallDump]       [ 0]   [10]   [ 1]   [ 1]    Integer: 2272       typeID
     * 09:54:54 [PlanetCallDump]       [ 0]   [10]   [ 1]   [ 2]       Real: 0.011281   headRadius
     */
    std::map<uint32, PI_Pin>::iterator pinItr = ccPin->pins.find(ecuID);
    if (pinItr == ccPin->pins.end()) {
        ccPin->ecus.erase(ecuID);
        _log(COLONY__ERROR, "Colony::InstallProgram() - ecuPinID %u not found in ccPin.pins map", ecuID);
        return;
    }

    std::map<uint32, PI_ECU>::iterator ecuItr = ccPin->ecus.find(ecuID);

	ecuItr->second.cycleCount = 0;
    ecuItr->second.expiryTime = 0;
    ecuItr->second.headTypeID = 0;
    ecuItr->second.programType = 0;
    pinItr->second.lastRunTime = 0;

    if (typeID < 1) {
        // uninstall program
        pinItr->second.state = PI::Pin::State::Idle;
        pinItr->second.installTime = 0;
        ecuItr->second.headRadius = 0.0;
        //ecuItr->second.heads.clear();
        // reset extraction quantity in ecu attrib.  this doesnt check for invalid item
        sItemFactory.GetItemRef(ecuID)->ResetAttribute(AttrPinExtractionQuantity);
        return;
    } else {
        // install program
        pinItr->second.state = PI::Pin::State::Active;
        pinItr->second.installTime = GetFileTimeNow();
        ecuItr->second.headRadius = headRadius;
        PyList* heads = new PyList();
        for (auto &cur : ecuItr->second.heads)
            heads->AddItem(new PyInt(cur.second.typeID));

        // set up extractor program data
        sPIDataMgr.GetProgramResultInfo(this, ecuID, typeID, headRadius, heads);
    }
}

void Colony::SetProgramResults(uint32 ecuID, uint16 typeID, uint16 numCycles, double headRadius, float cycleTime, uint32 qtyPerCycle)
{
    std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(ecuID);
    if (itr == ccPin->pins.end()) {
        _log(COLONY__ERROR, "Colony::SetProgramResults() - ecuPinID %u not found in ccPin.pins map", ecuID);
        return;
    }

    std::map<uint32, PI_ECU>::iterator ecuItr = ccPin->ecus.find(ecuID);

    itr->second.cycleTime = cycleTime * EvE::Time::Hour;
    itr->second.qtyPerCycle = qtyPerCycle;
	ecuItr->second.cycleCount = numCycles;
    ecuItr->second.programType = typeID;
    ecuItr->second.expiryTime = cycleTime * EvE::Time::Hour * numCycles + GetFileTimeNow();
    ecuItr->second.headRadius = headRadius;
    ecuItr->second.headTypeID = sPIDataMgr.GetHeadType(sItemFactory.GetItemRef(ecuID)->typeID(), typeID);

    m_db.UpdateECUPin(ecuID, ccPin);

    // save extraction quantity in ecu attrib    this doesnt check for invalid item
    sItemFactory.GetItemRef(ecuID)->SetAttribute(AttrPinExtractionQuantity, qtyPerCycle, false);

    // set process timer to 30m
    if (!m_colonyTimer.Enabled())
        m_colonyTimer.Start(sConfig.rates.ColonyTimer * EvE::Timer::Minute);
}
/* {'FullPath': u'UI/Messages', 'messageID': 256790, 'label': u'PlanetBlackListedBody'}(u'{planet} is not available for the general public.', None, {u'{planet}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'planet'}})
 * {'FullPath': u'UI/Messages', 'messageID': 256791, 'label': u'CannotInstallWithoutScanResultsBody'}(u'Your mining foreman reports that an intern seems to have misplaced the necessary mineral survey results. You will need to order a fresh deposit scan before this {typeName} can begin operating.', None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 256792, 'label': u'QueueCannotTrainPastMaximumLevelBody'}(u'You cannot train {typeName} further, as is it already at maximum level.', None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 256793, 'label': u'CannotUpgradeLinkAlreadyMaxedBody'}(u"You cannot upgrade this {typeName}, as it has already been upgraded to technology's bleeding edge.", None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 256794, 'label': u'CreateRouteDestinationCannotAcceptCommodityBody'}(u'You are unable to create that route, as the destination is unable to utilize {typeName}.', None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 256795, 'label': u'CreateRouteCommodityProductionTooSmallBody'}(u"You are unable to create this shipping route as the route's origin would not produce enough {typeName} to fulfill all of its existing routes, in addition to the new one.", None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 256796, 'label': u'CreateRouteCommodityNotProducedBody'}(u"You are unable to create a shipping route for {typeName}, as it is not produced at the route's origin.", None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
 */
PyDict* Colony::TransferCommodities(uint32 srcID, uint32 destID, std::map< uint16, uint32 > items) {
    std::map<uint32, PI_Pin>::iterator src = ccPin->pins.find(srcID);
    if (src == ccPin->pins.end()) {
        _log(COLONY__ERROR, "Colony::TransferCommodities() - srcItr %u not found in ccPin.pins map", srcID);
        if (m_client->CanThrow())
            throw CustomError("Source not found.");
        return nullptr; // make error and return.
    }
    std::map<uint32, PI_Pin>::iterator dest = ccPin->pins.find(destID);
    if (dest == ccPin->pins.end()) {
        _log(COLONY__ERROR, "Colony::TransferCommodities() - destItr %u not found in ccPin.pins map", destID);
        if (m_client->CanThrow())
            throw CustomError("Destination not found.");
        return nullptr; // make error and return.
    }
    /*{'FullPath': u'UI/Messages', 'messageID': 256630, 'label': u'ExpeditedTransferNotEnoughSpaceBody'}(u'There is not enough space at the transfer destination for the selected commodities.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 256775, 'label': u'CannotPutMissionItemInCargolinkBody'}(u'You cannot store the {typeName} in a planetary customs facility, as it an agent has issued a special embargo for this particular item.', None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
     * {'FullPath': u'UI/Messages', 'messageID': 256776, 'label': u'CannotExportCommodityNotEnoughBody'}(u'Your request to export {desired} units of {typeName} cannot be fulfilled, as the spaceport only has {contained} in stock.', None, {u'{contained}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'contained'}, u'{desired}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'desired'}, u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
     * {'FullPath': u'UI/Messages', 'messageID': 256777, 'label': u'CannotExportCommodityNotFoundBody'}(u"You cannot export {typeName}, as your spaceport's storehouse does not appear to contain any.", None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
     * {'FullPath': u'UI/Messages', 'messageID': 256778, 'label': u'RouteFailedValidationExpeditedSourceLacksCommodityQtyBody'}(u"You cannot perform this expedited transfer as the facility from which you're sourcing your commodities currently lacks the requested {qty} units of {typeName}.", None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}, u'{qty}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'qty'}})
     * {'FullPath': u'UI/Messages', 'messageID': 256779, 'label': u'RouteFailedValidationExpeditedSourceLacksCommodityBody'}(u'You cannot perform this expedited transfer, as the facility from which you are sourcing your commodities appears to lack the {typeName} which you wish to transfer.', None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
     */

    // capacities are checked in client.  proceed with xfer
    for (auto &cur : items) {
        std::map<uint16, uint32>::iterator srcItr = src->second.contents.find(cur.first), destItr = dest->second.contents.find(cur.first);
        if (srcItr != src->second.contents.end()) {
            if (srcItr->second > cur.second) {
                srcItr->second -= cur.second;
            } else {
                src->second.contents.erase(srcItr);
            }
        }   //  if src contents not found, assume client is right and proceed with xfer
        if (destItr != dest->second.contents.end()) {
            destItr->second += cur.second;
        } else {
            dest->second.contents[cur.first] = cur.second;
        }
    }

    //update pin contents
    m_db.SaveContents(ccPin);

    // simTime = time to stop (currentSimTime), sourceRunTime = lastRunTime
    PyDict* args = new PyDict();
    /** @todo this needs to be updated to use process times (for next cycle end) */
    args->SetItemString("simTime", new PyLong(GetFileTimeNow()));
    args->SetItemString("sourceRunTime", new PyLong(m_procTime));

    /*
     * def GetExpeditedTransferTime(linkBandwidth, commodities):
     *    commodityVolume = GetCommodityTotalVolume(commodities)
     *    return long(math.ceil(max(5 * MIN, float(commodityVolume) / linkBandwidth * HOUR)))
     */
    return args;
}

/*
Export fee = Base cost × tax rate (×1.5 if launched via CC)
Import fee = Base cost × tax rate × 0.5
To open the planet over view from anywhere Press F11 and in the side panel you can use the bottom window to select planet view by right clicking the menu box in the left corner.
By switching solar systems or regions in the above boxes you can scan planets in regions as far as your abilities allow.
In the solar system box you can use show info under each solar system and look at orbital bodies to get a list of planet type rather than look at them one at a time. You can also view planet directly from the list.
You can deploy Command Centers while docked, but you must be in the same system as the planet, and the command center must be in your ship's hold.
https://www.eve-icsc.com/jumptools/jumpplanner.php use this link you calculate LY range to see what systems will be in range based on your Remote Sensing skill level. It will help with planning.
*/

/*  import/export taxes
 *  GetProductLevel(typeID) will return PLevel of item.
 *  use that to calculate cost for import/export operations
 * Product     Command Center Export Cost  Launchpad Export Cost   Launchpad Import Cost
 *    P0         15/m3 or .15/unit           10/m3 or .1/unit        5/m3 or .05/unit
 *    P1          3/m3 or 1.14/unit           2/m3 or .76/unit       1/m3 or .38/unit
 *    P2          9/m3 or 13.5/unit           6/m3 or 9/unit         3/m3 or 4.5/unit
 *    P3        150/m3 or 900/unit          100/m3 or 600/unit      50/m3 or 300/unit
 *    P4        750/m3 or 75k/unit          500/m3 or 50k/unit     250/m3 or 25k/unit
 */

PyRep* Colony::LaunchCommodities(uint32 pinID, std::map< uint16, uint32 >& items)
{
    // this will export items from CC to jetcan in space.
    // launchpad (Spaceport) xfers items to/from customs office
    std::map<uint32, PI_Pin>::iterator pin = ccPin->pins.find(pinID);
    if (pin == ccPin->pins.end()) {
        _log(COLONY__ERROR, "Colony::LaunchCommodities() - pinID %u not found in ccPin.pins map", pinID);
        return nullptr;
    }

    // first - create jetcan, add to system, and put in orbit around planet
    // NOTE:  PI launches have 5d timers
    /** @todo check capacities before adding items */
    SystemManager* pSysMgr(m_pSE->SystemMgr());
    GPoint location(pSysMgr->GetSE(m_pSE->GetID())->GetPosition());
	/* NOTE:  launches spawn ~10000Km from customs office
	 * create entry in journal (pi launches)
	 * create bm?
	 *  cannot be scanned by probes (no anom sig), but does show on d-scan
	 * 5d timer
	 */
    location.MakeRandomPointOnSphere(m_pSE->GetRadius() + 2000000);   //2000km orbit for launch can
    ItemData canData(EVEDB::invTypes::PlanetaryLaunchContainer,
                    m_client->GetCharacterID(),  // owner is Character
                    pSysMgr->GetID(),
                    flagAutoFit,
                    "PI Commodities Container",  // do we want to advertise like this?
                    location);

    CargoContainerRef contRef = sItemFactory.SpawnCargoContainer(canData);
    if (contRef.get() == nullptr) {
        contRef->Delete();
        if (m_client->CanThrow())
            throw CustomError("Unable to spawn item of type %u.", EVEDB::invTypes::PlanetaryLaunchContainer);
    }

    FactionData data = FactionData();
        data.allianceID = m_client->GetAllianceID();
        data.corporationID = m_client->GetCorporationID();
        data.factionID = m_client->GetWarFactionID();
        data.ownerID = m_client->GetCharacterID();
    // create new container SE
    ContainerSE* cSE = new ContainerSE(contRef, *m_svcMgr, pSysMgr, data);
    contRef->SetMySE(cSE);      // item-to-entity internal interface
    //cSE->AnchorContainer();     // avoid GC checks on this container  -no.  has 5d timer set
    pSysMgr->AddEntity(cSE);

    /* second - reduce qtys in source container (CC pin.contents in this case)
     * create actual item (previously only virtual)
     * add to container
     * calculate taxes on items
     * charge char taxes upon launch
     */
    uint8 count(0);
    double cost(0);
    for (auto &cur : items) {
        std::map<uint16, uint32>::iterator cont = pin->second.contents.find(cur.first);
        if (cont != pin->second.contents.end()) {
            if (cont->second >= cur.second) {
                cont->second -= cur.second;
            } else {
                // set qty to amount contained in pin.
                cur.second = cont->second;
                pin->second.contents.erase(cont);
            }
        } else {
            _log(COLONY__WARNING, "Colony::LaunchCommodities() - item %u not found in command center", cur.first);
            continue;
        }

        //  if item not found in src contents, assume client is right and proceed with xfer
        switch (sPIDataMgr.GetProductLevel(cur.first)) {
            case 0:     cost += (    0.15f * cur.second);    break; //5
            case 1:     cost += (    1.14f * cur.second);    break; //400
            case 2:     cost += (   13.50f * cur.second);    break; //7200
            case 3:     cost += (  900.00f * cur.second);    break; //60000
            case 4:     cost += (75000.00f * cur.second);    break; //1200000
        }
        ItemData iData(cur.first, m_client->GetCharacterID(), locTemp, flagAutoFit, cur.second);
        InventoryItemRef iRef = sItemFactory.SpawnItem(iData);
        if (iRef.get() == nullptr)
            continue;
        // verify we're not overloading container capy
        if (contRef->GetMyInventory()->HasAvailableSpace(flagAutoFit, iRef)) {
            iRef->Move(cSE->GetID());
            iRef->SaveItem();
            ++count;
        } else {
            _log(COLONY__WARNING, "%s: PI Commodity Container %u is full.", m_client->GetName(), contRef->itemID());
            m_client->SendErrorMsg("Your Commodity Container is full.  Some items were not transferred.");
            break;
        }
    }

    if (count)
        if (is_log_enabled(COLONY__TRACE))
            _log(COLONY__TRACE, "Colony::LaunchCommodities() - Launched %u items from command center %u to %s (%u)", \
                        count, m_colonyID, contRef->name(), contRef->itemID() );

    contRef->SaveItem();
    pin->second.lastLaunchTime = GetFileTimeNow();

    // third - create db entry for launch
    m_db.SaveLaunch(contRef->itemID(), m_client->GetCharacterID(), pSysMgr->GetID(), m_pSE->GetID(), location);

    std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(m_colonyID);
    if (itr != ccPin->pins.end())
        itr->second.lastRunTime = m_procTime;

    // just update contents and launch time
    m_db.UpdatePins(m_colonyID, ccPin);
    m_db.SaveContents(ccPin);

    // fourth - take taxes and record entry in journal
    if (cost) {
        //take the money, send wallet blink event record the transaction in their journal.
        std::string reason = "DESC:  Launching PI items from ";
        reason += m_pSE->GetName();
        AccountService::TransferFunds(
                    m_client->GetCharacterID(),
                    corpCONCORD,  // pSysMgr->GetSovHolder(), customs office owner
                    cost,
                    reason.c_str(),
                    Journal::EntryType::PlanetaryExportTax,
                    m_pSE->GetID(),
                    Account::KeyType::Cash);
    }

    return new PyLong(pin->second.lastLaunchTime);
}

void Colony::PlanetXfer(uint32 spaceportID, std::map< uint32, uint16 > importItems, std::map< uint32, uint16 > exportItems, double taxRate)
{
	//High-sec Customs Offices(CO) have a 10% NPC tax rate
    // import is from CO to planet.  export is from planet to CO
    // this method will make the transfer of items from real to virtual and back as necessary

    std::map<uint32, PI_Pin>::iterator pin = ccPin->pins.find(spaceportID);
    if (pin == ccPin->pins.end()) {
        _log(COLONY__ERROR, "Colony::PlanetXfer() - pinID %u not found in ccPin.pins map", spaceportID);
        if (m_client->CanThrow())
            throw CustomError("Your SpacePort on %s was not found.", m_pSE->GetName());

        return;
    }

    uint8 toColony(0), fromColony(0);
    double cost(0.0);
    InventoryItemRef iRef(nullptr);
    std::map<uint16, uint32>::iterator itr;
    // import
    for (auto &cur : importItems) {
        // xfer real item to virtual
        iRef = sItemFactory.GetItemRef(cur.first);
        if (iRef.get() == nullptr) {
            _log(COLONY__ERROR, "Colony::PlanetXfer():import - itemRef for id %u not found in ItemFactory", cur.first);
            continue;   // should never happen
        }

        /** @todo  check for available capy and adjust qty accordingly.
         *       if spaceport cant hold entire xfer qty, xfer to full, and return rest back to CO.
         */
        itr = pin->second.contents.find(iRef->typeID());
        if (itr != pin->second.contents.end()) {
            itr->second += cur.second;
        } else {
            pin->second.contents[iRef->typeID()] = cur.second;
        }

        switch (sPIDataMgr.GetProductLevel(iRef->typeID())) {
            case 0:     cost += (     .05f * cur.second);    break; //5
            case 1:     cost += (    0.38f * cur.second);    break; //400
            case 2:     cost += (    4.50f * cur.second);    break; //7200
            case 3:     cost += (  300.00f * cur.second);    break; //60000
            case 4:     cost += (25000.00f * cur.second);    break; //1200000
        }

        iRef->ToVirtual(spaceportID);
        ++toColony;
    }

    if (toColony)
        if (is_log_enabled(COLONY__TRACE))
            _log(COLONY__TRACE, "Colony::PlanetXfer() - Imported %u items from customs office %u to spaceport %u", \
                            toColony, m_pSE->GetID(), spaceportID);

    if (cost) {
        //take the money, send wallet blink event record the transaction in their journal.
        std::string reason = "DESC:  Importing items to ";
        reason += m_pSE->GetName();
        AccountService::TransferFunds(
                            m_client->GetCharacterID(),
                            m_pSE->GetOwnerID(),
                            cost,
                            reason.c_str(),
                            Journal::EntryType::PlanetaryImportTax,
                            m_pSE->GetID(),
                            Account::KeyType::Cash);
		//TODO:  in empire space, there is also an NPC tax of 5% paid to sov holder and listed as "corp tax"
    }

    // reset cost for export taxes
    cost = 0;
    // export
    for (auto &cur : exportItems) {
        std::map<uint16, uint32>::iterator cont = pin->second.contents.find(cur.first);
        if (cont != pin->second.contents.end()) {
            if (cont->second > cur.second) {
                cont->second -= cur.second;
            } else {
                // set qty to amount contained in pin.
                cur.second = cont->second;
                pin->second.contents.erase(cont);   // remove item from pin.contents if exporting entire qty.
            }
        } else {
            _log(COLONY__WARNING, "Colony::PlanetXfer():export - item %u not found in spaceport", cur.first);
            //  if item not found in src contents, assume client is right and proceed with xfer?
            //continue;
        }

        switch (sPIDataMgr.GetProductLevel(cur.first)) {
            case 0:     cost += (    0.10f * cur.second);    break; //5
            case 1:     cost += (    0.76f * cur.second);    break; //400
            case 2:     cost += (    9.00f * cur.second);    break; //7200
            case 3:     cost += (  600.00f * cur.second);    break; //60000
            case 4:     cost += (50000.00f * cur.second);    break; //1200000
        }
        // xfer virtual item to real
        ItemData iData(cur.first, m_client->GetCharacterID(), locTemp, flagAutoFit, cur.second);
        InventoryItemRef iRef = sItemFactory.SpawnItem(iData);
        iRef->Move(m_pSE->GetCustomsOffice()->GetID(), flagHangar, true);
        ++fromColony;
    }

    if (fromColony)
        if (is_log_enabled(COLONY__TRACE))
            _log(COLONY__TRACE, "Colony::PlanetXfer() - Exported %u items from spaceport %u to customs office %u", \
                        fromColony, spaceportID, m_pSE->GetID());

    if (cost) {
        //take the money, send wallet blink event record the transaction in their journal.
        std::string reason = "DESC:  Exporting items from ";
        reason += m_pSE->GetName();
        AccountService::TransferFunds(
                            m_client->GetCharacterID(),
                            m_pSE->GetOwnerID(),
                            cost,
                            reason.c_str(),
                            Journal::EntryType::PlanetaryExportTax,
                            m_pSE->GetID(),
                            Account::KeyType::Cash);
		//TODO:  in empire space, there is also an NPC tax of 5% paid to sov holder and listed as "corp tax"
    }

    // update contents
    m_db.SaveContents(ccPin);
}

void Colony::PrioritizeRoute(uint16 routeID, uint8 priority) {
    // set priority level for route...still not sure how to use it
    std::map<uint16, PI_Route>::iterator routeItr = ccPin->routes.find(routeID);
    if (routeItr != ccPin->routes.end()) {
        routeItr->second.priority = priority;
        m_db.SaveRoutes(ccPin);
    }
}

PyTuple* Colony::GetPins() {
    uint8 index(0);
    PyTuple* pins(new PyTuple(ccPin->pins.size()));

    for (auto &cur : ccPin->pins) {
        PyDict* dict = new PyDict();
        dict->SetItem("id", new PyInt(cur.first));
        dict->SetItem("typeID", new PyInt(cur.second.typeID));
        dict->SetItem("ownerID", new PyInt(cur.second.ownerID));
        dict->SetItem("latitude", new PyFloat(cur.second.latitude));
        dict->SetItem("longitude", new PyFloat(cur.second.longitude));
        dict->SetItem("lastRunTime", (cur.second.lastRunTime > 0 ? new PyLong(cur.second.lastRunTime) : PyStatic.NewNone()));
        dict->SetItem("state", new PyInt(cur.second.state));
        dict->SetItem("level", new PyInt(cur.second.level));

        PyDict* contents(new PyDict());
        if (cur.second.isStorage) {
            for (auto &cur2 : cur.second.contents)
                contents->SetItem(new PyInt(cur2.first), new PyInt(cur2.second));
        }
        dict->SetItem("contents", contents);

        if (cur.second.isLaunchable)
            dict->SetItem("lastLaunchTime", (cur.second.lastLaunchTime > 0 ? new PyLong(cur.second.lastLaunchTime) : PyStatic.NewNone()));

        if (cur.second.isProcess) {
            if (cur.second.schematicID) {
                dict->SetItem("schematicID", new PyInt(cur.second.schematicID));
                std::map<uint32, PI_Plant>::iterator plantItr = ccPin->plants.find(cur.first);
                if (plantItr != ccPin->plants.end()) {
                    dict->SetItem("cycleTime", new PyLong(cur.second.cycleTime));
                    dict->SetItem("hasReceivedInputs", new PyBool(plantItr->second.hasReceivedInputs));
                    dict->SetItem("receivedInputsLastCycle", new PyBool(plantItr->second.receivedInputsLastCycle));
                } else {
                    dict->SetItem("cycleTime", PyStatic.NewZero());
                    dict->SetItem("hasReceivedInputs", PyStatic.NewFalse());
                    dict->SetItem("receivedInputsLastCycle", PyStatic.NewFalse());
                }
            } else {
                dict->SetItem("cycleTime", PyStatic.NewZero());
                dict->SetItem("hasReceivedInputs", PyStatic.NewFalse());
                dict->SetItem("receivedInputsLastCycle", PyStatic.NewFalse());
            }
        }

        if (cur.second.isECU) {
            std::map<uint32, PI_ECU>::iterator ecuItr = ccPin->ecus.find(cur.first);
            if (cur.second.installTime) {
                dict->SetItem("cycleTime", new PyLong(cur.second.cycleTime));
                dict->SetItem("expiryTime", new PyLong(ecuItr->second.expiryTime));
                dict->SetItem("headRadius", new PyFloat(ecuItr->second.headRadius));
                dict->SetItem("installTime", new PyLong(cur.second.installTime));
                dict->SetItem("programType", new PyInt(ecuItr->second.programType));
                dict->SetItem("qtyPerCycle", new PyInt(cur.second.qtyPerCycle));
            }
            PyList* list(new PyList());
            for (auto &head : ecuItr->second.heads) {
                PyTuple* tuple = new PyTuple(3);
                    tuple->SetItem(0, new PyInt(head.first));
                    tuple->SetItem(1, new PyFloat(head.second.latitude));
                    tuple->SetItem(2, new PyFloat(head.second.longitude));
                list->AddItem(tuple);
            }
            dict->SetItem("heads", list);
        }

        pins->SetItem(index++, new PyObject("util.KeyVal", dict));
    }
    return pins;
}

PyTuple* Colony::GetLinks() {
    uint8 index(0);
    PyTuple* links = new PyTuple(ccPin->links.size());
    for (auto &cur : ccPin->links) {
        PyDict* dict = new PyDict();
            dict->SetItem("linkID", new PyInt(cur.first));                 // this is link itemID
            dict->SetItem("endpoint1", new PyInt(cur.second.endpoint1));
            dict->SetItem("endpoint2", new PyInt(cur.second.endpoint2));
            dict->SetItem("level", new PyInt(cur.second.level));
            dict->SetItem("typeID", new PyInt(cur.second.typeID));          // typeID 2280
        links->SetItem(index++, new PyObject("util.KeyVal", dict));
    }
    return links;
}

PyTuple* Colony::GetRoutes() {
    uint8 index(0);
    PyTuple* routes = new PyTuple(ccPin->routes.size());

    for (auto &cur : ccPin->routes) {
        PyDict* dict = new PyDict();
            dict->SetItem("routeID", new PyInt(cur.first));                 // this is routeID (low number - assigned by client)
            dict->SetItem("commodityTypeID", new PyInt(cur.second.commodityTypeID));
            dict->SetItem("commodityQuantity", new PyInt(cur.second.commodityQuantity));

        PyList* list = new PyList();
        for (auto &cur2 : cur.second.path)                               // path of pinIDs this route will follow
            list->AddItem(new PyInt(cur2));
        dict->SetItem("path", list);                                    // list of paths on this route
        routes->SetItem(index++, new PyObject("util.KeyVal", dict));
    }
    return routes;
}

PyRep* Colony::GetColony() {
    if (m_newHead) {
        for (auto &cur : tempECUs) {
            std::map<uint32, PI_ECU>::iterator ecuItr = ccPin->ecus.find(cur);
            if (itr != ccPin->pins.end()) {
                m_db.SaveHeads(m_colonyID, m_client->GetCharacterID(), cur, ecuItr->second.heads);
            } else {
                _log(COLONY__ERROR, "Colony::GetColony()::SaveHeads() - headID %u not found in ccPin.pins map", cur);
            }
        }
        tempECUs.clear();
        m_newHead = false;
    }

    Update();   // update colony before sending data.

    PyDict* args = new PyDict();
        args->SetItem("pins", GetPins());
        args->SetItem("level", new PyInt(ccPin->level));
        args->SetItem("links", GetLinks());
        args->SetItem("routes", GetRoutes());
        args->SetItem("currentSimTime", new PyLong(m_procTime));
    PyObject* res = new PyObject("util.KeyVal", args);

    if (is_log_enabled(COLONY__GC_DUMP)) {
        _log(COLONY__GC_DUMP, "Colony::GetColony() Dump");
        res->Dump(COLONY__GC_DUMP, "    ");
    }

    // reset tempPinID-to-newPinID map after command loop is completed and all new pins have been created.
    tempPinIDs.clear();

    return res;
}

void Colony::Update(bool updateTimes/*false*/) {
    double profileStartTime(GetTimeUSeconds());

    /* loop thru process calls to update each pin to simulate production and logistics
     *  this will have to be fast, as there may/will be large time deltas between updates
     *  can loop each item to process for each time step (like i do for skill training)
     */

    if (is_log_enabled(COLONY__DEBUG))
        _log(COLONY__DEBUG, "Colony::Update() - Starting Update for colony %u on %s.", m_colonyID, m_pSE->GetName());

    // update colony time to current time
    m_procTime = GetFileTimeNow();

    // first, process ecus for raw matls.
    ProcessECUs(updateTimes);
    // second, process plants with matl's received
    ProcessPlants(updateTimes);

    // update CommandCenter runtime
    if (updateTimes) {
        std::map<uint32, PI_Pin>::iterator pinItr = ccPin->pins.find(m_colonyID);
        if (pinItr != ccPin->pins.end())
            pinItr->second.lastRunTime = m_procTime;

        // trigger to update pin contents
        m_toUpdate = true;
    }

    /** @note:  colony runtimes
	 *
	 * type		# pins	   runtime in us
	 * empty  	   0         45 - 100
	 * basic	 < 10       266 - 1174
	 * prod		10 - 20
	 * adv		20 - 40
	 * max		  40+
         *  Update completed in 1273.000us with 10 links, 10 pins, 6 plants, and 13 routes (s:13, d:13)
         *
	 */
    _log(COLONY__INFO, "Colony::Update() - Update completed in %.3fus with %lu links, %lu pins, %lu plants, and %lu routes (s:%lu, d:%lu) ", \
                    GetTimeUSeconds() - profileStartTime, ccPin->links.size(), ccPin->pins.size(), ccPin->plants.size(), ccPin->routes.size(), \
                    m_srcRoutes.size(), m_destRoutes.size());

    // profile timer for the colony updates
    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::colony, GetTimeUSeconds() - profileStartTime);
}

void Colony::ProcessECUs(bool& updateTimes) {
	if (ccPin->ecus.empty)
		return;
	
    uint16 cycles(0);
    uint32 amount(0);
    double delta(0.0), divisor(0.0);
    // commodity typeID, qty
    std::map<uint16, uint32>::iterator typeItr;		// typeID, qty (from route info)
    std::map<uint32, PI_Pin>::iterator ecuItr;	    // pinID, data
    std::map<uint32, PI_Pin>::iterator destItr;     // pinID, data
    for (auto &cur : ccPin->ecus) {
		// this should never be invalid
        ecuItr = ccPin->pins.find(cur.first);
        if (ecuItr->second.state < PI::Pin::State::Active) {
            if (is_log_enabled(COLONY__WARNING))
                _log(COLONY__WARNING, "Colony::ProcessECUs(%u) - Inactive", ecuItr->first);
            continue;
        }

        /** @todo  as i dont have data on planet resources, and am not tracking depletion, extraction qtys used here are
         * sent from the client during 'survey program' installation, and do not simulate the diminishing returns as shown in
         * the survey program. testing diminishing returns @ 95% (config value) and saved in route.qty
         * because of this, the values used here (and all subsequent processes) will be more than shown in client.
         */
        /** @note this is a simple process, as it only provides raw mats, simulating extraction from planet and
         *  shipped to storage or directly to plant for processing.
         * however, in the case of shipping directly to plant, we will have to store the mats in the plant queue
         * and wait for the ProcessPlants() call to use them, as this will avoid over-complicating things,
         * but it could get messy later....
         */

        // first - get elapsed times and generate runs to simulate.  this avoids looping
        if (ecuItr->second.expiryTime < m_procTime) {
            // ecu program is complete.  determine cycles remaining from last runtime and continue
            delta = round(((double)ecuItr->second.expiryTime - ecuItr->second.lastRunTime) / EvE::Time::Hour);
            ecuItr->second.lastRunTime = ecuItr->second.expiryTime;
            ecuItr->second.expiryTime = 0;
            ecuItr->second.state = PI::Pin::State::Idle;
            if (delta < 1.0) {
                // warning...no run count
                if (is_log_enabled(COLONY__WARNING))
                    _log(COLONY__WARNING, "Colony::ProcessECUs(%u) - delta < 1", ecuItr->first);
                continue;
            }
        } else {
            delta = ((double)m_procTime - ecuItr->second.lastRunTime) / EvE::Time::Hour;
        }
        divisor = (double)ecuItr->second.cycleTime / EvE::Time::Hour;
        cycles = static_cast<uint16>(floor(delta / divisor));

        if (cycles < 1) {
            if (is_log_enabled(COLONY__WARNING))
                _log(COLONY__WARNING, "Colony::ProcessECUs(%u) - cycles < 1.", ecuItr->first);
            continue;
        } 
		
		if (cycles > ecuItr->second.cycleCount)
			cycles = ecuItr->second.cycleCount;

        if (is_log_enabled(COLONY__DEBUG))
            _log(COLONY__DEBUG, "Colony::ProcessECUs(%u) - begin processing with %u of %u cycle%s (%0.3f / %0.3f)", \
                    ecuItr->first, cycles, ecuItr->second.cycleCount, ecuItr->second.cycleCount > 1 ? "s":"", delta, divisor);

        // second - see if this ecu has a route and move contents per route.  this will simulate xfer of raw matls from heads to storage
        auto routeItr = m_srcRoutes.equal_range(ecuItr->first);     // this ecu is route origin
        for (auto it = routeItr.first; it != routeItr.second; ++it) {
            //  get total matl xferd
            amount = it->second.commodityQuantity * cycles;

            // third - update destination contents per route movement as noted above (ECU does not store matls - nothing to deduct from)
            destItr = ccPin->pins.find(it->second.destItrID);
            typeItr = destItr->second.contents.find(it->second.commodityTypeID);
            if (typeItr != destItr->second.contents.end()) {
                // add to existing stack
                typeItr->second += amount;
            } else {
                // create new stack
                destItr->second.contents[it->second.commodityTypeID] = amount;
            }

			// trigger contents update
			destItr->second.update = true;

            if (is_log_enabled(COLONY__DEBUG))
                _log(COLONY__DEBUG, "Colony::ProcessECUs(%u) - Dest: %s(%u) updated with %u %s(%u).", \
                        ecuItr->first, sPIDataMgr.GetPinName(it->second.destItrID), it->second.destItrID, amount, \
                        sPIDataMgr.GetProductName(it->second.commodityTypeID), it->second.commodityTypeID);

			if (destItr->second.isPlant) {
				// routing is straight to plant.  trigger input
				std::map<uint32, PI_Plant>::iterator plantItr = ccPin->plants.find(it->second);
                // i dunno which is first...will have to look in client code again to determine which to use
				plantItr->second.hasReceivedInputs = true;
                plantItr->second.receivedInputsLastCycle = true;
			}
			
        	// trigger to update pin contents
       		m_toUpdate = true;
        }

        if (is_log_enabled(COLONY__DEBUG))
            _log(COLONY__DEBUG, "Colony::ProcessECUs(%u) - Processing complete.", ecuItr->first);

        // fourth - update pin runtime and set flag to trigger contents updates.
        ecuItr->second.lastRunTime += (ecuItr->second.cycleTime * cycles);
        updateTimes = true;
    }
}

void Colony::ProcessPlants(bool& updateTimes) {
    if (m_plantMap.empty())
        return; // nothing to do...

    /** @note  generally-accepted PI design has plant input/output from/to storage (spaceport or silo)
     * for input buffers and possibily feeding multiple plants.
	 * this design is arranged as shown below...
     * silo->plant(s)->silo->plant(s)->silo
	 *
     * however, there may be cases where the colony is restricted or other design constraints limit routing and
     * plants must be linked together, where the output of one provides the direct input of the next, as shown below...
     * silo->plant(s)->plant(s)->plant(s)->silo
     * with plants as needed for production requirements of the colony.
     *
     * this will need to check for and be able to process both cases, and could be somewhat complicated.
     *
     * plants will have to be processed in product order from p1 to p4 to provide input for downstream plants
     * this WILL have to loop for each product cycle to correctly set inputs and outputs for each plant, and provide
     * positive material control (and be more realistic) per run.   
	 * each pLevel will run multiple cycles based on run times within it's loop
     */

    uint8 curCycle(m_pLevel);
    uint16 tempCycles(0);
    int32 cycles(0), cycles2(0), amount(0), divisor(0), delta(0);
    std::map<uint32, PI_Pin>::iterator srcItr;		// either plant or storage {itemID, data}
    std::map<uint32, PI_Pin>::iterator destItr;		// either plant or storage {itemID, data}
    std::map<uint32, PI_Pin>::iterator plantPinItr;	// common pin data for plant {itemID, data}
    std::map<uint16, uint32>::iterator itemItr;	    // routed item [typeID, qty}
	std::map<uint32, PI_Plant>::iterator plantItr;  // plant-specific data  {itemID, data}
    _log(COLONY__INFO, "Colony::ProcessPlants() - Begin Plant Processing.  m_procTime: %lli", m_procTime);

    // can this loop be split into smaller calls?  (like warp in destiny)
	//    ...maybe, but will take some thought and doing to make it work right
    while (curCycle < 5) {
        if (is_log_enabled(COLONY__DEBUG))
            _log(COLONY__DEBUG, "Colony::ProcessPlants() - Begin Process loop for pLevel %u.", curCycle);

        // plants must be processed in order to correctly consume inputs, make products and send outputs to downstream recipients.
        auto cycleItr = m_plantMap.equal_range(curCycle);
        for (auto it = cycleItr.first; it != cycleItr.second; ++it) {
            _log(COLONY__INFO, "Colony::ProcessPlants() - Begin Processing for %s(%u)", sPIDataMgr.GetPinName(it->second), it->second);

            // first, find plant pin in plant map
            plantItr = ccPin->plants.find(it->second);
			plantPinItr = ccPin->pins.find(plantItr->first);
            if ((plantItr == ccPin->plants.end()) or (plantPinItr == ccPin->pins.end())) {
				// this should never hit...
                _log(COLONY__ERROR, "Colony::ProcessPlants() - Plant not found in [plants/pins] map");
                it = m_plantMap.erase(it);
                continue;
            }
            // plant pin found.  begin basic data integrity  checks

			// verify plant's not idle
            if ((plantPinItr->second.state == PI::Pin::State::Idle)
			or (plantPinItr->second.schematicID == 0))
                continue;

            // second, check processing times for active plants
            delta = static_cast<int32>(floor((double)(m_procTime - plantPinItr->second.lastRunTime)  / EvE::Time::Minute));
            divisor = static_cast<int32>(floor((double)plantPinItr->second.cycleTime / EvE::Time::Minute));
            if (divisor < 1) {
                if (is_log_enabled(COLONY__WARNING))
                    _log(COLONY__WARNING, "Colony::ProcessPlants() - divisor invalid (%i).  set plant to idle and continue.", divisor);
                plantPinItr->second.state = PI::Pin::State::Idle;
                continue;
            }

            if (delta < divisor) {
                if (is_log_enabled(COLONY__DEBUG))
                    _log(COLONY__DEBUG, "Colony::ProcessPlants() - Plant active but cycle incomplete (%i < %i).", \
                            delta, divisor);
                continue;
            }

            // we are doing 'batch' cycles here.  get #cycles completed based on proc times
            cycles = delta / divisor;
            if (cycles < 1) 
                continue;
            
            if (is_log_enabled(COLONY__DEBUG))
                _log(COLONY__DEBUG, "Colony::ProcessPlants() - current cycle count is %i (%i / %i).", \
                            cycles, delta, divisor);

            // basic data checks done.

            // third, check supply routes for available matls and xfer to this plant
            _log(COLONY__INFO, "Colony::ProcessPlants() - Begin Input Route loop for Plant.");
            auto destRouteItr = m_destRoutes.equal_range(plantItr->first);
            for (auto it = destRouteItr.first; it != destRouteItr.second; ++it) {
                // this route supplies current plant with input matls.
                srcItr = ccPin->pins.find(it->second.srcItrID);
                if (srcItr == ccPin->pins.end()) {
                    // route source pin not found.    should never hit
                    _log(COLONY__ERROR, "Colony::ProcessPlants() - Source Pin %u not found in ccPin.pins map", it->second.srcItrID);
                    it = m_destRoutes.erase(it);
                    plantItr->second.hasReceivedInputs = false;
                    continue;
                }
				
				// is source plant or ecu?
				if (srcItr.second.isECU or srcItr.second.isPlant) {
					//yep.  nothing to do here...mat'l/qty (supposedly) already routed
                    _log(COLONY__ERROR, "Colony::ProcessPlants() - Source %s is Plant or ECU.  Skipping this input routing loop.", sPIDataMgr.GetPinName(srcItr->first));
                    continue;
				}
				
                // source found as storage.  search for routed commodity and continue
                itemItr = srcItr->second.contents.find(it->second.commodityTypeID);
                if (itemItr == srcItr->second.contents.end()) {
                    if (is_log_enabled(COLONY__WARNING))
                        _log(COLONY__WARNING, "Colony::ProcessPlants() - Routed Commodity %s (%u) not found in Source Inventory.", \
                                sPIDataMgr.GetProductName(it->second.commodityTypeID), it->second.commodityTypeID);
                    plantItr->second.hasReceivedInputs = false;
                    break;
                }

                // check dest before moving anything
                destItr = ccPin->pins.find(it->second.destItrID);
                if (destItr == ccPin->pins.end()) {
					// should never hit
                    _log(COLONY__ERROR, "Colony::ProcessPlants() - Pin %u not found in ccPin.pins map for this plant.", \
                            it->second.destItrID);
                    it = m_destRoutes.erase(it);
                    continue;
                }
				
				// dest should be current plant  (searched by destID using currentPlantID)
				if (destItr->first != plantItr->first) {
					// should never hit
                    _log(COLONY__ERROR, "Colony::ProcessPlants() - route %u, dest %s(%u) != current plant %s(%u).  Breaking out.", \
                            it->first, sPIDataMgr.GetPinName(it->second.destItrID), it->second.destItrID, \
							sPIDataMgr.GetPinName(plantItr->first), plantItr->first);
					// what do we need to do here?  change current plant?
					break;
				}

                // destination valid; remove contents from storage pin
                amount = it->second.commodityQuantity * cycles;
                if (itemItr->second >= amount) {
                    itemItr->second -= amount;
                } else {
                    // not enough for all cycles
                    amount = itemItr->second;
                    srcItr->second.contents.erase(itemItr);
                }
				
				// trigger contents update
				srcItr->second.update = true;
				
                if (is_log_enabled(COLONY__DEBUG))
                    _log(COLONY__DEBUG, "Colony::ProcessPlants() - Removed %i %s(%u) from %s(%u).", \
                            amount, sPIDataMgr.GetProductName(it->second.commodityTypeID), it->second.commodityTypeID, \
							sPIDataMgr.GetPinName(srcItr->first), srcItr->first);

                // add contents to dest plant's pin
                itemItr = destItr->second.contents.find(it->second.commodityTypeID);
                if (itemItr != destItr->second.contents.end()) {
                    itemItr->second += amount;
                } else {
                    destItr->second.contents[it->second.commodityTypeID] = amount;
                }
				
				// trigger contents update
				destItr->second.update = true;
				
        		// trigger to update pin contents
       			m_toUpdate = true;
				
                if (is_log_enabled(COLONY__DEBUG))
                    _log(COLONY__DEBUG, "Colony::ProcessPlants() - Added %i %s(%u) to %s(%u).", \
                            amount, sPIDataMgr.GetProductName(it->second.commodityTypeID), it->second.commodityTypeID, \
							sPIDataMgr.GetPinName(destItr->first), destItr->first);
							
                // we have received a material from this route.  check for plant
                if (destItr->second.isProcess) {
                    //enable check for all required materials in this Schematic for this plant
                    plantItr->second.hasReceivedInputs = true;
                }

            }

            // verify plant pin
            destItr = ccPin->pins.find(plantItr->first);
            if (destItr == ccPin->pins.end()) {
				// should never hit
                _log(COLONY__ERROR, "Colony::ProcessPlants() - Dest %u not found in ccPin.pins map", plantItr->first);
                m_destRoutes.erase(destItr->first);
                break;
            }

            // fourth, process input material requirements
            cycles2 = 0;
            _log(COLONY__INFO, "Colony::ProcessPlants() - %s Input Check loop for Plant %u.", \
                    plantItr->second.hasReceivedInputs ? "Begin" : "Skipping", plantItr->first);

            plantItr->second.receivedInputsLastCycle = false;

            if (plantItr->second.hasReceivedInputs) {
                /*  if plant has received mats from routing (above), then check here for required qtys per Schematic.
                 *     input data is found in plantItr->second.data.inputs map (std::map<uint16, uint16> {typeID, qty})
                 *
                 *  if required mats are not present, set receivedInputsLastCycle=false, which will deny processing
                 *      and subsequent routing for this plant.
                 *
                 *  if all required qtys have been received, proceed with the following:
                 *   - remove mats from pin.contents
                 *   - set receivedInputsLastCycle=true
                 */
                if (plantItr->second.data.inputs.empty()) {
                    _log(COLONY__WARNING, "Colony::ProcessPlants() - Empty input map");
                    // skip further processing
                    plantItr->second.state = PI::Pin::State::Idle;
                    plantItr->second.hasReceivedInputs = false;
                    continue;
                }

                tempCycles = cycles;
                for (auto &mats : plantItr->second.data.inputs) {
                    // loop thru Schematic inputs to verify all required mats are present
                    itemItr = destItr->second.contents.find(mats.first);
                    if (itemItr == destItr->second.contents.end()) {
                        if (is_log_enabled(COLONY__DEBUG))
                            _log(COLONY__DEBUG, "Colony::ProcessPlants() - %s (%u) not found in Plant Inventory.  Break out of loop.", \
                                    sPIDataMgr.GetProductName(mats.first), mats.first);
                        // this required material was not found in plant inventory.  skip further processing
                        plantItr->second.hasReceivedInputs = false;
                        break;
                    }
                    if (itemItr->second >= (mats.second * cycles)) {
                        itemItr->second -= (mats.second * cycles);
                        plantItr->second.receivedInputsLastCycle = true;
                    } else {
                        // this required material was not sufficient quantity for (num cycles) runs.
                        // determine how many cycles we can run with current material quantity
                        if (is_log_enabled(COLONY__DEBUG))
                            _log(COLONY__DEBUG, "Colony::ProcessPlants() - Not enough %s(%u) for %i cycles.  Need %u, Have %u", \
                                    sPIDataMgr.GetProductName(mats.first), mats.first, cycles, mats.second * cycles, itemItr->second);
                        cycles2 = itemItr->second / mats.second;
                        if (cycles2 > 0) {
                            itemItr->second -= mats.second * cycles2;
                            plantItr->second.receivedInputsLastCycle = true;
                            if (is_log_enabled(COLONY__DEBUG))
                                _log(COLONY__DEBUG, "Colony::ProcessPlants() - Have enough material for %i cycles.", cycles2);
                        } else {
                            plantItr->second.hasReceivedInputs = false;
                            break;
                        }
                    }
					
					// trigger contents update
					destItr->second.update = true;
				
					//TODO:  check for qtys after inputs from multiple sources...
                    // set temp variable with minimum cycle count
                    if (tempCycles > cycles2)
                        tempCycles = cycles2;
                    cycles2 = 0;
                }
				
        		// trigger to update pin contents
        		m_toUpdate = true;
				
                // we have enough mat'l for at least one process.  set cycles based on material in inventory.
                if (cycles > tempCycles)
                    cycles = tempCycles;
            } else {
                // we have not received inputs last cycle
                plantItr->second.hasReceivedInputs = false;
                plantItr->second.receivedInputsLastCycle = false;
                break;
            }

            // at this point, we have looped thru all required mats and set plant variables accordingly.

            // fifth, process manufacturing cycle and move finished product per route
            _log(COLONY__INFO, "Colony::ProcessPlants() - %s Output Routing loop for %s(%u).", \
                    cycles > 0 ? "Begin" : "Skipping", sPIDataMgr.GetPinName(plantItr->first), plantItr->first);
					
            if (cycles and plantItr->second.receivedInputsLastCycle) {
                auto srcRouteItr = m_srcRoutes.equal_range(plantItr->first);
                for (auto it = srcRouteItr.first; it != srcRouteItr.second; ++it) {
                    // get destination pin and update qty there for this round
                    destItr = ccPin->pins.find(it->second.destItrID);
                    if (destItr == ccPin->pins.end()) {
                        _log(COLONY__ERROR, "Colony::ProcessPlants() - Dest %u not found in ccPin.pins map", it->second.destItrID);
                        m_srcRoutes.erase(it);
                        break;
                    }
                    // contents are stored in each pin.  PI_Pin.contents(std::map<uint16, uint32> typeID, qty)
                    // we have plant cycles for this loop, so multiply output by cycles to get a total to simulate the "active" plant
                    amount = it->second.commodityQuantity * cycles;

                    // pin item has capy attr. the above isnt needed.  use attributes!!
                    itemItr = destItr->second.contents.find(it->second.commodityTypeID);
                    if (itemItr != destItr->second.contents.end()) {
                        // add to existing
                        itemItr->second += amount;
                    } else {
                        // create new stack
                        destItr->second.contents[it->second.commodityTypeID] = amount;
                    }
					
					// trigger contents update
					destItr->second.update = true;

                    if (is_log_enabled(COLONY__DEBUG))
                        _log(COLONY__DEBUG, "Colony::ProcessPlants() - Added %u %s (%u) to Dest %u.", \
                                amount, sPIDataMgr.GetProductName(it->second.commodityTypeID), \
                                it->second.commodityTypeID, it->second.destItrID);

                    if (destItr->second.isStorage) {
                        //  if dest cant hold entire xfer qty, drop remainder in current pin contents (as opposed to loss)
                        _log(COLONY__DEBUG, "Colony::ProcessPlants() - Dest is storage");
                    } else if (destItr->second.isProcess) {
                        // find dest's plant data
                        //  the destination plant will have a P level of curCycle+1, and will process on next iteration
                        auto destPlantItr = ccPin->plants.find(destItr->first);
                        if (destPlantItr == ccPin->plants.end()) {
                            _log(COLONY__ERROR, "Colony::ProcessPlants() - Dest %u not found in ccPin.plants map", destItr->first);
                            m_srcRoutes.erase(it);
                            break;
                        }
                        //then set dest's hasReceivedInputs to true for subsequent processing
                        destPlantItr->second.hasReceivedInputs = true;
                    }

                    // this plant has used all inputs.  set received to false to begin next cycle of mat'l xfer
                    plantItr->second.hasReceivedInputs = false;
                }
					
				// trigger contents update
				destItr->second.update = true;
                // update last run time based on current process cycles
                destItr->second.lastRunTime += plantItr->second.cycleTime * cycles;
				// update colony process time (and pin contents)
        		updateTimes = true;
                // if there are materials left, verify qty and move excess back to previous storage, if applicable
            } else {
                // not enough mat'l for one cycle.
                plantItr->second.hasReceivedInputs = false;
                plantItr->second.receivedInputsLastCycle = false;
            }
			
            _log(COLONY__INFO, "Colony::ProcessPlants() -  %s(%u): Processing Complete", sPIDataMgr.GetPinName(it->second), it->second);
        }
		
        if (is_log_enabled(COLONY__DEBUG))
            _log(COLONY__DEBUG, "Colony::ProcessPlants() - Process loop complete for pLevel %u.", curCycle);

        // this pLevel cycle complete.  increment and begin next loop
        ++curCycle;
    }
}
