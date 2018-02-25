
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

    uint32 scanTimer = m_client->GetShip()->GetAttribute(AttrScanSpeed).get_int();  // attrib 1123
    OnSystemScanStarted ossst;
        ossst.timestamp = GetFileTimeNow();
        ossst.duration = scanTimer;
        ossst.scanProbesDict = new PyDict();
    PyTuple* ev = ossst.Encode();
    m_client->SendNotification("OnSystemScanStarted", "charid", &ev);
    m_client->SetScanTimer(scanTimer);

    /** @note this works....but i dont like it...
    std::vector<CosmicSignature> sig;
    m_client->SystemMgr()->GetAnomMgr()->GetAnomalyList(sig);

    PyList* resultList = new PyList();
    for (auto sigs : sig) {
        SystemScanResultPositive ssrp;
            ssrp.typeID = sigs.sigTypeID;
            ssrp.scanGroupID = sigs.scanGroupID;
            ssrp.groupID = sigs.sigGroupID;
            ssrp.strengthAttributeID = sigs.scanAttributeID;
            ssrp.dungeonName = sigs.sigName;
            ssrp.id = sigs.sigID;
            ssrp.deviation = 0;
            ssrp.degraded = false;
            ssrp.probeID = m_client->GetShipID();
            ssrp.certainty = 1;
            ssrp.pos = new PyNone();
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
    OnSystemScanStopped osssp;
        osssp.scanProbesDict = probeDict;
        osssp.systemScanResult = resultList;
        osssp.absentTargets = mtList;
        ev = osssp.Encode();
    m_client->SendNotification("OnSystemScanStopped", "charid", &ev);
    */
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
    m_client->SystemMgr()->GetAnomMgr()->GetSignatureList(sig);

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

/*
 *  to calculate the maximum possible deviation you use the constants provided for the type of probe,
 * the scan size your probes are set to, and your skill level of Astrometric Pinpointing.
 * Here is the formula:
 * Max Deviation = (Scan Range/Base Scan Range) × Base Maximum Deviation × (1 ? Pinpointing Skill/10)
 *
 * Maximum deviation at different ranges and levels
 * Scan            Astrometric Pinpointing Skill Level
 * Range         0        1      2       3       4       5
 * 0.25 AU     0.125   0.1125  0.100   0.0875  0.075   0.0625
 * 0.5 AU      0.25    0.225   0.2     0.175   0.15    0.125      --Combat Scanner Probes have a minimum scan range of 0.5 AU.
 * 1 AU        0.5     0.45    0.4     0.35    0.3     0.25
 * 2 AU        1       0.9     0.8     0.7     0.6     0.5
 * 4 AU        2       1.8     1.6     1.4     1.2     1
 * 8 AU        4       3.6     3.2     2.8     2.4     2
 * 16 AU       8       7.2     6.4     5.6     4.8     4
 * 32 AU      16      14.4    12.8    11.2     9.6     8          --Core Scanner Probes haves a maximum scan range of 32 AU.
 * 64 AU      32      28.8    25.6    22.4    19.2    16
 *
 *
 * http://wiki.eve-inspiracy.com/index.php?title=Base_Signature_Strength
 * Band        1/5     1/10    1/15    1/20    1/25    1/40    1/45    1/60    1/80
 * Percentage  20.0%   10.0%   6.67%   5.0%    4.0%    2.5%    2.22%   1.67%   1.25%
 *
 * http://wiki.eve-inspiracy.com/index.php?title=Complete_Signature_Strength_List
 * Signature Bands in High Sec
 * Bands for Unknown Signatures
 * 1/5
 * (20.0%) Band    1/10
 * (10.0%) Band    1/15
 * (6.67%) Band    1/20
 * (5.0%) Band     1/25
 * (4.0%) Band     1/40
 * (2.5%) Band     1/60
 * (1.67%) Band    1/80
 * (1.25%) Band    Unknown
 */