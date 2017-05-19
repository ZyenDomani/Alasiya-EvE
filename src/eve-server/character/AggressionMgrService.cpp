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
    Author:        caytchen
*/

#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "character/AggressionMgrService.h"

PyCallable_Make_InnerDispatcher(AggressionMgrService)

class AggressionMgrBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(AggressionMgrBound)

    AggressionMgrBound(PyServiceMgr* mgr)
    : PyBoundObject(mgr), m_dispatch(new Dispatcher(this))
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "AggressionMgrBound";

        PyCallable_REG_CALL(AggressionMgrBound, GetCriminalTimeStamps);
        PyCallable_REG_CALL(AggressionMgrBound, CheckLootRightExceptions);
    }

    virtual ~AggressionMgrBound()
    {
        delete m_dispatch;
    }

    virtual void Release()
    {
        // this is not recommended
        delete this;
    }

protected:
    PyCallable_DECL_CALL(GetCriminalTimeStamps);
    PyCallable_DECL_CALL(CheckLootRightExceptions);

    Dispatcher *const m_dispatch;
};

PyResult AggressionMgrBound::Handle_GetCriminalTimeStamps(PyCallArgs &call)
{
    // arguments: charID
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple))
    {
        codelog(SERVICE__ERROR, "%s: failed to decode arguments", call.client->GetName());
        return NULL;
    }

    return new PyDict();
}

PyResult AggressionMgrBound::Handle_CheckLootRightExceptions(PyCallArgs &call)
{
    // arguments: containerID
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple))
    {
        codelog(SERVICE__ERROR, "%s: failed to decode arguments", call.client->GetName());
        return NULL;
    }

    // return true to allow looting
    return new PyBool(true);
}

AggressionMgrService::AggressionMgrService(PyServiceMgr *mgr)
: PyService(mgr, "aggressionMgr"), m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);
}

AggressionMgrService::~AggressionMgrService()
{
    delete m_dispatch;
}

PyBoundObject *AggressionMgrService::_CreateBoundObject(Client *c, const PyRep *bind_args)
{
    _log(CLIENT__MESSAGE, "AggressionMgrService bind request for:");
    bind_args->Dump(CLIENT__MESSAGE, "    ");
    /*
     * 18:26:21 [ClientMessage] AggressionMgrService bind request for:
     * 18:26:21 [ClientMessage]     Integer field: 30002547
     */

    return(new AggressionMgrBound(m_manager));
}

/**  @todo  this is formula for hits...
void SystemEntity::AwardSecurityStatus(InventoryItemRef m_self, Character* pChar) {
    if (pChar->GetSecurityRating() > 5) return;     // npc's dont give secAward above +5.0
    if (m_self->GetAttribute(AttrEntitySecurityStatusKillBonus) == 0) return;
    //  TODO  this needs tweaking...
    //New Status = ((10 - Old Status) * Sec Incr) + Old Status
    double killBonus = m_self->GetAttribute(AttrEntitySecurityStatusKillBonus).get_float();
    killBonus /= 100;
    double oldSec = pChar->GetSecurityRating();
    double secAward = (((10 -oldSec) *killBonus) +oldSec) /1000;
    secAward *=  (1 + ( 0.05 * (pChar->GetSkillLevel(skillFastTalk, true))));      // 5% increase
    secAward *= sConfig.rates.secRate;
    if (secAward) {
        sLog.Magenta("SystemEntity::AwardSecurityStatus()"," %s(%u): killBonus: %f.  oldSec: %f.  secAward: %f.",
                     GetName(), GetID(), killBonus, oldSec, secAward);
        //pChar->secStatusChange( secAward );
        //std::string msg = "Status Change for killing pirates in ";
        //msg += System()->GetName();
        //pChar->SaveStandingChanges( m_self->itemID(),  pChar->itemID(), 1,  eventID,  eventType, secAward, msg);
    }
} */