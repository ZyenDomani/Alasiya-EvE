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
    Author:        AknorJaden
*/

#include "ship/modules/electronics_modules/TractorBeam.h"
#include "system/SystemBubble.h"


TractorBeam::TractorBeam( InventoryItemRef item, ShipRef ship )
: ActiveModule(item, ship)
{

}

void TractorBeam::Activate(SystemEntity* targetEntity)
{
	// Check to make sure target is NOT a static entity:
	// TODO: Check for target = asteroid, ice, or gas cloud then only allow tractoring if ship = Orca
	// TODO: DO NOT allow tractoring of Client-connected player ships
	if (!(targetEntity->IsStaticEntity()))
	{
		if (
		     (
				(m_Ship->typeID() == 28606)		// Orca is the only ship allowed to tractor asteroids and ice chunks
				&&
				(
				((getItem()->typeID() == 16278 || getItem()->typeID() == 22229) && (targetEntity->Item()->groupID() == EVEDB::invGroups::Ice))
				||
				(targetEntity->Item()->categoryID() == EVEDB::invCategories::Asteroid)
				||
				((targetEntity->Item()->groupID() == EVEDB::invGroups::Harvestable_Cloud) && (getItem()->groupID() == EVEDB::invGroups::Gas_Cloud_Harvester))
				)
				)
				||
				(targetEntity->Item()->groupID() == EVEDB::invGroups::Cargo_Container)
				||
				(targetEntity->Item()->groupID() == EVEDB::invGroups::Secure_Cargo_Container)
				||
				(targetEntity->Item()->groupID() == EVEDB::invGroups::Wreck)
			)
		{
			m_targetEntity = targetEntity;
			m_targetID = targetEntity->GetID();

			// Activate active processing component timer:
			m_AMPC->ActivateCycle();
			//_ShowCycle();
			//m_ActiveModuleProc->ProcessActiveCycle();
		}
	}
}

void TractorBeam::StopCycle(bool abort)
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
        "effect.TractorBeam",
        0,
        0,
        0,
        timeLeft,
        0
    );

    // Create Destiny Updates:
    GodmaOther go;
        go.shipID = m_Ship->itemID();
        go.slotID = m_Item->flag();
        go.chargeTypeID = 0;

    GodmaEnvironment ge;
        ge.selfID = m_Item->itemID();
        ge.charID = m_Ship->ownerID();
        ge.shipID = go.shipID;
        ge.targetID = m_targetID;
        ge.other = go.Encode();
        ge.area = new PyList;
        ge.effectID = effectTractorBeam;

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

    PyTuple* tmp2 = multi.Encode();

    m_Ship->GetOperator()->SendDogmaNotification("OnMultiEvent", "clientID", &tmp2);
}

double TractorBeam::DoCycle() {
        if ((!m_Ship->GetOperator()->GetSystemEntity()->Bubble())
            || (!m_Ship->GetOperator()->GetSystemEntity()->Bubble()->GetEntity(m_targetID)) )
        {
            Deactivate();
            return 0;
        }

		_ShowCycle();

		// Initiate continued Destiny Action to move tractored object toward ship
		DynamicSystemEntity * targetEntity = static_cast<DynamicSystemEntity *>(m_targetEntity);
		// Check for distance to target > 2000m + ship radius
		GVector distanceToTarget(m_Ship->position(), targetEntity->GetPosition());
		if (distanceToTarget.length() > (2000.0 + m_Ship->GetAttribute(AttrRadius).get_float())) {
			// Range higher?  Then start it moving toward ship @ 200m/s
			targetEntity->Destiny()->SetMaxVelocity(500.0);
			// Tractor objects at 500m/s:
			targetEntity->Destiny()->TractorBeamFollow(m_Ship->GetOperator()->GetSystemEntity(),
                                                       (2000.0 + m_Ship->GetAttribute(AttrRadius).get_float())
                                                      );
            return _GetDuration();
		} else {
			targetEntity->Destiny()->TractorBeamHalt();
			Deactivate();
		}
}

void TractorBeam::_ShowCycle()
{
    // Create Special Effect:
    m_Ship->GetOperator()->GetDestiny()->SendSpecialEffect
    (
        m_Ship,
        m_Item->itemID(),
        m_Item->typeID(),
        m_targetID,
        0,
        "effect.TractorBeam",
        1,
        1,
        1,
        _GetDuration(),
        1
    );

    // Create Destiny Updates:
    GodmaOther go;
        go.shipID = m_Ship->itemID();
        go.slotID = m_Item->flag();
        go.chargeTypeID = 0;

    GodmaEnvironment ge;
        ge.selfID = m_Item->itemID();
        ge.charID = m_Ship->ownerID();
        ge.shipID = go.shipID;
        ge.targetID = m_targetID;
        ge.other = go.Encode();
        ge.area = new PyList;
        ge.effectID = effectTractorBeam;

    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = 1;
        shipEff.active = 1;
        shipEff.environment = ge.Encode();
        shipEff.startTime = shipEff.timeNow;
        shipEff.duration = _GetDuration();
        shipEff.repeat = 1000;
        shipEff.error = new PyNone;

    std::vector<PyTuple*> events;
    events.push_back(shipEff.Encode());

    std::vector<PyTuple*> updates;

    m_Ship->GetOperator()->GetDestiny()->SendDestinyUpdate(updates, events, false);
}

void TractorBeam::_SetCapNeed()
{
    // this will be needed for modules and rigs that affect cap need for mining modules
    //double need = GetAttribute(AttrCapacitorNeed);

}
