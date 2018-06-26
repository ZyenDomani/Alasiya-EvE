

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "agents/Agent.h"

class AgentBound
: public PyBoundObject
{
public:

    PyCallable_Make_Dispatcher(AgentBound)

    AgentBound(PyServiceMgr *mgr, CorpAgent *agt);

    virtual ~AgentBound() { delete m_dispatch; }
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(DoAction);
    PyCallable_DECL_CALL(GetMyJournalDetails);
    PyCallable_DECL_CALL(GetAgentLocationWrap);
    PyCallable_DECL_CALL(GetInfoServiceDetails);
    PyCallable_DECL_CALL(GetMissionKeywords);
    PyCallable_DECL_CALL(GetMissionJournalInfo);
    PyCallable_DECL_CALL(GetMissionBriefingInfo);
    PyCallable_DECL_CALL(GetMissionObjectiveInfo);
    PyCallable_DECL_CALL(GetDungeonShipRestrictions);
    PyCallable_DECL_CALL(RemoveOfferFromJournal);
    PyCallable_DECL_CALL(GetOfferJournalInfo);
    PyCallable_DECL_CALL(GetEntryPoint);
    PyCallable_DECL_CALL(GotoLocation);
    PyCallable_DECL_CALL(WarpToLocation);

    protected:
        CorpAgent* m_agent;    //we do not own this.
        Dispatcher* m_dispatch;    //we own this
};
