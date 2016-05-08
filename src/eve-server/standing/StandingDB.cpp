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


#include "Client.h"
#include "standing/StandingDB.h"

/*  re-write of standing system  -allan 10Apr15
 *  all DB tables have identical field names for ease of implementation
 * Get Fields
 *    toID = me|myCorp|myAlliance
 *    fromID = char|agent|corp|faction|alliance
 *    standing =  fraction representing current value
 * Set Fields
 *    toID = char|agent|corp|faction|alliance
 *    fromID = me|myCorp|myAlliance
 *    standing =  fraction representing current value
 *
 DB tables [fields] (notes) {all table names are entity reputation from *table name* }
 repAgent [fromID, toID, standing] (from agents to characters. changed by missions status')
 repAlliance [fromID, toID, standing] (corporation<-->alliance, alliance<-->alliance - changed thru Corp window)
 repChar [fromID, toID, standing] (character<-->character, character<-->corporation - changed thru PnP window)
 repCorp [fromID, toID, standing] (corporation<-->character, corporation<-->corporation - changed thru Corp window)
 repFactions [fromID, toID, standing] (NPC Faction <--> NPC Faction) {populated, hard-coded - cant change}
 repNPCCorp [fromID, toID, standing] (NPC corps --> characters - changed by missions and faction kills)
 repStandingChanges [fromID, toID, eventID, eventTypeID, eventDateTime, modification, originalFromID, originalToID, int_1, int_2, int_3, msg] ()

 http://www.eveinfo.net/wiki/ind~4067.htm
 */

PyRep *StandingDB::GetFactionStandings() {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT fromID,toID,standing FROM repFactions"  )) {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }
    return DBResultToCRowset(res);
}


PyRep *StandingDB::GetCharStandings(Client *pClient) {
    /*  get faction, corp, agent for this char
     *      will need more work to get char corps/alliances/factions in the db.
     */
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT fromID, standing FROM repFactions WHERE toID = " // should this be warfactionID ??
                            " (SELECT factionID FROM crpNPCCorporations WHERE corporationID = %u)", pClient->GetCorporationID());
    sDatabase.RunQuery(res, "SELECT fromID, standing FROM repNPCCorp WHERE toID = %u", pClient->GetCorporationID());
    sDatabase.RunQuery(res, "SELECT fromID, standing FROM repAgent WHERE toID = %u", pClient->GetCharacterID());

    return DBResultToCRowset(res);
}

PyRep *StandingDB::GetCorpStandings(uint32 corpID) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT fromID, standing FROM repCorp WHERE toID=%u", corpID )) {
        _log(DATABASE__ERROR, "Error in GetCorpStandings query: %s", res.error.c_str());
        return NULL;
    }
    return DBResultToCRowset(res);
}

PyRep *StandingDB::GetCharNPCStandings(uint32 charID) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT toID, standing FROM chrNPCStandings WHERE fromID=%u", charID )) {
        _log(DATABASE__ERROR, "Error in GetCharNPCStandings query: %s", res.error.c_str());
        return NULL;
    }
    return DBResultToCRowset(res);
}


PyRep *StandingDB::PrimeCharStandings(uint32 charID) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT "
        " itemID AS ownerID,"
        " itemName AS ownerName,"
        " typeID"
        " FROM entity"
        " WHERE itemID < 0"
    ))
    {
        _log(DATABASE__ERROR, "Error in PrimeCharStandings query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}


PyRep *StandingDB::GetStandingTransactions(uint32 fromID, uint32 toID, uint32 direction, uint32 eventID, uint32 eventType, uint64 eventDateTime) {
    if (fromID == ownerCONCORD)
        ;

    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT "
        "  eventID,"
        "  eventTypeID,"
        "  eventDateTime,"
        "  fromID,"
        "  toID,"
        "  modification,"
        "  originalFromID,"
        "  originalToID,"
        "  int_1,"
        "  int_2,"
        "  int_3,"
        "  msg"
        " FROM repStandingChanges"
        " WHERE toID = %u"
        "  AND fromID = %u", toID, fromID )) {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }
    return DBResultToRowset(res);
}

PyRep* StandingDB::GetStandingCompositions(uint32 toID, uint32 fromID) {
    // ownerID, standing ...

    return NULL;
}

PyRep *StandingDB::GetSystemSovInfo(uint32 systemID) {
    /**
     *        sovInfo = sm.RemoteSvc('sovMgr').GetSystemSovereigntyInfo(session.solarsystemid2)
     *        ssAllianceID = sovInfo.allianceID if sovInfo else None
     *
     *        [PyTuple 1 items]
     *            [PySubStream 116 bytes]
     *                [PyObjectData Name: util.KeyVal]
     *                    [PyDict 7 kvp]
     *                        [PyString "contested"]            <<<< interger bool
     *                        [PyInt 0]
     *                        [PyString "corporationID"]
     *                        [PyInt 98049918]
     *                        [PyString "claimTime"]
     *                        [PyIntegerVar 129743663400000000]
     *                        [PyString "claimStructureID"]     <<<<  Territorial Claim Unit
     *                        [PyIntegerVar 1005712174146]
     *                        [PyString "hubID"]                <<<<  unknown
     *                        [PyIntegerVar 1005900797500]
     *                        [PyString "allianceID"]
     *                        [PyInt 99000289]
     *                        [PyString "solarSystemID"]
     *                        [PyInt 30000302]
     */
    //FIXME this needs work.....
/*
    DBQueryResult res;
    DBResultRow row;

    PyDict *args = new PyDict;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "   solarSystemID,"
        "   corporationID,"
        "   allianceID,"
        "   claimStructureID,"
        "   claimTime,"
        "   hubID,"
        "   contested"
        " FROM mapSystemSovInfo"
        "  WHERE solarSystemID = %u", systemID )) {
        codelog(SERVICE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    if (!res.GetRow(row)) {
        sLog.Error("StandingDB::GetSystemSovInfo()", "No Data for systemID %u.  Hacking PyDict.", systemID);
        res.Reset();
        sDatabase.RunQuery(res, "SELECT factionID FROM mapSolarSystems WHERE solarSystemID = %u", systemID );
        uint32 factionID = 0;
        if (res.GetRow(row))
            factionID = row.GetUInt(0);
        args->SetItemString( "contested", new PyInt(0));
        args->SetItemString( "corporationID", new PyInt(0));
        args->SetItemString( "claimTime", new PyLong(0));
        args->SetItemString( "claimStructureID", new PyInt(0));
        args->SetItemString( "hubID", new PyInt(0));
        args->SetItemString( "allianceID", new PyInt(factionID));
        args->SetItemString( "solarSystemID", new PyInt(systemID));
    } else {
        args->SetItemString( "contested", new PyInt(row.GetUInt(6)));
        args->SetItemString( "corporationID", new PyInt(row.GetUInt(1)));
        args->SetItemString( "claimTime", new PyLong(row.GetUInt64(4)));
        args->SetItemString( "claimStructureID", new PyInt(row.GetUInt(3)));
        args->SetItemString( "hubID", new PyInt(row.GetUInt(5)));
        args->SetItemString( "allianceID", new PyInt(row.GetUInt(2)));
        args->SetItemString( "solarSystemID", new PyInt(row.GetUInt(0)));
    }

    return new PyObject("util.KeyVal", args);
*/
return new PyNone;
}


double StandingDB::GetAgentStanding(uint32 toID, uint32 fromID) {
    DBQueryResult res;
    DBResultRow row;
    sDatabase.RunQuery(res, "SELECT fromID, standing FROM repAgent WHERE toID=%u AND fromID=%u", toID, fromID);
    if (res.GetRow(row))
        return row.GetDouble(0);
    else
        return 0.0f;
}

double StandingDB::GetAllianceStanding(uint32 toID, uint32 fromID) {
    DBQueryResult res;
    DBResultRow row;
    sDatabase.RunQuery(res, "SELECT standing FROM repAlliance WHERE toID=%u AND fromID=%u", toID, fromID);
    if (res.GetRow(row))
        return row.GetDouble(0);
    else
        return 0.0f;
}

double StandingDB::GetCharStanding(uint32 toID, uint32 fromID) {
    DBQueryResult res;
    DBResultRow row;
    sDatabase.RunQuery(res, "SELECT fromID, standing FROM repChar WHERE toID=%u AND fromID=%u", toID, fromID);
    if (res.GetRow(row))
        return row.GetDouble(0);
    else
        return 0.0f;
}

double StandingDB::GetCorpStanding(uint32 toID, uint32 fromID) {
    DBQueryResult res;
    DBResultRow row;
    sDatabase.RunQuery(res, "SELECT fromID, standing FROM repCorp WHERE toID=%u AND fromID=%u", toID, fromID);
    if (res.GetRow(row))
        return row.GetDouble(0);
    else
        return 0.0f;
}

double StandingDB::GetNPCCorpStanding(uint32 toID, uint32 fromID) {
    DBQueryResult res;
    DBResultRow row;
    sDatabase.RunQuery(res, "SELECT fromID, standing FROM repNPCCorp WHERE toID=%u AND fromID=%u", toID, fromID);
    if (res.GetRow(row))
        return row.GetDouble(0);
    else
        return 0.0f;
}

double StandingDB::GetFactionStanding(uint32 toID, uint32 fromID) {
    DBQueryResult res;
    DBResultRow row;
    sDatabase.RunQuery(res, "SELECT fromID, standing FROM repFactions WHERE toID=%u AND fromID=%u", toID, fromID);
    if (res.GetRow(row))
        return row.GetDouble(0);
    else
        return 0.0f;
}

double StandingDB::GetStandingChanges(uint32 charID) {
    return 0.0;
}

void StandingDB::SetAgentStanding(uint32 toID, uint32 fromID, double standing) {
    DBerror err;
    sDatabase.RunQuery(err,
                       "INSERT INTO `repAgent`(`toID`, `fromID`, `standing`) "
                       "VALUES (%u,%u,%f)", toID, fromID, standing );
}

void StandingDB::SetAllianceStanding(uint32 toID, uint32 fromID, double standing) {
    DBerror err;
    sDatabase.RunQuery(err,
                       "INSERT INTO `repAlliance`(`toID`, `fromID`, `standing`) "
                       "VALUES (%u,%u,%f)", toID, fromID, standing );
}

void StandingDB::SetCharStanding(uint32 toID, uint32 fromID, double standing) {
    DBerror err;
    sDatabase.RunQuery(err,
                       "INSERT INTO `repChar`(`toID`, `fromID`, `standing`) "
                       "VALUES (%u,%u,%f)", toID, fromID, standing );
}

void StandingDB::SetCorpStanding(uint32 toID, uint32 fromID, double standing) {
    DBerror err;
    sDatabase.RunQuery(err,
                       "INSERT INTO `repCorp`(`toID`, `fromID`, `standing`) "
                       "VALUES (%u,%u,%f)", toID, fromID, standing );
}

void StandingDB::SetNPCCorpStanding(uint32 toID, uint32 fromID, double standing) {
    DBerror err;
    sDatabase.RunQuery(err,
                       "INSERT INTO `repNPCCorp`(`toID`, `fromID`, `standing`) "
                       "VALUES (%u,%u,%f)", toID, fromID, standing );
}

void StandingDB::SaveStandingChanges(uint32 fromID, uint32 toID, uint32 direction, uint32 eventType, double amount, std::string msg) {
    DBQueryResult res;
    sDatabase.RunQuery(res,
        "INSERT INTO repStandingChanges"
        "  ( eventType,"
        "  eventDateTime,"
        "  fromID,"
        "  toID,"
        "  modification,"
        "  direction,"
        "  msg )"
        " VALUES (%u, %" PRIu64 ", %u, %u, %f, %u, '%s' )",
                eventType, Win32TimeNow(), fromID, toID, amount, direction, msg.c_str() );
}

//FIXME TODO  implement repStandingChanges after standing system is working....
