
 /**
  * @name WormholeMgr.h
  *     WH Spawn managment system for Alasiya EvEmu
  *
  * @Author:        Allan
  * @date:          12 December 2015
  *
  */
 

#ifndef EVEMU_SYSTEM_WORMHOLEMGR_H_
#define EVEMU_SYSTEM_WORMHOLEMGR_H_


#include "ServiceDB.h"
#include "utils/Singleton.h"

/* this class will control all aspects of
 * creating, monitoring, removing, logging
 * connecting, and saving of all wormholes.
 */

class PyServiceMgr;

class WormholeMgr
: public Singleton<WormholeMgr>
{
public:
    WormholeMgr();
    virtual ~WormholeMgr()                              { /* do nothing here */ }

    void Initialize(PyServiceMgr* svc);
    void Process();

private:
    ServiceDB* m_db;
    PyServiceMgr* m_services;

    Timer m_updateTimer;

    bool m_initalized;

};

//Singleton
#define sWHMgr \
    ( WormholeMgr::get() )

#endif  // EVEMU_SYSTEM_WORMHOLEMGR_H_



