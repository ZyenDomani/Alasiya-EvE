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
    Author:     Allan
*/

/*
# Ship Module Logging:
SHIP__MODULE_ERROR=1
# Charge not found reported in SHIP__MODULE_WARNING
SHIP__MODULE_WARNING=1
SHIP__MODULE_MESSAGE=0
# Mod Create/Populate and Undocking "OnlineModules" list dumped in SHIP__MODULE_INFO
SHIP__MODULE_INFO=1
# group tests, set module item online/offline, and Salvaging chance msgs in SHIP__MODULE_DEBUG
SHIP__MODULE_DEBUG=1
# Mod timer setting, Online/Offline calls and Effects msgs reported in SHIP__MODULE_TRACE
SHIP__MODULE_TRACE=1
*/
#include "Client.h"
#include "ship/Ship.h"
#include "ship/modules/GenericModule.h"

using namespace ModStates;
GenericModule::GenericModule( InventoryItemRef item, ShipItemRef ship )
{
    m_modRef = item;
    m_shipRef = ship;

    m_ModuleState = ModuleStates::MOD_UNFITTED;
    m_ChargeState = ChargeStates::CHG_UNLOADED;

    m_repeat = 0;
/*
    sLog.Blue("GenericModule()", "processing effect data for %u (%s)", item->itemID(), item->itemName().c_str());
    std::vector<TypeEffects> typeFx;
    sFxDataMgr.GetTypeEffect(item->itemID(), typeFx);
    Effect fx;
    for (auto cur : typeFx) {
        fx = sFxDataMgr.GetEffect(cur.effectID);
        sLog.Yellow("ConfigureEffects", "starting eval for %u:%s (%s)", fx.effectID, sFxProc.GetStateName(fx.effectState).c_str(), fx.effectName.c_str());
        sFxProc.EvaluateExpression(fx.preExpression);
        sFxProc.EvaluateExpression(fx.postExpression);
    }
*/
    _log(SHIP__MODULE_DEBUG, "Created GenericModule %p for item %s (%u).", this, item->itemName().c_str(), item->itemID());
}

GenericModule::~GenericModule()
{
    Offline();
}

void GenericModule::Online()
{
    if (m_ModuleState == ModuleStates::MOD_UNFITTED) {
        _log(SHIP__MODULE_WARNING, "GenericModule::Online() called for unfitted module %u(%s).",m_modRef->itemID(), m_modRef->itemName().c_str());
        return;
    }
    if (m_ModuleState != ModuleStates::MOD_OFFLINE) {
        _log(SHIP__MODULE_MESSAGE, "GenericModule::Online() called for non-offline module %u(%s).  State is %s", \
                m_modRef->itemID(), m_modRef->itemName().c_str(), GetModuleStateName(m_ModuleState).c_str());
        return;     // already online
    }

    // clear item's effectMap to avoid duplicating.
    m_modRef->ClearModifiers(); // ClearModifiers DELETES AttrIsOnline from the map!!  (elusive error)
    m_modRef->PutOnline(isRig());
    m_ModuleState = ModuleStates::MOD_ONLINE;
    ProcessEffects(Effects::dgmStatePassive, true);
    ProcessEffects(Effects::dgmStateOnline, true);
    sFxProc.ApplyEffects(m_modRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), m_shipRef->GetPilot()->IsInSpace());

    EvilNumber cpuNeed = (m_shipRef->GetAttribute(AttrCpuLoad) + GetAttribute(AttrCpu));
    EvilNumber pgNeed = (m_shipRef->GetAttribute(AttrPowerLoad) + GetAttribute(AttrPower));
    m_shipRef->SetAttribute(AttrCpuLoad, cpuNeed);
    m_shipRef->SetAttribute(AttrPowerLoad, pgNeed);

    _log(SHIP__MODULE_TRACE, "GenericModule::Online() - %u(%s) cpu: %.2f, pg: %.2f",m_modRef->itemID(), m_modRef->itemName().c_str(), cpuNeed.get_float(), pgNeed.get_float());

    if (m_ChargeState == ChargeStates::CHG_LOADED) {
        if (!m_chargeRef) {
            _log(SHIP__MODULE_ERROR, "GenericModule::Online() - module %u(%s) has ChargeState(ChargeStates::CHG_LOADED) but m_chargeRef = NULL.", \
                    m_modRef->itemID(), m_modRef->itemName().c_str());
        } else {
            _log(SHIP__MODULE_ERROR, "GenericModule::Online() - module %u(%s) loading charge %s.", m_modRef->itemID(), m_modRef->itemName().c_str(), m_chargeRef->itemName().c_str());
            m_chargeRef->ClearModifiers();
            for (auto it : m_chargeRef->type().m_stateFxMap) {
                fxData data;
                data.result = false;
                data.srcRef = m_chargeRef;
                data.math = data.targLoc = data.targAttr = data.srcAttr = data.grpID = data.typeID = data.fxSrc = 0;
                sFxProc.ParseExpression(m_chargeRef.get(), sFxDataMgr.GetExpression(it.second.preExpression), data, this);
            }
            if (m_shipRef->GetPilot()->IsInSpace())
                sFxProc.ApplyEffects(m_chargeRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), true);
        }
    }
}

void GenericModule::Offline()
{
    if (m_ModuleState == ModuleStates::MOD_OFFLINE) {
        _log(SHIP__MODULE_WARNING, "GenericModule::Offline() called for offline module %u(%s).",m_modRef->itemID(), m_modRef->itemName().c_str());
        return;
    }
    if (m_ModuleState == ModuleStates::MOD_UNFITTED) {
        _log(SHIP__MODULE_WARNING, "GenericModule::Offline() called for unfitted module %u(%s).",m_modRef->itemID(), m_modRef->itemName().c_str());
        return;
    }
    if (m_ModuleState == ModuleStates::MOD_DEACTIVATING) {
        _log(SHIP__MODULE_MESSAGE, "GenericModule::Offline() called for deactivating module %u(%s).",m_modRef->itemID(), m_modRef->itemName().c_str());
        m_ModuleState = ModuleStates::MOD_OFFLINE;
        m_modRef->PutOffline();
        return;
    }

    m_ModuleState = ModuleStates::MOD_DEACTIVATING;

    /* code for offlining module before MOD_OFFLINE state is set. */
    EvilNumber cpuNeed = (m_shipRef->GetAttribute(AttrCpuLoad) - GetAttribute(AttrCpu));
    EvilNumber pgNeed = (m_shipRef->GetAttribute(AttrPowerLoad) - GetAttribute(AttrPower));
    m_shipRef->SetAttribute(AttrCpuLoad, cpuNeed);
    m_shipRef->SetAttribute(AttrPowerLoad, pgNeed);

    _log(SHIP__MODULE_TRACE, "GenericModule::Offline() - %u(%s) cpu: %.2f, pg: %.2f",m_modRef->itemID(), m_modRef->itemName().c_str(), cpuNeed.get_float(), pgNeed.get_float());

    if (m_ChargeState == ChargeStates::CHG_LOADED) {
        if (!m_chargeRef) {
            _log(SHIP__MODULE_ERROR, "GenericModule::Offline() - module %u(%s) has ChargeState(ChargeStates::CHG_LOADED) but m_chargeRef = NULL.", \
                    m_modRef->itemID(), m_modRef->itemName().c_str());
        } else {
            m_chargeRef->ClearModifiers();
            for (auto it : m_chargeRef->type().m_stateFxMap) {
                fxData data;
                data.result = false;
                data.srcRef = m_chargeRef;
                data.math = data.targLoc = data.targAttr = data.srcAttr = data.grpID = data.typeID = data.fxSrc = 0;
                sFxProc.ParseExpression(m_chargeRef.get(), sFxDataMgr.GetExpression(it.second.postExpression), data, this);
            }
            if (m_shipRef->GetPilot()->IsInSpace())
                sFxProc.ApplyEffects(m_chargeRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), true);
        }
    }

    // clear item's effectMap to avoid duplicating.
    // each effect will need to be applied individually per group (passive, online, active, overloaded)
    m_modRef->ClearModifiers();
    ProcessEffects(Effects::dgmStatePassive, false);
    ProcessEffects(Effects::dgmStateOnline, false);
    sFxProc.ApplyEffects(m_modRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), m_shipRef->GetPilot()->IsInSpace());

    m_ModuleState = ModuleStates::MOD_OFFLINE;
    m_modRef->PutOffline();
    if (m_shipRef->IsDocking())
        m_modRef->SetAttribute(AttrIsOnline, true, true);
}

void GenericModule::Overload()
{
    // need to clear item's effectMap here to avoid duplicating.
    m_modRef->m_modifiers.clear();
    // it will be populated on it's next ProcessEffects() call with relevant data.
    ProcessEffects(Effects::dgmStateOverloaded, true);
    sFxProc.ApplyEffects(m_modRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), true);
}

void GenericModule::DeOverload()
{
    // need to clear item's effectMap here to avoid duplicating.
    m_modRef->m_modifiers.clear();
    // it will be populated on it's next ProcessEffects() call with relevant data.
    ProcessEffects(Effects::dgmStateOverloaded, false);
    sFxProc.ApplyEffects(m_modRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), true);
}

void GenericModule::ProcessEffects(Effects::State state, bool online/*false*/)
{
    // get module/charge pre/post effects in state x
    std::vector< Effect > effectVec;
    m_modRef->type().GetEffect(state, effectVec);
    _log(EFFECTS__TRACE, "GenericModule::ProcessEffects() called for %s. effects: %u, state: %s, online: %s", \
            m_modRef->itemName().c_str(), effectVec.size(), sFxProc.GetStateName(state).c_str(), (online ? "true" : "false"));
    for (auto it : effectVec) {
        if (it.effectID == 16)    // skip the online effect for now.  will hack the data for it later.
            continue;
        fxData data;
        data.result = false;
        data.srcRef = m_modRef;
        data.math = data.targLoc = data.targAttr = data.srcAttr = data.grpID = data.typeID = data.fxSrc = 0;
        /* module and charge effects will be added/removed from it's item
         * active/overload/gang/other effects will be applied and removed when called.
         */
        if (online)
            sFxProc.ParseExpression(m_modRef.get(), sFxDataMgr.GetExpression(it.preExpression), data, this);
        else
            sFxProc.ParseExpression(m_modRef.get(), sFxDataMgr.GetExpression(it.postExpression), data, this);
    }
}

std::string GenericModule::GetChargeStateName(ModStates::ChargeStates state)
{
    switch(state) {
        case ChargeStates::CHG_UNLOADED:     return "Unloaded"; break;
        case ChargeStates::CHG_LOADED:       return "Loaded"; break;
        case ChargeStates::CHG_LOADING:      return "Loading"; break;
        case ChargeStates::CHG_RELOADING:    return "Reloading"; break;
    }
}

std::string GenericModule::GetModuleStateName(ModStates::ModuleStates state)
{
    switch(state) {
        case ModuleStates::MOD_UNFITTED:     return "Unfitted"; break;
        case ModuleStates::MOD_OFFLINE:      return "Offline"; break;
        case ModuleStates::MOD_ONLINE:       return "Online"; break;
        case ModuleStates::MOD_ACTIVATED:    return "Activated"; break;
        case ModuleStates::MOD_DEACTIVATING: return "Deactivating"; break;
    }
}
