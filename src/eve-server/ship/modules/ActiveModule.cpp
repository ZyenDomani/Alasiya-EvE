
 /**
  * @name ActiveModule.cpp
  *   active module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */


#include "eve-server.h"

#include "ship/Missile.h"
#include "ship/modules/ActiveModule.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/BeltMgr.h"

ActiveModule::ActiveModule(InventoryItemRef iRef, ShipItemRef sRef)
: GenericModule(iRef, sRef),
m_timer(1000),
m_reloadTimer(10000)
{
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
    } else
        m_chargeRef = iRef;

    m_reloadTimer.Disable();

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
    m_timer.Disable();

    m_shipRef->ClearTargetRef();
}

void ActiveModule::Process()
{
    if (m_reloadTimer.Enabled()) {
        if (m_reloadTimer.Check(false)) {
            // charge loading complete
            m_reloadTimer.Disable();
            m_ChargeState = ModStates::ChargeStates::CHG_LOADED;
            // apply charge effects here after "loading" is complete
            sFxProc.ApplyEffects(m_chargeRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), true);
        }
    }
    if (m_ModuleState == ModStates::ModuleStates::MOD_ONLINE)
        return;

    // timing and verification function
    //check if we have signal to stop the cycle
    if ((m_Stop) and (m_ModuleState != ModStates::ModuleStates::MOD_ONLINE)) {
        //wait for time to run out and send deactivate to client
        if (m_timer.Check(false)) {
            m_timer.Disable();
            SetModuleState(ModStates::ModuleStates::MOD_DEACTIVATING);
            DeactivateCycle();
        }
        // we have stop signal....dont process any further
        return;
    }

    // the order of these next two is significant for reloading modules.  check for reload before DoCycle for this tic
    if (m_needsCharge) {
        // is this right?  should i do something else here?
        if ((m_chargeRef.get() == nullptr) or (m_ChargeState == ModStates::ChargeStates::CHG_UNLOADED) or (!m_chargeRef->quantity()) or (!m_chargeLoaded)) {
            UnloadCharge();
            SetModuleState(ModStates::ModuleStates::MOD_DEACTIVATING);
            DeactivateCycle(true);
            return;
        }
    }

    if (m_timer.Check())
        ProcessActiveCycle();
}

void ActiveModule::Activate(uint16 effectID, uint32 targetID/*0*/, int16 repeat/*0*/)
{
    if ((m_needsCharge) and ((!m_chargeLoaded) or (m_chargeRef.get() == nullptr))) {
        _log(SHIP__MODULE_WARNING, "ActiveModule::Activate() - Cannot find loaded charge for this module");
        if (m_shipRef->HasPilot())
            if (m_shipRef->GetPilot()->CanThrow())
                throw PyException( MakeCustomError( "Cannot find loaded charge for this module  - Ref: ServerError 15693"));
        return;
    }
    if (targetID) {
        m_targetID = targetID;
        m_targetSE = m_shipRef->GetPilot()->SystemMgr()->GetSE(targetID);
        if (m_targetSE == nullptr) {
            sLog.Error("ActiveModule::Activate()", "m_targetSE == NULL");
            m_shipRef->GetPilot()->SendErrorMsg("Current target was not found.  Ref: ServerError 25263");
            Clear();
            return;
        }
    }
    m_Stop = false;
    m_repeat = repeat;
    m_effectID = effectID;
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
    if (targetID)
        ApplyEffect(Effects::dgmStateTarget, true);

    ShowEffect(true, false);

    m_ModuleState = ModStates::ModuleStates::MOD_ACTIVATED;

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

    if (!m_repeat)
        m_Stop = true;
}

void ActiveModule::Deactivate(std::string effect/*""*/)
{
    if (m_ModuleState != ModStates::ModuleStates::MOD_ACTIVATED)
        return;

    _log(SHIP__MODULE_TRACE, "ActiveModule::Deactivate() - module %u(%s) remaining time %ums.", \
            m_modRef->itemID(), m_modRef->itemName().c_str(), GetRemainingCycleTimeMS());

    if ((m_effectID == EVEEffectID::miningLaser) or (m_effectID == EVEEffectID::miningClouds)) {
        AbortCycle();
        return;
    }

    m_Stop = true;

    SetModuleState(ModStates::ModuleStates::MOD_DEACTIVATING);
}

void ActiveModule::Overload()
{
    m_overLoaded = true;
    GenericModule::Overload();
}

void ActiveModule::DeOverload()
{
    m_overLoaded = false;
    GenericModule::DeOverload();
}

// yes, the xxCycle() shit below seems overkill, but each has a specific purpose
uint32 ActiveModule::DoCycle()
{
    if ((m_destinyMgr == nullptr) or (m_bubble == nullptr)) {
        // make error for no destiny/bubble
        Deactivate();
        return 0;
    }
    if ((m_targetID) and (m_targMgr != nullptr)) {
        // data consistency check
        if (m_targMgr->GetTarget(m_targetID) != m_targetSE) {
            _log(SHIP__MODULE_WARNING, "GetTarget() != m_targetSE");
            Deactivate();
            return 0;
        }
    }
    if (m_needsCharge) {
        // modules that use scripts arent considered as needsCharge, as they work with or without the script.
        if ((!m_chargeLoaded) or (m_chargeRef.get() == nullptr)) {
            m_shipRef->GetPilot()->SendErrorMsg("Your %s has no loaded charge.  Deactivating.", m_modRef->itemName().c_str());
            Deactivate();
            return 0;
        }
    }

    // not sure if this is entirely accurate...wip
    switch (m_modRef->groupID()) {
        case EVEDB::invGroups::Artifacts_and_Prototypes: { // (this module group will need specific code)
        } break;
        case EVEDB::invGroups::Passive_Targeting_System: { // (this passive module will need specific code)
        } break;
        case EVEDB::invGroups::Scan_Probe_Launcher: { // (this active module will need specific code)
        } break;
        case EVEDB::invGroups::Automated_Targeting_System: { // (this active module will need specific code)
        } break;

        case EVEDB::invGroups::ECM:
        case EVEDB::invGroups::ECCM:
        case EVEDB::invGroups::Gang_Coordinator:
        case EVEDB::invGroups::Cloaking_Device:
        case EVEDB::invGroups::Target_Painter:
        case EVEDB::invGroups::Siege_Module:
        case EVEDB::invGroups::Super_Weapon:
        case EVEDB::invGroups::Interdiction_Sphere_Launcher:
        case EVEDB::invGroups::Jump_Portal_Generator:
        case EVEDB::invGroups::Cynosural_Field:
        case EVEDB::invGroups::Remote_ECM_Burst:
        case EVEDB::invGroups::Warp_Disrupt_Field_Generator:
        case EVEDB::invGroups::Covert_Cynosural_Field_Generator:
        case EVEDB::invGroups::Energy_Destabilizer:
        case EVEDB::invGroups::Energy_Vampire:
        case EVEDB::invGroups::Smart_Bomb:
        case EVEDB::invGroups::ECM_Burst:
        // these neither require nor consume charges
        case EVEDB::invGroups::Remote_Sensor_Damper:
        case EVEDB::invGroups::Tracking_Link:
        case EVEDB::invGroups::Signal_Amplifier:
        case EVEDB::invGroups::Tracking_Enhancer:
        case EVEDB::invGroups::Sensor_Booster:
        case EVEDB::invGroups::Tracking_Computer:
        case EVEDB::invGroups::Projected_ECCM:
        case EVEDB::invGroups::Remote_Sensor_Booster:
        case EVEDB::invGroups::Tracking_Disruptor: {
        } break;
        case EVEDB::invGroups::Capacitor_Booster:{
            UpdateCharge(AttrCapacitorCharge, AttrCapacitorCapacity, AttrPowerTransferAmount, m_shipRef);
        } break;
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
        case EVEDB::invGroups::Missile_Launcher_Snowball:
        case EVEDB::invGroups::Missile_Launcher_Standard: {
            LaunchMissile();
        } break;
    }

    EvilNumber cycleTime = 0;
    if (m_modRef->HasAttribute(AttrDuration, cycleTime))
        return cycleTime.get_int();
    else if (m_modRef->HasAttribute(AttrSpeed, cycleTime))
        return cycleTime.get_int();
    else
        return 10000; // return 10s and make error for no duration attribute
}

void ActiveModule::AbortCycle()
{
    if (m_Stop)
        return;
    // Immediately stop active cycle for things such as insufficient cap, remove module, init warp, target left bubble, or miner deactivated by player:
    m_Stop = true;
    SetModuleState(ModStates::ModuleStates::MOD_DEACTIVATING);
    DeactivateCycle(true);
    m_timer.Disable();
}

void ActiveModule::DeactivateCycle(bool abort/*false*/)
{
    if (m_ModuleState != ModStates::ModuleStates::MOD_DEACTIVATING)
        return;

    ApplyEffect(Effects::dgmStateActive, false);
    if (m_targetID)
        ApplyEffect(Effects::dgmStateTarget, false);

    ShowEffect(false, abort);

    SetModuleState(ModStates::ModuleStates::MOD_ONLINE);

    switch (groupID()) {
        case EVEDB::invGroups::Tractor_Beam: {
            m_targetSE->DestinyMgr()->TractorBeamStop();
        } break;
        case EVEDB::invGroups::Afterburner:
        case EVEDB::invGroups::Microwarpdrive: {
            m_destinyMgr->SpeedBoost(true);
        } break;
        case EVEDB::invGroups::Stasis_Web: {
            m_targetSE->DestinyMgr()->WebbedMe(m_modRef, false);
        } break;
        case EVEDB::invGroups::Survey_Scanner: {
            // this is the complete belt scanner code here.
            PyTuple* tuple = new PyTuple(2);
            tuple->SetItem(0, new PyString("OnSurveyScanComplete"));
            PyList* list = new PyList();
            tuple->SetItem(1, list);
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
            events.push_back(tuple);
            std::vector<PyTuple*> updates;
            m_destinyMgr->SendDestinyUpdate(updates, events, false);
        } break;
        case EVEDB::invGroups::Ship_Scanner:
        case EVEDB::invGroups::Cargo_Scanner:
        case EVEDB::invGroups::System_Scanner: {
        } break;
    }

    Clear();
}

void ActiveModule::ProcessActiveCycle() {
    if (m_Stop)
        return;

    float newCap = (m_shipRef->GetAttribute(AttrCapacitorCharge).get_float() - GetAttribute(AttrCapacitorNeed)).get_float();
    if (newCap >= 0 ) {
        m_shipRef->SetAttribute(AttrCapacitorCharge, newCap);
        SetTimer(DoCycle());
    } else
        AbortCycle();
}

void ActiveModule::SetTimer(uint32 time) {
    if (!time)
        return;
    _log(SHIP__MODULE_TRACE, "ActiveModule::SetTimer() - Started with %u ms.", time);
    m_timer.Start(time);
}

void ActiveModule::LoadCharge(InventoryItemRef chargeRef)
{
    if (chargeRef.get() == nullptr) {
        _log(SHIP__MODULE_WARNING, "ActiveModule::LoadCharge() - Cannot find charge to load into this module");
        if (m_shipRef->HasPilot())
            if (m_shipRef->GetPilot()->CanThrow())
                throw PyException( MakeCustomError( "Cannot find charge to load into this module  - Ref: ServerError 15693"));
            return;
    }

    m_chargeRef = chargeRef;
    m_chargeLoaded = true;
    m_ChargeState = ModStates::ChargeStates::CHG_LOADING;

    /*
     * def OnChargeBeingLoadedToModule(self, moduleIDs, chargeTypeID, reloadTime):
     *  {returns}
     *        [PyTuple 3 items]
     *          [PyTuple 1 items]
     *            [PyIntegerVar 1005885547063]  << moduleID
     *          [PyInt 203]                     << chargeTypeID
     *          [PyFloat 10000]                 << reloadTime (ms)
     */
    if (m_shipRef->GetPilot()->IsInSpace() and !m_shipRef->GetPilot()->IsLogin()) {
        PyTuple* module = new PyTuple(1);
            module->SetItem(0, new PyInt(m_modRef->itemID()));
        PyTuple* tmp = new PyTuple(3);
            tmp->SetItem(0, module);
            tmp->SetItem(1, new PyInt(chargeRef->typeID()));
            tmp->SetItem(2, new PyInt(m_reloadTime));
        m_shipRef->GetPilot()->SendNotification("OnChargeBeingLoadedToModule", "shipid", &tmp, false); //unsequenced.
        m_reloadTimer.Start(m_reloadTime);
    }
    // process new charge's effects here
    chargeRef->ClearModifiers();
    fxData data;
    data.action = Effects::Action::dgmActInvalid;
    data.srcRef = chargeRef;
    for (auto it : chargeRef->type().m_stateFxMap) {
        data.math = data.targLoc = data.targAttr = data.srcAttr = data.grpID = data.typeID = data.fxSrc = 0;
        sFxProc.ParseExpression(chargeRef.get(), sFxDataMgr.GetExpression(it.second.preExpression), data, this);
    }
    if (m_shipRef->GetPilot()->IsLogin() or m_shipRef->GetPilot()->IsDocked()) {
        m_ChargeState = ModStates::ChargeStates::CHG_LOADED;
        sFxProc.ApplyEffects(chargeRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), m_shipRef->GetPilot()->IsInSpace());
    }
}

void ActiveModule::UnloadCharge()
{
    /** @todo  this isnt right.  need to remove EXISTING modifier data.....NOT this new data.
     *    also, DONT reset modifiermap before remoing, to use existing, modified data
     */
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
         */
        sFxProc.ApplyEffects(m_chargeRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), m_shipRef->GetPilot()->IsInSpace());
    }

    m_chargeRef = InventoryItemRef();       // Ensure ref is NULL
    m_chargeLoaded = false;
    m_ChargeState = ModStates::ChargeStates::CHG_UNLOADED;
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
    return true;
}


void ActiveModule::ShowEffect(bool active/*false*/, bool abort/*false*/)
{
    if (m_shipRef->GetPilot()->GetShipSE()->SysBubble() == nullptr)
        return;

    int64 abortTime = Win32TimeNow();
    if (abort) {
        active = false;
        if ((m_effectID == EVEEffectID::miningLaser) or (m_effectID == EVEEffectID::miningClouds))
            abortTime += (2 * Win32Time_Second);    // delay mining abort for 2s to simulate module "completing" its' cycle and dumping ore to cargo
    }

    uint16 chgTypeID = (m_chargeLoaded ? m_chargeRef->typeID() : 0);
    uint32 timeLeft = GetRemainingCycleTimeMS();
    EvilNumber cycleTime = 0;
    if (m_modRef->HasAttribute(AttrDuration, cycleTime))
        ;
    else if (m_modRef->HasAttribute(AttrSpeed, cycleTime))
        ;

    if ((m_targetID) and (m_destinyMgr != nullptr))
        m_destinyMgr->SendSpecialEffect(
                m_shipRef->itemID(),
                m_modRef->itemID(),
                m_modRef->typeID(),
                m_targetID,
                chgTypeID,
                m_guidStr,
                sFxDataMgr.isOffensive(m_effectID),
                (active ? true : false),   // start    - if (start = 0) THEN remove effect
                (active ? true : false),   // active   - if (start and active) THEN starting ONE-SHOT event of (duration)  (dunno what 'ONE-SHOT event' is)
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
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = (active ? 1 : 0);
        shipEff.active = (active ? 1 : 0);
        shipEff.environment = ge.Encode();
        shipEff.startTime = (abort ? (abortTime / Win32Time_Second) : (shipEff.timeNow - (timeLeft * Win32Time_Second)));  //if now - startTime > 150000000: return
        shipEff.duration = (abort ? 2 : (active ? cycleTime.get_float() : timeLeft));  // duration in seconds
        shipEff.repeat = m_repeat;
        if ((groupID() == EVEDB::invGroups::Salvager) and (abort)) {
            // Create Destiny Updates:
            PyTuple* type = new PyTuple(2);
                type->SetItem(0, new PyInt(cacheSolarSystemObjects));
                type->SetItem(1, new PyInt(m_targetSE->GetTypeID()));
            PyDict* dict = new PyDict;
                dict->SetItemString("type", type);
            PyTuple* tup = new PyTuple(2);
                tup->SetItem(0, new PyString("SalvagingSuccess"));
                tup->SetItem(1, dict);
            shipEff.error = tup;
        } else
            shipEff.error = new PyNone();
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    if (m_destinyMgr != nullptr)
        m_destinyMgr->SendDestinyUpdate(updates, events, false);
    else
        m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}


void ActiveModule::LaunchMissile()
{
    // Actually Launch a missile, creating a new Destiny object for it
    Client* pClient = m_shipRef->GetPilot();
    if (pClient == nullptr)
        return;
    ItemData idata(m_chargeRef->typeID(), pClient->GetCharacterID(), pClient->GetLocationID(), flagMissile, m_chargeRef->itemName().c_str(), m_shipRef->position() );
    InventoryItemRef missileRef = sItemFactory.SpawnItem(idata);
    if (missileRef.get() == nullptr)
        throw PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", \
                m_chargeRef->itemID(), m_chargeRef->itemName().c_str(), m_chargeRef->typeID() ) );

    SystemManager* pSystem = pClient->SystemMgr();
    Missile* pMissile = new Missile(missileRef, *(pSystem->GetServiceMgr()),  pSystem, m_modRef, m_targetSE, m_shipRef.get());
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
}
