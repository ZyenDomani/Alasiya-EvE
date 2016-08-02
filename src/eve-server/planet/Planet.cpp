
 /**
  * @name Planet.cpp
  *   Specific Class for individual planets.
  * this class will hold all planet data and relative info for each planet.
  *
  * @Author:         Allan
  * @date:   30 April 2016
  */


#include "Planet.h"
#include "PlanetDB.h"


/** @note  general design notes
 * planetse will have a Planet class to hold data and call other functions/methods as needed
 * the PlanetMgr class will manage all aspects of planet data, init'd as a single instance (no reason for multiples)
 *
 *
 */

PlanetDataMgr::PlanetDataMgr()
{
}

int PlanetDataMgr::Initialize()
{
    _Populate();
    return 1;
}

void PlanetDataMgr::_Populate()
{
    double start = GetTimeUSeconds();
    DBQueryResult* res = new DBQueryResult();
    DBResultRow row;

    m_db->GetPlanetData(*res);
    while (res->GetRow(row)) {
        // SELECT planet.typeID, resource.typeID
        m_planetData.insert(std::pair<uint32, uint32>(row.GetInt(0), row.GetInt(1)));
    }

    //cleanup
    SafeDelete(res);
    sLog.Log("     PlanetDataMgr", "%u planet data groups in %u buckets loaded in %.3fms.",
             m_planetData.size(), m_planetData.bucket_count(), (GetTimeUSeconds() - start));
}

Planet::Planet()
{

}


PlanetSE::PlanetSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system)
: StaticSystemEntity(self, services, system)
{
}

bool PlanetSE::LoadExtras(SystemDB* db) {
    if (!StaticSystemEntity::LoadExtras(db))
        return false;
    /** @todo use this to initialize planet data, create planet manager, or whatever else
     * i decide is needed for planet management
     *  this is called when SE is created.
     */
    return true;
}
