
 /**
  * @name WormholeMgr.h
  *     WH Spawn management system for Alasiya EvEmu
  *
  * @Author:        Allan
  * @date:          12 December 2015
  *
  */


#ifndef EVEMU_SYSTEM_WORMHOLEMGR_H_
#define EVEMU_SYSTEM_WORMHOLEMGR_H_


#include "ServiceDB.h"
#include "utils/Singleton.h"
#include "system/cosmicMgrs/ManagerDB.h"

/* this class will control all aspects of
 * creating, monitoring, removing, logging
 * connecting, and saving of wormholes.
 */

class PyServiceMgr;
class SystemManager;

class WormholeMgr
: public Singleton<WormholeMgr>
{
public:
    WormholeMgr();
    ~WormholeMgr();

    void Initialize(PyServiceMgr* svc);
    void Process();

    void Create(CosmicSignature& sig);
    // this will create a k162 and send data to anomalyMgr for inclusion
    void CreateExit(SystemManager* pFromSys, SystemManager* pToSys);

private:
    ManagerDB* m_mdb;
    ServiceDB* m_sdb;
    PyServiceMgr* m_services;

    Timer m_updateTimer;

    bool m_initalized;

    std::vector<uint32>         m_wormholes;   //exitID

};

//Singleton
#define sWHMgr \
    ( WormholeMgr::get() )

#endif  // EVEMU_SYSTEM_WORMHOLEMGR_H_



