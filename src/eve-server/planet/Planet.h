
 /**
  * @name Planet.h
  *   Specific Class for individual planets.
  * this class will hold all planet data and relative info for each planet.
  *
  * @Author:         Allan
  * @date:   30 April 2016
  */


#ifndef EVEMU_PLANET_PLANET_H_
#define EVEMU_PLANET_PLANET_H_

#include <unordered_map>
#include "system/SystemEntity.h"

class PlanetDB;

// this class is a singleton object to have a common place for all planet data
class PlanetDataMgr
: public Singleton< PlanetDataMgr >
{
public:
    PlanetDataMgr();
    virtual ~PlanetDataMgr() { /* nothing do to yet */ }

    // Initializes the Table:
    int Initialize();

    void GetPlanetData(uint32 planetID, std::vector<uint32> &typeIDs);

protected:
    void _Populate();


private:
    PlanetDB* m_db;

    std::unordered_multimap<uint32, uint32> m_planetData;
};

#define sPlanetDataMgr \
( PlanetDataMgr::get() )


class Planet
{
public:
    Planet();
    ~Planet()                                           { /* do nothing here */ }

protected:
private:

};


class Colony;
class PyServiceMgr;
class SystemManager;
class Call_ResourceDataDict;

class PlanetSE
: public StaticSystemEntity
{
public:
    PlanetSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~PlanetSE();

    /* class type pointer querys. */
    virtual PlanetSE* GetPlanetSE()                     { return this; }
    /* class type tests. */
    virtual bool IsPlanetSE()                           { return true; }

    /* virtual functions default to base class and overridden as needed */
    virtual bool LoadExtras(SystemDB *db);

    /* specific functions for this class */
    PyRep* GetPlanetInfo(Colony* pColony);
    PyRep* GetResourceData(Call_ResourceDataDict& dict);
    PyRep* GetPlanetResourceInfo();
    PyRep* GetExtractorsForPlanet(int32 planetID);


protected:
    PlanetResourceData m_data;

    bool m_hasColony = false;

};

#endif  // EVEMU_PLANET_PLANET_H_