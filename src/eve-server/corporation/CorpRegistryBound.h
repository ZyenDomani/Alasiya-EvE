


#ifndef __EVEMU_CORP_CORPREGISTRY_BOUND_H_
#define __EVEMU_CORP_CORPREGISTRY_BOUND_H_


#include "../eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "corporation/CorporationDB.h"

class CorpRegistryBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(CorpRegistryBound)

    CorpRegistryBound(PyServiceMgr *mgr, CorporationDB& db, uint32 corpID);
    virtual ~CorpRegistryBound() { delete m_dispatch; }
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(GetEveOwners);
    PyCallable_DECL_CALL(GetCorporation);
    PyCallable_DECL_CALL(GetCorporations);
    PyCallable_DECL_CALL(GetInfoWindowDataForChar);
    PyCallable_DECL_CALL(GetLockedItemLocations);
    PyCallable_DECL_CALL(AddCorporation);
    PyCallable_DECL_CALL(GetSuggestedTickerNames);
    PyCallable_DECL_CALL(GetOffices);
    PyCallable_DECL_CALL(GetStations);
    PyCallable_DECL_CALL(GetMyApplications);
    PyCallable_DECL_CALL(InsertApplication);
    PyCallable_DECL_CALL(GetApplications);
    PyCallable_DECL_CALL(UpdateApplicationOffer);
    PyCallable_DECL_CALL(DeleteApplication);
    PyCallable_DECL_CALL(UpdateApplication);
    PyCallable_DECL_CALL(UpdateDivisionNames);
    PyCallable_DECL_CALL(UpdateCorporation);
    PyCallable_DECL_CALL(UpdateLogo);
    PyCallable_DECL_CALL(SetAccountKey);
    PyCallable_DECL_CALL(GetMember);
    PyCallable_DECL_CALL(GetMembers);
    PyCallable_DECL_CALL(GetSharesByShareholder);
    PyCallable_DECL_CALL(GetShareholders);
    PyCallable_DECL_CALL(PayoutDividend);
    PyCallable_DECL_CALL(GetVoteCasesByCorporation);


protected:
    bool JoinCorporation(Client *who, uint32 newCorpID, const CorpData &roles);
    static void FillOCApplicationChange(Notify_OnCorporationApplicationChanged & OCAC, const ApplicationInfo & Old, const ApplicationInfo & New);

    CorporationDB& m_db;

    Dispatcher *const m_dispatch;

    uint32 m_corpID;
};

#endif  // __EVEMU_CORP_CORPREGISTRY_BOUND_H_
