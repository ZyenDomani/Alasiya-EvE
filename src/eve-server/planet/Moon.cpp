
/**
 * @name Moon.cpp
 *   Specific Class for individual moons.
 * this class will hold all moon data and relative info for each moon.
 *
 * @Author:         Allan
 * @date:   30 April 2016
 */

#include "Moon.h"


MoonSE::MoonSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system)
: StaticSystemEntity(self, services, system)
{
}

bool MoonSE::LoadExtras(SystemDB *db) {
    if (!StaticSystemEntity::LoadExtras(db))
        return false;
    /** @todo use this to initialize moon data, create planet manager for moon, or whatever else
     * i decide is needed for moon management
     *  this is called when SE is created.
     */
    return true;
}

/** @note  general design notes
 * moonse will have a Moon class to hold data and call other functions/methods as needed
 * the PlanetMgr class will manage all aspects of moon data, init'd as a single instance (no reason for multiples)
 *
 *
 */

Moon::Moon()
{

}

