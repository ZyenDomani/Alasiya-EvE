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
    Author:        Allan
*/

#include "ship/modules/ewar_modules/Webifier.h"
#include "system/SystemBubble.h"


Webifier::Webifier( InventoryItemRef item, ShipItemRef ship )
:ActiveModule(item, ship)
{

}

void Webifier::Activate(SystemEntity * targetEntity)
{   // only check i can think of right now to verify target is client, npc, or drone
    DynamicSystemEntity* pTarget = static_cast<DynamicSystemEntity *>(m_targetEntity);
    DestinyManager* pDestiny = pTarget->DestinyMgr();
    if (!pDestiny) return;

    m_targetEntity = targetEntity;
    m_targetID = targetEntity->GetID();

	// Activate active processing component timer:
	m_AMPC->ActivateCycle();
	//_ShowCycle();
    //m_ActiveModuleProc->ProcessActiveCycle();

    double multiplier = ((100 + m_Item->GetAttribute(AttrSpeedFactor).get_float()) /100);
    m_originalSpeed = pDestiny->GetMaxVelocity();
    double newSpeed = m_originalSpeed * multiplier;
    pDestiny->SetMaxVelocity(newSpeed);
    pDestiny->SetSpeedFraction();
}

void Webifier::Deactivate()
{
    if ((m_ModuleState != MOD_ACTIVATED) || (m_ModuleState == MOD_OFFLINE)) return;

	m_ModuleState = MOD_DEACTIVATING;
    m_AMPC->DeactivateCycle();

    DynamicSystemEntity* pTarget = static_cast<DynamicSystemEntity *>(m_targetEntity);
    DestinyManager* pDestiny = pTarget->DestinyMgr();
    pDestiny->SetMaxVelocity(m_originalSpeed);
    pDestiny->SetSpeedFraction();
}

void Webifier::StopCycle(bool abort)
{
    double timeLeft = m_AMPC->GetRemainingCycleTimeMS();
    timeLeft /= 100;

    // Create Special Effect:
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect
    (
        m_Ship,
        m_Item->itemID(),
        m_Item->typeID(),
        m_targetID,
        0,
        "effects.decreaseTargetSpeed",
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
        ge.effectID = effectDecreaseTargetSpeed;
    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = 0;
        shipEff.active = 0;
        shipEff.environment = ge.Encode();
        shipEff.startTime = (shipEff.timeNow - (timeLeft * Win32Time_Second));
        shipEff.duration = timeLeft;
        shipEff.repeat = 0;
        shipEff.error = new PyNone;
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

double Webifier::DoCycle()
{
        if ((!m_Ship->GetPilot()->GetShipSE()->SysBubble())
            || (!m_Ship->GetPilot()->GetShipSE()->SysBubble()->GetEntity(m_targetID)) )
        {
            Deactivate();
            return 0;
        }
        _ShowCycle();

        //check for range here..
        //  maxRange, overloadRangeBonus

        return _GetDuration();
}

void Webifier::_ShowCycle()
{
    /*
                  [PyTuple 2 items]
                    [PyInt 13627]
                    [PyTuple 2 items]
                      [PyString "OnSpecialFX"]
                      [PyTuple 14 items]
                        [PyIntegerVar 9000000000000163286]
                        [PyIntegerVar 9000000000000163286]
                        [PyInt 18072]
                        [PyIntegerVar 1006531110340]
                        [PyNone]
                        [PyList 0 items]
                        [PyString "effects.ModifyTargetSpeed"]
                        [PyBool True]
                        [PyInt 0]
                        [PyInt 0]
                        [PyFloat 5000]
                        [PyBool False]
                        [PyIntegerVar 129813154083845616]
                        [PyNone]
                        */
    // Create Special Effect:
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect
    (
        m_Ship,
        m_Item->itemID(),
        m_Item->typeID(),
        m_targetID,
        0,
        "effects.decreaseTargetSpeed",
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
        ge.effectID = effectDecreaseTargetSpeed;
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
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

void Webifier::_SetCapNeed()
{
    // this will be needed for modules and rigs that affect cap need for mining modules
    //double need = GetAttribute(AttrCapacitorNeed);

}

