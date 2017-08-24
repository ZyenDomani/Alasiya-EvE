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
    Author:     Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "exploration/Scan.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"

Scan::Scan(Client* pClient)
{
    m_client = pClient;
}

Scan::~Scan() {
    m_client = nullptr;
}

PyRep* Scan::ConeScan(Call_ConeScan args) {
    //  WORKING CODE...DONT FUCK WITH THIS!!  -allan 7Dec15
    size_t index = 0;
    std::vector<SystemEntity*> vector;
    m_client->GetShipSE()->SysBubble()->GetEntities(vector);
    PyList* list = new PyList(vector.size());
    DirectionScanResult res;
    for (auto i : vector) {
        res.id         = i->GetID();
        res.typeID     = i->GetSelf()->typeID();
        res.groupID    = i->GetSelf()->groupID();
        res.categoryID = i->GetSelf()->categoryID();
        list->SetItem(index, res.Encode());
        ++index;
    }

    return list;
}

void Scan::RequestScans(PyDict* dict) {
    sLog.White( "Scan::RequestScans()", "called by %s in system %u, bubble %u", \
        m_client->GetName(), m_client->GetSystemID(), m_client->GetShipSE()->SysBubble()->GetID() );

    bool useProbe = false;
    if (dict != nullptr) {
        useProbe = true;
        // put probe code here.
        //will probably be complicated.
    }

    // if probe, get current positions, move as needed (actually, just simulate by removing from bubble, time on distance, add to new bubble)
    //  query possible items within scan range, return result. rinse, repeat as needed.

    /* will have to write...
     *  an anomoly handling class/system,       ...started - system/cosmicMgrs/AnomolyMgr.cpp
     *  a dungeon creation/managing system,     ...started - system/cosmicMgrs/DungeonMgr.cpp
     *  a wormhole system,                      ...started - system/cosmicMgrs/WormholeMgr.cpp
     *  whatever else i find we need as i get to it.
     */

    //NOTE  for now, we are returning hard-coded data for appearance and bugfinding.

    uint32 scanTimer = m_client->GetShip()->GetAttribute(AttrScanSpeed).get_int();  // attrib 1123
    OnSystemScanStarted osss;
        osss.timestamp = Win32TimeNow();
        osss.duration = scanTimer;
        osss.scanProbesDict = new PyDict;
    PyTuple* ev = osss.Encode();
    m_client->SendNotification("OnSystemScanStarted", "charid", &ev);
    m_client->SetScanTimer(scanTimer);

    return;
}

/*
 * Data()
{

  scanSvc.GetProbeData (probeID)
  scanSvc.GetActiveProbes (probeID)
  scanSvc.GetProbeLabel(probe.probeID)
} */

void Scan::ScanStart()
{

}

void Scan::ScanResult() {
    //  WORKING CODE...DONT FUCK WITH THIS!!  -allan 11Dec15
    /** @todo basic code works.  will need updates and calc's for various things once system matures.  see notes */
    DBQueryResult* res = new DBQueryResult();
    m_db->GetSystemAnomalies(m_client->GetSystemID(), *res);
    PyList* resultList = new PyList;

    DBResultRow row;
    //(`typeID`, `scanGroupID`, `groupID`, `strengthAttributeID`, `dungeonName`, `sigID`, `x`, `y`, `z`)
    while (res->GetRow(row)) {
        SystemScanResultPositive ssrp;
            ssrp.typeID = row.GetInt(0);
            ssrp.scanGroupID = row.GetInt(1);   // see notes in CosmicMgrs/ManagerDB.h
            ssrp.groupID = row.GetInt(2);
            ssrp.strengthAttributeID = row.GetInt(3);  // see notes in CosmicMgrs/ManagerDB.h
            ssrp.dungeonName = row.GetText(4);
            ssrp.id = row.GetText(5);
            ssrp.deviation = 0;     /* for scan probes */
            ssrp.degraded = false;  /* will need to be set in *some* kind of test/conditional */
            ssrp.probeID = m_client->GetShipID();   /* will need to be corrected after implementing probes */
            ssrp.certainty = 1;     /* will need to be fixed. */
            ssrp.pos = new PyNone();  /* this is for probe positions (where applicable).  it uses the 'foo.Vector3' token, and coded in scan.xmlp */
        SSR_ObjectEx_Pos ssr_oed;
            ssr_oed.x = row.GetDouble(6);
            ssr_oed.y = row.GetDouble(7);
            ssr_oed.z = row.GetDouble(8);
        PyToken* token = new PyToken("foo.Vector3");
        PyTuple* oed_tuple = new PyTuple(2);
            oed_tuple->SetItem(0, token);
            oed_tuple->SetItem(1, ssr_oed.Encode());

        ssrp.data = new PyObjectEx(false, oed_tuple);  // oed goes here

        resultList->AddItem(ssrp.Encode());
    }

    // dict and list are both empty for now.
    PyDict* probeDict = new PyDict;
    PyList* mtList = new PyList(0);
    OnSystemScanStopped osss;
        osss.scanProbesDict = probeDict;
        osss.systemScanResult = resultList;
        osss.absentTargets = mtList;
    PyTuple* ev = osss.Encode();
    m_client->SendNotification("OnSystemScanStopped", "charid", &ev);

    SafeDelete(res);
}

void Scan::SurveyScan() {
    /*
     *
     *                      [PyString "OnSpecialFX"]
     *                      [PyTuple 14 items]
     *                        [PyIntegerVar 1002331681462]
     *                        [PyIntegerVar 1002332233248]
     *                        [PyInt 444]
     *                        [PyNone]
     *                        [PyNone]
     *                        [PyList 0 items]
     *                        [PyString "effects.SurveyScan"]
     *                        [PyBool False]
     *                        [PyInt 1]
     *                        [PyInt 1]
     *                        [PyFloat 4250]
     *                        [PyInt 0]
     *                        [PyIntegerVar 129509430135552798]
     *                        [PyNone]
     *
     */
}

/*
You can see in the scan window whether a system contains cosmic signatures,
but identifying them and pinpointing them requires scan probes.
The type is identified at 25% scan,
 the name is revealed at 75% scan
 the site is warpable at 100% scan.

Radar, Ladar, Gravimetric and Magnetometric sites will show type at 25% signal strength.
'Unknown' = Wormhole or Combat site will show type at 75% signal strength.
*/
 /*
  to calculate the maximum possible deviation you use the constants provided for the type of probe, the scan size your probes are set to, and your skill level of Astrometric Pinpointing.
Here is the formula:
Max Deviation = (Scan Range/Base Scan Range) × Base Maximum Deviation × (1 ? Pinpointing Skill/10)

 Maximum deviation at different ranges and levels
Scan Range  Astrometric Pinpointing Skill Level
0   1   2   3   4   5
0.25 AU1    0.125   0.1125  0.100   0.0875  0.075   0.0625
0.5 AU  0.25    0.225   0.2 0.175   0.15    0.125
1 AU    0.5 0.45    0.4 0.35    0.3 0.25
2 AU    1   0.9 0.8 0.7 0.6 0.5
4 AU    2   1.8 1.6 1.4 1.2 1
8 AU    4   3.6 3.2 2.8 2.4 2
16 AU   8   7.2 6.4 5.6 4.8 4
32 AU   16  14.4    12.8    11.2    9.6 8
64 AU2  32  28.8    25.6    22.4    19.2    16
1 Combat Scanner Probes have a minimum scan range of 0.5 AU.
2 Core Scanner Probes haves a maximum scan range of 32 AU.
*/

/*
 * class CosmicSignature {
 * public:
 *    std::string sigID;  // this is unique xxx-nnn id displayed in scanner
 *    std::string dungeonName;
 *    uint32 systemID;
 *    uint32 sigItemID;   // itemID of this entry
 *    uint16 typeID;
 *    uint16 groupID;
 *    uint16 scanGroupID; // see below
 *    uint16 strengthAttributeID; // see below
 *    double x;
 *    double y;
 *    double z;
 * };
 */

/*
AttrScanGravimetricStrengthBonus = 238,
AttrScanLadarStrengthBonus = 239,
AttrScanMagnetometricStrengthBonus = 240,
AttrScanRadarStrengthBonus = 241,
AttrScanSpeedMultiplier = 242,
*/

/*
    uint32 shipID = itemID();
    PyDict* chargeDict = new PyDict;
    for (auto cur : charges)
        chargeDict->SetItem(new PyInt((uint32)cur.first), cur.second->GetChargeStatusRow(shipID));

    PyToken* token = new PyToken("util.IndexedRows");
    PyTuple* tuple2 = new PyTuple(1);
        tuple2->SetItem(0, token);
    PyTuple* tuple1 = new PyTuple(2);
        tuple1->SetItem(0, tuple2);
        tuple1->SetItem(1, new PyDict);

    PyDict *result = new PyDict;
        result->SetItem(new PyInt(itemID()), new PyObjectEx_Type2(tuple1, chargeDict));
*/
