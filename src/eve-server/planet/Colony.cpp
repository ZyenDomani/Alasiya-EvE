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
    Updates:    Allan
*/

#include "eve-server.h"
#include "PyServiceMgr.h"
#include "Client.h"
#include "inventory/ItemType.h"
#include "planet/Colony.h"
#include "Planet.h"
// for launching shit...
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
 */
Colony::Colony(PyServiceMgr* mgr, Client* pclient,  SystemEntity* pSE)
:m_svcMgr(mgr),
m_client(pclient),
m_pSE(pSE->GetPlanetSE()),
ccPin(new PI_CCPin())
{
    tempPinIDs.clear();

/*
    2267    Base Metals
    2270    Noble Metals
    2272    Heavy Metals
    2306    Non-CS Crystals
    2307    Felsic Magma
    2268    Aqueous Liquids
    2308    Suspended Plasma
    2309    Ionic Solutions
    2310    Noble Gas
    2311    Reactive Gas
    2073    Microorganisms
    2286    Planktic Colonies
    2287    Complex Organisms
    2288    Carbon Compounds
    2305    Autotrophs

 * Basic_Commodities
    2389    Plasmoids
    2390    Electrolytes
    2392    Oxidizing Compound
    2393    Bacteria
    2395    Proteins
    2396    Biofuels
    2397    Industrial Fibers
    2398    Reactive Metals
    2399    Precious Metals
    2400    Toxic Metals
    2401    Chiral Structures
    3779    Biomass
    9828    Silicon
*/
}

Colony::~Colony()
{
    SafeDelete(ccPin);
}

void Colony::Init()
{
    // check for and load colony if the char has one on this planet
    if (m_db.LoadColony(m_client->GetCharacterID(), m_pSE->GetID(), ccPin))
        Load();

    if (m_loaded)
        Update();
}

void Colony::Load()
{
    m_db.LoadPins(ccPin->ccPinID, ccPin->pins);
    // now, lets see if there are any heads in the ECU.
    //  if so, go thru the bullshit of populating their data in the ecu (their pin data is loaded from db)
    for (auto cur : ccPin->pins) {
        if (cur.second.isECU) {
            if (cur.second.heads.size() > 0) {
                std::map<uint32, PI_Heads>::iterator itr = cur.second.heads.begin();
                for (; itr != cur.second.heads.end(); itr++) {
                    std::map<uint32, PI_Pin>::iterator itr2 = ccPin->pins.find(itr->first);
                    if (itr2 != ccPin->pins.end()) {
                        itr->second.typeID = itr2->second.typeID;
                        itr->second.ecuPinID = cur.first;
                        itr->second.latitude = itr2->second.latitude;
                        itr->second.longitude = itr2->second.longitude;
                    } else
                        _log(PLANET__ERROR, "Colony::Load() - pinID %u not found in ccPin.pins map", itr->first);
                }
            }
        }
    }

    if (ccPin->ccPinID) {
        m_loaded = true;
        ccPin->currentSimTime = Win32TimeNow();
    }
}

void Colony::Save()
{
    /** @todo maybe separate these saves */
    m_db.SavePins(ccPin);
    m_db.SaveLinks(ccPin);
    m_db.SaveRoutes(ccPin);
}

void Colony::Process()
{
    // not sure what im gonna do here yet.
    //  call Update() on a timer?  -- right now, Update() is called on Init() and before GetColony()
}

void Colony::AbandonColony()
{
    /** @todo  go thru entire pinMap and delete each itemRef.  */
    InventoryItemRef iRef = m_svcMgr->item_factory->GetItem(m_colonyID);
    iRef->Delete();
    m_db.DeleteColony(m_colonyID, m_pSE->GetID(), m_client->GetCharacterID());
    SafeDelete(ccPin);
    ccPin = new PI_CCPin();
    m_colonyID = 0;
}

void Colony::CreateCommandPin(uint32 pinID, uint32 typeID, double latitude, double longitude) {
    m_colonyID = pinID;
    ccPin->ccPinID = pinID;
    m_db.SaveCommandCenter(pinID, m_client->GetCharacterID(), m_pSE->GetID(), typeID, latitude, longitude);
    ccPin->level = PinLevel0;
    ccPin->currentSimTime = Win32TimeNow();
    CreatePin(EVEDB::invGroups::Command_Centers, pinID, typeID, latitude, longitude);
}

void Colony::CreatePin(uint32 groupID, uint32 pinID, uint32 typeID, double latitude, double longitude) {
    /** @todo will have to write code for effects and checks for pg/cpu/m3/etc for all of these.  */
    using namespace EVEDB::invGroups;
    PI_Pin pin;
    InventoryItemRef iRef = InventoryItemRef();
    if (groupID != Command_Centers) {
        // type, owner, location, flag, qty
        ItemData data(typeID, m_client->GetCharacterID(), m_pSE->GetID(), flagAutoFit, 1);
        iRef = m_svcMgr->item_factory->SpawnItem(data);

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
        iRef = m_svcMgr->item_factory->GetItem(m_colonyID);
        if (iRef->quantity() > 1) {
            // check for stack of CC items, and split as needed
            ItemData data(typeID, m_client->GetCharacterID(), 0, flagAutoFit, iRef->quantity() -1);
            InventoryItemRef iRef2 = m_svcMgr->item_factory->SpawnItem(data);
            iRef2->Move(m_client->GetShipID(), flagCargoHold);
            iRef->SetQuantity(1, false);
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
    pin.heads.clear();
    pin.contents.clear();

    switch(groupID) {
        case Command_Centers: {     // 1027
            pin.isLaunchable = true;
            pin.isCommandCenter = true;
            pin.lastLaunchTime = Win32TimeNow();
            pin.contents[2390] = 100;  //Electrolytes
            pin.contents[2392] = 100;  //Oxidizing Compound
            pin.contents[2393] = 100;  //Bacteria
            pin.contents[2395] = 100;  //Proteins
        } break;
        case  Processors: {         // 1028
            pin.isProcess = true;
        } break;
        case Extractor_Control_Units: { // 1063
            pin.isECU = true;
            pin.qtyPerCycle = (int16)iRef->GetAttribute(AttrPinExtractionQuantity).get_int();
        } break;
        case Spaceports:{   // 1030
            pin.isLaunchable = true;
            pin.lastLaunchTime = Win32TimeNow();
        } break;
        case Extractors: {  // 1026
            pin.isExtractor = true;
            pin.installTime = 0;
            /* nothing to do yet */
        } break;
        case Planetary_Links: { // 1036
            pin.isLink = true;
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
        } break;
        case Mercenary_Bases:
        case Capsuleer_Bases: {
            pin.isBase = true;
            /* nothing to do yet */
        } break;
    }

    iRef->Move(m_pSE->GetID(), flagPlanetSurface);
    iRef->ChangeSingleton(true);
    // cannot change attributes on PI items.....  :(
    //iRef->SetAttribute(AttrCpuLoad, m_cpu);
    //iRef->SetAttribute(AttrPowerLoad, m_pg);

    ccPin->pins[iRef->itemID()] = pin;

    if (groupID != Command_Centers)
        tempPinIDs.insert(std::pair<uint8, uint32>(pinID, iRef->itemID()));     // save map of tempID to itemID - this handles the stacked-calls from client to use real itemIDs

    _log(PLANET__TRACE, "Colony::CreatePin() - Created pin for %s(%u)", iRef->itemName().c_str(), iRef->itemID());
}

void Colony::CreateLink(uint32 src, uint32 dest, uint32 level) {
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
    InventoryItemRef iRef = m_svcMgr->item_factory->SpawnItem(data);
    iRef->Move(m_pSE->GetID(), flagPlanetSurface);
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

void Colony::CreateRoute(uint8 routeID, uint32 typeID, uint32 qty, PyList* path) {
    std::list<uint32> list1;
    list1.clear();
    for (size_t i = 0; i < path->size(); i++) {
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
    } else
        _log(PLANET__INFO, "Colony::CreateRoute() - tempPinIDs is empty.  This may not be an error.");

    PI_Route route;
        route.id = routeID;
        route.state = PINSTATE_IDLE;
        route.priority = RoutePriorityNorm;
        route.commodityTypeID = typeID;
        route.commodityQuantity = qty;
        route.path = list1;
    ccPin->routes[list1.back()] = route;
    _log(PLANET__TRACE, "Colony::CreateRoute() - Created route id %u for %u of typeID %u, making %u hops.", routeID, qty, typeID, (uint32)path->size());
}

void Colony::UpgradeCommandCenter(uint32 pinID, uint8 level) {
    ccPin->level = level;
    std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(pinID);
    if (itr != ccPin->pins.end()) {
        itr->second.level = level;
        m_db.SaveCCLevel(pinID, level);
        _log(PLANET__TRACE, "Colony::UpgradeCommandCenter() - Upgraded Command Center %u to level:%u", pinID, level);
    } else
        _log(PLANET__ERROR, "Colony::UpgradeCommandCenter() - pinID %u not found in ccPin.pins map", pinID);
}

void Colony::UpgradeLink(uint32 linkID, uint8 level)
{
    std::map<uint32, PI_Link>::iterator itr = ccPin->links.find(linkID);
    if (itr != ccPin->links.end()) {
        itr->second.level = level;
        m_db.SaveLinkLevel(linkID, level);
        _log(PLANET__TRACE, "Colony::UpgradeLink() - Upgraded Link %u to level %u", linkID, level);
    } else
        _log(PLANET__ERROR, "Colony::UpgradeLink() - linkID %u not found in ccPin.links map", linkID);
}

void Colony::RemovePin(uint32 pinID)
{
    ccPin->pins.erase(pinID);
    m_db.RemovePin(pinID);
    _log(PLANET__TRACE, "Colony::RemovePin() - Removed pin %u", pinID);
}

void Colony::RemoveLink(uint32 linkID)
{
    ccPin->links.erase(linkID);
    m_db.RemoveLink(linkID);
    _log(PLANET__TRACE, "Colony::RemoveLink() - Removed link %u", linkID);
}

void Colony::RemoveRoute(uint32 routeID)
{
    ccPin->routes.erase(routeID);
    m_db.RemoveRoute(routeID);
    _log(PLANET__TRACE, "Colony::RemoveRoute() - Removed route: %u", routeID);
}

void Colony::CreateExtractorHead()
{
    m_newHead = false;

    std::map<uint32, PI_Pin> ecuPins;
    std::map<uint32, PI_Pin>::iterator itr, itr3;
    for (auto cur : tempHeadIDs) {
        itr = ccPin->pins.find(cur.second.ecuPinID);
        if (itr != ccPin->pins.end()) {
            ecuPins.insert(std::pair<uint32, PI_Pin>(itr->first, itr->second));
            CreatePin(m_svcMgr->item_factory->GetType(itr->second.schematicID)->groupID(), cur.first, itr->second.schematicID, cur.second.latitude, cur.second.longitude);

            std::map<uint8, uint32>::iterator itr2 = tempPinIDs.find(cur.first);
            if (itr2 != tempPinIDs.end()) {
                PI_Heads args;
                    args.ecuPinID = cur.second.ecuPinID;
                    args.typeID = itr->second.schematicID;
                    args.latitude = cur.second.latitude;
                    args.longitude = cur.second.longitude;
                    args.qtyPerCycle = cur.second.qtyPerCycle;
                itr->second.heads[itr2->second] = args;
                // set info in extractor pinID
                itr3 = ccPin->pins.find(itr2->second);
                if (itr3 != ccPin->pins.end()) {
                    itr3->second.typeID = itr->second.schematicID;
                    itr3->second.cycleTime = itr->second.cycleTime;
                    itr3->second.resTypeID = itr->second.resTypeID;
                    itr3->second.expiryTime = itr->second.expiryTime;
                    itr3->second.headRadius = itr->second.headRadius;
                    itr3->second.installTime = itr->second.installTime;
                    itr3->second.lastRunTime = itr->second.lastRunTime;
                    itr3->second.qtyPerCycle = itr->second.qtyPerCycle;
                }
            }
        } else
            _log(PLANET__ERROR, "Colony::CreateExtractorHead() - ecuPinID %u not found in ccPin.pins map", cur.second.ecuPinID);
    }

    for (auto cur : ecuPins)
        m_db.SaveHeads(cur.second.heads);

    tempHeadIDs.clear();
}

void Colony::AddExtractorHead(uint32 ecuID, uint32 pinID, double latitude, double longitude)
{
    m_newHead = true;

    PI_Heads head;
        head.ecuPinID = ecuID;
        head.latitude = latitude;
        head.longitude = longitude;
    tempHeadIDs[pinID] = head;
}

void Colony::MoveExtractorHead(uint32 ecuID, uint32 pinID, double latitude, double longitude)
{
    if (IsTempPinID(pinID) and (tempHeadIDs.size() > 0)) {
        std::map<uint8, PI_Heads>::iterator itr = tempHeadIDs.find(pinID);
        if (itr != tempHeadIDs.end()) {
            itr->second.latitude = latitude;
            itr->second.longitude = longitude;
        } else
            _log(PLANET__ERROR, "Colony::MoveExtractorHead() - pinID %u not found in tempHeadIDs map", pinID);
    } else {
        std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(pinID);
        if (itr != ccPin->pins.end()) {
            itr->second.latitude = latitude;
            itr->second.longitude = longitude;
        } else
            _log(PLANET__ERROR, "Colony::MoveExtractorHead() - pinID %u not found in ccPin.pins map", pinID);
    }
}

void Colony::SetSchematic(uint32 pinID, uint8 schematicID)
{
    if (IsTempPinID(pinID) and (tempPinIDs.size() > 0)) {
        std::map<uint8, uint32>::iterator itr = tempPinIDs.find(pinID);
        if (itr != tempPinIDs.end())
            pinID = itr->second;
    }
    uint32 cycleTime = 0;  // get cycleTime from Schematic attribs

    std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(pinID);
    if (itr != ccPin->pins.end()) {
        itr->second.state = PINSTATE_ACTIVE;
        itr->second.expiryTime = (cycleTime * 60 * 60 * 10000000L) + Win32TimeNow();
        itr->second.schematicID = schematicID;
        itr->second.installTime = Win32TimeNow();
        itr->second.lastRunTime = Win32TimeNow();
        itr->second.hasReceivedInputs = false;
        itr->second.receivedInputsLastCycle = false;
        _log(PLANET__TRACE, "Colony::SetSchematic() - Set Schematic %u in pinID %u", schematicID, pinID);
    } else
        _log(PLANET__ERROR, "Colony::SetSchematic() - pinID %u not found in ccPin.pins map", pinID);
}

void Colony::InstallProgram(uint32 ecuID, uint16 typeID, float headRadius)
{
    std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(ecuID);
    if (itr != ccPin->pins.end()) {
        if (typeID and (itr->second.resTypeID != typeID)) {
            _log(PLANET__ERROR, "Colony::InstallProgram() - typeID %u does not match previously saved program", typeID);
        } else if (!typeID) {
            // uninstall program
            itr->second.resTypeID = 0;
            itr->second.expiryTime = 0;
            itr->second.installTime = 0;
            itr->second.lastRunTime = 0;
            itr->second.schematicID = 0;
            for (auto cur : itr->second.heads) {
                cur.second.typeID = 0;
                cur.second.qtyPerCycle = 0;
            }
        } else {
            itr->second.headRadius = headRadius;
            itr->second.installTime = Win32TimeNow();
            itr->second.lastRunTime = Win32TimeNow();
        }
    } else
        _log(PLANET__ERROR, "Colony::InstallProgram() - ecuPinID %u not found in ccPin.pins map", ecuID);
}

void Colony::SetProgramResults(uint32 ecuID, uint16 typeID, uint16 numCycles, float headRadius, float cycleTime)
{
    std::map<uint32, PI_Pin>::iterator itr = ccPin->pins.find(ecuID);
    if (itr != ccPin->pins.end()) {
        itr->second.cycleTime = (cycleTime * 60 * 60 * 10000000L);
        itr->second.resTypeID = typeID;
        itr->second.expiryTime = ((cycleTime * numCycles) * 60 * 60 * 10000000L) + Win32TimeNow();
        itr->second.headRadius = headRadius;
        itr->second.schematicID = GetHeadType(m_svcMgr->item_factory->GetItem(ecuID)->typeID(), typeID);
    } else
        _log(PLANET__ERROR, "Colony::SetProgramResults() - ecuPinID %u not found in ccPin.pins map", ecuID);
}

void Colony::LaunchCommodities(uint32 pinID, std::map< uint16, uint16 >& items)
{
    // this will export items from CC to jetcan in space.
    // launchpad (Spaceport) xfers items to/from customs office
    std::map<uint32, PI_Pin>::iterator pin = ccPin->pins.find(pinID);
    if (pin != ccPin->pins.end()) {
        // first - create jetcan, add to system, and put in orbit around planet
        // NOTE:  PI jetcans have 5d timers  - use typeID 24445 (freight container) for PI launches
        SystemManager* pSysMgr(m_pSE->SystemMgr());
        GPoint location(pSysMgr->GetSEFromInventory(m_pSE->GetID())->GetPosition());
        location.MakeRandomPointOnSphere(90000);
        ItemData canData(24445,                         // freight container
                        m_client->GetCharacterID(),  //owner is Character
                       pSysMgr->GetID(),
                        flagAutoFit,
                        "PI Commodities Container",
                        location);

        CargoContainerRef contRef = m_svcMgr->item_factory->SpawnCargoContainer(canData);
        if (!contRef)
            if (m_client->CanThrow())
                throw PyException(MakeCustomError("Unable to spawn item of type %u.", 24445));
            else
                ; //  make error here?

        ContainerData contData;
            contData.allianceID = m_client->GetAllianceID();
            contData.corporationID = m_client->GetCorporationID();
            contData.factionID = m_client->GetWarFactionID();
            contData.ownerID = m_client->GetCharacterID();
        // create new container
        ContainerSE* cSE = new ContainerSE(contRef, *m_svcMgr, pSysMgr, contData);
        contRef->SetMySE(cSE);      // item-to-entity interface
        cSE->AnchorContainer();     // avoid GC checks on this container
        pSysMgr->AddEntity(cSE);

        // second - reduce qtys in source container (CC contents in this case), and create actual item (previously only virtual) using same loop, and add to container
        for (auto cur : items) {
            std::map<uint16, uint32>::iterator cont = pin->second.contents.find(cur.first);
            if (cont != pin->second.contents.end()) {
                cont->second -= cur.second;
            }
            ItemData iData(cur.first, m_client->GetCharacterID(), cSE->GetID(), flagAutoFit, cur.second);
            InventoryItemRef iRef = m_svcMgr->item_factory->SpawnItem(iData);
        }
        contRef->SaveItem();
        pin->second.lastLaunchTime = Win32TimeNow();

        // third - create db entry for planet launches and save
        m_db.SaveLaunch(m_client->GetCharacterID(), pSysMgr->GetID(), m_pSE->GetID(), location);

        // update colony
        Update();
    } else
        _log(PLANET__ERROR, "Colony::LaunchCommodities() - pinID %u not found in ccPin.pins map", pinID);
}


PyTuple* Colony::GetPins()
{
    uint8 index = 0;
    PyTuple* pins(new PyTuple(ccPin->pins.size()));

    for (auto cur : ccPin->pins) {
        PyDict* dict(new PyDict());
        dict->SetItem("id", new PyInt(cur.first));
        PyDict* contents(new PyDict());
        contents->clear();
        for (auto cur2 : cur.second.contents)
            contents->SetItem(new PyInt(cur2.first), new PyInt(cur2.second));
        dict->SetItem("contents", contents);
        dict->SetItem("typeID", new PyInt(cur.second.typeID));
        dict->SetItem("ownerID", new PyInt(cur.second.ownerID));
        dict->SetItem("latitude", new PyFloat(cur.second.latitude));
        dict->SetItem("longitude", new PyFloat(cur.second.longitude));
        dict->SetItem("lastRunTime", new PyULong(cur.second.lastRunTime));
        dict->SetItem("state", new PyInt(cur.second.state));
        dict->SetItem("level", new PyInt(cur.second.level));
        if (cur.second.isLaunchable)
            dict->SetItem("lastLaunchTime", new PyULong(cur.second.lastLaunchTime));
        if ((cur.second.isProcess) and (cur.second.schematicID)) {
            dict->SetItem("cycleTime", new PyULong(cur.second.cycleTime));
            dict->SetItem("schematicID", new PyInt(cur.second.schematicID));
            dict->SetItem("hasReceivedInputs", new PyBool(cur.second.hasReceivedInputs));
            dict->SetItem("receivedInputsLastCycle", new PyBool(cur.second.receivedInputsLastCycle));
        }
        if (cur.second.isExtractor) {
            dict->SetItem("installTime", new PyULong(cur.second.installTime));
            dict->SetItem("programType", new PyInt(cur.second.resTypeID));
            dict->SetItem("cycleTime", new PyULong(cur.second.cycleTime));
        }
        if ((cur.second.isECU) and (cur.second.heads.size() > 0)) {
            dict->SetItem("installTime", new PyULong(cur.second.installTime));
            dict->SetItem("programType", new PyInt(cur.second.resTypeID));
            dict->SetItem("qtyPerCycle", new PyFloat(cur.second.qtyPerCycle));
            dict->SetItem("headRadius", new PyFloat(cur.second.headRadius));
            dict->SetItem("expiryTime", new PyULong(cur.second.expiryTime));
            dict->SetItem("cycleTime", new PyULong(cur.second.cycleTime));
            PyList* list = new PyList();
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
            dict->SetItem("linkID", new PyInt(cur.first));                 // this is link itemID (type 2280)
            dict->SetItem("endpoint1", new PyInt(cur.second.endpoint1));
            dict->SetItem("endpoint2", new PyInt(cur.second.endpoint2));
            dict->SetItem("level", new PyInt(cur.second.level));
            dict->SetItem("typeID", new PyInt(cur.second.typeID));
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
        PyList* list(new PyList());
        for (auto cur2 : cur.second.path)                               // path of pinIDs the route should follow
            list->AddItem(new PyInt(cur2));                             // this is is routeID (low number)
        dict->SetItem("routeID", new PyInt(cur.second.id));             // this is routeID (as sent by client)
        dict->SetItem("path", list);                                    // list of paths on this route
        dict->SetItem("commodityTypeID", new PyInt(cur.second.commodityTypeID));
        dict->SetItem("commodityQuantity", new PyInt(cur.second.commodityQuantity));

        PyObject* obj(new PyObject("util.KeyVal", dict));
        routes->SetItem(index++, obj);
    }
    return routes;
}

PyRep* Colony::GetColony()
{
    if (m_newHead) {
        // create new heads based on program in ecu
        CreateExtractorHead();
    }

    Update();   // update colony before sending data.

    PyDict* args(new PyDict());
        args->SetItem("pins", GetPins());
        args->SetItem("level", new PyInt(ccPin->level));
        args->SetItem("links", GetLinks());
        args->SetItem("routes", GetRoutes());
        args->SetItem("currentSimTime", new PyULong(ccPin->currentSimTime));
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

void Colony::Update()
{
    // update colony time to current time (based on this update, prior to sending out current colony status)
    ccPin->currentSimTime = Win32TimeNow();

    bool save = false;
    // first, check each ecu for currentSimTime vs expiryTime and update qtys as needed.
    ProcessECUs(save);
    // second, check each silo for currentSimTime vs expiryTime and update qtys as needed.
    ProcessSilos(save);
    // third, check each plant for currentSimTime vs lastRunTime and update qtys as needed.
    ProcessPlants(save);

    // save current state
    if (save)
        Save();
}

void Colony::ProcessECUs(bool& save)
{
    // first, check each ecu for currentSimTime vs expiryTime and update qtys as needed.
    //  also check for routes and update origins/destinations for qtys
    for (auto ecu : ccPin->pins) {
        if (ecu.second.isECU) {
            if ((ecu.second.expiryTime == 0 ) or (ecu.second.expiryTime > Win32TimeNow()))
                continue;
            // first - see if this ecu has a route and move contents per route
            //   note:  check for multiple routes/commodities
            for (auto route : ccPin->routes) {
                // second - update current contents per route movement as noted above -ecu doesnt store mat'l, but might later...
                // get route destination pin and update qty there for this round
                std::map<uint32, PI_Pin>::iterator dest = ccPin->pins.find(route.first);
                if (dest != ccPin->pins.end()) {
                    // contents are stored in each pin.  PI_Pin.contents(std::map<uint16, uint32> typeID, qty)
                    std::map<uint16, uint32>::iterator itr = dest->second.contents.find(route.second.commodityTypeID);
                    /** @todo  check for available capy and adjust qty accordingly.
                     *       if dest cant hold entire xfer qty, drop remainder in current pin contents
                     */
                    if (itr != dest->second.contents.end()) {
                        itr->second += route.second.commodityQuantity;
                        m_db.UpdateContents(dest->first, itr->first, itr->second);
                    } else {
                        dest->second.contents[route.second.commodityTypeID] = route.second.commodityQuantity;
                        m_db.AddContents(m_colonyID, dest->first, route.second.commodityTypeID, route.second.commodityQuantity);
                    }

                    if (dest->second.isProcess)
                        dest->second.hasReceivedInputs = true;
                } else
                    _log(PLANET__ERROR, "Colony::ProcessECUs()::Routes() - Dest pinID %u not found in ccPin.pins map", route.first);
            }
            // third - update extractors to new run time (find pins for this ecu in ecu->second.heads[std::map<uint32, PI_Heads>])
            for (auto head : ecu.second.heads)
                if (head.second.ecuPinID == ecu.first) {
                    std::map<uint32, PI_Pin>::iterator itr2 = ccPin->pins.find(head.first);    // get extractor pin here
                    if (itr2 != ccPin->pins.end())
                        itr2->second.lastRunTime = Win32TimeNow();      // update lastRunTime on this extractor
                    else
                        _log(PLANET__ERROR, "Colony::ProcessECUs()::Heads() - Head pinID %u not found in ccPin.pins map", head.first);
                }
            // fourth - reset lastRunTime, and complete this loop.
            ecu.second.lastRunTime = Win32TimeNow();
            save = true;
        }
    }
}

void Colony::ProcessSilos(bool& save)
{
    // second, check each silo for currentSimTime vs expiryTime and update qtys as needed.
    //  also check for routes and update origins/destinations for qtys - outbound should happen same time as inbounds, so no need to worry about capacities for now.
    for (auto silo : ccPin->pins) {
        if (silo.second.isStorage) {
            if ((silo.second.expiryTime == 0 ) or (silo.second.expiryTime > Win32TimeNow()))
                continue;
            // first - see if this silo has a route and move contents per route  - note:  check for multiple routes/commodities
            for (auto route : ccPin->routes) {
                // second - update current contents per route movement as noted above
                std::map<uint32, PI_Pin>::iterator dest = ccPin->pins.find(route.first);
                if (dest != ccPin->pins.end()) {
                    // contents are stored in each pin.  PI_Pin.contents(std::map<uint16, uint32> typeID, qty)
                    /** @todo   verify current qty is enough to xfer entire route qty.
                     *          check for available capy and adjust qty accordingly.
                     *       if dest cant hold entire xfer qty, drop remainder in current pin contents
                     */
                    uint32 amount = route.second.commodityQuantity;
                    std::map<uint16, uint32>::iterator contents = silo.second.contents.find(route.second.commodityTypeID);
                    if (contents != silo.second.contents.end()) {
                        if (contents->second <= amount) {
                            amount = contents->second;
                            silo.second.contents.erase(contents);
                            m_db.RemoveContents(silo.first, route.second.commodityTypeID);
                        } else {
                            contents->second -= route.second.commodityQuantity;
                            m_db.UpdateContents(silo.first, route.second.commodityTypeID, contents->second);
                        }
                    }
                    /** @todo  check for available space in dest and adjust accordingly  */
                // get route destination pin and update qty there for this round
                    std::map<uint16, uint32>::iterator itr = dest->second.contents.find(route.second.commodityTypeID);
                    if (itr != dest->second.contents.end()) {
                        itr->second += amount;
                        m_db.UpdateContents(dest->first, itr->first, itr->second);
                    } else {
                        dest->second.contents[route.second.commodityTypeID] = amount;
                        m_db.AddContents(m_colonyID, dest->first, route.second.commodityTypeID, amount);
                    }

                    if (dest->second.isProcess)
                        dest->second.hasReceivedInputs = true;

                } else
                    _log(PLANET__ERROR, "Colony::ProcessSilos()::Routes() - Dest pinID %u not found in ccPin.pins map", route.first);
                // third - reset lastRunTime, and complete this loop.
                silo.second.lastRunTime = Win32TimeNow();
                save = true;
            }
        }
    }
}

void Colony::ProcessPlants(bool& save)
{
    // third, check each plant for currentSimTime vs lastRunTime and update qtys as needed.
    //  also check for routes and update origins/destinations for qtys
    for (auto plant : ccPin->pins) {
        if (plant.second.isProcess) {
            if ((plant.second.expiryTime == 0 ) or (plant.second.expiryTime > Win32TimeNow()))
                continue;
            // first - see if this plant has a route and move contents per route
            for (auto route : ccPin->routes) {
                // second - update current contents per route movement as noted above
                // get route destination pin and update qty there for this round
                std::map<uint32, PI_Pin>::iterator dest = ccPin->pins.find(route.first);
                if (dest != ccPin->pins.end()) {
                    // contents are stored in each pin.  PI_Pin.contents(std::map<uint16, uint32> typeID, qty)
                    /** @todo  check for available capy and adjust qty accordingly.
                     *       if dest cant hold entire xfer qty, drop remainder in current pin contents
                     * do plants hold raw matls in storage?
                     */
                    std::map<uint16, uint32>::iterator itr = dest->second.contents.find(route.second.commodityTypeID);
                    if (itr != dest->second.contents.end()) {
                        itr->second += route.second.commodityQuantity;
                        m_db.UpdateContents(dest->first, itr->first, itr->second);
                    } else {
                        dest->second.contents[route.second.commodityTypeID] = route.second.commodityQuantity;
                        m_db.AddContents(m_colonyID, dest->first, route.second.commodityTypeID, route.second.commodityQuantity);
                    }

                    if (dest->second.isProcess)
                        dest->second.hasReceivedInputs = true;
                } else
                    _log(PLANET__ERROR, "Colony::ProcessPlants()::Routes() - Dest pinID %u not found in ccPin.pins map", route.first);
                // third - add new items per schematic/program.  check container space here (should be free, if route is found)
                // TODO:  write consumer/process code and complete this section
                bool processing = false;
                if (plant.second.receivedInputsLastCycle) {
                    processing = true;
                    plant.second.receivedInputsLastCycle = false;
                    // process plant program here,  (check for inputs/qtys and start process if able)
                    // NOTE:  remember to remove qty of inputs
                }

                if (plant.second.hasReceivedInputs) {
                    plant.second.hasReceivedInputs = false;
                    plant.second.receivedInputsLastCycle = true;
                    if (!processing) {
                        // processing hasnt started on previous check.
                        // process plant program here,  (check for inputs/qtys and start process if able)
                        // NOTE:  remember to remove qty of inputs
                    }
                }

                // fourth - reset lastRunTime, and complete this loop.
                plant.second.lastRunTime = Win32TimeNow();
                save = true;
            }
        }
    }
}

uint32 Colony::GetHeadType(uint16 ecuTypeID, uint16 resTypeID)
{
    switch (ecuTypeID) {
        case 2848: {  //Barren ECU
            switch (resTypeID) {
                case 2268: return 2409; //Barren Aqueous Liquid Extractor
                case 2267: return 2430; //Barren Base Metals Extractor
                case 2270: return 2435; //Barren Noble Metals Extractor
                case 2073: return 2449; //Barren Microorganisms Extractor
                case 2288: return 2459; //Barren Carbon Compounds Extractor
            }
        } break;
        case 3060: {  //Gas ECU
            switch (resTypeID) {
                case 2268: return 2416; //Gas Aqueous Liquid Extractor
                case 2309: return 2424; //Gas Ionic Solutions Extractor
                case 2310: return 2426; //Gas Noble Gas Extractor
                case 2311: return 2427; //Gas Reactive Gas Extractor
                case 2267: return 2433; //Gas Base Metals Extractor
            }
        } break;
        case 3061: {  //Ice ECU
            switch (resTypeID) {
                case 2268: return 2415; //Ice Aqueous Liquid Extractor
                case 2310: return 2423; //Ice Noble Gas Extractor
                case 2073: return 2432; //Ice Microorganisms Extractor
                case 2286: return 2438; //Ice Planktic Colonies Extractor
                case 2272: return 2441; //Ice Heavy Metals Extractor
            }
        } break;
        case 3062: {  //Lava ECU
            switch (resTypeID) {
                case 2308: return 2418; //Lava Suspended Plasma Extractor
                case 2267: return 2428; //Lava Base Metals Extractor
                case 2272: return 2439; //Lava Heavy Metals Extractor
                case 2306: return 2442; //Lava Non-CS Crystals Extractor
                case 2307: return 2448; //Lava Felsic Magma Extractor
            }
        } break;
        case 3063: {  //Oceanic ECU
            switch (resTypeID) {
                case 2268: return 2414; //Oceanic Aqueous Liquid Extractor
                case 2287: return 2458; //Oceanic Complex Organisms Extractor
                case 2286: return 2452; //Oceanic Planktic Colonies Extractor
                case 2288: return 2461; //Oceanic Carbon Compounds Extractor
                case 2073: return 2451; //Oceanic Microorganisms Extractor
            }
        } break;
        case 3064: {  //Plasma ECU
            switch (resTypeID) {
                case 2308: return 2417; //Plasma Suspended Plasma Extractor
                case 2267: return 2429; //Plasma Base Metals Extractor
                case 2270: return 2434; //Plasma Noble Metals Extractor
                case 2272: return 2440; //Plasma Heavy Metals Extractor
                case 2306: return 2443; //Plasma Non-CS Crystals Extractor
            }
        } break;
        case 3067: {  //Storm ECU
            switch (resTypeID) {
                case 2268: return 2413; //Storm Aqueous Liquid Extractor
                case 2308: return 2419; //Storm Suspended Plasma Extractor
                case 2309: return 2422; //Storm Ionic Solutions Extractor
                case 2310: return 2425; //Storm Noble Gas Extractor
                case 2267: return 2431; //Storm Base Metals Extractor
            }
        } break;
        case 3068: {  //Temperate ECU
            switch (resTypeID) {
                case 2268: return 2412; //Temperate Aqueous Liquid Extractor
                case 2073: return 2450; //Temperate Microorganisms Extractor
                case 2287: return 2453; //Temperate Complex Organisms Extractor
                case 2288: return 2460; //Temperate Carbon Compounds Extractor
                case 2305: return 2462; //Temperate Autotrophs Extractor
            }
        } break;
    }
    _log(PLANET__ERROR, "Colony::GetHeadType() - Extractor typeID not found using ECU typeID: %u and Resource typeID: %u", ecuTypeID, resTypeID);
    return 2412; //Temperate Aqueous Liquid Extractor  <<< as good a default as any...
}
