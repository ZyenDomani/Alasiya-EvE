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
    // incase module item has AttrIsOnline set to true....it shouldn't (IsOnline isnt persistant) but this is a catchall.
    m_modRef->PutOffline();

    ProcessEffects(Effects::dgmStatePassive, true);
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
    ProcessEffects(Effects::dgmStateOnline, true);
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
    // code for offlining module before MOD_OFFLINE state is set.
    ProcessEffects(Effects::dgmStateOnline, false);
    sFxProc.ApplyEffects(m_modRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get());

    m_ModuleState = MOD_OFFLINE;
    m_modRef->PutOffline();
}

void GenericModule::ProcessEffects(uint8 state, bool online/*false*/)
{
    // get module effects in state 0
    std::vector< Effect > effectVec;
    m_modRef->type().GetEffect(state, effectVec);
    for (auto it : effectVec) {
        fxData data;
        data.srcRef = m_modRef;
        data.math = data.targLoc = data.targAttr = data.srcAttr = data.grpID = data.typeID = data.fxSrc = 0;
        /* passive effects are added directly to ship item m_modifers
         * non-passive effects are added/deleted to/from module item and called on their effect target from ???
         * will need remove* methods finished for this to work properly
         */
        if (online)
            sFxProc.ParseExpression((state ? m_modRef.get() : m_shipRef.get()), sFxDataMgr.GetExpression(it.preExpression), data);
        else if (state)
            sFxProc.ParseExpression(m_modRef.get(), sFxDataMgr.GetExpression(it.postExpression), data);
    }
}
