
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

#include "system/SystemEntity.h"


class Planet
{
public:
    Planet();
    ~Planet()                                           { /* do nothing here */ }

protected:
private:

};


class PyServiceMgr;
class SystemManager;

class PlanetSE
: public StaticSystemEntity
{
public:
    PlanetSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~PlanetSE()                                 { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual PlanetSE* GetPlanetSE()                     { return this; }
    /* class type tests. */
    virtual bool IsPlanetSE()                           { return true; }

    /* virtual functions default to base class and overridden as needed */
    virtual bool LoadExtras(SystemDB *db);
};

#endif  // EVEMU_PLANET_PLANET_H_