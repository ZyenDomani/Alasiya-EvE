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

MissileLauncher::MissileLauncher( InventoryItemRef item, ShipItemRef ship )
: ActiveModule(item, ship)
{
    Character* pChar = m_shipRef->GetPilot()->GetChar().get();

    // as we hard-set attributes here, we need to make sure they are DEFAULT before adding bonuses.  (error fix)
    m_modRef->ResetAttribute(AttrSpeed);

    m_cycleTime = GetAttribute(AttrSpeed).get_float();
    m_cycleTime *= (1 - ( 0.02 * (pChar->GetSkillLevel(skillMissileLauncherOperation, true)))); //  2% decrease in rof
    m_cycleTime *= (1 - ( 0.03 * (pChar->GetSkillLevel(skillRapidLaunch, true))));              //  3% decrease in rof

    switch (m_modRef->typeID()) {
        case 2404:  //Standard Missile Launcher II
            m_cycleTime *=  (1 - ( 0.02 * (pChar->GetSkillLevel(skillLightMissileSpecialization, true)))); //  2% decrease in rof
            break;
        case 27805: //Standard Missile Launcher III
            m_cycleTime *=  (1 - ( 0.04 * (pChar->GetSkillLevel(skillLightMissileSpecialization, true)))); //  4% decrease in rof
            break;
        case 2410:  //Heavy Missile Launcher II
            m_cycleTime *=  (1 - ( 0.02 * (pChar->GetSkillLevel(skillHeavyMissileSpecialization, true)))); //  2% decrease in rof
            break;
        case 1877:  //Assault Missile Launcher II
            m_cycleTime *=  (1 - ( 0.02 * (pChar->GetSkillLevel(skillHeavyAssaultMissileSpecialization, true)))); //  2% decrease in rof
            break;
        case 19739: //Cruise Missile Launcher II
            m_cycleTime *=  (1 - ( 0.02 * (pChar->GetSkillLevel(skillCruiseMissileSpecialization, true)))); //  2% decrease in rof
            break;
        case 10631: //Rocket Launcher II
            m_cycleTime *=  (1 - ( 0.02 * (pChar->GetSkillLevel(skillRocketSpecialization, true)))); //  2% decrease in rof
            break;
        case 2420:  //Siege Missile Launcher II
            m_cycleTime *=  (1 - ( 0.02 * (pChar->GetSkillLevel(skillTorpedoSpecialization, true)))); //  2% decrease in rof
            break;
    }

    // need to put ship mods for rof here.

    // save adjusted attributes
    m_modRef->SetAttribute(AttrSpeed, m_cycleTime);
}

MissileLauncher::~MissileLauncher()
{
    m_modRef->ResetAttribute(AttrSpeed);
}

void MissileLauncher::Activate(SystemEntity* pSE)
{
    if (m_chargeRef) {
        m_targetEntity = pSE;
        m_targetID = pSE->GetID();
		// Activate active processing component timer:
		ActiveModule::Activate(pSE);
    } else {
        _log(SHIP__MODULE_WARNING, "MissileLauncher::Activate() - Cannot find loaded charge for this module" );
        if (m_shipRef->HasPilot())
            if (m_shipRef->GetPilot()->CanThrow())
                throw PyException( MakeCustomError( "Cannot find loaded charge for this module  - Ref: ServerError 15693" ) );
    }
}

void MissileLauncher::Overload()
{
    ActiveModule::Overload();
    m_cycleTime *= (1 + GetAttribute(AttrOverloadRofBonus).get_float());
    // save adjusted attributes
    m_modRef->SetAttribute(AttrSpeed, m_cycleTime);
}

void MissileLauncher::DeOverload()
{
    ActiveModule::DeOverload();
    m_cycleTime /= (1 + GetAttribute(AttrOverloadRofBonus).get_float());
    // save adjusted attributes
    m_modRef->SetAttribute(AttrSpeed, m_cycleTime);
}

void MissileLauncher::StopCycle(bool abort)
{
    // Create Destiny Updates:
    GodmaOther go;
        go.shipID = m_shipRef->itemID();
        go.slotID = m_modRef->flag();
        if (m_chargeRef)
            go.chargeTypeID = m_chargeRef->typeID();
        else
            go.chargeTypeID = 0;
    GodmaEnvironment ge;
        ge.selfID = m_modRef->itemID();
        ge.charID = m_shipRef->ownerID();
        ge.shipID = go.shipID;
        ge.targetID = m_targetID;
        ge.other = go.Encode();
        ge.area = new PyList;
        ge.effectID = effectUseMissiles;
    uint32 timeLeft = GetRemainingCycleTimeMS();
    timeLeft /= 1000;
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
    m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

double MissileLauncher::DoCycle() {
        if ((!m_shipRef->GetPilot()->GetShipSE()->SysBubble())
            || (!m_shipRef->GetPilot()->GetShipSE()->SysBubble()->GetEntity(m_targetID))
            || (!m_chargeLoaded) || (!m_chargeRef) )
        {
            Deactivate();
            return 0;
        }
        if (!m_chargeRef->quantity()) {
            Deactivate();
            return 0;
        }

        _ShowCycle();
        _LaunchMissile();

        return m_cycleTime;
}

void MissileLauncher::_LaunchMissile()
{
    // Actually Launch a missile, creating a new Destiny object for it
    Character* pChar = m_shipRef->GetPilot()->GetChar().get();
    SystemManager* pSystem = m_shipRef->GetPilot()->SystemMgr();
    ItemData idata(m_chargeRef->typeID(), pChar->itemID(), pChar->locationID(), flagMissile, m_chargeRef->itemName().c_str(), m_shipRef->position() );

    InventoryItemRef missileRef = m_shipRef->GetItemFactory()->SpawnItem(idata);

    if (!missileRef)
        throw PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", \
                            m_chargeRef->itemID(), m_chargeRef->itemName().c_str(), m_chargeRef->typeID() ) );

    pMissile = new Missile(missileRef, *(pSystem->GetServiceMgr()),  pSystem, m_modRef, m_targetEntity, m_shipRef.get());
    double distance = pMissile->GetSelf()->position().distance(m_targetEntity->GetPosition());
    double missileSpeed = pMissile->GetSelf()->GetAttribute(AttrMaxVelocity).get_float();
    missileSpeed *=  (1 + ( 0.1 * (pChar->GetSkillLevel(skillMissileProjection, true))));        // 10% increase in velocity
    double travelTime = (distance/missileSpeed);
    if (travelTime < 1) travelTime = 1;
    pMissile->SetSpeed(missileSpeed);
    pMissile->SetHitTimer(travelTime *1000);
    pMissile->DestinyMgr()->MakeMissile(pMissile);

    // Reduce ammo charge by 1 unit:
    m_chargeRef->SetQuantity(m_chargeRef->quantity() - 1);
}

void MissileLauncher::_ShowCycle()
{
    // Create Special Effect:
    m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect
    (
        m_shipRef,
        m_modRef->itemID(),
        m_modRef->typeID(),
        m_targetID,
        m_chargeRef->typeID(),
        "effects.MissileDeployment",
        true,
        true,
        true,
        m_cycleTime,
        1000
    );

    // Create Destiny Updates:
    GodmaOther go;
        go.shipID = m_shipRef->itemID();
        go.slotID = m_modRef->flag();
        go.chargeTypeID = m_chargeRef->typeID();
    GodmaEnvironment ge;
        ge.selfID = m_modRef->itemID();
        ge.charID = m_shipRef->ownerID();
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
        shipEff.duration = m_cycleTime;
        shipEff.repeat = m_chargeRef->quantity();
        shipEff.error = new PyNone();
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

