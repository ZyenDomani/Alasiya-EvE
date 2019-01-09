
 /**
  * @name ActiveModule.cpp
  *   active module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */


#include "eve-server.h"

#include "StatisticMgr.h"
#include "ship/Missile.h"
#include "ship/modules/ActiveModule.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/BeltMgr.h"
#include <exploration/Probes.h>
#include <exploration/Scan.h>

ActiveModule::ActiveModule(InventoryItemRef iRef, ShipItemRef sRef)
: GenericModule(iRef, sRef),
m_timer(0, true),
m_reloadTimer(0),
m_Stop(true),
m_targetID(0),
m_effectID(0),
m_guidStr(""),
m_bubble(nullptr),
m_targMgr(nullptr),
m_targetSE(nullptr),
m_destinyMgr(nullptr),
m_needsTarget(false)
{
    m_repeat = 1000;    //arbitrary.

    m_needsCharge = iRef->HasAttribute(AttrChargeGroup1);
    if (m_needsCharge) {
        switch (iRef->groupID()) {
            // these neither require nor consume charges
            case EVEDB::invGroups::Remote_Sensor_Damper:
            case EVEDB::invGroups::Tracking_Link:
            case EVEDB::invGroups::Signal_Amplifier:
            case EVEDB::invGroups::Tracking_Enhancer:
            case EVEDB::invGroups::Sensor_Booster:
            case EVEDB::invGroups::Tracking_Computer:
            case EVEDB::invGroups::Projected_ECCM:
            case EVEDB::invGroups::Remote_Sensor_Booster:
            case EVEDB::invGroups::Tracking_Disruptor:
            // mining laser can be used without charge, using default extraction rate
            case EVEDB::invGroups::Frequency_Mining_Laser: {
                m_needsCharge = false;
            } break;
        }
    } else {
        switch (iRef->groupID()) {
            case EVEDB::invGroups::Survey_Scanner:
            case EVEDB::invGroups::Ship_Scanner:
            case EVEDB::invGroups::Cargo_Scanner:
            case EVEDB::invGroups::System_Scanner: {
                float m_range = GetAttribute(AttrSurveyScanRange).get_float();
                m_range *= (1 + (0.03 * (m_shipRef->GetPilot()->GetChar()->GetSkillLevel(skillLongRangeTargeting, true)))); // 3% increase in range (here)
                SetAttribute(AttrSurveyScanRange, m_range);
            } break;
        }
    }

    // this is an internal variable only.
    m_reloadTime = GetAttribute(AttrReloadTime).get_int();
    // set default of 4s for turrets, 5s for snowball and probe launchers, 7s for missile launchers, and 10s for others.
    if (m_needsCharge)  {
        if (m_reloadTime < 1) {
            switch (iRef->groupID()) {
                case EVEDB::invGroups::Projectile_Weapon: {
                    m_reloadTime = 4000;
                } break;
                case EVEDB::invGroups::Missile_Launcher_Snowball:
                case EVEDB::invGroups::Scan_Probe_Launcher: {
                    m_reloadTime = 5000;
                } break;
                case EVEDB::invGroups::Missile_Launcher_Cruise:
                case EVEDB::invGroups::Missile_Launcher_Rocket:
                case EVEDB::invGroups::Missile_Launcher_Siege:
                case EVEDB::invGroups::Missile_Launcher_Standard:
                case EVEDB::invGroups::Missile_Launcher_Heavy:
                case EVEDB::invGroups::Missile_Launcher_Assault:
                case EVEDB::invGroups::Missile_Launcher_Defender:
                case EVEDB::invGroups::Missile_Launcher_Citadel:
                case EVEDB::invGroups::Missile_Launcher_Heavy_Assault:
                case EVEDB::invGroups::Missile_Launcher_Bomb: {
                    m_reloadTime = 7000;
                } break;
                default: {
                    m_reloadTime = 10000;
                } break;
            }
        }
    }

    Clear();

    if (m_reloadTime > 0)
        _log(SHIP__MODULE_TRACE, "Reload time for %s(%u) set to %ums", iRef->itemName().c_str(), iRef->itemID(), m_reloadTime);
}

void ActiveModule::Clear()
{
    m_Stop = true;
    m_repeat = 1000;
    m_targetID = 0;
    m_effectID = 0;
    m_guidStr = "";
    m_bubble = nullptr;
    m_targMgr = nullptr;
    m_targetSE = nullptr;
    m_destinyMgr = nullptr;
    m_needsTarget = false;

    m_timer.Disable();

    m_shipRef->ClearTargetRef();

    SetModuleState(Module::State::Online);
}

void ActiveModule::Process()
{
    // the order of Reload/Unload is significant.
    if (m_reloadTimer.Enabled()) {
        if (m_reloadTimer.Check(false)) {
            // charge loading complete
            m_reloadTimer.Disable();
            SetChargeState(Module::State::Loaded);
            // apply charge effects here after "loading" is complete
            sFxProc.ApplyEffects(m_chargeRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), true);
        }
    }
    if (m_ModuleState == Module::State::Online)
        return;

    if (m_timer.Check())
        ProcessActiveCycle();

    if (m_needsCharge) {
        // is this right?  should i do something else here?
        if ((m_chargeRef.get() == nullptr) or (m_ChargeState == Module::State::Unloaded)
        or (m_chargeRef->quantity() < 1) or (!m_chargeLoaded)) {
            UnloadCharge();
            SetModuleState(Module::State::Deactivating);
            DeactivateCycle(true);
        }
    }
}

void ActiveModule::Activate(uint16 effectID, uint32 targetID/*0*/, int16 repeat/*0*/)
{
    if (effectID == 16) {   // catchall for elusive online/offline error
        Online();
        return;
    }
    if ((m_needsCharge) and ((!m_chargeLoaded) or (m_chargeRef.get() == nullptr))) {
        Clear();
        throw PyException( MakeUserError( "CantFindChargeToAdd"));
    }
    if (IsValidTarget(targetID)) {
        m_needsTarget = true;       // this is just a guess.  may have to use groupID test to verify if this doesnt work right.
        m_targetID = targetID;
        m_targetSE = m_shipRef->GetPilot()->SystemMgr()->GetSE(targetID);
        if (m_targetSE == nullptr) {
            Clear();
            throw PyException( MakeUserError( "DeniedActivateTargetNotPresent"));
        }
        if (m_targetSE->TargetMgr() != nullptr)
            m_targetSE->TargetMgr()->AddTargetModule(this);
    }

    /*
     * AttrdisallowAgainstEwImmuneTarget
     * AttrDisallowOffensiveModifiers
     * AttrDisallowOffensiveModifierBonus
     */

    if (m_targetSE != nullptr)
        if (m_targetSE->GetSelf()->HasAttribute(AttrDisallowAssistance)) {
            Clear();
            throw PyException( MakeUserError( "DeniedActivateTargetAssistDisallowed"));
        }

    m_Stop = false;
    m_repeat = repeat;
    m_effectID = effectID;
    m_isWarpSafe = sFxDataMgr.isWarpSafe(effectID);
    m_guidStr = sFxDataMgr.GetEffectGuid(effectID);
    m_bubble = m_shipRef->GetPilot()->GetShipSE()->SysBubble();
    m_targMgr = m_shipRef->GetPilot()->GetShipSE()->TargetMgr();
    m_destinyMgr = m_shipRef->GetPilot()->GetShipSE()->DestinyMgr();

    if (!CanActivate()) {
        Clear();
        return;
    }

    // Do initial cycle immediately while we start timer
    SetTimer(DoCycle());

    if (!m_timer.Enabled()) {
        // if the timer wasnt set (for whatever reason), kill activation and return
        Clear();
        return;
    }

    ApplyEffect(Effects::dgmStateActive, true);
    if (IsValidTarget(targetID))
        ApplyEffect(Effects::dgmStateTarget, true);

    ShowEffect(true, false);

    SetModuleState(Module::State::Activated);

    switch (groupID()) {
        case EVEDB::invGroups::Afterburner:
        case EVEDB::invGroups::Microwarpdrive: {
            m_destinyMgr->SpeedBoost();
        } break;
        case EVEDB::invGroups::Tractor_Beam: {
            m_targetSE->DestinyMgr()->TractorBeamStart(m_shipRef->GetPilot()->GetShipSE());
        } break;
        case EVEDB::invGroups::Stasis_Web: {
            m_targetSE->DestinyMgr()->WebbedMe(m_modRef, true);
        } break;
    }

    if (m_repeat < 1)
        m_Stop = true;
}

void ActiveModule::Deactivate(std::string effect/*""*/)
{
    if (m_ModuleState != Module::State::Activated)
        return;

    _log(SHIP__MODULE_TRACE, "ActiveModule::Deactivate(%s) - module %s(%u) remaining time %ums.", \
            effect.c_str(), m_modRef->itemName().c_str(), m_modRef->itemID(), GetRemainingCycleTimeMS());

    if ((m_effectID == EVEEffectID::miningLaser) or (m_effectID == EVEEffectID::miningClouds)) {
        AbortCycle();
        return;
    } else if (effect.compare("TargetDestroyed") == 0) {
        // this is sent back in OnGodmaShipEffect packet
        m_targetSE = nullptr;
    }
    if (m_targetSE != nullptr)
        if (m_targetSE->TargetMgr() != nullptr)
            m_targetSE->TargetMgr()->RemoveTargetModule(this);

    m_Stop = true;
    SetModuleState(Module::State::Deactivating);
}

void ActiveModule::Overload()
{
    m_overLoaded = true;
    GenericModule::Overload();
}

void ActiveModule::DeOverload()
{
    GenericModule::DeOverload();
    m_overLoaded = false;
}

// yes, the xxCycle() shit below seems overkill, but each has a specific purpose
uint32 ActiveModule::DoCycle()
{
    if (m_destinyMgr == nullptr) {
        // make error for no destiny/bubble
        AbortCycle();
        return 0;
    }
    if (m_needsTarget) {
        if (m_targetSE == nullptr) {
            AbortCycle();
            return 0;
        }
        if (m_targetSE->GetID() != m_targetID) {
            AbortCycle();
            return 0;
        }
    }
    if (m_needsCharge) {
        // modules that use scripts arent considered as needsCharge, as they work with or without the script.
        if ((!m_chargeLoaded) or (m_chargeRef.get() == nullptr)) {
            //{'FullPath': u'UI/Messages', 'messageID': 259200, 'label': u'NoChargesBody'}(u'{launcher} has run out of charges', None, {u'{launcher}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'launcher'}})
            //{'FullPath': u'UI/Messages', 'messageID': 259232, 'label': u'NotEnoughChargesBody'}(u'{launcher} has {[numeric]got} charges, but needs {[numeric]need} to fire.', None, {u'{[numeric]got}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'got'}, u'{launcher}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'launcher'}, u'{[numeric]need}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'need'}})
            //{'FullPath': u'UI/Messages', 'messageID': 258889, 'label': u'TooManyChargesForLauncherBody'}(u'The launcher is currently holding {[numeric]excess} too many excess units.', None, {u'{[numeric]excess}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'excess'}})
            // can this throw?
            m_shipRef->GetPilot()->SendErrorMsg("Your %s has no loaded charge.  Deactivating.", m_modRef->itemName().c_str());
            AbortCycle();
            return 0;
        }
    }

    // not sure if this is entirely accurate...wip
    switch (m_modRef->groupID()) {
        case EVEDB::invGroups::Artifacts_and_Prototypes: { // (this module group will need specific code)
        } break;
        case EVEDB::invGroups::Passive_Targeting_System: { // (this passive module will need specific code)
        } break;
        case EVEDB::invGroups::Automated_Targeting_System: { // (this active module will need specific code)
        } break;

        // these im not sure about yet
        case EVEDB::invGroups::ECM:
        case EVEDB::invGroups::ECCM:
        case EVEDB::invGroups::Gang_Coordinator:
        case EVEDB::invGroups::Cloaking_Device:
        case EVEDB::invGroups::Siege_Module:
        case EVEDB::invGroups::Super_Weapon:
        case EVEDB::invGroups::Interdiction_Sphere_Launcher:    // launch a sphere (like missile and probe)
        case EVEDB::invGroups::Countermeasure_Launcher:     // dunno
        case EVEDB::invGroups::Jump_Portal_Generator:
        case EVEDB::invGroups::Cynosural_Field:
        case EVEDB::invGroups::Remote_ECM_Burst:
        case EVEDB::invGroups::Warp_Disrupt_Field_Generator:
        case EVEDB::invGroups::Covert_Cynosural_Field_Generator:
        case EVEDB::invGroups::Smart_Bomb:
        case EVEDB::invGroups::ECM_Burst: {
        } break;

        case EVEDB::invGroups::Capacitor_Booster:{
            UpdateCharge(AttrCapacitorCharge, AttrCapacitorCapacity, AttrPowerTransferAmount, m_shipRef);
        } break;
        // i *think* these first 2 go here....need testing
        case EVEDB::invGroups::Energy_Vampire:
        case EVEDB::invGroups::Energy_Destabilizer:
        case EVEDB::invGroups::Energy_Transfer_Array: {
            UpdateCharge(AttrCapacitorCharge, AttrCapacitorCapacity, AttrPowerTransferAmount, m_targetSE->GetSelf());
        } break;
        case EVEDB::invGroups::Shield_Transporter: {
            UpdateCharge(AttrShieldCharge, AttrShieldCapacity, AttrShieldBonus, m_targetSE->GetSelf());
        } break;
        case EVEDB::invGroups::Shield_Booster: {
            UpdateCharge(AttrShieldCharge, AttrShieldCapacity, AttrShieldBonus, m_shipRef);
        } break;
        case EVEDB::invGroups::Remote_Hull_Repairer: {
            UpdateDamage(AttrDamage, AttrStructureDamageAmount, m_targetSE->GetSelf());
        } break;
        case EVEDB::invGroups::Hull_Repair_Unit: {
            UpdateDamage(AttrDamage, AttrStructureDamageAmount, m_shipRef);
        } break;
        case EVEDB::invGroups::Armor_Repair_Projector: {
            UpdateDamage(AttrArmorDamage, AttrArmorDamageAmount, m_targetSE->GetSelf());
        } break;
        case EVEDB::invGroups::Armor_Repair_Unit: {
            UpdateDamage(AttrArmorDamage, AttrArmorDamageAmount, m_shipRef);
        } break;
        case EVEDB::invGroups::Projectile_Weapon:
        case EVEDB::invGroups::Hybrid_Weapon:
        case EVEDB::invGroups::Energy_Weapon: {
            // turret weapons still use specific code.
            ApplyDamage();
        } break;
        case EVEDB::invGroups::Missile_Launcher_Assault:
        case EVEDB::invGroups::Missile_Launcher_Bomb:   // not sure here
        case EVEDB::invGroups::Missile_Launcher_Citadel:
        case EVEDB::invGroups::Missile_Launcher_Cruise:
        case EVEDB::invGroups::Missile_Launcher_Defender:   // not sure here
        case EVEDB::invGroups::Missile_Launcher_Heavy:
        case EVEDB::invGroups::Missile_Launcher_Heavy_Assault:
        case EVEDB::invGroups::Missile_Launcher_Rocket:
        case EVEDB::invGroups::Missile_Launcher_Siege:
        case EVEDB::invGroups::Missile_Launcher_Standard: {
            LaunchMissile();
        } break;
        case EVEDB::invGroups::Missile_Launcher_Snowball: {
            LaunchSnowBall();
        } break;
        case EVEDB::invGroups::Scan_Probe_Launcher: {
            /** @todo  test for active probes vs skills here */
            LaunchProbe();
        } break;
        // these neither require nor consume charges
        case EVEDB::invGroups::Salvager:    //working
        case EVEDB::invGroups::Target_Painter:  // working
        case EVEDB::invGroups::Signal_Amplifier:    //working
        case EVEDB::invGroups::Sensor_Booster:  //working
        case EVEDB::invGroups::Tracking_Computer:   //working
        case EVEDB::invGroups::Tracking_Disruptor:    //working
        case EVEDB::invGroups::Remote_Sensor_Damper:
        case EVEDB::invGroups::Tracking_Link:
        case EVEDB::invGroups::Tracking_Enhancer:
        case EVEDB::invGroups::Projected_ECCM:
        case EVEDB::invGroups::Remote_Sensor_Booster: {
        } break;
    }

    // do heat damage if overloaded...this will be handled in shipItem class
    if (m_overLoaded)
        m_shipRef->HeatDamageCheck(this);

    EvilNumber cycleTime = 10000;   // default to 10s
    if (m_modRef->HasAttribute(AttrSpeed, cycleTime))
        ; //return cycleTime.get_int();
    else if (m_modRef->HasAttribute(AttrDuration, cycleTime))
        ; //return cycleTime.get_int();
    return cycleTime.get_int();
}

void ActiveModule::AbortCycle()
{
    if (m_Stop)
        return;
    // Immediately stop active cycle for things such as insufficient cap, remove module, init warp, target destoryed, target left bubble, or miner deactivated by player:
    m_Stop = true;
    SetModuleState(Module::State::Deactivating);
    DeactivateCycle(true);
    m_timer.Disable();
}

void ActiveModule::DeactivateCycle(bool abort/*false*/)
{
    if ((m_ModuleState != Module::State::Deactivating) and (!abort)) {
        _log(SHIP__MODULE_ERROR, "ActiveModule::DeactivateCycle() - Called on %s(%u) with current state %s and !abort.",  \
                m_modRef->itemName().c_str(), m_modRef->itemID(), GetModuleStateName(m_ModuleState).c_str());
        return;
    }

    ApplyEffect(Effects::dgmStateActive, false);
    if (IsValidTarget(m_targetID))
        ApplyEffect(Effects::dgmStateTarget, false);

    ShowEffect(false, abort);

    switch (groupID()) {
        case EVEDB::invGroups::Tractor_Beam: {
            if (m_targetSE != nullptr)
                m_targetSE->DestinyMgr()->TractorBeamStop();
        } break;
        case EVEDB::invGroups::Afterburner:
        case EVEDB::invGroups::Microwarpdrive: {
            m_destinyMgr->SpeedBoost(true);
        } break;
        case EVEDB::invGroups::Stasis_Web: {
            if (m_targetSE != nullptr)
                m_targetSE->DestinyMgr()->WebbedMe(m_modRef, false);
        } break;
        case EVEDB::invGroups::Survey_Scanner: {
            // this is the complete belt scanner code here.
            PyTuple* result = new PyTuple(2);
            result->SetItem(0, new PyString("OnSurveyScanComplete"));
            PyList* list = new PyList();
            result->SetItem(1, list);
            if (m_bubble->IsBelt()) {
                float m_range = GetAttribute(AttrSurveyScanRange).get_float();
                std::vector<AsteroidSE*> vList;
                m_shipRef->GetPilot()->GetShipSE()->SystemMgr()->GetBeltMgr()->GetList(sBubbleMgr.GetBeltID(m_bubble->GetID()), vList);
                for (auto pASE : vList) {
                    // allow ice scanning without a radius check....may change later.
                    if (m_bubble->IsIce() or (m_shipRef->position().distance(pASE->GetPosition()) < m_range)) {
                        PyTuple* tuple2 = new PyTuple(3);
                        tuple2->SetItem(0, new PyInt(pASE->GetID()));
                        tuple2->SetItem(1, new PyInt(pASE->GetTypeID()));
                        tuple2->SetItem(2, new PyInt(pASE->GetSelf()->GetAttribute(AttrQuantity).get_int()));
                        list->AddItem(tuple2);
                    }
                }
            }
            // Send results.
            std::vector<PyTuple*> events;
            events.push_back(result);
            std::vector<PyTuple*> updates;
            m_destinyMgr->SendDestinyUpdate(updates, events, false);
        } break;
        case EVEDB::invGroups::Ship_Scanner:
        case EVEDB::invGroups::Cargo_Scanner:
        case EVEDB::invGroups::System_Scanner: {
            if (m_targetSE != nullptr)
                ;  // not sure if we need this here.....
        } break;
    }

    Clear();
}

void ActiveModule::ProcessActiveCycle() {
    if (m_Stop)
        SetModuleState(Module::State::Deactivating);

    if (m_ModuleState == Module::State::Deactivating) {
        DeactivateCycle();
        return;
    }

    float newCap = (m_shipRef->GetAttribute(AttrCapacitorCharge).get_float() - GetAttribute(AttrCapacitorNeed)).get_float();
    if (newCap >= 0 ) {
        m_shipRef->SetAttribute(AttrCapacitorCharge, newCap);
        SetTimer(DoCycle());
    } else
        AbortCycle();
}

void ActiveModule::SetTimer(uint32 time) {
    if (time < 100) {
        m_Stop = true;
        return;
    }
    // timer must be restarted for fleet boosts to activate
    if (m_timer.Enabled())
        return;
    // updated timer will reset cycle time if changed, but i DO NOT have client display coded to reset...this will fuck up timer time in client.
    _log(SHIP__MODULE_TRACE, "ActiveModule::SetTimer() - %s with %u ms.", (m_timer.Enabled()? "Updated" : "Started"), time);
    m_timer.Start(time);
}

void ActiveModule::LoadCharge(InventoryItemRef chargeRef)
{
    if (chargeRef.get() == nullptr) {
        _log(SHIP__MODULE_WARNING, "ActiveModule::LoadCharge() - Cannot find charge to load into this module");
        return;
    }

    m_chargeRef = chargeRef;
    m_chargeLoaded = true;
    SetChargeState(Module::State::Loading);

    /*
     * def OnChargeBeingLoadedToModule(self, moduleIDs, chargeTypeID, reloadTime):
     *  {returns}
     *        [PyTuple 3 items]
     *          [PyTuple 1 items]
     *            [PyIntegerVar 1005885547063]  << moduleID
     *          [PyInt 203]                     << chargeTypeID
     *          [PyFloat 10000]                 << reloadTime (ms)
     */
    Client* pClient = m_shipRef->GetPilot();
    if (pClient == nullptr) {
        m_chargeRef = InventoryItemRef(nullptr);
        m_chargeLoaded = false;
        SetChargeState(Module::State::Unloaded);
        return;  // make error here?
    }
    if (pClient->IsInSpace() and !pClient->IsLogin()) {
        PyTuple* module = new PyTuple(1);
            module->SetItem(0, new PyInt(m_modRef->itemID()));
        PyTuple* tmp = new PyTuple(3);
            tmp->SetItem(0, module);
            tmp->SetItem(1, new PyInt(m_chargeRef->typeID()));
            tmp->SetItem(2, new PyInt(m_reloadTime));
        pClient->SendNotification("OnChargeBeingLoadedToModule", "shipid", &tmp, false); //unsequenced.
        m_reloadTimer.Start(m_reloadTime);
    }
    // process new charge's effects here
    m_chargeRef->ClearModifiers();
    fxData data {};
    data.action = Effects::Action::dgmActInvalid;
    data.srcRef = m_chargeRef;
    for (auto it : m_chargeRef->type().m_stateFxMap) {
        data.math = data.targLoc = data.targAttr = data.srcAttr = data.grpID = data.typeID = data.fxSrc = 0;
        sFxProc.ParseExpression(m_chargeRef.get(), sFxDataMgr.GetExpression(it.second.preExpression), data, this);
    }
    if (pClient->IsLogin() or pClient->IsDocked()) {
        SetChargeState(Module::State::Loaded);
        sFxProc.ApplyEffects(m_chargeRef.get(), pClient->GetChar().get(), m_shipRef.get(), pClient->IsInSpace());
    }
}

void ActiveModule::UnloadCharge()
{
    if (m_chargeRef.get() != nullptr) {
        // remove charge effects here
        m_chargeRef->ClearModifiers();
        for (auto it : m_chargeRef->type().m_stateFxMap) {
            fxData data;
            data.action = Effects::Action::dgmActInvalid;
            data.srcRef = m_chargeRef;
            data.math = data.targLoc = data.targAttr = data.srcAttr = data.grpID = data.typeID = data.fxSrc = 0;
            sFxProc.ParseExpression(m_chargeRef.get(), sFxDataMgr.GetExpression(it.second.postExpression), data, this);
        }
        /** @todo  this isnt right.  need to remove EXISTING modifier data.....NOT this new data.
         *    also, DONT reset modifiermap before remoing, to use existing, modified data
         *  NOTE:  i've no clue how to do that yet....
         */
        Client* pClient = m_shipRef->GetPilot();
        sFxProc.ApplyEffects(m_chargeRef.get(), pClient->GetChar().get(), m_shipRef.get(), pClient->IsInSpace());
    }

    m_chargeRef = InventoryItemRef(nullptr);       // Ensure ref is NULL
    m_chargeLoaded = false;
    SetChargeState(Module::State::Unloaded);
}

void ActiveModule::ApplyEffect(Effects::State state, bool active/*false*/)
{
    // process and apply module's active effects
    m_modRef->m_modifiers.clear();
    ProcessEffects(state, active);
    sFxProc.ApplyEffects(m_modRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), true);
}

void ActiveModule::UpdateCharge(uint16 attrID, uint16 testAttrID, uint16 srcAttrID, InventoryItemRef iRef)
{
    // Apply boost amount:
    EvilNumber newValue = iRef->GetAttribute(attrID);
    newValue += GetAttribute(srcAttrID);
    if (newValue > iRef->GetAttribute(testAttrID)) {
        newValue = iRef->GetAttribute(testAttrID);
        Deactivate();
    }
    iRef->SetAttribute(attrID, newValue);
}

void ActiveModule::UpdateDamage(uint16 attrID, uint16 srcAttrID, InventoryItemRef iRef)
{
    EvilNumber newValue = iRef->GetAttribute(attrID);
    newValue -= GetAttribute(srcAttrID);
    if (newValue < 0) {
        newValue = 0;
        Deactivate();
    }
    iRef->SetAttribute(attrID, newValue);
}

void ActiveModule::ReprocessCharge()
{
    if (m_chargeRef.get() == nullptr)
        return;
    /*  may not need to reset this...
    m_chargeRef->ClearModifiers();
    fxData data;
    data.action = Effects::Action::dgmActInvalid;
    for (auto it : m_chargeRef->type().m_stateFxMap) {
        data.srcRef = m_chargeRef;
        data.math = data.targLoc = data.targAttr = data.srcAttr = data.grpID = data.typeID = data.fxSrc = 0;
        sFxProc.ParseExpression(m_chargeRef.get(), sFxDataMgr.GetExpression(it.second.preExpression), data, this);
    } */
    sFxProc.ApplyEffects(m_chargeRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), m_shipRef->GetPilot()->IsInSpace());
}

bool ActiveModule::CanActivate()
{
    // there is still more to be done here.  wip
    //  specific modules that require specific tests are coded in their module class, which will call this if checks pass

    // check distance for targetable actions
    if (m_targetSE != nullptr) {
        if (!m_turret and !m_launcher) {
            float range = GetAttribute(AttrMaxRange).get_float();
            float distance = m_shipRef->position().distance(m_targetSE->GetPosition());
            if (distance > range) {
                m_shipRef->GetPilot()->SendNotifyMsg("The %s is %.0f meters from you, outside the effective range of your %s, which is %.0f meters.", \
                        m_targetSE->GetName(), distance, m_modRef->itemName().c_str(), range);
                return false;
            }
        }
        if (m_shipRef->groupID() == EVEDB::invGroups::Tractor_Beam)
            if (m_targetSE->IsContainerSE() or m_targetSE->IsWreckSE()) {
                m_shipRef->GetPilot()->SendNotifyMsg("You cannot tractor %s", m_targetSE->GetName());
                return false;
            }

        // if target is non-combatant deny attack
        if (sFxDataMgr.isOffensive(m_effectID)
            and ((m_targetSE->IsItemEntity())
                or (m_targetSE->IsStaticEntity())
                or (m_targetSE->IsAsteroidSE())
                or (m_targetSE->IsLogin())))
        {
            m_shipRef->GetPilot()->SendNotifyMsg("You cannot attack the %s", m_targetSE->GetName());
            return false;
        }
    }
    //AttrDeadspaceUnsafe
    return true;
}


void ActiveModule::ShowEffect(bool active/*false*/, bool abort/*false*/)
{
    int64 abortTime = GetFileTimeNow();
    if (abort) {
        active = false;
        if ((m_effectID == EVEEffectID::miningLaser) or (m_effectID == EVEEffectID::miningClouds))
            abortTime += (3 * EvE::Time::Second);    // delay mining abort for 3s to simulate module "completing" its' cycle and dumping ore to cargo
    }

    uint16 chgTypeID = (m_chargeLoaded ? m_chargeRef->typeID() : 0);
    uint32 timeLeft = GetRemainingCycleTimeMS();
    EvilNumber cycleTime = 0;
    if (m_modRef->HasAttribute(AttrDuration, cycleTime))
        ;
    else if (m_modRef->HasAttribute(AttrSpeed, cycleTime))
        ;

    if (IsValidTarget(m_targetID) and (m_destinyMgr != nullptr))
        m_destinyMgr->SendSpecialEffect(
                m_shipRef->itemID(),
                m_modRef->itemID(),
                m_modRef->typeID(),
                m_targetID,
                chgTypeID,
                m_guidStr,
                sFxDataMgr.isOffensive(m_effectID),
                active,   // start    - if (start = 0) THEN remove effect
                active,   // active   - if (start and active) THEN starting ONE-SHOT event of (duration)  (dunno what 'ONE-SHOT event' is)
                (double)timeLeft,           // duration in ms
                m_repeat);   // repeat   - if (repeat > 0) THEN starting REPEAT event  ELSE (repeat == 0) THEN starting TOGGLE event

    // Create Destiny Updates and GFx
    GodmaEnvironment ge;
        ge.selfID = m_modRef->itemID();
        ge.charID = m_shipRef->ownerID();
        ge.shipID = m_shipRef->itemID();
        ge.targetID = m_targetID;
        ge.area = new PyList();   // still dont know what this is.
        ge.effectID = m_effectID;

    if (chgTypeID) {
        GodmaOther go;  // "other" means "charge" in evelang
            go.shipID = ge.shipID;
            go.slotID = m_modRef->flag();
            go.chargeTypeID = chgTypeID;
        ge.other = go.Encode();
    } else {
        ge.other = new PyNone();
    }

    timeLeft /= 1000;
    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = GetFileTimeNow();
        shipEff.start = (active ? 1 : 0);
        shipEff.active = (active ? 1 : 0);
        shipEff.environment = ge.Encode();
        shipEff.startTime = (abort ? (abortTime / EvE::Time::Second) : (shipEff.timeNow - (timeLeft * EvE::Time::Second)));  //if now - startTime > 150000000: return
        shipEff.duration = (abort ? 2 : (active ? cycleTime.get_float() : timeLeft));  // duration in seconds
        shipEff.repeat = m_repeat;
        // will need to check and update for data miners here  (any other cases?)
        if ((groupID() == EVEDB::invGroups::Salvager) and (abort)) {
            // Create Destiny Updates:
            PyTuple* type = new PyTuple(2);
                type->SetItem(0, new PyInt(cacheSolarSystemObjects));
                type->SetItem(1, new PyInt(m_targetSE->GetTypeID()));
            PyDict* dict = new PyDict;
                dict->SetItemString("type", type);
            PyTuple* tuple = new PyTuple(2);
                tuple->SetItem(0, new PyString("SalvagingSuccess"));
                tuple->SetItem(1, dict);
            shipEff.error = tuple;
        } else if (m_needsTarget and (m_targetSE == nullptr)) {
            /*   these both give client warning -  [no messageID: 258855]
            if (IsValidTarget(m_targetID)) {
                PyDict* dict = new PyDict();
                    dict->SetItemString("moduleID", new PyInt(m_modRef->itemID()));
                    dict->SetItemString("targetID", new PyInt(m_targetID));
                PyTuple* tuple = new PyTuple(2);
                    tuple->SetItem(0, new PyString("TargetNoLongerPresent"));
                    tuple->SetItem(1, dict);
                shipEff.error = tuple;
            } else {
                PyDict* dict = new PyDict();
                    dict->SetItemString("moduleID", new PyInt(m_modRef->itemID()));
                PyTuple* tuple = new PyTuple(2);
                    tuple->SetItem(0, new PyString("TargetNoLongerPresentGeneric"));
                    tuple->SetItem(1, dict);
                shipEff.error = tuple;
            }
            // this one doesnt work, either.
            PyDict* dict = new PyDict();
                dict->SetItemString("moduleID", new PyInt(m_modRef->itemID()));
            PyTuple* tuple = new PyTuple(2);
                tuple->SetItem(0, new PyString("TargetNoLongerPresentGeneric"));
                tuple->SetItem(1, dict);
            shipEff.error = new PyNone();
            m_shipRef->GetPilot()->SendNotification("TargetNoLongerPresentGeneric", "charid", &tuple);
            */
            m_targetID = 0;
        /*
         * {'messageKey': 'TargetNoLongerPresent', 'dataID': 17881666, 'suppressable': False, 'bodyID': 258855, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1626}
         * u'TargetNoLongerPresentBody'}(u'{[item]moduleID.name} deactivates as the {[item]targetID.name} it was targeted at is no longer present.', None, {u'{[item]moduleID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleID'}, u'{[item]targetID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'targetID'}})
         * {'messageKey': 'TargetNoLongerPresentGeneric', 'dataID': 17875297, 'suppressable': False, 'bodyID': 256459, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3742}
         * u'TargetNoLongerPresentGenericBody'}(u'{[item]moduleID.name} deactivates as the item it was targeted at is no longer present.', None, {u'{[item]moduleID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleID'}})
         *
         */
        } else
            shipEff.error = new PyNone();
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;

    // this should never hit, but just in case...
    if (m_destinyMgr == nullptr)
        m_destinyMgr = m_shipRef->GetPilot()->GetShipSE()->DestinyMgr();
    m_destinyMgr->SendDestinyUpdate(updates, events, m_destinyMgr->IsWarping());
}

void ActiveModule::LaunchMissile()
{
    if (m_linked)
        if (!m_linkMaster) {    // only firing ONE missile for linked lanuchers, but they ALL use a charge
            // Reduce ammo charge by 1 unit:
            m_chargeRef->SetQuantity(m_chargeRef->quantity() - 1, true);
            // add data to StatisticMgr
            sStatMgr.Increment(Stat::pcMissiles);
            return;
        }

    // Launch a missile, creating a new Destiny object for it
    Client* pClient = m_shipRef->GetPilot();
    if (pClient == nullptr)
        return;
    ItemData idata(m_chargeRef->typeID(), pClient->GetCharacterID(), pClient->GetLocationID(), flagMissile, m_chargeRef->itemName().c_str(), m_shipRef->position() );
    InventoryItemRef missileRef = sItemFactory.SpawnItem(idata);
    if (missileRef.get() == nullptr)
        throw PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", \
        m_chargeRef->itemID(), m_chargeRef->itemName().c_str(), m_chargeRef->typeID() ) );

    SystemManager* pSystem = pClient->SystemMgr();
    Missile* pMissile = new Missile(missileRef, *(pSystem->GetServiceMgr()), pSystem, m_modRef, m_targetSE, m_shipRef->GetPilot()->GetShipSE(), this);
    if (pMissile == nullptr)
        return; // make error here

    double distance = pMissile->GetSelf()->position().distance(m_targetSE->GetPosition());
    double missileSpeed = pMissile->GetSelf()->GetAttribute(AttrMaxVelocity).get_float();
    double travelTime = (distance/missileSpeed);
    if (travelTime < 1)
        travelTime = 1;
    pMissile->SetSpeed(missileSpeed);
    pMissile->SetHitTimer(travelTime *1000);
    pMissile->DestinyMgr()->MakeMissile(pMissile);

    // Reduce ammo charge by 1 unit:
    m_chargeRef->SetQuantity(m_chargeRef->quantity() - 1, true);

    // tell target a missile has been launched at them.. (defender missile trigger for ship, tower, pos, npc, others?)
    m_targetSE->MissileLaunched(pMissile);

    // add data to StatisticMgr
    sStatMgr.Increment(Stat::pcMissiles);
}

void ActiveModule::LaunchProbe()
{
    Client* pClient = m_shipRef->GetPilot();
    if (pClient == nullptr)
        return;
    if (pClient->scan() == nullptr)
        pClient->SetScan(new Scan(pClient));

    uint8 pcount = pClient->scan()->GetProbeCount();
    if (pcount == (pClient->GetChar()->GetSkillLevel(skillAstrometrics) +1)) {
        pClient->SendErrorMsg("You can only control %u probes based on your current skills.", pcount);
        return;
    }

    GPoint pos(m_shipRef->position());
    pos.MakeRandomPointOnSphere(MakeRandomFloat(500 +m_shipRef->radius(), 1500 +m_shipRef->radius()));

    //ItemData( uint32 _typeID, uint32 _ownerID, uint32 _locationID, EVEItemFlags _flag, uint32 _quantity);
    // we are not changing singleton status of probes
    ItemData idata(m_chargeRef->typeID(), pClient->GetCharacterID(), pClient->GetLocationID(), flagAutoFit, 1);
    ProbeItemRef probeRef = ProbeItem::Spawn(idata);
    if (probeRef.get() == nullptr)
        throw PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", \
                m_chargeRef->itemID(), m_chargeRef->itemName().c_str(), m_chargeRef->typeID() ) );

    probeRef->Relocate(pos);
    SystemManager* pSystem = pClient->SystemMgr();
    ProbeSE* pProbe = new ProbeSE(probeRef, *(pSystem->GetServiceMgr()), pSystem, m_modRef, m_shipRef);
    if (pProbe == nullptr)
        return; // make error here

    pProbe->SendNewProbe();
    pSystem->AddEntity(pProbe);
    pClient->scan()->AddProbe(pProbe);

    // Reduce ammo charge by 1 unit:
    m_chargeRef->SetQuantity(m_chargeRef->quantity() - 1, true);

    // add data to StatisticMgr
    sStatMgr.Increment(Stat::probesLaunched);
}

void ActiveModule::LaunchSnowBall()
{

}
