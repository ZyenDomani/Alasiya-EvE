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
    Author:        AlTahir(DaVinci)
    Copied from Armor Transporter
*/

#include "ship/modules/hull_modules/HullTransporter.h"
#include "character/Character.h"


HullTransporter::HullTransporter( InventoryItemRef item, ShipRef ship )
: ActiveModule(item, ship)
{

}

void HullTransporter::StopCycle(bool abort)
{
    uint32 timeLeft = m_AMPC->GetRemainingCycleTimeMS();
    timeLeft /= 100;

    // Create Special Effect:
    m_Ship->GetOperator()->GetDestiny()->SendSpecialEffect
    (
        m_Ship,
        m_Item->itemID(),
        m_Item->typeID(),
        m_targetID,
        0,
        "effects.RemoteArmourRepair",
        0,
        0,
        0,
        timeLeft,
        0
    );

    // Create Destiny Updates:
    GodmaEnvironment ge;
        ge.selfID = m_Item->itemID();
        ge.charID = m_Ship->ownerID();
        ge.shipID = m_Ship->itemID();
        ge.targetID = m_targetID;
        ge.other = new PyNone;
        ge.area = new PyList;
        ge.effectID = effectRemoteHullRepair;

    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = 0;
        shipEff.active = 0;
        shipEff.environment = ge.Encode();
        shipEff.startTime = shipEff.timeNow;
        shipEff.duration = timeLeft;
        shipEff.repeat = 0;
        shipEff.error = new PyNone;

    PyList* events = new PyList;
        events->AddItem(shipEff.Encode());

    Notify_OnMultiEvent multi;
        multi.events = events;

    PyTuple* tmp = multi.Encode();

    m_Ship->GetOperator()->SendDogmaNotification("OnMultiEvent", "clientID", &tmp);
}

double HullTransporter::DoCycle()
{
        if (!m_Ship->GetOperator()->GetSystemEntity()->Bubble())
        {
            Deactivate();
            return 0;
        }
        _ShowCycle();

        // Apply repair amount:
        EvilNumber hullDamage = m_targetEntity->Item()->GetAttribute(AttrDamage);
        hullDamage -= m_Item->GetAttribute(AttrStructureDamageAmount);
        if (hullDamage <= 0) {
            hullDamage = 0;
        }
        m_targetEntity->Item()->SetAttribute(AttrDamage, hullDamage);

        return _GetDuration();
}

void HullTransporter::_ShowCycle()
{
    // Create Special Effect:
    m_Ship->GetOperator()->GetDestiny()->SendSpecialEffect
    (
     m_Ship,
     m_Item->itemID(),
     m_Item->typeID(),
     m_targetID,
     0,
     "effects.RemoteArmourRepair",
     0,
     1,
     1,
     _GetDuration(),
     1
    );

    // Create Destiny Updates:

    GodmaEnvironment ge;
    ge.selfID = m_Item->itemID();
    ge.charID = m_Ship->ownerID();
    ge.shipID = m_Ship->itemID();;
    ge.targetID = m_targetID;
    ge.other = new PyNone;
    ge.area = new PyList;
    ge.effectID = effectRemoteHullRepair;

    Notify_OnGodmaShipEffect shipEff;
    shipEff.itemID = ge.selfID;
    shipEff.effectID = ge.effectID;
    shipEff.timeNow = Win32TimeNow();
    shipEff.start = 1;
    shipEff.active = 1;
    shipEff.environment = ge.Encode();
    shipEff.startTime = shipEff.timeNow;
    shipEff.duration = _GetDuration();
    shipEff.repeat = 0;  //# times to repeat (should be ammo qty?)
    shipEff.error = new PyNone;

    std::vector<PyTuple*> events;
    events.push_back(shipEff.Encode());

    std::vector<PyTuple*> updates;

    m_Ship->GetOperator()->GetDestiny()->SendDestinyUpdate(updates, events, false);
}

double HullTransporter::_GetCapNeed()
{
	// This layout does not count the possible fleet bonuses, so it helps to set the cap need just once - when module's being fitted.
	// First off - pulling up the primary data - module's cap need and primary skill level, that will affect the cap need.
	double moduleCapNeed = m_Item->GetAttribute(AttrCapacitorNeed).get_float();

	// Now we do the initial cap need calculations
	double capacitorNeed = moduleCapNeed * (1 - (0.05 * m_Ship->GetOperator()->GetChar()->GetSkillLevel(skillRemoteHullRepairSystems)));

    return capacitorNeed;
}

void HullTransporter::_SetCapNeed()
{
    // this will be needed for modules and rigs that affect cap need for mining modules
    //double need = GetAttribute(AttrCapacitorNeed);

}
