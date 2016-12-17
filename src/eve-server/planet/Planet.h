
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

#include "EntityList.h"
#include "StaticDataMgr.h"
#include "system/SystemEntity.h"

/** @todo update this to create a planet item instead of the default celestial item */
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

    /* Process Calls - Overridden as needed in derived classes */
    virtual void                Process();

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

    void AbandonColony(Colony* pColony);
    void CreateCustomsOffice();
    void SetCustomsOffice(SystemEntity* pSE)            { pCO = pSE; }

    SystemEntity* GetCustomsOffice()                    { return pCO; }

    Colony* GetColony(Client* pClient);

protected:
    SystemEntity* pCO;  // our Customs Office SE  - we dont own this
    PlanetResourceData m_data;

    Timer m_colonyTimer;

    /* map of charID, Colony* for this planet.
     *   this is a hack, as the client will not reuse planet bound objects,
     * instead calling for a new object on every call.  this scheme will prevent data races on colony calls
     *  Colony* is owned by this planetSE
     */
    std::map<uint32, Colony*> m_colonies;

    bool m_hasColony = false;
};

#endif  // EVEMU_PLANET_PLANET_H_