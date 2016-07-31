/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2016 The EVEmu Team
 *    For the latest information visit http://evemu.org
 *    ------------------------------------------------------------------------------------
 *    This program is free software; you can redistribute it and/or modify it under
 *    the terms of the GNU Lesser General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option) any later
 *    version.
 *
 *    This program is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License along with
 *    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 *    http://www.gnu.org/copyleft/lesser.txt.
 *    ------------------------------------------------------------------------------------
 *    Author:        eve-moo
 */

#include "eve-server.h"

#include "ship/modules/electronics_modules/SurveyScanner.h"
#include "system/Asteroid.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/BeltMgr.h"

SurveyScanner::SurveyScanner(InventoryItemRef item, ShipItemRef ship)
: ActiveModule(item, ship)
{
    m_firstRun = true;

    pChar = m_Ship->GetPilot()->GetChar().get();
}

void SurveyScanner::Activate(SystemEntity* pSE)
{
    m_firstRun = true;
    m_AMPC->ActivateCycle();
    //_ShowCycle();
}

void SurveyScanner::Deactivate()
{
    if ((m_ModuleState != MOD_ACTIVATED) or (m_ModuleState == MOD_UNFITTED))
        return;
    ActiveModule::AbortCycle();
}

double SurveyScanner::DoCycle() {
    if (!m_Ship->GetPilot()->GetShipSE()->SysBubble()) {
        Deactivate();
        return 0;
    }

    if (m_firstRun) {
        _ShowCycle();
        m_firstRun = false;
    } else {
        SystemBubble *pBubble = m_Ship->GetPilot()->GetShipSE()->SysBubble();
        // Construct response packet.
        PyTuple *result = new PyTuple(2);
            result->SetItem(0, new PyString("OnSurveyScanComplete"));
        PyList *roids = new PyList();
            result->SetItem(1, roids);
        if (pBubble->IsBelt()) {
            std::vector<AsteroidSE*> vList;
            pBeltMgr->GetList(sBubbleMgr.GetSpawnID(pBubble->GetID()), vList);
            double maxDist = m_Item->GetAttribute(AttrSurveyScanRange).get_float();
            for (auto pASE : vList) {
                PyTuple *roid = new PyTuple(3);
                    roid->SetItem(0, new PyInt(pASE->GetID()));
                    roid->SetItem(1, new PyInt(pASE->GetTypeID()));
                    roid->SetItem(2, new PyInt(pASE->GetSelf()->GetAttribute(AttrQuantity).get_int()));
                roids->AddItem(roid);
            }
        }
        // Send results.
        std::vector<PyTuple*> events;
            events.push_back(result);
        std::vector<PyTuple*> updates;
        m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
        Deactivate();
    }

    return 0;
}

void SurveyScanner::StopCycle(bool abort)
{
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
        "effects.SurveyScan",
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
        ge.effectID = effectSurveyScan;
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
        shipEff.error = new PyNone;
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

void SurveyScanner::_ShowCycle()
{
    // Create Special Effect:
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect
    (
        m_Ship,
        m_Item->itemID(),
        m_Item->typeID(),
        m_targetID,
        0,
        "effects.SurveyScan",
        0,
        1,
        1,
        _GetDuration(),
        m_repeat
    );

    // Create Destiny Updates:
    GodmaEnvironment ge;
        ge.selfID = m_Item->itemID();
        ge.charID = m_Ship->ownerID();
        ge.shipID = m_Ship->itemID();
        ge.targetID = m_targetID;
        ge.other = new PyNone;
        ge.area = new PyList;
        ge.effectID = effectSurveyScan;
    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = 1;
        shipEff.active = 1;
        shipEff.environment = ge.Encode();
        shipEff.startTime = shipEff.timeNow;
        shipEff.duration = _GetDuration();
        shipEff.repeat = m_repeat;
        shipEff.error = new PyNone;
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

void SurveyScanner::_SetCapNeed()
{
    // this will be needed for modules and rigs that affect cap need for mining modules
    //double need = GetAttribute(AttrCapacitorNeed);
}
