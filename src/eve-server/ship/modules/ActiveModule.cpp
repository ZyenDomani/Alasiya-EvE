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

ActiveModule::ActiveModule(InventoryItemRef item, ShipItemRef ship)
: GenericModule(item, ship),
m_timer(1000),
m_reloadTimer(10000)
{
    m_targetEntity = nullptr;
    /** @todo  bubble isnt ready yet.  will have to update every time we change bubble */
    //m_bubble = ship->GetPilot()->GetShipSE()->SysBubble();
    /** @todo  destiny isnt ready yet.  will have to update when undocking, as this is created before ShipSE is */
    //m_destiny = ship->GetPilot()->GetShipSE()->DestinyMgr();
    m_chargeRef = InventoryItemRef();
    m_overLoaded = false;
    m_chargeLoaded = false;

    if (m_modRef->HasAttribute(AttrMaxRange))
        m_maxRange = GetAttribute(AttrMaxRange).get_int();

    if (m_modRef->HasAttribute(AttrDuration))
        m_cycleTime = GetAttribute(AttrDuration).get_float();

    if (m_modRef->HasAttribute(AttrCapacitorNeed))
        m_capNeed = GetAttribute(AttrCapacitorNeed).get_float();

    // this is an internal variable only.
    m_reloadTime = GetAttribute(AttrReloadTime).get_int();
    /* our db doesnt have reload times for launchers or projectile turrents.
     * set default of 4s for turrents, 5s for snowball and probe launchers, 7s for missile launchers, and 10s for others.
     * maybe make config option later to avoid hard-coding
     */
    if (!m_reloadTime) {
        if (m_modRef->groupID() == EVEDB::invGroups::Projectile_Weapon)
            m_reloadTime = 4000;
        else if (m_modRef->groupID() == EVEDB::invGroups::Missile_Launcher_Snowball
            or m_modRef->groupID() == EVEDB::invGroups::Scan_Probe_Launcher)
            m_reloadTime = 5000;
        else if (m_modRef->groupID() == EVEDB::invGroups::Missile_Launcher_Cruise
            or m_modRef->groupID() == EVEDB::invGroups::Missile_Launcher_Rocket
            or m_modRef->groupID() == EVEDB::invGroups::Missile_Launcher_Siege
            or m_modRef->groupID() == EVEDB::invGroups::Missile_Launcher_Standard
            or m_modRef->groupID() == EVEDB::invGroups::Missile_Launcher_Heavy
            or m_modRef->groupID() == EVEDB::invGroups::Missile_Launcher_Assault
            or m_modRef->groupID() == EVEDB::invGroups::Missile_Launcher_Defender
            or m_modRef->groupID() == EVEDB::invGroups::Missile_Launcher_Citadel
            or m_modRef->groupID() == EVEDB::invGroups::Missile_Launcher_Heavy_Assault
            or m_modRef->groupID() == EVEDB::invGroups::Missile_Launcher_Bomb)
            m_reloadTime = 7000;
        else
            m_reloadTime = 10000;
    }
    m_timer.Disable();
    m_reloadTimer.Disable();

    _log(SHIP__MODULE_TRACE, "Set reload time for %s(%u) to %ums", m_modRef->itemName().c_str(), m_modRef->itemID(), m_reloadTime);
}

void ActiveModule::Process()
{
    // timing and verification function
    //check if we have signal to stop the cycle
    if (m_Stop) {
        //wait for time to run out and send deactivate to client
        if (m_timer.Check(false)) {
            m_timer.Disable();
            DeactivateCycle();
            return;
        }
    }
    //check if the timer expired & subtract time
    if (m_timer.Check())
        ShouldProcessActiveCycle();

    if (m_reloadTimer.Enabled()) {
        if (m_reloadTimer.Check(false)) {
            // charge loading complete
            m_reloadTimer.Disable();
            m_ChargeState = MOD_LOADED;
        }
    }
}

void ActiveModule::Activate(SystemEntity* pSE)
{
    m_targetEntity = pSE;
    m_Stop = false;
    m_ModuleState = MOD_ACTIVATED;  //this HAS to be set before mod::DoCycle()

    /** @todo   these need to check for targetable actions, and apply changes accordingly */

    //active module class has a m_cycleTime variable that holds cycle time,
    // based on character skills and specific module attributes.  -allan 19Dec15
    SetTimer((uint32)DoCycle()); // Do initial cycle immediately while we start timer

    //DoEffect(true);
}

void ActiveModule::Deactivate()
{
    if ((m_ModuleState == MOD_UNFITTED)
        or (m_ModuleState == MOD_OFFLINE)
        or (m_ModuleState == MOD_DEACTIVATING))
        return;

    m_Stop = true;

    //DoEffect(false);
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

void ActiveModule::LoadCharge(InventoryItemRef charge)
{
	m_chargeRef = charge;
    m_chargeLoaded = true;
    m_ChargeState = MOD_RELOADING;
    /*
     * def OnChargeBeingLoadedToModule(self, moduleIDs, chargeTypeID, reloadTime):
     *  {returns}
     *        [PyTuple 3 items]
     *          [PyTuple 1 items]
     *            [PyIntegerVar 1005885547063]  << moduleID
     *          [PyInt 203]                     << chargeTypeID
     *          [PyFloat 10000]                 << reloadTime (ms)
     */
    PyTuple* module = new PyTuple(1);
        module->SetItem(0, new PyInt(m_modRef->itemID()));
    PyTuple* tmp = new PyTuple(3);
        tmp->SetItem(0, module);
        tmp->SetItem(1, new PyInt(charge->typeID()));
        tmp->SetItem(2, new PyInt(m_reloadTime));
    m_shipRef->GetPilot()->SendNotification("OnChargeBeingLoadedToModule", "shipid", &tmp, false); //unsequenced.
    m_reloadTimer.Start(m_reloadTime);

    // process and apply charge effects here
}

void ActiveModule::UnloadCharge()
{
	m_chargeRef = InventoryItemRef();		// Ensure ref is NULL
    m_chargeLoaded = false;
    m_ChargeState = MOD_UNLOADED;

    // remove charge effects here
}

double ActiveModule::DoCycle()
{
    if (m_shipRef->GetPilot()->GetShipSE()->SysBubble()) {
        _ShowCycle();
        //DoEffect();
        return m_cycleTime;
    }
    Deactivate();
    return 0;
}

void ActiveModule::AbortCycle()
{
    if (m_Stop or (GetModuleState() != MOD_ACTIVATED))
        return;
    // Immediately stop active cycle for things such as init warp, target left bubble, or miner deactivated by player:
    m_Stop = true;
    DeactivateCycle(true);
    SetModuleState(MOD_ONLINE);
    m_timer.Disable();
}

void ActiveModule::DoEffect(bool active /*false*/, std::string effect /*""*/)
{
    /** @todo  finish this when time permits.... */
    /** @todo will have to ensure default effect is coded (correctly) for ALL modules */

    std::string effectStr = "effects.";
    effectStr += effect;
    uint32 timeLeft = GetRemainingCycleTimeMS();
    timeLeft /= 1000;

    // Create Special Effect:
    m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect(
        m_shipRef,
        m_modRef->itemID(),
        m_modRef->typeID(),
        m_targetID,
        (m_chargeLoaded ? m_chargeRef->typeID() : 0),
        effectStr,
        0,   /* fixme */
        (active ? 1 : 0),
        (active ? 1 : 0),
        (active ? (Win32TimeNow() + (timeLeft * Win32Time_Second)) : Win32TimeNow()),
        0
    );

    // Create Destiny Updates:
    //  there are slight variations on this.  look into and fix as required.
    GodmaOther go;
        go.shipID = m_shipRef->itemID();
        go.slotID = m_modRef->flag();
        go.chargeTypeID = m_modRef->typeID();
    GodmaEnvironment ge;
        ge.selfID = m_modRef->itemID();
        ge.charID = m_shipRef->ownerID();
        ge.shipID = go.shipID;
        ge.targetID = m_targetID;
        ge.other = go.Encode();
        ge.area = new PyList;   // still dont know what this is.
        ge.effectID = 0;   /* fixme */
    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = (active ? 1 : 0);
        shipEff.active = (active ? 1 : 0);
        shipEff.environment = ge.Encode();
        shipEff.startTime = (active ? shipEff.timeNow : (shipEff.timeNow + (timeLeft * Win32Time_Second)));
        shipEff.duration = (active ? m_cycleTime : timeLeft);
        shipEff.repeat = m_repeat;
        shipEff.error = new PyNone(); /* look into setting this ... only used for salvaging? */
    std::vector<PyTuple*> events;
    events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

void ActiveModule::DeactivateCycle(bool abort/*false*/)
{
    m_ModuleState = MOD_DEACTIVATING;
    bool stacking = false;
    uint32 targetAttrID = 0, sourceAttrID = 0, testID = 0, groupID = m_modRef->groupID();
    StopCycle(abort);
}

void ActiveModule::ShouldProcessActiveCycle() {
    if (m_Stop)
        return;
    if (ShipHasCapCharge())
        ProcessActiveCycle();
    else
        m_Stop = true;
}

void ActiveModule::ProcessActiveCycle()
{
    if (m_Stop)
        return;

    // check if we are targeting another ship or not and apply attribute changes
    //maybe we can have a check for modules that repeat the same attributes so we
    //send the changes just once at activation and at deactivation      --in progress  -allan 19Dec15

    // reset timer here, in the case of cycle time changing for fleet bonuses
    SetTimer((uint32)DoCycle());
}

void ActiveModule::SetTimer(uint32 time) {
    if (!time)
        return;
    _log(SHIP__MODULE_TRACE, "ActiveModule::SetTimer() - Started with %u ms.", time);
    m_timer.Start(time);
}
