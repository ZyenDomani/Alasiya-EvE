



#include "cache/ObjCacheService.h"
#include "chat/LSCService.h"
#include "corporation/CorpRegistryBound.h"

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

        PyCallable_REG_CALL(SparseCorpOfficeListBound, Fetch)
        //PyCallable_REG_CALL(SparseCorpOfficeListBound, FetchByKey)
        //PyCallable_REG_CALL(SparseCorpOfficeListBound, GetByKey)
    }
    virtual ~SparseCorpOfficeListBound() {delete m_dispatch;}
    virtual void Release() {
        delete this;
    }

    PyCallable_DECL_CALL(Fetch) //(startPos, fetchSize)
    //PyCallable_DECL_CALL(FetchByKey) //([keys])
    //PyCallable_DECL_CALL(GetByKey) //(key)


protected:
    Dispatcher *const m_dispatch;

    CorporationDB& m_db;
};

PyResult SparseCorpOfficeListBound::Handle_Fetch(PyCallArgs &call) {
    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    return m_db.Fetch(call.client->GetCorporationID(), args.arg1, args.arg2);
}



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
    PyCallable_REG_CALL(CorpRegistryBound, GetSharesByShareholder);
    PyCallable_REG_CALL(CorpRegistryBound, GetShareholders);
    PyCallable_REG_CALL(CorpRegistryBound, PayoutDividend);
    PyCallable_REG_CALL(CorpRegistryBound, GetVoteCasesByCorporation);

    PyCallable_REG_CALL(CorpRegistryBound, GetBulletins);
    PyCallable_REG_CALL(CorpRegistryBound, GetRecentKillsAndLosses);
    PyCallable_REG_CALL(CorpRegistryBound, GetRoleGroups);
    PyCallable_REG_CALL(CorpRegistryBound, GetRoles);
    PyCallable_REG_CALL(CorpRegistryBound, GetLocationalRoles);

    PyCallable_REG_CALL(CorpRegistryBound, GetTitles);
    PyCallable_REG_CALL(CorpRegistryBound, UpdateTitle);
    PyCallable_REG_CALL(CorpRegistryBound, UpdateTitles);
    PyCallable_REG_CALL(CorpRegistryBound, DeleteTitle);

    PyCallable_REG_CALL(CorpRegistryBound, GetCorporateContacts);
    PyCallable_REG_CALL(CorpRegistryBound, AddCorporateContact);
    PyCallable_REG_CALL(CorpRegistryBound, EditContactsRelationshipID);
    PyCallable_REG_CALL(CorpRegistryBound, RemoveCorporateContacts);
    PyCallable_REG_CALL(CorpRegistryBound, EditCorporateContact);

    PyCallable_REG_CALL(CorpRegistryBound, CreateAlliance);
    PyCallable_REG_CALL(CorpRegistryBound, GetSuggestedAllianceShortNames);

    /*
    def GetMembersPaged(self, page):
        return self.GetCorpRegistry().GetMembersPaged(page)

    def GetMembersByIds(self, memberIDs):
        return self.GetCorpRegistry().GetMembersByIds(memberIDs)
    def GetMemberIDsWithMoreThanAvgShares(self):
        return self.GetCorpRegistry().GetMemberIDsWithMoreThanAvgShares()

    def GetMemberIDsByQuery(self, query, includeImplied, searchTitles):
        return self.GetCorpRegistry().GetMemberIDsByQuery(query, includeImplied, searchTitles)

        if eve.session.corprole & const.corpRoleDirector:
            return self.GetCorpRegistry().GetMemberTrackingInfo()
        else:
            return self.GetCorpRegistry().GetMemberTrackingInfoSimple()

    def GetRentalDetailsPlayer(self):
        return self.GetCorpRegistry().GetRentalDetailsPlayer()

    def GetRentalDetailsCorp(self):
        return self.GetCorpRegistry().GetRentalDetailsCorp()
*/

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
    return call.client->GetInfoWindowDataForChar(call.client);
}

PyResult CorpRegistryBound::Handle_GetCorporation(PyCallArgs &call) {
    return m_db.GetCorporation(call.client->GetCorporationID());
}

PyResult CorpRegistryBound::Handle_GetCorporations(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }
    return m_db.GetCorporations(args.arg);
}

PyResult CorpRegistryBound::Handle_AddCorporation(PyCallArgs &call) {
    Call_AddCorporation args;

    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    //first make sure the char can even afford it.
    int32 corp_cost = sConfig.rates.corpCost;

    if (call.client->GetBalance() < corp_cost) {
        _log(SERVICE__ERROR, "%s: Cannot afford corporation startup costs!", call.client->GetName());
        return nullptr;
    }

    //take the money out of their wallet (sends wallet blink event)
    // The amount has to be double!!!
    if (!call.client->AddBalance(-corp_cost)) {
        _log(SERVICE__ERROR, "%s: Failed to take money for corp startup!", call.client->GetName());
        return nullptr;
    }

    // Register new corp
    uint32 corpID;
    if (!m_db.AddCorporation(args, call.client->GetCharacterID(), call.client->GetStationID(), corpID)) {
        codelog(SERVICE__ERROR, "New corporation creation failed...");
        // return monies?
        return nullptr;
    }
    //adding a corporation might affect eveStaticOwners, so we gotta invalidate the cache...
    PyString* cache_name = new PyString( "config.StaticOwners" );
    m_manager->cache_service->InvalidateCache( cache_name );
    PySafeDecRef( cache_name );

    //record the transaction in their journal.
    std::string reason = "Creating new corporation: ";
    reason += args.corpName;
    if (!m_db.GiveCash(
        call.client->GetCharacterID(),
        refCorporationRegistrationFee,
        call.client->GetCharacterID(),  //eve system
        1,
        "1",
        call.client->GetUserID(),
        accountingKeyCash,
        -corp_cost,
        call.client->GetBalance(),
        reason.c_str()
        )
    ) {
        codelog(DATABASE__ERROR, "Failed to record corp creation transaction.");
    }

    // create default role title data
    m_db.CreateTitleData(corpID);

    uint32 location = call.client->GetLocationID();

    // Here we send a notification about creating a new corporation...
    Notify_OnCorporationChanged cc;
    cc.corpID = corpID;
    if (!m_db.CreateCorporationCreatePacket(cc, call.client->GetCorporationID(), corpID)) {
        codelog(SERVICE__ERROR, "Failed to create OnCorpChanged notification stream.");
        // This is a big problem, because this way we won't be able to see the difference...
        call.client->SendErrorMsg("Unable to notify about corp creation. Try logging in again.");
        return nullptr;
    }
    PyTuple* a1 = cc.Encode();
    PyTuple* a2 = cc.Encode();
    sEntityList.Multicast("OnCorporationChanged", "clientID", &a1, NOTIF_DEST__LOCATION, location);
    sEntityList.Multicast("OnCorporationChanged", "stationid", &a2, NOTIF_DEST__LOCATION, location);

    // Set char's roles in corp
    CorpData roles;
        roles.corpAccountKey = accountingKeyCash;
        roles.corpRole = corpRoleAdmin;
        roles.rolesAtAll = corpRoleAdmin;
        roles.rolesAtBase = corpRoleAdmin;
        roles.rolesAtHQ = corpRoleAdmin;
        roles.rolesAtOther = corpRoleAdmin;

    //loads up roles and alters session.
    if (!JoinCorporation(call.client, corpID, roles)) {
        _log(CLIENT__ERROR, "Failed to force char '%s' to join new corporation %u. This will be interesting.", call.client->GetName(), corpID);
        return nullptr;
    }

    return m_db.GetCorporations(corpID);
}

bool CorpRegistryBound::JoinCorporation(Client *who, uint32 newCorpID, const CorpData &roles) {

    who->GetChar()->JoinCorporation(newCorpID, roles);

    //who->JoinCorporationUpdate(newCorpID);
    return true;
}

PyResult CorpRegistryBound::Handle_GetMember(PyCallArgs &call) {

    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    return m_db.GetMember(args.arg);
}

PyResult CorpRegistryBound::Handle_GetMembers(PyCallArgs &call) {
    /*
    sLog.White( "CorpRegistryBound::Handle_GetMembers()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);
    */
    DBQueryResult res;
    m_db.GetMembers(m_corpID, res);
    uint32 rowCount = (uint32)res.GetRowCount();

    GetMembersSparseRowset ret;
    /*
    DBResultRow row;
    while (res.GetRow(row)) {
        GetMembersRet mRet;
        mRet.characterID = row.GetInt(0);
        mRet.corporationID = m_corpID;
        mRet.divisionID = 0;
        mRet.squadronID = 0;
        mRet.title = row.GetText(1);
        mRet.roles = row.GetUInt64(2);
        mRet.grantableRoles = row.GetUInt64(3);
        mRet.startDateTime = row.GetUInt64(4);
        mRet.baseID = 0;
        mRet.rolesAtHQ = row.GetUInt64(5);
        mRet.grantableRolesAtHQ = row.GetUInt64(6);
        mRet.rolesAtBase = row.GetUInt64(7);
        mRet.grantableRolesAtBase = row.GetUInt64(8);
        mRet.rolesAtOther = row.GetUInt64(9);
        mRet.grantableRolesAtOther = row.GetUInt64(10);
        mRet.titleMask = 0;
        mRet.accountKey = row.GetInt(11);
        mRet.rowDate = Win32TimeNow();
        mRet.blockRoles = 0;
    }
    */
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

PyResult CorpRegistryBound::Handle_GetRoleGroups(PyCallArgs &call) {
    return m_db.GetCorpRoleGroups();
}

PyResult CorpRegistryBound::Handle_GetRoles(PyCallArgs &call) {
    return m_db.GetCorpRoles();
}

PyResult CorpRegistryBound::Handle_GetTitles(PyCallArgs &call) {
    return m_db.GetTitles(m_corpID);
}

PyResult CorpRegistryBound::Handle_UpdateTitle(PyCallArgs &call) {
    /*
    def UpdateTitle(self, titleID, titleName, roles, grantableRoles, rolesAtHQ, grantableRolesAtHQ, rolesAtBase, grantableRolesAtBase, rolesAtOther, grantableRolesAtOther):
    self.GetCorpRegistry().UpdateTitle(titleID, titleName, roles, grantableRoles, rolesAtHQ, grantableRolesAtHQ, rolesAtBase, grantableRolesAtBase, rolesAtOther, grantableRolesAtOther)
    */
    sLog.White( "CorpRegistryBound::Handle_UpdateTitle()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);
    return nullptr;
}

PyResult CorpRegistryBound::Handle_UpdateTitles(PyCallArgs &call) {
    /*
    def UpdateTitles(self, titles):
    self.GetCorpRegistry().UpdateTitles(titles)
    */
    sLog.White( "CorpRegistryBound::Handle_UpdateTitles()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);
    return nullptr;
}

PyResult CorpRegistryBound::Handle_DeleteTitle(PyCallArgs &call) {
    /*
    def DeleteTitle(self, titleID):
    self.GetCorpRegistry().DeleteTitle(titleID)
    */
    sLog.White( "CorpRegistryBound::Handle_DeleteTitle()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);
    return nullptr;
}

PyResult CorpRegistryBound::Handle_UpdateMember(PyCallArgs &call) {
    sLog.White( "CorpRegistryBound::Handle_UpdateMember()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);
    //return self.GetCorpRegistry().UpdateMember(charIDToUpdate, title, divisionID, squadronID, roles, grantableRoles, rolesAtHQ, grantableRolesAtHQ, rolesAtBase, grantableRolesAtBase, rolesAtOther, grantableRolesAtOther, baseID, titleMask, blockRoles)

    return nullptr;
}

PyResult CorpRegistryBound::Handle_GetLocationalRoles(PyCallArgs &call) {
    sLog.White( "CorpRegistryBound::Handle_GetLocationalRoles()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);
    return nullptr;
}

PyResult CorpRegistryBound::Handle_GetBulletins(PyCallArgs &call) {
    sLog.White( "CorpRegistryBound::Handle_GetBulletins()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);
    return nullptr;
}

PyResult CorpRegistryBound::Handle_GetCorporateContacts(PyCallArgs &call)
{
    return m_db.GetContacts(m_corpID);
}

PyResult CorpRegistryBound::Handle_AddCorporateContact(PyCallArgs &call)
{
    //self.GetCorpRegistry().AddCorporateContact(contactID, relationshipID)
    sLog.White( "CorpRegistryBound::Handle_AddCorporateContact()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);
    return nullptr;
}

PyResult CorpRegistryBound::Handle_EditCorporateContact(PyCallArgs &call)
{
    //self.GetCorpRegistry().EditCorporateContact(contactID, relationshipID)
    sLog.White( "CorpRegistryBound::Handle_EditCorporateContact()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);
    return nullptr;
}

PyResult CorpRegistryBound::Handle_EditContactsRelationshipID(PyCallArgs &call)
{
    //self.GetCorpRegistry().EditContactsRelationshipID(contactIDs, relationshipID)
    sLog.White( "CorpRegistryBound::Handle_EditContactsRelationshipID()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);
    return nullptr;
}

PyResult CorpRegistryBound::Handle_RemoveCorporateContacts(PyCallArgs &call) {
   // self.GetCorpRegistry().RemoveCorporateContacts(contactIDs)
    sLog.White( "CorpRegistryBound::Handle_RemoveCorporateContacts()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);
    return nullptr;
}


PyResult CorpRegistryBound::Handle_GetRecentKillsAndLosses(PyCallArgs &call) {
    sLog.White( "CorpRegistryBound::Handle_GetRecentKillsAndLosses()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);
    return nullptr;
}

PyResult CorpRegistryBound::Handle_CreateAlliance(PyCallArgs &call) {
    //self.GetCorpRegistry().CreateAlliance(allianceName, shortName, description, url)
    sLog.White("CorpRegistryBound", "Handle_CreateAlliance() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_GetSuggestedAllianceShortNames(PyCallArgs &call) {
    sLog.White("CorpRegistryBound", "Handle_GetSuggestedAllianceShortNames() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_GetLockedItemLocations( PyCallArgs& call )
{    //takes characterID
    sLog.White( "CorpRegistryBound::Handle_GetLockedItemLocations()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    //this returns an empty list for me on live.
    return new PyList;
}

PyResult CorpRegistryBound::Handle_GetSuggestedTickerNames(PyCallArgs &call) {
    Call_SingleWStringArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    PyList * result = new PyList;
    Item_GetSuggestedTickerNames sTN;
    sTN.tN = "";
    uint32 cnLen = args.arg.length();
    // Easiest ticker-generation method: get the capital letters.
    for (uint32 i=0;i<cnLen;i++) {
        if (args.arg[i] >= 'A' && args.arg[i] <= 'Z') {
            sTN.tN += args.arg[i];
        }
    }
    result->AddItem( sTN.Encode() );

    return result;
}

PyResult CorpRegistryBound::Handle_GetStations(PyCallArgs &call) {
    // No param

    // Need to fetch stations of current corporation...
    return m_db.GetStations(call.client->GetCorporationID());
}

PyResult CorpRegistryBound::Handle_GetOffices(PyCallArgs &call) {
    PyBoundObject *bObj;
    bObj = new SparseCorpOfficeListBound(m_manager, m_db);
    if (bObj == NULL) {
        _log(SERVICE__ERROR, "%s: Unable to create bound object for:", call.client->GetName()); //errors here
        return nullptr;
    }

    /** @todo this is wrong....to fix later */
    /*
    [PyTuple 1 items]
      [PySubStream 114 bytes]
        [PyObjectData Name: util.SparseRowset]
          [PyTuple 3 items]
            [PyList 4 items]
              [PyString "stationID"]
              [PyString "typeID"]
              [PyString "officeID"]
              [PyString "officeFolderID"]
            [PySubStruct]
              [PySubStream 50 bytes]
                [PyTuple 3 items]
                  [PyString "N=789442:2172"]
                  [PyDict 1 kvp]
                    [PyString "realRowCount"]
                    [PyInt 8]
                  [PyIntegerVar 129753802088805346]
            [PyInt 8]
    [PyDict 1 kvp]
      [PyString "OID+"]
      [PyDict 1 kvp]
        [PyString "N=789442:2172"]
        [PyIntegerVar 129753802088805346]
        */
    CorpOfficeSparseRowset ret;

    //now we register
    PyDict *dict = new PyDict();

    // First time we only need the number of rows, not the data itself
    // Data will be fetched from the SparseRowset
    uint32 officeN = m_db.GetOffices(call.client->GetCorporationID());

    // No idea what this is
    dict->SetItemString("realRowCount", new PyInt(officeN));
    // But this one holds the real row number
    ret.officeNumber = officeN;

    ret.bindedObject = m_manager->BindObject(call.client, bObj, &dict);

    //call.client->temp_hack_officeLists[call.client->GetCorporationID()] = bindID; //m_manager->FindBoundObject(bObj);

    PyObject * res = ret.Encode();
    return res;
}

PyResult CorpRegistryBound::Handle_GetMyApplications(PyCallArgs &call) {
    /// We have a dict
    /// With an STI and an integer
    /// Ignore them for now
    return m_db.GetMyApplications(call.client->GetCharacterID());
}

PyResult CorpRegistryBound::Handle_InsertApplication(PyCallArgs &call) {
    /** Incoming:
     *  Integer: 777777777 <- corp id
     *  String: "Ignore me" <- text that was entered into the box
     */

    Call_InsertApplication res;
    if (!res.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    /// Insert query into the db
    ApplicationInfo aInfo;
    aInfo.charID = call.client->GetCharacterID();
    aInfo.corpID = res.corpID;
    aInfo.appText = res.message;
    if (!m_db.InsertApplication(aInfo)) {
        codelog(SERVICE__ERROR, "%s: Failed to insert application request", call.client->GetName());
        return nullptr;
    }

    /// BroadcastStuff::Notify( OnCorporationApplicationChanged ,...)
    Notify_OnCorporationApplicationChanged OCAC;
    ApplicationInfo oldInfo(false);
    FillOCApplicationChange(OCAC, oldInfo, aInfo);
    OCAC.corpID = res.corpID;
    OCAC.charID = aInfo.charID;

    PyTuple* notif = OCAC.Encode();
    // Who needs to know this?
    // Everyone who's in that corporation, right?

    MulticastTarget mct;
    mct.characters.insert(OCAC.charID);
    mct.corporations.insert(OCAC.corpID);
    sEntityList.Multicast(
        "OnCorporationApplicationChanged",
        "clientID", &notif, mct);


    /// need to find out what happens on the other side
    /// if there's anything at all on the other side

    /// Send an evemail to those who can decide
    /// Well, for the moment, send it to the ceo
    std::string
    subject = std::string("New application from ") + call.client->GetName(),
    body = res.message;
    std::vector<int32> recipients;
    recipients.push_back(m_db.GetCorporationCEO(res.corpID));
    m_manager->lsc_service->SendMail(call.client->GetCharacterID(), recipients, subject, body);

    /// Reply: ~\x00\x00\x00\x00\x01
    return nullptr;
}

void CorpRegistryBound::FillOCApplicationChange(Notify_OnCorporationApplicationChanged & OCAC, const ApplicationInfo & Old, const ApplicationInfo & New) {
    if (Old.valid) {
        OCAC.applicationDateTimeOld = new PyLong(Old.appTime);
        OCAC.applicationTextOld = new PyString(Old.appText);
        OCAC.characterIDOld = new PyInt(Old.charID);
        OCAC.corporationIDOld = new PyInt(Old.corpID);
        OCAC.deletedOld = new PyInt(Old.deleted);
        OCAC.grantableRolesOld = new PyLong(Old.grantRole);
        if (Old.lastCID) {
            OCAC.lastCorpUpdaterIDOld = new PyInt(Old.lastCID);
        } else {
            OCAC.lastCorpUpdaterIDOld = new PyNone();
        }
        OCAC.rolesOld = new PyLong(Old.role);
        OCAC.statusOld = new PyInt(Old.status);
    } else {
        OCAC.applicationDateTimeOld = new PyNone();
        OCAC.applicationTextOld = new PyNone();
        OCAC.characterIDOld = new PyNone();
        OCAC.corporationIDOld = new PyNone();
        OCAC.deletedOld = new PyNone();
        OCAC.grantableRolesOld = new PyNone();
        OCAC.lastCorpUpdaterIDOld = new PyNone();
        OCAC.rolesOld = new PyNone();
        OCAC.statusOld = new PyNone();
    }

    if (New.valid) {
        OCAC.applicationDateTimeNew = new PyLong(New.appTime);
        OCAC.applicationTextNew = new PyString(New.appText);
        OCAC.characterIDNew = new PyInt(New.charID);
        OCAC.corporationIDNew = new PyInt(New.corpID);
        OCAC.deletedNew = new PyInt(New.deleted);
        OCAC.grantableRolesNew = new PyLong(New.grantRole);
        if (New.lastCID) {
            OCAC.lastCorpUpdaterIDNew = new PyInt(New.lastCID);
        } else {
            OCAC.lastCorpUpdaterIDNew = new PyNone();
        }
        OCAC.rolesNew = new PyLong(New.role);
        OCAC.statusNew = new PyInt(New.status);
    } else {
        OCAC.applicationDateTimeNew = new PyNone();
        OCAC.applicationTextNew = new PyNone();
        OCAC.characterIDNew = new PyNone();
        OCAC.corporationIDNew = new PyNone();
        OCAC.deletedNew = new PyNone();
        OCAC.grantableRolesNew = new PyNone();
        OCAC.lastCorpUpdaterIDNew = new PyNone();
        OCAC.rolesNew = new PyNone();
        OCAC.statusNew = new PyNone();
    }
}

PyResult CorpRegistryBound::Handle_GetApplications(PyCallArgs &call) {
    return m_db.GetApplications(call.client->GetCorporationID());
}

/** AppInfo:
 *  status / corp side / user side
 *    0        new         applied
 *    1        update      reneg
 *    2        accepted    accepted
 *    4        error       reject
 *    6        offer       offer
 */
typedef enum {  //from eveConstants
    crpApplicationAppliedByCharacter = 0,
    crpApplicationRenegotiatedByCharacter = 1,
    crpApplicationAcceptedByCharacter = 2,
    crpApplicationRejectedByCharacter = 3,
    crpApplicationRejectedByCorporation = 4,
    crpApplicationRenegotiatedByCorporation = 5,
    crpApplicationAcceptedByCorporation = 6
} CorpApplicationStatus;

PyResult CorpRegistryBound::Handle_UpdateApplicationOffer(PyCallArgs &call) {
    sLog.White( "CorpRegistryBound::Handle_UpdateApplicationOffer()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);
    /** Incoming:
     *  Tuple
     *   - int 140000017    <- this is the charID, whose app should be handled
     *   - string message
     *   - int decision
     *      4: rejection
     *      6: acception
     *   - (none), so far
     */

    Call_UpdateApplicationOffer args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return new PyNone();
    }

    // OnCorporationApplicationChanged event, probably be good to make it two (or more) times, independently, depending on update type

    Notify_OnCorporationApplicationChanged OCAC;
    PyTuple * answer;


    switch (args.newStatus) {
        case crpApplicationRejectedByCorporation:
        {
            ApplicationInfo newInfo(true);
            ApplicationInfo oldInfo(true);
            ApplicationInfo invalidInfo(false);
            OCAC.charID = args.charID;
            OCAC.corpID = call.client->GetCorporationID();
            if (!m_db.GetCurrentApplicationInfo(OCAC.charID, OCAC.corpID, oldInfo)) {
                codelog(SERVICE__ERROR, "%s: Failed to query application for char %u corp %u", call.client->GetName(), OCAC.charID, OCAC.corpID);
                return new PyNone();
            }
            newInfo = oldInfo;
            newInfo.status = crpApplicationRejectedByCorporation;
            newInfo.lastCID = call.client->GetCharacterID();

            if (!m_db.UpdateApplication(newInfo)) {
                codelog(SERVICE__ERROR, "%s: Failed to update application", call.client->GetName());
                return new PyNone();
            }

            FillOCApplicationChange(OCAC, oldInfo, newInfo);
            answer = OCAC.Encode();
            sEntityList.Unicast(OCAC.charID,
                                "OnCorporationApplicationChanged",
                                "*corpid&corprole", &answer);

            FillOCApplicationChange(OCAC, oldInfo, invalidInfo);
            answer = OCAC.Encode();
            // Maybe this will remove the app from the corp
            sEntityList.Multicast(
                "OnCorporationApplicationChanged",
                "*corpid&corprole", &answer,
                NOTIF_DEST__CORPORATION, OCAC.corpID);
        } break;
        case crpApplicationAcceptedByCorporation: /// accepted
        {
            // the acceptor corporation MUST have free space!!
            /// OnCorporationApplicationChanged
            ApplicationInfo newInfo(true);
            ApplicationInfo oldInfo(true);
            OCAC.charID = args.charID;
            OCAC.corpID = call.client->GetCorporationID();
            if (!m_db.GetCurrentApplicationInfo(OCAC.charID, OCAC.corpID, oldInfo)) {
                codelog(SERVICE__ERROR, "%s: Failed to query application info for char %u corp %u", call.client->GetName(), OCAC.charID, OCAC.corpID);
                return nullptr;
            }
            newInfo = oldInfo;
            newInfo.status = crpApplicationAcceptedByCharacter;
            newInfo.lastCID = call.client->GetCharacterID();

            if (!m_db.UpdateApplication(newInfo)) {
                codelog(SERVICE__ERROR, "%s: Failed to update application for char %u corp %u", call.client->GetName(), OCAC.charID, OCAC.corpID);
                return nullptr;
            }

            FillOCApplicationChange(OCAC, oldInfo, newInfo);

            answer = OCAC.Encode();
            MulticastTarget mct;
            mct.characters.insert(OCAC.charID);
            mct.corporations.insert(OCAC.corpID);
            sEntityList.Multicast(
                "OnCorporationApplicationChanged",
                "*corpid&corprole", &answer, mct);

            //TODO: should probably put this into a function, since there may be other
            //places (gm commands at a minimum) where we want to change corp.
            /** TODO: Update employment history object, if present
             */
            // OnObjectPublicAttributesUpdated event        <<<---  needs to be updated. do search in packet logs
            Notify_OnObjectPublicAttributesUpdated N_pau;
            MemberAttributeUpdate change;

            N_pau.realRowCount = 4;
            N_pau.bindID = GetBindStr();
            N_pau.changePKIndexValue = args.charID;

            if (!m_db.CreateMemberAttributeUpdate(change, oldInfo.corpID, args.charID)) {
                codelog(SERVICE__ERROR, "Couldn't get data from the character. Sorry.");
                return nullptr;
            }

            N_pau.changes = change.Encode();

            answer = N_pau.Encode();
            sEntityList.Multicast(
                "OnObjectPublicAttributesUpdated",
                "objectID", &answer,
                NOTIF_DEST__CORPORATION, OCAC.corpID);

            // OnCorporationMemberChanged event
            Notify_OnCorpMemberChange ocmc;

            ocmc.charID = args.charID;
            ocmc.newCorpID = change.corporationIDNew->AsInt()->value();
            ocmc.oldCorpID = change.corporationIDOld->AsInt()->value();
            ocmc.newDate = OCAC.applicationDateTimeNew->AsInt()->value();
            ocmc.oldDate = OCAC.applicationDateTimeOld->AsInt()->value();

            // both corporations' members will be notified about the change
            MulticastTarget both_corps;
            both_corps.corporations.insert(ocmc.newCorpID);
            both_corps.corporations.insert(ocmc.oldCorpID);
            answer = ocmc.Encode();
            sEntityList.Multicast(
                "OnCorporationMemberChanged", "corpid",
                &answer, both_corps);

            //NOTE: this really should happen sooner, in case it fails.
            if (!m_db.JoinCorporation(args.charID, ocmc.newCorpID, ocmc.oldCorpID, CorpData())) {
                codelog(SERVICE__ERROR, "%s: Failed to record corp join for char %u corp %u", call.client->GetName(), OCAC.charID, OCAC.corpID);
                return nullptr;
            }

            Client* recruit = sEntityList.FindClientByCharID(ocmc.charID);
            if (recruit)
                recruit->UpdateCorpSession(recruit->GetChar().get());

        } break;
    }

    return new PyNone();
}

PyResult CorpRegistryBound::Handle_DeleteApplication(PyCallArgs & call) {
    /** Incoming:
     *  tuple of 2 elements, corpID and charID
     */
    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    Notify_OnCorporationApplicationChanged OCAC;

    ApplicationInfo newInfo(false);
    ApplicationInfo oldInfo(true);
    OCAC.corpID = args.arg1;
    OCAC.charID = args.arg2;
    if (!m_db.GetCurrentApplicationInfo(OCAC.charID, OCAC.corpID, oldInfo)) {
        codelog(SERVICE__ERROR, "%s: Failed to query application info for char %u corp %u", call.client->GetName(), OCAC.charID, OCAC.corpID);
        return nullptr;
    }

    FillOCApplicationChange(OCAC, oldInfo, newInfo);

    if (!m_db.DeleteApplication(oldInfo)) {
        codelog(SERVICE__ERROR, "%s: Failed to delete application info for char %u corp %u", call.client->GetName(), OCAC.charID, OCAC.corpID);
        return nullptr;
    }

    PyTuple * answer = OCAC.Encode();
    MulticastTarget mct;
    mct.characters.insert(OCAC.charID);
    mct.corporations.insert(OCAC.corpID);
    sEntityList.Multicast(
        "OnCorporationApplicationChanged",
        "*corpid&corprole", &answer, mct);

    return new PyInt(1);
}

PyResult CorpRegistryBound::Handle_UpdateApplication(PyCallArgs &call) {
    Call_UpdateApplication args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    ApplicationInfo oldInfo(true);
    ApplicationInfo newInfo;
    Notify_OnCorporationApplicationChanged OCAC;
    OCAC.charID = call.client->GetCharacterID();
    OCAC.corpID = args.corpID;
    if (!m_db.GetCurrentApplicationInfo(OCAC.charID, OCAC.corpID, oldInfo)) {
        codelog(SERVICE__ERROR, "%s: Failed to query application info for char %u corp %u", call.client->GetName(), OCAC.charID, OCAC.corpID);
        return nullptr;
    }
    newInfo = oldInfo;
    newInfo.appText = args.message;
    newInfo.status = args.status;

    if (!m_db.UpdateApplication(newInfo)) {
        codelog(SERVICE__ERROR, "%s: Failed to update application info for char %u corp %u", call.client->GetName(), OCAC.charID, OCAC.corpID);
        return nullptr;
    }

    FillOCApplicationChange(OCAC, oldInfo, newInfo);

    PyTuple* notif = OCAC.Encode();
    sEntityList.Unicast(OCAC.charID,
                        "OnCorporationApplicationChanged",
                        "clientID", &notif);

    FillOCApplicationChange(OCAC, ApplicationInfo(false), newInfo);
    notif = OCAC.Encode();
    sEntityList.Multicast(
        "OnCorporationApplicationChanged",
        "clientID", &notif,
        NOTIF_DEST__CORPORATION, OCAC.corpID);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_UpdateDivisionNames(PyCallArgs &call) {
    Call_UpdateDivisionNames args;

    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return new PyNone();
    }

    Notify_IntRaw notif;
    notif.data = new PyDict();
    notif.key = call.client->GetCorporationID();

    if (!m_db.UpdateDivisionNames(notif.key, args, (PyDict *)notif.data)) {
        codelog(SERVICE__ERROR, "%s: Failed to update division names for corp %u", call.client->GetName(), notif.key);
        return new PyNone();
    }

    MulticastTarget mct;
    mct.corporations.insert(notif.key);
    PyTuple * answer = notif.Encode();
    sEntityList.Multicast("OnCorporationChanged", "corpid", &answer, mct);
    answer = notif.Encode();
    sEntityList.Multicast("OnCorporationChanged", "clientID", &answer, mct);

    return new PyNone();
}

PyResult CorpRegistryBound::Handle_UpdateCorporation(PyCallArgs &call) {
    Call_UpdateCorporation upd;

    if (!upd.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    Notify_IntRaw notif;
    notif.key = call.client->GetCorporationID();
    notif.data = new PyDict();

    if (!m_db.UpdateCorporation(notif.key, upd, (PyDict*)notif.data)) {
        codelog(SERVICE__ERROR, "%s: Failed to update corporation data for corp %u", call.client->GetName(), notif.key);
        return new PyNone();
    }

    // Only send notification if it is needed...
    if (((PyDict*)notif.data)->items.size()) {
        MulticastTarget mct;
        mct.corporations.insert(notif.key);
        PyTuple * answer = notif.Encode();
        sEntityList.Multicast("OnCorporationChanged", "corpid", &answer, mct);
    }

    return new PyNone();
}

PyResult CorpRegistryBound::Handle_UpdateLogo(PyCallArgs &call) {
    Call_UpdateLogo upd;

    if (!upd.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    // Check if we have enough money
    uint32 logo_changeu;
    double logo_change;
    if (!m_db.GetConstant("corpLogoChangeCost", logo_changeu)) {
        codelog(SERVICE__ERROR, "%s: Failed to determine logo change costs.", call.client->GetName());
        return(new PyNone());
    }
    logo_change = logo_changeu;


    // It's here, to avoid callign GetCorporationID all the time
    Notify_IntRaw notif;
    notif.key = call.client->GetCorporationID();
    notif.data = new PyDict();

    uint16 accountKey = accountingKeyCash;  //FIXME  get proper corp wallet division
    double corp_orig = m_db.GetCorpBalance(notif.key, accountKey);
    if ( corp_orig < logo_change )
    {
        _log( SERVICE__ERROR, "%s: Cannot afford corporation logo change costs!", call.client->GetName() );
        call.client->SendErrorMsg( "Your corporation doesn't have enough money (%u ISK) to change it's logo!", logo_changeu );

        PyDecRef( notif.data );
        return new PyNone();
    }

    // Try to do the update. If it fails, we won't take the money.
    if ( !m_db.UpdateLogo( notif.key, upd, (PyDict*)notif.data ) )
    {
        codelog( SERVICE__ERROR, "Corporation logo change failed..." );

        PyDecRef( notif.data );
        return new PyNone();
    }

    //take the money out of their wallet (sends wallet blink event)
    // The amount has to be double!!!
    if ( !m_db.AddBalanceToCorp( notif.key, -logo_change ) )
    {
        codelog( SERVICE__ERROR, "%s: Failed to take money for corp logo change!", call.client->GetName() );

        PyDecRef( notif.data );
        return new PyNone();
    }

    double corp_new = m_db.GetCorpBalance(notif.key, accountKey);

    //record the transaction in the journal.
    if (!m_db.GiveCash(
        notif.key,
        refCorporationLogoChangeCost,
        notif.key,
        1,      // who should this one be? hq's station's owner?
        "1",
        notif.key,
        accountingKeyCash,
        -logo_change,
        corp_new,
        "Changing own corporation logo."
    )
    ) {
        codelog(DATABASE__ERROR, "Failed to record corp logo change transaction.");
        //no good reason to return... the money has actually been moved.
    }

    // Send notification about the cash change
    OnAccountChange oac;
    oac.accountKey = "cash";
    oac.balance = corp_new;
    oac.ownerid = notif.key;
    PyTuple * answer = oac.Encode();

    MulticastTarget mct;
    mct.corporations.insert(notif.key);
    sEntityList.Multicast("OnAccountChange", "*corpid&corpAccountKey", &answer, mct);

    // for those in the station
    mct.locations.insert(call.client->GetLocationID());
    answer = notif.Encode();
    sEntityList.Multicast("OnCorporationChanged", "corpid", &answer, mct);

    return m_db.GetCorporation(notif.key);
}

//22:31:22 L CorpRegistryBound::Handle_GetSharesByShareholder(): size= 1
PyResult CorpRegistryBound::Handle_GetSharesByShareholder(PyCallArgs &call) {
    /*
     16:55:44 L CorpRegistryBound::Handle_GetSharesByShareholder(): size= 1, 0=Boolean
     16:55:44 [SvcCall]   Call Arguments:
     16:55:44 [SvcCall]       Tuple: 1 elements
     16:55:44 [SvcCall]         [ 0] Boolean field: false
     16:55:44 [SvcCall]   Call Named Arguments:
     16:55:44 [SvcCall]     Argument 'machoVersion':
     16:55:44 [SvcCall]         Integer field: 1

     sLog.White( "CorpRegistryService::Handle_GetSharesByShareholder()", "size= %u", call.tuple->size() );
     call.Dump(CORP__CALL_DUMP);

     [PyObjectData Name: util.Rowset]
       [PyDict 3 kvp]
         [PyString "header"]
         [PyList 2 items]
           [PyString "corporationID"]
           [PyString "shares"]
         [PyString "RowClass"]
        [PyToken util.Row]
        [PyString "lines"]
        [PyList 2 items]
          [PyList 2 items]
            [PyInt 98038978]
            [PyIntegerVar 250]
          [PyList 2 items]
            [PyInt 1630077495]
            [PyIntegerVar 250]
     */

    return nullptr;
}

PyResult CorpRegistryBound::Handle_GetShareholders(PyCallArgs &call) {
    sLog.White( "CorpRegistryBound::Handle_GetShareholders()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_SetAccountKey(PyCallArgs &call) {

    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    call.client->GetChar()->SetAccountKey(args.arg);
    return nullptr;
}

PyResult CorpRegistryBound::Handle_PayoutDividend(PyCallArgs &call) {
    sLog.White( "CorpRegistryBound::Handle_PayoutDividend()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);
    //self.GetCorpRegistry().PayoutDividend(payShareholders, payoutAmount)

    return nullptr;
}

//21:59:17 L CorpRegistryBound::Handle_GetVoteCasesByCorporation(): size= 3
//21:59:20 L CorpRegistryBound::Handle_GetVoteCasesByCorporation(): size= 1
PyResult CorpRegistryBound::Handle_GetVoteCasesByCorporation(PyCallArgs &call) {
    /*
     2 *2:47:43 L CorpRegistryBound::Handle_GetVoteCasesByCorporation(): size= 3
     22:47:43 [SvcCall]   Call Arguments:
     22:47:43 [SvcCall]       Tuple: 3 elements
     22:47:43 [SvcCall]         [ 0] Integer field: 1001002
     22:47:43 [SvcCall]         [ 1] Integer field: 2
     22:47:43 [SvcCall]         [ 2] Integer field: 0
     */
    sLog.White( "CorpRegistryBound::Handle_GetVoteCasesByCorporation()", "size= %u", call.tuple->size() );

    call.Dump(CORP__CALL_DUMP);
    return nullptr;
}