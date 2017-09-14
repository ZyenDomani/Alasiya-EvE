
 /**
  * @name Scan.cpp
  *   Scanning methods for Alasiya EvE
  *
  * @Author:        Allan
  * @date:          7Dec15 (working)
  *
  */

// w.i.p.

#include "eve-server.h"

#include "Client.h"
#include "exploration/Scan.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/AnomalyMgr.h"

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
        osss.scanProbesDict = new PyDict();
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
    /** @todo  see client code to verify what it expects, and what it can calculate */
    std::vector<CosmicSignature> sig;
    m_client->SystemMgr()->GetAnomMgr()->GetAnomalyList(sig);

    PyList* resultList = new PyList();
    //. NOTE. cannot scan pos, wrecks, ships, mission sites, or escalations.  they DO have sigIDs, and can get to type (25%), but no farther
    for (auto sigs : sig) {
        SystemScanResultPositive ssrp;
            ssrp.typeID = sigs.sigTypeID;
            ssrp.scanGroupID = sigs.scanGroupID;
            ssrp.groupID = sigs.sigGroupID;
            ssrp.strengthAttributeID = sigs.scanAttributeID;
            ssrp.dungeonName = sigs.sigName;
            ssrp.id = sigs.sigID;
            ssrp.deviation = 0;     /* for scan probes */
            ssrp.degraded = false;  /* will need to be set in *some* kind of test/conditional */
            ssrp.probeID = m_client->GetShipID();   /* will need to be corrected after implementing probes */
            ssrp.certainty = 1; //((ssrp.typeID == EVEDB::invTypes::typeCosmicAnomaly) ? 1 : MakeRandomFloat());     /* will need to be fixed.  anomalies are full...others are random */
            ssrp.pos = new PyNone();  /* this is for probe positions (where applicable).  it uses the 'foo.Vector3' token, and coded in scan.xmlp */
        SSR_ObjectEx_Pos ssr_oed;
            ssr_oed.x = sigs.x;
            ssr_oed.y = sigs.y;
            ssr_oed.z = sigs.z;
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
