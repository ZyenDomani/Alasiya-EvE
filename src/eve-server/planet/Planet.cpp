
 /**
  * @name Planet.cpp
  *   Specific Class for individual planets.
  * this class will hold all planet data and relative info for each planet.
  *
  * @Author:         Allan
  * @date:   30 April 2016
  */


#include "Planet.h"


PlanetSE::PlanetSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system)
: StaticSystemEntity(self, services, system)
{
}

bool PlanetSE::LoadExtras(SystemDB *db) {
    if (!StaticSystemEntity::LoadExtras(db))
        return false;
    /** @todo use this to initialize planet data, create planet manager, or whatever else
     * i decide is needed for planet management
     *  this is called when SE is created.
     */
    return true;
}

/** @note  general design notes
 * planetse will have a Planet class to hold data and call other functions/methods as needed
 * the PlanetMgr class will manage all aspects of planet data, init'd as a single instance (no reason for multiples)
 * 
 *
 *
 *
 */

Planet::Planet()
{

}

