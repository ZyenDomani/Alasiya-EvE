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
*/

#include "ship/modules/propulsion_modules/Afterburner.h"


Afterburner::Afterburner( InventoryItemRef item, ShipRef ship )
: ActiveModule(item, ship)
{

}

void Afterburner::Activate(SystemEntity * targetEntity)
{
    m_AMPC->ActivateCycle();
    double maxSpeed = m_Ship->GetAttribute(AttrMaxVelocity).get_float();

    // Tell Destiny Manager about our updated speed so it properly tracks ship movement:
    m_Ship->GetOperator()->GetDestiny()->SetMaxVelocity(maxSpeed);
    m_Ship->GetOperator()->GetDestiny()->SetSpeedFraction(1.0);

    DoDestiny_SetMaxSpeed ms;
        ms.entityID = m_Ship->itemID();
        ms.speedValue = maxSpeed;
    PyTuple *tmp = ms.Encode();
    m_Ship->GetOperator()->GetDestiny()->SendSingleDestinyUpdate(&tmp);    //consumed
}

void Afterburner::Deactivate()
{
    ActiveModule::Deactivate();
    double maxSpeed = m_Ship->GetAttribute(AttrMaxVelocity).get_float();

    // Tell Destiny Manager about our updated speed so it properly tracks ship movement:
    m_Ship->GetOperator()->GetDestiny()->SetMaxVelocity(maxSpeed);
    m_Ship->GetOperator()->GetDestiny()->SetSpeedFraction(1.0);

    DoDestiny_SetMaxSpeed ms;
        ms.entityID = m_Ship->itemID();
        ms.speedValue = maxSpeed;
    PyTuple *tmp = ms.Encode();
    m_Ship->GetOperator()->GetDestiny()->SendSingleDestinyUpdate(&tmp);    //consumed
}

void Afterburner::StopCycle(bool abort)
{
	// Tell Destiny Manager about our new speed so it properly tracks ship movement:
    //TODO  get ship speed from destiny, using skill updates.
    m_Ship->GetOperator()->GetDestiny()->SetMaxVelocity(m_shipSpeed);
	m_Ship->GetOperator()->GetDestiny()->SetSpeedFraction();    //reset speed variables, update destiny, update client

    DoDestiny_SetMaxSpeed ms;
        ms.entityID = m_Ship->itemID();
        ms.speedValue = m_shipSpeed;
    PyTuple *tmp = ms.Encode();
    m_Ship->GetOperator()->GetDestiny()->SendSingleDestinyUpdate(&tmp);    //consumed

    std::string effectString = " ";
    uint32 effectID = 0;
    switch (m_Item->groupID())
    {
        case EVEDB::invGroups::Afterburner: {
            effectString = "effects.SpeedBoostMassAddition";
            effectID = effectSpeedBoostMassAddition;
        } break;

        case EVEDB::invGroups::Microwarpdrive: {
            effectString = "effects.SpeedBoostMassSigRad";
            effectID = effectSpeedBoostMassSigRad;
        } break;
    }

    uint32 timeLeft = m_AMPC->GetRemainingCycleTimeMS();
    timeLeft /= 100;

    // Create Special Effect:
    m_Ship->GetOperator()->GetDestiny()->SendSpecialEffect
    (
        m_Ship,
        m_Item->itemID(),
        m_Item->typeID(),
        0,
        0,
        effectString,
        0,
        0,
        0,
        timeLeft,
        0
     );

    GodmaEnvironment ge;
        ge.selfID = m_Item->itemID();
        ge.charID = m_Ship->ownerID();
        ge.shipID = m_Ship->itemID();
        ge.targetID = 0;
        ge.other = new PyNone;
        ge.area = new PyList;
        ge.effectID = effectID;

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


void Afterburner::_ShowCycle()
{
    std::string effectString = " ";
    uint32 effectID = 0;
    switch (m_Item->groupID())
    {
        case EVEDB::invGroups::Afterburner: {
            effectString = "effects.SpeedBoostMassAddition";
            effectID = effectSpeedBoostMassAddition;
        } break;

        case EVEDB::invGroups::Microwarpdrive: {
            effectString = "effects.SpeedBoostMassSigRad";
            effectID = effectSpeedBoostMassSigRad;
        } break;
    }

    // Create Special Effect:
    m_Ship->GetOperator()->GetDestiny()->SendSpecialEffect
    (
        m_Ship,
        m_Item->itemID(),
        m_Item->typeID(),
        0,
        0,
        effectString,
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
        ge.shipID = m_Ship->itemID();
        ge.targetID = 0;
        ge.other = new PyNone;
        ge.area = new PyList;
        ge.effectID = effectID;

    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = 1;
        shipEff.active = 1;
        shipEff.environment = ge.Encode();
        shipEff.startTime = shipEff.timeNow;
        shipEff.duration = _GetDuration();
        shipEff.repeat = 1000;  //# times to repeat (should be ammo qty?)
        shipEff.error = new PyNone;

    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());

    std::vector<PyTuple*> updates;

    m_Ship->GetOperator()->GetDestiny()->SendDestinyUpdate(updates, events, false);
}

void Afterburner::_SetCapNeed()
{
    // this will be needed for modules and rigs that affect cap need for mining modules
    //double need = GetAttribute(AttrCapacitorNeed);

}
