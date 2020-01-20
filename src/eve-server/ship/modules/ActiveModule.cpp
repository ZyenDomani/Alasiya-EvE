 /**
  * @name ActiveModule.cpp
  *   active module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */


#include "eve-server.h"

#include "StatisticMgr.h"
#include "exploration/Probes.h"
#include "exploration/Scan.h"
#include "ship/Missile.h"
#include "ship/modules/ActiveModule.h"
#include "ship/modules/ModuleItem.h"
#include "system/Container.h"
#include "system/cosmicMgrs/BeltMgr.h"


ActiveModule::ActiveModule(ModuleItemRef mRef, ShipItemRef sRef)
: GenericModule(mRef, sRef),
m_timer(0, true),
m_reloadTimer(0),
m_bubble(nullptr),
m_sysMgr(nullptr),
m_targMgr(nullptr),
m_targetSE(nullptr),
m_destinyMgr(nullptr)
{
    m_Stop = true;
    m_needsCharge = false;
    m_needsTarget = false;

    m_repeat = 1000;    //arbitrary.

    m_guidStr = "";
    m_targetID = 0;
    m_effectID = 0;

    // civilian turrets dont use charges.  this is checked/hacked in TurretModule() to fix error when firing.
    m_needsCharge = mRef->HasAttribute(AttrChargeGroup1);
    if (m_needsCharge) {
        switch (mRef->groupID()) {
            // these neither require nor consume charges...may be wrong.  some of these use scripts.  to verify
            case EVEDB::invGroups::Remote_Sensor_Damper:
            case EVEDB::invGroups::Tracking_Link:
            case EVEDB::invGroups::Signal_Amplifier:
            case EVEDB::invGroups::Tracking_Enhancer:
            case EVEDB::invGroups::Sensor_Booster:
            case EVEDB::invGroups::Tracking_Computer:
            case EVEDB::invGroups::Projected_ECCM:
            case EVEDB::invGroups::Remote_Sensor_Booster:
            case EVEDB::invGroups::Tracking_Disruptor:
            // t2 mining laser can be used without charge by using default extraction rate
            case EVEDB::invGroups::Frequency_Mining_Laser: {
                m_needsCharge = false;
            } break;
        }
    }

    // these groups receive a 3% increase in scan range on Alasiya
    switch (mRef->groupID()) {
        case EVEDB::invGroups::Ship_Scanner: {
            float range = GetAttribute(AttrShipScanRange).get_float();
            range *= (1 + (0.03 * (m_shipRef->GetPilot()->GetChar()->GetSkillLevel(EvESkill::LongRangeTargeting, true))));
            SetAttribute(AttrShipScanRange, range);
        } break;
        case EVEDB::invGroups::Cargo_Scanner: {
            float range = GetAttribute(AttrCargoScanRange).get_float();
            range *= (1 + (0.03 * (m_shipRef->GetPilot()->GetChar()->GetSkillLevel(EvESkill::LongRangeTargeting, true))));
            SetAttribute(AttrCargoScanRange, range);
        } break;
        case EVEDB::invGroups::Survey_Scanner: {
            float range = GetAttribute(AttrSurveyScanRange).get_float();
            range *= (1 + (0.03 * (m_shipRef->GetPilot()->GetChar()->GetSkillLevel(EvESkill::LongRangeTargeting, true))));
            SetAttribute(AttrSurveyScanRange, range);
        } break;
        case EVEDB::invGroups::Shield_Transporter: {
            float range = GetAttribute(AttrShieldTransferRange).get_float();
            range *= (1 + (0.03 * (m_shipRef->GetPilot()->GetChar()->GetSkillLevel(EvESkill::LongRangeTargeting, true))));
            SetAttribute(AttrShieldTransferRange, range);
        } break;
        case EVEDB::invGroups::Energy_Vampire:
        case EVEDB::invGroups::Energy_Transfer_Array: {
            float range = GetAttribute(AttrPowerTransferRange).get_float();
            range *= (1 + (0.03 * (m_shipRef->GetPilot()->GetChar()->GetSkillLevel(EvESkill::LongRangeTargeting, true))));
            SetAttribute(AttrPowerTransferRange, range);
        } break;
        case EVEDB::invGroups::Energy_Destabilizer: {
            float range = GetAttribute(AttrEnergyDestabilizationRange).get_float();
            range *= (1 + (0.03 * (m_shipRef->GetPilot()->GetChar()->GetSkillLevel(EvESkill::LongRangeTargeting, true))));
            SetAttribute(AttrEnergyDestabilizationRange, range);
        } break;
        case EVEDB::invGroups::Salvager:
        case EVEDB::invGroups::Target_Painter:
        case EVEDB::invGroups::Tracking_Disruptor:
        case EVEDB::invGroups::Gas_Cloud_Harvester:
        case EVEDB::invGroups::Remote_Sensor_Damper:
        case EVEDB::invGroups::Remote_Sensor_Booster:
        case EVEDB::invGroups::Armor_Repair_Projector: {
            float range = GetAttribute(AttrMaxRange).get_float();
            range *= (1 + (0.03 * (m_shipRef->GetPilot()->GetChar()->GetSkillLevel(EvESkill::LongRangeTargeting, true))));
            SetAttribute(AttrMaxRange, range);
        } break;
        /*  these are 50AU.  we dont need to increase it...
        case EVEDB::invGroups::System_Scanner:  {
            float range = GetAttribute(AttrScanRange).get_float();        // range in AU
            range *= (1 + (0.03 * (m_shipRef->GetPilot()->GetChar()->GetSkillLevel(EvESkill::LongRangeTargeting, true))));
            SetAttribute(AttrScanRange, range);
        } break;
        */
    }

    // this is an internal variable only.
    m_reloadTime = GetAttribute(AttrReloadTime).get_uint32();
    // set default of 4s for turrets, 5s for snowball and probe launchers, 7s for missile launchers, and 10s for others.
    if (m_needsCharge)  {
        if (m_reloadTime < 1) {
            switch (mRef->groupID()) {
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

    //Clear();
    //GM_Modules = 353,

    if (m_reloadTime > 0)
        _log(MODULE__TRACE, "Reload time for %s(%u) set to %ums", mRef->name(), mRef->itemID(), m_reloadTime);
}

void ActiveModule::Clear()
{
    _log(MODULE__TRACE, "%s calling Clear()", m_modRef->name());
    if (m_targetSE != nullptr)
        if (m_targetSE->TargetMgr() != nullptr)
            m_targetSE->TargetMgr()->RemoveTargetModule(this);

    m_targetSE = nullptr;

    m_Stop = true;
    m_repeat = 1000;
    m_targetID = 0;
    m_effectID = 0;
    m_guidStr = "";
    m_bubble = nullptr;
    m_sysMgr = nullptr;
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
            m_chargeRef->SetAttribute(AttrQuantity, m_chargeRef->quantity(), false);
            // apply charge effects here after loading is complete, but only for empty modules (no previous charge fx)
            if (!m_chargeLoaded)
                sFxProc.ApplyEffects(m_chargeRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), true);
            m_chargeLoaded = true;
            SetChargeState(Module::State::Loaded);
        }
    }

    if (m_ModuleState < Module::State::Deactivating)
        return;

    // if chargestate is loading or reloading, deny further processing and let m_reloadTimer handle it.
    if (m_ChargeState > Module::State::Loaded) {
        _log(MODULE__TRACE, "ActiveModule::Process - %s on %s is loading.", m_modRef->name(), m_shipRef->name());
        return;
    }

    // check for module cycle timer.  if module is stopped, this will process the deactivation and set stop
    if (m_timer.Check())
        ProcessActiveCycle();

    if (m_Stop)
        return;

    if (m_needsTarget) {
        if (m_targetSE == nullptr) {
            _log(MODULE__TRACE, "%s - targetSE == nullptr.  calling AbortCycle()", m_modRef->name());
            m_shipRef->GetPilot()->SendErrorMsg("The target for your %s not found.  Aborting current cycle.", m_modRef->name());
            AbortCycle();
            return;
        }
        if (m_targetSE->GetID() != m_targetID) {
            _log(MODULE__TRACE, "%s - targetSE != targetID.  calling AbortCycle()", m_modRef->name());
            m_shipRef->GetPilot()->SendErrorMsg("Your %s has target mismatch.  Aborting current cycle.", m_modRef->name());
            AbortCycle();
            return;
        }
    }

    if (m_needsCharge) {
        // is this right?  should i do something else here?
        if (m_ChargeState == Module::State::Unloaded) {
            _log(MODULE__TRACE, "ActiveModule::Process - %s on %s needs charge but moduleState is unloaded.", m_modRef->name(), m_shipRef->name());
            m_Stop = true;
        }
        if (!m_chargeLoaded) {
            _log(MODULE__TRACE, "ActiveModule::Process - %s on %s needs charge but chargeLoaded is false.", m_modRef->name(), m_shipRef->name());
            m_Stop = true;
        }
        if (m_chargeRef.get() == nullptr) {
            _log(MODULE__TRACE, "ActiveModule::Process - %s on %s needs charge but chargeRef is NULL.", m_modRef->name(), m_shipRef->name());
            m_Stop = true;
        } else if (m_chargeRef->quantity() < 1) {
            _log(MODULE__TRACE, "ActiveModule::Process - %s on %s needs charge but has 0 qty.", m_modRef->name(), m_shipRef->name());
            m_Stop = true;
            UnloadCharge();
            DeactivateCycle(true);
            // dont send msg to client about no loaded charge...just silently DeactivateCycle.
            return;
        }
        if (m_Stop) {
            m_shipRef->GetPilot()->SendErrorMsg("Your %s has no loaded charge.  Deactivating.", m_modRef->name());
            DeactivateCycle(true);
        }
    }
}

void ActiveModule::RemoveTarget(SystemEntity* pSE) {
    if (m_targetSE == pSE) {
        _log(MODULE__TRACE, "ActiveModule::RemoveTarget called on %s on %s to remove %s", m_modRef->name(), m_shipRef->name(), pSE->GetName());
        Deactivate();
    }
}

void ActiveModule::Activate(uint16 effectID, uint32 targetID/*0*/, int16 repeat/*0*/)
{
    if (effectID == 16) {   // catchall for elusive online/offline error
        Online();
        return;
    }
    if (m_needsCharge and (!m_chargeLoaded or (m_chargeRef.get() == nullptr))) {
        _log(MODULE__TRACE, "ActiveModule::Activate - %s: needsCharge: %s, chargeLoaded: %s, chargeRef: %s", \
                m_modRef->name(), m_needsCharge?"True":"False", m_chargeLoaded?"True":"False", \
                m_chargeRef.get() == nullptr ? "(none)":"m_chargeRef->name()");
        Clear();
        throw PyException(MakeCustomError("Your %s doesn't seem to be loaded.", m_modRef->name()));
    }
    if (IsValidTarget(targetID)) {
        // this is just a guess.  may have to use groupID test to verify if this doesnt work right.
        // also need to make check for modules acting on OUR ship....in which case this will be wrong.
        m_needsTarget = true;
        m_targetID = targetID;
        m_targetSE = m_shipRef->GetPilot()->SystemMgr()->GetSE(targetID);
        if (m_targetSE == nullptr) {
            Clear();
            throw PyException(MakeUserError("DeniedActivateTargetNotPresent"));
        }
    }

    if (m_targetSE != nullptr) {
        /** @todo criminal shit isnt written yet....fix this once it is.
        if (sFxDataMgr.isAssistance(effectID))
            if (m_targetSE->HasPilot())
                if (m_targetSE->GetPilot()->IsCriminal())
                    throw PyException(MakeUserError("ModuleActivationDeniedCriminalAssistance"));
        */

        /*
         * AttrdisallowAgainstEwImmuneTarget
         * AttrDisallowOffensiveModifiers
         * AttrDisallowOffensiveModifierBonus
         */

        if (m_targetSE->GetSelf()->HasAttribute(AttrDisallowAssistance)) {
            Clear();
            throw PyException(MakeUserError("DeniedActivateTargetAssistDisallowed"));
        }
        if (m_targetSE->IsCOSE()) {
            Clear();
            throw PyException(MakeCustomError("Attacking Customs Offices isn't implemented at this time."));
        }
        if (m_targetSE->TargetMgr() != nullptr)
            m_targetSE->TargetMgr()->AddTargetModule(this);
    }

    m_Stop = false;
    m_repeat = repeat;
    m_effectID = effectID;

    if (!CanActivate()) {
        Clear();
        return;
    }

    m_isWarpSafe = sFxDataMgr.isWarpSafe(m_effectID);
    m_guidStr = sFxDataMgr.GetEffectGuid(m_effectID);

    Ship* pShip = m_shipRef->GetPilot()->GetShipSE();
    m_bubble = pShip->SysBubble();
    m_sysMgr = pShip->SystemMgr();
    m_targMgr = pShip->TargetMgr();
    m_destinyMgr = pShip->DestinyMgr();

    switch (groupID()) {
        case EVEDB::invGroups::Afterburner:
        case EVEDB::invGroups::Microwarpdrive: {
            m_destinyMgr->SpeedBoost();
        } break;
        case EVEDB::invGroups::Tractor_Beam: {
            if (m_targetSE != nullptr) {
                if (m_targetSE->DestinyMgr()->IsTractored()) {
                    std::map<std::string, PyRep *> arg;
                    arg["module"] = new PyInt(m_targetSE->GetID());
                    Clear();
                    throw PyException(MakeUserError("InvalidTargetCanAlreadyTractored", arg));
                }
                // test for ownership here...wip
                // once crim shit is implemented, allow for tractoring no matter owner.
                bool owner = false, fleet = false, corp = false, ally = false, war = false;
                if (m_targetSE->GetOwnerID() == m_shipRef->ownerID())
                    owner = true;
                if (m_targetSE->GetCorporationID() == pShip->GetCorporationID())
                    corp = true;
                if (m_targetSE->GetAllianceID() == pShip->GetAllianceID())
                    ally = true;
                if (m_targetSE->GetWarFactionID() == pShip->GetWarFactionID())
                    war = true;
                if (m_shipRef->GetPilot()->InFleet())
                    if (m_shipRef->GetPilot()->GetFleetID() == m_targetSE->GetFleetID())
                        fleet = true;

                if (owner or fleet or corp or ally or war) {
                    m_targetSE->DestinyMgr()->TractorBeamStart(pShip, GetAttribute(AttrMaxTractorVelocity));
                } else {
                    std::map<std::string, PyRep *> arg;
                    arg["module"] = new PyInt(m_targetSE->GetID());
                    Clear();
                    throw PyException(MakeUserError("InvalidTargetCanOwner", arg));
                }

            }
        } break;
        case EVEDB::invGroups::Stasis_Web: {
            if (m_targetSE != nullptr)
                m_targetSE->DestinyMgr()->WebbedMe(m_modRef, true);
        } break;
    }

    // Do initial cycle immediately while we start timer
    SetTimer(DoCycle());

    if (!m_timer.Enabled()) {
        // if the timer wasnt set (for whatever reason), kill activation and return
        Clear();
        return;
    }

    ApplyEffect(FX::State::Active, true);
    if (IsValidTarget(targetID))
        ApplyEffect(FX::State::Target, true);

    ShowEffect(true, false);

    SetModuleState(Module::State::Activated);

    --m_repeat;
    if (m_repeat < 1) {
        m_Stop = true;
        //m_timer.Disable();
        //DeactivateCycle();
    }

    // check for one-hit kills and stop module after cycle completes
    if (m_needsTarget)
        if (m_targetSE->IsDead()) {
            m_Stop = true;
            //m_timer.Disable();
            //DeactivateCycle();
        }
}

void ActiveModule::Deactivate(std::string effect/*""*/)
{
    if (m_ModuleState != Module::State::Activated)
        return;

    _log(MODULE__TRACE, "ActiveModule::Deactivate(%s) - module %s(%u) remaining time %ums.", \
            effect.c_str(), m_modRef->name(), m_modRef->itemID(), GetRemainingCycleTimeMS());

    SetModuleState(Module::State::Deactivating);

    if ((m_effectID == EVEEffectID::miningLaser) or (m_effectID == EVEEffectID::miningClouds)) {
        DeactivateCycle(true);
        return;
    }

    // if we're not mining, wait for module to complete current cycle then shut it down.
    m_Stop = true;

    /* these are not needed.  Clear() calls RemoveTargetModule
    if (effect.compare("TargetDestroyed") == 0) {
        if (m_targetSE->TargetMgr() != nullptr)
            m_targetSE->TargetMgr()->RemoveTargetModule(this);
        // this is sent back in OnGodmaShipEffect packet
        //m_targetSE = nullptr;
    }
    if (effect.compare("CargoFull") == 0) {
        if (m_targetSE->TargetMgr() != nullptr)
            m_targetSE->TargetMgr()->RemoveTargetModule(this);
    }
    */
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

uint32 ActiveModule::DoCycle()
{
    if (m_destinyMgr == nullptr) {
        // make error for no destiny/bubble
        AbortCycle();
        return 0;
    }
    // not sure if this is entirely accurate...wip
    switch (m_modRef->groupID()) {
        case EVEDB::invGroups::Artifacts_and_Prototypes: {
        } break;
        // these passive modules will need specific code
        case EVEDB::invGroups::Missile_Launcher_Defender:
        case EVEDB::invGroups::Countermeasure_Launcher:
        case EVEDB::invGroups::Passive_Targeting_System: {
        } break;
        // these active modules will need specific code
        case EVEDB::invGroups::Cynosural_Field:
        case EVEDB::invGroups::Covert_Cynosural_Field_Generator:
        case EVEDB::invGroups::Automated_Targeting_System: {
        } break;

        // these im not sure about yet
        case EVEDB::invGroups::ECM:
        case EVEDB::invGroups::ECCM:
        case EVEDB::invGroups::Gang_Coordinator:
        case EVEDB::invGroups::Cloaking_Device:
        case EVEDB::invGroups::Siege_Module:
        case EVEDB::invGroups::Super_Weapon:
        case EVEDB::invGroups::Interdiction_Sphere_Launcher:    // launch a sphere (like missile and probe)
        case EVEDB::invGroups::Jump_Portal_Generator:
        case EVEDB::invGroups::Remote_ECM_Burst:
        case EVEDB::invGroups::Warp_Disrupt_Field_Generator:
        case EVEDB::invGroups::Smart_Bomb:
        case EVEDB::invGroups::ECM_Burst: {
        } break;

        case EVEDB::invGroups::Capacitor_Booster:{
            ConsumeCharge();
            UpdateCharge(AttrCapacitorCharge, AttrCapacitorCapacity, AttrPowerTransferAmount, m_shipRef);
            m_repeat = 1;
        } break;
        // i *think* these first 2 go here....need testing
        case EVEDB::invGroups::Energy_Vampire:
        case EVEDB::invGroups::Energy_Destabilizer:
        case EVEDB::invGroups::Energy_Transfer_Array: {
            if (m_targetSE != nullptr)
                UpdateCharge(AttrCapacitorCharge, AttrCapacitorCapacity, AttrPowerTransferAmount, m_targetSE->GetSelf());
        } break;
        case EVEDB::invGroups::Shield_Transporter: {
            if (m_targetSE != nullptr)
                UpdateCharge(AttrShieldCharge, AttrShieldCapacity, AttrShieldBonus, m_targetSE->GetSelf());
        } break;
        case EVEDB::invGroups::Shield_Booster: {
            UpdateCharge(AttrShieldCharge, AttrShieldCapacity, AttrShieldBonus, m_shipRef);
        } break;
        case EVEDB::invGroups::Remote_Hull_Repairer: {
            if (m_targetSE != nullptr)
                UpdateDamage(AttrDamage, AttrStructureDamageAmount, m_targetSE->GetSelf());
        } break;
        case EVEDB::invGroups::Hull_Repair_Unit: {
            UpdateDamage(AttrDamage, AttrStructureDamageAmount, m_shipRef);
        } break;
        case EVEDB::invGroups::Armor_Repair_Projector: {
            if (m_targetSE != nullptr)
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
    _log(MODULE__TRACE, "%s calling AbortCycle() - stop:%s", m_modRef->name(), m_Stop?"true":"false");
    // if stop is already set, let module finish cycle
    if (m_Stop)
        return;
    // Immediately stop active cycle for things such as insufficient cap, remove module, init warp, target destroyed, target left bubble, or miner deactivated by player:
    m_Stop = true;
    SetModuleState(Module::State::Deactivating);
    DeactivateCycle(true);
    m_timer.Disable();
}

void ActiveModule::DeactivateCycle(bool abort/*false*/)
{
    _log(MODULE__TRACE, "%s calling DeactivateCycle()", m_modRef->name());
    if ((m_ModuleState == Module::State::Activated) and (!abort)) {
        _log(MODULE__ERROR, "ActiveModule::DeactivateCycle() - Called on %s(%u) with current state %s and !abort.",  \
                m_modRef->name(), m_modRef->itemID(), GetModuleStateName(m_ModuleState));
        EvE::traceStack();
   //     return;
    }

    ApplyEffect(FX::State::Active, false);
    if (IsValidTarget(m_targetID)
    and (m_targetSE != nullptr))
        ApplyEffect(FX::State::Target, false);
    else if (m_needsTarget)
        _log(MODULE__WARNING, "%s - DeactivateCycle() - need target = true and targetID: %u, targSE: %x", \
                m_modRef->name(), m_targetID, m_targetSE);

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
            if (abort) {
                Clear();
                return;
            }
            // this is the complete belt scanner rsp code here.
            PyTuple* result = new PyTuple(2);
            result->SetItem(0, new PyString("OnSurveyScanComplete"));
            PyList* list = new PyList();
            std::vector<AsteroidSE*> vList;
            m_sysMgr->GetBeltMgr()->GetList(sBubbleMgr.GetBeltID(m_bubble->GetID()), vList);
            // when roids are spawned, BeltMgr sets this bubble "IsBelt = true", even in anomalies
            if (m_bubble->IsBelt()) {
                float m_range = GetAttribute(AttrSurveyScanRange).get_float();
                float distance = 0;
                for (auto pASE : vList) {
                    distance = m_shipRef->position().distance(pASE->GetPosition());
                    distance -= pASE->GetRadius();
                    distance -= m_shipRef->radius(); // do we need this one here?
                    if (distance < m_range) {
                        PyTuple* tuple2 = new PyTuple(3);
                        tuple2->SetItem(0, new PyInt(pASE->GetID()));
                        tuple2->SetItem(1, new PyInt(pASE->GetTypeID()));
                        tuple2->SetItem(2, new PyInt(pASE->GetSelf()->GetAttribute(AttrQuantity).get_int()));
                        list->AddItem(tuple2);
                    }
                }
            } else if (m_bubble->IsIce()) {
                // allow ice scanning without a radius check....may change later.
                for (auto pASE : vList) {
                    PyTuple* tuple2 = new PyTuple(3);
                    tuple2->SetItem(0, new PyInt(pASE->GetID()));
                    tuple2->SetItem(1, new PyInt(pASE->GetTypeID()));
                    tuple2->SetItem(2, new PyInt(pASE->GetSelf()->GetAttribute(AttrQuantity).get_int()));
                    list->AddItem(tuple2);
                }
            }

            result->SetItem(1, list);
            // Send results.
            /*
            std::vector<PyTuple*> events;
            events.push_back(result);
            std::vector<PyTuple*> updates;
            m_destinyMgr->SendDestinyUpdate(updates, events, true);
            */
            m_shipRef->GetPilot()->QueueDestinyEvent(&result);
        } break;
        //case EVEDB::invGroups::Data_Miner:
        // some data containers will pop after successful access.  currently incomplete
        case EVEDB::invGroups::Salvager: {
            if (IsSuccess()) {
                _log(MODULE__TRACE, "%s - DeactivateCycle() - Salvage successful.  deleting target %s.", m_modRef->name(), m_targetSE->GetName());
                m_targMgr->RemoveTarget(m_targetSE);
                // just in case other modules are targeting this object, let them know it was destroyed.
                if (m_targetSE->TargetMgr() != nullptr)
                    m_targetSE->TargetMgr()->Destroyed();
                m_targetSE->Delete();
                SafeDelete(m_targetSE);
            }
        } break;
        /*
        case EVEDB::invGroups::Warp_Scrambler: {
            if (m_targetSE != nullptr)
                m_targetSE->GetSelf()->SetAttribute(AttrWarpScrambleStatus);
        } break;
        case EVEDB::invGroups::Warp_Core_Stabilizer: {
            if (m_targetSE != nullptr)
                m_targetSE->GetSelf()->SetAttribute(AttrWarpScrambleStatus);
        } break;
        */
        case EVEDB::invGroups::Ship_Scanner:
        case EVEDB::invGroups::Cargo_Scanner:{
            if (m_targetSE != nullptr)
                ;  // not sure if we need this here.....do these work like belt scanner?
        } break;
        // not sure just how this works yet
        // case EVEDB::invGroups::System_Scanner:
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
    // updated timer will reset cycle time if changed, but i DO NOT have client display coded to reset...this will fuck up timer time in client.  (worse than it already is)
    _log(MODULE__TRACE, "ActiveModule::SetTimer() - %s with %u ms for %s on %s.", \
            (m_timer.Enabled()? "Updated" : "Started"), time, m_modRef->name(), m_shipRef->name());
    m_timer.Start(time);
}

void ActiveModule::LoadCharge(InventoryItemRef chargeRef)
{
    if (chargeRef.get() == nullptr) {
        _log(MODULE__WARNING, "ActiveModule::LoadCharge() for %s - Cannot find charge to load into this module", m_modRef->name());
        return;
    }

    Client* pClient = m_shipRef->GetPilot();
    if (pClient == nullptr) {
        // these two are just in case...
        m_chargeRef->DeleteAttribute(AttrQuantity);
        m_chargeRef = InventoryItemRef(nullptr);
        m_chargeLoaded = false;
        SetChargeState(Module::State::Unloaded);
        return;  // make error here?
    }

    m_chargeRef = chargeRef;
    SetChargeState(Module::State::Loading);

    /*  **** this sets "reload blink" status on weapon button
     * def OnChargeBeingLoadedToModule(self, moduleIDs, chargeTypeID, reloadTime):
     *  {returns}
     *        [PyTuple 3 items]
     *          [PyTuple 1 items]
     *            [PyIntegerVar 1005885547063]  << moduleID (can be multiple, but not coded for multiples yet)
     *          [PyInt 203]                     << chargeTypeID
     *          [PyFloat 10000]                 << reloadTime (ms)
     */
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

    // process new charge's effects (load timer will determine if fx are applied based on existing charge)
    m_chargeRef->ClearModifiers();
    for (auto it : m_chargeRef->type().m_stateFxMap) {
        fxData data = fxData();
        data.action = FX::Action::Invalid;
        data.srcRef = m_chargeRef;
        sFxProc.ParseExpression(m_chargeRef.get(), sFxDataMgr.GetExpression(it.second.preExpression), data, this);
    }
    if (pClient->IsDocked() or (pClient->IsInSpace() and pClient->IsLogin())) {
        m_chargeLoaded = true;
        SetChargeState(Module::State::Loaded);
        sFxProc.ApplyEffects(m_chargeRef.get(), pClient->GetChar().get(), m_shipRef.get(), pClient->IsInSpace());
        //m_modRef->SetAttribute(AttrQuantity, m_chargeRef->quantity(), false);
        m_chargeRef->SetAttribute(AttrQuantity, m_chargeRef->quantity(), false);
    }
}

//{'FullPath': u'UI/Messages', 'messageID': 259200, 'label': u'NoChargesBody'}(u'{launcher} has run out of charges', None, {u'{launcher}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'launcher'}})
//{'FullPath': u'UI/Messages', 'messageID': 259232, 'label': u'NotEnoughChargesBody'}(u'{launcher} has {[numeric]got} charges, but needs {[numeric]need} to fire.', None, {u'{[numeric]got}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'got'}, u'{launcher}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'launcher'}, u'{[numeric]need}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'need'}})
//{'FullPath': u'UI/Messages', 'messageID': 258889, 'label': u'TooManyChargesForLauncherBody'}(u'The launcher is currently holding {[numeric]excess} too many excess units.', None, {u'{[numeric]excess}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'excess'}})

//{'FullPath': u'UI/Messages', 'messageID': 259152, 'label': u'NoSpaceForReplacedItemBody'}(u'There is not enough space in the cargo hold for the charges currently in the {item} to be moved into.', None, {u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}})

void ActiveModule::UnloadCharge()
{
    _log(MODULE__TRACE, "%s calling AM::UnloadCharge()", m_modRef->name());
    if (m_chargeRef.get() != nullptr) {
        if (m_chargeRef->HasAttribute(AttrUnfitCapCost)) {
            float cap = m_shipRef->GetAttribute(AttrCapacitorCharge).get_float();
            cap -= m_chargeRef->GetAttribute(AttrUnfitCapCost).get_float();
            m_shipRef->SetAttribute(AttrCapacitorCharge, cap);
        }
        // remove charge effects here
        m_chargeRef->ClearModifiers();
        for (auto it : m_chargeRef->type().m_stateFxMap) {
            fxData data = fxData();
            data.action = FX::Action::Invalid;
            data.srcRef = m_chargeRef;
            sFxProc.ParseExpression(m_chargeRef.get(), sFxDataMgr.GetExpression(it.second.postExpression), data, this);
        }
        /** @todo  this isnt right.  need to remove EXISTING modifier data.....NOT this new data.
         *    also, DONT reset modifiermap before removing, to use existing, modified data
         *  NOTE:  i've no clue how to do that yet....
         */
        Client* pClient = m_shipRef->GetPilot();
        sFxProc.ApplyEffects(m_chargeRef.get(), pClient->GetChar().get(), m_shipRef.get(), pClient->IsInSpace());
    }

    m_chargeLoaded = false;
    if (m_chargeRef.get() != nullptr) {
        // unloading charge goes straight to attribMap, to send data to client without changing item qty
        m_chargeRef->GetAttributeMap()->AlterChargeQuantity(0, false);
        m_chargeRef->DeleteAttribute(AttrQuantity);
        m_chargeRef = InventoryItemRef(nullptr);       // Ensure ref is NULL
    }
    m_modRef->DeleteAttribute(AttrQuantity);
    SetChargeState(Module::State::Unloaded);
}

void ActiveModule::ConsumeCharge() {
    // common code to reduce ammo by one unit.
    // this also updates item quantity
    m_chargeRef->AlterChargeQuantity(-1);
}

void ActiveModule::ApplyEffect(int8 state, bool active/*false*/)
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
    if (newValue < EvilZero) {
        newValue = EvilZero;
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
    for (auto it : m_chargeRef->type().m_stateFxMap) {
        fxData data = fxData();
        data.action = FX::Action::dgmActInvalid;
        data.srcRef = m_chargeRef;
        sFxProc.ParseExpression(m_chargeRef.get(), sFxDataMgr.GetExpression(it.second.preExpression), data, this);
    } */
    sFxProc.ApplyEffects(m_chargeRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), m_shipRef->GetPilot()->IsInSpace());
}

bool ActiveModule::CanActivate()
{
    // there is still more to be done here.  wip
    //  modules that require specific tests are coded in their module class, which will call this if their specific checks pass

    // check distance for targetable actions
    if (m_targetSE != nullptr) {
        // if target is non-combatant deny attack
        if (sFxDataMgr.isOffensive(m_effectID))
            if ((m_targetSE->IsItemEntity())
            or (m_targetSE->IsStaticEntity())
            or (m_targetSE->IsAsteroidSE()))
           // or (m_targetSE->IsLogin()))       // this is incomplete, so always returns false
            {
                m_shipRef->GetPilot()->SendNotifyMsg("You cannot attack the %s.  Ref: ServerError 16228.", m_targetSE->GetName());
                return false;
            }

        // weapons use ships AttrMaxTargetRange which is checked in TargetMgr
        if (m_turret or m_launcher)
            return  true;

        // test for specific targets and distance here
        float range = 0.0f;
        using namespace EVEDB::invGroups;
        switch (groupID()) {
            case Tractor_Beam: {
                /** @todo  add checks for other items vs cap tractors and maybe some items for small tractors */
                bool allowed = false;
                if ( m_targetSE->IsWreckSE())
                    allowed = true;
                else if (m_targetSE->IsContainerSE()) {
                    if (m_targetSE->GetContSE()->IsAnchored()) {
                        m_shipRef->GetPilot()->SendNotifyMsg("The %s is anchored.  It cannot be tractored.", m_targetSE->GetName());
                        return false;
                    }
                    allowed = true;
                }

                if (allowed) {
                    range = GetAttribute(AttrMaxRange).get_float();
                } else {
                    m_shipRef->GetPilot()->SendNotifyMsg("You cannot tractor the %s.", m_targetSE->GetName());
                    return false;
                }
            } break;
            case Shield_Transporter: {
                range = GetAttribute(AttrShieldTransferRange).get_float();
            } break;
            case Energy_Vampire:
            case Energy_Transfer_Array: {
                range = GetAttribute(AttrPowerTransferRange).get_float();
            } break;
            case Energy_Destabilizer: {
                range = GetAttribute(AttrEnergyDestabilizationRange).get_float();
            } break;
            case Warp_Scrambler: {
                range = GetAttribute(AttrWarpScrambleRange).get_float();
            } break;
            case Cargo_Scanner: {
                range = GetAttribute(AttrCargoScanRange).get_float();
            } break;
            case Ship_Scanner: {
                range = GetAttribute(AttrShipScanRange).get_float();
            } break;
            case Shield_Disruptor:      // no modules in this group
            case Stasis_Web:
            case Salvager:
            case Strip_Miner:
            case Mining_Laser:
            case Target_Painter:
            case Tracking_Disruptor:
            case Gas_Cloud_Harvester:
            case Remote_Sensor_Damper:
            case Remote_Sensor_Booster:
            case Armor_Repair_Projector:
            case Frequency_Mining_Laser:  {
                range = GetAttribute(AttrMaxRange).get_float();
            } break;

            // these scanners *may* not hit here, as they dont need a target, and we really dont want them to test for target
            //  the pilot *may* have a target locked, in which case we really dont want these to hit
            /*
            case System_Scanner: {
                range = GetAttribute(AttrScanRange).get_float();    // this is in AU
                range *= oneauinmeters;
            } break;
            case Survey_Scanner: {
                range = GetAttribute(AttrSurveyScanRange).get_float();
            } break;
            */
            default: {
                // make error here with group
                range = GetAttribute(AttrMaxRange).get_float();
                _log(SHIP__WARNING, "Activate::RangeTest - Default hit for %s(%u) group: %u.  Using %.1f", \
                            m_modRef->name(), m_modRef->itemID(), m_modRef->groupID(), range);
            } break;
        }

        float distance = m_shipRef->position().distance(m_targetSE->GetPosition());
        distance -= m_targetSE->GetRadius();

        _log(MODULE__MESSAGE, "Activate::RangeTest - distance between %s and target %s: %.1f.  range of %s is %.1f", \
                    m_shipRef->name(), m_targetSE->GetName(), distance, m_modRef->name(), range);

        if (distance > range) {
            m_shipRef->GetPilot()->SendNotifyMsg("The %s is %.0f meters from you, outside the effective range of your %s, which is %.0f meters.", \
                    m_targetSE->GetName(), distance, m_modRef->name(), range);
            return false;
        }
    }
    //AttrDeadspaceUnsafe
    //AttrMaxGroupActive
    return true;
}


void ActiveModule::ShowEffect(bool active/*false*/, bool abort/*false*/)
{
    int64 abortTime = GetFileTimeNow();
    if (abort) {
        active = false;
        if ((m_effectID == EVEEffectID::miningLaser)
        or (m_effectID == EVEEffectID::miningClouds))
            abortTime += (5 * EvE::Time::Second);    // delay abort for 5s to simulate module "completing" its' cycle and dumping ore to cargo
        else
            abortTime += (3 * EvE::Time::Second);    // delay abort for 3s to simulate module "completing" its' cycle
    }

    uint16 chgTypeID = (m_chargeLoaded ? m_chargeRef->typeID() : 0);
    uint32 timeLeft = GetRemainingCycleTimeMS();
    EvilNumber cycleTime = EvilZero;
    // these are a bit weird...this HasAttribute() set variable if exists, but need to test each case.
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
        ge.target = IsValidTarget(m_targetID) ? new PyInt(m_targetID) : PyStatic.NewNone();
        ge.area = new PyList();   // still dont know what this is.
        ge.effectID = m_effectID;

    if (chgTypeID) {
        GodmaOther go;  // "other" means "charge" in evelang
            go.shipID = ge.shipID;
            go.slotID = m_modRef->flag();
            go.chargeTypeID = chgTypeID;
        ge.other = go.Encode();
    } else {
        ge.other = PyStatic.NewNone();
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
            shipEff.error = PyStatic.NewNone();
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
            shipEff.error = PyStatic.NewNone();

    PyTuple* tuple = shipEff.Encode();
    /*
    if (is_log_enabled(MODULE__DUMP)) {
        _log(MODULE__DUMP ,"ShowEffects() Dump");
        tuple->Dump(MODULE__DUMP, "    ");
    }
*/
    /*
     std::vector<PyTuple*> events;
         events.push_back(tuple);
     std::vector<PyTuple*> updates;

    // this should never hit, but just in case...
     if (m_destinyMgr == nullptr)
         m_destinyMgr = m_shipRef->GetPilot()->GetShipSE()->DestinyMgr();

     m_destinyMgr->SendDestinyUpdate(updates, events, m_destinyMgr->IsWarping());
     */

    if (m_destinyMgr->IsWarping() or (m_bubble == nullptr))
        m_shipRef->GetPilot()->QueueDestinyEvent(&tuple);
    else
        m_bubble->BubblecastDestinyEvent(&tuple, "destiny");
}

void ActiveModule::LaunchMissile()
{
    // nust not throw here...
    //throw PyException( MakeUserError("TargetingMissileToSelf"));
    //throw PyException( MakeUserError("NoCharges"));

    //{'FullPath': u'UI/Messages', 'messageID': 259200, 'label': u'NoChargesBody'}(u'{launcher} has run out of charges', None, {u'{launcher}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'launcher'}})

    // AttrAimedLaunch ?  what is this used for?
    if (m_linked)
        if (!m_linkMaster) {    // only firing ONE missile for linked lanuchers, but they ALL use a charge
            // Reduce ammo charge by 1 unit:
            ConsumeCharge();
            // add data to StatisticMgr
            sStatMgr.Increment(Stat::pcMissiles);
            return;
        }

    // Launch a missile, creating a new Destiny object for it
    Client* pClient = m_shipRef->GetPilot();
    if (pClient == nullptr)
        return;
    ItemData idata(m_chargeRef->typeID(), pClient->GetCharacterID(), pClient->GetLocationID(), flagMissile, m_chargeRef->name(), m_shipRef->position() );
    InventoryItemRef missileRef = sItemFactory.SpawnItem(idata);
    if (missileRef.get() == nullptr) {
        _log(ITEM__ERROR ,"Unable to spawn item #%u:'%s' of type %u.", m_chargeRef->itemID(), m_chargeRef->name(), m_chargeRef->typeID());
        pClient->SendErrorMsg("Your %s in %s experienced a loading error and was disabled.", m_chargeRef->name(), m_modRef->name());
        AbortCycle();
        return;
    }

    SystemManager* pSystem = pClient->SystemMgr();
    Missile* pMissile = new Missile(missileRef, *(pSystem->GetServiceMgr()), pSystem, m_modRef, m_targetSE, m_shipRef->GetPilot()->GetShipSE(), this);
    if (pMissile == nullptr) {
        _log(ITEM__ERROR ,"Unable to create SE #%u:'%s' of type %u.", m_chargeRef->itemID(), m_chargeRef->name(), m_chargeRef->typeID());
        pClient->SendErrorMsg("Your %s in %s experienced a launching error and was disabled.", m_chargeRef->name(), m_modRef->name());
        AbortCycle();
        return;
    }

    float distance = pMissile->GetSelf()->position().distance(m_targetSE->GetPosition());
    float missileSpeed = pMissile->GetSelf()->GetAttribute(AttrMaxVelocity).get_float();
    float travelTime = (distance/missileSpeed);
    if (travelTime < 1)
        travelTime = 1;
    pMissile->SetSpeed(missileSpeed);
    pMissile->SetHitTimer(travelTime *1000);
    pMissile->DestinyMgr()->MakeMissile(pMissile);

    // Reduce ammo charge by 1 unit:
    ConsumeCharge();

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
    if (pcount == (pClient->GetChar()->GetSkillLevel(EvESkill::Astrometrics) +1)) {
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
        throw PyException(MakeCustomError("Unable to spawn item #%u:'%s' of type %u.", \
                m_chargeRef->itemID(), m_chargeRef->name(), m_chargeRef->typeID() ) );

    probeRef->SetPosition(pos);
    SystemManager* pSystem = pClient->SystemMgr();
    ProbeSE* pProbe = new ProbeSE(probeRef, *(pSystem->GetServiceMgr()), pSystem, m_modRef, m_shipRef);
    if (pProbe == nullptr)
        return; // make error here

    pProbe->SendNewProbe();
    pSystem->AddEntity(pProbe);
    pClient->scan()->AddProbe(pProbe);

    // Reduce ammo charge by 1 unit:
    ConsumeCharge();

    // add data to StatisticMgr
    sStatMgr.Increment(Stat::probesLaunched);
}

void ActiveModule::LaunchSnowBall()
{

}
