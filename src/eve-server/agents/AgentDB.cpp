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
    Updates:    Allan
*/

/** @todo  fix this....not all agents have an entry in chrNPCCharacters table.  */

#include "eve-server.h"

#include "agents/AgentDB.h"

PyObjectEx *AgentDB::GetAgents() {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "    agt.agentID,"
        "    agt.agentTypeID,"
        "    agt.divisionID,"
        "    agt.level,"
        "    agt.quality,"
        "    agt.corporationID,"
        "    chr.stationID,"
        "    chr.gender,"
        "    bl.bloodlineID"
        " FROM agtAgents AS agt"
        " LEFT JOIN chrNPCCharacters AS chr ON chr.characterID = agt.agentID"
        " LEFT JOIN bloodlineTypes AS bl ON bl.bloodlineID = agt.agentTypeID"
    ))
    {
        codelog(DATABASE__ERROR, "Error in GetAgents query: %s", res.error.c_str());
        return NULL;
    }

    _log(DATABASE__RESULTS, "GetAgents returned %u items", res.GetRowCount());
    return DBResultToCRowset(res);
}

bool AgentDB::LoadAgentLocation(uint32 agentID, uint32 &locationID, uint32 &locationType) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   agt.locationID, "
        "   itm.typeID "
        " FROM agtAgents AS agt"
        " LEFT JOIN mapDenormalize AS itm ON itm.itemID = agt.locationID"
        " WHERE agt.agentID=%u", agentID))
    {
        codelog(DATABASE__ERROR, "Error in LoadAgentLocation query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row))
        return false;
    locationID = row.GetUInt(0);
    locationType = row.GetUInt(1);
    return true;
}

PyRep *AgentDB::GetAgentSolarSystem(uint32 AgentID){
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        " SELECT"
        "  s.solarSystemID"
        " FROM agtAgents AS a"
        "  LEFT JOIN staStations AS s ON s.stationID = a.locationID"
        " WHERE a.agentID = %u",AgentID)) {
        codelog(DATABASE__ERROR, "Error in GetAgentSolarSystem query for Agent = %u", AgentID);
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "SystemID of Agent %u not found.", AgentID);
        return NULL;
    }

    PyRep *result(nullptr);
    PyTuple *t = new PyTuple(1);
    t->items[0] = new PyList(row.GetUInt(0));
    result = t;
    return result;
}

#ifdef NOT_DONE
AgentLevel *AgentDB::LoadAgentLevel(uint8 level)
{
    AgentLevel *result = new AgentLevel;

    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "   m.missionID,m.missionName,m.missionLevel,"
        "   m.missionTypeID,t.missionTypeName,"
        "   m.importantMission"
        " FROM agtMissions AS m"
        "  NATURAL JOIN agtMissionTypes AS t"
        " WHERE missionLevel=%d",
        level ))
    {
        codelog(DATABASE__ERROR, "Error in LoadAgentLevel query: %s", res.error.c_str());
        delete result;
        return nullptr;
    }

    _log(DATABASE__RESULTS, "LoadAgentLevel returned %u items", res.GetRowCount());

    std::list<uint32> IDs;
    IDs.clear();
    DBResultRow row;

    while(res.GetRow(row)) {
        AgentMissionSpec *spec = new AgentMissionSpec;
        spec->missionID = row.GetUInt(0);
        spec->missionName = row.GetText(1);
        spec->missionLevel = row.GetUInt(2);
        spec->missionTypeID = row.GetUInt(3);
        spec->missionTypeName = row.GetText(4);
        spec->importantMission = (row.GetUInt(6)==0)?false:true;
        result->missions.push_back(spec);
    }
}

#endif
