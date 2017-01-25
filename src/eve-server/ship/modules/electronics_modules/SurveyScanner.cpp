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
 *    Author:   Allan
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

    pChar = m_shipRef->GetPilot()->GetChar().get();

    m_modRef->ResetAttribute(AttrDuration);
    m_modRef->ResetAttribute(AttrSurveyScanRange);

    // get module range
    m_range = GetAttribute(AttrSurveyScanRange).get_double();
    // get module duration
    m_cycleTime = GetAttribute(AttrDuration).get_double();

    m_range *= (1 + (0.03 * (pChar->GetSkillLevel(skillLongRangeTargeting, true)))); // 3% increase in range (here)
    m_cycleTime *= (1 - (0.02 * (pChar->GetSkillLevel(skillSignatureAnalysis, true)))); // 2% decrease in duration (here)

    if (m_shipRef->type().id() == 28606) { /* orca */
        m_range *= 500;               // 500% increase in range
    } else if (m_shipRef->type().id() == 28352) {  /* rorqual */
        m_range *= 900;               // 900% increase in range
    } else if (m_shipRef->type().groupID() == EVEDB::invGroups::MiningBarge) {
        m_range *= (1 + (0.05 * (pChar->GetSkillLevel(skillMiningBarge, true)))); // 5% increase in range (here)
        m_cycleTime *= (1 - (0.02 * (pChar->GetSkillLevel(skillMiningBarge, true)))); // 2% decrease in duration (here)
    } else if (m_shipRef->type().groupID() == EVEDB::invGroups::Exhumer) {
        m_range *= (1 + (0.1 * (pChar->GetSkillLevel(skillExhumers, true))));    // 10% increase in range (here)
        m_cycleTime *= (1 - (0.05 * (pChar->GetSkillLevel(skillExhumers, true)))); // 5% decrease in duration (here)
    } else {
        // lets modify range by 20% and duration by 5% for small mining ships.
        switch (m_shipRef->typeID()) {
            case 591: { /* Tormentor */
                m_range *= (1 + (0.2 * (pChar->GetSkillLevel(skillAmarrFrigate, true))));
                m_cycleTime *= (1 - (0.05 * (pChar->GetSkillLevel(skillAmarrFrigate, true))));
            } break;
            case 582: { /* Bantam */
                m_range *= (1 + (0.2 * (pChar->GetSkillLevel(skillCaldariFrigate, true))));
                m_cycleTime *= (1 - (0.05 * (pChar->GetSkillLevel(skillCaldariFrigate, true))));
            } break;
            case 592: { /* Navitas */
                m_range *= (1 + (0.2 * (pChar->GetSkillLevel(skillGallenteFrigate, true))));
                m_cycleTime *= (1 - (0.05 * (pChar->GetSkillLevel(skillGallenteFrigate, true))));
            } break;
            case 599: { /* Burst */
                m_range *= (1 + (0.2 * (pChar->GetSkillLevel(skillMinmatarFrigate, true))));
                m_cycleTime *= (1 - (0.05 * (pChar->GetSkillLevel(skillMinmatarFrigate, true))));
            } break;
            case 620: { /* Osprey */
                m_range *= (1 + (0.2 * (pChar->GetSkillLevel(skillCaldariCruiser, true))));
                m_cycleTime *= (1 - (0.05 * (pChar->GetSkillLevel(skillCaldariCruiser, true))));
            } break;
            case 631: { /* Scythe */
                m_range *= (1 + (0.2 * (pChar->GetSkillLevel(skillMinmatarCruiser, true))));
                m_cycleTime *= (1 - (0.05 * (pChar->GetSkillLevel(skillMinmatarCruiser, true))));
            } break;
        }
    }

    // save adjusted attributes
    m_modRef->SetAttribute(AttrDuration, m_cycleTime);
    m_modRef->SetAttribute(AttrSurveyScanRange, m_range);
}

SurveyScanner::~SurveyScanner()
{
    m_modRef->ResetAttribute(AttrDuration);
    m_modRef->ResetAttribute(AttrSurveyScanRange);
}

void SurveyScanner::Activate(SystemEntity* pSE)
{
    m_firstRun = true;
    ActiveModule::Activate(pSE);
    //_ShowCycle();
}

double SurveyScanner::DoCycle() {
    if (!m_shipRef->GetPilot()->GetShipSE()->SysBubble()) {
        Deactivate();
        return 0;
    }

    if (m_firstRun) {
        _ShowCycle();
        m_firstRun = false;
    } else {
        SystemEntity* pShipSE = m_shipRef->GetPilot()->GetShipSE();
        SystemBubble* pBubble = pShipSE->SysBubble();
        PyTuple* tuple = new PyTuple(2);
            tuple->SetItem(0, new PyString("OnSurveyScanComplete"));
        PyList* list = new PyList();
            tuple->SetItem(1, list);
        if (pBubble->IsBelt()) {
            std::vector<AsteroidSE*> vList;
            pShipSE->SystemMgr()->GetBeltMgr()->GetList(sBubbleMgr.GetSpawnID(pBubble->GetID()), vList);
            for (auto pASE : vList) {
                // allow ice scanning without a radius check....may change later.
                if (pShipSE->SysBubble()->IsIce() or (m_shipRef->position().distance(pASE->GetPosition()) < m_range)) {
                    PyTuple* tuple2 = new PyTuple(3);
                        tuple2->SetItem(0, new PyInt(pASE->GetID()));
                        tuple2->SetItem(1, new PyInt(pASE->GetTypeID()));
                        tuple2->SetItem(2, new PyInt(pASE->GetSelf()->GetAttribute(AttrQuantity).get_int()));
                    list->AddItem(tuple2);
                }
            }
        }
        // Send results.
        std::vector<PyTuple*> events;
            events.push_back(tuple);
        std::vector<PyTuple*> updates;
        pShipSE->DestinyMgr()->SendDestinyUpdate(updates, events, false);
        AbortCycle();
        return 0;
    }

    return m_cycleTime;
}

void SurveyScanner::StopCycle(bool abort)
{
    uint32 timeLeft = GetRemainingCycleTimeMS();
    timeLeft /= 1000;

    // Create Special Effect:
    m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect
    (
        m_shipRef,
        m_modRef->itemID(),
        m_modRef->typeID(),
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
        ge.selfID = m_modRef->itemID();
        ge.charID = m_shipRef->ownerID();
        ge.shipID = m_shipRef->itemID();
        ge.targetID = m_targetID;
        ge.other = new PyNone();
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
        shipEff.error = new PyNone();
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

void SurveyScanner::_ShowCycle()
{
    // Create Special Effect:
    m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect
    (
        m_shipRef,
        m_modRef->itemID(),
        m_modRef->typeID(),
        m_targetID,
        0,
        "effects.SurveyScan",
        0,
        1,
        1,
        m_cycleTime,
        m_repeat
    );

    // Create Destiny Updates:
    GodmaEnvironment ge;
        ge.selfID = m_modRef->itemID();
        ge.charID = m_shipRef->ownerID();
        ge.shipID = m_shipRef->itemID();
        ge.targetID = m_targetID;
        ge.other = new PyNone();
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
        shipEff.duration = m_cycleTime;
        shipEff.repeat = m_repeat;
        shipEff.error = new PyNone();
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

void SurveyScanner::_SetCapNeed()
{
    // this will be needed for modules and rigs that affect cap need for mining modules
    //double need = GetAttribute(AttrCapacitorNeed);
}
