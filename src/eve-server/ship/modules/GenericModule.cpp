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

#include "Client.h"
#include "ship/Ship.h"
#include "ship/modules/GenericModule.h"

GenericModule::GenericModule( InventoryItemRef item, ShipItemRef ship )
{
    m_modRef = item;
    m_shipRef = ship;

    m_ModuleState = MOD_UNFITTED;
    m_ChargeState = MOD_UNLOADED;

    m_repeat = 0;
}

GenericModule::~GenericModule()
{
    m_modRef->PutOffline();
}

void GenericModule::Online()
{
    if (m_ModuleState == MOD_UNFITTED)
        return;  // make error here for online called for unfitted module?  isnt this error printed elsewhere? -nope
    if (m_ModuleState != MOD_OFFLINE)
        return;     // already online

    m_modRef->PutOnline(isRig());
    m_ModuleState = MOD_ONLINE;

    m_shipRef->SetAttribute(AttrCpuLoad, m_shipRef->GetAttribute(AttrCpuLoad) + GetAttribute(AttrCpu));
    m_shipRef->SetAttribute(AttrPowerLoad, m_shipRef->GetAttribute(AttrPowerLoad) + GetAttribute(AttrPower));
}

void GenericModule::Offline()
{
    if (m_ModuleState == MOD_OFFLINE)
        return; // make console note about offline call to offline module?  code trace, maybe?
    if (m_ModuleState == MOD_UNFITTED)
        return;  // make error here for offline called for unfitted module?  isnt this error printed elsewhere?
    if (m_ModuleState == MOD_DEACTIVATING)
        return;     // already deactivating

    m_ModuleState = MOD_DEACTIVATING;
    /* code for offlining module before MOD_OFFLINE state is set. */
    m_shipRef->SetAttribute(AttrCpuLoad, m_shipRef->GetAttribute(AttrCpuLoad) - GetAttribute(AttrCpu));
    m_shipRef->SetAttribute(AttrPowerLoad, m_shipRef->GetAttribute(AttrPowerLoad) - GetAttribute(AttrPower));

    // need to clear item's effectMap here to avoid duplicating.
    // it will be populated on it's next ProcessEffects() call with relevant data.
    m_modRef->m_modifiers.clear();
    ProcessEffects(Effects::dgmStateOnline, false);
    sFxProc.ApplyEffects(m_modRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get());

    m_ModuleState = MOD_OFFLINE;
    m_modRef->PutOffline();

}

void GenericModule::Overload()
{
    // need to clear item's effectMap here to avoid duplicating.
    m_modRef->m_modifiers.clear();
    // it will be populated on it's next ProcessEffects() call with relevant data.
    ProcessEffects(Effects::dgmStateOverloaded, true);
    sFxProc.ApplyEffects(m_modRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get());
}

void GenericModule::DeOverload()
{
    // need to clear item's effectMap here to avoid duplicating.
    m_modRef->m_modifiers.clear();
    // it will be populated on it's next ProcessEffects() call with relevant data.
    ProcessEffects(Effects::dgmStateOverloaded, false);
    sFxProc.ApplyEffects(m_modRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get());
}

void GenericModule::ProcessEffects(uint8 state, bool online/*false*/)
{
    // get module/charge pre/post effects in state x
    std::vector< Effect > effectVec;
    m_modRef->type().GetEffect(state, effectVec);
    for (auto it : effectVec) {
        if (it.id == 16)    // skip the online effect for now.  will hack the data for it later.
            continue;
        fxData data;
        data.result = false;
        data.srcRef = m_modRef;
        data.math = data.targLoc = data.targAttr = data.srcAttr = data.grpID = data.typeID = data.fxSrc = 0;
        /* module effects will be added/removed from module item
         * passive effects will be applied when ship undocks and removed when ship docks
         * active/overload/gang/other effects will be applied and removed when called.
         */
        if (online)
            sFxProc.ParseExpression(m_modRef.get(), sFxDataMgr.GetExpression(it.preExpression), data, this);
        else
            sFxProc.ParseExpression(m_modRef.get(), sFxDataMgr.GetExpression(it.postExpression), data, this);
    }
}
