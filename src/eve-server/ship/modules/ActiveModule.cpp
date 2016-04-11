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

ActiveModule::ActiveModule(InventoryItemRef item, ShipRef ship)
: GenericModule(item, ship)
{
    m_ActiveModuleProc = new ActiveModuleProcessingComponent(item, this, ship);
    m_chargeRef = InventoryItemRef();
    m_overLoaded = false;
    m_chargeLoaded = false;
}

ActiveModule::~ActiveModule()
{
    SafeDelete(m_ActiveModuleProc);
}

void ActiveModule::Process()
{
    m_ActiveModuleProc->Process();
}

void ActiveModule::Deactivate()
{
    if ((m_ModuleState != MOD_ACTIVATED) || (m_ModuleState == MOD_OFFLINE) || (m_ModuleState == MOD_UNFITTED)) return;

    m_ModuleState = MOD_DEACTIVATING;
    m_ActiveModuleProc->StopCycle();
}

void ActiveModule::Overload()
{
    GenericModule::Overload();
}

void ActiveModule::DeOverload()
{
    GenericModule::DeOverload();
}

void ActiveModule::Load(InventoryItemRef charge)
{
	m_chargeRef = charge;
    m_chargeLoaded = true;
    m_ChargeState = MOD_LOADED;
}

void ActiveModule::Unload()
{
	m_chargeRef = InventoryItemRef();		// Ensure ref is NULL
    m_chargeLoaded = false;
    m_ChargeState = MOD_UNLOADED;
}

double ActiveModule::DoCycle()
{
    if (m_Ship->GetOperator()->GetSystemEntity()->Bubble()) {
        _ShowCycle();
        return _GetDuration();
    }
    Deactivate();
    return 0;
}

bool ActiveModule::RequiresTarget()
{
    if (m_Effects->HasDefaultEffect())
        return (m_Effects->GetDefaultEffect()->GetIsAssistance() || m_Effects->GetDefaultEffect()->GetIsOffensive());
    else
        return false;
}

void ActiveModule::DoEffect(bool active)
{
    /* common location for all modules that have a visual effect when active */
}
