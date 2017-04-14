
 /**
  * @name Salvager.cpp
  *   salvage module class
  * @Author:         Allan
  * @date:   11 August 2016   -UD/RW 12 April 2017
  */


#include "ship/modules/Salvager.h"
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

    pChar = m_shipRef->GetPilot()->GetChar().get();
}
void Salvager::Activate(uint16 effectID, uint32 targetID, int16 repeat)
{
    ActiveModule::Activate(effectID, targetID, repeat);
    // reset for each activation
    m_success = false;
    m_firstRun = true;
}

bool Salvager::CanActivate()
{
    if (m_targetSE->IsContainerSE() or m_targetSE->IsWreckSE())
        return ActiveModule::CanActivate();
    return false;
}

uint32 Salvager::DoCycle()
{
    if (m_firstRun) {
        m_firstRun = false;
    } else if (!m_success) {
        SendFailure();
        CheckSuccess();
    } else if (m_success) {
        DropSalvage();
        AbortCycle();
        return 0;
    } else {
        _log(SHIP__MODULE_ERROR, "Salvage DoCycle hit end of conditional.");
    }

    return ActiveModule::DoCycle();
}

/*
                  [PyTuple 3 items]
                    [PyString "OnRemoteMessage"]
                    [PyString "SalvagingFailure"]       (this is also a dungeon trigger)
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
        type->SetItem(1, new PyInt(m_targetSE->GetTypeID()));
    PyDict* dict = new PyDict;
        dict->SetItemString("type", type);
    PyTuple* tup = new PyTuple(3);
        tup->SetItem(0, new PyString("OnRemoteMessage"));
        tup->SetItem(1, new PyString("SalvagingFailure"));
        tup->SetItem(2, dict);
    std::vector<PyTuple*> events;
        events.push_back(tup);
    std::vector<PyTuple*> updates;
    m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

void Salvager::CheckSuccess()
{ // same forumla used in analyzing and data salvage
    m_accessChance = m_targetSE->GetSelf()->GetAttribute(AttrAccessDifficulty).get_int();

    uint8 roll = MakeRandomInt(0,100);
    if (roll < m_accessChance)
        m_success = true;

    _log(SHIP__MODULE_DEBUG, "Salvager::CheckSuccess - chance: %i, roll: %u, success: %s", \
                            m_accessChance, roll, (m_success ? "true" : "false"));
}

void Salvager::DropSalvage()
{
    std::vector<uint32> list;
    sDGM_Salvage_Table.GetSalvage(m_targetSE->GetWarFactionID(), list);
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
            ItemData iLoot(cur, pChar->itemID(), m_targetSE->GetID(), flagAutoFit, quantity);
            itemRef = pChar->GetItemFactory()->SpawnItem(iLoot);
            if (!itemRef) // we'll get over it...continue
                continue;
            itemRef->Move(m_shipRef->itemID(), flagCargoHold);
            m_shipRef->AddItem(itemRef);
        }
    }

    m_accessChance = 0;

    if (!m_targetSE->GetSelf()->GetInventory()->IsEmpty()) {
        std::map<uint32, InventoryItemRef> shipLoot;
        shipLoot.clear();
        m_targetSE->GetSelf()->GetInventory()->GetInventoryList(shipLoot);

        ItemData p_idata(23,   // 23 = cargo container
                        m_targetSE->GetSelf()->ownerID(),
                        m_targetSE->GetLocationID(),
                        flagAutoFit,
                        "Jettisoned Loot Container",
                        m_targetSE->GetPosition());

        CargoContainerRef jetCanRef = pChar->GetItemFactory()->SpawnCargoContainer(p_idata);
        if (!jetCanRef)
            throw PyException(MakeCustomError("Unable to spawn item of type %u.", 23));

        for (auto cur : shipLoot)
            cur.second->Move(jetCanRef->itemID(),flagAutoFit);

        // create new container
        FactionData contData;
            contData.allianceID = m_targetSE->GetAllianceID();
            contData.corporationID = m_targetSE->GetCorporationID();
            contData.factionID = m_targetSE->GetWarFactionID();
            contData.ownerID = m_targetSE->GetSelf()->ownerID();
        ContainerSE* cSE = new ContainerSE(jetCanRef, m_targetSE->GetServices(), m_targetSE->SystemMgr(), contData);
        jetCanRef->SetMySE(cSE);
        m_targetSE->SystemMgr()->AddEntity(cSE);
        m_targetSE->DestinyMgr()->SendJettisonPacket();
    }
    m_targetSE->SystemMgr()->RemoveEntity(m_targetSE);
    m_targetSE->GetSelf()->Delete();
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