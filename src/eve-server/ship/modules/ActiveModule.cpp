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
    Author:        Luck
    Updates:    Allan
*/


#include "eve-server.h"

#include "ship/modules/ActiveModule.h"
#include "system/SystemManager.h"

using namespace ModStates;

ActiveModule::ActiveModule(InventoryItemRef item, ShipItemRef ship)
: GenericModule(item, ship),
m_timer(1000, true),    // this needs to be accurate
m_reloadTimer(10000)
{
    m_repeat = 1000;
    m_targetID = 0;
    m_effectID = 0;
    m_guidStr = "";
    m_targetEntity = nullptr;
    /** @todo  bubble isnt ready yet.  will have to update every time we change bubble */
    //m_bubble = ship->GetPilot()->GetShipSE()->SysBubble();
    m_chargeRef = InventoryItemRef();
    m_overLoaded = false;
    m_needsCharge = item->HasAttribute(AttrChargeGroup1);
    if (m_needsCharge) {
        switch (item->groupID()) {
            case EVEDB::invGroups::Remote_Sensor_Damper:
            case EVEDB::invGroups::Tracking_Link:
            case EVEDB::invGroups::Signal_Amplifier:
            case EVEDB::invGroups::Tracking_Enhancer:
            case EVEDB::invGroups::Sensor_Booster:
            case EVEDB::invGroups::Tracking_Computer:
            case EVEDB::invGroups::Projected_ECCM:
            case EVEDB::invGroups::Remote_Sensor_Booster:
            case EVEDB::invGroups::Tracking_Disruptor: {
                m_needsCharge = false;
            }
        }
    }

    m_chargeLoaded = false;

    if (item->HasAttribute(AttrMaxRange))
        m_maxRange = GetAttribute(AttrMaxRange).get_int();

    if (item->HasAttribute(AttrCapacitorNeed))
        m_capNeed = GetAttribute(AttrCapacitorNeed).get_float();

    // this is an internal variable only.
    m_reloadTime = GetAttribute(AttrReloadTime).get_int();
    /* our db doesnt have reload times for launchers or projectile turrents.
     * set default of 4s for turrents, 5s for snowball and probe launchers, 7s for missile launchers, and 10s for others.
     * maybe make config option later to avoid hard-coding
     */
    if ((!m_reloadTime) and m_needsCharge)  {
        switch (item->groupID()) {
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
    m_timer.Disable();
    m_reloadTimer.Disable();

    if ((m_reloadTime > 0) and (m_reloadTime < 9000))
        _log(SHIP__MODULE_TRACE, "Reload time for %s(%u) set to %ums", item->itemName().c_str(), item->itemID(), m_reloadTime);
}

void ActiveModule::Process()
{
    // timing and verification function
    //check if we have signal to stop the cycle
    if ((m_Stop) and (m_ModuleState != ModuleStates::MOD_ONLINE)) {
        //wait for time to run out and send deactivate to client
        if (m_timer.Check(false)) {
            m_timer.Disable();
            DeactivateCycle();
        }
        // we have stop signal....dont process any further
        return;
    }

    // the order of these next two is significant for reloading modules...
    if (m_needsCharge) {
        if (m_reloadTimer.Enabled()) {
            if (m_reloadTimer.Check(false)) {
                // charge loading complete
                m_reloadTimer.Disable();
                m_ChargeState = ChargeStates::CHG_LOADED;
                // apply charge effects here after "loading" is complete
                sFxProc.ApplyEffects(m_chargeRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), true);
            }
        }
        if (!m_chargeLoaded)
            return;
    }

    if (m_timer.Check())
        ProcessActiveCycle();
}

void ActiveModule::Activate(uint16 effectID, uint32 targetID/*0*/, int16 repeat/*0*/)
{
    if ((m_needsCharge) and ((!m_chargeLoaded) or (!m_chargeRef))) {
        _log(SHIP__MODULE_WARNING, "ActiveModule::Activate() - Cannot find loaded charge for this module");
        if (m_shipRef->HasPilot())
            if (m_shipRef->GetPilot()->CanThrow())
                throw PyException( MakeCustomError( "Cannot find loaded charge for this module  - Ref: ServerError 15693"));
        return;
    }
    if (targetID) {
        m_targetID = targetID;
        m_targetEntity = m_shipRef->GetPilot()->SystemMgr()->GetSE(targetID);
        if (!m_targetEntity) {
            sLog.Error("ActiveModule::Activate()", "m_targetEntity == NULL");
            m_shipRef->GetPilot()->SendErrorMsg("Current target was not found.  Ref: ServerError 25263");
            return;
        }
    }
    m_Stop = false;
    m_repeat = repeat;
    m_effectID = effectID;
    m_guidStr = sFxDataMgr.GetEffectGuid(effectID);
    m_destiny = m_shipRef->GetPilot()->GetShipSE()->DestinyMgr();

    if (!CanActivate())
        return;

    SetTimer(DoCycle()); // Do initial cycle immediately while we start timer

    if (!m_timer.Enabled())
        return;

    ApplyEffect(Effects::dgmStateActive, true);
    ShowEffect(true, false);

    m_ModuleState = ModuleStates::MOD_ACTIVATED;

    if (!m_repeat)
        m_Stop = true;
}

void ActiveModule::Deactivate(std::string effect/*""*/)
{
    if (m_ModuleState != ModuleStates::MOD_ACTIVATED)
        return;

    _log(SHIP__MODULE_TRACE, "ActiveModule::Deactivate() - module %u(%s) remaining time %ums.", \
            m_modRef->itemID(), m_modRef->itemName().c_str(), GetRemainingCycleTimeMS());

    if ((m_effectID == EVEEffectID::miningLaser) or (m_effectID == EVEEffectID::miningClouds)) {
        AbortCycle();
        return;
    }

    m_Stop = true;

    SetModuleState(ModuleStates::MOD_DEACTIVATING);
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

// yes, the xCycle() shit below seems overkill, but each has a specific purpose
uint32 ActiveModule::DoCycle()
{
    if ((!m_destiny) or (!m_shipRef->GetPilot()->GetShipSE()->SysBubble())) {
        // make error for no destiny/bubble
        Deactivate();
        return 0;
    }
    if (m_chargeLoaded) {
        if (!m_chargeRef) {
            // send error to client?
            Deactivate();
            return 0;
        }
        if (m_needsCharge)  {
            uint16 amount = m_chargeRef->GetAttribute(AttrQuantity).get_int();
            m_chargeRef->SetAttribute(AttrQuantity, --amount);
        }
    }

    EvilNumber cycleTime = 0;
    if (m_modRef->HasAttribute(AttrDuration, cycleTime))
        return cycleTime.get_int();
    else if (m_modRef->HasAttribute(AttrSpeed, cycleTime))
        return cycleTime.get_int();
    else
        ; // make error for no duration attribute
}

void ActiveModule::AbortCycle()
{
    if (m_Stop)
        return;
    // Immediately stop active cycle for things such as insufficient cap, remove module, init warp, target left bubble, or miner deactivated by player:
    m_Stop = true;
    DeactivateCycle(true);
    m_timer.Disable();
}

void ActiveModule::DeactivateCycle(bool abort/*false*/)
{
    if (m_ModuleState != ModuleStates::MOD_DEACTIVATING)
        return;

    m_repeat = 0;

    ApplyEffect(Effects::dgmStateActive, false);
    ShowEffect(false, abort);

    SetModuleState(ModuleStates::MOD_ONLINE);

    m_targetID = 0;
    m_targetEntity = nullptr;
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

void ActiveModule::LoadCharge(InventoryItemRef charge)
{
    m_chargeRef = charge;
    m_chargeLoaded = true;
    m_ChargeState = ChargeStates::CHG_LOADING;
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
            tmp->SetItem(1, new PyInt(charge->typeID()));
            tmp->SetItem(2, new PyInt(m_reloadTime));
        m_shipRef->GetPilot()->SendNotification("OnChargeBeingLoadedToModule", "shipid", &tmp, false); //unsequenced.
        m_reloadTimer.Start(m_reloadTime);
    }
    // process new charge's effects here
    charge->ClearModifiers();
    for (auto it : charge->type().m_stateFxMap) {
        fxData data;
        data.action = Effects::Action::dgmActInvalid;
        data.srcRef = charge;
        data.math = data.targLoc = data.targAttr = data.srcAttr = data.grpID = data.typeID = data.fxSrc = 0;
        sFxProc.ParseExpression(charge.get(), sFxDataMgr.GetExpression(it.second.preExpression), data, this);
    }
    if (m_shipRef->GetPilot()->IsLogin()) {
        m_ChargeState = ChargeStates::CHG_LOADED;
        sFxProc.ApplyEffects(charge.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), m_shipRef->GetPilot()->IsInSpace());
    }
}

void ActiveModule::UnloadCharge()
{
    // remove charge effects here
    m_chargeRef->ClearModifiers();
    for (auto it : m_chargeRef->type().m_stateFxMap) {
        fxData data;
        data.action = Effects::Action::dgmActInvalid;
        data.srcRef = m_chargeRef;
        data.math = data.targLoc = data.targAttr = data.srcAttr = data.grpID = data.typeID = data.fxSrc = 0;
        sFxProc.ParseExpression(m_chargeRef.get(), sFxDataMgr.GetExpression(it.second.postExpression), data, this);
    }
    sFxProc.ApplyEffects(m_chargeRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), m_shipRef->GetPilot()->IsInSpace());

    m_chargeRef = InventoryItemRef();       // Ensure ref is NULL
    m_chargeLoaded = false;
    m_ChargeState = ChargeStates::CHG_UNLOADED;
}

void ActiveModule::ApplyEffect(Effects::State state, bool active/*false*/)
{
    // process and apply module's active effects
    m_modRef->m_modifiers.clear();
    ProcessEffects(state, active);
    sFxProc.ApplyEffects(m_modRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), true);
}

void ActiveModule::ShowEffect(bool active, bool abort /*""*/)
{
    if (!m_shipRef->GetPilot()->GetShipSE()->SysBubble())
        return;

    int64 abortTime = Win32TimeNow();
    if (abort) {
        active = false;
        if ((m_effectID == EVEEffectID::miningLaser) or (m_effectID == EVEEffectID::miningClouds))
            abortTime += (3 * Win32Time_Second);    // delay mining abort for 3s to simulate module "completing" its' cycle and dumping ore to cargo
    }

    uint16 chgTypeID = (m_chargeLoaded ? m_chargeRef->typeID() : 0);
    uint32 timeLeft = GetRemainingCycleTimeMS();

    if ((m_targetID) and (m_destiny))
        m_destiny->SendSpecialEffect(
                m_shipRef->itemID(),
                m_modRef->itemID(),
                m_modRef->typeID(),
                m_targetID,
                chgTypeID,
                m_guidStr,
                sFxDataMgr.isOffensive(m_effectID),
                (active ? true : false),   // start    - if (start = 0) THEN remove effect
                (active ? true : false),   // active   - if (start and active) THEN starting ONE-SHOT event of (duration)  (dunno what 'ONE-SHOT event' is)
                (double)timeLeft,           // duration
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
        shipEff.duration = (abort ? 1000 : (active ? GetAttribute(AttrDuration).get_float() : timeLeft));
        shipEff.repeat = m_repeat;
        shipEff.error = new PyNone(); /* look into setting this ... only used for salvaging? */
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    if (m_destiny)
        m_destiny->SendDestinyUpdate(updates, events, false);
    else
        m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}
