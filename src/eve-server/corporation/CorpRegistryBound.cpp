/*
 *
 *
 *
 */

#include <string>

#include "EVE_Corp.h"
#include "CorpData.h"
#include "StaticDataMgr.h"
#include "account/AccountService.h"
#include "cache/ObjCacheService.h"
#include "chat/LSCService.h"
#include "corporation/CorpRegistryBound.h"
#include "station/StationDB.h"
#include "station/StationDataMgr.h"

class SparseCorpOfficeListBound
: public PyBoundObject
{
public:
    // or CorpRegistryBound?
    PyCallable_Make_Dispatcher(SparseCorpOfficeListBound)

    SparseCorpOfficeListBound(PyServiceMgr *mgr, CorporationDB& db)
    : PyBoundObject(mgr),
    m_dispatch(new Dispatcher(this)),
    m_db(db)
    {
        _SetCallDispatcher(m_dispatch);

        PyCallable_REG_CALL(SparseCorpOfficeListBound, Fetch);
        PyCallable_REG_CALL(SparseCorpOfficeListBound, SelectByUniqueColumnValues);
        //PyCallable_REG_CALL(SparseCorpOfficeListBound, FetchByKey)
        //PyCallable_REG_CALL(SparseCorpOfficeListBound, GetByKey)
    }
    virtual ~SparseCorpOfficeListBound() {delete m_dispatch;}
    virtual void Release() {
        delete this;
    }

    PyCallable_DECL_CALL(Fetch); //(startPos, fetchSize)
    PyCallable_DECL_CALL(SelectByUniqueColumnValues);
    //PyCallable_DECL_CALL(FetchByKey) //([keys])
    //PyCallable_DECL_CALL(GetByKey) //(key)


protected:
    Dispatcher *const m_dispatch;

    CorporationDB& m_db;
};

PyResult SparseCorpOfficeListBound::Handle_Fetch(PyCallArgs &call) {
    sLog.White( "SparseCorpOfficeListBound::Handle_Fetch()", "size= %u", call.tuple->size() );
    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    return m_db.Fetch(call.client->GetCorporationID(), args.arg1, args.arg2);
}

// this is called in a few places, to get officeID, but
//   uses a method chain from GetOffices() (from bound office object) and is only called when accessing containers and item(s) are owned by corp.
//  however, current code requires for these items to show in containers' inventory, they have to be owned by the hangar's owner
PyResult SparseCorpOfficeListBound::Handle_SelectByUniqueColumnValues(PyCallArgs &call) {
    //offices = sm.GetService('corp').GetMyCorporationsOffices().SelectByUniqueColumnValues('stationID', [quoteData.containerID])
    //   rows = sm.StartService('corp').GetMyCorporationsOffices().SelectByUniqueColumnValues('officeID', [invItem.locationID])

    /*      the following code iterates thru the 'offices' return, but appears to send only one set of data (list of dict?)
     *
                        for office in offices:
                            if quoteData.containerID == office.stationID:
                                officeFolderID = office.officeFolderID
                                officeID = office.officeID
     */
    /*
     * 00:31:23 W SparseCorpOfficeListBound::Handle_SelectByUniqueColumnValues(): size= 2
     * 00:31:23 [CorpCallDump]   Call Arguments:
     * 00:31:23 [CorpCallDump]      Tuple: 2 elements
     * 00:31:23 [CorpCallDump]       [ 0]     String: 'officeID'        << could also be 'stationID'
     * 00:31:23 [CorpCallDump]       [ 1]   List: 1 elements
     * 00:31:23 [CorpCallDump]       [ 1]   [ 0]    Integer: 100000003
     */
    sLog.White( "SparseCorpOfficeListBound::Handle_SelectByUniqueColumnValues()", "size= %u", call.tuple->size() );
    call.Dump(CORP__WARNING);   //so this will dump on all calls

    std::vector<OfficeData> data;
    std::string str = PyRep::StringContent(call.tuple->GetItem(0));
    uint32 locationID = PyRep::IntegerValue(call.tuple->GetItem(1)->AsList()->GetItem(0));

    if (str.compare("officeID") == 0)
        stDataMgr.GetStationOfficeIDs(locationID, data);    // this method checks for the type of locationID sent.
    else if (str.compare("stationID") == 0)
        stDataMgr.GetStationOfficeIDs(locationID, data);
    else
        ;  // make error here?

    PyList* list = new PyList();
    for (auto cur : data) {
        PyDict* dict = new PyDict();
        dict->SetItemString("officeID", new PyInt(cur.officeID));
        dict->SetItemString("officeFolderID", new PyInt(cur.folderID));
        dict->SetItemString("stationID", new PyInt(cur.stationID));
        list->AddItem(new PyObject("util.KeyVal", dict));
    }

    list->Dump(CORP__RSP_DUMP, "    ");
    return list;
}

/*
 * CORP__ERROR
 * CORP__WARNING
 * CORP__INFO
 * CORP__MESSAGE
 * CORP__TRACE
 * CORP__CALL
 * CORP__CALL_DUMP
 * CORP__RSP_DUMP
 * CORP__DB_ERROR
 * CORP__DB_WARNING
 * CORP__DB_INFO
 * CORP__DB_MESSAGE
 */

CorpRegistryBound::CorpRegistryBound(PyServiceMgr *mgr, CorporationDB& db, uint32 corpID)
: PyBoundObject(mgr),
 m_db(db),
 m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    m_strBoundObjectName = "CorpRegistryBound";

    PyCallable_REG_CALL(CorpRegistryBound, GetEveOwners);
    PyCallable_REG_CALL(CorpRegistryBound, GetCorporation);
    PyCallable_REG_CALL(CorpRegistryBound, GetCorporations);
    PyCallable_REG_CALL(CorpRegistryBound, GetInfoWindowDataForChar);
    PyCallable_REG_CALL(CorpRegistryBound, GetLockedItemLocations);
    PyCallable_REG_CALL(CorpRegistryBound, AddCorporation);
    PyCallable_REG_CALL(CorpRegistryBound, GetSuggestedTickerNames);
    PyCallable_REG_CALL(CorpRegistryBound, GetOffices);
    PyCallable_REG_CALL(CorpRegistryBound, GetStations);

    PyCallable_REG_CALL(CorpRegistryBound, CreateRecruitmentAd);
    PyCallable_REG_CALL(CorpRegistryBound, GetRecruiters);
    PyCallable_REG_CALL(CorpRegistryBound, GetRecruitmentAdsForCorporation);
    PyCallable_REG_CALL(CorpRegistryBound, GetMyApplications);
    PyCallable_REG_CALL(CorpRegistryBound, InsertApplication);
    PyCallable_REG_CALL(CorpRegistryBound, GetApplications);
    PyCallable_REG_CALL(CorpRegistryBound, UpdateApplicationOffer);
    PyCallable_REG_CALL(CorpRegistryBound, DeleteApplication);
    PyCallable_REG_CALL(CorpRegistryBound, UpdateApplication);
    PyCallable_REG_CALL(CorpRegistryBound, UpdateDivisionNames);
    PyCallable_REG_CALL(CorpRegistryBound, UpdateCorporation);
    PyCallable_REG_CALL(CorpRegistryBound, UpdateLogo);
    PyCallable_REG_CALL(CorpRegistryBound, SetAccountKey);
    PyCallable_REG_CALL(CorpRegistryBound, GetMember);
    PyCallable_REG_CALL(CorpRegistryBound, GetMembers);
    PyCallable_REG_CALL(CorpRegistryBound, UpdateMember);

    PyCallable_REG_CALL(CorpRegistryBound, MoveCompanyShares);
    PyCallable_REG_CALL(CorpRegistryBound, MovePrivateShares);
    PyCallable_REG_CALL(CorpRegistryBound, GetSharesByShareholder);
    PyCallable_REG_CALL(CorpRegistryBound, GetShareholders);
    PyCallable_REG_CALL(CorpRegistryBound, PayoutDividend);

    PyCallable_REG_CALL(CorpRegistryBound, CanViewVotes);
    PyCallable_REG_CALL(CorpRegistryBound, InsertVoteCase);
    PyCallable_REG_CALL(CorpRegistryBound, GetVoteCasesByCorporation);
    PyCallable_REG_CALL(CorpRegistryBound, GetSanctionedActionsByCorporation);

    PyCallable_REG_CALL(CorpRegistryBound, AddBulletin);
    PyCallable_REG_CALL(CorpRegistryBound, GetBulletins);

    PyCallable_REG_CALL(CorpRegistryBound, CreateLabel);
    PyCallable_REG_CALL(CorpRegistryBound, GetLabels);
    PyCallable_REG_CALL(CorpRegistryBound, DeleteLabel);
    PyCallable_REG_CALL(CorpRegistryBound, EditLabel);
    PyCallable_REG_CALL(CorpRegistryBound, AssignLabels);
    PyCallable_REG_CALL(CorpRegistryBound, RemoveLabels);

    PyCallable_REG_CALL(CorpRegistryBound, GetRecentKillsAndLosses);
    PyCallable_REG_CALL(CorpRegistryBound, GetRoleGroups);
    PyCallable_REG_CALL(CorpRegistryBound, GetRoles);
    PyCallable_REG_CALL(CorpRegistryBound, GetLocationalRoles);

    PyCallable_REG_CALL(CorpRegistryBound, GetTitles);
    PyCallable_REG_CALL(CorpRegistryBound, UpdateTitle);
    PyCallable_REG_CALL(CorpRegistryBound, UpdateTitles);
    PyCallable_REG_CALL(CorpRegistryBound, DeleteTitle);
    PyCallable_REG_CALL(CorpRegistryBound, ExecuteActions);

    PyCallable_REG_CALL(CorpRegistryBound, GetCorporateContacts);
    PyCallable_REG_CALL(CorpRegistryBound, AddCorporateContact);
    PyCallable_REG_CALL(CorpRegistryBound, EditContactsRelationshipID);
    PyCallable_REG_CALL(CorpRegistryBound, RemoveCorporateContacts);
    PyCallable_REG_CALL(CorpRegistryBound, EditCorporateContact);

    PyCallable_REG_CALL(CorpRegistryBound, CreateAlliance);
    PyCallable_REG_CALL(CorpRegistryBound, GetSuggestedAllianceShortNames);

    PyCallable_REG_CALL(CorpRegistryBound, GetMembersPaged);
    PyCallable_REG_CALL(CorpRegistryBound, GetMembersByIds);
    PyCallable_REG_CALL(CorpRegistryBound, GetMemberIDsWithMoreThanAvgShares);
    PyCallable_REG_CALL(CorpRegistryBound, GetMemberIDsByQuery);
    PyCallable_REG_CALL(CorpRegistryBound, GetMemberTrackingInfo);
    PyCallable_REG_CALL(CorpRegistryBound, GetMemberTrackingInfoSimple);
    PyCallable_REG_CALL(CorpRegistryBound, GetRentalDetailsPlayer);
    PyCallable_REG_CALL(CorpRegistryBound, GetRentalDetailsCorp);

    PyCallable_REG_CALL(CorpRegistryBound, UpdateCorporationAbilities);
    PyCallable_REG_CALL(CorpRegistryBound, UpdateStationManagementSettings);

    m_corpID = corpID;
}

PyResult CorpRegistryBound::Handle_GetEveOwners(PyCallArgs &call) {
    /* this is a method-chaining call.
     * it comes with the bind request for a particular corp.
     *
     * the client wants a member list for given corp
     */
    return m_db.GetEveOwners(m_corpID);
}

PyResult CorpRegistryBound::Handle_GetInfoWindowDataForChar( PyCallArgs& call )
{    //takes characterID
    //  returns corpID, allianceID, title
    return CharacterDB::GetInfoWindowDataForChar(call.client->GetCharacterID());
}

PyResult CorpRegistryBound::Handle_GetCorporation(PyCallArgs &call) {
    return m_db.GetCorporation(m_corpID);
}

void CorpRegistryBound::JoinCorporation(Client *who, const CorpData& data) {
    who->GetChar()->JoinCorporation(data);
}

PyResult CorpRegistryBound::Handle_GetRoles(PyCallArgs &call)
{   // working
    return m_db.GetCorpRoles();
}

PyResult CorpRegistryBound::Handle_GetRoleGroups(PyCallArgs &call)
{   // working
    return m_db.GetCorpRoleGroups();
}

PyResult CorpRegistryBound::Handle_GetTitles(PyCallArgs &call)
{   // working
    return m_db.GetTitles(m_corpID);
}

PyResult CorpRegistryBound::Handle_GetBulletins(PyCallArgs &call)
{   // working
    return m_db.GetBulletins(m_corpID);
}

PyResult CorpRegistryBound::Handle_GetCorporateContacts(PyCallArgs &call)
{   // working
    return m_db.GetContacts(m_corpID);
}

PyResult CorpRegistryBound::Handle_GetApplications(PyCallArgs &call)
{   // working
    return m_db.GetApplications(m_corpID);
}

PyResult CorpRegistryBound::Handle_GetRecruitmentAdsForCorporation(PyCallArgs &call)
{   // recruitments = self.GetCorpRegistry().GetRecruitmentAdsForCorporation()
    return m_db.GetAdRegistryData();
}

PyResult CorpRegistryBound::Handle_GetMyApplications(PyCallArgs &call)
{   // working
    return m_db.GetMyApplications(call.client->GetCharacterID());
}

PyResult CorpRegistryBound::Handle_GetMemberTrackingInfo(PyCallArgs &call)
{   // working
    return m_db.GetMemberTrackingInfo(m_corpID);
}

PyResult CorpRegistryBound::Handle_GetMemberTrackingInfoSimple(PyCallArgs &call)
{   // working
    return m_db.GetMemberTrackingInfoSimple(m_corpID);
}

PyResult CorpRegistryBound::Handle_GetStations(PyCallArgs &call)
{   // not working
    return m_db.GetStations(m_corpID);
}

PyResult CorpRegistryBound::Handle_GetShareholders(PyCallArgs &call)
{   // working
    return m_db.GetShares(m_corpID);
}

PyResult CorpRegistryBound::Handle_GetSharesByShareholder(PyCallArgs &call)
{   // working
    Call_SingleBoolArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    uint32 owner = (arg.arg ? call.client->GetCorporationID() : call.client->GetCharacterID());
    return m_db.GetMyShares(owner);
}

PyResult CorpRegistryBound::Handle_GetCorporations(PyCallArgs &call) {
    // working
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }
    return m_db.GetCorporations(args.arg);
}

PyResult CorpRegistryBound::Handle_GetRecentKillsAndLosses(PyCallArgs &call)
{   // working
    Call_GetRecentKillsAndLosses args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    return m_db.GetKillsAndLosses(m_corpID, args.number, args.offset);
}

PyResult CorpRegistryBound::Handle_SetAccountKey(PyCallArgs &call)
{   // working
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    call.client->GetChar()->SetAccountKey(args.arg);
    // returns nodeID and timestamp
    PyTuple* tuple = new PyTuple(2);
    tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
    tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    return tuple;
}

PyResult CorpRegistryBound::Handle_GetMember(PyCallArgs &call)
{   // not working
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    return m_db.GetMember(args.arg);
}

PyResult CorpRegistryBound::Handle_GetMembers(PyCallArgs &call)
{   // working
    // this just wants a member count and time
    uint16 rowCount = m_db.GetMemberCount(m_corpID);

    GetMembersSparseRowset ret;
    PyDict *dict = new PyDict();
    dict->SetItemString("realRowCount", new PyInt(rowCount));
    PyTuple* tuple = new PyTuple(3);
    tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
    tuple->SetItem(1, dict);
    tuple->SetItem(2, new PyLong(Win32TimeNow()));
    ret.tuple = tuple;
    ret.realRowCount = rowCount;

    return ret.Encode();
}

PyResult CorpRegistryBound::Handle_UpdateDivisionNames(PyCallArgs &call)
{   // working
    sLog.White( "CorpRegistryBound", "Handle_UpdateDivisionNames() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_UpdateDivisionNames args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return PyStatic.NewNone();
    }

    Notify_IntRaw notif;
        notif.key = m_corpID;
        notif.data = new PyDict();

    if (!m_db.UpdateDivisionNames(notif.key, args, notif.data)) {
        codelog(SERVICE__ERROR, "%s: Failed to update division names for corp %u", call.client->GetName(), notif.key);
        PyDecRef( notif.data );
        return PyStatic.NewNone();
    }

    // Only send notification if it is needed...
    if (notif.data->items.size()) {
        MulticastTarget mct;
            mct.corporations.insert(notif.key);
        PyTuple * answer = notif.Encode();
        sEntityList.Multicast("OnCorporationChanged", "corpid", &answer, mct);
        call.client->SendNotification("OnCorporationChanged", "clientID", &answer);
    }

    // returns nodeID and timestamp
    PyTuple* tuple = new PyTuple(2);
    tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
    tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    return tuple;
}

PyResult CorpRegistryBound::Handle_GetMembersPaged(PyCallArgs &call) {
    //return self.GetCorpRegistry().GetMembersPaged(page)
    /*
     * 11:35:17 W CorpRegistryBound::Handle_GetMembersPaged(): size= 1
     * 11:35:17 [CorpCallDump]   Call Arguments:
     * 11:35:17 [CorpCallDump]       Tuple: 1 elements
     * 11:35:17 [CorpCallDump]         [ 0] Integer field: 1
     */
    sLog.White( "CorpRegistryBound::Handle_GetMembersPaged()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    DBQueryResult res;
    m_db.GetMembersPaged(m_corpID, arg.arg, res);
    //uint32 rowCount = (uint32)res.GetRowCount();

    PyList* list = new PyList();
    DBResultRow row;
    while (res.GetRow(row)) {
        //SELECT characterID, corporationID, title, rolesAtAll, grantableRoles, startDateTime, rolesAtHQ, grantableRolesAtHQ, \
        rolesAtBase, grantableRolesAtBase, rolesAtOther, grantableRolesAtOther, titleMask, corpAccountKey, blockRoles, name
        PyDict* dict = new PyDict();
        dict->SetItemString( "characterID",             new PyInt(row.GetInt(0)));
        dict->SetItemString( "corporationID",           new PyInt(row.GetInt(1)));
        dict->SetItemString( "divisionID",              new PyInt(0));
        dict->SetItemString( "squadronID",              new PyInt(0));
        dict->SetItemString( "title",                   new PyInt(row.GetInt(2)));
        dict->SetItemString( "roles",                   new PyLong(row.GetInt64(3)));
        dict->SetItemString( "grantableRoles",          new PyInt(row.GetInt(4)));
        dict->SetItemString( "startDateTime",           new PyLong(row.GetInt64(5)));
        dict->SetItemString( "baseID",                  new PyInt(0)); /** @todo update this */
        dict->SetItemString( "rolesAtHQ",               new PyLong(row.GetInt64(6)));
        dict->SetItemString( "grantableRolesAtHQ",      new PyLong(row.GetInt64(7)));
        dict->SetItemString( "rolesAtBase",             new PyLong(row.GetInt64(8)));
        dict->SetItemString( "grantableRolesAtBase",    new PyLong(row.GetInt64(9)));
        dict->SetItemString( "rolesAtOther",            new PyLong(row.GetInt64(10)));
        dict->SetItemString( "grantableRolesAtOther",   new PyLong(row.GetInt64(11)));
        dict->SetItemString( "titleMask",               new PyLong(row.GetInt64(12))); // titleID
        dict->SetItemString( "accountKey",              new PyInt(row.GetInt(13)));
        dict->SetItemString( "rowDate",                 new PyLong(GetFileTimeNow())); //may not be right
        dict->SetItemString( "blockRoles",              new PyBool(row.GetInt(14)));
        dict->SetItemString( "ownerName",               new PyString(row.GetText(15)));
        list->AddItem(new PyObject("util.KeyVal", dict));
    }

    return list;
}

PyResult CorpRegistryBound::Handle_GetMembersByIds(PyCallArgs &call) {
    //return self.GetCorpRegistry().GetMembersByIds(memberIDs)
    sLog.White( "CorpRegistryBound::Handle_GetMembersByIds()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_GetMembersByID args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    PyList* list = new PyList();
    for (PyList::const_iterator itr = args.memberIDs->begin(); itr != args.memberIDs->end(); ++itr)
        list->AddItem(new PyObject("util.KeyVal", m_db.GetMember((*itr)->AsInt()->value())));

    return list;
}

PyResult CorpRegistryBound::Handle_AddCorporation(PyCallArgs &call) {
    Client* pClient(call.client);
    Call_AddCorporation args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", pClient->GetName());
        return nullptr;
    }
    //CorpTickerNameInvalidTaken

    double corp_cost = sConfig.rates.corpCost;
    if (pClient->GetBalance(Account::CreditType::ISK) < corp_cost) {
        _log(SERVICE__ERROR, "%s: Cannot afford corporation startup costs!", pClient->GetName());
        return nullptr;
    }

    //adding a corporation might affect eveStaticOwners, so we gotta invalidate the cache...
    PyString* cache_name = new PyString( "config.StaticOwners" );
    m_manager->cache_service->InvalidateCache( cache_name );
    PySafeDecRef( cache_name );

    // Register new corp
    uint32 corpID = 0;
    if (!m_db.AddCorporation(args, pClient, corpID)) {
        codelog(SERVICE__ERROR, "New corporation creation failed.");
        return nullptr;
    }
    // create default role title data
    m_db.CreateTitleData(corpID);

    //take the money, send wallet blink event record the transaction in their journal.
    std::string reason = "DESC: Creating new corporation: ";
    reason += args.corpName;
    reason += " (";
    reason += args.corpTicker;
    reason += ")";
    AccountService::TranserFunds(
                                pClient->GetCharacterID(),
                                m_db.GetStationOwner(pClient->GetStationID()),  // station owner files paperwork, this is fee for that
                                corp_cost,
                                reason.c_str(),
                                Journal::EntryType::CorporationRegistrationFee,
                                pClient->GetStationID(),
                                Account::KeyType::Cash);

    // add corp event for creating new corp
    m_db.AddItemEvent(corpID, pClient->GetCharacterID(), Corp::EventType::CreatedCorporation);

    // create corp channel
    m_manager->lsc_service->CreateSystemChannel(corpID);

    CorpData data;
        data.name = args.corpName;
        data.ticker = args.corpTicker;
        data.taxRate = args.taxRate;
        data.allianceID = 0;
        data.warFactionID = 0;
        data.baseID = pClient->GetLocationID();
        data.corpHQ = pClient->GetLocationID();
        data.corporationID = corpID;
        data.corpAccountKey = Account::KeyType::Cash;
        data.corpRole = Corp::Role::Admin;
        data.rolesAtAll = Corp::Role::Admin;
        data.rolesAtBase = Corp::Role::Admin;
        data.rolesAtHQ = Corp::Role::Admin;
        data.rolesAtOther = Corp::Role::Admin;
        data.grantableRoles = Corp::Role::Admin;
        data.grantableRolesAtBase = Corp::Role::Admin;
        data.grantableRolesAtHQ = Corp::Role::Admin;
        data.grantableRolesAtOther = Corp::Role::Admin;
    // update corp data and refresh session data.
    pClient->GetChar()->JoinCorporation(data);

    // Here we send a notification about creating a new corporation...
    OnCorporationChanged cc;
    cc.corpID = corpID;
    if (!m_db.CreateCorporationCreatePacket(cc, m_corpID, corpID)) {
        codelog(SERVICE__ERROR, "Failed to create OnCorpChanged notification stream.");
        // This is a big problem, because this way we won't be able to see the difference...
        pClient->SendErrorMsg("Unable to notify about corp creation.");
        return nullptr;
    }
    PyTuple* a1 = cc.Encode();
    PyIncRef(a1);
    // send single to client
    pClient->SendNotification("OnCorporationChanged", "clientID", &a1);
    // send multi to station guests
    PyIncRef(a1);
    sEntityList.Multicast("OnCorporationChanged", "stationid", &a1, NOTIF_DEST__LOCATION, pClient->GetLocationID());

    return m_db.GetCorporations(corpID);
}

PyResult CorpRegistryBound::Handle_DeleteTitle(PyCallArgs &call) {
    /*
     *    def DeleteTitle(self, titleID):
     *    self.GetCorpRegistry().DeleteTitle(titleID)
     */
    sLog.White( "CorpRegistryBound::Handle_DeleteTitle()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    m_db.DeleteTitle(call.client->GetCorporationID(), arg.arg);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_UpdateTitle(PyCallArgs &call) {
    // self.GetCorpRegistry().UpdateTitle(titleID, titleName, roles, grantableRoles, rolesAtHQ, grantableRolesAtHQ, rolesAtBase, grantableRolesAtBase, rolesAtOther, grantableRolesAtOther)
    sLog.White( "CorpRegistryBound::Handle_UpdateTitle()", "size= %u", call.tuple->size() );
    //call.Dump(CORP__CALL_DUMP);

    Call_UpdateTitleData args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    PyDict* updates = new PyDict();
    if (m_db.UpdateTitle(m_corpID, args, updates)) {
        OnTitleChanged change;
            change.corpID = m_corpID;
            change.titleID = args.titleID;
            change.changes = updates;
        PyTuple* notif = change.Encode();
        MulticastTarget mct;
            mct.corporations.insert(m_corpID);
        sEntityList.Multicast("OnTitleChanged", "corpid", &notif, mct);
    }

    //OnTitleChanged
    return nullptr;
}

PyResult CorpRegistryBound::Handle_UpdateTitles(PyCallArgs &call) {
    //    self.GetCorpRegistry().UpdateTitles(titles)
    sLog.White( "CorpRegistryBound::Handle_UpdateTitles()", "size= %u", call.tuple->size() );
    //call.Dump(CORP__CALL_DUMP);
    if (!call.tuple->GetItem(0)->IsObject()) {
        codelog(CORP__ERROR, "Tuple Item is wrong type: %s.  Expected PyObject.", call.tuple->GetItem(0)->TypeString());
        return nullptr;
    }
    Call_UpdateTitle arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    if (!arg.object->arguments()->IsDict()) {
        codelog(CORP__ERROR, "Object Argument is wrong type: %s.  Expected PyDict.", arg.object->arguments()->TypeString());
        return nullptr;
    }

    PyDict* dict = arg.object->arguments()->AsDict();
    //dict->Dump(CORP__TRACE, "    ");
    PyRep* rep = dict->GetItemString("lines");
    if (!rep->IsList()) {
        codelog(CORP__ERROR, "'lines' item is not PyList: %s", rep->TypeString());
        return nullptr;
    }

    Call_UpdateTitleData args;
    PyList* list = rep->AsList();
    for (PyList::const_iterator itr = list->begin(); itr != list->end(); ++itr) {
        if (!(*itr)->IsList()) {
            codelog(CORP__ERROR, "itr item is not PyList: %s", (*itr)->TypeString());
            continue;
        }
        PyDict* updates = new PyDict();
        PyList* list2 = (*itr)->AsList();
        args.titleID = list2->GetItem(0)->AsInt()->value();
        args.titleName = PyRep::StringContent(list2->GetItem(1));
        args.roles = PyRep::IntegerValue(list2->GetItem(2));
        args.grantableRoles = PyRep::IntegerValue(list2->GetItem(3));
        args.rolesAtHQ = PyRep::IntegerValue(list2->GetItem(4));
        args.grantableRolesAtHQ = PyRep::IntegerValue(list2->GetItem(5));
        args.rolesAtBase = PyRep::IntegerValue(list2->GetItem(6));
        args.grantableRolesAtBase = PyRep::IntegerValue(list2->GetItem(7));
        args.rolesAtOther = PyRep::IntegerValue(list2->GetItem(8));
        args.grantableRolesAtOther = PyRep::IntegerValue(list2->GetItem(9));
        if (m_db.UpdateTitle(m_corpID, args, updates)) {
            OnTitleChanged change;
                change.corpID = m_corpID;
                change.titleID = args.titleID;
                change.changes = updates;
            PyTuple* notif = change.Encode();
            MulticastTarget mct;
                mct.corporations.insert(m_corpID);
            sEntityList.Multicast("OnTitleChanged", "corpid", &notif, mct);
        }
    }

    return nullptr;
}

PyResult CorpRegistryBound::Handle_GetSuggestedTickerNames(PyCallArgs &call)
{
    Call_SingleWStringArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    PyList* result = new PyList();
    SuggestedTickerName sTN;
    sTN.tName.clear();
    uint32 cnLen = args.arg.length();
    // Easiest ticker-generation method: get the capital letters.
    for (uint32 i=0; i < cnLen; ++i)
        if (args.arg[i] >= 'A' && args.arg[i] <= 'Z')
            sTN.tName += args.arg[i];

        result->AddItem( sTN.Encode() );
    return result;
}

PyResult CorpRegistryBound::Handle_GetSuggestedAllianceShortNames(PyCallArgs &call)
{
    sLog.White("CorpRegistryBound", "Handle_GetSuggestedAllianceShortNames() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_SingleWStringArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }
    PyList* result = new PyList();
    SuggestedShortName sSN;
    sSN.sName.clear();
    uint32 cnLen = args.arg.length();
    // Easiest ticker-generation method: get the capital letters.
    for (uint32 i=0; i < cnLen; ++i)
        if (args.arg[i] >= 'A' && args.arg[i] <= 'Z')
            sSN.sName += args.arg[i];

        result->AddItem( sSN.Encode() );
    return result;
}

PyResult CorpRegistryBound::Handle_UpdateCorporation(PyCallArgs &call) {
    sLog.White( "CorpRegistryBound", "Handle_UpdateCorporation() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_UpdateCorporation args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    Notify_IntRaw notif;
    notif.key = m_corpID;
    notif.data = new PyDict();

    if (!m_db.UpdateCorporation(notif.key, args, notif.data)) {
        codelog(SERVICE__ERROR, "%s: Failed to update corporation data for corp %u", call.client->GetName(), notif.key);
        PyDecRef( notif.data );
        return PyStatic.NewNone();
    }

    // Only send notification if it is needed...
    if (notif.data->items.size()) {
        MulticastTarget mct;
        mct.corporations.insert(notif.key);
        PyTuple * answer = notif.Encode();
        sEntityList.Multicast("OnCorporationChanged", "corpid", &answer, mct);
        call.client->SendNotification("OnCorporationChanged", "clientID", &answer);
    }

    return PyStatic.NewNone();
}

PyResult CorpRegistryBound::Handle_UpdateLogo(PyCallArgs &call)
{
    Call_UpdateLogo args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    Notify_IntRaw notif;
    notif.key = m_corpID;
    notif.data = new PyDict();

    // Check if we have enough money
    double logo_change = 100;
    if (AccountDB::GetCorpBalance(notif.key, Account::KeyType::Cash) < logo_change) {
        _log( SERVICE__ERROR, "%s: Cannot afford corporation logo change costs!", call.client->GetName());
        call.client->SendErrorMsg("Your corporation doesn't have enough money (%u ISK) to change it's logo!", logo_change);
        PyDecRef( notif.data );
        return nullptr;
    }

    if (!m_db.UpdateLogo(notif.key, args, notif.data))    {
        codelog( SERVICE__ERROR, "Corporation logo change failed..." );
        PyDecRef( notif.data );
        return nullptr;
    }

    std::string reason = "Change Corporation Logo.";
    // move cash and record the transaction.
    AccountService::TranserFunds(
        call.client->GetCharacterID(),
                                 m_db.GetStationOwner(call.client->GetStationID()),
                                 logo_change,
                                 reason,
                                 Journal::EntryType::CorporationLogoChangeCost,
                                 Account::KeyType::Cash);   // main wallet

    // Send notification to those in the station
    MulticastTarget mct;
    mct.locations.insert(call.client->GetLocationID());
    PyTuple *answer = notif.Encode();
    sEntityList.Multicast("OnCorporationChanged", "corpid", &answer, mct);

    return m_db.GetCorporation(notif.key);
}

PyResult CorpRegistryBound::Handle_AddBulletin(PyCallArgs &call) {
    // self.GetCorpRegistry().AddBulletin(title, body)
    // self.GetCorpRegistry().AddBulletin(title, body, bulletinID=bulletinID, editDateTime=editDateTime)    <-- this is to update bulletin
    sLog.White( "CorpRegistryBound::Handle_AddBulletin()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_AddBulletin args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    bool edit = false;
    int64 editDateTime = 0;
    if (call.byname.find("editDateTime") != call.byname.end()) {
        if (call.byname.find("editDateTime")->second->IsLong()) {
            edit = true;
            editDateTime = call.byname.find("editDateTime")->second->AsLong()->value();
        } else
            _log(CORP__ERROR, "Handle_AddBulletin - editDateTime is of the wrong type: '%s'.  Expected PyString or PyWString.", \
                            call.byname.find("editDateTime")->second->TypeString());
    }

    int64 bulletinID = 0;
    if (call.byname.find("bulletinID") != call.byname.end()) {
        if (call.byname.find("bulletinID")->second->IsInt()) {
            edit = true;
            bulletinID = call.byname.find("bulletinID")->second->AsInt()->value();
        } else
            _log(CORP__ERROR, "Handle_AddBulletin - bulletinID is of the wrong type: '%s'.  Expected PyInt.", \
                            call.byname.find("bulletinID")->second->TypeString());
    }

    if (edit)
        m_db.EditBulletin(bulletinID, call.client->GetCharacterID(), editDateTime, args.title, args.body);
    else
        m_db.AddBulletin(m_corpID, m_corpID, call.client->GetCharacterID(), args.title, args.body);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_GetRecruiters(PyCallArgs &call) {
    sLog.White( "CorpRegistryBound::Handle_GetRecruiters()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    return m_db.GetRecruiters(args.arg2/*, args.arg1*/);
}

PyResult CorpRegistryBound::Handle_CreateRecruitmentAd(PyCallArgs &call) {
    // return self.GetCorpRegistry().CreateRecruitmentAd(days, typeMask, allianceID, description, channelID, recruiters, title)
    sLog.White( "CorpRegistryBound::Handle_CreateRecruitmentAd()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);
    /*
     * 00:41:50 [SvcCall] Service CorpRegistryBound::CreateRecruitmentAd()
     * 00:41:50 W CorpRegistryBound::Handle_CreateRecruitmentAd(): size= 7
     * 00:41:50 [CorpCallDump]   Call Arguments:
     * 00:41:50 [CorpCallDump]      Tuple: 7 elements
     * 00:41:50 [CorpCallDump]       [ 0]    Integer: 14
     * 00:41:50 [CorpCallDump]       [ 1]    Integer: 34672663
     * 00:41:50 [CorpCallDump]       [ 2]       None
     * 00:41:50 [CorpCallDump]       [ 3]    WString: 'test'
     * 00:41:50 [CorpCallDump]       [ 4]    Integer: 0
     * 00:41:50 [CorpCallDump]       [ 5]   List: 1 elements
     * 00:41:50 [CorpCallDump]       [ 5]   [ 0]    Integer: 90000000
     * 00:41:50 [CorpCallDump]       [ 6]    WString: 'test'
     */

    Call_CreateRecruitmentAd args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    int32 adID = m_db.CreateAdvert(call.client, m_corpID, args.typeMask, args.days, m_db.GetCorpMemberCount(m_corpID), args.description, args.channelID, args.title);

    std::vector<int32> recruiters;
    for (PyList::const_iterator itr = args.recruiters->begin(); itr != args.recruiters->end(); ++itr)
        recruiters.push_back((*itr)->AsInt()->value());

    m_db.AddRecruiters(adID, (int32)m_corpID, recruiters);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_GetLabels(PyCallArgs &call) {
    sLog.White("CorpRegistryBound", "Handle_GetLabels() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return m_db.GetLabels(call.client->GetCorporationID());
}

PyResult CorpRegistryBound::Handle_MoveCompanyShares(PyCallArgs &call) {
    // return self.GetCorpRegistry().MoveCompanyShares(corporationID, toShareholderID, numberOfShares)
    sLog.White( "CorpRegistryBound::Handle_MoveCompanyShares()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_MoveShares args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    uint32 corpID = 0;
    Client* pClient = sEntityList.FindClientByCharID(args.toShareholderID);
    // not sure if it's possible to xfer shares to offline chars, but test anyway
    if (pClient == nullptr) {
        corpID = CharacterDB::GetCorpID(args.toShareholderID);
    } else
        corpID = pClient->GetCorporationID();

    m_db.MoveShares(m_corpID, corpID, args);
    return nullptr;
}

PyResult CorpRegistryBound::Handle_MovePrivateShares(PyCallArgs &call) {
    // return self.GetCorpRegistry().MovePrivateShares(corporationID, toShareholderID, numberOfShares)
    sLog.White( "CorpRegistryBound::Handle_MovePrivateShares()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_MoveShares args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    uint32 corpID = 0;
    Client* pClient = sEntityList.FindClientByCharID(args.toShareholderID);
    // not sure if it's possible to xfer shares to offline chars, but test anyway
    if (pClient == nullptr) {
        corpID = CharacterDB::GetCorpID(args.toShareholderID);
    } else
        corpID = pClient->GetCorporationID();

    // gonna have to do this one different...
    //  will need shares OF WHAT corpID also.
    m_db.MoveShares(call.client->GetCharacterID(), corpID, args);
    return nullptr;
}


/**     ***********************************************************************
 * @note   these below are not coded or partially coded
 */

PyResult CorpRegistryBound::Handle_PayoutDividend(PyCallArgs &call) {
    //self.GetCorpRegistry().PayoutDividend(payShareholders, payoutAmount)
    /*** shareholders
     * 04:42:43 W CorpRegistryBound::Handle_PayoutDividend(): size= 2
     * 04:42:43 [CorpCallDump]   Call Arguments:
     * 04:42:43 [CorpCallDump]       Tuple: 2 elements
     * 04:42:43 [CorpCallDump]         [ 0] Integer field: 1            <-- int bool?
     * 04:42:43 [CorpCallDump]         [ 1] Real field: 1.000000        <-- amount
     *** members
     * 04:42:50 W CorpRegistryBound::Handle_PayoutDividend(): size= 2
     * 04:42:50 [CorpCallDump]   Call Arguments:
     * 04:42:50 [CorpCallDump]       Tuple: 2 elements
     * 04:42:50 [CorpCallDump]         [ 0] Integer field: 0
     * 04:42:50 [CorpCallDump]         [ 1] Real field: 1.000000
     */
    sLog.White( "CorpRegistryBound::Handle_PayoutDividend()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_PayoutDividend args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    // get list of ids to pay.  this includes corp shareholders if paying to shares
    std::vector<uint32> toIDs;
    if (args.payShareholders) {

    } else {

    }

    // get total amount and divide by # of ids to pay
    float amount = args.payoutAmount / toIDs.size();
    if (amount < 0.01)
        return nullptr;  //make error here?

        // pay each id and record xfer
        std::string reason = "Dividend Payment from ";
    reason += ""; //corp name here
    for (auto cur : toIDs)
        AccountService::TranserFunds(m_corpID, cur, amount, reason.c_str(), Journal::EntryType::CorporationDividendPayment, call.client->GetCharacterID());

    return nullptr;
}

PyResult CorpRegistryBound::Handle_GetMemberIDsWithMoreThanAvgShares(PyCallArgs &call) {
    // return self.GetCorpRegistry().GetMemberIDsWithMoreThanAvgShares()
    sLog.White( "CorpRegistryBound::Handle_GetMemberIDsWithMoreThanAvgShares()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}


PyResult CorpRegistryBound::Handle_GetMemberIDsByQuery(PyCallArgs &call) {
    //return self.GetCorpRegistry().GetMemberIDsByQuery(query, includeImplied, searchTitles)
    Call_GetMemberIDsByQuery_Main args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    if (args.data->empty()) {
        call.client->SendErrorMsg("You must choose a role to search for.");
        return nullptr;
    }

    /*
     *                query.append([property, operator, value])
     *                query.append([joinOperator, property, operator, value])
     *
     * 09:01:13 [CorpCallDump]   Call Arguments:
     * 09:01:13 [CorpCallDump]       Tuple: 3 elements
     * 09:01:13 [CorpCallDump]         [ 0] List: 3 elements
     * 09:01:13 [CorpCallDump]         [ 0]   [ 0] List: 3 elements
     * 09:01:13 [CorpCallDump]         [ 0]   [ 0]   [ 0] String: 'roles'
     * 09:01:13 [CorpCallDump]         [ 0]   [ 0]   [ 1] Integer field: 7
     * 09:01:13 [CorpCallDump]         [ 0]   [ 0]   [ 2] Integer field: 134217728
     * 09:01:13 [CorpCallDump]         [ 0]   [ 1] List: 4 elements
     * 09:01:13 [CorpCallDump]         [ 0]   [ 1]   [ 0] Integer field: 2             <-- joinOp  (AND/OR)
     * 09:01:13 [CorpCallDump]         [ 0]   [ 1]   [ 1] String: 'roles'              <-- queryType   (Corp::QueryType {roles/charID/baseID/joinDate})
     * 09:01:13 [CorpCallDump]         [ 0]   [ 1]   [ 2] Integer field: 7             <-- searchOp (Corp::SearchOp)
     * 09:01:13 [CorpCallDump]         [ 0]   [ 1]   [ 3] Integer field: 2199023255552 >-- value   (depends on searchOp, int, int64)
     * 09:01:13 [CorpCallDump]         [ 0]   [ 2] List: 4 elements
     * 09:01:13 [CorpCallDump]         [ 0]   [ 2]   [ 0] Integer field: 2
     * 09:01:13 [CorpCallDump]         [ 0]   [ 2]   [ 1] String: 'rolesAtOther'
     * 09:01:13 [CorpCallDump]         [ 0]   [ 2]   [ 2] Integer field: 8
     * 09:01:13 [CorpCallDump]         [ 0]   [ 2]   [ 3] Integer field: 8192
     * 09:01:13 [CorpCallDump]         [ 1] Integer field: 0
     * 09:01:13 [CorpCallDump]         [ 2] Integer field: 0
     */

    uint8 queryType = Corp::QueryType::Roles;
    uint8 joinOp = Corp::JoinOp::OR;
    uint8 searchOp = Corp::SearchOp::EQUAL;
    int64 searchRole = 0;
    std::string searchString = "";

    // get corp memberlist
    std::list<Corp::QueryMembers> resList;
    m_db.GetMembersForQuery(m_corpID, resList);

    /* SELECT characterID, startDateTime, titleMask, blockRoles,
     *       rolesAtAll, rolesAtHQ, rolesAtBase, rolesAtOther,
     *       grantableRoles, grantableRolesAtHQ, grantableRolesAtBase, grantableRolesAtOther
     */

    // get query format, perform query, and store results
    PyRep* searchRaw(nullptr);
    PyList* list(nullptr);
    for (PyList::const_iterator itr = args.data->begin(); itr != args.data->end(); ++itr) {
        list = (*itr)->AsList();
        if (list == nullptr)
            continue;

        // aquire query format and options and perform query
        /*  complicated search query here....this will get quite complicated....
         *      my thoughts.
         * this is performed thru corp window using a multitude of options, to get specific members based on quite variable criteria
         *   for first loop, we will perform initial query (list3) on the full member list, resList, and those that do not fit are removed
         *   each subsquent loop will query resList for additional parameters
         * std::list is used as it does not invalidate iterators when members are erased
         */
        if (list->size() == 3) {
            Call_GetMemberIDsByQuery_List3 args3;
            if (!args3.Decode(&list)) {
                codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
                return nullptr;
            }
            searchOp = args3.searchOp;
            if (searchOp > 8)
                searchString = PyRep::StringContent(args3.valueRaw);
            else
                searchRole = PyRep::IntegerValue(args3.valueRaw);
            queryType = GetQueryType(args3.queryType);

            resList;

        } else if (list->size() == 4) {
            Call_GetMemberIDsByQuery_List4 args4;
            if (!args4.Decode(&list)) {
                codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
                return nullptr;
            }
            joinOp = args4.joinOp;
            searchOp = args4.searchOp;
            if (searchOp > 8)
                searchString = PyRep::StringContent(args4.valueRaw);
            else
                searchRole = PyRep::IntegerValue(args4.valueRaw);
            queryType = GetQueryType(args4.queryType);

            resList;

        } else {
            _log(CORP__ERROR, "CorpRegistryBound::Handle_GetMemberIDsByQuery() - Invalid data size: %u.  Expected 3 or 4.", list->size());
            continue;
        }


    }

    // populate results
    if (list == nullptr)
        list = new PyList();
    else
        list->clear();
    for (auto cur : resList)
        list->AddItem(new PyInt(cur.characterID));

    // return results
    return list;
}

uint8 CorpRegistryBound::GetQueryType(std::string queryType)
{
    if (queryType.compare("roles") == 0)
        return Corp::QueryType::Roles;
    else if (queryType.compare("rolesAtHQ") == 0)
        return Corp::QueryType::Roles;
    else if (queryType.compare("rolesAtBase") == 0)
        return Corp::QueryType::Roles;
    else if (queryType.compare("rolesAtOther") == 0)
        return Corp::QueryType::Roles;
    else if (queryType.compare("grantableRoles") == 0)
        return Corp::QueryType::Roles;
    else if (queryType.compare("grantableRolesAtHQ") == 0)
        return Corp::QueryType::Roles;
    else if (queryType.compare("grantableRolesAtBase") == 0)
        return Corp::QueryType::Roles;
    else if (queryType.compare("grantableRolesAtOther") == 0)
        return Corp::QueryType::Roles;
    else if (queryType.compare("baseID") == 0)
        return Corp::QueryType::BaseID;
    else if (queryType.compare("startDateTime") == 0)
        return Corp::QueryType::StartDateTime;
    else if (queryType.compare("characterID") == 0)     //this is actually string.  check searchOp for details
        return Corp::QueryType::CharID;
    else if (queryType.compare("titleMask") == 0)
        return Corp::QueryType::TitleMask;
    else
        _log(CORP__ERROR, "CorpRegistryBound::GetQueryType() - Invalid QueryType: %s", queryType.c_str());
}

PyResult CorpRegistryBound::Handle_UpdateMember(PyCallArgs &call) {
    //return self.GetCorpRegistry().UpdateMember(charIDToUpdate, title, divisionID, squadronID, roles, grantableRoles, rolesAtHQ, grantableRolesAtHQ, rolesAtBase, grantableRolesAtBase, rolesAtOther, grantableRolesAtOther, baseID, titleMask, blockRoles)
    /** @todo there is more to this call......havent fully figured it out yet.  */
    sLog.White( "CorpRegistryBound::Handle_UpdateMember()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_UpdateMember args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }
    if (!IsCharacter(args.charID))
        return nullptr;

    int64 oldRole = 0;
    bool grantable = false;  // boolean - do new roles have grantable privs?  they may.

    Client* pClient = sEntityList.FindClientByCharID(args.charID);
    if (pClient == nullptr) {
        oldRole = CharacterDB::GetCorpRole(args.charID);
        CharacterDB::SetCorpRole(args.charID, args.roles);
    } else {
        oldRole = pClient->GetCorpRole();
        CorpData data = pClient->GetChar()->GetCorpData();
        data.rolesAtAll = args.roles;
        data.rolesAtBase = args.rolesAtBase;
        data.rolesAtHQ = args.rolesAtHQ;
        data.rolesAtOther = args.rolesAtOther;
        data.grantableRolesAtBase = args.grantableRolesAtBase;
        data.grantableRolesAtHQ = args.grantableRolesAtHQ;
        data.grantableRolesAtOther = args.grantableRolesAtOther;
        data.grantableRoles = args.grantableRoles;
        // update corp data and refresh session data.
        pClient->GetChar()->UpdateCorpData(data);
    }

    // check roleGroups data for bitmaks for grantable roles
    //      use bitmaks to set 'grantable' bool

    m_db.AddRoleHistory(m_corpID, args.charID, call.client->GetCharacterID(), oldRole, args.roles, grantable);
    return nullptr;
}

PyResult CorpRegistryBound::Handle_GetLocationalRoles(PyCallArgs &call) {
    // this gets grantable roles for title (i think)
    sLog.White( "CorpRegistryBound::Handle_GetLocationalRoles()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);
    return nullptr;
}

PyResult CorpRegistryBound::Handle_AddCorporateContact(PyCallArgs &call) {
    //self.GetCorpRegistry().AddCorporateContact(contactID, relationshipID)
    sLog.White( "CorpRegistryBound::Handle_AddCorporateContact()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_CorporateContactData args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    return nullptr;
}

PyResult CorpRegistryBound::Handle_EditCorporateContact(PyCallArgs &call) {
    //self.GetCorpRegistry().EditCorporateContact(contactID, relationshipID)
    sLog.White( "CorpRegistryBound::Handle_EditCorporateContact()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_CorporateContactData args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    return nullptr;
}

PyResult CorpRegistryBound::Handle_EditContactsRelationshipID(PyCallArgs &call) {
    //self.GetCorpRegistry().EditContactsRelationshipID(contactIDs, relationshipID)
    sLog.White( "CorpRegistryBound::Handle_EditContactsRelationshipID()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_EditCorporateContacts args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    return nullptr;
}

PyResult CorpRegistryBound::Handle_RemoveCorporateContacts(PyCallArgs &call) {
   // self.GetCorpRegistry().RemoveCorporateContacts(contactIDs)
    sLog.White( "CorpRegistryBound::Handle_RemoveCorporateContacts()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_RemoveCorporateContacts args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    return nullptr;
}

// this is a member role/title update by memberIDs called from corp->members->find in role->task mgmt
PyResult CorpRegistryBound::Handle_ExecuteActions(PyCallArgs &call) {
    //    sm.GetService('corp').ExecuteActions(self.targetIDs, actions)
    sLog.White("CorpRegistryBound", "Handle_ExecuteActions() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_ExecuteActions args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }
    args.Dump(CORP__TRACE);

    /*
     *   args.memberIDs = list of charIDs to update
     *   args.verb      = one of add, remove, set, give
     *   args.queryType = all role* types or title
     *   args.value     = roleMask or titleID
     */

    return nullptr;
}

PyResult CorpRegistryBound::Handle_CreateLabel(PyCallArgs &call) {
    // return self.GetCorpRegistry().CreateLabel(name, color)
    sLog.White("CorpRegistryBound", "Handle_CreateLabel() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_DeleteLabel(PyCallArgs &call) {
    // self.GetCorpRegistry().DeleteLabel(labelID)
    sLog.White("CorpRegistryBound", "Handle_DeleteLabel() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_EditLabel(PyCallArgs &call) {
    // self.GetCorpRegistry().EditLabel(labelID, name, color)
    sLog.White("CorpRegistryBound", "Handle_EditLabel() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_AssignLabels(PyCallArgs &call) {
    // self.GetCorpRegistry().AssignLabels(contactIDs, labelMask)
    sLog.White("CorpRegistryBound", "Handle_AssignLabels() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_RemoveLabels(PyCallArgs &call) {
    // self.GetCorpRegistry().RemoveLabels(contactIDs, labelMask)
    sLog.White("CorpRegistryBound", "Handle_RemoveLabels() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_CreateAlliance(PyCallArgs &call) {
    //self.GetCorpRegistry().CreateAlliance(allianceName, shortName, description, url)
    sLog.White("CorpRegistryBound", "Handle_CreateAlliance() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_GetLockedItemLocations( PyCallArgs& call )
{    /*
    03:15:28 W CorpRegistryBound::Handle_GetLockedItemLocations(): size= 0
    03:15:28 [CorpCallDump]   Call Arguments:
    03:15:28 [CorpCallDump]       Tuple: Empty
    */
    // called from corp.assets.lockdown
    sLog.White( "CorpRegistryBound::Handle_GetLockedItemLocations()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    //this returns an empty list for me on live.
    return new PyList();
}

PyResult CorpRegistryBound::Handle_GetOffices(PyCallArgs &call) {
    sLog.White( "CorpRegistryBound", "Handle_GetOffices() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    /** @todo  wtf is this shit???  fix it!  */
    PyBoundObject* bObj = new SparseCorpOfficeListBound(m_manager, m_db);
    if (bObj == NULL) {
        _log(SERVICE__ERROR, "%s: Unable to create bound object for:", call.client->GetName()); //errors here
        return nullptr;
    }

    /** @todo this is wrong....to fix later
     *
      [PySubStream 113 bytes]
        [PyObjectData Name: util.SparseRowset]
          [PyTuple 3 items]
            [PyList 4 items]
              [PyString "stationID"]
              [PyString "typeID"]
              [PyString "officeID"]
              [PyString "officeFolderID"]
            [PySubStruct]
              [PySubStream 49 bytes]
                [PyTuple 3 items]
                  [PyString "N=795216:873"]
                  [PyDict 1 kvp]
                    [PyString "realRowCount"]
                    [PyInt 7]
                  [PyIntegerVar 129773015637103342]
            [PyInt 7]
    [PyDict 1 kvp]
      [PyString "OID+"]
      [PyDict 1 kvp]
        [PyString "N=795216:873"]
        [PyIntegerVar 129773015637103342]
     */

    // this sends header info and # offices in station
    // Data will be fetched from the SparseRowset
    CorpOfficeSparseRowset ret;

    ret.officeNumber = StationDB::GetOfficeCount(m_corpID);

    PyDict *dict = new PyDict();
    dict->SetItemString("realRowCount", new PyInt(ret.officeNumber));

    ret.bindedObject = m_manager->BindObject(call.client, bObj, &dict);

    PyObject * res = ret.Encode();
    res->Dump(CORP__RSP_DUMP, "    ");
    return res;
}

PyResult CorpRegistryBound::Handle_InsertApplication(PyCallArgs &call) {
    /** Incoming:
     *  Integer: 777777777 <- corp id
     *  String: "Ignore me" <- text that was entered into the box
     */

    sLog.White( "CorpRegistryBound", "Handle_InsertApplication() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_InsertApplication res;
    if (!res.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    /// Insert query into the db
    ApplicationInfo aInfo;
        aInfo.valid = true;
        aInfo.charID = call.client->GetCharacterID();
        aInfo.corpID = res.corpID;
        aInfo.appText = res.message;
        aInfo.role = Corp::Role::Member;
        aInfo.grantRole = Corp::Role::Member;  // this is "None"
        aInfo.status = Corp::AppStatus::AppliedByCharacter;
        aInfo.appTime = GetFileTimeNow();
        aInfo.deleted = false;
        aInfo.lastCID = 0;

    if (!m_db.InsertApplication(aInfo)) {
        codelog(SERVICE__ERROR, "%s: Failed to insert application request", call.client->GetName());
        return nullptr;
    }

    /// BroadcastStuff::Notify( OnCorporationApplicationChanged ,...)
    // this is for an existing application, to change data.
    // since this is a new application, there is no data to change
    ApplicationInfo oldInfo;
        oldInfo.valid = false;
        /*
        oldInfo.charID = call.client->GetCharacterID();
        oldInfo.corpID = res.corpID;
        oldInfo.appText = res.message;
        oldInfo.role = Corp::Role::Member;
        oldInfo.grantRole = Corp::Role::Member;  // this is "None"
        oldInfo.status = Corp::AppStatus::AppliedByCharacter;
        oldInfo.appTime = GetFileTimeNow();
        oldInfo.deleted = false;
        oldInfo.lastCID = 0;
        */
    OnCorporationApplicationChanged OCAC;
    FillOCApplicationChange(OCAC, oldInfo, aInfo);
    OCAC.corpID = res.corpID;
    OCAC.charID = aInfo.charID;

    PyTuple* notif = OCAC.Encode();
    // Who needs to know this?
    // Everyone who's in that corporation, right?

    MulticastTarget mct;
        mct.characters.insert(OCAC.charID);
        mct.corporations.insert(OCAC.corpID);
    sEntityList.Multicast("OnCorporationApplicationChanged", "clientID", &notif, mct);

    if (IsPlayerCorp(res.corpID))
        m_db.AddItemEvent(res.corpID, call.client->GetCharacterID(), Corp::EventType::AppliedForMembershipOfCorporation);

    /// need to find out what happens on the other side, if there's anything at all on the other side

    /// Send an evemail to those who can decide
    /// Well, for the moment, send it to the ceo
    std::string subject = "New application from ";
    subject += call.client->GetName();
    std::vector<int32> recipients;
    recipients.push_back(m_db.GetCorporationCEO(res.corpID));
    m_manager->lsc_service->SendMail(call.client->GetCharacterID(), recipients, subject, res.message);

    /// Reply: ~\x00\x00\x00\x00\x01
    return nullptr;
}

void CorpRegistryBound::FillOCApplicationChange(OnCorporationApplicationChanged& OCAC, const ApplicationInfo& Old, const ApplicationInfo& New) {
    if (Old.valid) {
        OCAC.applicationIDOld = new PyInt(Old.appID);
        OCAC.applicationDateTimeOld = new PyLong(Old.appTime);
        OCAC.applicationTextOld = new PyString(Old.appText);
        OCAC.characterIDOld = new PyInt(Old.charID);
        OCAC.corporationIDOld = new PyInt(Old.corpID);
        OCAC.deletedOld = new PyInt(Old.deleted);
        OCAC.grantableRolesOld = new PyLong(Old.grantRole);
        if (Old.lastCID) {
            OCAC.lastCorpUpdaterIDOld = new PyInt(Old.lastCID);
        } else {
            OCAC.lastCorpUpdaterIDOld = PyStatic.NewNone();
        }
        OCAC.rolesOld = new PyLong(Old.role);
        OCAC.statusOld = new PyInt(Old.status);
    } else {
        OCAC.applicationIDOld = PyStatic.NewNone();
        OCAC.applicationDateTimeOld = PyStatic.NewNone();
        OCAC.applicationTextOld = PyStatic.NewNone();
        OCAC.characterIDOld = PyStatic.NewNone();
        OCAC.corporationIDOld = PyStatic.NewNone();
        OCAC.deletedOld = PyStatic.NewNone();
        OCAC.grantableRolesOld = PyStatic.NewNone();
        OCAC.lastCorpUpdaterIDOld = PyStatic.NewNone();
        OCAC.rolesOld = PyStatic.NewNone();
        OCAC.statusOld = PyStatic.NewNone();
    }

    if (New.valid) {
        OCAC.applicationIDNew = new PyInt(New.appID);
        OCAC.applicationDateTimeNew = new PyLong(New.appTime);
        OCAC.applicationTextNew = new PyString(New.appText);
        OCAC.characterIDNew = new PyInt(New.charID);
        OCAC.corporationIDNew = new PyInt(New.corpID);
        OCAC.deletedNew = new PyInt(New.deleted);
        OCAC.grantableRolesNew = new PyLong(New.grantRole);
        if (New.lastCID) {
            OCAC.lastCorpUpdaterIDNew = new PyInt(New.lastCID);
        } else {
            OCAC.lastCorpUpdaterIDNew = PyStatic.NewNone();
        }
        OCAC.rolesNew = new PyLong(New.role);
        OCAC.statusNew = new PyInt(New.status);
    } else {
        OCAC.applicationIDNew = PyStatic.NewNone();
        OCAC.applicationDateTimeNew = PyStatic.NewNone();
        OCAC.applicationTextNew = PyStatic.NewNone();
        OCAC.characterIDNew = PyStatic.NewNone();
        OCAC.corporationIDNew = PyStatic.NewNone();
        OCAC.deletedNew = PyStatic.NewNone();
        OCAC.grantableRolesNew = PyStatic.NewNone();
        OCAC.lastCorpUpdaterIDNew = PyStatic.NewNone();
        OCAC.rolesNew = PyStatic.NewNone();
        OCAC.statusNew = PyStatic.NewNone();
    }
}

PyResult CorpRegistryBound::Handle_UpdateApplicationOffer(PyCallArgs &call) {
    //     return self.GetCorpRegistry().UpdateApplicationOffer(characterID, applicationText, status, applicationDateTime = None) NOTE: time not used.
    Call_UpdateApplicationOffer args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return PyStatic.NewNone();
    }

    ApplicationInfo oldInfo;
        oldInfo.valid = true;
    if (!m_db.GetCurrentApplicationInfo(args.charID, m_corpID, oldInfo)) {
        codelog(SERVICE__ERROR, "%s: Failed to query application for char %u corp %u", call.client->GetName(), args.charID, m_corpID);
        return PyStatic.NewNone();
    }

    ApplicationInfo newInfo;
        newInfo = oldInfo;
        newInfo.valid = true;
        newInfo.status = args.newStatus;
        newInfo.lastCID = call.client->GetCharacterID();
        // newInfo.appTime = args.appDateTime;

    if (args.newStatus == Corp::AppStatus::RejectedByCorporation) {
        if (!m_db.UpdateApplication(newInfo)) {
            codelog(SERVICE__ERROR, "%s: Failed to update application", call.client->GetName());
            return PyStatic.NewNone();
        }
    }

    PyTuple* answer(nullptr);
    OnCorporationApplicationChanged OCAC;
        OCAC.charID = args.charID;
        OCAC.corpID = m_corpID;
    if (args.newStatus == Corp::AppStatus::AcceptedByCorporation) {  // accepted
        /** @todo check memberlimit */
        if (!m_db.UpdateApplication(newInfo)) {
            codelog(SERVICE__ERROR, "%s: Failed to update application for char %u corp %u", call.client->GetName(), OCAC.charID, m_corpID);
            return nullptr;
        }

        MemberAttributeUpdate change;
        if (!m_db.CreateMemberAttributeUpdate(oldInfo.corpID, args.charID, change)) {
            codelog(SERVICE__ERROR, "Couldn't get data from the character. Sorry.");
            return nullptr;
        }

        // OnObjectPublicAttributesUpdated event        <<<---  needs to be updated. do search in packet logs
        OnObjectPublicAttributesUpdated N_pau;
            N_pau.realRowCount = 4;
            N_pau.bindID = GetBindStr();
            N_pau.changePKIndexValue = args.charID;
            N_pau.changes = change.Encode();
        answer = N_pau.Encode();
        sEntityList.Multicast("OnObjectPublicAttributesUpdated", "objectID", &answer, NOTIF_DEST__CORPORATION, m_corpID);

        // OnCorporationMemberChanged event
        OnCorpMemberChange ocmc;
            ocmc.charID = args.charID;
            ocmc.newCorpID = change.corporationIDNew->AsInt()->value();
            ocmc.oldCorpID = change.corporationIDOld->AsInt()->value();
            ocmc.newDate = OCAC.applicationDateTimeNew->AsInt()->value();
            ocmc.oldDate = OCAC.applicationDateTimeOld->AsInt()->value();
        // both corporations' members will be notified about the change
        MulticastTarget both_corps;
            both_corps.corporations.insert(m_corpID);
            both_corps.corporations.insert(ocmc.oldCorpID);
        answer = ocmc.Encode();
        sEntityList.Multicast("OnCorporationMemberChanged", "corpid", &answer, both_corps);

        CorpData data;
            sItemFactory.db()->GetCorpData(args.charID, data);
            data.corpAccountKey = Account::KeyType::Cash;
            data.corpRole = Corp::Role::Member;
            data.rolesAtAll = Corp::Role::Member;
            data.rolesAtBase = Corp::Role::Member;
            data.rolesAtHQ = Corp::Role::Member;
            data.rolesAtOther = Corp::Role::Member;
            data.grantableRoles = Corp::Role::Member;
            data.grantableRolesAtBase = Corp::Role::Member;
            data.grantableRolesAtHQ = Corp::Role::Member;
            data.grantableRolesAtOther = Corp::Role::Member;
            data.corporationID = m_corpID;
        CorporationDB::GetCorpData(data);
        Client* recruit = sEntityList.FindClientByCharID(ocmc.charID);   // this returns nullptr for offline chars
        if (recruit != nullptr)
            recruit->GetChar()->JoinCorporation(data);
        else
            CharacterDB::AddEmployment(args.charID, m_corpID);

        // add corp events for changing both_corps
        if (IsPlayerCorp(ocmc.oldCorpID))
            m_db.AddItemEvent(ocmc.oldCorpID, args.charID, Corp::EventType::LeftCorporation);
        if (IsPlayerCorp(m_corpID))
            m_db.AddItemEvent(m_corpID, args.charID, Corp::EventType::JoinedCorporation);
        if (!m_db.JoinCorporation(args.charID, m_corpID, ocmc.oldCorpID, data)) {
            codelog(SERVICE__ERROR, "%s: Failed to record corp join for char %u corp %u", call.client->GetName(), OCAC.charID, m_corpID);
            return nullptr;
        }
    };

    FillOCApplicationChange(OCAC, oldInfo, newInfo);
    answer = OCAC.Encode();
    MulticastTarget mct;
        mct.characters.insert(OCAC.charID);
        mct.corporations.insert(m_corpID);
    sEntityList.Multicast("OnCorporationApplicationChanged", "*corpid&corprole", &answer, mct);

    return PyStatic.NewNone();
}

PyResult CorpRegistryBound::Handle_DeleteApplication(PyCallArgs & call) {
    sLog.White( "CorpRegistryBound", "Handle_GetFactionInfo() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return PyStatic.NewFalse();
    }

    OnCorporationApplicationChanged OCAC;
        OCAC.corpID = args.arg1;
        OCAC.charID = args.arg2;
    ApplicationInfo newInfo;
        newInfo.valid = false;
    ApplicationInfo oldInfo;
        oldInfo.valid = true;
    if (!m_db.GetCurrentApplicationInfo(OCAC.charID, OCAC.corpID, oldInfo)) {
        codelog(SERVICE__ERROR, "%s: Failed to query application info for char %u corp %u", call.client->GetName(), OCAC.charID, OCAC.corpID);
        return PyStatic.NewFalse();
    }

    FillOCApplicationChange(OCAC, oldInfo, newInfo);
    if (!m_db.DeleteApplication(oldInfo)) {
        codelog(SERVICE__ERROR, "%s: Failed to delete application info for char %u corp %u", call.client->GetName(), OCAC.charID, OCAC.corpID);
        return PyStatic.NewFalse();
    }

    PyTuple* answer = OCAC.Encode();
    MulticastTarget mct;
        mct.characters.insert(OCAC.charID);
        mct.corporations.insert(OCAC.corpID);
    sEntityList.Multicast("OnCorporationApplicationChanged", "*corpid&corprole", &answer, mct);

    return PyStatic.NewTrue();
}

PyResult CorpRegistryBound::Handle_UpdateApplication(PyCallArgs &call) {
    sLog.White( "CorpRegistryBound", "Handle_UpdateApplication() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_UpdateApplication args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    ApplicationInfo oldInfo;
    oldInfo.valid = true;
    OnCorporationApplicationChanged OCAC;
    OCAC.charID = call.client->GetCharacterID();
    OCAC.corpID = args.corpID;
    if (!m_db.GetCurrentApplicationInfo(OCAC.charID, OCAC.corpID, oldInfo)) {
        codelog(SERVICE__ERROR, "%s: Failed to query application info for char %u corp %u", call.client->GetName(), OCAC.charID, OCAC.corpID);
        return nullptr;
    }

    ApplicationInfo newInfo;
    newInfo.valid = true;
    newInfo = oldInfo;
    newInfo.appText = args.message;
    newInfo.status = args.status;

    if (!m_db.UpdateApplication(newInfo)) {
        codelog(SERVICE__ERROR, "%s: Failed to update application info for char %u corp %u", call.client->GetName(), OCAC.charID, OCAC.corpID);
        return nullptr;
    }

    FillOCApplicationChange(OCAC, oldInfo, newInfo);

    PyTuple* notif = OCAC.Encode();
    sEntityList.Unicast(OCAC.charID, "OnCorporationApplicationChanged", "clientID", &notif);

    ApplicationInfo invalidInfo;
    invalidInfo.valid = false;
    FillOCApplicationChange(OCAC, invalidInfo, newInfo);
    notif = OCAC.Encode();
    sEntityList.Multicast("OnCorporationApplicationChanged", "clientID", &notif, NOTIF_DEST__CORPORATION, OCAC.corpID);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_CanViewVotes(PyCallArgs &call) {
    //  bCan = self.GetCorpRegistry().CanViewVotes(corpid)
    sLog.White( "CorpRegistryBound::Handle_CanViewVotes()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return PyStatic.NewFalse();
}

PyResult CorpRegistryBound::Handle_InsertVoteCase(PyCallArgs &call) {
    //  return self.GetCorpRegistry().InsertVoteCase(voteCaseText, description, corporationID, voteType, voteCaseOptions, startDateTime, endDateTime)
    sLog.White( "CorpRegistryBound::Handle_InsertVoteCase()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    /*
     * 10:10:55 W CorpRegistryBound::Handle_InsertVoteCase(): size= 7
     * 10:10:55 [CorpCallDump]   Call Arguments:
     * 10:10:55 [CorpCallDump]       Tuple: 7 elements
     * 10:10:55 [CorpCallDump]       [ 0]    WString: 'test'
     * 10:10:55 [CorpCallDump]       [ 1]    WString: 'test vote'
     * 10:10:55 [CorpCallDump]       [ 2]    Integer: 98000000
     * 10:10:55 [CorpCallDump]       [ 3]    Integer: 4
     * 10:10:55 [CorpCallDump]       [ 4] Object:
     * 10:10:55 [CorpCallDump]       [ 4] Type:     String: 'util.Rowset'
     * 10:10:55 [CorpCallDump]       [ 4] Args: Dictionary: 3 entries
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Key:     String: 'lines'
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Value: List: 2 elements
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Value: [ 0] List: 4 elements
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Value: [ 0] [ 0]    WString: 'yay'
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Value: [ 0] [ 1]    Integer: 0
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Value: [ 0] [ 2]       None: 0
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Value: [ 0] [ 3]       None: 0
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Value: [ 1] List: 4 elements
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Value: [ 1] [ 0]    WString: 'nay'
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Value: [ 1] [ 1]    Integer: 1
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Value: [ 1] [ 2]       None: 0
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 0] Value: [ 1] [ 3]       None: 0
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 1] Key:     String: 'RowClass'
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 1] Value: Token: 'util.Row'
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 2] Key:     String: 'header'
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 2] Value: List: 4 elements
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 2] Value: [ 0]     String: 'choice'
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 2] Value: [ 1]     String: 'itemID'
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 2] Value: [ 2]     String: 'typeID'
     * 10:10:55 [CorpCallDump]       [ 4] Args: [ 2] Value: [ 3]     String: 'locationID'
     * 10:10:55 [CorpCallDump]       [ 5]       Long: 131575686552352973
     * 10:10:55 [CorpCallDump]       [ 6]       Long: 131576550552352973
     * 10:10:55 [CorpCallDump]   Call Named Arguments:
     * 10:10:55 [CorpCallDump]     Argument 'machoVersion':
     * 10:10:55 [CorpCallDump]            Integer: 1
     */

    if (!call.tuple->GetItem(4)->IsObject()) {
        codelog(CORP__ERROR, "Tuple Item is wrong type: %s.  Expected PyObject.", call.tuple->GetItem(0)->TypeString());
        return nullptr;
    }
    Call_InsertVoteCase args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    if (!args.voteCaseOptions->arguments()->IsDict()) {
        codelog(CORP__ERROR, "voteCaseOptions Argument is wrong type: %s.  Expected PyDict.", args.voteCaseOptions->arguments()->TypeString());
        return nullptr;
    }

    //m_db.AddVoteCase(m_corpID, call.client->GetCharacterID, args);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_GetVoteCasesByCorporation(PyCallArgs &call)
{   // not working....needs filters coded
    /*** from closed
     * 04:24:15 W CorpRegistryBound::Handle_GetVoteCasesByCorporation(): size= 3
     * 04:24:15 [CorpCallDump]   Call Arguments:
     * 04:24:15 [CorpCallDump]       Tuple: 3 elements
     * 04:24:15 [CorpCallDump]         [ 0] Integer field: 98000000
     * 04:24:15 [CorpCallDump]         [ 1] Integer field: 1
     * 04:24:15 [CorpCallDump]         [ 2] Integer field: 20
     *** from open
     * 04:24:20 W CorpRegistryBound::Handle_GetVoteCasesByCorporation(): size= 3
     * 04:24:20 [CorpCallDump]   Call Arguments:
     * 04:24:20 [CorpCallDump]       Tuple: 3 elements
     * 04:24:20 [CorpCallDump]         [ 0] Integer field: 98000000
     * 04:24:20 [CorpCallDump]         [ 1] Integer field: 2
     * 04:24:20 [CorpCallDump]         [ 2] Integer field: 0
     *
     */
    /*
        [PyObjectData Name: util.IndexRowset]
          [PyDict 4 kvp]
            [PyString "items"]
            [PyDict 0 kvp]
            [PyString "RowClass"]
            [PyToken util.Row]
            [PyString "idName"]
            [PyString "voteCaseID"]
            [PyString "header"]
            [PyList 8 items]
              [PyString "voteCaseID"]
              [PyString "voteCaseText"]
              [PyString "description"]
              [PyString "corporationID"]
              [PyString "characterID"]
              [PyString "voteType"]
              [PyString "startDateTime"]
              [PyString "endDateTime"]
              */
    sLog.White( "CorpRegistryBound::Handle_GetVoteCasesByCorporation()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return m_db.GetVoteItems(m_corpID);
}

PyResult CorpRegistryBound::Handle_GetSanctionedActionsByCorporation(PyCallArgs &call) {
    //  rows = sm.GetService('corp').GetSanctionedActionsByCorporation(eve.session.corpid, state)
    sLog.White( "CorpRegistryBound::Handle_GetSanctionedActionsByCorporation()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_GetRentalDetailsPlayer(PyCallArgs &call) {
    //return self.GetCorpRegistry().GetRentalDetailsPlayer()
    sLog.White( "CorpRegistryBound::Handle_GetRentalDetailsPlayer()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_GetRentalDetailsCorp(PyCallArgs &call) {
    // return self.GetCorpRegistry().GetRentalDetailsCorp()
    sLog.White( "CorpRegistryBound::Handle_GetRentalDetailsCorp()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_UpdateCorporationAbilities(PyCallArgs &call) {
    sLog.White( "CorpRegistryBound::Handle_UpdateCorporationAbilities()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    //this will need to update corp memberlimit, allowed races, and then update all members with new data

    return nullptr;
}

PyResult CorpRegistryBound::Handle_UpdateStationManagementSettings(PyCallArgs &call) {
    //  self.corpStationMgr.UpdateStationManagementSettings(self.modifiedServiceAccessRulesByServiceID, self.modifiedServiceCostModifiers, self.modifiedRentableItems, self.station.stationName, self.station.description, self.station.dockingCostPerVolume, self.station.officeRentalCost, self.station.reprocessingStationsTake, self.station.reprocessingHangarFlag, self.station.exitTime, self.station.standingOwnerID)

    sLog.White( "CorpRegistryBound::Handle_UpdateStationManagementSettings()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    // not real sure what this does yet....outpost shit maybe?

    return nullptr;
}

