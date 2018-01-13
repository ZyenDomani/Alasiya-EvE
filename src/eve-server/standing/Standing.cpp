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

PyResult Standing::Handle_GetCharStandings(PyCallArgs &call)
{
    return m_db.GetCharStandings(call.client);
}

PyResult Standing::Handle_GetCorpStandings(PyCallArgs &call)
{
    return m_db.GetCorpStandings(call.client);
}

PyResult Standing::Handle_GetNPCNPCStandings(PyCallArgs &call)
{
    return m_db.GetFactionStandings();
}

/** @todo  need to add a standing from ownerCONCORD to any/all charID, corpID, allyID  for security rating (as seen in client code) */

PyResult Standing::Handle_GetSecurityRating(PyCallArgs &call) {
    //takes an integer: characterID
    Call_SingleIntegerArg arg;
    if(!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return NULL;
    }

    CharacterRef c = sItemFactory.GetCharacter( arg.arg );
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

// cannot find a call to this one
PyResult Standing::Handle_GetMyStandings(PyCallArgs &call) {
    /* still working on this one (cause i dont completely understand it yet) */
  sLog.White( "Standing::Handle_GetMyStandings()", "size= %u", call.tuple->size() );
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

PyResult Standing::Handle_GetStandingTransactions(PyCallArgs &call) {
    /**
     * data = sm.RemoteSvc('standing2').GetStandingTransactions(fromID, toID, direction, eventID, eventType, eventDateTime)
     */
    sLog.White( "Standing::Handle_GetStandingTransactions()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    Call_GetStandingTransactions args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return NULL;
    }

    return m_db.GetStandingTransactions(args.fromID, args.toID, args.direction, args.eventID, args.eventType, args.eventDateTime);
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
    sLog.White( "Standing::Handle_GetStandingCompositions()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    Call_GetStandingComposition args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return NULL;
    }

    return m_db.GetStandingCompositions(args.toID, args.fromID);
}