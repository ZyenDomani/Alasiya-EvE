
 /**
  * @name SystemGPoint.cpp
  *   a group of methods and functions to get random points in a given solarsystem
  * @Author:         Allan
  * @date:   31 July 2014
  */


//work in progress

#include "eve-server.h"

#include "system/SystemGPoint.h"

/**
 *   the pupose of this class is to have a common location with methods used to define
 *      random points in solar systems based on planet and moon positions.
 *   GetRandPointOnPlanet() will query solar systems for planets, pick a random planet,
 *      and define a coordnate within that planet's bubble.
 *   GetRandPointOnMoon() does same as above, but using moons.
 *   Get2RandPlanets() will pick a random point between 2 planets, for warp-out/warp-in
 *      and other things as we see fit.
 *   Get3RandPlanets() does same as above, but using 3 planets.
 *   GetRandPointInSystem() will define a random point within a given system.
 *
 *   this class of methods should be used for positioning mission space, cosmic signatures,
 *      anomolies, complexes, and other things needing a random position in a given system.
 *
 *   these methods will also be used for random npc spawns and their warping/movement.
 *
 *   class DBGPointEntity has index, itemID, radius, and position, and is found in SystemDB.
 *      see copy of class decelaration below
 *
 *  NOTE i remember reading *somewhere* that ALL COSMIC SPAWNS are within *some distance* (4au?) from planets.
 *       ....cant find that info now.  -allan 31Jul14
 */


/// int64 MakeRandomInt( int64 low, int64 high )
/// double MakeRandomFloat( double low, double high )

/*   copied from system/SystemDB.cpp
 * namespace SystemDB:
 * class DBGPointEntity {
 *  public:
 *	   uint8 idx;
 *    uint32 itemID;
 *    GPoint position;
 *    double radius;
 *    double x;
 *    double y;
 *    double z;
 * };
 *
*/

void SystemGPoint::GetPlanets(uint32 systemID) {
    uint8 total;
    std::vector<DBGPointEntity> planetIDs;

    m_db.GetPlanets(systemID, &planetIDs, &total);
}

void SystemGPoint::GetMoons(uint32 systemID) {
    uint8 total;
    std::vector<DBGPointEntity> moonIDs;

    m_db.GetMoons(systemID, &moonIDs, &total);
}

const GPoint SystemGPoint::GetRandPointOnPlanet(uint32 systemID) {
    uint8 total;
    std::vector<DBGPointEntity> planetIDs;

    m_db.GetPlanets(systemID, &planetIDs, &total);

    uint16 i = MakeRandomInt(1, total);
    return (planetIDs[i].position +planetIDs[i].radius +50000);

}

const GPoint SystemGPoint::GetRandPointOnMoon(uint32 systemID) {
    uint8 total;
    std::vector<DBGPointEntity> moonIDs;

    m_db.GetMoons(systemID, &moonIDs, &total);

    uint16 i = MakeRandomInt(1, total);
    return (moonIDs[i].position +moonIDs[i].radius +10000);

}

uint32 SystemGPoint::GetRandPlanet(uint32 systemID) {
	uint8 total;
    std::vector<DBGPointEntity> planetIDs;

    m_db.GetPlanets(systemID, &planetIDs, &total);

    uint16 i = MakeRandomInt(1, total);
    return (planetIDs[i].itemID);
}

const GPoint SystemGPoint::Get2RandPlanets(uint32 systemID) {
    uint8 total;
    std::vector<DBGPointEntity> planetIDs;

    m_db.GetPlanets(systemID, &planetIDs, &total);

}

const GPoint SystemGPoint::Get3RandPlanets(uint32 systemID) {
    uint8 total;
    std::vector<DBGPointEntity> planetIDs;

    m_db.GetPlanets(systemID, &planetIDs, &total);

}

uint32 SystemGPoint::GetRandMoon(uint32 systemID) {
	uint8 total;
    std::vector<DBGPointEntity> moonIDs;

    m_db.GetMoons(systemID, &moonIDs, &total);

    uint16 i = MakeRandomInt(1, total);
    return (moonIDs[i].itemID);
}

const GPoint SystemGPoint::GetRandPointInSystem(uint32 systemID, uint64 distance) {
    // get system max diameter, verify distance is within system.

}
