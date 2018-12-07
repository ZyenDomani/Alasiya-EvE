/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
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
*/

#include "eve-server.h"

#include "PyServiceMgr.h"
#include "Client.h"
#include "inventory/ItemType.h"
#include "planet/Colony.h"
#include "planet/Planet.h"
#include "planet/PlanetDataMgr.h"

// for launching shit...
#include "account/AccountService.h" // fund xfer and journal logging methods
#include "system/Container.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"

/** @todo:
 * items to work on...
 *  import/export tax
 *  cost of building
 *  timers (launch, run, current, logistics)
 *  planet items attribs/effects
 *  item->Move() logistics
 *
    __notifyevents__ = [
    'OnPlanetCommandCenterDeployedOrRemoved',
     'OnPlanetPinsChanged',
     'OnColonyPinCountUpdated',
 */
/* The list of restricted systems is:

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
/*10% + % Player Tax - 1% per level of Customs Code Expertise
 * Export fee = Base cost * tax rate (*1.5 if launched via Command Center)
Import fee = Base cost * tax rate * 0.5

Example: Exporting a unit of Biomass (P1) using a Launchpad from a low-sec planet with a 10% Player Tax will cost will cost 40 ISK (400 * 10% * 1 )
Importing that unit of Biomass to a high-sec factory planet will cost at minimum an additional 20 ISK (400 * (10% + % Player Tax) * 0.5 )

Base costs for each tier of products can be found in this table. Base costs are per unit.
Commodity level     Base Cost
R0  5 ISK
P1  400 ISK
P2  7,200 ISK
P3  60,000 ISK
P4  1,200,000 ISK
*/
Colony::Colony(PyServiceMgr* mgr, Client* pClient, SystemEntity* pSE)
:m_svcMgr(mgr),
m_client(pClient),
m_pSE(pSE->GetPlanetSE()),
ccPin(new PI_CCPin())
{
    m_active = false;
    m_loaded = false;
    m_newHead = false;

    m_pg = 0;
    m_cpu = 0;
    m_pLevel = 5;
    m_colonyID = 0;
    m_procTime = 0; // process check.  init to zero and changed in pin program install/update
    tempPinIDs.clear();
    _log(PLANET__DEBUG, "Colony::Colony() c'tor called for %s(%u)", pClient->GetCharacterName().c_str(), pClient->GetCharacterID());
}

Colony::~Colony()
{
    SafeDelete(ccPin);
}

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

void Colony::Load()
{
    if (m_loaded) {
        Update();
        return;
    }

    m_db.LoadPins(m_colonyID, ccPin->pins);
    m_db.LoadLinks(m_colonyID, ccPin->links);
    m_db.LoadRoutes(m_colonyID, ccPin->routes);

    LoadPlants();

    if (ccPin->pins.size() > 0)
        m_loaded = true;
}

void Colony::LoadPlants()
{
    for (auto cur: ccPin->pins) {
        if (cur.second.isProcess) {
            PI_Plant plant;
                plant.state = cur.second.state;
                plant.cycleTime = cur.second.cycleTime;
                plant.expiryTime = cur.second.expiryTime;
                plant.installTime = cur.second.installTime;
                plant.lastRunTime = cur.second.lastRunTime;
                plant.schematicID = cur.second.schematicID;
                plant.hasReceivedInputs = cur.second.hasReceivedInputs;
                plant.receivedInputsLastCycle = cur.second.receivedInputsLastCycle;
                if (plant.schematicID)
                    sPIDataMgr.GetSchematicData(plant.schematicID, plant.data);
                plant.order = GetProductLevel(plant.data.outputType);   // i am ordering plant processing by output Plevel
                m_pLevel = (uint8)EvE::min(m_pLevel, plant.order);
            ccPin->plants[cur.first] = plant;
        }
    }
}

void Colony::UpdatePlants()
{
    for (auto cur : ccPin->plants) {
        std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(cur.first);
        if (itr != ccPin->pins.end()) {
            itr->second.state = cur.second.state;
            itr->second.cycleTime = cur.second.cycleTime;
            itr->second.expiryTime = cur.second.expiryTime;
            itr->second.installTime = cur.second.installTime;
            itr->second.lastRunTime = cur.second.lastRunTime;
            itr->second.schematicID = cur.second.schematicID;
            itr->second.qtyPerCycle = cur.second.qtyPerCycle;
            itr->second.hasReceivedInputs = cur.second.hasReceivedInputs;
            itr->second.receivedInputsLastCycle = cur.second.receivedInputsLastCycle;
        }
    }
}

void Colony::Save()
{
    /** @todo maybe separate these saves */
    UpdatePlants(); // this MUST be called before Save() and GetColony() to update plant pin with current data
    m_db.SavePins(ccPin);
    m_db.SaveLinks(ccPin);
    //m_db.SaveRoutes(ccPin);
    m_db.SaveContents(ccPin);
}

void Colony::Shutdown()
{
    Update();
    UpdatePlants();
    m_db.SavePins(ccPin);
    m_db.SaveContents(ccPin);
}

// called by PlanetSE::Process()
void Colony::Process()
{
    Update();
    //  this is part of clever code to avoid db hits on every update.
    //  this method will check for updated contents and save to db as needed.
    //  it tics every 30m
    for (auto cur : ccPin->pins) {
        if ((cur.second.update) and (cur.second.isStorage)) {
            m_db.RemoveContents(cur.first);
            m_db.SavePinContents(m_colonyID, cur.first, cur.second.contents);
        }
        cur.second.update = false;
    }
}

uint32 Colony::GetOwner()
{
    return m_client->GetCharacterID();
}

void Colony::AbandonColony()
{
    /** @todo  go thru entire pinMap and delete each itemRef.  */
    InventoryItemRef iRef = sItemFactory.GetItem(m_colonyID);
    iRef->Delete();
    m_db.DeleteColony(m_colonyID, m_pSE->GetID(), m_client->GetCharacterID());
    SafeDelete(ccPin);
    ccPin = new PI_CCPin();
    m_colonyID = 0;
}

void Colony::CreateCommandPin(uint32 itemID, uint32 typeID, double latitude, double longitude) {
    m_colonyID = itemID;
    ccPin->ccPinID = itemID;
    m_db.SaveCommandCenter(itemID, m_client->GetCharacterID(), m_pSE->GetID(), typeID, latitude, longitude);
    ccPin->level = PinLevel0;
    ccPin->currentSimTime = GetFileTimeNow();
    CreatePin(EVEDB::invGroups::Command_Centers, itemID, typeID, latitude, longitude);
    m_db.SavePins(ccPin);
}

void Colony::CreatePin(uint32 groupID, uint32 pinID, uint32 typeID, double latitude, double longitude) {
    /** @todo will have to write code for effects and checks for pg/cpu/m3/etc for all of these.  */
    using namespace EVEDB::invGroups;
    PI_Pin pin;
    InventoryItemRef iRef = InventoryItemRef(nullptr);
    if (groupID != Command_Centers) {
        // type, owner, location, flag, qty
        ItemData data(typeID, m_client->GetCharacterID(), m_pSE->GetID(), flagAutoFit, 1);
        iRef = sItemFactory.SpawnItem(data);

        /*  this shit doesnt work....changes arent sent to client.  not sure why
        m_pg = iRef->GetAttribute(AttrPowerLoad).get_int();
        m_cpu = iRef->GetAttribute(AttrCpuLoad).get_int();
        if (groupID != Planetary_Links) {
            // reset pg/cpu needs based on char skills for all modules (ex links)
            m_pg *= (1 - ( 0.05 * (m_client->GetChar()->GetSkillLevel(skillEngineering, true))));               // 5% decrease in need
            m_pg *= (1 - ( 0.01 * (m_client->GetChar()->GetSkillLevel(skillEnergyManagement, true))));          // 1% decrease in need
            m_pg *= (1 - ( 0.01 * (m_client->GetChar()->GetSkillLevel(skillCommandCenterUpgrades, true))));     // 1% decrease in need
            m_pg *= (1 - ( 0.01 * (m_client->GetChar()->GetSkillLevel(skillEnergySystemsOperation, true))));    // 1% decrease in need

            m_cpu *= (1 - ( 0.05 * (m_client->GetChar()->GetSkillLevel(skillElectronics, true))));              // 5% decrease in need
            m_cpu *= (1 - ( 0.01 * (m_client->GetChar()->GetSkillLevel(skillCommandCenterUpgrades, true))));    // 1% decrease in need
        } */
    } else {
        iRef = sItemFactory.GetItem(m_colonyID);
        if (iRef->quantity() > 1) {
            // check for stack of CC items, and split as needed
            ItemData data(typeID, m_client->GetCharacterID(), 0, flagAutoFit, iRef->quantity() -1);
            InventoryItemRef iRef2 = sItemFactory.SpawnItem(data);
            iRef2->Move(m_client->GetShipID(), flagCargoHold);
            iRef->SetQuantity(1);
        }
        m_client->GetShip()->RemoveItem(iRef);
    }

    pin.typeID = typeID;
    pin.ownerID = m_client->GetCharacterID();
    pin.latitude = latitude;
    pin.longitude = longitude;
    pin.level = PinLevels::PinLevel0;
    pin.state = PinStates::PINSTATE_ACTIVE;
    pin.lastRunTime = 0;
    pin.lastLaunchTime = 0;
    pin.heads.clear();
    pin.contents.clear();

    // for contents updating
    pin.update = false;

    switch(groupID) {
        case Command_Centers: {     // 1027
            pin.isStorage = true;
            pin.isLaunchable = true;
            pin.isCommandCenter = true;
        } break;
        case  Processors: {         // 1028
            //pin.isStorage = true;
            pin.isProcess = true;
            PI_Plant plant;
            ccPin->plants[iRef->itemID()] = plant;
        } break;
        case Extractor_Control_Units: { // 1063
            pin.isECU = true;
            pin.qtyPerCycle = (int16)iRef->GetAttribute(AttrPinExtractionQuantity).get_int();
        } break;
        case Spaceports:{   // 1030
            pin.isStorage = true;
            pin.isLaunchable = true;
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
        case Storage_Facilities: {  // 1029
            pin.isStorage = true;
            /* nothing to do yet */
            // contents for testing
            pin.contents[2390] = 100;  //Electrolytes
        } break;
        case Mercenary_Bases:
        case Capsuleer_Bases: {
            pin.isBase = true;
            /* nothing to do yet */
        } break;
    }

    iRef->Move(m_pSE->GetID(), flagPlanetSurface, true);
    iRef->ChangeSingleton(true);
    // cannot change attributes on PI items.....  :(
    //iRef->SetAttribute(AttrCpuLoad, m_cpu);
    //iRef->SetAttribute(AttrPowerLoad, m_pg);

    ccPin->pins[iRef->itemID()] = pin;

    if (groupID != Command_Centers)
        tempPinIDs.insert(std::pair<uint8, uint32>(pinID, iRef->itemID()));     // save map of tempID to itemID - this handles the stacked-calls from client to use real itemIDs

    _log(PLANET__TRACE, "Colony::CreatePin() - Created pin for %s(%u)", iRef->itemName().c_str(), iRef->itemID());

    /** @todo  do the construction cost thing...
     / /take the money, send wallet blink *event record the transaction in their journal.
     std::string reason = "DESC:  ";
     reason += itoa(call.client->GetStationID());
     AccountService::TranserFunds(
         call.client->GetCharacterID(),
         args.stationID, // fix this to get ownerID
         money,
         reason.c_str(),
         Journal::EntryType::MarketTransaction,
         call.client->GetStationID(),
         Account::KeyType::Cash);
         */
}

void Colony::CreateLink(uint32 src, uint32 dest, uint16 level) {
    if (IsTempPinID(src) and (tempPinIDs.size() > 0)) {
        std::map<uint8, uint32>::iterator itr = tempPinIDs.find(src);
        if (itr != tempPinIDs.end())
            src = itr->second;
    }
    if (IsTempPinID(dest) and (tempPinIDs.size() > 0)) {
        std::map<uint8, uint32>::iterator itr = tempPinIDs.find(dest);
        if (itr != tempPinIDs.end())
            dest = itr->second;
    }
    ItemData data(2280, m_client->GetCharacterID(), 0, flagAutoFit, 1);
    InventoryItemRef iRef = sItemFactory.SpawnItem(data);
    iRef->Move(m_pSE->GetID(), flagPlanetSurface, true);
    iRef->SaveItem();

    PI_Link link;
        link.state = PINSTATE_IDLE;
        link.level = level;
        link.endpoint1 = src;
        link.endpoint2 = dest;
        link.typeID = 2280; // Only link type in the game.
    ccPin->links[iRef->itemID()] = link;
    _log(PLANET__TRACE, "Colony::CreateLink() - Created link - id:%u,  src:%u, dest:%u, level:%u", iRef->itemID(), src, dest, level);
}

void Colony::CreateRoute(uint16 routeID, uint32 typeID, uint32 qty, PyList* path) {
    // routeID is sent as tempID like pins.
    std::list<uint32> list1;
    list1.clear();
    for (size_t i = 0; i < path->size(); ++i) {
        if (path->GetItem(i)->IsTuple())
            list1.push_back(path->GetItem(i)->AsTuple()->GetItem(1)->AsInt()->value());
        else if (path->GetItem(i)->IsInt())
            list1.push_back(path->GetItem(i)->AsInt()->value());
        else
            _log(PLANET__ERROR, "Colony::CreateRoute() - List item type unrecognized: %s", path->GetItem(1)->TypeString());
    }

    if (tempPinIDs.size() > 0) {
        std::list<uint32> list2;
        list2.clear();
        for (auto cur : list1) {
            if (IsTempPinID(cur)) {
                std::map<uint8, uint32>::iterator itr = tempPinIDs.find(cur);
                if (itr != tempPinIDs.end())
                    list2.push_back(itr->second);
            } else
                list2.push_back(cur);
        }
        list1.clear();
        list1 = list2;
    }

    PI_Route route;
        route.state = PINSTATE_IDLE;
        route.priority = RoutePriorityNorm;
        route.commodityTypeID = typeID;
        route.commodityQuantity = qty;
        route.srcPinID = list1.front();
        route.destPinID = list1.back();
        route.path = list1;

    routeID = m_db.SaveRoute(m_colonyID, route);
    ccPin->routes[routeID] = route;
    _log(PLANET__TRACE, "Colony::CreateRoute() - Created route id %u for %u of typeID %u, making %u hops.", routeID, qty, typeID, (uint32)path->size());

    // route has been created and added to list.  check for materials being moved, and if source has the mat, remove qty and send to dest.
    std::map<uint32, PI_Pin>::iterator source = ccPin->pins.find(route.srcPinID);
    if (source != ccPin->pins.end()) {
        // remove contents from storage pin
        uint32 amount = route.commodityQuantity;
        std::map<uint16, uint32>::iterator itr = source->second.contents.find(route.commodityTypeID);
        if (itr != source->second.contents.end()) {
            if (itr->second > amount)
                itr->second -= amount;
            else
                source->second.contents.erase(itr);
            source->second.update = true;
        } else {
            // this material wasnt found in source container....cant move what we aint got..
            return;
        }

        // add contents to dest pin if we have any
        std::map<uint32, PI_Pin>::iterator dest = ccPin->pins.find(route.destPinID);
        if (dest != ccPin->pins.end()) {
            itr = dest->second.contents.find(route.commodityTypeID);
            if (itr != dest->second.contents.end())
                itr->second += amount;
            else
                dest->second.contents[route.commodityTypeID] = amount;
            dest->second.update = true;
            // we have received a material from this route. enable check for all required materials for this Schematic
            dest->second.hasReceivedInputs = true;
        }
        // make error for dest pin not found?
    }
}

void Colony::UpgradeCommandCenter(uint32 pinID, int8 level) {
    ccPin->level = level;
    std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(pinID);
    if (itr != ccPin->pins.end()) {
        itr->second.level = level;
        m_db.SaveCCLevel(pinID, level);
        _log(PLANET__TRACE, "Colony::UpgradeCommandCenter() - Upgraded Command Center %u to level:%u", pinID, level);
    } else
        _log(PLANET__ERROR, "Colony::UpgradeCommandCenter() - pinID %u not found in ccPin.pins map", pinID);
}

void Colony::UpgradeLink(uint32 src, uint32 dest, uint8 level)
{
    /** @todo  figure out how to do this one....
    std::map<uint32, PI_Link>::iterator itr = ccPin->links.find(src);
    */
}

void Colony::RemovePin(uint32 pinID)
{
    ccPin->pins.erase(pinID);
    ccPin->plants.erase(pinID);  // may or may not be here.
    m_db.RemovePin(pinID);
    _log(PLANET__TRACE, "Colony::RemovePin() - Removed pin %u", pinID);
}

void Colony::RemoveLink(uint32 src, uint32 dest)
{
    std::map<uint32, PI_Link>::iterator itr = ccPin->links.begin();
    for (; itr != ccPin->links.end(); ++itr) {
        if (itr->second.endpoint1 == src) {
            if (itr->second.endpoint2 == dest) {
                _log(PLANET__TRACE, "Colony::RemoveLink() - Removing linkID %u - src: %u, dest: %u", itr->first, src, dest);
                m_db.RemoveLink(itr->first);
                ccPin->links.erase(itr);
                return;
            }
        }
    }
}

void Colony::RemoveRoute(uint16 routeID)
{
    ccPin->routes.erase(routeID);
    m_db.RemoveRoute(routeID);
    _log(PLANET__TRACE, "Colony::RemoveRoute() - Removed route: %u", routeID);
}

void Colony::AddExtractorHead(uint32 ecuID, uint16 headID, double latitude, double longitude)
{
    std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(ecuID);
    if (itr != ccPin->pins.end()) {
        m_newHead = true;
        tempECUs.push_back(ecuID);
        PI_Heads head;
            head.typeID = itr->second.schematicID;
            head.ecuPinID = ecuID;
            head.latitude = latitude;
            head.longitude = longitude;
        itr->second.heads[headID] = head;
    } else
        _log(PLANET__ERROR, "Colony::AddExtractorHead() - ecuID %u not found in ccPin.pins map", ecuID);
}

void Colony::MoveExtractorHead(uint32 ecuID, uint16 headID, double latitude, double longitude)
{
    std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(ecuID);
    if (itr != ccPin->pins.end()) {
        std::map<uint16, PI_Heads>::iterator head = itr->second.heads.find(headID);
        if (head != itr->second.heads.end()) {
            m_newHead = true;
            tempECUs.push_back(ecuID);
            // find head and update....
            head->second.latitude = latitude;
            head->second.longitude = longitude;
        } else
            _log(PLANET__ERROR, "Colony::MoveExtractorHead() - headID %u not found in pin.heads map", headID);
    } else
        _log(PLANET__ERROR, "Colony::MoveExtractorHead() - ecuID %u not found in ccPin.pins map", ecuID);
}

void Colony::KillExtractorHead(uint32 ecuID, uint16 headID)
{
    std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(ecuID);
    if (itr != ccPin->pins.end())
        itr->second.heads.erase(headID);
    else
        _log(PLANET__ERROR, "Colony::KillExtractorHead() - ecuID %u not found in ccPin.pins map", ecuID);
}

void Colony::SetSchematic(uint32 pinID, uint16 schematicID)
{
    if (IsTempPinID(pinID) and (tempPinIDs.size() > 0)) {
        std::map<uint8, uint32>::iterator itr = tempPinIDs.find(pinID);
        if (itr != tempPinIDs.end())
            pinID = itr->second;
    }

    std::map<uint32, PI_Plant>::iterator itr = ccPin->plants.find(pinID);
    if (itr != ccPin->plants.end()) {
        if (schematicID) {
            sPIDataMgr.GetSchematicData(schematicID, itr->second.data);
            itr->second.state = PINSTATE_IDLE;
            itr->second.order = GetProductLevel(itr->second.data.outputType);
            itr->second.cycleTime = itr->second.data.cycleTime * 10000000L;
            itr->second.installTime = GetFileTimeNow();
            itr->second.expiryTime = itr->second.data.cycleTime + itr->second.installTime;
            itr->second.qtyPerCycle = itr->second.data.outputQty;
            itr->second.schematicID = schematicID;
            itr->second.lastRunTime = GetFileTimeNow();
            itr->second.hasReceivedInputs = false;
            itr->second.receivedInputsLastCycle = false;
            m_pLevel = (uint8)EvE::min(m_pLevel, itr->second.order);
        } else {
            PI_Schematic data;
            itr->second.data = data;
            itr->second.state = PINSTATE_IDLE;
            itr->second.cycleTime = 0;
            itr->second.expiryTime = 0;
            itr->second.qtyPerCycle = 0;
            itr->second.installTime = 0;
            itr->second.lastRunTime = 0;
            itr->second.schematicID = 0;
            itr->second.hasReceivedInputs = false;
            itr->second.receivedInputsLastCycle = false;
        }
        _log(PLANET__TRACE, "Colony::SetSchematic() - Set Schematic %u in plantID %u", schematicID, pinID);
    } else
        _log(PLANET__ERROR, "Colony::SetSchematic() - plantID %u not found in ccPin.plants map", pinID);
}

void Colony::InstallProgram(uint32 ecuID, uint16 typeID, float headRadius)
{
    std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(ecuID);
    if (itr != ccPin->pins.end()) {
        if (typeID and (itr->second.programType != typeID)) {
            _log(PLANET__ERROR, "Colony::InstallProgram() - typeID %u does not match previously saved program", typeID);
        } else if (!typeID) {
            // uninstall program
            itr->second.cycleTime = 0;
            itr->second.programType = 0;
            itr->second.expiryTime = 0;
            itr->second.qtyPerCycle = 0;
            itr->second.installTime = 0;
            itr->second.lastRunTime = 0;
            itr->second.schematicID = 0;
            for (auto cur : itr->second.heads)
                cur.second.typeID = 0;
        } else {
            itr->second.headRadius = headRadius;
            itr->second.installTime = GetFileTimeNow();
            itr->second.lastRunTime = GetFileTimeNow();
        }
    } else
        _log(PLANET__ERROR, "Colony::InstallProgram() - ecuPinID %u not found in ccPin.pins map", ecuID);
}

void Colony::SetProgramResults(uint32 ecuID, uint16 typeID, uint16 numCycles, float headRadius, float cycleTime)
{
    std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(ecuID);
    if (itr != ccPin->pins.end()) {
        itr->second.cycleTime = (cycleTime * 60 * 60 * 10000000L);
        itr->second.programType = typeID;
        itr->second.expiryTime = ((cycleTime * numCycles) * 60 * 60 * 10000000L) + GetFileTimeNow();
        itr->second.headRadius = headRadius;
        if (itr->second.qtyPerCycle < 1) {
            InventoryItemRef iRef = sItemFactory.GetItem(ecuID);
            itr->second.qtyPerCycle = iRef->GetAttribute(AttrPinExtractionQuantity).get_int();
        }
        itr->second.schematicID = GetHeadType(sItemFactory.GetItem(ecuID)->typeID(), typeID);
    } else
        _log(PLANET__ERROR, "Colony::SetProgramResults() - ecuPinID %u not found in ccPin.pins map", ecuID);
}

PyDict* Colony::TransferCommodities(uint32 srcID, uint32 destID, std::map< uint16, uint32 > items)
{
    std::map<uint32, PI_Pin>::iterator src = ccPin->pins.find(srcID);
    if (src == ccPin->pins.end()) {
        return nullptr; // make error and exit.
    }
    std::map<uint32, PI_Pin>::iterator dest = ccPin->pins.find(destID);
    if (dest == ccPin->pins.end()) {
        return nullptr; // make error and exit.
    }

    // capacities are checked in client.  procede with xfer
    for (auto cur : items) {
        std::map<uint16, uint32>::iterator srcItr = src->second.contents.find(cur.first), destItr = dest->second.contents.find(cur.first);
        if (srcItr->second > cur.second)
            srcItr->second -= cur.second;
        else
            src->second.contents.erase(srcItr);
        if (destItr != dest->second.contents.end())
            destItr->second += cur.second;
        else
            dest->second.contents[cur.first] = cur.second;
    }

    // call update to process received mats and xfer per route if needed
    bool update = false;
    ProcessPlants(update);
    if (update)
        m_db.UpdatePinTimes(ccPin);

    ccPin->currentSimTime = GetFileTimeNow();
    // simTime = time to stop (currentSimTime), sourceRunTime = lastRunTime
    PyDict* args(new PyDict());
    args->SetItem("simTime", new PyLong(ccPin->currentSimTime));
    src->second.lastRunTime = GetFileTimeNow() + (EvE::Time::Minute * 15);  // arbitrary 15 minute delivery time
    args->SetItem("sourceRunTime", new PyLong(src->second.lastRunTime));

    return args;
}

void Colony::LaunchCommodities(uint32 pinID, std::map< uint16, uint32 >& items)
{
    // this will export items from CC to jetcan in space.
    // launchpad (Spaceport) xfers items to/from customs office
    std::map<uint32, PI_Pin>::iterator pin = ccPin->pins.find(pinID);
    if (pin != ccPin->pins.end()) {
        // first - create jetcan, add to system, and put in orbit around planet
        // NOTE:  PI launcheds have 5d timers
        /** @todo check capacities before adding items */
        SystemManager* pSysMgr(m_pSE->SystemMgr());
        GPoint location(pSysMgr->GetSE(m_pSE->GetID())->GetPosition());
        location.MakeRandomPointOnSphere(m_pSE->GetRadius() + 1000000);   //1000km orbit for launch can
        ItemData canData(EVEDB::invTypes::typePlanetaryLaunchContainer,
                        m_client->GetCharacterID(),  // owner is Character
                        pSysMgr->GetID(),
                        flagAutoFit,
                        "PI Commodities Container",
                        location);

        CargoContainerRef contRef = sItemFactory.SpawnCargoContainer(canData);
        if (!contRef)
            if (m_client->CanThrow())
                throw PyException(MakeCustomError("Unable to spawn item of type %u.", 2263));
            else
                ; //  make error here?

        FactionData contData;
            contData.allianceID = m_client->GetAllianceID();
            contData.corporationID = m_client->GetCorporationID();
            contData.factionID = m_client->GetWarFactionID();
            contData.ownerID = m_client->GetCharacterID();
        // create new container SE
        ContainerSE* cSE = new ContainerSE(contRef, *m_svcMgr, pSysMgr, contData);
        contRef->SetMySE(cSE);      // item-to-entity internal interface
        //cSE->AnchorContainer();     // avoid GC checks on this container  -no.  has 5d timer set
        pSysMgr->AddEntity(cSE);

        /* second - reduce qtys in source container (CC pin.contents in this case)
         * create actual item (previously only virtual)
         * add to container
         * calculate taxes on items
         * charge char taxes upon launch
         */
        double cost = 0;
        for (auto cur : items) {
            std::map<uint16, uint32>::iterator cont = pin->second.contents.find(cur.first);
            /** @todo  check for qtys here */
            if (cont != pin->second.contents.end()) {
                cont->second -= cur.second;
                if (cont->second <= 0)
                    pin->second.contents.erase(cont);   // remove item from pin.contents if launching entire qty.
            } // make error if item not found in pin.contents?

            switch (GetProductLevel(cur.first)) {
                case 0:     cost += (    0.15 * cur.second);    break;
                case 1:     cost += (    1.14 * cur.second);    break;
                case 2:     cost += (    9.00 * cur.second);    break;
                case 3:     cost += (  900.00 * cur.second);    break;
                case 4:     cost += (75000.00 * cur.second);    break;
            }
            ItemData iData(cur.first, m_client->GetCharacterID(), 0, flagAutoFit, cur.second);
            InventoryItemRef iRef = sItemFactory.SpawnItem(iData);
            iRef->Move(cSE->GetID(), flagAutoFit, true);
        }

        //take the money, send wallet blink event record the transaction in their journal.
        std::string reason = "DESC:  Exporting items from ";
        reason += m_pSE->GetName();
        AccountService::TranserFunds(
                    m_client->GetCharacterID(),
                    ownerUnknown,  // not sure who to send this to
                    cost,
                    reason.c_str(),
                    Journal::EntryType::PlanetaryExportTax,
                    m_pSE->GetID(),
                    Account::KeyType::Cash);

        contRef->SaveItem();
        pin->second.lastLaunchTime = GetFileTimeNow();

        // third - create db entry for launch
        m_db.SaveLaunch(contRef->itemID(), m_client->GetCharacterID(), pSysMgr->GetID(), m_pSE->GetID(), location);

        // update colony
        Update(true);   // must update and save CC's lastLaunchTime here
    } else
        _log(PLANET__ERROR, "Colony::LaunchCommodities() - pinID %u not found in ccPin.pins map", pinID);
}

/** @todo  add import/export taxes
 *  GetProductLevel(typeID) will return PLevel of item.
 *  use that to calculate cost for import/export operations
Product     Command Center Export Cost  Launchpad Export Cost   Launchpad Import Cost
    P0         15/m3 or .15/unit           10/m3 or .1/unit        5/m3 or .05/unit
    P1          3/m3 or 1.14/unit           2/m3 or .76/unit       1/m3 or .38/unit
    P2          9/m3 or 13.5/unit           6/m3 or 9/unit         3/m3 or 4.5/unit
    P3        150/m3 or 900/unit          100/m3 or 600/unit      50/m3 or 300/unit
    P4        750/m3 or 75k/unit          500/m3 or 50k/unit     250/m3 or 25k/unit
*/
void Colony::PrioritizeRoute()
{

}


PyTuple* Colony::GetPins()
{
    uint8 index = 0;
    PyTuple* pins(new PyTuple(ccPin->pins.size()));

    for (auto cur : ccPin->pins) {
        PyDict* dict(new PyDict());
        dict->SetItem("id", new PyInt(cur.first));
        dict->SetItem("typeID", new PyInt(cur.second.typeID));
        dict->SetItem("ownerID", new PyInt(cur.second.ownerID));
        dict->SetItem("latitude", new PyFloat(cur.second.latitude));
        dict->SetItem("longitude", new PyFloat(cur.second.longitude));
        dict->SetItem("lastRunTime", new PyLong(cur.second.lastRunTime));
        dict->SetItem("state", new PyInt(cur.second.state));
        dict->SetItem("level", new PyInt(cur.second.level));

        if (cur.second.isLaunchable)
            dict->SetItem("lastLaunchTime", new PyLong(cur.second.lastLaunchTime));

        if ((cur.second.isProcess) and (cur.second.schematicID)) {
            dict->SetItem("cycleTime", new PyLong(cur.second.cycleTime));
            dict->SetItem("schematicID", new PyInt(cur.second.schematicID));
            dict->SetItem("hasReceivedInputs", new PyBool(cur.second.hasReceivedInputs));
            dict->SetItem("receivedInputsLastCycle", new PyBool(cur.second.receivedInputsLastCycle));
        }

        PyDict* contents(new PyDict());
        contents->clear();
        if (cur.second.isStorage) {
            for (auto cur2 : cur.second.contents)
                contents->SetItem(new PyInt(cur2.first), new PyInt(cur2.second));
        }
        dict->SetItem("contents", contents);

        if (cur.second.isECU) {
            if (cur.second.installTime > 0) {
                dict->SetItem("cycleTime", new PyLong(cur.second.cycleTime));
                dict->SetItem("expiryTime", new PyLong(cur.second.expiryTime));
                dict->SetItem("headRadius", new PyFloat(cur.second.headRadius));
                dict->SetItem("installTime", new PyLong(cur.second.installTime));
                dict->SetItem("programType", new PyInt(cur.second.programType));
                dict->SetItem("qtyPerCycle", new PyFloat(cur.second.qtyPerCycle));
            }
            PyList* list = new PyList();
            list->clear();
            for (auto head : cur.second.heads) {
                PyTuple* tuple(new PyTuple(3));
                    tuple->SetItem(0, new PyInt(head.first));
                    tuple->SetItem(1, new PyFloat(head.second.latitude));
                    tuple->SetItem(2, new PyFloat(head.second.longitude));
                list->AddItem(tuple);
            }
            dict->SetItem("heads", list);
        }

        PyObject* obj(new PyObject("util.KeyVal", dict));
        pins->SetItem(index++, obj);
    }
    return pins;
}

PyTuple* Colony::GetLinks()
{
    uint8 index = 0;
    PyTuple* links(new PyTuple(ccPin->links.size()));

    for (auto cur : ccPin->links) {
        PyDict* dict(new PyDict());
            dict->SetItem("linkID", new PyInt(cur.first));                 // this is link itemID
            dict->SetItem("endpoint1", new PyInt(cur.second.endpoint1));
            dict->SetItem("endpoint2", new PyInt(cur.second.endpoint2));
            dict->SetItem("level", new PyInt(cur.second.level));
            dict->SetItem("typeID", new PyInt(cur.second.typeID));          // typeID 2280
        PyObject* obj(new PyObject("util.KeyVal", dict));
        links->SetItem(index++, obj);
    }
    return links;
}

PyTuple* Colony::GetRoutes()
{
    uint8 index = 0;
    PyTuple* routes(new PyTuple(ccPin->routes.size()));

    for (auto cur : ccPin->routes) {
        PyDict* dict(new PyDict());
            dict->SetItem("routeID", new PyInt(cur.first));                 // this is routeID (low number - assigned by client)
            dict->SetItem("commodityTypeID", new PyInt(cur.second.commodityTypeID));
            dict->SetItem("commodityQuantity", new PyInt(cur.second.commodityQuantity));

        PyList* list(new PyList());
        for (auto cur2 : cur.second.path)                               // path of pinIDs this route will follow
            list->AddItem(new PyInt(cur2));
        dict->SetItem("path", list);                                    // list of paths on this route

        PyObject* obj(new PyObject("util.KeyVal", dict));
        routes->SetItem(index++, obj);
    }
    return routes;
}

PyRep* Colony::GetColony()
{
    if (m_newHead) {
        for (auto cur : tempECUs) {
            std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(cur);
            if (itr != ccPin->pins.end())
                m_db.SaveHeads(m_colonyID, m_client->GetCharacterID(), cur, itr->second.heads);
            else
                _log(PLANET__ERROR, "Colony::GetColony()::SaveHeads() - headID %u not found in ccPin.pins map", cur);
        }
        tempECUs.clear();
        m_newHead = false;
    }

    Update();   // update colony before sending data.
    Save();     // save here after update.  this also saves routes and links   do we need to save on EVERY GetColony() call?

    PyDict* args(new PyDict());
        args->SetItem("pins", GetPins());
        args->SetItem("level", new PyInt(ccPin->level));
        args->SetItem("links", GetLinks());
        args->SetItem("routes", GetRoutes());
        args->SetItem("currentSimTime", new PyLong(ccPin->currentSimTime));
    PyObject* res(new PyObject("util.KeyVal", args));

    _log(PLANET__DEBUG, "Colony::GetColony() Dump");
    if (is_log_enabled(PLANET__GC_DUMP))
        res->Dump(PLANET__GC_DUMP, "    ");

    // reset tempPinID-to-newPinID map after command loop is completed and all new pins have been created.
    tempPinIDs.clear();

    std::map<uint32, PI_Pin>::iterator pin = ccPin->pins.find(m_colonyID);
    m_db.UpdatePlanetsForChar(m_pSE->SystemMgr()->GetID(), m_pSE->GetID(), m_client->GetCharacterID(), pin->second.typeID, ccPin->pins.size());

    return res;
}

void Colony::Update(bool updateTimes/*false*/)
{
    double profileStartTime = 0.0;
    //if (sConfig.debug.UseProfiling)
        profileStartTime = GetTimeUSeconds();

    m_procTime = GetFileTimeNow();
    /* loop thru process calls to update each pin to simulate production and logistics
     *  this will have to be fast, as there may/will be large time deltas between updates
     *  can loop each item to process for each time step (like i do for skill training)
     */
    // first process ecus for raw matls.
    ProcessECUs(updateTimes);
    // second process plants for production.
    ProcessPlants(updateTimes);

    m_procTime = 0;

    // update colony time to current time (based on this update, prior to sending out current colony status)
    ccPin->currentSimTime = GetFileTimeNow();

    // update current pin times
    if (updateTimes)
        m_db.UpdatePinTimes(ccPin);

    _log(PLANET__TRACE, "Colony::Update() - Update completed in %.3fus", GetTimeUSeconds() - profileStartTime);

    // profile timer for the colony updates
    if (sConfig.debug.UseProfiling)
        sProfile.AddTime(_colonyProfile, GetTimeUSeconds() - profileStartTime);
}

void Colony::ProcessECUs(bool& updateTimes)
{
    for (auto ecu : ccPin->pins) {
        if (ecu.second.isECU) {
            if ((ecu.second.expiryTime == 0 ) or (ecu.second.cycleTime == 0) or (ecu.second.expiryTime > m_procTime))
                continue;
            /** @todo  as i dont have data on planet resources, and am not tracking depletion, extraction qtys used here are
             * sent from the client during 'survey program' installation, and do not simulate the diminishing returns as shown in
             * the survey program. (testing diminishing returns @ 99%)
             * because of this, the values used here (and all subsquent processes) will be more than shown in client.
             */
            /** @note this is a simple process, as it only provides raw mats, simulating extraction from planet and
             *  shipped to storage or directly to plant for processing.
             * however, in the case of shipping directly to plant, we will have to store the mats in the plant queue
             * and wait for the ProcessPlants() call to use them, as this will avoid overcomplicating things,
             * but it could get messy later....
             */
            // dont loop...get #cycles to 'run' and set amounts accordingly
            double delta = m_procTime - ecu.second.lastRunTime;
            uint16 cycles = floor(delta / ecu.second.cycleTime);
            // first - see if this ecu has a route and move contents per route.  this will simulate aquisition of raw matls from heads to storage
            for (auto route : ccPin->routes) {
                // verify this route begins at this pin.
                if (route.second.srcPinID != ecu.first)
                    continue;
                // second - update current contents per route movement as noted above (there are no stored contents to update in the ECU)
                // get route destination pin and update qty
                std::map<uint32, PI_Pin>::iterator dest = ccPin->pins.find(route.second.destPinID);
                if (dest != ccPin->pins.end()) {
                    // contents are stored in each pin.  PI_Pin.contents(std::map<uint16, uint32>(typeID, qty))
                    std::map<uint16, uint32>::iterator itr = dest->second.contents.find(route.second.commodityTypeID);
                    /** @todo  check for available capy and adjust qty accordingly.
                    *       if dest cant hold entire xfer qty, xfer to full, and drop rest. (material loss)
                    */
                    if (itr != dest->second.contents.end())
                        itr->second += (route.second.commodityQuantity * cycles);
                    else
                        dest->second.contents[route.second.commodityTypeID] = (route.second.commodityQuantity * cycles);
                    route.second.commodityQuantity *= 0.99; // diminishing returns on extraction - not accurate, but POC
                    dest->second.update = true;
                } else {
                    _log(PLANET__ERROR, "Colony::ProcessECUs()::Routes() - Dest pinID %u not found in ccPin.pins map", route.second.destPinID);
                    continue;
                }
            }
            // third - reset cycle times.
            // set expiryTime to previous runtime plus cycleTime to simulate end of cycle
            ecu.second.expiryTime = ecu.second.lastRunTime + (ecu.second.cycleTime * cycles);
            // set lastRunTime to last-processed cycle's expire time.
            ecu.second.lastRunTime = ecu.second.expiryTime;
            updateTimes = true;
        }
    }
}

void Colony::ProcessPlants(bool& updateTimes)
{
    if (ccPin->plants.empty() or (m_pLevel < 1))
        return; // nothing to do...

    /** @note  generally-accepted design has plant input and output to/from silo(spaceport or storage) for input buffers and follows this guideline...
     * silo->plant->silo->plant->silo
     * however, there may be rare cases where colony is restricted or other design constraints limit routing and
     * plants must be linked together, where the output of one provides the direct input of the next, as follows...
     * silo->plant->plant->plant->silo
     * with plants as needed for production requirements of the colony.
     *
     * this will need to check for and be able to process both cases, and could be somewhat complicated.
     *
     * plants will have to be processed in order from p1 to p4 products, to provide input for downstream plants
     * this WILL have to loop for each cycle to correctly set inputs and outputs for each plant, and provide
     * positive material control (and be more realistic) per run.
     *
     */
    /** @note  plants are stored separate from other pins, to avoid the cycles and checks for plants in this call.
     * this will also avoid the unnecessary plant-specific data to be stored in std pins for all items
     */
    // m_pLevel is set to lowest 'P' level of produced items, and used to correctly order plant processing
    uint8 curCycle = m_pLevel;

    while (curCycle < 5) {
        for (auto plant : ccPin->plants) {
            // plants must be processed in order to correctly send products to downstream receipents.
            // this allows for both silo->plant->silo->plant and silo->plant->plant->plant->silo routing (or any combination of plant and silo routing)
            if (plant.second.order != curCycle)
                continue;
            if ((plant.second.cycleTime == 0) or (plant.second.expiryTime > m_procTime))
                continue;

            // first - current cycle is complete.  move finished product per route
            // second - check for available matls and begin production if able
            // both checks can be done in same loop.  they dont cross or overlap due to process order (according to curCycle)
            for (auto route : ccPin->routes) {
                // verify this route begins at this pin.  should be only ONE route here for this output
                // also check that plant was active last round, to produce product to send out
                if ((route.second.srcPinID == plant.first) and (plant.second.state == PinStates::PINSTATE_ACTIVE)) {
                    plant.second.state = PinStates::PINSTATE_IDLE;  // set to idle.  this may not be needed.
                    // get destination pin and update qty there for this round
                    std::map<uint32, PI_Pin>::iterator dest = ccPin->pins.find(route.second.destPinID);
                    if (dest != ccPin->pins.end()) {
                        // contents are stored in each pin.  PI_Pin.contents(std::map<uint16, uint32> typeID, qty)
                        if (dest->second.isStorage) {
                            //  if dest cant hold entire xfer qty, drop remainder in current pin contents (as opposed to loss)
                            ; /** @todo  create/set/implement storage capy for pin  */
                        }
                        std::map<uint16, uint32>::iterator itr = dest->second.contents.find(route.second.commodityTypeID);
                        if (itr != dest->second.contents.end())
                            itr->second += route.second.commodityQuantity;
                        else
                            dest->second.contents[route.second.commodityTypeID] = route.second.commodityQuantity;
                        dest->second.update = true;
                    }
                }
                if (route.second.destPinID == plant.first) {
                    // this route supplies this plant with input matls.
                    // verify supplier is NOT a plant or ECU here, as this was done in previous checks.
                    std::map<uint32, PI_Pin>::iterator source = ccPin->pins.find(route.second.srcPinID);
                    if (source != ccPin->pins.end()) {
                        if (!source->second.isStorage)
                            continue;
                        // source is storage.  continue with processing
                        // remove contents from storage pin
                        uint32 amount = route.second.commodityQuantity;
                        std::map<uint16, uint32>::iterator itr = source->second.contents.find(route.second.commodityTypeID);
                        if (itr != source->second.contents.end()) {
                            if (itr->second > amount)
                                itr->second -= amount;
                            else
                                source->second.contents.erase(itr);
                            source->second.update = true;
                        } else {
                            // this material wasnt found in source container....cant move what we aint got..
                            continue;
                        }

                        // add contents to plant pin
                        std::map<uint32, PI_Pin>::iterator dest = ccPin->pins.find(plant.first);
                        if (dest != ccPin->pins.end()) {
                            itr = dest->second.contents.find(route.second.commodityTypeID);
                            if (itr != dest->second.contents.end())
                                itr->second += amount;
                            else
                                dest->second.contents[route.second.commodityTypeID] = amount;
                            dest->second.update = true;
                            // we have received a material from this route. enable check for all required materials for this Schematic
                            plant.second.hasReceivedInputs = true;
                        }
                        // make error for plant pin not found?
                    }
                }
                //  current plant not found in routes.... not making an error here, but probably should for invalid route
            }
            // all plants have now sent and/or received product as defined by routing info for this cycle

            // third - process material requirements - see notes
            while (plant.second.hasReceivedInputs) {
                /*  if plant has received mats from routing (above), then check here for
                 * required qtys per Schematic which is found in plant.second.data.inputs map (std::map<uint16, uint16> {typeID, qty})
                 *  if all required qtys have been received, need to do following list...
                 * - remove mats from pin.contents
                 * - set receivedInputsLastCycle=true
                 *
                 *  if required mats are not present, set timers to 0 and receivedInputsLastCycle=false, which will deny processing and subsquent routing for this plant.
                 */
                std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(plant.first);
                for (auto mats : plant.second.data.inputs) {
                    // loop thru Schematic inputs to verify all required mats are present
                    std::map<uint16, uint32>::iterator contents = itr->second.contents.find(mats.first);
                    if (contents != itr->second.contents.end()) {
                        if (contents->second >= mats.second) {
                            contents->second -= mats.second;
                            if (contents->second < 1)
                                itr->second.contents.erase(contents);   // qty is 0.  remove item from map.
                        } else {
                            // this required material was not sufficient quantity for a full run.
                            // set hasReceivedInputs=false to break out of this loop.  no reason to continue at this point.
                            plant.second.hasReceivedInputs = false;
                            plant.second.receivedInputsLastCycle = false;
                            continue;
                        }
                    } else {
                        // this required material was not found in plant 'storage' (contents)
                        // set hasReceivedInputs=false to break out of this loop.  no reason to continue at this point.
                        plant.second.hasReceivedInputs = false;
                        plant.second.receivedInputsLastCycle = false;
                        continue;
                    }
                }
                // at this point, all required mats were found.
                // set hasReceivedInputs=false to break out of this loop  and receivedInputsLastCycle=true to process cycle times on following check
                plant.second.hasReceivedInputs = false;
                plant.second.receivedInputsLastCycle = true;
            }

            //  fourth - process cycle - see notes
            if (plant.second.receivedInputsLastCycle) {
                /* plant has received all required mats for a production run.
                 * set timers for runtimes and state to active
                 * this will allow routing (and subsquent process checks) on next loop, as defined in beginning of this loop
                 */
                plant.second.state = PinStates::PINSTATE_ACTIVE;
                plant.second.receivedInputsLastCycle = false;

                // update lastRunTime to expire time of this cycle
                plant.second.lastRunTime = plant.second.expiryTime;
                // set expiryTime to previous runtime plus cycleTime to simulate end of next cycle
                plant.second.expiryTime += plant.second.cycleTime;
                updateTimes = true;
            } else {
                plant.second.state = PinStates::PINSTATE_IDLE;
                // lastRunTime remains, as it doesnt change when plant is idle
                plant.second.expiryTime = 0;  // no run to process means no expiryTime either.
                updateTimes = true;
            }
        }
        ++curCycle;
    }
}

uint32 Colony::GetHeadType(uint16 ecuTypeID, uint16 programType)
{
    switch (ecuTypeID) {
        case 2848: {  //Barren ECU
            switch (programType) {
                case 2268: return 2409; //Barren Aqueous Liquid Extractor
                case 2267: return 2430; //Barren Base Metals Extractor
                case 2270: return 2435; //Barren Noble Metals Extractor
                case 2073: return 2449; //Barren Microorganisms Extractor
                case 2288: return 2459; //Barren Carbon Compounds Extractor
            }
        } break;
        case 3060: {  //Gas ECU
            switch (programType) {
                case 2268: return 2416; //Gas Aqueous Liquid Extractor
                case 2309: return 2424; //Gas Ionic Solutions Extractor
                case 2310: return 2426; //Gas Noble Gas Extractor
                case 2311: return 2427; //Gas Reactive Gas Extractor
                case 2267: return 2433; //Gas Base Metals Extractor
            }
        } break;
        case 3061: {  //Ice ECU
            switch (programType) {
                case 2268: return 2415; //Ice Aqueous Liquid Extractor
                case 2310: return 2423; //Ice Noble Gas Extractor
                case 2073: return 2432; //Ice Microorganisms Extractor
                case 2286: return 2438; //Ice Planktic Colonies Extractor
                case 2272: return 2441; //Ice Heavy Metals Extractor
            }
        } break;
        case 3062: {  //Lava ECU
            switch (programType) {
                case 2308: return 2418; //Lava Suspended Plasma Extractor
                case 2267: return 2428; //Lava Base Metals Extractor
                case 2272: return 2439; //Lava Heavy Metals Extractor
                case 2306: return 2442; //Lava Non-CS Crystals Extractor
                case 2307: return 2448; //Lava Felsic Magma Extractor
            }
        } break;
        case 3063: {  //Oceanic ECU
            switch (programType) {
                case 2268: return 2414; //Oceanic Aqueous Liquid Extractor
                case 2287: return 2458; //Oceanic Complex Organisms Extractor
                case 2286: return 2452; //Oceanic Planktic Colonies Extractor
                case 2288: return 2461; //Oceanic Carbon Compounds Extractor
                case 2073: return 2451; //Oceanic Microorganisms Extractor
            }
        } break;
        case 3064: {  //Plasma ECU
            switch (programType) {
                case 2308: return 2417; //Plasma Suspended Plasma Extractor
                case 2267: return 2429; //Plasma Base Metals Extractor
                case 2270: return 2434; //Plasma Noble Metals Extractor
                case 2272: return 2440; //Plasma Heavy Metals Extractor
                case 2306: return 2443; //Plasma Non-CS Crystals Extractor
            }
        } break;
        case 3067: {  //Storm ECU
            switch (programType) {
                case 2268: return 2413; //Storm Aqueous Liquid Extractor
                case 2308: return 2419; //Storm Suspended Plasma Extractor
                case 2309: return 2422; //Storm Ionic Solutions Extractor
                case 2310: return 2425; //Storm Noble Gas Extractor
                case 2267: return 2431; //Storm Base Metals Extractor
            }
        } break;
        case 3068: {  //Temperate ECU
            switch (programType) {
                case 2268: return 2412; //Temperate Aqueous Liquid Extractor
                case 2073: return 2450; //Temperate Microorganisms Extractor
                case 2287: return 2453; //Temperate Complex Organisms Extractor
                case 2288: return 2460; //Temperate Carbon Compounds Extractor
                case 2305: return 2462; //Temperate Autotrophs Extractor
            }
        } break;
    }
    _log(PLANET__ERROR, "Colony::GetHeadType() - Extractor typeID not found using ECU typeID: %u and Resource typeID: %u", ecuTypeID, programType);
    return 2412; //Temperate Aqueous Liquid Extractor  <<< as good a default as any...
}

uint8 Colony::GetProductLevel(uint16 typeID)
{
    switch (typeID) {
    // P0 - Raw Materials
        case  2267: //Base Metals
        case  2270: //Noble Metals
        case  2272: //Heavy Metals
        case  2306: //Non-CS Crystals
        case  2307: //Felsic Magma
        case  2268: //Aqueous Liquids
        case  2308: //Suspended Plasma
        case  2309: //Ionic Solutions
        case  2310: //Noble Gas
        case  2311: //Reactive Gas
        case  2073: //Microorganisms
        case  2286: //Planktic Colonies
        case  2287: //Complex Organisms
        case  2288: //Carbon Compounds
        case  2305: //Autotrophs
            return 0;

    // P1 - Basic Commodities
        case  2389: //Plasmoids
        case  2390: //Electrolytes
        case  2392: //Oxidizing Compound
        case  2393: //Bacteria
        case  2395: //Proteins
        case  2396: //Biofuels
        case  2397: //Industrial Fibers
        case  2398: //Reactive Metals
        case  2399: //Precious Metals
        case  2400: //Toxic Metals
        case  2401: //Chiral Structures
        case  3779: //Biomass
        case  9828: //Silicon
        case  3683: //Oxygen
        case  3645: //Water
            return 1;

    // P2 - Refined Commodities
        case    44: //Enriched Uranium
        case  2312: //Supertensile Plastics
        case  2317: //Oxides
        case  2319: //Test Cultures
        case  2321: //Polyaramids
        case  2327: //Microfiber Shielding
        case  2328: //Water-Cooled CPU
        case  2329: //Biocells
        case  2463: //Nanites
        case  3689: //Mechanical Parts
        case  3691: //Synthetic Oil
        case  3693: //Fertilizer
        case  3695: //Polytextiles
        case  3697: //Silicate Glass
        case  3725: //Livestock
        case  3775: //Viral Agent
        case  3828: //Construction Blocks
        case  9830: //Rocket Fuel
        case  9832: //Coolant
        case  9836: //Consumer Electronics
        case  9838: //Superconductors
        case  9840: //Transmitter
        case  9842: //Miniature Electronics
        case 15317: //Genetically Enhanced Livestock
            return 2;

    // P3 - Specialized Commodities
        case  2344: //Condensates
        case  2345: //Camera Drones
        case  2346: //Synthetic Synapses
        case  2348: //Gel-Matrix Biopaste
        case  2349: //Supercomputers
        case  2351: //Smartfab Units
        case  2352: //Nuclear Reactors
        case  2354: //Neocoms
        case  2358: //Biotech Research Reports
        case  2360: //Industrial Explosives
        case  2361: //Hermetic Membranes
        case  2366: //Hazmat Detection Systems
        case  2367: //Cryoprotectant Solution
        case  9834: //Guidance Systems
        case  9846: //Planetary Vehicles
        case  9848: //Robotics
        case 12836: //Transcranial Microcontrollers
        case 17136: //Ukomi Superconductors
        case 17392: //Data Chips
        case 17898: //High-Tech Transmitters
        case 28974: //Vaccines
            return 3;

    // P4 - Advanced Commodities
        case  2867: //Broadcast Node
        case  2868: //Integrity Response Drones
        case  2869: //Nano-Factory
        case  2870: //Organic Mortar Applicators
        case  2871: //Recursive Computing Module
        case  2872: //Self-Harmonizing Power Core
        case  2875: //Sterile Conduits
        case  2876: //Wetware Mainframe
            return 4;
    }
    _log(PLANET__ERROR, "Colony::GetProductLevel() - Commodity product level not found for typeID: %u", typeID);
    return 0;
}


/*
 *
    PlanetaryImportTax = 96,     // * Planet ID
    PlanetaryExportTax = 97,     // * Planet ID
    PlanetaryConstruction = 98,
    AttrImportTax = 1638,
    AttrExportTax = 1639,
    AttrImportTaxMultiplier = 1640,
    AttrExportTaxMultiplier = 1641,
    AttrnpcCustomsOfficeTaxRate = 1780,
    AttrdefaultCustomsOfficeTaxRate = 1781,

Command Center Properties
Level    Capy    CPU        PG           Upgrade Cost
0       500 m3  1,675 tf    6,000 MW       0  ISK
1       500 m3  7,057 tf    9,000 MW     580k ISK
2       500 m3  12,136 tf   12,000 MW    930k ISK
3       500 m3  17,215 tf   15,000 MW    1.2m ISK
4       500 m3  21,315 tf   17,000 MW    1.5m ISK
5       500 m3  25,415 tf   19,000 MW    2.1m ISK

Structure Properties
Name                        CPU         Power       Cost
Extractor Control Unit      400 tf      2600 MW      45m ISK
Extractor Head              110 tf      550 MW        0  ISK
Basic Industry Facility     200 tf      800 MW       75m ISK
Advanced Industry Facility  500 tf      700 MW      250m ISK
High-Tech Industry Facility 1100 tf     400 MW      525m ISK
Storage Facility            500 tf      700 MW      250m ISK
Space Port                  3600 tf     700 MW      900m ISK


 *
 * Link Requirements by Distance
 Distance   CPU         Power
2.5 km      16 tf       11 MW
10 km       18 tf       12 MW
20 km       20 tf       14 MW
50 km       26 tf       18 MW
100 km      36 tf       26 MW
200 km      56 tf       41 MW
500 km      116 tf      86 MW
1000 km     215 tf      160 MW
2000 km     416 tf      311 MW
5000 km     1016 tf     761 MW
40000 km    8016 tf     6001 MW


Link Upgrade Costs

Data on relative costs of upgrading the link capacity (uses a link that is 500km as a base):
Level   Capacity    CPU         Power
0       250 m3      116 tf      86 MW
1       500 m3      280 tf      183 MW
2       1000 m3     481 tf      291 MW
3       2000 m3     713 tf      407 MW
4       4000 m3     968 tf      528 MW
5       8000 m3     1245 tf     655 MW
6       16 km3      1542 tf     786 MW
7       32 km3      1855 tf     921 MW
8       64 km3      2185 tf     1059 MW
9       128 km3     2530 tf     1200 MW
10      256 km3     2889 tf     1344 MW

*/