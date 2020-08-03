
 /**
  * @name Scan.cpp
  *   Scanning methods for Alasiya EvE
  *
  * @Author:        Allan
  * @date:          7Dec15 (working)
  * @udpate:        12Mar18 (probes)
  *
  */

// w.i.p.

/* SCAN__ERROR
 * SCAN__WARNING
 * SCAN__MESSAGE
 * SCAN__DEBUG
 * SCAN__INFO
 * SCAN__TRACE
 * SCAN__DUMP
 * SCAN__RSPDUMP
 */

#include "eve-server.h"

#include "Client.h"
#include "StatisticMgr.h"
#include "exploration/Scan.h"
#include "Probes.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/AnomalyMgr.h"

Scan::Scan(Client* pClient)
: m_client(pClient),
  m_system(pClient->SystemMgr())
{
    m_probeScan = false;
}

void Scan::AddProbe(ProbeSE* pProbe)
{
    pProbe->SetScan(this);
    m_probeMap.emplace(pProbe->GetID(), pProbe);
}

void Scan::RemoveProbe(ProbeSE* pProbe)
{
    m_probeMap.erase(pProbe->GetID());
    pProbe->RemoveScan();
}

// this will need it's own timer call....not sure how yet.
void Scan::ProcessScan(bool useProbe/*false*/)
{
    if (!useProbe) {
        ShipScanResult();
        return;
    }
    if (m_probeScan) {
        _log(SCAN__TRACE, "Scan::ProcessScan() - m_probeScan = true for %s in system %u", m_client->GetName(), m_client->GetSystemID());
        for (auto cur : m_probeMap) // may not need this
            cur.second->SendStateChange(Probe::State::Idle);
        ProbeScanResult();
        return;
    }

    bool idle = true;
    uint16 ntime = 0, duration = m_client->GetShip()->GetAttribute(AttrDuration).get_int();
    if (duration < 1000)
        duration = 10000;    // 10s default probe scan time.
    for (auto cur : m_probeMap) {
        if (cur.second->IsMoving()) {
            idle = false;
            ntime = cur.second->GetMoveTime() *100;
            if (ntime < duration)
                duration = ntime;
        }
    }
    if (idle) {
        //duration *= (1 - (0.05 * m_client->GetChar()->GetSkillLevel(EvESkill::Astrometrics)));           // −5% scan probe scan time per level
        //duration *= (1 - (0.1 * m_client->GetChar()->GetSkillLevel(EvESkill::AstrometricAcquisition)));  // −10% scan probe scan time per level
        m_probeScan = true;
        SystemScanStarted(duration);
        for (auto cur : m_probeMap) {
            //cur.second->SendStateChange(Probe::State::Scanning);
            cur.second->StartStateTimer(duration);
        }
    }
    m_client->SetScanTimer(duration, true);
}


PyRep* Scan::ConeScan(Call_ConeScan args) {
    /** @todo  this needs to use given args to determine objects found... */
    //  WORKING CODE...DONT FUCK WITH THIS!!  -allan 7Dec15
    /*
     * 01:16:27 [Bound] ScanBound::ConeScan()
     * 01:16:27 [ScanTrace] ScanBound::Handle_ConeScan() - size= 5
     * 01:16:27 [ScanDump]   Call Arguments:
     * 01:16:27 [ScanDump]      Tuple: 5 elements
     * 01:16:27 [ScanDump]       [ 0]       Real: 6.283185      <- ScanAngle in rads
     * 01:16:27 [ScanDump]       [ 1]    Integer: 10000000      <- range in m
     * 01:16:27 [ScanDump]       [ 2]       Real: 0.000000      <- x
     * 01:16:27 [ScanDump]       [ 3]       Real: 0.000000      <- y
     * 01:16:27 [ScanDump]       [ 4]       Real: -1.000000     <- z
     */
    std::vector<SystemEntity*> vector;
    if (m_client->IsShowall())
        m_client->SystemMgr()->GetCurrentEntities(vector);
    else
        m_client->GetShipSE()->SysBubble()->GetEntityVec(vector);
    PyList* list = new PyList();
    for (auto cur : vector) {
        DirectionScanResult res;
            res.id         = cur->GetID();
            res.typeID     = cur->GetSelf()->typeID();
            res.groupID    = cur->GetSelf()->groupID();
            res.categoryID = cur->GetSelf()->categoryID();  // this may not be needed.  client code only asks for id,type,group
        list->AddItem(res.Encode());
    }

    return list;
}

void Scan::RequestScans(PyDict* dict) {
    uint16 duration = m_client->GetShip()->GetAttribute(AttrScanSpeed).get_uint32();
    if ((dict == nullptr) or dict->empty()) {
        _log(SCAN__INFO, "Scan::RequestScans() called by %s in %s using ship scanner.", \
                m_client->GetName(), m_client->GetSystemName().c_str());

        OnSystemScanStarted ossst;
            ossst.timestamp = GetFileTimeNow();
            ossst.duration = duration;
            ossst.scanProbesDict = new PyDict();
        PyTuple* ev = ossst.Encode();
        m_client->SendNotification("OnSystemScanStarted", "charid", &ev);
        m_client->SetScanTimer(duration);
        return;
    }

    _log(SCAN__INFO, "Scan::RequestScans() called by %s in %s using %u probes.",\
            m_client->GetName(), m_client->GetSystemName().c_str(), dict->size());

    uint32 probeID = 0;
    PyDict::const_iterator cItr = dict->begin();
    for (; cItr != dict->end(); ++cItr) {
        // find probe in map....
        probeID = PyRep::IntegerValueU32(cItr->first);  // key
        std::map<uint32, ProbeSE*>::iterator pItr = m_probeMap.find(probeID);
        if (pItr == m_probeMap.end()) {
            _log(SCAN__ERROR, "Probe %u wasnt found in the probeMap for %s(%u)", probeID, \
                    m_client->GetName(), m_client->GetCharacterID());
            continue;  // make error here?
        }

        Call_ProbeDataObj args;
        if (!args.Decode(cItr->second)) { // value
            _log(SERVICE__ERROR, "Scan::RequestScans::DecodeProbeData: Failed to decode arguments.");
            // make error here
            continue;
        }

        ProbeData data = ProbeData();
            data.state = args.state;    // do we need this?
            data.expiry = args.expiry;
            data.rangeStep = args.rangeStep;
            data.scanRange = args.scanRange;
        // set probe target
        PyObjectEx* obj = args.destination->AsObjectEx();
        PyTuple* dest = obj->header()->AsTuple()->GetItem(1)->AsTuple();
            data.dest.x = dest->GetItem(0)->AsFloat()->value();
            data.dest.y = dest->GetItem(1)->AsFloat()->value();
            data.dest.z = dest->GetItem(2)->AsFloat()->value();
        pItr->second->UpdateProbe(data);
    }

    m_client->SetScanTimer(duration, true);
}

void Scan::SystemScanStarted(uint16 duration)
{
    _log(SCAN__TRACE, "Scan::SystemScanStarted()  for %s in system %u", m_client->GetName(), m_client->GetSystemID());

    GPoint pos(NULL_ORIGIN);
    PyDict* probeDict = new PyDict();
    for (auto cur : m_probeMap) {
        // probe data here...
        ScanProbesDict spd;
            spd.expiry = cur.second->GetExpiryTime();
            spd.maxDeviation = cur.second->GetDeviation();
            spd.probeID = cur.first;
            spd.state = cur.second->GetState();
            spd.rangeStep = cur.second->GetRangeStep();
            spd.scanRange = cur.second->GetScanRange();
            spd.scanStrength = cur.second->GetScanStrength();
            spd.typeID = cur.second->GetSelf()->typeID();
        pos = cur.second->GetPosition();
        ScanResultPos srp;
            srp.x = pos.x;
            srp.y = pos.y;
            srp.z = pos.z;
        PyToken* token = new PyToken("foo.Vector3");
        PyTuple* oed_tuple = new PyTuple(2);
            oed_tuple->SetItem(0, token);
            oed_tuple->SetItem(1, srp.Encode());
        spd.pos = new PyObjectEx(false, oed_tuple);  // oed goes here
        /*
        pos = cur.second->GetDestination();
        ScanResultPos dest;
            dest.x = pos.x;
            dest.y = pos.y;
            dest.z = pos.z;
        PyIncRef(token);
        PyTuple* oed_tuple_dest = new PyTuple(2);
            oed_tuple_dest->SetItem(0, token);
            oed_tuple_dest->SetItem(1, dest.Encode());
        spd.destination = new PyObjectEx(false, oed_tuple_dest);  // oed goes here
        */
        PyIncRef(spd.pos);
        spd.destination = spd.pos;
        probeDict->SetItem(new PyInt(cur.first), spd.Encode());
    }

    OnSystemScanStarted ossst;
        ossst.timestamp = GetFileTimeNow();
        ossst.duration = duration;
        ossst.scanProbesDict = probeDict;
    PyTuple* ev = ossst.Encode();
    ev->Dump(SCAN__RSPDUMP, "sss-    ");
    m_client->SendNotification("OnSystemScanStarted", "charid", &ev, false);
}

void Scan::ShipScanResult() {
    //  WORKING CODE...DONT FUCK WITH THIS!!  -allan 11Dec15
    /** @todo  see client code to verify what it expects, and what it can calculate */
    // client scan data found in EVE_Scanning.h
    std::vector<CosmicSignature> anom;
    m_system->GetAnomMgr()->GetAnomalyList(anom);
    if (m_client->IsShowall()) {
        m_system->GetAllEntities(anom);
        sBubbleMgr.GetBubbleCenterMarkers(m_system->GetID(), anom);
    }

    PyList* resultList = new PyList();
    // NOTE. cannot scan pos, wrecks, ships, mission sites, or escalations.  they DO have sigIDs, and can get to type (25%), but no farther
    for (auto anoms : anom) {
        SystemScanResult ssr;
            ssr.typeID = anoms.sigTypeID;
            ssr.scanGroupID = anoms.scanGroupID;
            ssr.groupID = anoms.sigGroupID;
            ssr.strengthAttributeID = anoms.scanAttributeID;
            ssr.dungeonName = anoms.sigName;
            ssr.id = anoms.sigID;
            ssr.deviation = 0;     /* for scan probes */
            ssr.degraded = false;  /* dunno what this does.  have only seen 'false' in packets */
            ssr.probeID = new PyInt(m_client->GetShipID());
            ssr.certainty = 1;
            ssr.pos = new PyNone();
        ScanResultPos ssr_oed;
            ssr_oed.x = anoms.position.x;
            ssr_oed.y = anoms.position.y;
            ssr_oed.z = anoms.position.z;
        PyTuple* oed_tuple = new PyTuple(2);
            oed_tuple->SetItem(0, new PyToken("foo.Vector3"));
            oed_tuple->SetItem(1, ssr_oed.Encode());
        ssr.data = new PyObjectEx(false, oed_tuple);  // oed goes here
        resultList->AddItem(ssr.Encode());
    }

    // dict and list are both empty for now.
    PyDict* probeDict = new PyDict();
    PyList* mtList = new PyList();
    OnSystemScanStopped osss;
        osss.scanProbesDict = probeDict; // this seems to be just a list of probeIDs
        osss.systemScanResult = resultList;
        osss.absentTargets = mtList;
    PyTuple* ev = osss.Encode();
    ev->Dump(SCAN__RSPDUMP, "ssr-    ");
    m_client->SendNotification("OnSystemScanStopped", "charid", &ev);
}

void Scan::ProbeScanResult()
{
    // this will use outline of above code, but be MUCH more complicated...
    _log(SCAN__TRACE, "Scan::ProbeScanResult()  for %s in system %u", m_client->GetName(), m_client->GetSystemID());

    PyList* resultList = new PyList();
    std::vector<CosmicSignature> sig, anom;

    m_system->GetAnomMgr()->GetAnomalyList(anom);
    for (auto anoms : anom) {
        SystemScanResult ssr;
            ssr.typeID = anoms.sigTypeID;
            ssr.scanGroupID = anoms.scanGroupID;
            ssr.groupID = anoms.sigGroupID;
            ssr.strengthAttributeID = anoms.scanAttributeID;
            ssr.dungeonName = anoms.sigName;
            ssr.id = anoms.sigID;
            ssr.deviation = 0;     /* 0 for anomalies */
            ssr.degraded = false;
            ssr.probeID = new PyInt(m_client->GetShipID());
            ssr.certainty = anoms.sigStrength;
            ssr.pos = new PyNone();
        ScanResultPos ssr_oed;
            ssr_oed.x = anoms.position.x;
            ssr_oed.y = anoms.position.y;
            ssr_oed.z = anoms.position.z;
        PyToken* token = new PyToken("foo.Vector3");
        PyTuple* oed_tuple = new PyTuple(2);
            oed_tuple->SetItem(0, token);
            oed_tuple->SetItem(1, ssr_oed.Encode());
        ssr.data = new PyObjectEx(false, oed_tuple);  // oed goes here
        resultList->AddItem(ssr.Encode());
    }

    m_system->GetAnomMgr()->GetSignatureList(sig);
    for (auto sigs : sig) {
        SignalData data = SignalData();
            data.sig = sigs;
            data.probes = nullptr;
            data.probePos = nullptr;
        if (GetProbeDataForSig(data)) {
            SystemScanResult ssr;
                ssr.id = sigs.sigID;
                ssr.dungeonName = sigs.sigName;
                ssr.typeID = sigs.sigTypeID;
                ssr.groupID = sigs.sigGroupID;
                ssr.scanGroupID = sigs.scanGroupID;
                ssr.strengthAttributeID = sigs.scanAttributeID;
                ssr.degraded = false;
                ssr.deviation = data.deviation; //deviation is the distance between the scan result shown on the map and the actual location of your target.
                ssr.certainty = data.certainty; // this is listed as "signal strength" in scan window
                ssr.probeID = data.probes;
                ssr.pos = data.probePos;
            ScanResultPos ssr_oed;
                ssr_oed.x = data.sig.position.x;
                ssr_oed.y = data.sig.position.y;
                ssr_oed.z = data.sig.position.z;
            PyToken* token = new PyToken("foo.Vector3");
            PyTuple* oed_tuple = new PyTuple(2);
                oed_tuple->SetItem(0, token);
                oed_tuple->SetItem(1, ssr_oed.Encode());
            ssr.data = new PyObjectEx(false, oed_tuple);  // oed goes here
            resultList->AddItem(ssr.Encode());
        }
    }

    GPoint pos(NULL_ORIGIN);
    PyDict* probeDict = new PyDict();
    for (auto cur : m_probeMap) {
        // probe data here...
        ScanProbesDict spd;
        spd.expiry = cur.second->GetExpiryTime();
        spd.maxDeviation = cur.second->GetDeviation();
        pos = cur.second->GetPosition();
        ScanResultPos ssr_oed;
            ssr_oed.x = pos.x;
            ssr_oed.y = pos.y;
            ssr_oed.z = pos.z;
        PyToken* token = new PyToken("foo.Vector3");
        PyTuple* oed_tuple = new PyTuple(2);
            oed_tuple->SetItem(0, token);
            oed_tuple->SetItem(1, ssr_oed.Encode());
        spd.pos = new PyObjectEx(false, oed_tuple);  // oed goes here
        spd.destination = spd.pos;
        spd.probeID = cur.first;
        spd.state = cur.second->GetState();
        spd.rangeStep = cur.second->GetRangeStep();
        spd.scanRange = cur.second->GetScanRange();
        spd.scanStrength = cur.second->GetScanStrength();
        spd.typeID = cur.second->GetSelf()->typeID();
        probeDict->SetItem(new PyInt(cur.first), spd.Encode());
    }

    // this will be sigs that are no longer present in current scan range
    //  will have to keep previous list and compare with current list to populate this
    // may not be used.....testing
    PyList* absentList = new PyList();

    OnSystemScanStopped osssp;
        osssp.scanProbesDict = probeDict;
        osssp.systemScanResult = resultList;
        osssp.absentTargets = absentList;
    PyTuple* ev = osssp.Encode();
    ev->Dump(SCAN__RSPDUMP, "psr-    ");
    m_client->SendNotification("OnSystemScanStopped", "charid", &ev);
}


/*  probeID is for those probes that pick up this signal.
 * pos is probe position
 *    will have to call a method to determine sig type, probe data, and signal specifics for each signal
 */
bool Scan::GetProbeDataForSig(SignalData& data)
{
    /*  this will determine sig position vs probe position, range, strength
     * to decide if this sig is picked up by a probe, and return probe data
     * based on that.
     *
struct CosmicSignature {
    uint8 dungeonType;
    uint16 sigTypeID;
    uint16 sigGroupID;
    uint16 scanGroupID;
    uint16 scanAttributeID;
    uint32 ownerID;
    uint32 systemID;
    uint32 sigItemID;   // itemID of this entry
    GPoint position;
    std::string sigID;  // this is unique xxx-nnn id displayed in scanner
    std::string sigName;
};
     */

    float dist = 0;
    std::vector<ProbeSE*> probeVec;
    // this will use actual distance from signal to probe.  later, we will 'adjust' distance based on deviation
    for (auto cur : m_probeMap) {
        dist = cur.second->GetPosition().distance(data.sig.position);
        _log(SCAN__DEBUG, "Scan::GetProbeDataForSig()  distance from probe %u to signal '%s' -> %.2f", cur.first, data.sig.sigName.c_str(), dist);
        if (cur.second->GetScanRange() > dist)
            probeVec.push_back(cur.second);
    }

    _log(SCAN__TRACE, "Scan::GetProbeDataForSig()  probeVec size: %u for signal %s", probeVec.size(), data.sig.sigName.c_str());

    if (probeVec.empty())
        return false;

    // probeID is integer for single probe, PyTuple for multiple.
    GetSignalData(data, probeVec, data.sig.position);
    if (probeVec.size() > 1) {
        PyTuple* tuple = new PyTuple(probeVec.size());
        PyList* list = new PyList();
        uint8 count = 0;
        GPoint pos(NULL_ORIGIN);
        for (auto cur : probeVec) {
            tuple->SetItem(count++, new PyInt(cur->GetID()));
            pos = cur->GetPosition();
            ScanResultPos ssr_oed;
                ssr_oed.x = pos.x;
                ssr_oed.y = pos.y;
                ssr_oed.z = pos.z;
            PyToken* token = new PyToken("foo.Vector3");
            PyTuple* oed_tuple = new PyTuple(2);
                oed_tuple->SetItem(0, token);
                oed_tuple->SetItem(1, ssr_oed.Encode());
            list->AddItem(new PyObjectEx(false, oed_tuple));
        }
        data.probes = tuple;
        // there is *something* here where one of the positions is given as a nested list of objects
        data.probePos = list;
    } else {
        ScanResultPos ssr_oed;
            ssr_oed.x = probeVec.at(0)->GetPosition().x;
            ssr_oed.y = probeVec.at(0)->GetPosition().y;
            ssr_oed.z = probeVec.at(0)->GetPosition().z;
        PyToken* token = new PyToken("foo.Vector3");
        PyTuple* oed_tuple = new PyTuple(2);
            oed_tuple->SetItem(0, token);
            oed_tuple->SetItem(1, ssr_oed.Encode());
        data.probes = new PyInt(probeVec.at(0)->GetID());
        data.probePos = new PyObjectEx(false, oed_tuple);
    }
    return true;
}

void Scan::GetSignalData(SignalData& data, std::vector<ProbeSE*>& probeVec, GPoint& point)
{
    uint8 probeCount = probeVec.size();
    float probeMultiplier = 0.0f;
    switch(probeCount) {
        //  new style...already calculated (in python) for 1 to 8 probes...
        case 1: probeMultiplier = 0.25774312594204907; break;
        case 2: probeMultiplier = 0.5130245854773758; break;
        case 3: probeMultiplier = 0.7234132613571191; break;
        case 4: probeMultiplier = 0.8824741410676007; break;
        case 5: probeMultiplier = 0.9963325352118082; break;
        case 6: probeMultiplier = 1.0754155621393995; break;
        case 7: probeMultiplier = 1.1296251734489133; break;
        case 8: probeMultiplier = 1.1666968137637062; break;
    }

    /** @todo  why did i put this switch here???
    // NOTE: cannot scan pos, wrecks, ships, mission sites, or escalations.  they DO have sigIDs, and can get to type (25%), but no farther
    switch(data.sig.scanGroupID) {
        case Scanning::Group::Anomaly: //detected using ship sensors  - will not hit here.
        case Scanning::Group::Celestial://unknown  (unused)
        case Scanning::Group::Scrap: //wrecks in system (unused)
        case Scanning::Group::DroneOrProbe://player items
        case Scanning::Group::Ship: //abandoned ships
        case Scanning::Group::Structure: //all pos structures
        case Scanning::Group::Signature: { //advanced anomaly.  need probes to scan
            switch (data.sig.dungeonType) {
                case Dungeon::Type::Mission: // npc mission
                case Dungeon::Type::Gravimetric:// roids
                case Dungeon::Type::Magnetometric:// salvage and archeology
                case Dungeon::Type::Radar:// hacking
                case Dungeon::Type::Ladar: // gas mining
                case Dungeon::Type::Wormhole:
                case Dungeon::Type::Anomaly:// non-rated dungeon that isnt required to scan with probes - will not hit here.
                case Dungeon::Type::Unrated:// non-rated dungeon  no waves, possible escalation to complex
                case Dungeon::Type::Escalation://  new dungeon from previous site. very limited access
                case Dungeon::Type::Rated:// DED rated dungeon
                    break;
            }
        } break;
    }  */

    /** @todo...determine probe angles to target (for >1 probe) to modify scan strength of probe.  */
    data.deviation = 0;
    float scanStrength = 0, rangeMod = 0, dist = 0;
    if (probeCount > 1) {
        /*  loop thru probes and get range mods and sigStrength for each.
         *  combine all probe's data to get good sum based on probe range and strength
         */
        int8 count = 0;
        float probeSig = 0;
        for (auto cur : probeVec) {
            dist = cur->GetPosition().distance(point);
            rangeMod = cur->GetRangeModifier(dist);     //log(-(pow(dist /32, 2)));
            scanStrength = cur->GetScanStrength();
            data.deviation += cur->GetDeviation();  // combine deviation (for now...may find a better way later.)
            probeSig = data.sig.sigStrength * scanStrength * probeMultiplier * rangeMod;
            data.certainty += probeSig;
            _log(SCAN__TRACE, "Scan::GetSignalData()  Probe #%u - dist: %.4fAU,  rangeMod: %.5f, scanStrength: %.5f, multiplier: %.5f, probeSig: %.5f", \
                    ++count, dist /ONE_AU_IN_METERS, rangeMod, scanStrength, probeMultiplier, probeSig);
        }
        // get average deviation from all probes
        data.deviation /= count;
    } else {
        dist = (probeVec.at(0)->GetPosition().distance(point));
        rangeMod = probeVec.at(0)->GetRangeModifier(dist);      //log(-(pow(dist /32, 2)));
        scanStrength = probeVec.at(0)->GetScanStrength();
        data.deviation = probeVec.at(0)->GetDeviation();
        data.certainty = data.sig.sigStrength * scanStrength * probeMultiplier * rangeMod /2;
        _log(SCAN__TRACE, "Scan::GetSignalData()  single - dist: %.4fAU, rangeMod: %.5f, scanStrength: %.5f, multiplier: %.5f", \
                dist /ONE_AU_IN_METERS, rangeMod, scanStrength, probeMultiplier);
    }

    // set minimum to 0.01%  nothing less will show in client
    if (data.certainty < 0.0001)
        data.certainty = 0.0001;

    data.deviation *= (1 - (data.certainty > 1.0f ? 0.98 : data.certainty));

    if (data.certainty > 0.99)
        sStatMgr.Increment(Stat::sitesScanned);

    // modify reported signal position based on deviation
    point.MakeRandomPointOnSphereLayer(data.deviation /2, data.deviation);
    data.sig.position = point;

    _log(SCAN__TRACE, "Scan::GetSignalData() - certainty for signal %s(%s) is %.5f (sigStrength:%.5f) \n Deviation: %.0f (%.3f AU)", \
            data.sig.sigName.c_str(), data.sig.sigID.c_str(), data.certainty, data.sig.sigStrength, data.deviation, (data.deviation / ONE_AU_IN_METERS));
}

/* center of tetrahedron formula
 * (((x1+x2+x3+x4) /4), ((y1+y2+y3+y4) /4), ((z1+z2+z3+z4) /4))
 */
/*
    One probe will only tell you if something is in range of the probe.
This will generate a red sphere inside your probe’s bubble that is centered on your probe.
The sphere gives you a general idea of how far the target is from your probe.
    Two probes will tell you that something exists on an imaginary ring which shows on what plane and the general area where your target is.
    Three probes will produce two possible locations for your target.
These are still not warpable and will usually show up as just one small circle on your map screen but there will be two entries under your scan results screen with the same ID#.
    Four or more probes will give you a single location shown by a red (less than 50% strength), yellow (greater than 50%), or green (100% hit).
By maneuvering your probes and decreasing their range they will eventually give a warpable result when the signal strength reaches 100% and turns green.
*/

/*
 * Probe Strengths As Function Of Angles
 *
 * Two probes at same distance to target, with angle to probes varying along a circle.
 * Measurements are relative. 1AU probe radius, approximately 0.2AU probe distance from target.
 *
 *    0°      50%
 *    10°     54%
 *    22°     57%
 *    45°     65%
 *    90°     79%
 *    135°    93%
 *    180°    96%
 *    270°    79%
 */


// these are for moon scanning
void Scan::ScanStart()
{

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
