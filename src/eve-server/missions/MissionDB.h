
 /**
  * @name MissionDB.cpp
  *   memory object caching system for managing and saving ingame data specific to missions
  *
  * @Author:        Allan
  * @date:      24 June 2018
  *
  */


#ifndef _EVE_SERVER_MISSION_DATABASE_H__
#define _EVE_SERVER_MISSION_DATABASE_H__

#include "../ServiceDB.h"
#include "../eve-server.h"
#include "../../eve-common/EVE_Missions.h"

class MissionDB
{
public:
    MissionDB();
    ~MissionDB()                                        { /* do nothing here */ }

    static void LoadMissionData(DBQueryResult& res);
    static void GetMissionData(uint16 missionID);

    static void LoadCourierData(DBQueryResult& res);
    static void CreateOfferID(MissionOffer& data);
    static void LoadMissionOffers(DBQueryResult& res);

    void                GetInfo();

protected:

};



#endif  // _EVE_SERVER_MISSION_DATABASE_H__