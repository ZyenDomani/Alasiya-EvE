
 /**
  * @name Moon.h
  *   Specific Class for individual moons.
  * this class will hold all moon data and relative info for each moon.
  *
  * @Author:         Allan
  * @date:   30 April 2016
  */


#ifndef EVEMU_PLANET_MOON_H_
#define EVEMU_PLANET_MOON_H_

#include "system/SystemEntity.h"

class PyServiceMgr;
class SystemManager;
class Moon;

class MoonSE
: public StaticSystemEntity
{
public:
    MoonSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~MoonSE()                                   { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual MoonSE* GetMoonSE()                         { return this; }
    /* class type tests. */
    virtual bool IsMoonSE()                             { return true; }

    /* virtual functions default to base class and overridden as needed */
    virtual bool LoadExtras(SystemDB *db);
};


class Moon
{
public:
    Moon();
    ~Moon()       { /* do nothing here */ }

};


#endif  // EVEMU_PLANET_MOON_H_