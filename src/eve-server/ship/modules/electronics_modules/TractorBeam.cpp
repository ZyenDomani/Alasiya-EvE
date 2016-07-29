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

#include "ship/modules/electronics_modules/TractorBeam.h"
#include "system/SystemBubble.h"


TractorBeam::TractorBeam( InventoryItemRef item, ShipItemRef ship )
: ActiveModule(item, ship)
{

}
/*
        [PySubStream 156 bytes]
          [PyObjectEx Normal]
            [PyTuple 3 items]
              [PyToken ccp_exceptions.UserError]
              [PyTuple 2 items]
                [PyString "TargetTooFar"]
                [PyDict 3 kvp]
                  [PyString "distance"]
                  [PyString "20,000"]
                  [PyString "modulename"]
                  [PyTuple 2 items]
                    [PyInt 4]
                    [PyInt 24348]
                  [PyString "targetname"]
                  [PyString "Aknor Jaden's Myrmidon Wreck"]
              [PyDict 2 kvp]
                [PyString "msg"]
                [PyString "TargetTooFar"]
                [PyString "dict"]
                [PyDict 3 kvp]
                  [PyString "distance"]
                  [PyString "20,000"]
                  [PyString "modulename"]
                  [PyTuple 2 items]
                    [PyInt 4]
                    [PyInt 24348]
                  [PyString "targetname"]
                  [PyString "Aknor Jaden's Myrmidon Wreck"]
                  */
void TractorBeam::Activate(SystemEntity* pSE)
{
    /** @todo allow orca-specific tractoring */
    /** @todo allow tractoring if not anchored */
    if (pSE->IsContainerSE() or pSE->IsWreckSE()) {
        m_targetEntity = pSE;
        m_targetID = pSE->GetID();

        // Activate active processing component timer:
        m_AMPC->ActivateCycle();
        //_ShowCycle();
        //m_ActiveModuleProc->ProcessActiveCycle();
    }
}

void TractorBeam::StopCycle(bool abort)
{
    m_targetEntity->DestinyMgr()->TractorBeamStop();

    uint32 timeLeft = m_AMPC->GetRemainingCycleTimeMS();
    timeLeft /= 100;

    // Create Special Effect:
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect
    (
        m_Ship,
        m_Item->itemID(),
        m_Item->typeID(),
        m_targetID,
        0,
        "effects.TractorBeam",
        0,
        0,
        0,
        0,
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
        ge.effectID = effectTractorBeam;
    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = 0;
        shipEff.active = 0;
        shipEff.environment = ge.Encode();
        shipEff.startTime = (shipEff.timeNow + (timeLeft * Win32Time_Second));
        shipEff.duration = _GetDuration();
        shipEff.repeat = 0;
        shipEff.error = new PyNone;
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

double TractorBeam::DoCycle() {
        if ((!m_Ship->GetPilot()->GetShipSE()->SysBubble())
            or (!m_Ship->GetPilot()->GetShipSE()->SysBubble()->GetEntity(m_targetID)) )
        {
            Deactivate();
            return 0;
        }

		_ShowCycle();

		GVector distanceToTarget(m_Ship->position(), m_targetEntity->GetPosition());
        if (distanceToTarget.length() < (m_Item->GetAttribute(AttrMaxRange).get_float())) {
            m_targetEntity->DestinyMgr()->TractorBeamStart(m_Ship->GetPilot()->GetShipSE());
            return _GetDuration();
		} else {
			Deactivate();
		}
}

void TractorBeam::_ShowCycle()
{
    // Create Special Effect:
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect
    (
        m_Ship,
        m_Item->itemID(),
        m_Item->typeID(),
        m_targetID,
        0,
        "effects.TractorBeam",
        1,
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
        ge.targetID = m_targetID;
        ge.other = new PyNone;
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
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

void TractorBeam::_SetCapNeed()
{
    // this will be needed for modules and rigs that affect cap need for mining modules
    //double need = GetAttribute(AttrCapacitorNeed);
}