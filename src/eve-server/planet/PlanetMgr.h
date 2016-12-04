
 /**
  * @name PlanetMgr.h
  *   Specific Class for managing planet resources
  * @Author:         Allan
  * @date:   30 April 2016
  */


#ifndef EVEMU_PLANET_PLANETMGR_H_
#define EVEMU_PLANET_PLANETMGR_H_

#include "planet/PlanetDB.h"
#include "planet/PlanetDataMgr.h"


class Client;
class Colony;
class PlanetSE;
class UUNCommand;
class UUNCommandList;
class PyServiceMgr;
class PlanetMgr
{
public:
    PlanetMgr(PyServiceMgr *mgr, Client* pClient, PlanetSE* pPlanet, Colony* pColony);
    virtual ~PlanetMgr()    { /* do nothing here */ }

    PyRep* UpdateNetwork(UUNCommandList& uuncl);
    PyRep* GetProgramResultInfo(uint32 pinID, uint32 typeID, PyList* heads, float headRadius);

protected:
    void CreatePin(UUNCommand& nc);
    void RemovePin(UUNCommand& nc);
    void CreateLink(UUNCommand& nc);
    void RemoveLink(UUNCommand& nc);
    void SetLinkLevel(UUNCommand& nc);
    void CreateRoute(UUNCommand& nc);
    void RemoveRoute(UUNCommand& nc);
    void SetSchematic(UUNCommand& nc);
    void UpgradeCommandCenter(UUNCommand& nc);
    void AddExtractorHead(UUNCommand& nc);
    void KillExtractorHead(UUNCommand& nc);
    void MoveExtractorHead(UUNCommand& nc);
    void InstallProgram(UUNCommand& nc);
    void PrioritizeRoute(UUNCommand& nc);

private:
    PyServiceMgr* m_svcMgr;
    Client* m_client;
    Colony* m_colony;
    PlanetSE* m_planet;

    PlanetDB m_db;
};


#endif  // EVEMU_PLANET_PLANETMGR_H_