
 /**
  * @name Prospector.cpp
  *   prospector module class (salvage, hacking, data mining)
  * @Author:         Allan
  * @date:   11 August 2016   -UD/RW 12 April 2017  -UD/RN 10 Feburary 2018
  */


#include "StaticDataMgr.h"
#include "ship/modules/Prospector.h"
#include "system/Container.h"
#include "system/SystemManager.h"

/* this class is for all salvage and data mining types */

Prospector::Prospector( InventoryItemRef item, ShipItemRef ship )
: ActiveModule(item, ship)
{
    m_success = false;
    m_firstRun = true;
    m_salvager = false;
    m_dataMiner = false;

    if (m_modRef->groupID() == EVEDB::invGroups::Salvager)
        m_salvager = true;
    else if (m_modRef->groupID() == EVEDB::invGroups::Salvager)
        m_dataMiner = true;

    m_accessChance = 0;

    pChar = m_shipRef->GetPilot()->GetChar().get();
}

void Prospector::Activate(uint16 effectID, uint32 targetID, int16 repeat)
{
    // reset for each activation
    m_success = false;
    m_firstRun = true;
    m_accessChance = m_targetSE->GetSelf()->GetAttribute(AttrAccessDifficulty).get_int();

    ActiveModule::Activate(effectID, targetID, repeat);
}

bool Prospector::CanActivate()
{
    if (m_salvager)
        if (m_targetSE->IsWreckSE())
            return ActiveModule::CanActivate();
    if (m_dataMiner)
        if (m_targetSE->IsContainerSE())
            return ActiveModule::CanActivate();

    throw PyException( MakeUserError( "DeniedActivateTargetModuleDisallowed"));
}

uint32 Prospector::DoCycle()
{
    if (m_firstRun) {
        m_firstRun = false;
        return ActiveModule::DoCycle();
    }

    CheckSuccess();
    if (!m_success) {
        SendFailure();
    } else if (m_success) {
        if (m_salvager)
            DropSalvage();
        if (m_dataMiner)
            DropItems();
        AbortCycle();
        return (m_accessChance = 0);
    } else {
        _log(SHIP__MODULE_ERROR, "Prospector::DoCycle() hit end of conditional.");
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
void Prospector::SendFailure()
{
    if (m_salvager) {
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
    if (m_dataMiner) {

    }
}

void Prospector::CheckSuccess()
{
    int8 chance = m_accessChance;
    chance += GetAttribute(AttrAccessDifficultyBonus).get_int();

    uint8 roll = MakeRandomInt(0,100);
    if (roll < chance)
        m_success = true;

    _log(SHIP__MODULE_DEBUG, "Prospector::CheckSuccess - chance: %i, roll: %u, success: %s", chance, roll, (m_success ? "true" : "false"));
}

void Prospector::DropSalvage()
{
    std::vector<uint32> list;
    list.clear();
    sDataMgr.GetSalvage(atoi(m_targetSE->GetSelf()->customInfo().c_str()), list);

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
            itemRef = sItemFactory.SpawnItem(iLoot);
            if (!itemRef) // we'll get over it...continue
                continue;
            itemRef->Move(m_shipRef->itemID(), flagCargoHold);
            m_shipRef->AddItem(itemRef);
        }
    }

    if (!m_targetSE->GetSelf()->GetMyInventory()->IsEmpty()) {
        std::map<uint32, InventoryItemRef> shipLoot;
        shipLoot.clear();
        m_targetSE->GetSelf()->GetMyInventory()->GetInventoryList(shipLoot);

        ItemData p_idata(23,   // 23 = cargo container
                        m_targetSE->GetSelf()->ownerID(),
                        m_targetSE->GetLocationID(),
                        flagAutoFit,
                        "Jettisoned Loot Container",
                        m_targetSE->GetPosition());

        CargoContainerRef jetCanRef = sItemFactory.SpawnCargoContainer(p_idata);
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

void Prospector::DropItems()
{
    // this will be for data miners and hacking/archeology shit.  dunno what all we'll need at this point.
    //  update StaticDataMgr for these items also.
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