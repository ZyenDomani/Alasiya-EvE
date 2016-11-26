
 /**
  * @name PlanetMgr.cpp
  *   Specific Class for managing planet resources
  *         this is based on preliminary work by Comet0
  * @Author:         Allan
  * @date:   30 April 2016
  * @update:  4 November 2016  - began rewrite
  *          16 November 2016  - basic system working
  */



#include "eve-server.h"

#include "PyService.h"
#include "inventory/ItemType.h"
#include "inventory/InventoryItem.h"
#include "packets/Planet.h"
#include "planet/Colony.h"
#include "planet/PlanetMgr.h"
#include "Planet.h"
#include "Client.h"


PlanetMgr::PlanetMgr(PyServiceMgr *mgr, Client* pClient, PlanetSE* pPlanet, Colony* pColony)
:m_svcMgr(mgr),
 m_client(pClient),
 m_colony(pColony),
m_planet(pPlanet)
{

}

PyRep* PlanetMgr::GetProgramResultInfo(uint32 pinID, uint32 typeID, PyList* heads, float headRadius)
{
    //  ECU pinID, resource typeID, list of {headID, lat, long}, radius of head (small number...rad maybe?)
    // qtyToDistribute, cycleTime, numCycles = self.remoteHandler.GetProgramResultInfo(pinID, typeID, pin.heads, headRadius)

    /*
     *
    SEC = 10000000L
    MIN = SEC * 60L
    HOUR = MIN * 60L
RADIUS_DRILLAREAMAX = 0.05
RADIUS_DRILLAREAMIN = 0.01
RADIUS_DRILLAREADIFF = RADIUS_DRILLAREAMAX - RADIUS_DRILLAREAMIN

def GetProgramLengthFromHeadRadius(headRadius):
    return ((headRadius - RADIUS_DRILLAREAMIN) / RADIUS_DRILLAREADIFF) * 335 + 1   << length in hours between 1 and 336  (336h = 14d)
def GetCycleTimeFromProgramLength(programLength):
    return 0.25 * 2 ^ max(0, math.floor(math.log(programLength / 25.0, 2)) + 1)

        programLength = planetCommon.GetProgramLengthFromHeadRadius(headRadius)
        cycleTime = planetCommon.GetCycleTimeFromProgramLength(programLength)
        numCycles = int(programLength / cycleTime)
        cycleTime = int(cycleTime * HOUR)

    */
    InventoryItemRef iRef = m_svcMgr->item_factory->GetItem(pinID);
    float cycleTime = 0, length = 0;
    uint16 numCycles = 0;
    double one = ((headRadius - 0.01f) /0.04);
    length = one * 335;
    double two = log2(++length /25);
    cycleTime = EvE::max(floor(two));
    cycleTime = 0.25 *(pow(2, ++cycleTime));
    numCycles = (uint16)(length / cycleTime);
    int64 iCycleTime = cycleTime * 60 * 60 * 10000000L;

    _log(PLANET__TRACE, "PlanetMgr::GetProgramResultInfo() -  cycleTime:%.2f, iCycleTime:%" PRIi64 ", length:%.2f, numCycles:%u, (one:%.5f, two:%.5f)", \
                            cycleTime, iCycleTime, length, numCycles, one, two);

    PyTuple* res = new PyTuple(3);
        res->SetItem(0, new PyInt(iRef->GetAttribute(AttrPinExtractionQuantity).get_int()));    //qtyToDistribute
        res->SetItem(1, new PyLong(iCycleTime));    //cycleTime - in usec
        res->SetItem(2, new PyInt(numCycles));    //numCycles

    if (is_log_enabled(PLANET__RES_DUMP))
        res->Dump(PLANET__RES_DUMP, "    ");

    m_colony->SetProgramResults(pinID, typeID, numCycles, headRadius, cycleTime);

    PySafeDecRef(heads);
    return res;
}

PyRep* PlanetMgr::UpdateNetwork(UUNCommandList& uuncl)
{
    for (int i = 0; i < uuncl.commandList->size(); i++) {
        UUNCommand uunc;
        if (!uunc.Decode(uuncl.commandList->GetItem(i)->AsTuple())) {
            _log(SERVICE__ERROR, "Failed to decode args for UUNCommand");
            uuncl.commandList->Dump(PLANET__WARNING, "      ");
            return nullptr;
        }
        _log(PLANET__TRACE, "PlanetMgr::UserUpdateNetwork() - loop: %u, command: %u", i, uunc.command);
        switch (uunc.command) {
            case PinCommands::CreatePin:                CreatePin(uunc);                break;
            case PinCommands::RemovePin:                RemovePin(uunc);                break;
            case PinCommands::CreateLink:               CreateLink(uunc);               break;
            case PinCommands::RemoveLink:               RemoveLink(uunc);               break;
            case PinCommands::CreateRoute:              CreateRoute(uunc);              break;
            case PinCommands::SetLinkLevel:             SetLinkLevel(uunc);             break;
            case PinCommands::UpgradeCommandCenter:     UpgradeCommandCenter(uunc);     break;
            case PinCommands::SetSchematic:             SetSchematic(uunc);             break;
            case PinCommands::RemoveRoute:              RemoveRoute(uunc);              break;
            case PinCommands::AddExtractorHead:         AddExtractorHead(uunc);         break;
            case PinCommands::MoveExtractorHead:        MoveExtractorHead(uunc);        break;
            case PinCommands::InstallProgram:           InstallProgram(uunc);           break;
            /** @todo not handled yet... */
            case PinCommands::KillExtractorHead:        KillExtractorHead(uunc);        break;
            case PinCommands::PrioritizeRoute:          PrioritizeRoute(uunc);          break;
            default: {
                // case not handled yet.
                _log(PLANET__ERROR, "PlanetMgr::UserUpdateNetwork() Invalid command switch %i", uunc.command);
                ;
            } break;
        }
    }

    return m_colony->GetColony();
}

void PlanetMgr::UpgradeCommandCenter(UUNCommand& nc)
{
    m_colony->UpgradeCommandCenter(nc.command_data->GetItem(0)->AsInt()->value(), nc.command_data->GetItem(1)->AsInt()->value());
}

void PlanetMgr::CreatePin(UUNCommand& nc)
{
    using namespace EVEDB::invGroups;
    uint32 typeID = nc.command_data->GetItem(1)->AsInt()->value();
    uint32 groupID = m_svcMgr->item_factory->GetType(typeID)->groupID();
    switch (groupID) {
        case Command_Centers: {
            UUNCCommandCenter uunccc;
            if (!uunccc.Decode(nc.command_data)) {
                _log(SERVICE__ERROR, "Failed to decode args for UUNCCommandCenter!");
                nc.command_data->Dump(PLANET__WARNING, "      ");
            }
            m_colony->CreateCommandPin(uunccc.pinID, uunccc.typeID, uunccc.latitude, uunccc.longitude);
            m_planet->CreateCustomsOffice();
        } break;
        case Storage_Facilities:
        case Processors:
        case Extractor_Control_Units:
        case Planetary_Links:
        case Extractors: {
            UUNCStandardPin uuncsp;
            if (!uuncsp.Decode(nc.command_data)) {
                _log(SERVICE__ERROR, "Failed to decode args for UUNCStandardPin!");
                nc.command_data->Dump(PLANET__WARNING, "      ");
            }
            m_colony->CreatePin(groupID, uuncsp.pinID2, uuncsp.typeID, uuncsp.latitude, uuncsp.longitude);
        } break;
        case Spaceports: {
            // Not Supported yet
            m_client->SendErrorMsg("PI Spaceports (and their Planet Customs Offices) are not yet supported.");
            _log(PLANET__ERROR, "PlanetMgr::UserUpdateNetwork::CreatePin() Planet Spaceports (type/group %u/%u) not supported.", typeID, groupID);
        } break;
        case Mercenary_Bases:
        case Capsuleer_Bases:{
            // Not Supported yet
            _log(PLANET__ERROR, "PlanetMgr::UserUpdateNetwork::CreatePin() Planet Bases (type/group %u/%u) not supported.", typeID, groupID);
        } break;
        default: {
            // Invalid...
            _log(PLANET__ERROR, "PlanetMgr::UserUpdateNetwork::CreatePin() Invalid type/group %u/%u", typeID, groupID);
        } break;
    }
}

void PlanetMgr::CreateLink(UUNCommand& nc)
{
    uint32 src = 0, dest = 0, level = 0;
    if (nc.command_data->GetItem(0)->IsInt()) {
        if (nc.command_data->GetItem(1)->IsInt()) {
            UUNCLinkExist uuncle;
            if (!uuncle.Decode(nc.command_data)) {
                _log(SERVICE__ERROR, "Failed to decode args for UUNCLinkExist!");
                nc.command_data->Dump(PLANET__WARNING, "      ");
            }
            src = uuncle.src;
            dest = uuncle.dest;
            level = uuncle.level;
        } else {
            UUNCLinkCommand uunclc;
            if (!uunclc.Decode(nc.command_data)) {
                _log(SERVICE__ERROR, "Failed to decode args for UUNCLinkCommand!");
                nc.command_data->Dump(PLANET__WARNING, "      ");
            }
            src = uunclc.src;
            dest = uunclc.dest2;
            level = uunclc.level;
        }
    } else if (nc.command_data->GetItem(0)->IsTuple()) {
        UUNCLinkStandard uuncls;
        if (!uuncls.Decode(nc.command_data)) {
            _log(SERVICE__ERROR, "Failed to decode args for UUNCLinkStandard!");
            nc.command_data->Dump(PLANET__WARNING, "      ");
        }
        src = uuncls.src2;
        dest = uuncls.dest2;
        level = uuncls.level;
    } else {
        //Invalid...
        _log(PLANET__TRACE, "PlanetMgr::UserUpdateNetwork::CreateLink() command_data type unrecognized: %s", nc.command_data->GetItem(0)->TypeString());
    }
    m_colony->CreateLink(src, dest, level);
}

void PlanetMgr::CreateRoute(UUNCommand& nc)
{
    Call_CreateRoute args;
    if (!args.Decode(nc.command_data)) {
        _log(SERVICE__ERROR, "Failed to decode args for Call_CreateRoute");
        nc.command_data->Dump(PLANET__WARNING, "      ");
    }

    PyIncRef(args.path);
    m_colony->CreateRoute(args.routeID, args.typeID, args.qty, args.path);
}

void PlanetMgr::RemovePin(UUNCommand& nc)
{
    uint32 pinID = 0;
    if (nc.command_data->GetItem(0)->IsInt()) {
        pinID = nc.command_data->GetItem(0)->AsInt()->value();
    } else if (nc.command_data->GetItem(0)->IsTuple()) {
        pinID = nc.command_data->GetItem(0)->AsTuple()->GetItem(1)->AsInt()->value();
    } else {
        //Invalid...
        _log(PLANET__TRACE, "PlanetMgr::UserUpdateNetwork::RemovePin() command_data type unrecognized: %s", nc.command_data->GetItem(0)->TypeString());
        nc.command_data->Dump(PLANET__WARNING, "      ");
    }
    m_colony->RemovePin(pinID);
}

void PlanetMgr::RemoveLink(UUNCommand& nc)
{
    uint32 linkID = 0;
    if (nc.command_data->GetItem(0)->IsInt()) {
        linkID = nc.command_data->GetItem(0)->AsInt()->value();
    } else if (nc.command_data->GetItem(0)->IsTuple()) {
        linkID = nc.command_data->GetItem(0)->AsTuple()->GetItem(1)->AsInt()->value();
    } else {
        //Invalid...
        _log(PLANET__TRACE, "PlanetMgr::UserUpdateNetwork::RemoveLink() command_data type unrecognized: %s", nc.command_data->GetItem(0)->TypeString());
        nc.command_data->Dump(PLANET__WARNING, "      ");
    }
    m_colony->RemoveLink(linkID);
}

void PlanetMgr::RemoveRoute(UUNCommand& nc)
{
    uint32 routeID = 0;
    if (nc.command_data->GetItem(0)->IsInt()) {
        routeID = nc.command_data->GetItem(0)->AsInt()->value();
    } else if (nc.command_data->GetItem(0)->IsTuple()) {
        routeID = nc.command_data->GetItem(0)->AsTuple()->GetItem(1)->AsInt()->value();
    } else {
        //Invalid...
        _log(PLANET__TRACE, "PlanetMgr::UserUpdateNetwork::RemoveRoute() command_data type unrecognized: %s", nc.command_data->GetItem(0)->TypeString());
        nc.command_data->Dump(PLANET__WARNING, "      ");
    }
    m_colony->RemoveRoute(routeID);
}

void PlanetMgr::SetLinkLevel(UUNCommand& nc)
{
    uint32 linkID = 0;
    if (nc.command_data->GetItem(0)->IsInt()) {
        linkID = nc.command_data->GetItem(0)->AsInt()->value();
    } else if (nc.command_data->GetItem(0)->IsTuple()) {
        linkID = nc.command_data->GetItem(0)->AsTuple()->GetItem(1)->AsInt()->value();
    } else {
        //Invalid...
        _log(PLANET__TRACE, "PlanetMgr::UserUpdateNetwork::SetLinkLevel() command_data(0) type unrecognized: %s", nc.command_data->GetItem(0)->TypeString());
        nc.command_data->Dump(PLANET__WARNING, "      ");
    }
    uint8 level = 0;
    if (nc.command_data->GetItem(1)->IsInt()) {
        level = nc.command_data->GetItem(1)->AsInt()->value();
    } else if (nc.command_data->GetItem(1)->IsTuple()) {
        level = nc.command_data->GetItem(1)->AsTuple()->GetItem(1)->AsInt()->value();
    } else {
        //Invalid...
        _log(PLANET__TRACE, "PlanetMgr::UserUpdateNetwork::SetLinkLevel() command_data(1) type unrecognized: %s", nc.command_data->GetItem(0)->TypeString());
        nc.command_data->Dump(PLANET__WARNING, "      ");
    }
    m_colony->UpgradeLink(linkID, level);
}

void PlanetMgr::SetSchematic(UUNCommand& nc)
{
    uint32 pinID = 0;
    if (nc.command_data->GetItem(0)->IsTuple())
        pinID = nc.command_data->GetItem(0)->AsTuple()->GetItem(1)->AsInt()->value();
    else if (nc.command_data->GetItem(0)->IsInt())
        pinID = nc.command_data->GetItem(0)->AsInt()->value();
    uint8 schematicID = nc.command_data->GetItem(1)->AsInt()->value();  // 65 - 137
    m_colony->SetSchematic(pinID, schematicID);
}

void PlanetMgr::AddExtractorHead(UUNCommand& nc)
{
    Call_AddMoveExtractorHead args;
    if (!args.Decode(nc.command_data)) {
        _log(SERVICE__ERROR, "Failed to decode args for Call_AddMoveExtractorHead");
        nc.command_data->Dump(PLANET__WARNING, "      ");
    }

    m_colony->AddExtractorHead(args.ecuID, args.pinID, args.latitude, args.longitude);
}

void PlanetMgr::MoveExtractorHead(UUNCommand& nc)
{
    Call_AddMoveExtractorHead args;
    if (!args.Decode(nc.command_data)) {
        _log(SERVICE__ERROR, "Failed to decode args for Call_AddMoveExtractorHead");
        nc.command_data->Dump(PLANET__WARNING, "      ");
    }

    m_colony->MoveExtractorHead(args.ecuID, args.pinID, args.latitude, args.longitude);
}

void PlanetMgr::InstallProgram(UUNCommand& nc)
{
    Call_InstallProgram args;
    if (!args.Decode(nc.command_data)) {
        _log(SERVICE__ERROR, "Failed to decode args for Call_InstallProgram");
        nc.command_data->Dump(PLANET__WARNING, "      ");
    }

    m_colony->InstallProgram(args.ecuID, args.typeID, args.headRadius);
}

void PlanetMgr::KillExtractorHead(UUNCommand& nc)
{
    _log(PLANET__TRACE, "PlanetMgr::UserUpdateNetwork::KillExtractorHead()");

    nc.Dump(PLANET__WARNING, "      ");

}

void PlanetMgr::PrioritizeRoute(UUNCommand& nc)
{
    _log(PLANET__TRACE, "PlanetMgr::UserUpdateNetwork::PrioritizeRoute()");

    nc.Dump(PLANET__WARNING, "      ");

}

/*
 * self.changes.AddCommand(planet.COMMAND_CREATEPIN, pinID=commandCenterID, typeID=typeID, latitude=latitude, longitude=longitude)
 * self.changes.AddCommand(planet.COMMAND_CREATEPIN, pinID=pinID, typeID=typeID, latitude=latitude, longitude=longitude)
 * self.changes.AddCommand(planet.COMMAND_SETSCHEMATIC, pinID=pinID, schematicID=schematicID)
 * self.changes.AddCommand(planet.COMMAND_REMOVEPIN, pinID=pinID)
 * self.changes.AddCommand(planet.COMMAND_CREATELINK, endpoint1=pin1ID, endpoint2=pin2ID, level=0)
 * self.changes.AddCommand(planet.COMMAND_REMOVELINK, endpoint1=pin1ID, endpoint2=pin2ID)
 * self.changes.AddCommand(planet.COMMAND_SETLINKLEVEL, endpoint1=pin1ID, endpoint2=pin2ID, level=newLevel)
 * self.changes.AddCommand(planet.COMMAND_CREATEROUTE, routeID=routeID, path=path, typeID=typeID, quantity=quantity)
 * self.changes.AddCommand(planet.COMMAND_REMOVEROUTE, routeID=routeID)
 * self.changes.AddCommand(planet.COMMAND_UPGRADECOMMANDCENTER, pinID=pinID, level=level)
 * self.changes.AddCommand(planet.COMMAND_ADDEXTRACTORHEAD, pinID=pinID, headID=headID, latitude=latitude, longitude=longitude)
 * self.changes.AddCommand(planet.COMMAND_KILLEXTRACTORHEAD, pinID=pinID, headID=headID)
 * self.changes.AddCommand(planet.COMMAND_MOVEEXTRACTORHEAD, pinID=pinID, headID=headID, latitude=latitude, longitude=longitude)
 * self.changes.AddCommand(planet.COMMAND_INSTALLPROGRAM, pinID=pinID, typeID=typeID, headRadius=headRadius)
 * self.changes.AddCommand(planet.COMMAND_PRIORITIZEROUTE, routeID=routeID, priority=priority)
 */

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
