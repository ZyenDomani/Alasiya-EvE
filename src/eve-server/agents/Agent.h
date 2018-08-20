
 /**
  * @name Agent.h
  *   agent specific code
  *    original agent code by zhur, this was completely rewritten based on new data.
  *
  * @Author:        Allan
  * @date:      19 June 2018
  *
  */

#ifndef _EVE_SERVER_AGENT_H
#define _EVE_SERVER_AGENT_H

#include "../../eve-common/EVE_Agent.h"
#include "agents/AgentDB.h"

class Client;

class Agent {
public:
    Agent(uint32 id);
    ~Agent()                                        { /* do nothing here */ }

    bool Load();
    uint32 GetID() { return m_agentID; }

    uint32 GetLoyaltyPoints(Client *who);
    void DoAction(Client *who, uint32 actionID, std::string &say, std::map<uint32, std::string> &choices);


    PyDict* GetLocationWrap();
    PyObject* GetInfoServiceDetails();


    uint32 GetSystemID()        { return m_data.solarSystemID; }
    uint32 GetStationID()       { return m_data.stationID; }

    void SetMission(bool set=false)         { m_mission = set; }
    bool HasMission()                       { return m_mission; }
    bool IsLocator()                        { return m_data.locator; }
    bool IsResearch()                       { return m_data.research; }

protected:
    const uint32 m_agentID;
    AgentData m_data;

    /** @todo  this will need to be a map of char/bool for existing mission status  */
    // not sure if each agent has separate bind for diff chars or not.
    bool m_mission;
};

#endif  // _EVE_SERVER_AGENT_H