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


void AgentDB::LoadAgentData(uint32 agentID, AgentData& data)
{
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT"
        "   agt.agentTypeID,"
        "   agt.divisionID,"
        "   agt.level,"
        "   agt.quality,"
        "   agt.corporationID,"
        "   agt.locationID, "   //5
        "   agt.isLocator,"
        "   chr.solarSystemID,"
        "   chr.stationID,"
        "   chr.gender,"
        "   bl.bloodlineID,"    //10
        "   itm.typeID "
        " FROM agtAgents AS agt"
        " LEFT JOIN chrNPCCharacters AS chr ON chr.characterID = agt.agentID"
        " LEFT JOIN bloodlineTypes AS bl ON bl.typeID = chr.typeID"
        " LEFT JOIN mapDenormalize AS itm ON itm.itemID = agt.locationID"
        " WHERE agt.agentID = %u", agentID))
    {
        codelog(DATABASE__ERROR, "Error in GetAgents query: %s", res.error.c_str());
        return;
    }

    /** @todo  there may be some errors here with agents in space or NOT in stations....will have to test and fix as they come up.  */
    DBResultRow row;
    if (res.GetRow(row)) {
        if (row.GetUInt(10) == 0) {
            _log(DATABASE__MESSAGE, "No charTypeID for Agent %u", agentID);
            return;
        }
        data.typeID         = row.GetUInt(0);
        data.divisionID     = row.GetUInt(1);
        data.level          = row.GetUInt(2);
        data.quality        = row.GetInt(3);
        data.corporationID  = row.GetUInt(4);
        data.locationID     = row.GetUInt(5);
        data.locator        = row.GetBool(6);
        data.solarSystemID  = row.GetUInt(7);
        data.stationID      = row.GetUInt(8);
        data.gender         = row.GetBool(9);
        data.bloodlineID    = row.GetUInt(10);
        data.locationTypeID = row.GetUInt(11);
    }
}


// SELECT `agentID`, `typeID`, `level` FROM `agtSkillLevel` WHERE

