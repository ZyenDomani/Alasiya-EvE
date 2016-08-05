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
    m_reloadTimer(10000)
{
    m_AMPC = new ActiveModuleProcessingComponent(item, this, ship);
    /** @todo  bubble isnt ready yet.  will have to update everytime we change bubble */
    //m_bubble = ship->GetPilot()->GetShipSE()->SysBubble();
    /** @todo  destiny isnt ready yet.  will have to update when undocking, as this is created before ShipSE is */
    //m_destiny = ship->GetPilot()->GetShipSE()->DestinyMgr();
    m_chargeRef = InventoryItemRef();
    m_overLoaded = false;
    m_chargeLoaded = false;

    m_reloadTime = m_Item->GetAttribute(AttrReloadTime).get_int();
    /* our db doesnt have reload times for launchers or projectile turrents.
     * set default of 4s for turrents, 5s for snowball and probe launchers, 7s for missile launchers, and 10s for others.
     * maybe make config option later to avoid hard-coding
     */
    if (!m_reloadTime) {
        if (m_Item->groupID() == EVEDB::invGroups::Projectile_Weapon)
            m_reloadTime = 4000;
        else if (m_Item->groupID() == EVEDB::invGroups::Missile_Launcher_Snowball
            or m_Item->groupID() == EVEDB::invGroups::Scan_Probe_Launcher)
            m_reloadTime = 5000;
        else if (m_Item->groupID() == EVEDB::invGroups::Missile_Launcher_Cruise
            or m_Item->groupID() == EVEDB::invGroups::Missile_Launcher_Rocket
            or m_Item->groupID() == EVEDB::invGroups::Missile_Launcher_Siege
            or m_Item->groupID() == EVEDB::invGroups::Missile_Launcher_Standard
            or m_Item->groupID() == EVEDB::invGroups::Missile_Launcher_Heavy
            or m_Item->groupID() == EVEDB::invGroups::Missile_Launcher_Assault
            or m_Item->groupID() == EVEDB::invGroups::Missile_Launcher_Defender
            or m_Item->groupID() == EVEDB::invGroups::Missile_Launcher_Citadel
            or m_Item->groupID() == EVEDB::invGroups::Missile_Launcher_Heavy_Assault
            or m_Item->groupID() == EVEDB::invGroups::Missile_Launcher_Bomb)
            m_reloadTime = 7000;
        else
            m_reloadTime = 10000;
    }
    m_reloadTimer.Disable();
    _log(SHIP__MODULE_TRACE, "Set reload time for %s(%u) to %ums", m_Item->itemName().c_str(), m_Item->itemID(), m_reloadTime);
}

ActiveModule::~ActiveModule()
{
    SafeDelete(m_AMPC);
}

void ActiveModule::Process()
{
    m_AMPC->Process();

    if (m_reloadTimer.Enabled()) {
        if (m_reloadTimer.Check(false)){
            m_reloadTimer.Disable();
            // charge loading complete
            m_ChargeState = MOD_LOADED;
        }
    }
}

void ActiveModule::Activate(SystemEntity* pSE)
{
    m_targetEntity = pSE;
    m_AMPC->ActivateCycle();

    //DoEffect(true);
}

void ActiveModule::Deactivate()
{
    if ((m_ModuleState != MOD_ACTIVATED) or (m_ModuleState == MOD_UNFITTED)) return;

    m_ModuleState = MOD_DEACTIVATING;
    m_AMPC->StopCycle();

    //DoEffect(false);
}

    /** @todo  Overload and DeOverload will need to check for running module,
     * and if so, cancel that run, then restart with overloaded settings.
     * if not running, start with overloaded settings.
     */
void ActiveModule::Overload()
{
    GenericModule::Overload();
    m_ModuleState = MOD_OVERLOADED;
}

void ActiveModule::DeOverload()
{
    GenericModule::DeOverload();
    m_ModuleState = MOD_ONLINE;
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
        module->SetItem(0, new PyInt(m_Item->itemID()));
    PyTuple* tmp = new PyTuple(3);
        tmp->SetItem(0, module);
        tmp->SetItem(1, new PyInt(charge->typeID()));
        tmp->SetItem(2, new PyInt(m_reloadTime));
    m_Ship->GetPilot()->SendNotification("OnChargeBeingLoadedToModule", "shipid", &tmp, false); //unsequenced.
    m_reloadTimer.Start(m_reloadTime);
}

void ActiveModule::UnloadCharge()
{
	m_chargeRef = InventoryItemRef();		// Ensure ref is NULL
    m_chargeLoaded = false;
    m_ChargeState = MOD_UNLOADED;
}

double ActiveModule::DoCycle()
{
    if (m_Ship->GetPilot()->GetShipSE()->SysBubble()) {
        _ShowCycle();
        //DoEffect();
        return _GetDuration();
    }
    Deactivate();
    return 0;
}

void ActiveModule::AbortCycle()
{
    m_AMPC->AbortCycle();
}

void ActiveModule::DoEffect(bool active /*false*/, std::string effect /*""*/)
{
    /** @todo  finish this when time permits.... */
    /** @todo will have to ensure default effect is coded (correctly) for ALL modules */

    std::string effectStr = "effects.";
    effectStr += effect;
    uint32 timeLeft = m_AMPC->GetRemainingCycleTimeMS();
    timeLeft /= 100;

    // Create Special Effect:
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect(
        m_Ship,
        m_Item->itemID(),
        m_Item->typeID(),
        m_targetID,
        (m_chargeLoaded?m_chargeRef->typeID():0),
        effectStr,
        m_Effects->GetDefaultEffect()->GetIsOffensive(),
        (active ? 1 : 0),
        (active ? 1 : 0),
        (active ? (Win32TimeNow() + (timeLeft * Win32Time_Second)) : Win32TimeNow()),
        0
    );

    // Create Destiny Updates:
    //  there are slight variations on this.  look into and fix as required.
    GodmaOther go;
        go.shipID = m_Ship->itemID();
        go.slotID = m_Item->flag();
        go.chargeTypeID = m_Item->typeID();
    GodmaEnvironment ge;
        ge.selfID = m_Item->itemID();
        ge.charID = m_Ship->ownerID();
        ge.shipID = go.shipID;
        ge.targetID = m_targetID;
        ge.other = go.Encode();
        ge.area = new PyList;   // still dont know what this is.
        ge.effectID = m_Effects->GetDefaultEffect()->GetEffectID();
    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = (active ? 1 : 0);
        shipEff.active = (active ? 1 : 0);
        shipEff.environment = ge.Encode();
        shipEff.startTime = (active ? shipEff.timeNow : (shipEff.timeNow + (timeLeft * Win32Time_Second)));
        shipEff.duration = (active ? _GetDuration() : timeLeft);
        shipEff.repeat = m_repeat;
        shipEff.error = new PyNone; /* look into setting this ... only used for salvaging? */
    std::vector<PyTuple*> events;
    events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}
