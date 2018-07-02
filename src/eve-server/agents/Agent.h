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
#ifndef __AGENT_H_INCL__
#define __AGENT_H_INCL__

#include "../../eve-common/EVE_Agent.h"
#include "agents/AgentDB.h"

class Client;

class CorpAgent {
public:
    CorpAgent(uint32 id);
    ~CorpAgent()                                        { /* do nothing here */ }

    bool Load(AgentDB *from);
    uint32 GetID() { return m_agentID; }

    uint32 GetLoyaltyPoints(Client *who);
    void DoAction(Client *who, uint32 actionID, std::string &say, std::map<uint32, std::string> &choices);


    PyDict* GetLocationWrap();
    PyObject* GetInfoServiceDetails();


    uint32 GetSystemID()        { return m_data.solarSystemID; }
    uint32 GetStationID()       { return m_data.stationID; }

    void SetMission(bool set=false)         { m_mission = set; }
    bool HasMission()                       { return m_mission; }

protected:
    const uint32 m_agentID;
    AgentData m_data;

    bool m_mission;
};

#endif