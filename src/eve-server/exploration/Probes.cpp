 /**
  * @name ProbeItems.cpp
  *     ProbeItem SE class for Alasiya EvEmu
  *
  * @Author:        Allan
  * @date:          10 March 2018
  *
  */


#include "eve-server.h"

#include "Client.h"
#include "PyServiceMgr.h"
#include "exploration/Probes.h"
#include "exploration/Scan.h"
#include "inventory/AttributeEnum.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"

/* SCAN__ERROR
 * SCAN__WARNING
 * SCAN__MESSAGE
 * SCAN__DEBUG
 * SCAN__INFO
 * SCAN__TRACE
 * SCAN__DUMP
 * SCAN__RSPDUMP
 */

ProbeItem::ProbeItem(uint32 itemID, const ItemType& _type, const ItemData& _data)
: InventoryItem(itemID, _type, _data)
{

}
/*
            Scanner_Probe = 479,
            Survey_Probe = 492,
            Warp_Disruption_Probe = 548,
            Obsolete_Probes = 972,
            */
uint32 ProbeItem::CreateItemID(ItemData& data)
{
    return InventoryItem::CreateItemID(data);
}

ProbeItemRef ProbeItem::Load(uint32 itemID)
{
    return InventoryItem::Load<ProbeItem>(itemID );
}

ProbeItemRef ProbeItem::Spawn(ItemData& data)
{
    uint32 itemID = ProbeItem::CreateItemID(data);
    if (itemID == 0 )
        return ProbeItemRef();

    return ProbeItem::Load(itemID);
}

ProbeSE::ProbeSE(ProbeItemRef self, PyServiceMgr& services, SystemManager* system, InventoryItemRef moduleRef, ShipItemRef shipRef)
: DynamicSystemEntity(self, services, system),
m_scan(nullptr),
m_client(shipRef->GetPilot()),
m_lifeTimer(0),
m_stateTimer(0),
m_returnTimer(0)
{
    m_warID = m_client->GetWarFactionID();
    m_allyID = m_client->GetAllianceID();
    m_corpID = m_client->GetCorporationID();
    m_ownerID = m_client->GetCharacterID();

    m_state = Probe::State::Idle;
    m_shipRef = shipRef;
    m_moduleRef = moduleRef;
    m_secStatus = m_client->GetSecurityRating();

    m_expiry = GetFileTimeNow() + (Win32Time_Hour *2);  // 2h default lifespan  any way to increase this?
    m_lifeTimer.Start((Win32Time_Hour *2) *60 *60 *1000);  // convert 2h to ms

    m_scanStrength = self->GetAttribute(AttrBaseSensorStrength).get_int();
    m_scanDeviation = self->GetAttribute(AttrBaseMaxScanDeviation).get_float();

    // this isnt used like this, just placeholders to init variables
    m_rangeStep = self->GetAttribute(AttrRangeFactor).get_int();
    m_scanRange = self->GetAttribute(AttrBaseScanRange).get_float();

    // i think we may have to do probe modifiers here....they are not being done thru fx system
    Character* pChar = m_client->GetChar().get();

    // not sure about this one yet....
    if (self->groupID() == EVEDB::invGroups::Survey_Probe) {
        // 5% reduction to flight time?  10% for t2 ship
    }

    // skills
    m_scanStrength *= (1 + (0.05 * pChar->GetSkillLevel(skillAstrometrics)));             // +5% scan probe strength per level
    m_scanStrength *= (1 + (0.05 * pChar->GetSkillLevel(skillSignatureAnalysis)));        // +5% scan probe strength per level
    m_scanStrength *= (1 + (0.1 * pChar->GetSkillLevel(skillAstrometricRangefinding)));   // +10% scan probe strength per level
    m_scanDeviation *= (1 - (0.05 * pChar->GetSkillLevel(skillAstrometrics)));            // -5% scan probe deviation per level
    m_scanDeviation *= (1 - (0.1 * pChar->GetSkillLevel(skillAstrometricPinpointing)));   // -10% scan probe deviation per level
    m_scanDeviation *= (1 - (0.05 * pChar->GetSkillLevel(skillSensorLinking)));           // -5% scan probe deviation per level

    // ship
    switch (shipRef->typeID()) {
        //t1
        case 29248: { /* Magnate */
            m_scanStrength *= (1 + (0.05 * (pChar->GetSkillLevel(skillAmarrFrigate, true)))); // +5% scan probe strength per level
        } break;
        case 605: { /* Heron */
            m_scanStrength *= (1 + (0.05 * (pChar->GetSkillLevel(skillCaldariFrigate, true)))); // +5% scan probe strength per level
        } break;
        case 607: { /* Imicus */
            m_scanStrength *= (1 + (0.05 * (pChar->GetSkillLevel(skillGallenteFrigate, true)))); // +5% scan probe strength per level
        } break;
        case 586: { /* Probe */
            m_scanStrength *= (1 + (0.05 * (pChar->GetSkillLevel(skillMinmatarFrigate, true)))); // +5% scan probe strength per level
        } break;
        //t2 - Anathema, Buzzard, Cheetah, Helios
        case 11188:  /* Anathema */
        case 11192:  /* Buzzard */
        case 11172:  /* Helios */
        case 11182: { /* Cheetah */
            m_scanStrength *= (1 + (0.1 * (pChar->GetSkillLevel(skillCovertOps, true)))); // +10% scan probe strength per level
        } break;
        //t3
        // just test for subsystem here... typeIDs 30042, 30052, 30062, 30072
        // use same general code as GetRigScanBonus() in MM below...
    }

    // modules (launchers)
    if (moduleRef->HasAttribute(AttrScanStrengthBonus))
        m_scanStrength *= moduleRef->GetAttribute(AttrScanStrengthBonus).get_float();

    // rigs
    m_scanStrength *= shipRef->GetModuleManager()->GetRigScanBonus();

    // implants
    // since we dont have these, im gonna leave it off for now.

    // fudge scan strength to make my fomulas work right...wip
    m_scanStrength /= 10;

/*
 * Astrometrics (3x, 450k ISK): +5% scan strength per level, −5% max scan deviation per level, −5% scan probe scan time per level
 * Astrometric Rangefinding (8x, 450k ISK): +5% probe scan strength
 * Astrometric Pinpointing (5x, 450k ISK): −5% maximum scan deviation
 * Astrometric Acquisition (5x, 450k ISK): −5% scan time
 */

    /*
    AttrScanRange = 765,
    AttrMinScanDeviation = 787,
    AttrMaxScanDeviation = 788,
    AttrScanAnalyzeCount = 791,
    AttrScanStrengthBonus = 846,  this is on char
    AttrScanGravimetricStrengthPercent = 1027,
    AttrScanLadarStrengthPercent = 1028,
    AttrScanMagnetometricStrengthPercent = 1029,
    AttrScanRadarStrengthPercent = 1030,
    AttrScanProbeStrength = 1116,
    AttrScanStrengthSignatures = 1117,
    AttrScanStrengthDronesProbes = 1118,
    AttrScanStrengthScrap = 1119,
    AttrScanStrengthShips = 1120,
    AttrScanStrengthStructures = 1121,
    AttrMaxScanGroups = 1122,
    AttrScanDuration = 1123,     How long this probe has to scan until it can obtain results. (mt data)
    AttrScanResolutionMultiplierSet = 1135,
    AttrScanAllStrength = 1136,
    AttrMaxScanDeviationModifier = 1156,    this is on char
    AttrScanFrequencyResult = 1161,
    AttrScanGenericStrength = 1169,
    AttrProbeCanScanShips = 1413,
    AttrScanGravimetricStrengthMultiplier = 1473,
    AttrScanLadarStrengthMultiplier = 1474,
    AttrScanMagnetometricStrengthMultiplier = 1475,
    AttrScanRadarStrengthMultiplier = 1476,
    AttrSignatureRadiusBonusMultiplier = 1477,
    AttrMaxTargetRangeBonusMultiplier = 1478,
    AttrScanResolutionBonusMultiplier = 1479,

    // these 4 are implants
    AttrScanRadarStrengthModifier = 1565,
    AttrScanLadarStrengthModifier = 1566,
    AttrScanGravimetricStrengthModifier = 1567,
    AttrScanMagnetometricStrengthModifier = 1568,
    */
}

void ProbeSE::Process()
{
    SystemEntity::Process();
    // this may not work right....will need to test.
    if (m_lifeTimer.Check()) {
        Delete();
        return;
    }
    if (m_returnTimer.Check()) {
        m_returnTimer.Disable();
        _log(SCAN__DEBUG, "ProbeSE::Process() return timer hit for probeID %u", m_self->itemID());
        sBubbleMgr.Add(this);
        SendWarpEnd();
        SendRemoveProbe();
        m_scan->RemoveProbe(this);
        m_system->RemoveEntity(this);
        m_self->Move(m_client->GetShipID(), flagCargoHold, true);
        return;
    }
    if (m_stateTimer.Check()) {
        m_stateTimer.Disable();
        _log(SCAN__DEBUG, "ProbeSE::Process() state timer hit for probeID %u  state: %s", m_self->itemID(), GetStateName(m_state).c_str());
        sBubbleMgr.Add(this);
        if (m_state == Probe::State::Warping)
            SendWarpEnd();
        m_state = Probe::State::Idle;
        SendStateChange(m_state);
    }
}

void ProbeSE::Delete()
{
    m_system->RemoveEntity(this);
    m_self->Delete();
    SystemEntity::Delete();
}

bool ProbeSE::IsMoving()
{
    switch (m_state) {
        case Probe::State::Idle:
        case Probe::State::Scanning:
            return false;
        case Probe::State::Moving:
        case Probe::State::Warping:
        case Probe::State::Returning:
            return true;
    }
    return false;
}


PyDict* ProbeSE::MakeSlimItem()
{
    _log(SE__SLIMITEM, "MakeSlimItem for ProbeSE %s(%u)", GetName(), m_self->itemID());
    PyDict* slim = new PyDict();
        slim->SetItemString("itemID",           new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",           new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",          new PyInt(m_ownerID));
        slim->SetItemString("corpID",           new PyInt(m_corpID));
        slim->SetItemString("allianceID",       new PyInt(m_allyID));
        slim->SetItemString("warFactionID",     new PyInt(m_warID));
        slim->SetItemString("numLaunchers",     new PyInt(1));
        slim->SetItemString("sourceModuleID",   new PyInt(m_moduleRef->itemID()));
        slim->SetItemString("securityStatus",   new PyFloat(m_secStatus));
    return slim;
}

void ProbeSE::MakeDamageState(DoDestinyDamageState &into) {
    into.shield = 1.0;
    into.recharge = 1000000;
    into.timestamp = Win32TimeNow();
    into.armor = 1.0;
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_double() / m_self->GetAttribute(AttrHP).get_double());
}

void ProbeSE::UpdateProbe(ProbeData& data)
{
    m_scanRange = data.scanRange;
    m_rangeStep = data.rangeStep;
    m_destination = data.dest;

    float time = 1, dist = GetPosition().distance(m_destination);
    if (dist > BUBBLE_RADIUS_METERS){
        m_state = Probe::State::Warping;
        time += floor(dist / (m_self->GetAttribute(AttrWarpSpeedMultiplier).get_float() *ONE_AU_IN_METERS));
        SendWarpStart(time);
    } else if (dist > 2500) {
        m_state = Probe::State::Moving;
        time += floor(dist / m_self->GetAttribute(AttrMaxVelocity).get_float());
    }

    SendStateChange(m_state);
    m_bubble->Remove(this);
    m_stateTimer.Start(time *1000);

    _log(SCAN__DEBUG, "ProbeSE::UpdateProbe()  id:%u, state: %s, scanRange: %.2f, step: %u, dist:%.2f, time: %.3f", \
                GetID(), GetStateName(m_state).c_str(), m_scanRange, m_rangeStep, dist, time );
}

void ProbeSE::RecoverProbe(PyList* list)
{
    m_destination = m_client->GetShipSE()->GetPosition() + MakeRandomInt(1000, 2500);
    float time = 1, dist = GetPosition().distance(m_destination);
    if (dist > BUBBLE_RADIUS_METERS){
        //m_state = Probe::State::Warping;
        time += floor(dist / (m_self->GetAttribute(AttrWarpSpeedMultiplier).get_float() *ONE_AU_IN_METERS));
        //SendWarpStart(time);
        m_bubble->Remove(this);
    } else if (dist > 2500) {
        //m_state = Probe::State::Moving;
        time += floor(dist / m_self->GetAttribute(AttrMaxVelocity).get_float());
    }

    // add to list if still controlled by player
    list->AddItem(new PyInt(m_self->itemID()));
    m_returnTimer.Start(time *1000);
    SendStateChange(Probe::State::Returning);
}

void ProbeSE::SendNewProbe()
{
    SSR_ObjectEx_Pos ssr_oed;
        ssr_oed.x = GetPosition().x;
        ssr_oed.y = GetPosition().y;
        ssr_oed.z = GetPosition().z;
    PyToken* token = new PyToken("foo.Vector3");
    PyTuple* oed_tuple = new PyTuple(2);
        oed_tuple->SetItem(0, token);
        oed_tuple->SetItem(1, ssr_oed.Encode());
    PyDict* newProbe = new PyDict();
        newProbe->SetItemString("probeID",      new PyLong(m_self->itemID()));
        newProbe->SetItemString("typeID",       new PyInt(m_self->typeID()));
        newProbe->SetItemString("scanRange",    new PyFloat(m_scanRange));
        newProbe->SetItemString("expiry",       new PyLong(m_expiry));
        newProbe->SetItemString("pos",          new PyObjectEx(false, oed_tuple));
    PyTuple* ev = new PyTuple(1);
        ev->SetItem(0, new PyObject("util.KeyVal", newProbe));
    m_client->SendNotification("OnNewProbe", "clientID", &ev);  // this is sequenced
}

void ProbeSE::SendStateChange(uint8 state)
{
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyLong(m_self->itemID()));
        tuple->SetItem(1, new PyInt(state));
    m_client->SendNotification("OnProbeStateChanged", "clientID", &tuple, false);  // this is not sequenced
}

void ProbeSE::SendRemoveProbe()
{
    m_destiny->SetPosition(NULL_ORIGIN);
    PyTuple* ev = new PyTuple(1);
        ev->SetItem(0, new PyLong(m_self->itemID()));
    m_client->SendNotification("OnRemoveProbe", "clientID", &ev);  // this is sequenced
}

void ProbeSE::SendWarpStart(float travelTime/*0*/)
{
    // OnProbeWarpStart(self, probeID, fromPos, toPos, startTime, duration)
    PyToken* token = new PyToken("foo.Vector3");
    SSR_ObjectEx_Pos posFrom;
        posFrom.x = m_self->position().x;
        posFrom.y = m_self->position().y;
        posFrom.z = m_self->position().z;
    SSR_ObjectEx_Pos posTo;
        posTo.x = m_destination.x;
        posTo.y = m_destination.y;
        posTo.z = m_destination.z;
    PyTuple* from = new PyTuple(2);
        from->SetItem(0, token);
        from->SetItem(1, posFrom.Encode());
    PyTuple* to = new PyTuple(2);
        to->SetItem(0, token);
        to->SetItem(1, posTo.Encode());
    PyTuple* tuple = new PyTuple(5);
        tuple->SetItem(0, new PyLong(m_self->itemID()));    //probeID
        tuple->SetItem(1, new PyObjectEx(false, from));     //from
        tuple->SetItem(2, new PyObjectEx(false, to));       //to
        tuple->SetItem(3, new PyLong(GetFileTimeNow()));    //startTime
        tuple->SetItem(4, new PyFloat(travelTime));         //duration in ms
    m_client->SendNotification("OnProbeWarpStart", "clientID", &tuple, false);  // this is not sequenced
}

void ProbeSE::SendWarpEnd()
{
    m_destiny->SetPosition(m_destination);
    PyTuple* tuple = new PyTuple(1);
        tuple->SetItem(0, new PyLong(m_self->itemID()));
    m_client->SendNotification("OnProbeWarpEnd", "clientID", &tuple, false);  // this is not sequenced
}

void ProbeSE::SendSlimChange()
{
    PyDict* slim = new PyDict();
        slim->SetItemString("itemID",           new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",           new PyInt(m_self->typeID()));
        slim->SetItemString("categoryID",       new PyInt(m_self->categoryID()));
        slim->SetItemString("ownerID",          new PyInt(m_ownerID));
        slim->SetItemString("corpID",           new PyInt(m_corpID));
        slim->SetItemString("allianceID",       new PyInt(m_allyID));
        slim->SetItemString("warFactionID",     new PyInt(m_warID));
        slim->SetItemString("numLaunchers",     new PyInt(1));
        slim->SetItemString("sourceModuleID",   new PyInt(m_moduleRef->itemID()));
        slim->SetItemString("securityStatus",   new PyFloat(m_secStatus));
        slim->SetItemString("warpingAway",      PyStatic.NewTrue());    // this is sent when probe warps
    PyTuple* probeData = new PyTuple(2);
        probeData->SetItem(0, new PyLong(m_self->itemID()));
        probeData->SetItem(1, new PyObject("foo.SlimItem", slim));
    PyTuple* updates = new PyTuple(2);
        updates->SetItem(0, new PyString("OnSlimItemChange"));
        updates->SetItem(1, probeData);
    m_destiny->SendSingleDestinyUpdate(&updates, true);
}

std::string ProbeSE::GetStateName(uint8 state)
{
    switch(state) {
        case Probe::State::Inactive:   return "Inactive";
        case Probe::State::Idle:       return "Idle";
        case Probe::State::Moving:     return "Moving";
        case Probe::State::Warping:    return "Warping";
        case Probe::State::Scanning:   return "Scanning";
        case Probe::State::Returning:  return "Returning";
    }
}

float ProbeSE::GetDeviation()
{
    //Max Deviation = (Scan Range/Base Scan Range) × Base Maximum Deviation × (1 - Pinpointing Skill/10)
    float maxDeviation = m_scanRange /m_self->GetAttribute(AttrBaseScanRange).get_float();
    maxDeviation *= m_scanDeviation;
    maxDeviation *= (1 - (m_client->GetChar()->GetSkillLevel(skillAstrometricPinpointing) /10));
    return maxDeviation;
}

/*
 *  to calculate the maximum possible deviation you use the constants provided for the type of probe,
 * the scan size your probes are set to, and your skill level of Astrometric Pinpointing.
 * Here is the formula:
 * Max Deviation = (Scan Range/Base Scan Range) × Base Maximum Deviation × (1 - Pinpointing Skill/10)
 *
 * Maximum deviation in AU at different ranges and levels
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
 */

float ProbeSE::GetRangeModifier(float dist)
{
    switch(m_rangeStep) {
        case 8:  return (dist / m_scanRange) * 0.125;
        case 7:  return (dist / m_scanRange) * 0.25;
        case 6:  return (dist / m_scanRange) * 0.375;
        case 5:  return (dist / m_scanRange) * 0.50;
        case 4:  return (dist / m_scanRange) * 0.625;
        case 3:  return (dist / m_scanRange) * 0.75;
        case 2:  return (dist / m_scanRange) * 0.875;
        case 1:  return (dist / m_scanRange);
    }
}

float ProbeSE::GetScanStrength()
{
    switch(m_rangeStep) {
        case 8:  return m_scanStrength * 0.125;
        case 7:  return m_scanStrength * 0.25;
        case 6:  return m_scanStrength * 0.375;
        case 5:  return m_scanStrength * 0.50;
        case 4:  return m_scanStrength * 0.625;
        case 3:  return m_scanStrength * 0.75;
        case 2:  return m_scanStrength * 0.875;
        case 1:  return m_scanStrength;
    }
}
