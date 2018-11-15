
/**
 * @name MapData.h
 *   a group of methods and functions to get map info.
 *     this is mostly used for getting random points in system, system jumps, and misc mission destination info
 * @Author:         Allan
 * @date:   13 November 2018
 */


#ifndef _EVE_MAP_MAPDATA_H_
#define _EVE_MAP_MAPDATA_H_

#include "../eve-server.h"

#include "../../eve-common/EVE_Missions.h"

class Agent;

class MapData
: public Singleton< MapData >
{
public:
    MapData();
    ~MapData();

    int                 Initialize();

    void                Clear();
    void                Close()                         { Clear(); }

    void                GetInfo();


    void GetMissionDestination(Agent* pAgent, uint8 misionType, MissionOffer& offer);

protected:
    void                Populate();

private:

    std::multimap<uint32, uint32>        m_regionJumps;  //fromSys/toSys
    std::multimap<uint32, uint32>        m_constJumps;   //fromSys/toSys
    std::multimap<uint32, uint32>        m_systemJumps;  //fromSys/toSys

};

//Singleton
#define sMapData \
( MapData::get() )


#endif  // _EVE_MAP_MAPDATA_H_
