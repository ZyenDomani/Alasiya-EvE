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

/*
 * STANDING__ERROR
 * STANDING__WARNING
 * STANDING__MESSAGE
 * STANDING__DEBUG
 * STANDING__INFO
 * STANDING__TRACE
 * STANDING__DUMP
 * STANDING__RSPDUMP
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
    return sStandingMgr.GetFactionStandings();
}

/** @todo  need to add a standing from corpCONCORD to any/all charID, corpID, allyID  for security rating (as seen in client code) */

PyResult Standing::Handle_GetSecurityRating(PyCallArgs &call) {
    //takes an integer: characterID
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    CharacterRef cRef = sItemFactory.GetCharacter( arg.arg );
    if  (cRef.get() == nullptr) {
        _log(STANDING__WARNING, "Character %u not found.", arg.arg);
        return nullptr;
    }

    return new PyFloat( cRef->GetSecurityRating() );
}

PyResult Standing::Handle_GetMyKillRights(PyCallArgs &call) {
    // self.killRightsCache, self.killedRightsCache = sm.RemoteSvc('standing2').GetMyKillRights()
    // each cache holds k,v where key is toID or fromID
    _log(STANDING__MESSAGE,  "Standing::Handle_GetMyKillRights()");
    PyTuple* KillRights = new PyTuple(2);
    PyDict* killRightsCache = new PyDict();
    PyDict* killedRightsCache = new PyDict();
        KillRights->items[0] = killRightsCache;
        KillRights->items[1] = killedRightsCache;

    if (is_log_enabled(STANDING__RSPDUMP)) {
        _log(STANDING__RSPDUMP, "Standing::Handle_GetMyKillRights() RSP:" );
        KillRights->Dump(STANDING__RSPDUMP, "    ");
    }

    return KillRights;
}

// cannot find a call to this one
PyResult Standing::Handle_GetMyStandings(PyCallArgs &call) {
    /* still working on this one (cause i dont completely understand it yet) */
    _log(STANDING__MESSAGE,  "Standing::Handle_GetMyStandings()");
  call.Dump(STANDING__DUMP);

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
    _log(STANDING__MESSAGE,  "Standing::Handle_GetStandingTransactions()");
    call.Dump(STANDING__DUMP);

    Call_GetStandingTransactions args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    /*
     * 22:45:01 [SvcCall] Service standing2::GetStandingTransactions()
     * ****  didnt have dump activated here....
     *
     * EXCEPTION #19 logged at  11/18/2018 22:45:01
     * Caught at:
     * /common/lib/bluepy.py(86) CallWrapper
     * /client/script/ui/control/entries.py(2279) OnClick
     * /../carbon/client/script/ui/control/scroll.py(518) SelectNode
     * /../carbon/client/script/ui/control/scroll.py(522) ReportSelectionChange
     * /client/script/ui/shared/neocom/charactersheet.py(448) OnSelectEntry
     * /client/script/ui/shared/neocom/charactersheet.py(726) Load
     * /client/script/ui/shared/neocom/charactersheet.py(995) ShowSecurityStatus
     * /common/script/util/eveformat.py(590) FmtStandingTransaction
     * Thrown at:
     * /common/script/util/eveformat.py(481) FmtStandingTransaction
     * /../carbon/common/script/sys/cfg.py(1220) Get
     * /../carbon/common/script/sys/cfg.py(1090) Prime
     * /../carbon/common/script/sys/cfg.py(1158) _Prime
     *        localKeysToGet = set()
     *        conf = <RemoteService: config>
     *        self = <Instance of Recordset.EveOwners>
     *                       Key column: ownerID, Cache entries: 11413
     *                       Field names: ownerID, ownerName, typeID, gender, ownerNameID
     *        keysIAlreadyHave = set()
     *        key = 0
     *        fk = ()
     *        keysToGet = set([0])
     *        fetch = <function callable at 0x52711570>
     * ValueError: need more than 0 values to unpack
     */

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
_log(STANDING__MESSAGE,  "Standing::Handle_GetStandingCompositions()");
    call.Dump(STANDING__DUMP);

    Call_GetStandingComposition args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    return m_db.GetStandingCompositions(args.toID, args.fromID);
}