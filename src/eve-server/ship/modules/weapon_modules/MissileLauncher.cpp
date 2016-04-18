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
    Updates:    Allan
*/

#include "Client.h"
#include "character/Character.h"
#include "ship/Missile.h"
#include "ship/modules/weapon_modules/MissileLauncher.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"

MissileLauncher::MissileLauncher( InventoryItemRef item, ShipRef ship )
: ActiveModule(item, ship)
{
    Character* pChar = m_Ship->GetOperator()->GetChar().get();
    m_ROF *= (1 - ( 0.02 * (pChar->GetSkillLevel(skillMissileLauncherOperation, true)))); //  2% decrease in rof
    m_ROF *= (1 - ( 0.03 * (pChar->GetSkillLevel(skillRapidLaunch, true))));              //  3% decrease in rof

    switch (m_Item->typeID()) {
        case 2404:  //Standard Missile Launcher II
            m_ROF *=  (1 - ( 0.02 * (pChar->GetSkillLevel(skillLightMissileSpecialization, true)))); //  2% decrease in rof
            break;
        case 27805: //Standard Missile Launcher III
            m_ROF *=  (1 - ( 0.04 * (pChar->GetSkillLevel(skillLightMissileSpecialization, true)))); //  4% decrease in rof
            break;
        case 2410:  //Heavy Missile Launcher II
            m_ROF *=  (1 - ( 0.02 * (pChar->GetSkillLevel(skillHeavyMissileSpecialization, true)))); //  2% decrease in rof
            break;
        case 1877:  //Assault Missile Launcher II
            m_ROF *=  (1 - ( 0.02 * (pChar->GetSkillLevel(skillHeavyAssaultMissileSpecialization, true)))); //  2% decrease in rof
            break;
        case 19739: //Cruise Missile Launcher II
            m_ROF *=  (1 - ( 0.02 * (pChar->GetSkillLevel(skillCruiseMissileSpecialization, true)))); //  2% decrease in rof
            break;
        case 10631: //Rocket Launcher II
            m_ROF *=  (1 - ( 0.02 * (pChar->GetSkillLevel(skillRocketSpecialization, true)))); //  2% decrease in rof
            break;
        case 2420:  //Siege Missile Launcher II
            m_ROF *=  (1 - ( 0.02 * (pChar->GetSkillLevel(skillTorpedoSpecialization, true)))); //  2% decrease in rof
            break;
    }

    // need to put ship mods for rof here.
}

void MissileLauncher::Activate(SystemEntity * targetEntity)
{
    if (m_chargeRef) {
		m_targetEntity = targetEntity;
		m_targetID = targetEntity->GetID();

		// Activate active processing component timer:
		m_AMPC->ActivateCycle();
	} else {
        _log(SHIP__MODULE_ERROR,  "MissileLauncher::Activate() - Cannot find charge that is supposed to be loaded into this module!" );
		throw PyException( MakeCustomError( "ERROR!  Cannot find charge that is supposed to be loaded into this module!" ) );
    }
}

void MissileLauncher::Overload()
{
    //ActiveModule::Overload();
    m_ROF *= (1 + m_Item->GetAttribute(AttrOverloadRofBonus).get_float());
}

void MissileLauncher::DeOverload()
{
    //ActiveModule::DeOverload();
    m_ROF /= (1 + m_Item->GetAttribute(AttrOverloadRofBonus).get_float());
}

void MissileLauncher::StopCycle(bool abort)
{
    // Create Destiny Updates:
    GodmaOther go;
        go.shipID = m_Ship->itemID();
        go.slotID = m_Item->flag();
        if (m_chargeRef)
            go.chargeTypeID = m_chargeRef->typeID();
        else
            go.chargeTypeID = 0;

    GodmaEnvironment ge;
        ge.selfID = m_Item->itemID();
        ge.charID = m_Ship->ownerID();
        ge.shipID = go.shipID;
        ge.targetID = m_targetID;
        ge.other = go.Encode();
        ge.area = new PyList;
        ge.effectID = effectUseMissiles;

    uint32 timeLeft = m_AMPC->GetRemainingCycleTimeMS();
    timeLeft /= 100;

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

double MissileLauncher::DoCycle() {
        if ((!m_Ship->GetOperator()->GetSystemEntity()->Bubble())
            || (!m_Ship->GetOperator()->GetSystemEntity()->Bubble()->GetEntity(m_targetID))
            || (!m_chargeLoaded) || (!m_chargeRef) )
        {
            Deactivate();
            return 0;
        }
        if (!m_chargeRef->quantity()) {
            Deactivate();
            return 0;
        }

        _LaunchMissile();
        _ShowCycle();

        return _GetROF();
}

void MissileLauncher::_LaunchMissile()
{
    // Actually Launch a missile, creating a new Destiny object for it
    Character* pChar = m_Ship->GetOperator()->GetChar().get();
    SystemManager* pSystem = m_Ship->GetOperator()->GetClient()->System();
    ItemData idata(m_chargeRef->typeID(), pChar->itemID(), pChar->locationID(), flagMissile, m_chargeRef->itemName().c_str(), m_Ship->position() );

    InventoryItemRef missileRef = pSystem->GetServiceMgr()->item_factory->SpawnItem(idata);

    if (!missileRef)
        throw PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", \
                            m_chargeRef->itemID(), m_chargeRef->itemName().c_str(), m_chargeRef->typeID() ) );

    Missile* pMissileObj = new Missile( pSystem, *(pSystem->GetServiceMgr()), missileRef, m_Item, m_targetEntity, m_Ship.get(), m_Ship->position() );

    pMissile = pMissileObj;

    double distance = pMissile->Item()->position().distance(m_targetEntity->GetPosition());
    double missileSpeed = pMissile->Item()->GetAttribute(AttrMaxVelocity).get_float();
    missileSpeed *=  (1 + ( 0.1 * (pChar->GetSkillLevel(skillMissileProjection, true))));        // 10% increase in velocity
    double travelTime = (distance/missileSpeed);
    pMissile->SetSpeed(missileSpeed);
    pMissile->Destiny()->MakeMissile(pMissile);
    pMissile->SetHitTimer(travelTime *1000);

    // Reduce ammo charge by 1 unit:
    m_chargeRef->SetQuantity(m_chargeRef->quantity() - 1);
}

void MissileLauncher::_ShowCycle()
{
    // Create Special Effect:
    pMissile->Destiny()->SendSpecialEffect
    (
        m_Ship,
        m_Item->itemID(),
        m_Item->typeID(),
        m_targetID,
        m_chargeRef->typeID(),
        "effects.MissileDeployment",
        1,
        1,
        1,
        _GetROF(),
        1
    );

    // Create Destiny Updates:
    GodmaOther go;
        go.shipID = m_Ship->itemID();
        go.slotID = m_Item->flag();
        go.chargeTypeID = m_chargeRef->typeID();

    GodmaEnvironment ge;
        ge.selfID = m_Item->itemID();
        ge.charID = m_Ship->ownerID();
        ge.shipID = go.shipID;
        ge.targetID = m_targetID;
        ge.other = go.Encode();
        ge.area = new PyList;
        ge.effectID = effectUseMissiles;

    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = 1;
        shipEff.active = 1;
        shipEff.environment = ge.Encode();
        shipEff.startTime = shipEff.timeNow;
        shipEff.duration = _GetROF();
        shipEff.repeat = m_chargeRef->quantity();  //# times to repeat (should be ammo qty?)
        shipEff.error = new PyNone;

    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());

    std::vector<PyTuple*> updates;

    pMissile->Destiny()->SendDestinyUpdate(updates, events, false);
}

double MissileLauncher::_GetROF() {
    return m_ROF;
}

void MissileLauncher::_SetCapNeed()
{
    // this will be needed for modules and rigs that affect cap need for mining modules
    //double need = GetAttribute(AttrCapacitorNeed);

}
