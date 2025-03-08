
 /**
  * @name StandingMgr.cpp
  *   memory object caching system for managing and saving standings data
  *   methods and functions relating to manipulation of standings
  *
  * @Author:        Allan
  * @date:      14 Novemeber 2018
  *
  */


#ifndef EVE_STANDING_STANDINGMGR_H
#define EVE_STANDING_STANDINGMGR_H

#include "../eve-server.h"

#include "../../eve-common/EVE_Standings.h"

#include "standing/StandingDB.h"

class Agent;

class StandingMgr
: public Singleton< StandingMgr >
{
public:
    StandingMgr();
    ~StandingMgr()                                      { /* do nothing here */ }

    int                 Initialize();

    void                Clear();
    void                Close()                         { Clear(); }
    void                GetInfo();

    PyObjectEx*         GetFactionStandings()           { PyIncRef(m_factionStandings); return m_factionStandings; }

    // this will calculate corp or faction standings to character
    float               GetEffectiveStanding(uint32 fromID, Character* pChar);    // to be used by all corp/faction checks
     // this is raw standings
    float               GetRawStanding(uint32 fromID, uint32 toID);  //  fromID = char|agent|corp|faction|alliance;   toID = me|myCorp|myAlliance.

    void                SetStanding(uint32 fromID, uint32 toID, float value);
    // this is only from agents, used to update standings from missions.  covers agent, corp, faction, fleet sharing, derived
    void                UpdateStandings(Character* pChar, Agent* pAgent, uint8 eventID, std::string missionName, bool important=false);
    // this is called to update any standings except directly from missions  (missions will call this after calculations)
    void                UpdateStandings(uint32 fromID, uint32 toID, uint16 eventType, float pctChange, std::string msg);
    // as stated
    void                UpdateDerivedStandings( uint32 fromID, uint32 toID, uint16 eventType, float pctChange, std::string msg );

protected:
    void                Populate();

private:
    PyObjectEx*         m_factionStandings;

    // load all standing data here for quick access
    typedef std::map<uint32, float>        standingData;
    std::map<uint32, standingData>         m_standings;         // fromID/data[toID, standing]
    std::map<uint32, standingData>         m_standingsRevered;  // toID/data[fromID, standing]

};


//Singleton
#define sStandingMgr \
( StandingMgr::get() )



#endif  // EVE_STANDING_STANDINGMGR_H