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
    Author:        Luck (outline only)
    Updates:    Allan   (coded working system)
*/

#include "eve-server.h"

#include <unordered_map>
#include "EntityList.h"
#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "station/TradeService.h"
#include "system/SystemManager.h"
#include "system/Container.h"

PyCallable_Make_InnerDispatcher(TradeService);

class TradeBound
: public PyBoundObject
{
  friend TradeService;

public:
    PyCallable_Make_Dispatcher(TradeBound)

    TradeBound(PyServiceMgr *mgr)
    : PyBoundObject(mgr),
      m_dispatch(new Dispatcher(this))
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "TradeBound";
        m_TSvc = (TradeService*)(mgr->LookupService("trademgr"));

        PyCallable_REG_CALL(TradeBound, List);
        PyCallable_REG_CALL(TradeBound, IsCEOTrade);
        PyCallable_REG_CALL(TradeBound, GetItemID);
        PyCallable_REG_CALL(TradeBound, GetItem);
        PyCallable_REG_CALL(TradeBound, Add);
        PyCallable_REG_CALL(TradeBound, MultiAdd);
        PyCallable_REG_CALL(TradeBound, ToggleAccept);
        PyCallable_REG_CALL(TradeBound, OfferMoney);
        PyCallable_REG_CALL(TradeBound, Abort);
    }
    virtual ~TradeBound()
    {
        delete m_dispatch;
    }

    virtual void Release() {
        //this needs to die
        delete this;
    }

    void ExchangeItems(Client* pClient, Client* pOther, TradeSession* pTSes);
    void CancelTrade(Client* pClient, Client* pOther, TradeSession* pTSes);

    PyCallable_DECL_CALL(List);
    PyCallable_DECL_CALL(IsCEOTrade);
    PyCallable_DECL_CALL(GetItemID);
    PyCallable_DECL_CALL(GetItem);
    PyCallable_DECL_CALL(Add);
    PyCallable_DECL_CALL(MultiAdd);
    PyCallable_DECL_CALL(ToggleAccept);
    PyCallable_DECL_CALL(OfferMoney);
    PyCallable_DECL_CALL(Abort);

protected:
    Dispatcher *const m_dispatch;
    TradeService* m_TSvc;                // get registered TradeService object
};


TradeService::TradeService(PyServiceMgr *mgr)
: PyService(mgr, "trademgr"),
  m_dispatch(new Dispatcher(this))
{
    m_SvcMgr = mgr;
    _SetCallDispatcher(m_dispatch);
    m_SessionID = minTradeCont;

    PyCallable_REG_CALL(TradeService, InitiateTrade);
}

TradeService::~TradeService() {
    delete m_dispatch;
}

PyBoundObject* TradeService::_CreateBoundObject(Client* pClient, const PyRep *bind_args) {
    // each client's trade session has it's own bound object.
    //   create code for multiple sessions per client, using TradeBound and TradeSession.
    Trade_BindArgs args;
    //temp crap until I rework _CreateBoundObject's signature
    PyRep *t = bind_args->Clone();
    if(!args.Decode(&t)) {
        codelog(SERVICE__ERROR, "Failed to decode bind args from '%s'", pClient->GetName());
        return nullptr;
    }
    _log(COLLECT__OTHER_DUMP, "Trade bind request for:");
    args.Dump(COLLECT__OTHER_DUMP, "    ");

    /** @todo update to multiple trade sessions per client.  current code only allows one at a time. */

    // check to see if this is target calling for a bound object.  if not, create new session
    std::map<uint32, ActiveSession>::iterator itr = m_activeSessions.find(args.myID);
    if (itr == m_activeSessions.end()) {
        TradeSession* pTSes = new TradeSession;
        pClient->SetTradeSession(pTSes);
        uint32 contID = GetTradeSessionID();
        pTSes->m_tradeSession.containerID = contID;
        pTSes->m_tradeSession.stationID   = args.stationID;
        pTSes->m_tradeSession.myID        = args.myID;
        pTSes->m_tradeSession.herID       = args.herID;
        pTSes->m_tradeSession.myState     = false;
        pTSes->m_tradeSession.herState    = false;
        pTSes->m_tradeSession.myMoney     = args.myMoney;
        pTSes->m_tradeSession.herMoney    = args.herMoney;
        pTSes->m_tradeSession.fileTime    = args.fileTime;
        ActiveSession cAS;
            cAS.myID = args.myID;
            cAS.herID = args.herID;
            cAS.contID = contID;
            cAS.ourTS = pTSes;
        m_activeSessions.insert(std::make_pair(args.myID, cAS));
        m_activeSessions.insert(std::make_pair(args.herID, cAS));
    } else {
        pClient->SetTradeSession(itr->second.ourTS);
    }

    TradeBound* pTB = new TradeBound(m_manager);
    return pTB;
}

PyResult TradeBound::Handle_OfferMoney(PyCallArgs &call) {
    TradeSession* pTSes = call.client->GetTradeSession();
    Client* pClient = sEntityList.FindClientByCharID(pTSes->m_tradeSession.myID);
    Client* pOther = sEntityList.FindClientByCharID(pTSes->m_tradeSession.herID);
    PyList* list = new PyList(2);

    if (call.client->GetCharacterID() == pTSes->m_tradeSession.myID) {
        // this is 'my'
        pTSes->m_tradeSession.myMoney = call.tuple->GetItem(0)->AsFloat()->value();
        list->SetItem(0, new PyFloat(pTSes->m_tradeSession.myMoney));  //myMoney
        list->SetItem(1, new PyFloat(pTSes->m_tradeSession.herMoney)); //herMoney
    } else if (call.client->GetCharacterID() == pTSes->m_tradeSession.herID) {
        // this is 'her'
        pTSes->m_tradeSession.herMoney = call.tuple->GetItem(0)->AsFloat()->value();
        list->SetItem(0, new PyFloat(pTSes->m_tradeSession.myMoney));  //myMoney
        list->SetItem(1, new PyFloat(pTSes->m_tradeSession.herMoney)); //herMoney
    } else {
        list->SetItem(0, new PyFloat(0.0f)); //myMoney
        list->SetItem(1, new PyFloat(0.0f)); //herMoney
        _log(CLIENT__ERROR, "TradeBound::Handle_OfferMoney() : %s(%u) & %s(%u) - clients are neither mine nor hers.", \
        pClient->GetName(), pClient->GetCharacterID(), pOther->GetName(), pOther->GetCharacterID());
        return new PyNone();
    }

    //  reset states after offer changes..
    pTSes->m_tradeSession.myState  = false;
    pTSes->m_tradeSession.herState = false;
    // send changes
    PyTuple* tuple = new PyTuple(3);
        tuple->SetItem(0, new PyString("MoneyOffer"));
        tuple->SetItem(1, new PyInt(pTSes->m_tradeSession.containerID));
        tuple->SetItem(2, list->Clone());
    PyTuple* tuple1 = new PyTuple(3);
        tuple1->SetItem(0, new PyString("MoneyOffer"));
        tuple1->SetItem(1, new PyInt(pTSes->m_tradeSession.containerID));
        tuple1->SetItem(2, list);
    // now send it, bypassing the extra shit and wrong dest name added in Client::SendNotification
    pClient->SendNotification("OnTrade", "charid", &tuple);
    pOther->SendNotification("OnTrade", "charid", &tuple1);
    // returns none
    return new PyNone();
}

PyResult TradeBound::Handle_Abort(PyCallArgs &call) {
    TradeSession* pTSes = call.client->GetTradeSession();
    Client* pClient = sEntityList.FindClientByCharID(pTSes->m_tradeSession.myID);
    Client* pOther = sEntityList.FindClientByCharID(pTSes->m_tradeSession.herID);

    CancelTrade(pClient, pOther, pTSes);

    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyString("Cancel"));
        tuple->SetItem(1, new PyInt(pTSes->m_tradeSession.containerID));
    PyTuple* tuple1 = new PyTuple(2);
        tuple1->SetItem(0, new PyString("Cancel"));
        tuple1->SetItem(1, new PyInt(pTSes->m_tradeSession.containerID));
    // now send it, bypassing the extra shit and wrong dest name added in Client::SendNotification
    pClient->SendNotification("OnTrade", "charid", &tuple);
    pOther->SendNotification("OnTrade", "charid", &tuple1);
    m_TSvc->RemoveActiveSession(pTSes->m_tradeSession.myID);
    m_TSvc->RemoveActiveSession(pTSes->m_tradeSession.herID);
    SafeDelete(pTSes);
    pClient->ClearTradeSession();
    pOther->ClearTradeSession();
    // returns none
    return new PyNone();
}

void TradeBound::CancelTrade(Client* pClient, Client* pOther, TradeSession* pTSes)
{
    // trade cancelled.  send items back to owner. (monies not taken at this point)
    PyDict* dict = new PyDict;
    dict->SetItem(new PyInt(ixLocationID), new PyInt(pTSes->m_tradeSession.containerID));

    ItemFactory* factory = pClient->services().item_factory;
    uint32 stationID = pTSes->m_tradeSession.stationID;
    for (auto cur : pTSes->m_tradelist) {
        InventoryItemRef itemRef = factory->GetItem(cur.itemID);
        if (!itemRef)  {
            _log(SERVICE__ERROR, "TradeBound::CancelTrade() - Failed to get ItemRef.");
            continue;
        }

        itemRef->Move(stationID, flagHangar, true);
    }
}

PyResult TradeBound::Handle_ToggleAccept(PyCallArgs &call) {
    TradeSession* pTSes = call.client->GetTradeSession();
    Client* pClient = nullptr;
    Client* pOther = nullptr;

    bool myAccept = pTSes->m_tradeSession.myState;
    bool herAccept = pTSes->m_tradeSession.herState;

    if (call.client->GetCharacterID() == pTSes->m_tradeSession.myID) {
        // this is 'my'
        pClient = sEntityList.FindClientByCharID(pTSes->m_tradeSession.myID);
        pOther = sEntityList.FindClientByCharID(pTSes->m_tradeSession.herID);
        myAccept = call.tuple->GetItem(0)->AsBool()->value();
    } else if (call.client->GetCharacterID() == pTSes->m_tradeSession.herID) {
        // this is 'her'
        pClient = sEntityList.FindClientByCharID(pTSes->m_tradeSession.herID);
        pOther = sEntityList.FindClientByCharID(pTSes->m_tradeSession.myID);
        herAccept = call.tuple->GetItem(0)->AsBool()->value();
    } else {
        _log(PLAYER__TRADE_MESSAGE, "TradeBound::Handle_ToggleAccept() : %s(%u) & %s(%u) - clients are neither mine nor hers.", \
                    pClient->GetName(), pClient->GetCharacterID(), pOther->GetName(), pOther->GetCharacterID());
        return new PyNone();
    }

    bool forceTrade = false;
    if (call.byname.find("forceTrade") != call.byname.cend())
        if (!call.byname.find("forceTrade")->second->IsNone())
            forceTrade = call.byname.find("forceTrade")->second->AsBool()->value();

    if (forceTrade) {
        pTSes->m_tradeSession.myState  = true;
        pTSes->m_tradeSession.herState = true;
    } else {
        pTSes->m_tradeSession.myState  = myAccept;
        pTSes->m_tradeSession.herState = herAccept;
    }

    _log(PLAYER__TRADE_MESSAGE, "TradeBound::Handle_ToggleAccept() is now %s/%s. forceTrade is %s.", \
                (myAccept ? "true" : "false"), (herAccept ? "true" : "false"), (forceTrade ? "true" : "false"));

    PyTuple* tuple = new PyTuple(3);
        tuple->SetItem(0, new PyString("StateToggle"));
        tuple->SetItem(1, new PyBool(myAccept));
        tuple->SetItem(2, new PyBool(herAccept));
    PyTuple* tuple1 = new PyTuple(3);
        tuple1->SetItem(0, new PyString("StateToggle"));
        tuple1->SetItem(1, new PyBool(myAccept));
        tuple1->SetItem(2, new PyBool(herAccept));
    // now send it, bypassing the extra shit and wrong dest name added in Client::SendNotification
    pClient->SendNotification("OnTrade", "charid", &tuple);
    pOther->SendNotification("OnTrade", "charid", &tuple1);

    if (myAccept && herAccept) {
        ExchangeItems(pClient, pOther, pTSes);      // trade completed.
        m_TSvc->RemoveActiveSession(pTSes->m_tradeSession.myID);
        m_TSvc->RemoveActiveSession(pTSes->m_tradeSession.herID);
        pClient->ClearTradeSession();
        pOther->ClearTradeSession();
    }

    // returns none
    return new PyNone();
}

PyResult TradeBound::Handle_GetItemID(PyCallArgs &call) {
    _log(CLIENT__CALL_DUMP, "TradeBound::Handle_GetItemID() size=%u", call.tuple->size() );
    call.Dump(CLIENT__CALL_DUMP);
    // still not sure what this does...only returns PyNone in packet logs.
    // returns none
    return new PyNone();
}

/** @todo  refresh other window when item added */
PyResult TradeBound::Handle_Add(PyCallArgs &call) {
    Call_TwoIntegerArgs args;
    /*  .arg1 = itemID to insert into trade
     *  .arg2 = item's current containerID
     *  call.byname "qty"
     */
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        Handle_Abort(call);
        return new PyNone();
    }
    InventoryItemRef itemRef = call.client->services().item_factory->GetItem(args.arg1);
    if (!itemRef)  {
        _log(PLAYER__TRADE_MESSAGE, "TradeBound::Handle_Add() - Failed to get ItemRef.");
        //  should i abort trade, or just return null here?  single add, so not a big deal.
        //     return null, let them try again if they want.  maybe later add config option?
        //Handle_Abort(call);   << this will cancel and nullify the trade session
        return new PyNone();
    }

    TradeSession* pTSes = call.client->GetTradeSession();
    Client* pClient = sEntityList.FindClientByCharID(pTSes->m_tradeSession.myID);
    Client* pOther = sEntityList.FindClientByCharID(pTSes->m_tradeSession.herID);

    if (call.client->GetCharacterID() == pTSes->m_tradeSession.myID) {
        // this is 'my'
    } else if (call.client->GetCharacterID() == pTSes->m_tradeSession.herID) {
        // this is 'her'
    } else {
        _log(PLAYER__TRADE_MESSAGE, "TradeBound::Handle_() : %s(%u) & %s(%u) - clients are neither mine nor hers.", \
        pClient->GetName(), pClient->GetCharacterID(), pOther->GetName(), pOther->GetCharacterID());
        return new PyNone();
    }

    uint32 flag = 0;
    if (call.byname.find("flag") != call.byname.cend())
        if (!call.byname.find("flag")->second->IsNone())
            flag = call.byname.find("flag")->second->AsInt()->value();
    uint32 qty = 0;
    if (call.byname.find("qty") != call.byname.cend())
        if (!call.byname.find("qty")->second->IsNone())
            qty = call.byname.find("qty")->second->AsInt()->value();

    uint32 tradeContainerID = pTSes->m_tradeSession.containerID;
    TradeSession::TradeItems mTI;
        mTI.itemID = args.arg1;
        mTI.typeID = itemRef->typeID();
        mTI.ownerID = call.client->GetCharacterID();
        mTI.locationID = tradeContainerID;
        mTI.flagID = itemRef->flag();
        mTI.quantity = qty;
        mTI.groupID = itemRef->groupID();
        mTI.singleton = itemRef->singleton();
        mTI.categoryID = itemRef->categoryID();
        mTI.customInfo = "";
        pTSes->m_tradelist.insert(pTSes->m_tradelist.end(), mTI);

        itemRef->Move(tradeContainerID, (EVEItemFlags)flag, true);

    PyDict* dict = new PyDict;
        dict->SetItem(new PyInt(ixLocationID), new PyInt(args.arg2));

    DBRowDescriptor* header = m_TSvc->CreateHeader();
    PyPackedRow* row = new PyPackedRow( header );
        row->SetField( "itemID",        new PyLong(mTI.itemID));
        row->SetField( "typeID",        new PyInt(mTI.typeID));
        row->SetField( "ownerID",       new PyInt(mTI.ownerID));
        row->SetField( "locationID",    new PyLong(mTI.locationID));
        row->SetField( "flagID",        new PyInt(mTI.flagID));
        row->SetField( "stacksize",     new PyInt(mTI.quantity));
        row->SetField( "groupID",       new PyInt(mTI.groupID));
        row->SetField( "singleton",     new PyBool(mTI.singleton));
        row->SetField( "categoryID",    new PyInt(mTI.categoryID));
        row->SetField( "customInfo",    new PyString(mTI.customInfo));

    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, row->Clone());
        tuple->SetItem(1, dict->Clone());
    PyTuple* tuple1 = new PyTuple(2);
        tuple1->SetItem(0, row);
        tuple1->SetItem(1, dict);
    // now send it, bypassing the extra shit and wrong dest name added in Client::SendNotification
    pClient->SendNotification("OnItemChange", "charid", &tuple);
    pOther->SendNotification("OnItemChange", "charid", &tuple1);

    //  reset states after offer changes..
    pTSes->m_tradeSession.myState  = false;
    pTSes->m_tradeSession.herState = false;
    // return none
    return new PyNone();
}

PyResult TradeBound::Handle_MultiAdd(PyCallArgs &call) {
    TradeMultiAddList args;
    /*  .ints = list of itemIDs to insert into trade
     *  .contID = item's current containerID
     *  call.byname "flag"
     */
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        Handle_Abort(call);
        return new PyNone();
    }
    uint32 flag = 0;
    if (call.byname.find("flag") != call.byname.cend())
        if (!call.byname.find("flag")->second->IsNone())
            flag = call.byname.find("flag")->second->AsInt()->value();

    PyDict* dict = new PyDict;
        dict->SetItem(new PyInt(ixLocationID), new PyInt(args.contID));

    TradeSession* pTSes = call.client->GetTradeSession();
    Client* pClient = sEntityList.FindClientByCharID(pTSes->m_tradeSession.myID);
    Client* pOther = sEntityList.FindClientByCharID(pTSes->m_tradeSession.herID);

    if (call.client->GetCharacterID() == pTSes->m_tradeSession.myID) {
        // this is 'my'
    } else if (call.client->GetCharacterID() == pTSes->m_tradeSession.herID) {
        // this is 'her'
    } else {
        _log(PLAYER__TRADE_MESSAGE, "TradeBound::Handle_MultiAdd() : %s(%u) & %s(%u) - clients are neither mine nor hers.", \
        pClient->GetName(), pClient->GetCharacterID(), pOther->GetName(), pOther->GetCharacterID());
        return new PyNone();
    }

    uint32 charID = call.client->GetCharacterID();
    uint32 tradeContID = pTSes->m_tradeSession.containerID;
    ItemFactory* factory = call.client->services().item_factory;

    DBRowDescriptor* header = m_TSvc->CreateHeader();
    std::vector<int32> list = args.ints;
    for (auto cur : list) {
        InventoryItemRef itemRef = factory->GetItem(cur);
        if (!itemRef)  {
            _log(SERVICE__ERROR, "TradeBound::Handle_Add() - Failed to get ItemRef.");
            continue;
        }

        TradeSession::TradeItems mTI;
            mTI.itemID = cur;
            mTI.typeID = itemRef->typeID();
            mTI.ownerID = charID;
            mTI.locationID = tradeContID;
            mTI.flagID = flag;
            mTI.quantity = itemRef->quantity();
            mTI.groupID = itemRef->groupID();
            mTI.singleton = itemRef->singleton();
            mTI.categoryID = itemRef->categoryID();
            mTI.customInfo = "";
        pTSes->m_tradelist.insert(pTSes->m_tradelist.end(), mTI);
        itemRef->Move(tradeContID, (EVEItemFlags)flag, true);

        PyPackedRow* row = new PyPackedRow( header );
            row->SetField( "itemID",        new PyLong(mTI.itemID));
            row->SetField( "typeID",        new PyInt(mTI.typeID));
            row->SetField( "ownerID",       new PyInt(mTI.ownerID));
            row->SetField( "locationID",    new PyLong(mTI.locationID));
            row->SetField( "flagID",        new PyInt(mTI.flagID));
            row->SetField( "stacksize",     new PyInt(mTI.quantity));
            row->SetField( "groupID",       new PyInt(mTI.groupID));
            row->SetField( "singleton",     new PyBool(mTI.singleton));
            row->SetField( "categoryID",    new PyInt(mTI.categoryID));
            row->SetField( "customInfo",    new PyString(mTI.customInfo));
        PyTuple* tuple = new PyTuple(2);
            tuple->SetItem(0, row->Clone());
            tuple->SetItem(1, dict->Clone());
        PyTuple* tuple1 = new PyTuple(2);
            tuple1->SetItem(0, row);
            tuple1->SetItem(1, dict->Clone());
        // now send it, bypassing the extra shit and wrong dest name added in Client::SendNotification
        pClient->SendNotification("OnItemChange", "charid", &tuple);
        pOther->SendNotification("OnItemChange", "charid", &tuple1);
    }

    //  reset states after offer changes.
    //  reset states after offer changes..
    pTSes->m_tradeSession.myState  = false;
    pTSes->m_tradeSession.herState = false;
    // return none
    return new PyNone();
}

PyResult TradeBound::Handle_GetItem(PyCallArgs &call) {
    TradeSession* pTSes = call.client->GetTradeSession();
    DBRowDescriptor* header = new DBRowDescriptor;
        header->AddColumn( "itemID",     DBTYPE_I8 );
        header->AddColumn( "typeID",     DBTYPE_I4 );
        header->AddColumn( "ownerID",    DBTYPE_I4 );
        header->AddColumn( "locationID", DBTYPE_I8 );
        header->AddColumn( "flagID",     DBTYPE_I2 );
        header->AddColumn( "quantity",   DBTYPE_I4 );
        header->AddColumn( "groupID",    DBTYPE_I2 );
        header->AddColumn( "categoryID", DBTYPE_I4 );
        header->AddColumn( "customInfo", DBTYPE_STR );
    PyPackedRow* row = new PyPackedRow( header );
        row->SetField( "itemID",        new PyLong(pTSes->m_tradeSession.containerID));
        row->SetField( "typeID",        new PyInt(53));     // type Trade Window
        row->SetField( "ownerID",       new PyInt(1));      // EvE_System
        row->SetField( "locationID",    new PyLong(pTSes->m_tradeSession.stationID));
        row->SetField( "flagID",        new PyNone());
        row->SetField( "quantity",      new PyInt(-1));     // singleton
        row->SetField( "groupID",       new PyInt(EVEDB::invGroups::Trade_Session ) );
        row->SetField( "categoryID",    new PyInt(EVEDB::invCategories::Trading));
        row->SetField( "customInfo",    new PyNone());
    return row;
}

PyResult TradeBound::Handle_IsCEOTrade(PyCallArgs &call) {
    _log(CLIENT__CALL_DUMP, "TradeBound::Handle_IsCEOTrade() size=%u", call.tuple->size() );
    call.Dump(CLIENT__CALL_DUMP);
/*
    if (call.tuple->size() > 0)
        _log(CLIENT__CALL_DUMP, "TradeBound::Handle_IsCEOTrade() returned %s. need more code here.", \
            (call.tuple->GetItem(0)->AsBool()->value() ? "true" : "false"));
*/
    //TODO will have to work on this later.  need corps working correctly first.
    return new PyBool(false);
}

PyResult TradeBound::Handle_List(PyCallArgs &call) {
    TradeSession* pTSes = call.client->GetTradeSession();
    PyList* list = new PyList();

    DBRowDescriptor* header = new DBRowDescriptor;
        header->AddColumn( "itemID",     DBTYPE_I8 );
        header->AddColumn( "typeID",     DBTYPE_I4 );
        header->AddColumn( "ownerID",    DBTYPE_I4 );
        header->AddColumn( "locationID", DBTYPE_I8 );
        header->AddColumn( "flagID",     DBTYPE_I2 );
        header->AddColumn( "stacksize",  DBTYPE_I4 );
        header->AddColumn( "groupID",    DBTYPE_I2 );
        header->AddColumn( "singleton",  DBTYPE_BOOL );
        header->AddColumn( "categoryID", DBTYPE_I4 );
        header->AddColumn( "customInfo", DBTYPE_STR );
    for (auto itr : pTSes->m_tradelist) {
        PyPackedRow* row = new PyPackedRow( header );
            row->SetField( "itemID",        new PyLong(itr.itemID));
            row->SetField( "typeID",        new PyInt(itr.typeID));
            row->SetField( "ownerID",       new PyInt(itr.ownerID));
            row->SetField( "locationID",    new PyLong(itr.locationID));
            row->SetField( "flagID",        new PyInt(itr.flagID));
            row->SetField( "stacksize",     new PyInt(itr.quantity));
            row->SetField( "groupID",       new PyInt(itr.groupID));
            row->SetField( "singleton",     new PyBool(itr.singleton));
            row->SetField( "categoryID",    new PyInt(itr.categoryID));
            row->SetField( "customInfo",    new PyString(itr.customInfo));
        list->AddItem(row);
    }

    PyTuple* tuple2 = new PyTuple(1);
        tuple2->SetItem(0, list);
    PyToken* token = new PyToken("__builtin__.set");
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, token);
        tuple->SetItem(1, tuple2);
    TradeListData tld;
        tld.tradeContainerID = pTSes->m_tradeSession.containerID;
        tld.myID             = pTSes->m_tradeSession.myID;
        tld.herID            = pTSes->m_tradeSession.herID;
        tld.myMoney          = pTSes->m_tradeSession.myMoney;
        tld.herMoney         = pTSes->m_tradeSession.herMoney;
        tld.myState          = pTSes->m_tradeSession.myState;
        tld.herState         = pTSes->m_tradeSession.herState;
        tld.list             = new PyObjectEx(false, tuple);
    PyList* itemNames = new PyList(5);
        itemNames->SetItem(0, new PyString("tradeContainerID"));
        itemNames->SetItem(1, new PyString("traders"));
        itemNames->SetItem(2, new PyString("state"));
        itemNames->SetItem(3, new PyString("money"));
        itemNames->SetItem(4, new PyString("items"));
    TradeListRsp tlr;
        tlr.header  = itemNames;
        tlr.line    = tld.Encode();
    return tlr.Encode();
}

void TradeBound::ExchangeItems(Client* pClient, Client* pOther, TradeSession* pTSes) {
    // trade completed.  perform item and money exchange
    if (pClient->GetCharacterID() == pTSes->m_tradeSession.herID) {
        pOther = sEntityList.FindClientByCharID(pTSes->m_tradeSession.herID);
        pClient = sEntityList.FindClientByCharID(pTSes->m_tradeSession.myID);
    }
    pClient->AddBalance(-pTSes->m_tradeSession.myMoney);
    pClient->AddBalance(pTSes->m_tradeSession.herMoney);
    pOther->AddBalance(-pTSes->m_tradeSession.herMoney);
    pOther->AddBalance(pTSes->m_tradeSession.myMoney);

    PyDict* dict = new PyDict;
        dict->SetItem(new PyInt(ixLocationID), new PyInt(pTSes->m_tradeSession.containerID));

    ItemFactory* factory = pClient->services().item_factory;
    uint32 stationID = pTSes->m_tradeSession.stationID;
    for (auto cur : pTSes->m_tradelist) {
        InventoryItemRef itemRef = factory->GetItem(cur.itemID);
        if (!itemRef)  {
            _log(SERVICE__ERROR, "TradeBound::Handle_Add() - Failed to get ItemRef.");
            continue;
        }

        uint32 newOwnerID = (cur.ownerID == pTSes->m_tradeSession.myID ? pTSes->m_tradeSession.herID : pTSes->m_tradeSession.myID);
        itemRef->ChangeOwner(newOwnerID, true);
        itemRef->Move(stationID, flagHangar, true);

        if ((itemRef->categoryID() == EVEDB::invCategories::Ship)
            || (itemRef->groupID() == EVEDB::invGroups::Audit_Log_Secure_Container)
            || (itemRef->groupID() == EVEDB::invGroups::Cargo_Container)
            || (itemRef->groupID() == EVEDB::invGroups::Freight_Container)
            || (itemRef->groupID() == EVEDB::invGroups::Secure_Cargo_Container))
            m_TSvc->TransferContainerContents(pClient->SystemMgr(), itemRef, newOwnerID);
    }

    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyString("TradeComplete"));
        tuple->SetItem(1, new PyInt(pTSes->m_tradeSession.containerID));
    PyTuple* tuple1 = new PyTuple(2);
        tuple1->SetItem(0, new PyString("TradeComplete"));
        tuple1->SetItem(1, new PyInt(pTSes->m_tradeSession.containerID));
    // now send it, bypassing the extra shit and wrong dest name added in Client::SendNotification
    pClient->SendNotification("OnTrade", "charid", &tuple);
    pOther->SendNotification("OnTrade", "charid", &tuple1);
}

void TradeService::TransferContainerContents(SystemManager* pSysMgr, InventoryItemRef itemRef, uint32 newOwnerID)
{
    std::map<uint32, InventoryItemRef> InventoryMap;
    InventoryMap.clear();

    if (itemRef->categoryID() == EVEDB::invCategories::Ship) {
        ShipItemRef shipRef = pSysMgr->GetShipFromInventory(itemRef->itemID());
        if (!shipRef)
            shipRef = m_SvcMgr->item_factory->GetShip(itemRef->itemID());
        if (!shipRef->GetMyInventory()->IsEmpty())
            shipRef->GetMyInventory()->GetInventoryList(InventoryMap);
    } else {
        CargoContainerRef contRef = pSysMgr->GetContainerFromInventory(itemRef->itemID());
        if (!contRef)
            contRef = m_SvcMgr->item_factory->GetCargoContainer(itemRef->itemID());
        if (!contRef->IsEmpty())
            contRef->GetMyInventory()->GetInventoryList(InventoryMap);
    }

    for (auto cur : InventoryMap)
        cur.second->ChangeOwner(newOwnerID, true);
}

PyResult TradeService::Handle_InitiateTrade(PyCallArgs &call) {
    Client* target = nullptr;
    if (call.client->GetTradeSession()) {
        target = sEntityList.FindClientByCharID( call.client->GetTradeSession()->m_tradeSession.herID );
        call.client->SendErrorMsg("You are currently trading with %s.  You can only trade with one player at a time.", target->GetName());
        return nullptr;
    }

    Call_SingleIntegerArg args;
    //    .arg is char to trade with
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }
    target = sEntityList.FindClientByCharID( args.arg );
    if (target->GetTradeSession()) {
        Client* otarget = sEntityList.FindClientByCharID( call.client->GetTradeSession()->m_tradeSession.herID );
        call.client->SendErrorMsg("%s is currently trading with %s.  Try again later.", target->GetName(), otarget->GetName());
        return nullptr;
    }

    // wtf is this ???
    uint32 warID = 0;
    if (call.byname.find("warID") != call.byname.cend())
        if (!call.byname.find("warID")->second->IsNone())
            warID = call.byname.find("warID")->second->AsInt()->value();

    InitiateTradeRsp_NoCash rsp_nc;
        rsp_nc.nodeID    = call.client->services().GetNodeID();
        rsp_nc.stationID = target->GetStationID();
        rsp_nc.myID      = call.client->GetCharacterID();
        rsp_nc.herID     = target->GetCharacterID();
        rsp_nc.money     = 0;
        rsp_nc.state     = 0;
        rsp_nc.when      = Win32TimeNow();

    PyObject* resp = rsp_nc.Encode();
    InitiateTrade(target, resp->Clone());

    return resp;
}

void TradeService::InitiateTrade(Client* pClient, PyRep* resp) {
    PyTuple* tuple = new PyTuple(3);
        tuple->SetItem(0, new PyString("Initiate"));
        tuple->SetItem(1, new PyInt(pClient->GetCharacterID()));
        tuple->SetItem(2, resp);
    // now send it, bypassing the extra shit and wrong dest name added in Client::SendNotification
    pClient->SendNotification("OnTrade", "charid", &tuple);
}

void TradeService::RemoveActiveSession(uint32 myID) {
    std::map<uint32, ActiveSession>::iterator itr = m_activeSessions.find(myID);
    if (itr != m_activeSessions.end())
        m_activeSessions.erase(itr);
}

void TradeService::CancelTrade(Client* pClient) {
    TradeSession* pTSes = pClient->GetTradeSession();
    Client* pOther = sEntityList.FindClientByCharID(pTSes->m_tradeSession.herID);

    TradeBound* pTB = new TradeBound(m_manager);
    pTB->CancelTrade(pClient, pOther, pTSes);

    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyString("Cancel"));
        tuple->SetItem(1, new PyInt(pTSes->m_tradeSession.containerID));
    PyTuple* tuple1 = new PyTuple(2);
        tuple1->SetItem(0, new PyString("Cancel"));
        tuple1->SetItem(1, new PyInt(pTSes->m_tradeSession.containerID));
    // now send it, bypassing the extra shit and wrong dest name added in Client::SendNotification
    pClient->SendNotification("OnTrade", "charid", &tuple);
    pOther->SendNotification("OnTrade", "charid", &tuple1);
    RemoveActiveSession(pTSes->m_tradeSession.myID);
    RemoveActiveSession(pTSes->m_tradeSession.herID);
    SafeDelete(pTSes);
    pClient->ClearTradeSession();
    pOther->ClearTradeSession();
    // returns none
}

DBRowDescriptor* TradeService::CreateHeader() {
    DBRowDescriptor* header = new DBRowDescriptor;
        header->AddColumn( "itemID",     DBTYPE_I8 );
        header->AddColumn( "typeID",     DBTYPE_I4 );
        header->AddColumn( "ownerID",    DBTYPE_I4 );
        header->AddColumn( "locationID", DBTYPE_I8 );
        header->AddColumn( "flagID",     DBTYPE_I2 );
        header->AddColumn( "stacksize",  DBTYPE_I4 );
        header->AddColumn( "groupID",    DBTYPE_I2 );
        header->AddColumn( "singleton",  DBTYPE_BOOL );
        header->AddColumn( "categoryID", DBTYPE_I4 );
        header->AddColumn( "customInfo", DBTYPE_STR );
    return header;
}

uint32 TradeService::GetTradeSessionID()
{
    if (m_SessionID < maxTradeCont)
        return ++m_SessionID;
    else
        return (m_SessionID = minTradeCont);
}
