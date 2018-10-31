
 /**
  * @name MissionDataMgr.h
  *   memory object caching system for managing and saving ingame data specific to missions
  *
  * @Author:        Allan
  * @date:      24 June 2018
  *
  */


#ifndef _EVE_SERVER_MISSION_DATAMANAGER_H__
#define _EVE_SERVER_MISSION_DATAMANAGER_H__


#include "../eve-server.h"
#include "missions/MissionDB.h"


class MissionDataMgr
: public Singleton< MissionDataMgr >
{
public:
    MissionDataMgr();
    ~MissionDataMgr();

    int                 Initialize();

    void                Clear();
    void                Close()                         { Clear(); }
    void                GetInfo();

    void                LoadMissionOffers(uint32 charID, std::vector<MissionOffer>& data);
    void                CreateMissionOffer(uint8 typeID, uint8 level, MissionOffer& data);

    void                UpdateMissionData();

    std::string         GetTypeName(uint8 typeID);

protected:
    void                Populate();

private:
    std::map<std::string, uint32> m_names;
    std::multimap<uint8, CourierData> m_courier;    // level/data
    std::multimap<uint8, CourierData> m_mining;     // level/data
    std::multimap<uint8, MissionData> m_missions;   // level/data
    std::multimap<uint32, MissionOffer> m_offers;   // charID/data
    std::multimap<uint32, MissionOffer> m_xoffers;   // charID/data
    std::multimap<uint32, MissionOffer> m_aoffers;   // agentID/data

};

//Singleton
#define sMissionDataMgr \
( MissionDataMgr::get() )


#endif  // _EVE_SERVER_MISSION_DATAMANAGER_H__
