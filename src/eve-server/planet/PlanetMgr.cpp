
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

#include "Client.h"
#include "PyService.h"
#include "account/AccountService.h"
#include "inventory/ItemType.h"
#include "inventory/InventoryItem.h"
#include "packets/Planet.h"
#include "planet/Colony.h"
#include "planet/Planet.h"
#include "planet/PlanetMgr.h"
#include "planet/PlanetDataMgr.h"


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
    InventoryItemRef iRef = sItemFactory.GetItem(pinID);
    float cycleTime = 0, length = 0;
    uint16 numCycles = 0;
    double one = ((headRadius - 0.01f) /0.04);
    length = one * 335;
    double two = log2(++length /25);
    cycleTime = EvE::max(floor(two));
    cycleTime = 0.25 *(pow(2, ++cycleTime));
    numCycles = (uint16)(length / cycleTime);
    int64 iCycleTime = cycleTime * 60 * 60 * 10000000L;

    _log(PLANET__TRACE, "PlanetMgr::GetProgramResultInfo() -  cycleTime:%.2f, iCycleTime:%lli, length:%.2f, numCycles:%u, (one:%.5f, two:%.5f)", \
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
    bool cancel = false;
    for (int i = 0; i < uuncl.commandList->size(); ++i) {
        if (cancel)
            return m_colony->GetColony();
        UUNCommand uunc;
        if (!uunc.Decode(uuncl.commandList->GetItem(i)->AsTuple())) {
            _log(SERVICE__ERROR, "Failed to decode args for UUNCommand");
            uuncl.commandList->Dump(PLANET__WARNING, "      ");
            return nullptr;
        }
        _log(PLANET__TRACE, "PlanetMgr::UserUpdateNetwork() - loop: %u, command: %u", i, uunc.command);
        switch (uunc.command) {
            case PI::Command::CreatePin:                cancel = CreatePin(uunc);                break;
            case PI::Command::RemovePin:                RemovePin(uunc);                break;
            case PI::Command::CreateLink:               CreateLink(uunc);               break;
            case PI::Command::RemoveLink:               RemoveLink(uunc);               break;
            case PI::Command::CreateRoute:              CreateRoute(uunc);              break;
            case PI::Command::SetLinkLevel:             SetLinkLevel(uunc);             break;
            case PI::Command::UpgradeCommandCenter:     cancel = UpgradeCommandCenter(uunc);     break;
            case PI::Command::SetSchematic:             SetSchematic(uunc);             break;
            case PI::Command::RemoveRoute:              RemoveRoute(uunc);              break;
            case PI::Command::AddExtractorHead:         AddExtractorHead(uunc);         break;
            case PI::Command::MoveExtractorHead:        MoveExtractorHead(uunc);        break;
            case PI::Command::InstallProgram:           InstallProgram(uunc);           break;
            case PI::Command::KillExtractorHead:        KillExtractorHead(uunc);        break;
            /** @todo not handled yet... */
            case PI::Command::PrioritizeRoute:          PrioritizeRoute(uunc);          break;
            default: {
                _log(PLANET__ERROR, "PlanetMgr::UserUpdateNetwork() Invalid command switch %i", uunc.command);
            } break;
        }
    }

    return m_colony->GetColony();
}

bool PlanetMgr::UpgradeCommandCenter(UUNCommand& nc)
{
    // the return here is used to cancel loop in UpdateNetwork.  return false = continue

    int8 oldLevel = m_colony->GetLevel(), newLevel = (int8)nc.command_data->GetItem(1)->AsInt()->value();
    int32 cost = 0;
    while (oldLevel != newLevel) {
        //  calculate total upgrade cost in cases where upgrading multiple levels at once
        switch (oldLevel) {
            case PI::Pin::Level0: cost += 580000; break;
            case PI::Pin::Level1: cost += 930000; break;
            case PI::Pin::Level2: cost += 1200000; break;
            case PI::Pin::Level3: cost += 1500000; break;
            case PI::Pin::Level4: cost += 2100000; break;
        }
        ++oldLevel;
    }
    //take the money, send wallet blink event record the transaction in their journal.
    std::string reason = "DESC:  Command Center upgrade on ";
    reason += m_planet->GetName();
    AccountService::TranserFunds(
                    m_client->GetCharacterID(),
                    ownerUnknown,  // not sure who to send this to
                    cost,
                    reason.c_str(),
                    Journal::EntryType::PlanetaryConstruction,
                    m_planet->GetID(),
                    Account::KeyType::Cash);

    m_colony->UpgradeCommandCenter(nc.command_data->GetItem(0)->AsInt()->value(), newLevel);
    return false;
}

bool PlanetMgr::CreatePin(UUNCommand& nc)
{
    // the return here is used to break out of loop if needed.  return false = continue
    using namespace EVEDB::invGroups;
    uint32 typeID = nc.command_data->GetItem(1)->AsInt()->value();
    uint32 groupID = sItemFactory.GetType(typeID)->groupID();
    switch (groupID) {
        case Command_Centers: {
            //take the money, send wallet blink event record the transaction in their journal.
            std::string reason = "DESC:  Command Center construction on ";
            reason += m_planet->GetName();
            AccountService::TranserFunds(
                        m_client->GetCharacterID(),
                        ownerUnknown,  // not sure who to send this to
                        90000,
                        reason.c_str(),
                        Journal::EntryType::PlanetaryConstruction,
                        m_planet->GetID(),
                        Account::KeyType::Cash);

            UUNCCommandCenter uunccc;
            if (!uunccc.Decode(nc.command_data)) {
                _log(SERVICE__ERROR, "Failed to decode args for UUNCCommandCenter!");
                nc.command_data->Dump(PLANET__WARNING, "      ");
            }
            m_colony->CreateCommandPin(uunccc.pinID, uunccc.typeID, uunccc.latitude, uunccc.longitude);
            if (!m_planet->GetCustomsOffice())
                m_planet->CreateCustomsOffice();
            return false;
        } break;
        case Mercenary_Bases:
        case Capsuleer_Bases:{
            // Not Supported yet
            _log(PLANET__ERROR, "PlanetMgr::UserUpdateNetwork::CreatePin() Planet Bases (type/group %u/%u) not supported.", typeID, groupID);
            return false;
        } break;
    }
    uint32 cost = 0;
    std::string pinString = "";
    switch (groupID) {
        case Storage_Facilities: {
            cost = 250000;
            pinString = "Silo";
        } break;
        case Processors: {
            switch (typeID) {
                case 2469:   //   Lava Basic Industry Facility
                case 2471:   //    Plasma Basic Industry Facility
                case 2473:   //    Barren Basic Industry Facility
                case 2481:   //    Temperate Basic Industry Facility
                case 2483:   //    Storm Basic Industry Facility
                case 2490:   //    Oceanic Basic Industry Facility
                case 2492:   //   Gas Basic Industry Facility
                case 2493: { //   Ice Basic Industry Facility
                    cost = 75000;
                    pinString = "Basic Plant";
                } break;
                case 2470:   //   Lava Advanced Industry Facility
                case 2472:   //   Plasma Advanced Industry Facility
                case 2474:   //   Barren Advanced Industry Facility
                case 2480:   //   Temperate Advanced Industry Facility
                case 2484:   //   Storm Advanced Industry Facility
                case 2485:   //   Oceanic Advanced Industry Facility
                case 2491:   //   Ice Advanced Industry Facility
                case 2494: { //   Gas Advanced Industry Facility
                    cost = 250000;
                    pinString = "Advanced Plant";
                } break;
                case 2475:   //   Barren High-Tech Production Plant
                case 2482: { //    Temperate High-Tech Production Plant
                    cost = 525000;
                    pinString = "High-Tech Plant";
                } break;
            }
        } break;
        case Extractor_Control_Units: {
            cost = 45000;
            pinString = "ECU";
        } break;
        case Spaceports: {
            cost = 900000;
            pinString = "LaunchPad";
        } break;
        case Planetary_Links: {
            cost = 1000;
            pinString = "Link";
        } break;
        case Extractors: {
            cost = 100;
            pinString = "Extractor Head";
        } break;
        UUNCStandardPin uuncsp;
        if (!uuncsp.Decode(nc.command_data)) {
            _log(SERVICE__ERROR, "Failed to decode args for UUNCStandardPin!");
            nc.command_data->Dump(PLANET__WARNING, "      ");
        }
        m_colony->CreatePin(groupID, uuncsp.pinID2, uuncsp.typeID, uuncsp.latitude, uuncsp.longitude);
    }
    //take the money, send wallet blink event record the transaction in their journal.
    std::string reason = "DESC:  ";
    reason += pinString.c_str();
    reason += " Construction on ";
    reason += m_planet->GetName();
    AccountService::TranserFunds(
                m_client->GetCharacterID(),
                ownerUnknown,  // not sure who to send this to
                cost,
                reason.c_str(),
                Journal::EntryType::PlanetaryConstruction,
                m_planet->GetID(),
                Account::KeyType::Cash);

    return false;
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
    _log(PLANET__TRACE, "PlanetMgr::UserUpdateNetwork::RemoveLink()");
    nc.Dump(PLANET__WARNING, "      ");

    m_colony->RemoveLink(nc.command_data->GetItem(0)->AsInt()->value(), nc.command_data->GetItem(1)->AsInt()->value());
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
    m_colony->UpgradeLink(nc.command_data->GetItem(0)->AsInt()->value(), nc.command_data->GetItem(1)->AsInt()->value(), nc.command_data->GetItem(2)->AsInt()->value());
}

void PlanetMgr::SetSchematic(UUNCommand& nc)
{
    uint32 pinID = 0;
    if (nc.command_data->GetItem(0)->IsTuple())
        pinID = nc.command_data->GetItem(0)->AsTuple()->GetItem(1)->AsInt()->value();
    else if (nc.command_data->GetItem(0)->IsInt())
        pinID = nc.command_data->GetItem(0)->AsInt()->value();
    uint16 schematicID = nc.command_data->GetItem(1)->AsInt()->value();  // 65 - 137
    m_colony->SetSchematic(pinID, schematicID);
}

void PlanetMgr::AddExtractorHead(UUNCommand& nc)
{
    Call_AddMoveExtractorHead args;
    if (!args.Decode(nc.command_data)) {
        _log(SERVICE__ERROR, "Failed to decode args for Call_AddMoveExtractorHead");
        nc.command_data->Dump(PLANET__WARNING, "      ");
    }

    m_colony->AddExtractorHead(args.ecuID, (uint16)args.headID, args.latitude, args.longitude);
}

void PlanetMgr::MoveExtractorHead(UUNCommand& nc)
{
    Call_AddMoveExtractorHead args;
    if (!args.Decode(nc.command_data)) {
        _log(SERVICE__ERROR, "Failed to decode args for Call_AddMoveExtractorHead");
        nc.command_data->Dump(PLANET__WARNING, "      ");
    }

    m_colony->MoveExtractorHead(args.ecuID, (uint16)args.headID, args.latitude, args.longitude);
}

void PlanetMgr::InstallProgram(UUNCommand& nc)
{
    Call_InstallProgram args;
    if (!args.Decode(nc.command_data)) {
        _log(SERVICE__ERROR, "Failed to decode args for Call_InstallProgram");
        nc.command_data->Dump(PLANET__WARNING, "      ");
    }

    m_colony->InstallProgram(args.ecuID, (uint16)args.typeID, args.headRadius);
}

void PlanetMgr::KillExtractorHead(UUNCommand& nc)
{
    m_colony->KillExtractorHead(nc.command_data->GetItem(0)->AsInt()->value(), nc.command_data->GetItem(1)->AsInt()->value());
}

void PlanetMgr::PrioritizeRoute(UUNCommand& nc)
{
    _log(PLANET__TRACE, "PlanetMgr::UserUpdateNetwork::PrioritizeRoute()");
    nc.Dump(PLANET__WARNING, "      ");

    m_colony->PrioritizeRoute();
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

/** @note   link distance formula from client
 *
 *    def GetDistance(self, otherInstallation):
 *        diffLong = self.longitude - otherInstallation.longitude
 *        cosDiffLong = math.cos(diffLong)
 *        cosMyLat = math.cos(self.latitude)
 *        sinMyLat = math.sin(self.latitude)
 *        cosOthLat = math.cos(otherInstallation.latitude)
 *        sinOthLat = math.sin(otherInstallation.latitude)
 *        nom1 = (cosMyLat + math.sin(diffLong)) ** 2
 *        nom2 = (cosMyLat * sinOthLat - sinMyLat * cosOthLat * cosDiffLong) ** 2
 *        denom = sinMyLat * sinOthLat + cosMyLat * cosOthLat * cosDiffLong
 *        return math.atan2(math.sqrt(nom1 + nom2), denom)
 */

/** @note CC Info
 *
commandCenterInfoPerLevel = {0: util.KeyVal(powerOutput=6000, cpuOutput=1675, upgradeCost=0),
 1: util.KeyVal(powerOutput=9000, cpuOutput=7057, upgradeCost=580000),
 2: util.KeyVal(powerOutput=12000, cpuOutput=12136, upgradeCost=1510000),
 3: util.KeyVal(powerOutput=15000, cpuOutput=17215, upgradeCost=2710000),
 4: util.KeyVal(powerOutput=17000, cpuOutput=21315, upgradeCost=4210000),
 5: util.KeyVal(powerOutput=19000, cpuOutput=25415, upgradeCost=6310000)}
 */

/** @note routing info (from client)
 *
    def FindShortestPath(self, sourcePin, destinationPin):
        """
            Simple shortest-path/undirected-graph thing.
            Uses a Dijkstra subfunction.
        """
        if sourcePin is None or destinationPin is None:
            return
        if sourcePin == destinationPin:
            return []
        distanceDict, predecessorDict = self.Dijkstra(sourcePin, destinationPin)
        if destinationPin not in distanceDict or destinationPin not in predecessorDict:
            return []
        path = []
        currentPin = destinationPin
        while currentPin is not None:
            path.append(currentPin.id)
            if currentPin is sourcePin:
                break
            if currentPin not in predecessorDict:
                raise RuntimeError("CurrentPin not in predecessor dict. There's no path. How did we get here?!")
            currentPin = predecessorDict[currentPin]

        path.reverse()
        if path[0] != sourcePin.id:
            return []
        return path

    def FindShortestPathIDs(self, sourcePinID, destinationPinID):
        return self.FindShortestPath(self.GetPin(sourcePinID), self.GetPin(destinationPinID))

    def Dijkstra(self, sourcePin, destinationPin):
        """
        (from http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/119466)
        Find shortest paths from the start vertex to all
        vertices nearer than or equal to the end.

        This particular implementation is adapted from the starMapSvc's
        copy of the algorithm, which is apparently unused.
        However, it matches up with the algorithm.
        """
        D = {}
        P = {}
        Q = planetCommon.priority_dict()
        Q[sourcePin] = 0.0
        while len(Q) > 0:
            vPin = Q.smallest()
            D[vPin] = Q[vPin]
            if vPin == destinationPin:
                break
            Q.pop_smallest()
            for wDestinationID in self.colonyData.GetLinksForPin(vPin.id):
                wLink = self.GetLink(vPin.id, wDestinationID)
                wPin = self.GetPin(wDestinationID)
                vwLength = D[vPin] + self._GetLinkWeight(wLink, wPin, vPin)
                if wPin in D:
                    if vwLength < D[wPin]:
                        raise ValueError, 'Dijkstra: found better path to already-final vertex'
                elif wPin not in Q or vwLength < Q[wPin]:
                    Q[wPin] = vwLength
                    P[wPin] = vPin

        return (D, P)
 */