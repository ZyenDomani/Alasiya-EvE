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
    Updates:        Allan
*/

#include "standing/Standing.h"

/*  re-write of standing system  -allan 10Apr15
 * see notes in StandingDB.cpp
 */

PyCallable_Make_InnerDispatcher(Standing)

Standing::Standing(PyServiceMgr *mgr)
: PyService(mgr, "standing2"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(Standing, GetSecurityRating);
    PyCallable_REG_CALL(Standing, GetMyKillRights);
    PyCallable_REG_CALL(Standing, GetMyStandings);
    PyCallable_REG_CALL(Standing, GetCharStandings);
    PyCallable_REG_CALL(Standing, GetCorpStandings);
    PyCallable_REG_CALL(Standing, GetNPCNPCStandings);
    PyCallable_REG_CALL(Standing, GetStandingTransactions);
    PyCallable_REG_CALL(Standing, GetStandingCompositions);

}

Standing::~Standing() {
    delete m_dispatch;
}

PyResult Standing::Handle_GetSecurityRating(PyCallArgs &call) {
    //takes an integer: characterID
    Call_SingleIntegerArg arg;
    if(!arg.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode args.");
        return NULL;
    }

    CharacterRef c = m_manager->item_factory->GetCharacter( arg.arg );
    if( !c ) {
        _log(SERVICE__ERROR, "Character %u not found.", arg.arg);
        return NULL;
    }

    return new PyFloat( c->GetSecurityRating() );
}

PyResult Standing::Handle_GetMyKillRights(PyCallArgs &call) {
    // self.killRightsCache, self.killedRightsCache = sm.RemoteSvc('standing2').GetMyKillRights()
    // each cache holds k,v where key is toID or fromID
    PyTuple *tu = new PyTuple(2);
    PyDict *u1 = new PyDict();
    PyDict *u2 = new PyDict();
        tu->items[0] = u1;
        tu->items[1] = u2;
    return tu;
}

PyResult Standing::Handle_GetMyStandings(PyCallArgs &call) {
    /* still working on this one (cause i dont completely understand it yet) */
  sLog.Log( "Standing::Handle_GetMyStandings()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALL_DUMP);

    PyRep *charstandings = m_db.GetCharStandings(call.client);
    PyRep *charprime = m_db.PrimeCharStandings(call.client->GetCharacterID());   //prime, as in to set initial values (initialize)
    PyRep *npccharstandings = m_db.GetCharNPCStandings(call.client->GetCharacterID());

    PyDict *corpstandings = new PyDict();
    PyDict *corpprime = new PyDict();
    PyDict *npccorpstandings = new PyDict();

    PyTuple *tu = new PyTuple(6);
        tu->items[0] = charstandings;
        tu->items[1] = charprime;
        tu->items[2] = npccharstandings;
        tu->items[3] = corpstandings;
        tu->items[4] = corpprime;
        tu->items[5] = npccorpstandings;
    PyRep *result = tu;
    return result;
}

PyResult Standing::Handle_GetCharStandings(PyCallArgs &call) {
    ObjectCachedSessionMethodID method_id(GetName(), "GetCharStandings", call.client->GetCharacterID());

    if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
        PyRep *t = m_db.GetCharStandings(call.client);

        m_manager->cache_service->GiveCache(method_id, &t);
    }

    return m_manager->cache_service->MakeObjectCachedSessionMethodCallResult(method_id, "charID");
}

PyResult Standing::Handle_GetCorpStandings(PyCallArgs &call) {
  sLog.Log( "Standing::Handle_GetCorpStandings()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALL_DUMP);

    ObjectCachedSessionMethodID method_id(GetName(), "GetCorpStandings", call.client->GetCorporationID());

    if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
        PyRep *t = m_db.GetCorpStandings(call.client->GetCorporationID());

        m_manager->cache_service->GiveCache(method_id, &t);
    }

    return m_manager->cache_service->MakeObjectCachedSessionMethodCallResult(method_id, "corpID");
}

PyResult Standing::Handle_GetNPCNPCStandings(PyCallArgs &call) {
    // this is NPC<-->NPC standings
    /*
    PyRep *result = NULL;
    ObjectCachedMethodID method_id(GetName(), "GetNPCNPCStandings");

    //check to see if this method is in the cache already.
    if (!m_manager->cache_service->IsCacheLoaded(method_id)) {
        //this method is not in cache yet, load up the contents and cache it.
        result = m_db.GetFactionStandings();
        if (result == NULL)
            result = new PyNone();
        m_manager->cache_service->GiveCache(method_id, &result);
    }

    //now we know its in the cache one way or the other, so build a
    //cached object cached method call result.
    result = m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id);
    */
    return /*result*/m_db.GetFactionStandings();
}

PyResult Standing::Handle_GetStandingTransactions(PyCallArgs &call) {
    /**
     * data = sm.RemoteSvc('standing2').GetStandingTransactions(fromID, toID, direction, eventID, eventType, eventDateTime)
     */
    sLog.Log( "Standing::Handle_GetStandingTransactions()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    Call_GetStandingTransactions args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
        return NULL;
    }

    PyRep *result = m_db.GetStandingTransactions(args.fromID, args.toID, args.direction, args.eventID, args.eventType, args.eventDateTime);
    return result;
}

PyResult Standing::Handle_GetStandingCompositions(PyCallArgs &call) {
/**  no clue what this is yet
                self.sr.data = sm.RemoteSvc('standing2').GetStandingCompositions(fromID, toID)
            if self.sr.data:
                prior = 0.0
                for each in self.sr.data:
                    if each.ownerID == fromID:
                        prior = each.standing
*/
    sLog.Log( "Standing::Handle_GetStandingCompositions()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    Call_GetStandingComposition args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
        return NULL;
    }

    PyRep *result = m_db.GetStandingCompositions(args.toID, args.fromID);
    return result;
}