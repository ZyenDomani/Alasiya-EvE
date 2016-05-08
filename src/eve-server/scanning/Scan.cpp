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
#include "scanning/Scan.h"
#include "system/SystemBubble.h"

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
    sLog.Log( "Scan::RequestScans()", "called by %s in system %u, bubble %u", \
        m_client->GetName(), m_client->GetSystemID(), m_client->GetShipSE()->SysBubble()->GetID() );

    bool useProbe = false;
    if (dict) {
        useProbe = true;
        // put probe code here.
        //will probably be complicated.
    }

    // if probe, get current positions, move as needed (actually, just simulate by removing from bubble, time on distance, add to new bubble)
    //  query possible items within scan range, return result. rinse, repeat as needed.

    /* will have to write...
     *  an anomoly handling class/system,       ...started - system/AnomolyMgr.cpp
     *  a dungeon creation/managing system,     ...started - dungeon/DungeonMgr.cpp
     *  a wormhole system,                      ...started - system/WormholeMgr.cpp
     *  whatever else i find we need as i get to it.
     */

    //NOTE  for now, we are returning hard-coded data for appearance and bugfinding.

    uint32 scanTimer = m_client->GetShip()->GetAttribute(AttrScanSpeed).get_int();
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
    DBQueryResult* res = new DBQueryResult();
    m_db->GetScanResults(*res);
    PyList* resultList = new PyList;

    DBResultRow row;
    //(`typeID`, `scanGroupID`, `groupID`, `strengthAttributeID`, `dungeonName`, `id`, `x`, `y`, `z`)
    while (res->GetRow(row)) {
        SSR_ObjectEx_Pos ssr_oed;
            ssr_oed.x = row.GetDouble(6);
            ssr_oed.y = row.GetDouble(7);
            ssr_oed.z = row.GetDouble(8);

        SystemScanResultPositive ssrp;
            ssrp.typeID = row.GetInt(0);
            ssrp.scanGroupID = row.GetInt(1);
            ssrp.groupID = row.GetInt(2);
            ssrp.strengthAttributeID = row.GetInt(3);
            ssrp.dungeonName = row.GetText(4);
            ssrp.id = row.GetText(5);
            ssrp.deviation = 0;
            ssrp.degraded = false;
            ssrp.probeID = m_client->GetShipID();
            ssrp.certainty = 1;
            ssrp.pos = new PyNone;

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
