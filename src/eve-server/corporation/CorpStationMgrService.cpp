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
    Author:        Zhur
*/

#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "chat/LSCService.h"
#include "corporation/CorpStationMgrService.h"

class CorpStationMgrIMBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(CorpStationMgrIMBound)

    CorpStationMgrIMBound(PyServiceMgr *mgr, CorporationDB& db, uint32 station_id)
    : PyBoundObject(mgr),
      m_dispatch(new Dispatcher(this)),
      m_db(db),
      m_stationID(station_id)
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "CorpStationMgrIMBound";

        //PyCallable_REG_CALL(CorpStationMgrIMBound, GetEveOwners);
        PyCallable_REG_CALL(CorpStationMgrIMBound, GetCorporateStationInfo);
        PyCallable_REG_CALL(CorpStationMgrIMBound, DoStandingCheckForStationService);
        PyCallable_REG_CALL(CorpStationMgrIMBound, GetPotentialHomeStations);
        PyCallable_REG_CALL(CorpStationMgrIMBound, SetHomeStation);
        PyCallable_REG_CALL(CorpStationMgrIMBound, SetCloneTypeID);
        PyCallable_REG_CALL(CorpStationMgrIMBound, GetQuoteForRentingAnOffice);
        PyCallable_REG_CALL(CorpStationMgrIMBound, GetNumberOfUnrentedOffices);
        //testing
        PyCallable_REG_CALL(CorpStationMgrIMBound, RentOffice);
        PyCallable_REG_CALL(CorpStationMgrIMBound, GetStationOffices);
        PyCallable_REG_CALL(CorpStationMgrIMBound, GetCorporateStationOffice);
        PyCallable_REG_CALL(CorpStationMgrIMBound, MoveCorpHQHere);
    }
    virtual ~CorpStationMgrIMBound() { delete m_dispatch; }
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    //PyCallable_DECL_CALL(GetEveOwners);
    PyCallable_DECL_CALL(GetCorporateStationInfo);
    PyCallable_DECL_CALL(DoStandingCheckForStationService);
    PyCallable_DECL_CALL(GetPotentialHomeStations);
    PyCallable_DECL_CALL(SetHomeStation);
    PyCallable_DECL_CALL(SetCloneTypeID);
    PyCallable_DECL_CALL(GetQuoteForRentingAnOffice);
    PyCallable_DECL_CALL(RentOffice);
    PyCallable_DECL_CALL(GetStationOffices);
    PyCallable_DECL_CALL(GetNumberOfUnrentedOffices);
    //testing
    PyCallable_DECL_CALL(GetCorporateStationOffice);
    PyCallable_DECL_CALL(MoveCorpHQHere);

protected:
    Dispatcher *const m_dispatch;

    CorporationDB& m_db;    //we do not own this
    const uint32 m_stationID;
};

PyCallable_Make_InnerDispatcher(CorpStationMgrService)

CorpStationMgrService::CorpStationMgrService(PyServiceMgr *mgr)
: PyService(mgr, "corpStationMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(CorpStationMgrService, GetStationServiceStates)
    //PyCallable_REG_CALL(CorpStationMgrService, GetImprovementStaticData)
}

CorpStationMgrService::~CorpStationMgrService() {
    delete m_dispatch;
}


PyBoundObject *CorpStationMgrService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
    if(!bind_args->IsInt()) {
        codelog(SERVICE__ERROR, "%s Service: invalid bind argument type %s", GetName(), bind_args->TypeString());
        return nullptr;
    }
    return new CorpStationMgrIMBound( m_manager, m_db, bind_args->AsInt()->value() );
}

PyResult CorpStationMgrIMBound::Handle_GetCorporateStationInfo(PyCallArgs &call) {
    /* returns:
     *  list(
     *      eveowners:
     *          rowset: ownerID,ownerName,typeID
     *      corporations:
     *          rowset:corporationID,corporationName,description,shares,graphicID,memberCount,ceoID,stationID,raceID,corporationType,creatorID,hasPlayerPersonnelManager,tickerName,sendCharTerminationMessage,shape1,shape2,shape3,color1,color2,color3,typeface,memberLimit,allowedMemberRaceIDs,url,taxRate,minimumJoinStanding,division1,division2,division3,division4,division5,division6,division7,allianceID
     *      offices (may be None):
     *          rowset: corporationID,itemID,officeFolderID
     *  )
     */

    PyList *l = new PyList();

    PyRep *tmp;

    tmp = m_db.ListStationOwners(m_stationID);
    if(tmp == NULL) {
        codelog(SERVICE__ERROR, "Failed to get owners.");
        return nullptr;
    }
    l->AddItem( tmp );

    tmp = m_db.ListStationCorps(m_stationID);
    if(tmp == NULL) {
        codelog(SERVICE__ERROR, "Failed to get corps");
        return nullptr;
    }
    l->AddItem( tmp );

    tmp = m_db.ListStationOffices(m_stationID);
    if(tmp == NULL) {
        codelog(SERVICE__ERROR, "Failed to get offices.");
        return nullptr;
    }
    l->AddItem( tmp );

    return(l);

/*
#warning still using a hacked cache file here!

    std::string abs_fname = "../data/cache/fgAAAAAsLBAOY29ycFN0YXRpb25NZ3IERJiTAxAXR2V0Q29ycG9yYXRlU3RhdGlvbkluZm8.cache";

    PySubStream *ss = new PySubStream();

    if(!m_manager->GetCache()->LoadCachedFile(abs_fname.c_str(), "GetCorporateStationInfo", ss)) {
        _log(CLIENT__ERROR, "GetCorporateStationInfo Failed to load cache file '%s'", abs_fname.c_str());
        ss->decoded = new PyNone();
    } else {
        //hack:
        ss->length -= 82;
        uint8 *d = new uint8[ss->length];
        memcpy(d, ss->data+82, ss->length);
        delete ss->data;
        delete ss->decoded;
        ss->data = d;
        ss->decoded = NULL;
    }

    return(ss);*/
}


PyResult CorpStationMgrIMBound::Handle_DoStandingCheckForStationService(PyCallArgs &call) {
    //   corpStationMgr.DoStandingCheckForStationService(stationServiceID)
    /*
     * 23:09:41 L Server: DoStandingCheckForStationService call made to
     * 23:09:41 L CorpStationMgrIMBound::Handle_DoStandingCheckForStationService(): size= 1
     * 23:09:41 [SvcCall]   Call Arguments:
     * 23:09:41 [SvcCall]       Tuple: 1 elements
     * 23:09:41 [SvcCall]         [ 0] Integer field: 8192
     * 23:09:41 L Server: DoStandingCheckForStationService call made to
     * 23:09:41 L CorpStationMgrIMBound::Handle_DoStandingCheckForStationService(): size= 1
     * 23:09:41 [SvcCall]   Call Arguments:
     * 23:09:41 [SvcCall]       Tuple: 1 elements
     * 23:09:41 [SvcCall]         [ 0] Integer field: 16384
     * 18:49:22 L CorpStationMgrIMBound::Handle_DoStandingCheckForStationService(): size= 1
     * 18:49:22 [SvcCall]   Call Arguments:
     * 18:49:22 [SvcCall]       Tuple: 1 elements
     * 18:49:22 [SvcCall]         [ 0] Integer field: 65536
     *
     */
    sLog.Log( "CorpStationMgrIMBound::Handle_DoStandingCheckForStationService()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    // takes an int (seen 512 and 1024 and 2048)
    //seems to return None, or throw an exception

    PyRep *result = new PyNone();

    return result;
}

PyResult CorpStationMgrIMBound::Handle_GetPotentialHomeStations(PyCallArgs &call) {
    PyRep *result = NULL;
    //returns a rowset: stationID, typeID

    _log(CLIENT__ERROR, "Hacking GetPotentialHomeStations");
    result = m_db.ListCorpStations(call.client->GetCorporationID());

    return result;
}

PyResult CorpStationMgrIMBound::Handle_SetHomeStation(PyCallArgs &call) {

    //this takes an integer: stationID
    //price is prompted for on the client side.

    sLog.Debug( "CorpStationMgrIMBound", "Called SetHomeStation stub." );

    return new PyNone;
}

PyResult CorpStationMgrIMBound::Handle_SetCloneTypeID(PyCallArgs &call) {

    //this takes an integer: cloneTypeID
    //price is prompted for on the client side.

    Call_SetCloneTypeID arg;
    if(!arg.Decode(&call.tuple)){
        sLog.Debug("CoporationMgrIMBound","Failed to determine Clone Type");
    }

    //Get cost of clone
    int cost = m_db.GetCloneTypeCostByID(arg.CloneTypeID);

    //Check if player has enough money
    if(call.client->GetBalance() > cost) {
        //subtract amount
        call.client->AddBalance(-cost);
    }

    //update type of clone
    CharacterDB c_db;
    c_db.ChangeCloneType(call.client->GetCharacterID(), arg.CloneTypeID);

    //sLog.Debug( "CorpStationMgrIMBound", "Called SetCloneTypeID stub." );

    return new PyNone;
}

PyResult CorpStationMgrIMBound::Handle_GetQuoteForRentingAnOffice(PyCallArgs &call) {
    // No incoming params...
    uint32 stationID = call.client->GetStationID();

    // Unless I produce an invalid ISK value (probably a NAN), this won't fail,
    // the dialog box will be displayed... have to make sure this doesn't fail
    return (new PyInt(m_db.GetQuoteForRentingAnOffice(stationID)));
}

PyResult CorpStationMgrIMBound::Handle_RentOffice(PyCallArgs &call) {
    // 1 param, corp rent price    //TODO: check against what we think it should cost.
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Wrong incoming param in RentOffice");
        return nullptr;
    }

    uint32 location = call.client->GetLocationID();

    // check if the corp has enough money
    double corpBalance = m_db.GetCorpBalance(call.client->GetCorporationID(), accountingKeyCash);  //FIXME  get proper corp wallet division
    if (corpBalance < arg.arg) {
        _log(SERVICE__ERROR, "%s: Corp doesn't have enough money to rent an office.", call.client->GetName());
        return nullptr;
    }

    // We should also check if the station has a free office atm...
    OfficeInfo oInfo(call.client->GetCorporationID(), call.client->GetStationID());

    if (!oInfo.officeID) {
        codelog(SERVICE__ERROR, "%s: Error at renting a new office", call.client->GetName());
        return nullptr;
    }
    oInfo.officeID = m_db.ReserveOffice(oInfo);
    // should we also put this into the entity table?

    /*
     *    Broadcast #1
     *
     *        [PyString "OnAccountChange"]
     *        [PyList 0 items]
     *        [PyString "*corpid&corpAccountKey"]
     *    [PyInt 5654387]
     *    [PyTuple 1 items]
     *      [PyTuple 2 items]
     *        [PyInt 0]
     *        [PySubStream 31 bytes]
     *          [PyTuple 2 items]
     *            [PyInt 0]
     *            [PyTuple 2 items]
     *              [PyInt 1]
     *              [PyTuple 3 items]
     *                [PyString "cash"]
     *                [PyInt 98038978]
     *                [PyFloat 9017992.75] */
    /*
     *    Broadcast #2
     *        [PyString "OnNotificationReceived"]
     *        [PyList 0 items]
     *        [PyString "clientID"]
     *    [PyInt 5654387]
     *    [PyTuple 1 items]
     *      [PyTuple 2 items]
     *        [PyInt 0]
     *        [PySubStream 168 bytes]
     *          [PyTuple 2 items]
     *            [PyInt 0]
     *            [PyTuple 2 items]
     *              [PyInt 1]
     *              [PyTuple 5 items]
     *                [PyInt 342402174]
     *                [PyInt 10]
     *                [PyInt 1000167]
     *                [PyIntegerVar 129492968400000000]
     *                [PyDict 8 kvp]
     *                  [PyString "debtorID"]
     *                  [PyInt 98038978]
     *                  [PyString "creditorID"]
     *                  [PyInt 1000167]
     *                  [PyString "billTypeID"]
     *                  [PyInt 2]
     *                  [PyString "amount"]
     *                  [PyInt 981907]
     *                  [PyString "externalID2"]
     *                  [PyInt 60014683]
     *                  [PyString "externalID"]
     *                  [PyInt 27]
     *                  [PyString "currentDate"]
     *                  [PyIntegerVar 129492968683459696]
     *                  [PyString "dueDate"]
     *                  [PyIntegerVar 129518888683422295]
     *    [PyDict 1 kvp]
     *      [PyString "sn"]
     *      [PyIntegerVar 4]
     */
    /*
     *    Broadcast #3
     *        [PyString "OnItemsChanged"]
     *        [PyList 0 items]
     *        [PyString "*stationid&corpid"]
     *    [PyInt 5654387]
     *    [PyTuple 1 items]
     *      [PyTuple 2 items]
     *        [PyInt 0]
     *        [PySubStream 177 bytes]
     *          [PyTuple 2 items]
     *            [PyInt 0]
     *            [PyTuple 2 items]
     *              [PyInt 1]
     *              [PyTuple 2 items]
     *                [PyList 1 items]
     *                  [PyPackedRow 33 bytes]
     *                    ["itemID" => <176312294> [I8]]
     *                    ["typeID" => <27> [I4]]
     *                    ["ownerID" => <98038978> [I4]]
     *                    ["locationID" => <66014684> [I8]]
     *                    ["flagID" => <81> [I2]]
     *                    ["quantity" => <-1> [I4]]
     *                    ["groupID" => <16> [I2]]
     *                    ["categoryID" => <3> [I2]]
     *                    ["customInfo" => <empty string> [Str]]
     *                [PyDict 1 kvp]
     *                  [PyInt 2]
     *                  [PyInt 4]
     */
    /*
     *    Broadcast #4
     *        [PyString "OnOfficeRentalChanged"]
     *        [PyList 0 items]
     *        [PyString "stationid"]
     *    [PyInt 5654387]
     *    [PyTuple 1 items]
     *      [PyTuple 2 items]
     *        [PyInt 0]
     *        [PySubStream 27 bytes]
     *          [PyTuple 2 items]
     *            [PyInt 0]
     *            [PyTuple 2 items]
     *              [PyInt 1]
     *              [PyTuple 3 items]
     *                [PyInt 98038978]
     *                [PyIntegerVar 176312294]
     *                [PyInt 66014684]
     *    [PyNone]
     *
     */
    /*
     *    Broadcast #5
     *
     *    -- actual response from call
     *    [PyTuple 1 items]
     *      [PySubStream 11 bytes]
     *        [PyIntegerVar 176312294]    << office item id
     *    [PyNone]
     */
    /*
     *    Broadcast #6
     *    -- this should go to all docked players in this station
     *        [PyString "OnObjectPublicAttributesUpdated"]
     *        [PyList 0 items]
     *        [PyString "objectID"]
     *    [PyInt 5654387]
     *    [PyTuple 1 items]
     *      [PyTuple 2 items]
     *        [PyInt 0]
     *        [PySubStream 178 bytes]
     *          [PyTuple 2 items]
     *            [PyInt 0]
     *            [PyTuple 2 items]
     *              [PyInt 1]
     *              [PyTuple 4 items]
     *                [PyString "N=698477:223415"]
     *                [PyDict 1 kvp]
     *                  [PyString "realRowCount"]
     *                  [PyInt 1]
     *                [PyTuple 0 items]
     *                [PyDict 4 kvp]
     *                  [PyString "partial"]
     *                  [PyList 1 items]
     *                    [PyString "realRowCount"]
     *                  [PyString "notificationParams"]
     *                  [PyDict 0 kvp]
     *                  [PyString "change"]
     *                  [PyDict 4 kvp]
     *                    [PyString "typeID"]
     *                    [PyTuple 2 items]
     *                      [PyNone]
     *                      [PyInt 1529]
     *                    [PyString "stationID"]
     *                    [PyTuple 2 items]
     *                      [PyNone]
     *                      [PyInt 60014683]
     *                    [PyString "officeFolderID"]
     *                    [PyTuple 2 items]
     *                      [PyNone]
     *                      [PyInt 66014684]
     *                    [PyString "officeID"]
     *                    [PyTuple 2 items]
     *                      [PyNone]
     *                      [PyIntegerVar 176312294]
     *                  [PyString "changePKIndexValue"]
     *                  [PyInt 60014683]
     *    [PyNone]
     */

    // Now we have the new office, let's update the officelist... if we have to...

        Notify_OnObjectPublicAttributesUpdated N_pau;
        OfficeAttributeUpdate change;

        // This way we can get the current bounded object's boundID
        N_pau.realRowCount = 2;
        N_pau.bindID = GetBindStr();
        N_pau.changePKIndexValue = oInfo.stationID;

        change.newOfficeFolderID = oInfo.officeFolderID;
        change.newOfficeID = oInfo.officeID;
        change.newStationID = oInfo.stationID;
        change.newTypeID = oInfo.typeID;
        N_pau.changes = change.Encode();

        PyTuple * res1 = N_pau.Encode(); // This is good enough as there are no old values atm
        // Who has to know about this public object's update?

        // This has to be sent to everyone in the station
        // For now, broadcast it
        sEntityList.Multicast("OnObjectPublicAttributesUpdated", "objectID", &res1, NOTIF_DEST__LOCATION, location, false);


    // remove the money
    m_db.AddBalanceToCorp(oInfo.corporationID, -double(arg.arg));
    corpBalance -= arg.arg;    // This is the new corp money. Do I have to make a casting here?
    // record the transaction
    m_db.GiveCash(oInfo.corporationID, refOfficeRentalFee, oInfo.corporationID, oInfo.stationID, "unknown", call.client->GetUserID(), accountingKeyCash, -double(arg.arg), corpBalance, "Renting office for 30 days");

    MulticastTarget mct;
    mct.corporations.insert(oInfo.corporationID);
    OnAccountChange ac;
    ac.accountKey = "cash";
    ac.ownerid = oInfo.corporationID; //call.client->GetCharacterID();
    ac.balance = corpBalance;
    PyTuple *res2 = ac.Encode();
    sEntityList.Multicast("OnAccountChange", "*corpid&corpAccountKey", &res2, mct);

    // This was the second notification


    // Now comes an OnItemChange notification, no need to do anything server-side
    util_Row Noic_row;

    Noic_row.header.push_back( "itemID" );
    Noic_row.header.push_back( "typeID" );
    Noic_row.header.push_back( "ownerID" );
    Noic_row.header.push_back( "locationID" );
    Noic_row.header.push_back( "flag" );
    Noic_row.header.push_back( "contraband" );
    Noic_row.header.push_back( "singleton" );
    Noic_row.header.push_back( "quantity" );
    Noic_row.header.push_back( "groupID" );
    Noic_row.header.push_back( "categoryID" );
    Noic_row.header.push_back( "customInfo" );

    Noic_row.line = new PyList;
    Noic_row.line->AddItemInt( oInfo.officeID );
    Noic_row.line->AddItemInt( 27 );
    Noic_row.line->AddItemInt( ac.ownerid );
    Noic_row.line->AddItemInt( oInfo.officeFolderID );
    Noic_row.line->AddItemInt( flagOfficeSlot1 );
    Noic_row.line->AddItemInt( 0 );
    Noic_row.line->AddItemInt( 1 );
    Noic_row.line->AddItemInt( 1 );
    Noic_row.line->AddItemInt( EVEDB::invGroups::Station_Services );
    Noic_row.line->AddItemInt( EVEDB::invCategories::Station );
    Noic_row.line->AddItem( new PyNone );

    NotifyOnItemChange Noic;
    Noic.itemRow = Noic_row.Encode();
    Noic.changes[ixOwnerID] = new PyInt( 4 );

    PyTuple* res3 = Noic.Encode();
    // This is a possible broadcast-candidate
    sEntityList.Multicast("OnItemChange", "*stationid&corpid", &res3, NOTIF_DEST__LOCATION, location, false);

    // End of the third notification


    // Next is OnOfficeRentalChange, still no job on server-side

    Notify_OnOfficeRentalChanged N_oorc;
    N_oorc.ownerID = ac.ownerid;
    N_oorc.officeID = oInfo.officeID;
    N_oorc.officeFolderID = oInfo.officeFolderID;

    PyTuple * res4 = N_oorc.Encode(); // No need for fastencode, no null values
    // This is definately a broadcast-candidate
    sEntityList.Multicast("OnOfficeRentalChanged", "stationid", &res4, NOTIF_DEST__LOCATION, location);

    // End of the fourth notification


    // OnBillReceived, an essentially empty tuple, just to tell the client that there is something,
    // maybe for blinking purpose?

    Notify_OnBillReceived N_obr;
    PyTuple * res5 = N_obr.Encode();
    call.client->SendNotification("OnBillReceived", "*corpid&corprole", &res5, false);
    // Why do we create a bill, when the office is already paid? Maybe that's why it's empty...


    // End of the fifth notification


    // OnMessage notification, the LSC packet NotifyOnMessage can be used, along with the StoreNewEVEMail
    // Who to send notification? corpRoleJuniorAccountant and equiv? atm it's enough to send it to the renter
    // TODO: get the correct evemail content from somewhere
    // TODO: send it to every corp member who's affected by it. corpRoleAccountant, corpRoleJuniorAccountant or equiv
    m_manager->lsc_service->SendMail(
        m_db.GetStationCorporationCEO(oInfo.stationID),
        call.client->GetCharacterID(),
        "Bill issued",
        "Bill issued for renting an office");

    // End of the sixth notification, so far so good...


    // One last thing: create that damn bill somewhere soon... an example would be nice...

    return (new PyInt(oInfo.officeID));
}

PyResult CorpStationMgrIMBound::Handle_GetStationOffices( PyCallArgs& call )
{
    sLog.Log( "CorpStationMgrIMBound::Handle_GetStationOffices()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    //Hack: Just passing the client an empty PyList to stop it throwing an exception.
    //TODO: Fid out what needs to be in the PyList and when to send it.
    sLog.Debug( "CorpStationMgrIMBound", "Called GetStationOffices stub." );
    /*
    [PySubStream 99 bytes]
        [PyObjectData Name: objectCaching.CachedMethodCallResult]
          [PyTuple 3 items]
            [PyDict 1 kvp]
              [PyString "versionCheck"]
              [PyTuple 3 items]
                [PyString "always"]
                [PyNone]
                [PyNone]
            [PySubStream 6 bytes]
              [PyList 0 items]
            [PyList 2 items] //Cache Information
              [PyIntegerVar 129533580031608440] //Timestamp (fileTime)
              [PyInt 52428965] //Hash??


      [PySubStream 569 bytes]
        [PyObjectData Name: objectCaching.CachedMethodCallResult]
          [PyTuple 3 items]
            [PyDict 1 kvp]
              [PyString "versionCheck"]
              [PyTuple 3 items]
                [PyString "always"]
                [PyNone]
                [PyNone]
            [PySubStream 472 bytes]
              [PyObjectEx Type2]
                [PyTuple 2 items]
                  [PyTuple 1 items]
                    [PyToken dbutil.CRowset]
                  [PyDict 1 kvp]
                    [PyString "header"]
                    [PyObjectEx Normal]
                      [PyTuple 2 items]
                        [PyToken blue.DBRowDescriptor]
                        [PyTuple 1 items]
                          [PyTuple 3 items]
                            [PyTuple 2 items]
                              [PyString "corporationID"]
                              [PyInt 3]
                            [PyTuple 2 items]
                              [PyString "itemID"]
                              [PyInt 20]
                            [PyTuple 2 items]
                              [PyString "officeFolderID"]
                              [PyInt 20]
                [PyPackedRow 21 bytes]
                  ["corporationID" => <98035543> [I4]]
                  ["itemID" => <152212018> [I8]]
                  ["officeFolderID" => <66014684> [I8]]
                [PyPackedRow 21 bytes]
                  ["corporationID" => <1337582783> [I4]]
                  ["itemID" => <153600468> [I8]]
                  ["officeFolderID" => <66014684> [I8]]
                [PyPackedRow 21 bytes]
                  ["corporationID" => <1238908264> [I4]]
                  ["itemID" => <164730922> [I8]]
                  ["officeFolderID" => <66014684> [I8]]
            [PyList 2 items]
              [PyIntegerVar 129492958706190905]
              [PyInt -1622429963]
              */
    PyTuple * arg_tuple = new PyTuple(3);



    PyDict* itr_1 = new PyDict();
    itr_1->SetItem("versionCheck", new_tuple("always", new PyNone, new PyNone));

    arg_tuple->SetItem(0, itr_1);
    arg_tuple->SetItem(1, new PySubStream( new PyList() ) );
    arg_tuple->SetItem(2, new_tuple(new PyLong(129533580031608440LL), new PyInt(52428965) ) );
    return new PyObject( "objectCaching.CachedMethodCallResult", arg_tuple );
}

PyResult CorpStationMgrIMBound::Handle_GetNumberOfUnrentedOffices( PyCallArgs &call )
{
    sLog.Log( "CorpStationMgrIMBound::Handle_GetNumberOfUnrentedOffices()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    //TODO: add handler that queries the data from the StationType struct.  Not exactly sure how to do this,
    //        but will involve call.client->GetStationID as the arguments to StationType.officeSlots() (hopefully)
    return new PyInt(3);
}

PyResult CorpStationMgrIMBound::Handle_GetCorporateStationOffice(PyCallArgs &call) {
  /*
14:09:26 L CorpStationMgrIMBound::Handle_GetCorporateStationOffice(): size= 0
14:09:26 [SvcCall]   Call Arguments:
14:09:26 [SvcCall]       Tuple: Empty
14:09:26 [SvcCall]   Call Named Arguments:
14:09:26 [SvcCall]     Argument 'machoVersion':
14:09:26 [SvcCall]         Integer field: 1
  sLog.Log( "CorpStationMgrIMBound::Handle_GetCorporateStationOffice()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);
*/
    return new PyTuple(0);
}

PyResult CorpStationMgrIMBound::Handle_MoveCorpHQHere(PyCallArgs &call)
{
  sLog.Log( "CorpStationMgrIMBound::Handle_MoveCorpHQHere()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    return new PyTuple(0);

    /* client error CorpHQIsAtThisStation */
}

PyResult CorpStationMgrService::Handle_GetStationServiceStates(PyCallArgs &call)
{
    /*   i *THINK* this is only sent for outposts.....stationID is 61m (above static stations)
     * since it has NOT been called yet on evemu, it very well could be outposts only (cause we dont have any)
     *
     *  **UPDATE**
     *   i was right.  found this in code...
     * if util.IsOutpost(eve.session.stationid) or sm.GetService('godma').GetType(eve.stationItem.stationTypeID).isPlayerOwnable == 1:
     *     self.serviceItemsState = sm.RemoteSvc('corpStationMgr').GetStationServiceStates()
     *
     *
     * ==================== Sent from Client 98 bytes
     *
     * [PyObjectData Name: macho.CallReq]
     *  [PyTuple 6 items]
     *    [PyInt 6]
     *    [PyObjectData Name: macho.MachoAddress]
     *      [PyTuple 4 items]
     *        [PyInt 2]
     *        [PyInt 0]
     *        [PyIntegerVar 45]
     *        [PyNone]
     *    [PyObjectData Name: macho.MachoAddress]
     *      [PyTuple 3 items]
     *        [PyInt 8]
     *        [PyString "corpStationMgr"]
     *        [PyNone]
     *    [PyInt 5654387]
     *    [PyTuple 1 items]
     *      [PyTuple 2 items]
     *        [PyInt 0]
     *        [PySubStream 39 bytes]
     *          [PyTuple 4 items]
     *            [PyInt 1]
     *            [PyString "GetStationServiceStates"]
     *            [PyTuple 0 items]
     *            [PyDict 1 kvp]
     *              [PyString "machoVersion"]
     *              [PyInt 1]
     *    [PyNone]
     *
     *
     *
     * ==================== Sent from Server 347 bytes
     *
     * [PyObjectData Name: macho.CallRsp]
     *  [PyTuple 6 items]
     *    [PyInt 7]
     *    [PyObjectData Name: macho.MachoAddress]
     *      [PyTuple 3 items]
     *        [PyInt 8]
     *        [PyString "corpStationMgr"]
     *        [PyNone]
     *    [PyObjectData Name: macho.MachoAddress]
     *      [PyTuple 4 items]
     *        [PyInt 2]
     *        [PyIntegerVar 15001000001023]
     *        [PyIntegerVar 45]
     *        [PyNone]
     *    [PyInt 5654387]
     *    [PyTuple 1 items]
     *      [PySubStream 279 bytes]
     *        [PyDict 6 kvp]
     *          [PyInt 512]
     *          [PyObjectData Name: util.Row]
     *            [PyDict 2 kvp]
     *              [PyString "header"]
     *              [PyList 5 items]
     *                [PyString "solarSystemID"]
     *                [PyString "stationID"]
     *                [PyString "serviceID"]
     *                [PyString "stationServiceItemID"]
     *                [PyString "isEnabled"]
     *              [PyString "line"]
     *              [PyList 5 items]
     *                [PyInt 30001984]
     *                [PyInt 61000012]              << stationID 61m = outpost
     *                [PyInt 512]
     *                [PyIntegerVar 318021030]
     *                [PyInt 1]
     *          [PyInt 16384]
     *          [PyObjectData Name: util.Row]
     *            [PyDict 2 kvp]
     *              [PyString "header"]
     *              [PyList 5 items]
     *                [PyString "solarSystemID"]
     *                [PyString "stationID"]
     *                [PyString "serviceID"]
     *                [PyString "stationServiceItemID"]
     *                [PyString "isEnabled"]
     *              [PyString "line"]
     *              [PyList 5 items]
     *                [PyInt 30001984]
     *                [PyInt 61000012]
     *                [PyInt 16384]
     *                [PyIntegerVar 318021027]
     *                [PyInt 1]
     *          [PyInt 4096]
     *          [PyObjectData Name: util.Row]
     *            [PyDict 2 kvp]
     *              [PyString "header"]
     *              [PyList 5 items]
     *                [PyString "solarSystemID"]
     *                [PyString "stationID"]
     *                [PyString "serviceID"]
     *                [PyString "stationServiceItemID"]
     *                [PyString "isEnabled"]
     *              [PyString "line"]
     *              [PyList 5 items]
     *                [PyInt 30001984]
     *                [PyInt 61000012]
     *                [PyInt 4096]
     *                [PyIntegerVar 318021029]
     *                [PyInt 1]
     *          [PyInt 8192]
     *          [PyObjectData Name: util.Row]
     *            [PyDict 2 kvp]
     *              [PyString "header"]
     *              [PyList 5 items]
     *                [PyString "solarSystemID"]
     *                [PyString "stationID"]
     *                [PyString "serviceID"]
     *                [PyString "stationServiceItemID"]
     *                [PyString "isEnabled"]
     *              [PyString "line"]
     *              [PyList 5 items]
     *                [PyInt 30001984]
     *                [PyInt 61000012]
     *                [PyInt 8192]
     *                [PyIntegerVar 318021028]
     *                [PyInt 1]
     *          [PyInt 16]
     *          [PyObjectData Name: util.Row]
     *            [PyDict 2 kvp]
     *              [PyString "header"]
     *              [PyList 5 items]
     *                [PyString "solarSystemID"]
     *                [PyString "stationID"]
     *                [PyString "serviceID"]
     *                [PyString "stationServiceItemID"]
     *                [PyString "isEnabled"]
     *              [PyString "line"]
     *              [PyList 5 items]
     *                [PyInt 30001984]
     *                [PyInt 61000012]
     *                [PyInt 16]
     *                [PyIntegerVar 1002331174723]
     *                [PyInt 1]
     *          [PyInt 65536]
     *          [PyObjectData Name: util.Row]
     *            [PyDict 2 kvp]
     *              [PyString "header"]
     *              [PyList 5 items]
     *                [PyString "solarSystemID"]
     *                [PyString "stationID"]
     *                [PyString "serviceID"]
     *                [PyString "stationServiceItemID"]
     *                [PyString "isEnabled"]
     *              [PyString "line"]
     *              [PyList 5 items]
     *                [PyInt 30001984]
     *                [PyInt 61000012]
     *                [PyInt 65536]
     *                [PyIntegerVar 318021026]
     *                [PyInt 1]
     *    [PyNone]
     */
  sLog.Log( "CorpStationMgrService::Handle_GetStationServiceStates()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    return new PyTuple(0);
}
