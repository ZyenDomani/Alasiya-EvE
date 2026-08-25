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
        added real spherical harmonics  2aug26
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
ccData(new PI_CCData()),
m_client(pClient),
pPinList(new PyList()),
m_colonyTimer(0),
m_active(false),
m_loaded(false),
m_newHead(false),
m_toUpdate(false),
m_pLevel(0),
m_pg(0),	// not used
m_cpu(0),	// not used
m_colonyID(0),
m_procTime(0)	// process check.  init to zero and stores last proc time, which is lastRunTime in command center
{
    assert(m_pSE != nullptr);

    _log(COLONY__DEBUG, "Colony::Colony() c'tor called for %s(%u) by %s(%u)", pSE->GetName(), pSE->GetID(), pClient->GetName(), pClient->GetCharacterID());
}

Colony::~Colony()
{
    SafeDelete(ccData);
    // this isnt right....gotta look into it more
    //PySafeDecRef(pPinList);
}

// run initial update then client processes after colony data is sent
void Colony::Init()
{
    if (m_loaded)
        return;

    // check for and load colony if the char has one on this planet
    if (m_db.LoadColony(m_client->GetCharacterID(), m_pSE->GetID(), ccData)) {
        m_colonyID = ccData->colonyID;
        Load();
    }

    if (m_loaded)
        Update();
}

void Colony::Shutdown()
{
    Update();
    Save();
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

    m_db.LoadPins(m_colonyID, ccData->pins);
    m_db.LoadLinks(m_colonyID, ccData->links);
    m_db.LoadRoutes(m_colonyID, ccData->routes);

    LoadPlants();

    for (auto &cur : ccData->routes) {
        m_srcRoutes.emplace(cur.second.srcPinID, cur.second);
        m_destRoutes.emplace(cur.second.destPinID, cur.second);
    }

    m_loaded = (!ccData->pins.empty());
}

void Colony::Save() {
    m_db.SavePins(ccData);
    m_db.SaveLinks(ccData);
    m_db.SaveRoutes(ccData);
    m_db.SaveAllContents(ccData);
    m_db.UpdatePlanetPins(m_colonyID, ccData->pins.size());
}

// called by PlanetSE::Process() for loaded colony.
void Colony::Process() {
    if (m_colonyTimer.Check()) { //  this will process colony data every 5 mins. (typical cycle time is 30m)
        if (ccData->pins.empty()) {
            m_colonyTimer.Disable();
            return;
        }

        Update();
    }

    if (m_toUpdate) {
        // this will loop thru updated pins to send to client for refresh
        for (auto &cur : ccData->pins) {
            // has this pin's data been updated since last save?
            if (cur.second.update) {
                pPinList->AddItemInt(cur.first);
            }
        }

        m_toUpdate = false;

        SendUpdate();
    }
}

void Colony::SendUpdate() {
    if (!pPinList->empty()) {
        PyTuple* tuple = new PyTuple(1);
        PyIncRef(pPinList);
        tuple->SetItem(0, pPinList);
        //may also need OnItemChange with this
        m_client->SendNotification("OnRefreshPins", "clientID", &tuple, false);
        pPinList->clear();
    }
}

uint32 Colony::GetOwner() {
    return m_client->GetCharacterID();
}

void Colony::LoadPlants() {
    for (auto &cur: ccData->pins) {
        // set proc time on load
        if (cur.second.isCommandCenter) {
            m_procTime = cur.second.lastRunTime;
            continue;
        }

        if (cur.second.isECU) {
            DBQueryResult res;
            DBResultRow row;
            PI::ECU ecu = PI::ECU();

            m_db.LoadECU(cur.first, res);
            if (res.GetRow(row)) {
                ecu.expiryTime              = row.GetInt64(0);
                ecu.headRadius              = row.GetDouble(1);
                ecu.programType             = row.GetUInt16(2);
                ecu.cycleCount              = row.GetUInt16(3);
                ecu.headTypeID              = sPIDataMgr.GetHeadType(sItemFactory.GetItemRef(cur.first)->typeID(), row.GetUInt16(2));
            }

            m_db.LoadHeads(cur.first, ecu.heads);
            ccData->ecus[cur.first] = ecu;
            continue;
        }

        // load plants in mem object
        if (cur.second.isProcess) {
            PI::Plant plant                  = PI::Plant();
            plant.data                      = PI::Schematic();
            plant.hasReceivedInputs         = false;
            plant.receivedInputsLastCycle   = false;

            if (cur.second.schematicID) {
                sPIDataMgr.GetSchematicData(cur.second.schematicID, plant.data);
                cur.second.cycleTime    = plant.data.cycleTime * EvE::Time::Second; // data.cycleTime is in seconds
                cur.second.qtyPerCycle  = plant.data.outputQty;     // this is not saved
                plant.pLevel        	= sPIDataMgr.GetProductLevel(plant.data.outputType);
                // i am ordering plant processing by output's Plevel
                if (m_pLevel < 1)
                    m_pLevel = plant.pLevel;
                if (plant.pLevel < m_pLevel)
                    m_pLevel = plant.pLevel;
                // only inserting active plants in map
                m_plantMap.emplace(plant.pLevel, cur.first);
            }

            ccData->plants[cur.first] = plant;
        }
    }

    // set process timer
    if (!m_colonyTimer.Enabled())
        m_colonyTimer.Start(sConfig.rates.ColonyTimer * EvE::Timer::Minute);
}

void Colony::AbandonColony()
{
    /** @todo  go thru entire pinMap and delete each itemRef to remove pin/link contents from db. */
    for (auto &cur : ccData->pins) {
        m_db.RemovePin(cur.first);
        m_db.RemoveContents(cur.first);
        sItemFactory.RemoveItem(cur.first);
    }
    for (auto &cur : ccData->links) {
        m_db.RemovePin(cur.first);
        sItemFactory.RemoveItem(cur.first);
    }
    InventoryItemRef iRef = sItemFactory.GetItemRef(m_colonyID);
    iRef->Delete();
    m_db.DeleteColony(m_colonyID, m_pSE->GetID(), m_client->GetCharacterID());
    SafeDelete(ccData);
    ccData = new PI_CCData();
    m_colonyID = 0;
    m_colonyTimer.Disable();
}

void Colony::CreateCommandPin(uint32 itemID, uint16 typeID, double latitude, double longitude) {
    m_colonyID = itemID;
    ccData->colonyID = itemID;
    ccData->level = PI::Pin::Level0;
    CreatePin(EVEDB::invGroups::Command_Centers, itemID, typeID, latitude, longitude);
    m_db.SaveCommandCenter(itemID, m_client->GetCharacterID(), m_pSE->GetID(), typeID);
    m_db.AddPlanetForChar(m_pSE->SystemMgr()->GetID(), m_pSE->GetID(), m_client->GetCharacterID(), m_colonyID, m_pSE->GetTypeID());
}

void Colony::CreatePin(uint32 groupID, uint32 pinID, uint16 typeID, double latitude, double longitude) {
    using namespace EVEDB::invGroups;
    PI::PinData pin = PI::PinData();
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
        ItemData data(typeID, m_client->GetCharacterID(), m_pSE->GetID(), flagAutoFit);
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
    pin.level = PI::Pin::Level0;
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
            ccData->plants[iRef->itemID()] = PI::Plant();
        } break;
        case Extractor_Control_Units: { // 1063
            pin.isECU = true;
            pin.qtyPerCycle = iRef->GetAttribute(AttrPinExtractionQuantity).get_int();
            ccData->ecus[iRef->itemID()] = PI::ECU();
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

    m_db.CreatePin(ccData->colonyID, iRef->itemID(), pin);
    ccData->pins[iRef->itemID()] = pin;

    // save map of tempID to itemID - this handles the stacked-calls from client to use real itemIDs
    if (groupID != Command_Centers)
        tempPinIDs.emplace(pinID, iRef->itemID());

    if (is_log_enabled(COLONY__INFO))
        _log(COLONY__INFO, "Colony::CreatePin() - Created pin for %s(%u)", iRef->name(), iRef->itemID());
}

void Colony::CreateLink(uint32 src, uint32 dest, uint16 level) {
    if (IsTempPinID(src) and (tempPinIDs.size() > 0)) {
        std::unordered_map<uint32, uint32>::iterator itr = tempPinIDs.find(src);
        if (itr != tempPinIDs.end())
            src = itr->second;
    }
    if (IsTempPinID(dest) and (tempPinIDs.size() > 0)) {
        std::unordered_map<uint32, uint32>::iterator itr = tempPinIDs.find(dest);
        if (itr != tempPinIDs.end())
            dest = itr->second;
    }
    ItemData data(2280, m_client->GetCharacterID(), locTemp, flagAutoFit, 1);
    InventoryItemRef iRef = sItemFactory.SpawnItem(data);
    iRef->Move(m_pSE->GetID(), flagPlanetSurface, true);
    iRef->SaveItem();

    PI::Link link = PI::Link();
        link.state = PI::Pin::State::Idle;
        link.level = level;
        link.endpoint1 = src;
        link.endpoint2 = dest;
        link.typeID = 2280; // Only link type in the game.
    ccData->links[iRef->itemID()] = link;

    m_db.SaveLinks(ccData);
    if (is_log_enabled(COLONY__INFO))
        _log(COLONY__INFO, "Colony::CreateLink() - Created link - id:%u,  from %s to %s, level:%u", \
            iRef->itemID(), sPIDataMgr.GetPinName(src), sPIDataMgr.GetPinName(dest), level);
}

void Colony::CreateRoute(int16 routeID, uint16 typeID, int32 qty, PyList* path) {
    // routeID is sent as tempID like pins.
    std::list<uint32> list1;
    for (uint16 i = 0; i < path->size(); ++i) {
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
        std::unordered_map<uint32, uint32>::iterator itr;
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
        list1 = list2;
    }

    PI::Route route = PI::Route();
        route.state = PI::Pin::State::Idle;
        route.priority = PI::RoutePriority::Norm;
        route.commodityTypeID = typeID;
        route.commodityQuantity = qty;
        route.srcPinID = list1.front();
        route.destPinID = list1.back();
        route.path = list1;

    std::unordered_map<uint32, PI::PinData>::iterator srcItr = ccData->pins.find(route.srcPinID);
    if (srcItr == ccData->pins.end()) {
         // source not found.
        throw UserError("RouteFailedValidationPinDoesNotExist");
    }
    std::unordered_map<uint32, PI::PinData>::iterator destItr = ccData->pins.find(route.destPinID);
    if (destItr == ccData->pins.end()) {
        // destination not found.
        throw UserError("RouteFailedValidationPinDoesNotExist");
    }

    routeID = m_db.SaveRoute(m_colonyID, route);
    ccData->routes.emplace(routeID, route);

    m_srcRoutes.emplace(route.srcPinID, route);
    m_destRoutes.emplace(route.destPinID, route);

    if (is_log_enabled(COLONY__INFO))
        _log(COLONY__INFO, "Colony::CreateRoute() - Created route id %u for %i of typeID %u, making %u hops.", routeID, qty, typeID, (uint32)path->size() - 1);

    // if source isnt storage, it probably dont have this material
    if (!srcItr->second.isStorage)
        return;

    // do we really wanna move contents? live doesnt...but we do.
    std::map<uint16, uint32>::iterator itemItr = srcItr->second.contents.find(route.commodityTypeID);
    if (itemItr == srcItr->second.contents.end()) {
        // this material wasnt found in source container....cant move what we aint got..
        //throw UserError("RouteFailedValidationExpeditedSourceLacksCommodity") \
                    .AddTypeID(typeID);
        return;
    }

    int32 amount = route.commodityQuantity;
    // remove contents from storage pin
    if (itemItr->second > amount) {
        itemItr->second -= amount;
    } else {
        amount = itemItr->second;
        srcItr->second.contents.erase(itemItr);
    }

    // add contents to dest pin if we have any
    itemItr = destItr->second.contents.find(route.commodityTypeID);
    if (itemItr != destItr->second.contents.end()) {
        // should we verify pin capy?  probably so...eventually
        itemItr->second += amount;
    } else {
        destItr->second.contents.emplace_hint(itemItr, route.commodityTypeID, amount);
    }
}

void Colony::UpgradeCommandCenter(uint32 pinID, int8 level) {
    ccData->level = level;
    std::unordered_map<uint32, PI::PinData>::iterator itr = ccData->pins.find(pinID);

    // do we need to check skill here or is it done in client?
    //{'FullPath': u'UI/Messages', 'messageID': 256742, 'label': u'CantUpgradeCommandCenterSkillRequiredBody'}(u'You cannot upgrade the command center. You need {skillName} at {requestedLevel} but only have it at {currentLevel}.', None, {u'{requestedLevel}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'requestedLevel'}, u'{skillName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'skillName'}, u'{currentLevel}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'currentLevel'}})

    if (itr != ccData->pins.end()) {
        if (itr->second.level > 4) {
            throw UserError("CannotUpgradeLinkAlreadyMaxed")
            .AddFormatValue("typeName", new PyString(sDataMgr.GetTypeName(itr->second.typeID)));
        }
        itr->second.level = level;
        m_db.SaveCCLevel(pinID, level);
        if (is_log_enabled(COLONY__INFO))
            _log(COLONY__INFO, "Colony::UpgradeCommandCenter() - Upgraded Command Center %u to level:%u", pinID, level);
    } else {
        _log(COLONY__ERROR, "Colony::UpgradeCommandCenter() - pinID %u not found in ccData.pins map", pinID);
    }
}

void Colony::UpgradeLink(uint32 src, uint32 dest, uint8 level) {
    auto itr = std::find_if(ccData->links.begin(), ccData->links.end(),
        [src, dest](const std::pair<uint32, PI::Link>& pair) {
            return ((pair.second.endpoint1 == src) and (pair.second.endpoint2 == dest));
        }
    );

    if (itr != ccData->links.end()) {
        if (itr->second.level > PI::Pin::Level9) {
            throw UserError("CannotUpgradeLinkAlreadyMaxed")
            .AddFormatValue("typeName", new PyString(sDataMgr.GetTypeName(itr->second.typeID)));
        }
        itr->second.level = level;
        m_db.SaveLinks(ccData);
    }
}

void Colony::RemovePin(uint32 pinID)
{
    uint8 linkCount = 0, routeCount = 0, pathCount = 0;
    // check for and remove links to this pinID
    std::unordered_map<uint32, PI::Link>::iterator linkItr = ccData->links.begin();
    while (linkItr != ccData->links.end()) {
        if (linkItr->second.endpoint1 == pinID) {
            m_db.RemoveLink(linkItr->first);
            linkItr = ccData->links.erase(linkItr);
            ++linkCount;
            continue;
        }
        if (linkItr->second.endpoint2 == pinID) {
            m_db.RemoveLink(linkItr->first);
            linkItr = ccData->links.erase(linkItr);
            ++linkCount;
            continue;
        }
        ++linkItr;
    }

    // check for routes to, from, or thru this pin
    std::unordered_map<uint16, PI::Route>::iterator routeItr = ccData->routes.begin();
    while (routeItr != ccData->routes.end()) {
        if (routeItr->second.srcPinID == pinID) {
            m_db.RemoveRoute(routeItr->first);
            routeItr = ccData->routes.erase(routeItr);
            ++routeCount;
            continue;
        }
        if (routeItr->second.destPinID == pinID) {
            m_db.RemoveRoute(routeItr->first);
            routeItr = ccData->routes.erase(routeItr);
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

    // find this pin in map, then process delete accordingly
    std::unordered_map<uint32, PI::PinData>::iterator pinItr = ccData->pins.find(pinID);
    if (pinItr != ccData->pins.end()) {
        if (pinItr->second.isProcess) {
            std::unordered_map<uint32, PI::Plant>::iterator plantItr = ccData->plants.find(pinID);
            auto cycleItr = m_plantMap.equal_range(plantItr->second.pLevel);
            for (auto it = cycleItr.first; it != cycleItr.second; ++it) {
                if (it->second == pinID)
                    m_plantMap.erase(it);
            }
            ccData->plants.erase(plantItr);
        } else if (pinItr->second.isECU) {
            ccData->ecus.erase(pinID);
        }
        ccData->pins.erase(pinItr);
    }

    m_db.RemovePin(pinID);
    m_db.SaveLinks(ccData);
    m_db.SaveRoutes(ccData);
    m_db.UpdatePlanetPins(m_colonyID, ccData->pins.size());

    if (is_log_enabled(COLONY__INFO))
        _log(COLONY__INFO, "Colony::RemovePin() - Removed pin %u with %u routes and %u links.  Upset %u routes by removing this pin", \
                            pinID, routeCount, linkCount, pathCount);
}

void Colony::RemoveLink(uint32 src, uint32 dest) {
    std::unordered_map<uint32, PI::Link>::iterator linkItr = ccData->links.begin();
    for (; linkItr != ccData->links.end(); ++linkItr) {
        if (linkItr->second.endpoint1 == src) {
            if (linkItr->second.endpoint2 == dest) {
                if (is_log_enabled(COLONY__INFO))
                    _log(COLONY__INFO, "Colony::RemoveLink() - Removing linkID %u - src: %u, dest: %u", linkItr->first, src, dest);
                m_db.RemoveLink(linkItr->first);
                ccData->links.erase(linkItr);
                return;
            }
        }
    }
}

void Colony::RemoveRoute(uint16 routeID) {
    std::unordered_map<uint16, PI::Route>::iterator routeItr = ccData->routes.find(routeID);
    if (routeItr != ccData->routes.end()) {
        m_srcRoutes.erase(routeItr->second.srcPinID);
        m_destRoutes.erase(routeItr->second.destPinID);
    }
    ccData->routes.erase(routeID);
    m_db.RemoveRoute(routeID);

    if (is_log_enabled(COLONY__INFO))
        _log(COLONY__INFO, "Colony::RemoveRoute() - Removed route: %u", routeID);
}

void Colony::AddExtractorHead(uint32 ecuID, uint16 headID, double latitude, double longitude) {
    std::unordered_map<uint32, PI::ECU>::iterator ecuItr = ccData->ecus.find(ecuID);
    if (ecuItr == ccData->ecus.end()) {
        _log(COLONY__ERROR, "Colony::MoveExtractorHead() - ecuID %u not found in ccData.pins map", ecuID);
        return;
    }

    // When a player locks in their extraction layout, use the scanning range
    // to apply a permanent, hidden coordinate drift to their pins on the server
    // Level 5 = Perfect accuracy (0% offset)
    // Level 0 = Up to a 5% coordinate wobble error
    float errorMargin = (5 - m_client->GetChar()->GetSkillLevel(EvESkill::Planetology)) * 0.01f;
    /*
    if (errorMargin > 0.0f) {
        // Inject a slight, permanent position error to the pin coordinates
        // to simulate their character's "bad survey reading"
        latitude += MakeRandomFloat(-maxWobbleRadius, maxWobbleRadius) * EvE::Trig::RadiansInDegrees;
        longitude += MakeRandomFloat(-maxWobbleRadius, maxWobbleRadius) * EvE::Trig::RadiansInDegrees;
    } */

    m_newHead = true;
    tempECUs.push_back(ecuID);
    PI::Heads head = PI::Heads();
        head.typeID = ecuItr->second.programType;
        head.ecuPinID = ecuID;
        head.latitude = latitude;
        head.longitude = longitude;
    ecuItr->second.heads[headID] = head;
}

void Colony::MoveExtractorHead(uint32 ecuID, uint16 headID, double latitude, double longitude) {
    std::unordered_map<uint32, PI::ECU>::iterator ecuItr = ccData->ecus.find(ecuID);
    if (ecuItr == ccData->ecus.end()) {
        _log(COLONY__ERROR, "Colony::MoveExtractorHead() - ecuID %u not found in ccData.pins map", ecuID);
        return;
    }

    std::unordered_map<uint16, PI::Heads>::iterator headItr = ecuItr->second.heads.find(headID);
    if (headItr == ecuItr->second.heads.end()) {
        _log(COLONY__ERROR, "Colony::MoveExtractorHead() - headID %u not found in pin.heads map", headID);
        return;
    }

    float errorMargin = (5 - m_client->GetChar()->GetSkillLevel(EvESkill::Planetology)) * 0.01f;
    /*
    if (errorMargin > 0.0f) {
        // Inject a slight, permanent position error to the pin coordinates
        // to simulate their character's "bad survey reading"
        latitude += MakeRandomFloat(-maxWobbleRadius, maxWobbleRadius) * EvE::Trig::RadiansInDegrees;
        longitude += MakeRandomFloat(-maxWobbleRadius, maxWobbleRadius) * EvE::Trig::RadiansInDegrees;
    } */

    m_newHead = true;
    tempECUs.push_back(ecuID);
    // find head and update....
    headItr->second.latitude = latitude;
    headItr->second.longitude = longitude;
}

void Colony::KillExtractorHead(uint32 ecuID, uint16 headID) {
    std::unordered_map<uint32, PI::ECU>::iterator ecuItr = ccData->ecus.find(ecuID);
    if (ecuItr == ccData->ecus.end()) {
        _log(COLONY__ERROR, "Colony::KillExtractorHead() - ecuID %u not found in ccData.pins map", ecuID);
        return;
    }
    ecuItr->second.heads.erase(headID);
}

void Colony::SetSchematic(uint32 pinID, uint8 schematicID/*0*/) {
    if (IsTempPinID(pinID) and (tempPinIDs.size() > 0)) {
        std::unordered_map<uint32, uint32>::iterator itr = tempPinIDs.find(pinID);
        if (itr != tempPinIDs.end())
            pinID = itr->second;
    }

    std::unordered_map<uint32, PI::Plant>::iterator plantItr = ccData->plants.find(pinID);
    if (plantItr == ccData->plants.end()) {
        _log(COLONY__ERROR, "Colony::SetSchematic() - plantID %u not found in ccData.plants map", pinID);
        return;
    }

    std::unordered_map<uint32, PI::PinData>::iterator pinItr = ccData->pins.find(pinID);

    if (schematicID) {
        // install new schematic.  set lastRunTime to 0.  set installTime to now.   update
        sPIDataMgr.GetSchematicData(schematicID, plantItr->second.data);
        plantItr->second.pLevel                  = sPIDataMgr.GetProductLevel(plantItr->second.data.outputType);
        pinItr->second.cycleTime                 = plantItr->second.data.cycleTime * EvE::Time::Second;
        pinItr->second.qtyPerCycle               = plantItr->second.data.outputQty;
        pinItr->second.schematicID               = schematicID;
        pinItr->second.lastRunTime               = GetFileTimeNow();
        pinItr->second.installTime               = GetFileTimeNow();
        plantItr->second.hasReceivedInputs       = false;
        plantItr->second.receivedInputsLastCycle = false;

        if (m_pLevel < 1)
            m_pLevel = plantItr->second.pLevel;
        if (plantItr->second.pLevel < m_pLevel)
            m_pLevel = plantItr->second.pLevel;

        m_plantMap.emplace(plantItr->second.pLevel, pinItr->first);

        /* if we have available matl, update storage and plant
         *   this will take some doing...
         */

        pinItr->second.state = PI::Pin::State::Idle;

        // set process timer
        if (!m_colonyTimer.Enabled())
            m_colonyTimer.Start(sConfig.rates.ColonyTimer * EvE::Timer::Minute);

        if (is_log_enabled(COLONY__INFO))
            _log(COLONY__INFO, "Colony::SetSchematic() - Set Schematic %u in %s(%u)",
                schematicID, sPIDataMgr.GetPinName(pinID), pinID);
    } else {
        pinItr->second.state = PI::Pin::State::Disabled;
        // remove plant from map to avoid processing empty plant
        auto cycleItr = m_plantMap.equal_range(plantItr->second.pLevel);
        for (auto it = cycleItr.first; it != cycleItr.second; ++it) {
            if (it->second == pinID) {
                m_plantMap.erase(it);
                break;
            }
        }

        if (is_log_enabled(COLONY__INFO))
            _log(COLONY__INFO, "Colony::SetSchematic() - Cleared %s(%u)", sPIDataMgr.GetPinName(pinID), pinID);
    }
}

void Colony::InstallProgram(uint32 ecuID, uint16 typeID, double headRadius) {
    /*
     * 09:54:54 [PlanetCallDump]       [ 0]   [10]   [ 1]  Tuple: 3 elements
     * 09:54:54 [PlanetCallDump]       [ 0]   [10]   [ 1]   [ 0]    Integer: 140000565  ecuID
     * 09:54:54 [PlanetCallDump]       [ 0]   [10]   [ 1]   [ 1]    Integer: 2272       typeID
     * 09:54:54 [PlanetCallDump]       [ 0]   [10]   [ 1]   [ 2]       Real: 0.011281   headRadius
     */
    std::unordered_map<uint32, PI::PinData>::iterator pinItr = ccData->pins.find(ecuID);
    if (pinItr == ccData->pins.end()) {
        ccData->ecus.erase(ecuID);
        _log(COLONY__ERROR, "Colony::InstallProgram() - ecuPinID %u not found in ccData.pins map", ecuID);
        return;
    }

    std::unordered_map<uint32, PI::ECU>::iterator ecuItr = ccData->ecus.find(ecuID);
    if (ecuItr == ccData->ecus.end()) {
        _log(COLONY__ERROR, "Colony::InstallProgram() - ecuPinID %u not found in ccData.ecus map", ecuID);
        return;
    }

    pinItr->second.launchTime = 0;
    ecuItr->second.cycleCount = 0;
    ecuItr->second.expiryTime = 0;
    ecuItr->second.headTypeID = 0;
    ecuItr->second.programType = 0;

    if (typeID < 1) {
        // uninstall program
        pinItr->second.state = PI::Pin::State::Disabled;
        pinItr->second.installTime = 0;
        pinItr->second.lastRunTime = 0;
        ecuItr->second.headRadius = 0.0;
        // reset extraction quantity in ecu attrib.  this doesnt check for invalid item
        sItemFactory.GetItemRef(ecuID)->ResetAttribute(AttrPinExtractionQuantity);
        return;
    } else {
        // install program
        pinItr->second.state = PI::Pin::State::Active;  // set ECU to currently running
        pinItr->second.installTime = GetFileTimeNow();
        pinItr->second.lastRunTime = GetFileTimeNow();
        ecuItr->second.headRadius = headRadius;

        // create head list to pass as args
        PyList* heads = new PyList();
        for (auto& head : ecuItr->second.heads) {
            PyTuple* data = new PyTuple(3);
                data->SetItemInt(0, head.first);
                data->SetItemFloat(1, head.second.latitude);
                data->SetItemFloat(2, head.second.longitude);
            heads->AddItem(data);
        }

        // recalculate output with data sent, in case it changed.
        PyRep* res = sPIDataMgr.GetProgramResultInfo(this, ecuID, typeID, headRadius, heads);
        res->DecRef();
        pinItr->second.launchTime = ecuItr->second.expiryTime;
    }
}

void Colony::SetProgramResults(uint32 ecuID, uint16 typeID, uint16 numCycles, double headRadius, float cycleTime, int32 qtyPerCycle)
{
    std::unordered_map<uint32, PI::PinData>::iterator itr = ccData->pins.find(ecuID);
    if (itr == ccData->pins.end()) {
        _log(COLONY__ERROR, "Colony::SetProgramResults() - ecuPinID %u not found in ccData.pins map", ecuID);
        return;
    }

    std::unordered_map<uint32, PI::ECU>::iterator ecuItr = ccData->ecus.find(ecuID);

    itr->second.cycleTime = cycleTime * EvE::Time::Hour;
    itr->second.qtyPerCycle = qtyPerCycle;
    ecuItr->second.cycleCount = numCycles;
    ecuItr->second.expiryTime = cycleTime * EvE::Time::Hour * numCycles + GetFileTimeNow();
    ecuItr->second.headRadius = headRadius;
    ecuItr->second.headTypeID = sPIDataMgr.GetHeadType(sItemFactory.GetItemRef(ecuID)->typeID(), typeID);
    ecuItr->second.programType = typeID;        // this is the output (P0) typeID

    m_db.UpdateECUPin(ecuID, ccData);

    // save extraction quantity in ecu attrib    this doesnt check for invalid item
    sItemFactory.GetItemRef(ecuID)->SetAttribute(AttrPinExtractionQuantity, qtyPerCycle, false);

    // set process timer based on cycle time (in hours)
    if (!m_colonyTimer.Enabled())
        m_colonyTimer.Start(/*sConfig.rates.ColonyTimer*/ cycleTime * EvE::Timer::Hour);
}
/* {'FullPath': u'UI/Messages', 'messageID': 256790, 'label': u'PlanetBlackListedBody'}(u'{planet} is not available for the general public.', None, {u'{planet}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'planet'}})
 * {'FullPath': u'UI/Messages', 'messageID': 256791, 'label': u'CannotInstallWithoutScanResultsBody'}(u'Your mining foreman reports that an intern seems to have misplaced the necessary mineral survey results. You will need to order a fresh deposit scan before this {typeName} can begin operating.', None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 256794, 'label': u'CreateRouteDestinationCannotAcceptCommodityBody'}(u'You are unable to create that route, as the destination is unable to utilize {typeName}.', None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 256795, 'label': u'CreateRouteCommodityProductionTooSmallBody'}(u"You are unable to create this shipping route as the route's origin would not produce enough {typeName} to fulfill all of its existing routes, in addition to the new one.", None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 256796, 'label': u'CreateRouteCommodityNotProducedBody'}(u"You are unable to create a shipping route for {typeName}, as it is not produced at the route's origin.", None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
 */

PyDict* Colony::TransferCommodities(uint32 srcID, uint32 destID, std::map< uint16, uint32 > items) {
    //  this call actually sends path src->dest to verify throughput, which we dont do yet...
    std::unordered_map<uint32, PI::PinData>::iterator srcItr = ccData->pins.find(srcID);
    if (srcItr == ccData->pins.end()) {
        _log(COLONY__ERROR, "Colony::TransferCommodities() - srcItr %u not found in ccData.pins map", srcID);
        if (m_client->CanThrow())
            throw CustomError("Source not found.");
        return nullptr; // make error and return.
    }
    std::unordered_map<uint32, PI::PinData>::iterator destItr = ccData->pins.find(destID);
    if (destItr == ccData->pins.end()) {
        _log(COLONY__ERROR, "Colony::TransferCommodities() - destItr %u not found in ccData.pins map", destID);
        if (m_client->CanThrow())
            throw CustomError("Destination not found.");
        return nullptr; // make error and return.
    }
    /**  TODO:  figure out how to implement these....if needed
     * {'FullPath': u'UI/Messages', 'messageID': 256630, 'label': u'ExpeditedTransferNotEnoughSpaceBody'}(u'There is not enough space at the transfer destination for the selected commodities.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 256775, 'label': u'CannotPutMissionItemInCargolinkBody'}(u'You cannot store the {typeName} in a planetary customs facility, as it an agent has issued a special embargo for this particular item.', None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
     * {'FullPath': u'UI/Messages', 'messageID': 256776, 'label': u'CannotExportCommodityNotEnoughBody'}(u'Your request to export {desired} units of {typeName} cannot be fulfilled, as the spaceport only has {contained} in stock.', None, {u'{contained}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'contained'}, u'{desired}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'desired'}, u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
     * {'FullPath': u'UI/Messages', 'messageID': 256777, 'label': u'CannotExportCommodityNotFoundBody'}(u"You cannot export {typeName}, as your spaceport's storehouse does not appear to contain any.", None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
     * {'FullPath': u'UI/Messages', 'messageID': 256778, 'label': u'RouteFailedValidationExpeditedSourceLacksCommodityQtyBody'}(u"You cannot perform this expedited transfer as the facility from which you're sourcing your commodities currently lacks the requested {qty} units of {typeName}.", None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}, u'{qty}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'qty'}})
     * {'FullPath': u'UI/Messages', 'messageID': 256779, 'label': u'RouteFailedValidationExpeditedSourceLacksCommodityBody'}(u'You cannot perform this expedited transfer, as the facility from which you are sourcing your commodities appears to lack the {typeName} which you wish to transfer.', None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
     */

    // capacities are checked in client.  proceed with xfer
    for (auto &cur : items) {
        std::map<uint16, uint32>::iterator srcItemItr = srcItr->second.contents.find(cur.first);
        std::map<uint16, uint32>::iterator destItemItr = destItr->second.contents.find(cur.first);
        if (srcItemItr != srcItr->second.contents.end()) {
            if (srcItemItr->second >= cur.second) {
                // qty ok
                srcItemItr->second -= cur.second;
            } else {
                // available != requested.  updated requested to available
                cur.second = srcItemItr->second;
                srcItr->second.contents.erase(srcItemItr);
            }
        }
        //  if src contents not found, assume client is right and proceed with xfer
        if (destItemItr != destItr->second.contents.end()) {
            // add to existing stack
            destItemItr->second += cur.second;
        } else {
            // create new stack
            destItr->second.contents.emplace_hint(destItemItr, cur.first, cur.second);
        }
    }

    // save xfer time in src pin  (per client)
    srcItr->second.lastRunTime = GetFileTimeNow();

    //self.StimulateIdlePin(destPin)

    // try OnItemChange here with ixLocation to update items   maybe later move item to limbo until xfer time is up
    //sm.ScatterEvent('OnRefreshPins', [path[0], path[-1]])  [path to, from]

    PyDict* args = new PyDict();
    args->SetItemString("simTime", new PyLong(m_procTime));
    args->SetItemString("sourceRunTime", new PyLong(GetFileTimeNow()));

    /*   this calculates the time required to xfer to dest (from client)
     * def GetExpeditedTransferTime(linkBandwidth, commodities):
     *    commodityVolume = GetCommodityTotalVolume(commodities)
     *    return long(math.ceil(max(5 * MIN, float(commodityVolume) / linkBandwidth * HOUR)))
     */
    return args;
}

/*
Export fee = Base cost × tax rate (×1.5 if launched via CC)
Import fee = Base cost × tax rate × 0.5
To open the planetover view from anywhere, Press F11 and in the side panel.  You can use the bottom window to
select planet view by right clicking the menu box in the left corner.
By switching solar systems or regions in the above boxes, you can scan planets in regions as far as your
abilities allow.
In the solar system box, you can use show info under each solar system and look at orbital bodies to get a list
of planet type rather than look at them one at a time. You can also view planet directly from the list.
You can deploy Command Centers while docked, but you must be in the same system as the planet, and the command
center must be in your ship's hold.
https://www.eve-icsc.com/jumptools/jumpplanner.php
Using this link, you calculate LY range to see what systems will be in range based on your Remote Sensing
skill level. It will help with planning.
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
    std::unordered_map<uint32, PI::PinData>::iterator pinItr = ccData->pins.find(pinID);
    if (pinItr == ccData->pins.end()) {
        _log(COLONY__ERROR, "Colony::LaunchCommodities() - pinID %u not found in ccData.pins map", pinID);
        return nullptr;
    }

    // first - create jetcan, add to system, and put in orbit around planet
    // NOTE:  PI launches have 5d timers
    /** @todo check capacities before adding items */
    SystemManager* pSysMgr = m_pSE->SystemMgr();
    Vector3d location = pSysMgr->GetSE(m_pSE->GetID())->GetPosition();
	/* NOTE:  launches spawn ~10000Km from customs office
	 * create entry in journal (pi launches)
	 * create bm?
	 *  cannot be scanned by probes (no anom sig), but does show on d-scan
	 * 5d timer
	 */
    uint32 dist = (MakeRandomInt(8, 20) * 1000000); // 8 - 20 * 1000Km
    location.MakeRandomPointOnSphere(m_pSE->GetRadius() + dist);   //orbit for launch can
    ItemData canData(EVEDB::invTypes::PlanetaryLaunchContainer,
                    m_client->GetCharacterID(),  // owner is Character
                    pSysMgr->GetID(),
                    flagAutoFit,
                     "Totally Normal Space Container",  // do we want to advertise like this?
                    location,
                    "PI Commodities Container");

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
    uint8 count = 0;
    double cost = 0;
    for (auto &cur : items) {
        std::map<uint16, uint32>::iterator cont = pinItr->second.contents.find(cur.first);
        if (cont != pinItr->second.contents.end()) {
            if (cont->second >= cur.second) {
                cont->second -= cur.second;
            } else {
                // set qty to amount contained in pin.
                cur.second = cont->second;
                pinItr->second.contents.erase(cont);
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

    // third - create db entry for launch
    m_db.SaveLaunch(contRef->itemID(), m_client->GetCharacterID(), pSysMgr->GetID(), m_pSE->GetID(), location);

    pinItr->second.launchTime = GetFileTimeNow();
    m_db.UpdateCCLaunch(pinItr->first, pinItr->second.launchTime);

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

    if (count) {
        // tell client to refresh colony pin for this container
        PyList* list = new PyList();
            list->AddItemInt(pinID);
        PyTuple* tuple = new PyTuple(1);
            tuple->items[0] = list;
        //may also need OnItemChange with this
        m_client->SendNotification("OnRefreshPins", "clientID", &tuple, false);
    }

    return new PyLong(pinItr->second.launchTime);
}

void Colony::PlanetXfer(uint32 spaceportPinID, std::map< uint32, uint16 >& importItems, std::map< uint32, uint16 >& exportItems, double taxRate)
{
    //High-sec Customs Offices(CO) have a 10% NPC tax rate
    // import is from CO to planet.  export is from planet to CO
    // this method will make the transfer of items from real to virtual and back as necessary

    std::unordered_map<uint32, PI::PinData>::iterator pinItr = ccData->pins.find(spaceportPinID);
    if (pinItr == ccData->pins.end()) {
        _log(COLONY__ERROR, "Colony::PlanetXfer() - pinID %u not found in ccData.pins map", spaceportPinID);
        if (m_client->CanThrow())
            throw CustomError("Your SpacePort on %s was not found.", m_pSE->GetName());
        return;
    }

    //{'FullPath': u'UI/Messages', 'messageID': 256577, 'label': u'CannotImportNotEnoughWarehouseSpaceBody'}(u'You cannot import commodities to that spaceport, as it does not have sufficient storage space to handle the incoming goods.', None, None)
    //{'FullPath': u'UI/Messages', 'messageID': 256626, 'label': u'CannotExportNotEnoughSpaceBody'}(u'You cannot export commodities to the customs office, as it does not have sufficient storage space to handle the incoming goods.', None, None)

    uint8 toColony = 0, fromColony = 0;
    double cost = 0.0;
    InventoryItemRef iRef;
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
        itr = pinItr->second.contents.find(iRef->typeID());
        if (itr != pinItr->second.contents.end()) {
            itr->second += cur.second;
        } else {
            pinItr->second.contents.emplace_hint(itr, iRef->typeID(), cur.second);
        }

        switch (sPIDataMgr.GetProductLevel(iRef->typeID())) {
            case 0:     cost += (     .05f * cur.second);    break; //5
            case 1:     cost += (    0.38f * cur.second);    break; //400
            case 2:     cost += (    4.50f * cur.second);    break; //7200
            case 3:     cost += (  300.00f * cur.second);    break; //60000
            case 4:     cost += (25000.00f * cur.second);    break; //1200000
        }

        //iRef->ToVirtual(pinID);
        iRef->Move(spaceportPinID, flagAutoFit, true);
        ++toColony;
    }

    if (toColony) {
        if (is_log_enabled(COLONY__TRACE))
            _log(COLONY__TRACE, "Colony::PlanetXfer() - Imported %u items from customs office %u to spaceport %u", \
                            toColony, m_pSE->GetID(), spaceportPinID);
    }

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
        std::map<uint16, uint32>::iterator cont = pinItr->second.contents.find(cur.first);
        if (cont != pinItr->second.contents.end()) {
            if (cont->second > cur.second) {
                cont->second -= cur.second;
            } else {
                // set qty to amount contained in pin.
                cur.second = cont->second;
                pinItr->second.contents.erase(cont);   // remove item from pin.contents if exporting entire qty.
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
        // make note of origin (....like im doing QA and need MTRs)
        std::string origin = "Made on ";
        origin += m_pSE->GetName();
        ItemData iData(cur.first, m_client->GetCharacterID(), spaceportPinID, flagAutoFit, cur.second, origin.c_str());
        InventoryItemRef iRef = sItemFactory.SpawnItem(iData);
        iRef->Move(m_pSE->GetCustomsOffice()->GetID(), flagHangar, true);
        ++fromColony;
    }

    if (fromColony) {
        // save xfer time in src pin  (per client)
        pinItr->second.lastRunTime = GetFileTimeNow();

        if (is_log_enabled(COLONY__TRACE))
            _log(COLONY__TRACE, "Colony::PlanetXfer() - Exported %u items from spaceport %u to customs office %u", \
                    fromColony, spaceportPinID, m_pSE->GetCustomsOffice()->GetID());
    }

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

    if (toColony or fromColony) {
        // send pin change notification...may also need OnItemChange with this
        PyList* list = new PyList();
            list->AddItemInt(spaceportPinID);
        PyTuple* tuple = new PyTuple(1);
            tuple->items[0] = list;
        m_client->SendNotification("OnRefreshPins", "clientID", &tuple, false);
    }
}

void Colony::PrioritizeRoute(uint16 routeID, int8 priority) {
    // set priority level for route...still not sure how to use it
    std::unordered_map<uint16, PI::Route>::iterator routeItr = ccData->routes.find(routeID);
    if (routeItr != ccData->routes.end()) {
        routeItr->second.priority = priority;
        m_db.SaveRoutes(ccData);
    }
}

PyTuple* Colony::GetPins(bool live/*false*/) {
    uint8 index = 0;
    PyTuple* pins(new PyTuple(ccData->pins.size()));

    for (auto &cur : ccData->pins) {
        PyDict* dict = new PyDict();
        //  required for all pins
        dict->SetItem("id", new PyInt(cur.first));
        if (live) {
            dict->SetItem("state", new PyInt(cur.second.state)); // cc state.active calls pin.refresh @ 1Hz;  this screws with poco (refresh)
        } else {
            dict->SetItem("state", new PyInt(PI::Pin::State::Idle)); // cc state.active calls pin.refresh @ 1Hz;  this screws with poco (refresh)
        }
        dict->SetItem("level", new PyInt(cur.second.level));
        dict->SetItem("typeID", new PyInt(cur.second.typeID));
        dict->SetItem("ownerID", new PyInt(cur.second.ownerID));
        dict->SetItem("latitude", new PyFloat(cur.second.latitude));
        dict->SetItem("longitude", new PyFloat(cur.second.longitude));
        dict->SetItem("lastRunTime", (cur.second.lastRunTime ? new PyLong(cur.second.lastRunTime) : PyStatic.NewNone()));

        // begin pin-specific data
        PyDict* contents = new PyDict();
        if (cur.second.isStorage) {
            for (auto &cur2 : cur.second.contents)
                contents->SetItem(new PyInt(cur2.first), new PyInt(cur2.second));
        }
        // required for all pins
        dict->SetItem("contents", contents);

        if (cur.second.isLaunchable) {
            // cc and spaceport launchTime saved as db.pin.expiryTime
            dict->SetItem("lastLaunchTime", (cur.second.launchTime > 0 ? new PyLong(cur.second.launchTime) : PyStatic.NewNone()));
        }

        if (cur.second.isProcess) {
            if (cur.second.schematicID) {
                dict->SetItem("schematicID", new PyInt(cur.second.schematicID));
                dict->SetItem("cycleTime", new PyLong(cur.second.cycleTime));
                std::unordered_map<uint32, PI::Plant>::iterator plantItr = ccData->plants.find(cur.first);
                dict->SetItem("hasReceivedInputs", new PyBool(plantItr->second.hasReceivedInputs));
                dict->SetItem("receivedInputsLastCycle", new PyBool(plantItr->second.receivedInputsLastCycle));
            } else {
                dict->SetItem("schematicID", PyStatic.NewNone());
                dict->SetItem("hasReceivedInputs", PyStatic.NewFalse());
                dict->SetItem("receivedInputsLastCycle", PyStatic.NewFalse());
            }
        }

        if (cur.second.isECU) {
            std::unordered_map<uint32, PI::ECU>::iterator ecuItr = ccData->ecus.find(cur.first);
            if (ecuItr->second.programType) {
                dict->SetItem("cycleTime", new PyLong(cur.second.cycleTime));
                dict->SetItem("expiryTime", (ecuItr->second.expiryTime ? new PyLong(ecuItr->second.expiryTime) : PyStatic.NewNone()));
                dict->SetItem("headRadius", new PyFloat(ecuItr->second.headRadius));
                dict->SetItem("installTime", (cur.second.installTime ? new PyLong(cur.second.installTime) : PyStatic.NewNone()));
                dict->SetItem("programType", new PyInt(ecuItr->second.programType));
                dict->SetItem("qtyPerCycle", new PyInt(cur.second.qtyPerCycle));
            } else {
                dict->SetItem("programType", PyStatic.NewNone());
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
    uint8 index = 0;
    PyTuple* links = new PyTuple(ccData->links.size());
    for (auto &cur : ccData->links) {
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
    uint8 index = 0;
    PyTuple* routes = new PyTuple(ccData->routes.size());

    for (auto &cur : ccData->routes) {
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
            std::unordered_map<uint32, PI::ECU>::iterator ecuItr = ccData->ecus.find(cur);
            if (ecuItr != ccData->ecus.end()) {
                m_db.SaveHeads(m_colonyID, m_client->GetCharacterID(), cur, ecuItr->second.heads);
            } else {
                _log(COLONY__ERROR, "Colony::GetColony()::SaveHeads() - headID %u not found in ccData.ecus map", cur);
            }
        }
        tempECUs.clear();
        m_newHead = false;
    }

    Update();   // update colony before sending data.

    PyDict* args = new PyDict();
        args->SetItem("pins", GetPins(true));
        args->SetItem("level", new PyInt(ccData->level));
        args->SetItem("links", GetLinks());
        args->SetItem("routes", GetRoutes());
        args->SetItem("currentSimTime", new PyLong(m_procTime));
    PyObject* res = new PyObject("util.KeyVal", args);

    if (is_log_enabled(COLONY__GC_DUMP)) {
        _log(COLONY__GC_DUMP, "Colony::GetColony() Dump");
        res->Dump(COLONY__GC_DUMP, "    ");
    }

    // reset temp-to-new PinID map after command loop is completed and all new pins have been created.
    tempPinIDs.clear();

    return res;
}

void Colony::Update() {
    double profileStartTime = GetTimeUSeconds();

    /* loop thru process calls to update each pin to simulate production and logistics
     *  this will have to be fast, as there may/will be large time deltas between updates
     *  can loop each item to process for each time step (like i do for skill training)
     */

    if (is_log_enabled(COLONY__DEBUG))
        _log(COLONY__DEBUG, "Colony::Update() - Starting Update for colony %u on %s.", m_colonyID, m_pSE->GetName());

    // update colony time to current time
    m_procTime = GetFileTimeNow();

    // first, process ecus for raw matls.
    ProcessECUs();
    // second, process plants with matl's received
    ProcessPlants();

    // update CommandCenter runtime
    std::unordered_map<uint32, PI::PinData>::iterator pinItr = ccData->pins.find(m_colonyID);
    if (pinItr != ccData->pins.end())
        pinItr->second.lastRunTime = m_procTime;

    /** @note:  colony runtimes
	 *
	 * type		# pins	   runtime in us
	 * empty  	   1         45 - 100   (47 avg)
         * basic	 2 - 10     177 - 5364.250us  (177 avg for Inactive)
	 * prod		10 - 20
	 * adv		20 - 40
	 * max		  40+
         * Update completed in 5364.250us with 8 links, 10 pins, 5 plants, and 13 routes (s:13, d:8)
         * Update completed in 10812.250us with 8 links, 10 pins, 5 plants, and 13 routes (s:13, d:8)
         * Update completed in 2668.250us with 8 links, 10 pins, 5 plants, and 13 routes (s:13, d:8)
         *
         *
         */
    if (is_log_enabled(COLONY__INFO))
        _log(COLONY__INFO, "Colony::Update() - Update completed in %.3fus with %lu links, %lu pins, %lu plants, and %lu routes (s:%lu, d:%lu) ", \
                    GetTimeUSeconds() - profileStartTime, ccData->links.size(), ccData->pins.size(), ccData->plants.size(), ccData->routes.size(), \
                    m_srcRoutes.size(), m_destRoutes.size());

    // profile timer for the colony updates
    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::colony, GetTimeUSeconds() - profileStartTime);
}

void Colony::ProcessECUs() {
    if (ccData->ecus.empty() or m_srcRoutes.empty())
        return;

    uint16 cycles = 0;
    int32 amount = 0;
    double delta = 0.0;         // hours since last run
    double divisor = 0.0;       // cycleTime in hours
    // routed item [typeID, qty}
    std::map<uint16, uint32>::iterator itemItr;
    // this is our source {pinID, data}
    std::unordered_map<uint32, PI::PinData>::iterator ecuPinItr;
    // either plant or storage {itemID, data}
    std::unordered_map<uint32, PI::PinData>::iterator destPinItr;
    for (auto &ecu : ccData->ecus) {
        if (is_log_enabled(COLONY__DEBUG))
            _log(COLONY__DEBUG, "Colony::ProcessECUs() - Start Processing %s(%u).", \
                    sPIDataMgr.GetPinName(ecu.first), ecu.first);

        /** @note this is a simple process, as it only provides raw mats, simulating extraction from planet and
         *  shipped to storage or directly to plant for processing.
         * however, in the case of shipping directly to plant, we will have to store the mats in the plant queue
         * and wait for the ProcessPlants() call to use them, as this will avoid over-complicating things,
         * but it could get messy later....
         */

        // make sure we reset amount for this run
        amount = 0;
        // this should never be invalid
        ecuPinItr = ccData->pins.find(ecu.first);
        // first - get elapsed times and generate runs to simulate.  this avoids looping
        if (ecu.second.expiryTime < m_procTime) {
            // check for valid run time
            if (ecu.second.expiryTime == 0) {
                if (is_log_enabled(COLONY__WARNING))
                    _log(COLONY__WARNING, "Colony::ProcessECUs() - Inactive");
                ecuPinItr->second.state = PI::Pin::State::Idle;
                continue;
            }
            // ecu program is complete.  determine cycles remaining from last runtime and continue
            delta = std::round(((double)ecu.second.expiryTime - ecuPinItr->second.lastRunTime) / EvE::Time::Hour);
            ecuPinItr->second.lastRunTime = ecu.second.expiryTime;  // no need to reset expiryTime
            ecuPinItr->second.state = PI::Pin::State::Idle;
            ecu.second.expiryTime = 0;
            if (delta < 1.0) {
                // warning...no run count
                if (is_log_enabled(COLONY__WARNING))
                    _log(COLONY__WARNING, "Colony::ProcessECUs() - delta < 1.  Continuing Loop.");
                continue;
            }
        } else {
            ecuPinItr->second.state = PI::Pin::State::Active;
            delta = ((double)m_procTime - ecuPinItr->second.lastRunTime) / EvE::Time::Hour;
        }
        divisor = (double)ecuPinItr->second.cycleTime / EvE::Time::Hour;
        cycles = static_cast<uint16>(floor(delta / divisor));

        if (cycles < 1) {
            if (is_log_enabled(COLONY__WARNING))
                _log(COLONY__WARNING, "Colony::ProcessECUs() - cycles < 1.  Continuing Loop.");
            continue;
        }

        if (cycles > ecu.second.cycleCount)
            cycles = ecu.second.cycleCount;

        if (is_log_enabled(COLONY__DEBUG))
            _log(COLONY__DEBUG, "Colony::ProcessECUs() - begin processing with %u of %u cycle%s (%0.3f / %0.3f)", \
                    cycles, ecu.second.cycleCount, ecu.second.cycleCount > 1 ? "s":"", delta, divisor);

        // get planet heat map for this resource
        std::string& resourceBuffer = m_pSE->GetResourceBuffer(ecu.second.programType);
        float duration = divisor * cycles;
        if (is_log_enabled(COLONY__DEBUG)) {
            _log(COLONY__DEBUG, "Colony::ProcessECUs() - extraction duration is %0.2fh", duration);
            //_log(COLONY__DEBUG, "Colony::ProcessECUs() - resourceBuffer: %s", resourceBuffer.c_str());
        }

        // Re-pack your volatile pin structures so our Gaussian falloff can read the step-by-step decay
        std::vector<PI::ActiveEcuHead> progressiveMasks;
        for (const auto& activeEcu : ccData->ecus) {
            if (activeEcu.second.programType != ecu.second.programType)
                continue;
            for (const auto& activeHead : activeEcu.second.heads) {
                PI::ActiveEcuHead mask;
                mask.pinID = activeHead.second.ecuPinID;
                mask.latitude = activeHead.second.latitude;
                mask.longitude = activeHead.second.longitude;
                mask.headRadius = activeEcu.second.headRadius;
                mask.depletionAmount = activeHead.second.currentDepletion;
                progressiveMasks.push_back(mask);
            }
        }

        int32 amount = sPlanetDataMgr.CalculateEcuYield(m_pSE, cycles, resourceBuffer, ecu.second.heads, progressiveMasks);

        _log(COLONY__ERROR, "Colony::ProcessECUs() - amount after extraction is %i", amount);

        // second - see if this ecu has a route and move contents per route.  this will simulate xfer of raw matls from heads to storage
        auto routeItr = m_srcRoutes.equal_range(ecu.first);     // this ecu is route origin
        // how many routes from this origin?  this is a count of ecu->dest when dest can be multiple places
        int32 dist = EvE::max(std::distance(routeItr.first, routeItr.second) - 1, 1);
        // make sure amount is valid
        if (dist > amount)
            continue;
        // adjust amount to evenly supply all routes
        amount /= dist;
        for (auto it = routeItr.first; it != routeItr.second; ++it) {
            // third - update destination contents per route movement as noted above (ECU does not store matls - nothing to deduct from)
            destPinItr = ccData->pins.find(it->second.destPinID);
            itemItr = destPinItr->second.contents.find(it->second.commodityTypeID);
            if (itemItr != destPinItr->second.contents.end()) {
                // add to existing stack
                itemItr->second += amount;
            } else {
                // create new stack
                destPinItr->second.contents.emplace_hint(itemItr, it->second.commodityTypeID, amount);
            }

            // trigger contents update
            destPinItr->second.update = true;

            if (destPinItr->second.isProcess) {
                // routing is straight to plant.  trigger input
                std::unordered_map<uint32, PI::Plant>::iterator plantItr = ccData->plants.find(it->second.destPinID);
                if (plantItr == ccData->plants.end())
                    continue;  // make error here?
                // this triggers plant input loop for req mat'l
                plantItr->second.hasReceivedInputs = true;
            }

            if (is_log_enabled(COLONY__DEBUG))
                _log(COLONY__DEBUG, "Colony::ProcessECUs(%i) - Dest: %s(%u) updated with %0.0f %s(%u).", \
                        dist, sPIDataMgr.GetPinName(destPinItr->first), it->second.destPinID, amount, \
                        sPIDataMgr.GetProductName(it->second.commodityTypeID), it->second.commodityTypeID);
        }

        if (is_log_enabled(COLONY__DEBUG))
            _log(COLONY__DEBUG, "Colony::ProcessECUs() - Processing complete.");

        // fourth - update pin runtime...
        ecuPinItr->second.lastRunTime = m_procTime;
        //  and set flag to trigger pin contents update
        m_toUpdate = true;
    }
}

void Colony::ProcessPlants() {
    if (m_plantMap.empty() or (m_pLevel < 1))
        return; // nothing to do...

    /** @note  generally-accepted PI design has plant input/output from/to storage (spaceport or silo)
     * for input buffers and possibly feeding multiple plants.
     * this design is arranged as follows...
     *
     * silo->plant(s)->silo->plant(s)->silo
     *
     * however, there may be cases where the colony is restricted or other design constraints limit routing and
     * plants must be linked together, where the output of one provides the direct input of the next, as follows...
     *
     * silo->plant(s)->plant(s)->plant(s)->silo
     *
     * with plants as needed for production requirements of the colony.
     *
     * this will need to check for and be able to process both cases, and could be somewhat complicated.
     *
     * plants will have to be processed in product order from p1 to p4 to provide input for downstream plants
     * this WILL have to loop for each product cycle to correctly set inputs and outputs for each plant, and provide
     * positive material control (and be more realistic) per run.
     * each pLevel will run multiple cycles based on run times within it's loop
     * UPDATE:  each plant/pLevel will need single-runs to allow proper item routing from sources
     *    i.e.  silo->plants
     *    first plant to hit here with multiple cycles will consume all available material,
     *    leaving none for remaining plants fed same material from same silo
     */

    uint8 curCycle = m_pLevel;
    uint16 tempCycles = 0;
    int32 cycles = 0, cycles2 = 0, amount = 0;
    int32 delta = 0;    // minutes since last run
    int32 divisor = 0;  //cycleTime in minutes
    // either plant or storage {itemID, data}
    std::unordered_map<uint32, PI::PinData>::iterator srcPinItr;
    // either plant or storage {itemID, data}
    std::unordered_map<uint32, PI::PinData>::iterator destPinItr;
    // common pin data for plant {itemID, data}
    std::unordered_map<uint32, PI::PinData>::iterator plantPinItr;
    // stored item [typeID, qty}
    std::map<uint16, uint32>::iterator itemItr;
    // plant-specific data  {itemID, data}
    std::unordered_map<uint32, PI::Plant>::iterator plantItr;

    if (is_log_enabled(COLONY__INFO))
        _log(COLONY__INFO, "Colony::ProcessPlants() - Begin Plant Processing.  m_procTime: %lli", m_procTime);

    // can this loop be split into smaller calls?  (like warp in destiny)
    //    ...maybe, but will take some thought and doing to make it work right
    while (curCycle < 5) {
        if (is_log_enabled(COLONY__DEBUG))
            _log(COLONY__DEBUG, "Colony::ProcessPlants() - Begin Process loop for pLevel %u.", curCycle);

        // plants must be processed in order to correctly consume inputs, make products and send outputs to downstream recipients.
        auto cycleItr = m_plantMap.equal_range(curCycle);
        for (auto it = cycleItr.first; it != cycleItr.second; ++it) {
            if (is_log_enabled(COLONY__INFO))
                _log(COLONY__INFO, "Colony::ProcessPlants() - Begin Processing for %s(%u)", \
                    sPIDataMgr.GetPinName(it->second), it->second);

            // first, find plant pin in plant map
            plantItr = ccData->plants.find(it->second);
            plantPinItr = ccData->pins.find(plantItr->first);

            // verify plant can run (or is running)
            if (plantPinItr->second.schematicID == 0) {
                if (is_log_enabled(COLONY__DEBUG))
                    _log(COLONY__DEBUG, "Colony::ProcessPlants() - No schematic installed.  Continuing Loop.");
                continue;
            }

            // second, check processing times for active plants
            delta = static_cast<int32>((m_procTime - plantPinItr->second.lastRunTime)  / EvE::Time::Minute);
            divisor = static_cast<int32>(std::ceil(plantPinItr->second.cycleTime / EvE::Time::Minute));
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
                plantPinItr->second.state                = PI::Pin::State::Active;
                plantItr->second.hasReceivedInputs       = true;
                plantItr->second.receivedInputsLastCycle = true;
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
            if (is_log_enabled(COLONY__INFO))
                _log(COLONY__INFO, "Colony::ProcessPlants() - Begin Input Route loop for %s(%u).", \
                    sPIDataMgr.GetPinName(plantItr->first), plantItr->first);
            auto destRouteItr = m_destRoutes.equal_range(plantItr->first);
            for (auto it = destRouteItr.first; it != destRouteItr.second; ++it) {
                // this route supplies current plant with input matls.
                srcPinItr = ccData->pins.find(it->second.srcPinID);
                // is source plant or ecu?
                if (srcPinItr->second.isECU or srcPinItr->second.isProcess) {
                    //yep.  nothing to do here...mat'l/qty (supposedly) already routed
                    if (is_log_enabled(COLONY__DEBUG))
                        _log(COLONY__DEBUG, "Colony::ProcessPlants() - Source %s is Plant or ECU.  Skipping this input routing loop.", \
                            sPIDataMgr.GetPinName(srcPinItr->first));
                    continue;
                }

                // source found as storage.  search for routed commodity and continue
                itemItr = srcPinItr->second.contents.find(it->second.commodityTypeID);
                if (itemItr == srcPinItr->second.contents.end()) {
                    if (is_log_enabled(COLONY__WARNING))
                        _log(COLONY__WARNING, "Colony::ProcessPlants() - Routed Commodity %s(%u) not found in %s(%u). Break out.", \
                                sPIDataMgr.GetProductName(it->second.commodityTypeID), it->second.commodityTypeID, \
                                sPIDataMgr.GetPinName(srcPinItr->first), srcPinItr->first);
                    plantItr->second.hasReceivedInputs = false;
                    break;
                }

                // check dest before moving anything
                destPinItr = ccData->pins.find(it->second.destPinID);
                // dest should be current plant  (searched by destID using currentPlantID)
                if (destPinItr->first != plantItr->first) {
                    // should never hit
                    _log(COLONY__ERROR, "Colony::ProcessPlants() - route %u, dest %s(%u) != current plant %s(%u).  Breaking out.", \
                            it->first, sPIDataMgr.GetPinName(it->second.destPinID), it->second.destPinID, \
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
                    srcPinItr->second.contents.erase(itemItr);
                }

                // trigger contents update
                srcPinItr->second.update = true;

                if (is_log_enabled(COLONY__DEBUG))
                    _log(COLONY__DEBUG, "Colony::ProcessPlants() - Removed %i %s(%u) from %s(%u).", \
                            amount, sPIDataMgr.GetProductName(it->second.commodityTypeID), it->second.commodityTypeID, \
                            sPIDataMgr.GetPinName(srcPinItr->first), srcPinItr->first);

                // add contents to dest plant's pin
                itemItr = destPinItr->second.contents.find(it->second.commodityTypeID);
                if (itemItr != destPinItr->second.contents.end()) {
                    itemItr->second += amount;
                } else {
                    destPinItr->second.contents.emplace_hint(itemItr, it->second.commodityTypeID, amount);
                }

                // trigger contents update
                destPinItr->second.update = true;

                if (is_log_enabled(COLONY__DEBUG))
                    _log(COLONY__DEBUG, "Colony::ProcessPlants() - Added %i %s(%u) to %s(%u).", \
                            amount, sPIDataMgr.GetProductName(it->second.commodityTypeID), it->second.commodityTypeID, \
                            sPIDataMgr.GetPinName(destPinItr->first), destPinItr->first);

                // we have received a material from this route.  check for plant
                if (destPinItr->second.isProcess) {
                    //enable check for all required materials in this Schematic for this plant
                    plantItr->second.hasReceivedInputs = true;
                }

            }

            // fourth, process input material requirements
            // verify plant pin
            destPinItr = ccData->pins.find(plantItr->first);
            cycles2 = 0;
            if (is_log_enabled(COLONY__INFO))
                _log(COLONY__INFO, "Colony::ProcessPlants() - %s Input Check loop for %s(%u).", \
                    plantItr->second.hasReceivedInputs ? "Begin" : "Skipping", \
                    sPIDataMgr.GetPinName(plantItr->first), plantItr->first);

            plantItr->second.receivedInputsLastCycle = false;

            if (plantItr->second.hasReceivedInputs) {
                /*  if plant has received mats from routing (above), then check here for required qtys per Schematic.
                 *     input data is found in plantItr->second.data.inputs map (std::unordered_map<uint16, uint16> {typeID, qty})
                 *
                 *  if required mats are not present, set receivedInputsLastCycle=false, which will deny processing
                 *      and subsequent routing for this plant.
                 *
                 *  if all required qtys have been received, proceed with the following:
                 *   - remove mats from pin.contents
                 *   - set receivedInputsLastCycle=true
                 */
                // this should no longer hit
                if (plantItr->second.data.inputs.empty()) {
                    _log(COLONY__WARNING, "Colony::ProcessPlants() - Empty input map. Continuing matl process loop.");
                    // skip further processing
                    destPinItr->second.state = PI::Pin::State::Idle;
                    plantItr->second.hasReceivedInputs = false;
                    continue;
                }

                tempCycles = cycles;
                for (auto &mats : plantItr->second.data.inputs) {
                    // loop thru Schematic inputs to verify all required mats are present
                    itemItr = destPinItr->second.contents.find(mats.first);
                    if (itemItr == destPinItr->second.contents.end()) {
                        if (is_log_enabled(COLONY__DEBUG))
                            _log(COLONY__DEBUG, "Colony::ProcessPlants() - %s (%u) not found in %s(%u).  Break out of loop.", \
                                    sPIDataMgr.GetProductName(mats.first), mats.first, \
                                    sPIDataMgr.GetPinName(plantItr->first), (plantItr->first));
                        // this required material was not found in plant inventory.  skip further processing
                        plantItr->second.hasReceivedInputs = false;
                        cycles = 0;
                        break;
                    }
                    if (itemItr->second >= (mats.second * tempCycles)) {
                        itemItr->second -= (mats.second * tempCycles);
                        plantItr->second.receivedInputsLastCycle = true;
                    } else {
                        // this required material was not sufficient quantity for (num cycles) runs.
                        // determine how many cycles we can run with current material quantity
                        if (is_log_enabled(COLONY__DEBUG))
                            _log(COLONY__DEBUG, "Colony::ProcessPlants() - Not enough %s(%u) for %u cycles.  Need %u, Have %u", \
                            sPIDataMgr.GetProductName(mats.first), mats.first, tempCycles, mats.second * tempCycles, itemItr->second);
                        cycles2 = itemItr->second / mats.second;
                        if (cycles2 > 0) {
                            itemItr->second -= mats.second * cycles2;
                            plantItr->second.receivedInputsLastCycle = true;
                        } else {
                            plantItr->second.hasReceivedInputs = false;
                            cycles = 0;
                            break;
                        }
                        // set temp variable with minimum cycle count
                        if (tempCycles > cycles2)
                            tempCycles = cycles2;
                        cycles2 = 0;
                    }

                    if (is_log_enabled(COLONY__DEBUG))
                        _log(COLONY__DEBUG, "Colony::ProcessPlants() - Have enough %s for %u cycles.", \
                                sPIDataMgr.GetProductName(mats.first), tempCycles);
                }

                // we have enough mat'l for at least one process.  set cycles based on material in inventory.
                if (cycles > tempCycles)
                    cycles = tempCycles;
            } else {
                // we have not received inputs last cycle
                plantItr->second.hasReceivedInputs = false;
                plantItr->second.receivedInputsLastCycle = false;
                cycles = 0;
            }

            // at this point, we have looped thru all required mats and set plant variables accordingly.

            // fifth, process manufacturing cycle and move finished product per route
            if (is_log_enabled(COLONY__INFO))
                _log(COLONY__INFO, "Colony::ProcessPlants() - %s Output Routing loop for %s(%u).", \
                    cycles > 0 ? "Begin" : "Skipping", sPIDataMgr.GetPinName(plantItr->first), plantItr->first);

            if ((cycles > 0) and plantItr->second.receivedInputsLastCycle) {
                auto srcRouteItr = m_srcRoutes.equal_range(plantItr->first);
                for (auto it = srcRouteItr.first; it != srcRouteItr.second; ++it) {
                    // get destination pin and update qty there for this round
                    destPinItr = ccData->pins.find(it->second.destPinID);
                    // contents are stored in each pin.  PI::PinData.contents(std::unordered_map<uint16, uint32> typeID, qty)
                    // we have plant cycles for this loop, so multiply output by cycles to get a total to simulate the "active" plant
                    amount = it->second.commodityQuantity * cycles;
                    if (amount < 1) {
                        _log(COLONY__ERROR, "Colony::ProcessPlants() - amount after routing is %i", amount);
                        break;
                    }

                    // pin item has capy attr. the above isnt needed.  use attributes!!
                    itemItr = destPinItr->second.contents.find(it->second.commodityTypeID);
                    if (itemItr != destPinItr->second.contents.end()) {
                        // add to existing
                        itemItr->second += amount;
                    } else {
                        // create new stack
                        destPinItr->second.contents.emplace_hint(itemItr, it->second.commodityTypeID, amount);
                    }

                    // trigger contents update
                    destPinItr->second.update = true;

                    if (is_log_enabled(COLONY__DEBUG))
                        _log(COLONY__DEBUG, "Colony::ProcessPlants() - Added %u %s (%u) to %s(%u).", \
                                amount, sPIDataMgr.GetProductName(it->second.commodityTypeID), \
                                it->second.commodityTypeID, sPIDataMgr.GetPinName(it->second.destPinID), \
                                it->second.destPinID);

                    if (destPinItr->second.isStorage) {
                        //  if dest cant hold entire xfer qty, drop remainder in current pin contents (as opposed to loss)
                        if (is_log_enabled(COLONY__DEBUG))
                            _log(COLONY__DEBUG, "Colony::ProcessPlants() - Dest is storage");
                    } else if (destPinItr->second.isProcess) {
                        // find dest's plant data
                        //  the destination plant will have a P level of curCycle+1, and will process on next iteration
                        auto destPlantItr = ccData->plants.find(destPinItr->first);
                        //then set dest's hasReceivedInputs to true for subsequent processing
                        destPlantItr->second.hasReceivedInputs = true;
                    }
                }

                // set plant to currently running
                plantPinItr->second.state = PI::Pin::State::Active;
                // update plant run time
                plantPinItr->second.lastRunTime += (plantPinItr->second.cycleTime * cycles);
                // trigger to update pin contents
                m_toUpdate = true;

                if (is_log_enabled(COLONY__DEBUG))
                    _log(COLONY__DEBUG,  "Colony::ProcessPlants() - Output Routing Successful.  Run Time Updated.");
                // if there are materials left, verify qty and move excess back to previous storage, if applicable
            } else {
                // set plant to currently running
                plantPinItr->second.state = PI::Pin::State::Idle;
                // update plant run time
                plantPinItr->second.lastRunTime = m_procTime;
            }

            if (is_log_enabled(COLONY__INFO))
                _log(COLONY__INFO, "Colony::ProcessPlants() - Processing Complete for %s(%u)", \
                    sPIDataMgr.GetPinName(it->second), it->second);
        }

        if (is_log_enabled(COLONY__DEBUG))
            _log(COLONY__DEBUG, "Colony::ProcessPlants() - Process loop complete for pLevel %u.", curCycle);

        // this pLevel cycle complete.  increment and begin next loop
        ++curCycle;
    }
}
