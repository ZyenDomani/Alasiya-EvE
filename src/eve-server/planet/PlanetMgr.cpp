
 /**
  * @name PlanetMgr.cpp
  *   Specific Class for managing planet resources
  * @Author:         Allan
  * @date:   30 April 2016
  */



#include "eve-server.h"

#include "PyService.h"
#include "inventory/ItemType.h"
#include "packets/Planet.h"
#include "planet/Colony.h"
#include "planet/PlanetMgr.h"


PlanetMgr::PlanetMgr(PyServiceMgr *mgr, Client* pClient, PlanetSE* pPlanet, Colony* pColony)
:m_svcMgr(mgr),
 m_client(pClient),
 m_colony(pColony),
m_planet(pPlanet)
{

}

PyRep* PlanetMgr::UpdateNetwork(UUNCommandList& uuncl)
{
    for (int i = 0; i < uuncl.commandList->size(); i++) {
        UUNCommand uunc;
        if (!uunc.Decode(uuncl.commandList->GetItem(i)->AsTuple())) {
            _log(SERVICE__ERROR, "Failed to decode args for UUNCommand");
            return nullptr;
        }
        _log(PLANET__TRACE, "  UserUpdateNetwork: loop: %u, command: %u", i, uunc.command);
        uunc.Dump(PLANET__DUMP, "    ");
        switch (uunc.command) {
            case PinCommands::CreatePin:                CreatePin(uunc);                break;
            case PinCommands::RemovePin:                RemovePin(uunc);                break;
            case PinCommands::CreateLink:               CreateLink(uunc);               break;
            case PinCommands::RemoveLink:               RemoveLink(uunc);               break;
            case PinCommands::SetLinkLevel:             SetLinkLevel(uunc);             break;
            case PinCommands::UpgradeCommandCenter:     UpgradeCommandCenter(uunc);     break;
            /** @todo not handled yet... */
            case PinCommands::CreateRoute:              CreateRoute(uunc);              break;
            case PinCommands::RemoveRoute:              RemoveRoute(uunc);              break;
            case PinCommands::SetSchematic:             SetSchematic(uunc);             break;
            case PinCommands::AddExtractorHead:         AddExtractorHead(uunc);         break;
            case PinCommands::KillExtractorHead:        KillExtractorHead(uunc);        break;
            case PinCommands::MoveExtractorHead:        MoveExtractorHead(uunc);        break;
            case PinCommands::InstallProgram:           InstallProgram(uunc);           break;
        }
    }
    return m_colony->GetColony();
}

void PlanetMgr::CreatePin(UUNCommand& nc)
{
    uint32 typeID = nc.command_data->GetItem(1)->AsInt()->value();
    uint32 groupID = m_svcMgr->item_factory->GetType(typeID)->groupID();
    if (groupID == EVEDB::invGroups::Command_Centers) {
        UUNCCommandCenter uunccc;
        if (!uunccc.Decode(nc.command_data)) {
            _log(SERVICE__ERROR, "Failed to decode args for UUNCCommandCenter!");
        }
        uunccc.Dump(PLANET__DUMP, "      ");

        if (!m_colony->CreateCommandPin(uunccc.pinID, uunccc.typeID, uunccc.latitude, uunccc.longitude)) {
            _log(PLANET__ERROR, "  UserUpdateNetwork: Failed to create command center");
        } else {
            _log(PLANET__MESSAGE, "  UserUpdateNetwork: Success creating command center");
        }
    } else if (groupID == EVEDB::invGroups::Storage_Facilities
        or groupID == EVEDB::invGroups::Processors
        or groupID == EVEDB::invGroups::Extractor_Control_Units
        or groupID == EVEDB::invGroups::Planetary_Links
        or groupID == EVEDB::invGroups::Extractors
        or groupID == EVEDB::invGroups::Spaceports) {
        UUNCStandardPin uuncsp;
        if (!uuncsp.Decode(nc.command_data)) {
            _log(SERVICE__ERROR, "Failed to decode args for UUNCStandardPin!");
        }
        uuncsp.Dump(PLANET__DUMP, "      ");

        if (!m_colony->CreatePin(uuncsp.pinID2, uuncsp.typeID, uuncsp.latitude, uuncsp.longitude)) {
            _log(PLANET__ERROR, "  UserUpdateNetwork: Failed to create new pin");
        } else {
            _log(PLANET__MESSAGE, "  UserUpdateNetwork: Success creating new pin");
        }
    } else {
        // Invalid...
        _log(PLANET__ERROR, "  UserUpdateNetwork: INVALID CREATEPIN groupID %u", groupID);
    }
}

void PlanetMgr::CreateLink(UUNCommand& nc)
{
    if (nc.command_data->GetItem(0)->IsInt()) {
        UUNCLinkCommand uunclc;
        if (!uunclc.Decode(nc.command_data)) {
            _log(SERVICE__ERROR, "Failed to decode args for UUNCLinkCommand!");
        }
        uunclc.Dump(PLANET__DUMP, "      ");

        if (!m_colony->CreateLink(uunclc.src, uunclc.dest2, uunclc.level, true)) {
            _log(PLANET__ERROR, "  UserUpdateNetwork: Failed to create link");
        } else {
            _log(PLANET__MESSAGE, "  UserUpdateNetwork: Success creating link");
        }
    } else if (nc.command_data->GetItem(0)->IsTuple()) {
        UUNCLinkStandard uuncls;
        if (!uuncls.Decode(nc.command_data)) {
            _log(SERVICE__ERROR, "Failed to decode args for UUNCLinkStandard!");
        }
        uuncls.Dump(PLANET__DUMP, "      ");

        if (!m_colony->CreateLink(uuncls.src2, uuncls.dest2, uuncls.level, false)) {
            _log(PLANET__ERROR, "  UserUpdateNetwork: Failed to create link");
        } else {
            _log(PLANET__MESSAGE, "  UserUpdateNetwork: Success creating link");
        }
    } else {
        //Invalid...
        _log(PLANET__TRACE, "  UserUpdateNetwork: INVALID CREATELINK");
    }
}

void PlanetMgr::RemoveLink(UUNCommand& nc)
{
    if (nc.command_data->GetItem(0)->IsInt()) {
        uint32 src = nc.command_data->GetItem(0)->AsInt()->value();
        uint32 dest2 = nc.command_data->GetItem(1)->AsTuple()->GetItem(1)->AsInt()->value();
        if (!m_colony->RemoveLink(src, dest2, true)) {
            _log(PLANET__ERROR, "  UserUpdateNetwork: Failed to remove link");
        } else {
            _log(PLANET__MESSAGE, "  UserUpdateNetwork: Success removing link");
        }
    } else if (nc.command_data->GetItem(0)->IsTuple()) {
        uint32 src = nc.command_data->GetItem(0)->AsTuple()->GetItem(1)->AsInt()->value();
        uint32 dest2 = nc.command_data->GetItem(1)->AsTuple()->GetItem(1)->AsInt()->value();
        if (!m_colony->RemoveLink(src, dest2, false)) {
            _log(PLANET__ERROR, "  UserUpdateNetwork: Failed to remove link");
        } else {
            _log(PLANET__MESSAGE, "  UserUpdateNetwork: Success removing link");
        }
    }
}

void PlanetMgr::RemovePin(UUNCommand& nc)
{
    uint32 pinID = nc.command_data->GetItem(0)->AsTuple()->GetItem(1)->AsInt()->value();
    if (!m_colony->RemovePin(pinID)) {
        _log(PLANET__ERROR, "  UserUpdateNetwork: Failed to remove pin");
    } else {
        _log(PLANET__MESSAGE, "  UserUpdateNetwork: Success removing pin");
    }
}

void PlanetMgr::SetLinkLevel(UUNCommand& nc)
{
    if (nc.command_data->GetItem(0)->IsInt()) {
        UUNCLinkCommand uunclc;
        if (!uunclc.Decode(nc.command_data)) {
            _log(SERVICE__ERROR, "Failed to decode args for UUNCLinkCommand!");
        }
        uunclc.Dump(PLANET__DUMP, "      ");

        if (!m_colony->UpgradeLink(uunclc.src, uunclc.dest2, uunclc.level, true)) {
            _log(PLANET__ERROR, "  UserUpdateNetwork: Failed to upgrade link");
        } else {
            _log(PLANET__MESSAGE, "  UserUpdateNetwork: Success upgrading link");
        }
    } else if (nc.command_data->GetItem(0)->IsTuple()) {
        UUNCLinkStandard uuncls;
        if (!uuncls.Decode(nc.command_data)) {
            _log(SERVICE__ERROR, "Failed to decode args for UUNCLinkStandard!");
        }
        uuncls.Dump(PLANET__DUMP, "      ");

        if (!m_colony->UpgradeLink(uuncls.src2, uuncls.dest2, uuncls.level, false)) {
            _log(PLANET__ERROR, "  UserUpdateNetwork: Failed to upgrade link");
        } else {
            _log(PLANET__MESSAGE, "  UserUpdateNetwork: Success upgrading link");
        }
    }
}

void PlanetMgr::UpgradeCommandCenter(UUNCommand& nc)
{
    uint32 pinID = nc.command_data->GetItem(0)->AsInt()->value();
    uint32 level = nc.command_data->GetItem(1)->AsInt()->value();
    m_colony->UpgradeCommandCenter(pinID, level);
}

void PlanetMgr::CreateRoute(UUNCommand& nc)
{

}

void PlanetMgr::AddExtractorHead(UUNCommand& nc)
{

}

void PlanetMgr::InstallProgram(UUNCommand& nc)
{

}

void PlanetMgr::KillExtractorHead(UUNCommand& nc)
{

}

void PlanetMgr::MoveExtractorHead(UUNCommand& nc)
{

}

void PlanetMgr::RemoveRoute(UUNCommand& nc)
{

}

void PlanetMgr::SetSchematic(UUNCommand& nc)
{

}


    /* these are for PI */
/*
    AttrHarvesterType = 709,
    AttrHarvesterQuality = 710,
    AttrLogisticalCapacity = 1631,
    AttrPlanetRestriction = 1632,
    AttrPowerLoadPerKm = 1633,
    AttrCPULoadPerKm = 1634,
    AttrCPULoadLevelModifier = 1635,
    AttrPowerLoadLevelModifier = 1636,
    AttrImportTax = 1638,
    AttrExportTax = 1639,
    AttrImportTaxMultiplier = 1640,
    AttrExportTaxMultiplier = 1641,
    AttrPinExtractionQuantity = 1642,
    AttrPinCycleTime = 1643,
    AttrExtractorDepletionRange = 1644,
    AttrExtractorDepletionRate = 1645,
    AttrSpecialCommandCenterHoldCapacity = 1646,
    */

/*
piLaunchOrbitDecayTime = DAY * 5
piCargoInOrbit = 0
piCargoDeployed = 1
piCargoClaimed = 2
piCargoDeleted = 3
*/

/*
planetResourceScanDistance = 1000000000
planetResourceProximityDistant = 0
planetResourceProximityRegion = 1
planetResourceProximityConstellation = 2
planetResourceProximitySystem = 3
planetResourceProximityPlanet = 4
planetResourceProximityLimits = [(2, 6),
 (4, 10),
 (6, 15),
 (10, 20),
 (15, 30)]
planetResourceScanningRanges = [9.0,
 7.0,
 5.0,
 3.0,
 1.0]
planetResourceUpdateTime = 1 * HOUR
planetResourceMaxValue = 1.21
*/