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


Afterburner::Afterburner( InventoryItemRef item, ShipItemRef ship )
: ActiveModule(item, ship)
{
}

/* speed modifier ....
 * base speed * (1 + ( base boost of AB/MWD * ( thrust / (ship mass + module mass))))
 * thrust is defined in module attrib 567 and listed in effectID 710
 * module mass is the mass of any plate and/or ab/mwd modules fitted to ship
 */
void Afterburner::Activate(SystemEntity* pSE)
{
    ActiveModule::Activate(pSE);
    DestinyManager* pDestiny = m_shipRef->GetPilot()->GetShipSE()->DestinyMgr();
    if (!pDestiny) return;  // make error msg here?

    m_shipSpeed = pDestiny->GetMaxVelocity();
    double maxSpeed = m_shipRef->GetAttribute(AttrMaxVelocity).get_float();

    // Tell Destiny Manager about our updated speed so it properly tracks ship movement:
    pDestiny->SetMaxVelocity(maxSpeed);
    pDestiny->SetSpeedFraction(1.0);

    DoDestiny_SetMaxSpeed ms;
        ms.entityID = m_shipRef->itemID();
        ms.speedValue = maxSpeed;
    PyTuple *tmp = ms.Encode();
    pDestiny->SendSingleDestinyUpdate(&tmp);    //consumed
}

void Afterburner::Deactivate()
{
    if (m_ModuleState != MOD_ACTIVATED)
        return;
    ActiveModule::Deactivate();

    double maxSpeed = m_shipRef->GetAttribute(AttrMaxVelocity).get_float();

    DestinyManager* pDestiny = m_shipRef->GetPilot()->GetShipSE()->DestinyMgr();
    if (!pDestiny) return;  // make error msg here?

    // Tell Destiny Manager about our updated speed so it properly tracks ship movement:
    pDestiny->SetMaxVelocity(maxSpeed);
    pDestiny->SetSpeedFraction(1.0);

    DoDestiny_SetMaxSpeed ms;
        ms.entityID = m_shipRef->itemID();
        ms.speedValue = maxSpeed;
    PyTuple *tmp = ms.Encode();
    pDestiny->SendSingleDestinyUpdate(&tmp);    //consumed
}

void Afterburner::StopCycle(bool abort)
{
    DestinyManager* pDestiny = m_shipRef->GetPilot()->GetShipSE()->DestinyMgr();
    if (!pDestiny) return;  // make error msg here?
	// Tell Destiny Manager about our new speed so it properly tracks ship movement:
    //TODO  get ship speed from destiny, using skill updates.
    pDestiny->SetMaxVelocity(m_shipSpeed);
    pDestiny->SetSpeedFraction();    //reset speed variables, update destiny, update client

    DoDestiny_SetMaxSpeed ms;
        ms.entityID = m_shipRef->itemID();
        ms.speedValue = m_shipSpeed;
    PyTuple *tmp = ms.Encode();
    pDestiny->SendSingleDestinyUpdate(&tmp);    //consumed

    std::string effectString = " ";
    uint32 effectID = 0;
    switch (m_modRef->groupID())
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

    uint32 timeLeft = GetRemainingCycleTimeMS();
    timeLeft /= 1000;

    // Create Special Effect:
    pDestiny->SendSpecialEffect
    (
        m_shipRef,
        m_modRef->itemID(),
        m_modRef->typeID(),
        0,
        0,
        effectString,
        0,
        0,
        0,
        timeLeft,
        0
     );

    GodmaOther go;
        go.shipID = m_shipRef->itemID();
        go.slotID = m_modRef->flag();
        go.chargeTypeID = 0;
    GodmaEnvironment ge;
        ge.selfID = m_modRef->itemID();
        ge.charID = m_shipRef->ownerID();
        ge.shipID = go.shipID;
        ge.targetID = m_targetID;
        ge.other = go.Encode();
        ge.area = new PyList;
        ge.effectID = effectID;
    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = 0;
        shipEff.active = 0;
        shipEff.environment = ge.Encode();
        shipEff.startTime = (shipEff.timeNow + (timeLeft * Win32Time_Second));
        shipEff.duration = timeLeft;
        shipEff.repeat = 0;
        shipEff.error = new PyNone();
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    pDestiny->SendDestinyUpdate(updates, events, false);
}


void Afterburner::_ShowCycle()
{
    std::string effectString = " ";
    uint32 effectID = 0;
    switch (m_modRef->groupID())
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
    m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect
    (
        m_shipRef,
        m_modRef->itemID(),
        m_modRef->typeID(),
        0,
        0,
        effectString,
        0,
        1,
        1,
        m_cycleTime,
        1
     );

    // Create Destiny Updates:
    GodmaOther go;
        go.shipID = m_shipRef->itemID();
        go.slotID = m_modRef->flag();
        go.chargeTypeID = 0;
    GodmaEnvironment ge;
        ge.selfID = m_modRef->itemID();
        ge.charID = m_shipRef->ownerID();
        ge.shipID = go.shipID;
        ge.targetID = m_targetID;
        ge.other = go.Encode();
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
        shipEff.duration = m_cycleTime;
        shipEff.repeat = m_repeat;
        shipEff.error = new PyNone();
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

void Afterburner::_SetCapNeed()
{
    // this will be needed for modules and rigs that affect cap need for mining modules
    //double need = GetAttribute(AttrCapacitorNeed);

}
