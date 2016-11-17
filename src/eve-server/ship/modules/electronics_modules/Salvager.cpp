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

#include "ship/modules/electronics_modules/Salvager.h"
#include "system/Container.h"
#include "system/LootSystem.h"
#include "system/SystemManager.h"

/* this class is for all salvage and data mining types */

Salvager::Salvager( InventoryItemRef item, ShipItemRef ship )
: ActiveModule(item, ship)
{
    m_success = false;
    m_firstRun = true;

    m_accessChance = 0;

    pChar = m_Ship->GetPilot()->GetChar().get();
}

void Salvager::Activate(SystemEntity* pSE)
{
    // reset for each activation
    m_success = false;
    m_firstRun = true;
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

double Salvager::DoCycle() {
    if ((!m_Ship->GetPilot()->GetShipSE()->SysBubble())
        or (!m_Ship->GetPilot()->GetShipSE()->SysBubble()->GetEntity(m_targetID)) )
    {
        Deactivate();
        return 0;
    }

    if (m_firstRun) {
        _ShowCycle();
        m_firstRun = false;
    } else if (!m_success) {
        _ShowCycle();
        SendFailure();
        CheckSuccess();
    } else if (m_success) {
        AbortCycle();
        DropSalvage();
        return 0;
    } else {
        _log(SHIP__MODULE_ERROR, "Salvage DoCycle hit end of conditional.");
    }

    return m_cycleTime;
}

void Salvager::StopCycle(bool abort)
{
    uint32 timeLeft = m_AMPC->GetRemainingCycleTimeMS();
    timeLeft /= 1000;

    // Create Special Effect:
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect
    (
        m_Ship,
        m_Item->itemID(),
        m_Item->typeID(),
        m_targetID,
        0,
        "effects.Salvaging",
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
        ge.other = new PyNone();
        ge.area = new PyList;
        ge.effectID = effectSalvaging;
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
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

void Salvager::_ShowCycle()
{
    // Create Special Effect:
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect
    (
        m_Ship,
        m_Item->itemID(),
        m_Item->typeID(),
        m_targetID,
        0,
        "effects.Salvaging",
        0,
        1,
        1,
        m_cycleTime,
        1000
    );

    // Create Destiny Updates:
    GodmaEnvironment ge;
        ge.selfID = m_Item->itemID();
        ge.charID = m_Ship->ownerID();
        ge.shipID = m_Ship->itemID();
        ge.targetID = m_targetID;
        ge.other = new PyNone();
        ge.area = new PyList;
        ge.effectID = effectSalvaging;
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
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

void Salvager::_SetCapNeed()
{
    // this will be needed for modules and rigs that affect cap need for mining modules
    //double need = GetAttribute(AttrCapacitorNeed);
}

/*
                  [PyTuple 3 items]
                    [PyString "OnRemoteMessage"]
                    [PyString "SalvagingFailure"]
                    [PyDict 1 kvp]
                      [PyString "type"]
                      [PyTuple 2 items]
                        [PyInt 4]           << cacheSolarSystemObjects???  cant find another reference for this.  always 4 so far.
                        [PyInt 26513]       << wreck type id

                    [PyTuple 2 items]       << this goes into effect.error
                      [PyString "SalvagingSuccess"]
                      [PyDict 1 kvp]
                        [PyString "type"]
                        [PyTuple 2 items]
                          [PyInt 4]         << cacheSolarSystemObjects???
                          [PyInt 26513]
                        */
void Salvager::SendFailure()
{
    PyTuple* type = new PyTuple(2);
        type->SetItem(0, new PyInt(cacheSolarSystemObjects));
        type->SetItem(1, new PyInt(m_targetEntity->GetTypeID()));
    PyDict* dict = new PyDict;
        dict->SetItemString("type", type);
    PyTuple* tup = new PyTuple(3);
        tup->SetItem(0, new PyString("OnRemoteMessage"));
        tup->SetItem(1, new PyString("SalvagingFailure"));
        tup->SetItem(2, dict);
    std::vector<PyTuple*> events;
        events.push_back(tup);
    std::vector<PyTuple*> updates;
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

void Salvager::CheckSuccess()
{ // same forumla used in analyzing and data salvage
    m_accessChance = m_targetEntity->GetSelf()->GetAttribute(AttrAccessDifficulty).get_int();
    int8 bonus = (GetAttribute(AttrAccessDifficultyBonus).get_int() * pChar->GetSkillLevel(skillSalvaging));

    /** @todo need to check for salvage tackle and add to chance here */

    uint8 roll = MakeRandomInt(0,100);
    if (roll < (m_accessChance + bonus))
        m_success = true;

    _log(SHIP__MODULE_DEBUG, "Salvager::CheckSuccess - chance: %i, bonus: %i, roll: %u, success: %s", \
                            m_accessChance, bonus, roll, (m_success ? "true" : "false"));
}

void Salvager::DropSalvage()
{
    uint32 factionID = m_targetEntity->GetWarFactionID();

    std::vector<uint32> list;
    sDGM_Salvage_Table.GetSalvage(factionID, list);
    uint8 drop = 0;
    switch (m_accessChance) {       // drop qty * rate in config
        case  30: drop = 1; break;  //  1 to 3
        case  20: drop = 2; break;  //  2 to 6
        case  10: drop = 3; break;  //  3 to 9
        case   0: drop = 4; break;  //  4 to 12
        case -10: drop = 5; break;  //  5 to 15
    }

    if (!list.empty()) {
        InventoryItemRef itemRef;
        uint32 quantity = 0, minDrop = drop, maxDrop = (drop * 3 * sConfig.rates.RateDropItem);
        for (auto cur : list) {
            // each drop has 50/50 chance.  may need to change this later.   base on char's salvage skill?
            if (IsEven(MakeRandomInt(0,10)))
                continue;
            quantity = (MakeRandomInt(minDrop, maxDrop));
            ItemData iLoot(cur, pChar->itemID(), m_targetEntity->GetID(), flagAutoFit, quantity);
            itemRef = pChar->GetItemFactory()->SpawnItem(iLoot);
            if (!itemRef) // we'll get over it...continue
                continue;
            itemRef->Move(m_Ship->itemID(), flagCargoHold);
            m_Ship->AddItem(itemRef);
        }
    }

    m_accessChance = 0;

    uint32 timeLeft = m_AMPC->GetRemainingCycleTimeMS();
    timeLeft /= 1000;
    // Create Destiny Updates:
    PyTuple* type = new PyTuple(2);
        type->SetItem(0, new PyInt(cacheSolarSystemObjects));
        type->SetItem(1, new PyInt(m_targetEntity->GetTypeID()));
    PyDict* dict = new PyDict;
        dict->SetItemString("type", type);
    PyTuple* tup = new PyTuple(2);
        tup->SetItem(0, new PyString("SalvagingSuccess"));
        tup->SetItem(1, dict);
    GodmaEnvironment ge;
        ge.selfID = m_Item->itemID();
        ge.charID = m_Ship->ownerID();
        ge.shipID = m_Ship->itemID();
        ge.targetID = m_targetID;
        ge.other = new PyNone();
        ge.area = new PyList;
        ge.effectID = effectSalvaging;
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
        shipEff.error = tup;
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);

    if (!m_targetEntity->GetSelf()->GetInventory()->IsEmpty()) {
        std::map<uint32, InventoryItemRef> shipLoot;
        shipLoot.clear();
        m_targetEntity->GetSelf()->GetInventory()->GetInventoryList(shipLoot);

        ItemData p_idata(23,   // 23 = cargo container
                        m_targetEntity->GetSelf()->ownerID(),
                        m_targetEntity->GetLocationID(),
                        flagAutoFit,
                        "Jettisoned Loot Container",
                        m_targetEntity->GetPosition());

        CargoContainerRef jetCanRef = pChar->GetItemFactory()->SpawnCargoContainer(p_idata);
        if (!jetCanRef)
            throw PyException(MakeCustomError("Unable to spawn item of type %u.", 23));

        for (auto cur : shipLoot)
            cur.second->Move(jetCanRef->itemID(),flagAutoFit);

        // create new container
        ContainerData contData;
            contData.allianceID = m_targetEntity->GetAllianceID();
            contData.corporationID = m_targetEntity->GetCorporationID();
            contData.factionID = m_targetEntity->GetWarFactionID();
            contData.ownerID = m_targetEntity->GetSelf()->ownerID();
        ContainerSE* cSE = new ContainerSE(jetCanRef, m_targetEntity->GetServices(), m_targetEntity->SystemMgr(), contData);
        jetCanRef->SetMySE(cSE);
        m_targetEntity->SystemMgr()->AddEntity(cSE);
        m_targetEntity->DestinyMgr()->SendJettisonPacket(m_targetEntity->GetSelf());
    }
    m_targetEntity->SystemMgr()->RemoveEntity(m_targetEntity);
    m_targetEntity->GetSelf()->Delete();
}

/*
 *  accessDifficultyBonus       << salvage tackle(10), salvage tackleII(15),  salvage skill : salvagerI +5 per level, salvagerII +7 per level
 *  accessDifficulty (s:30,m:20,l:10,f:0,t2:0,o:-10,s:-20)           << in the item to salvage
 *
 *
 *  accessDifficultyBonus       << civilian analyzer(2), implant(5), analyzerII(7)
 *  accessDifficulty (0.000001)    << for analyzing structures ()
 *
 *
 */