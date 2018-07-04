
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


#include "eve-server.h"
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


    void    GetMissionNameIDs();

protected:
    void                Populate();

    std::map<std::string, uint32> m_names;
    std::multimap<uint8, CourierData> m_courier;        // level/data
    std::multimap<uint8, CourierData> m_mining;        // level/data
    std::multimap<uint32, MissionOffer> m_offers;       // charID/data


};

//Singleton
#define sMissionDataMgr \
( MissionDataMgr::get() )


#endif  // _EVE_SERVER_MISSION_DATAMANAGER_H__
