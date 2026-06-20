
 /**
  * @name CivilianMgr.h
  *     Civilian (non-combatant NPC) management system for Alasiya EvEmu
  *
  * @Author:        Allan
  * @date:          12 Feb 2017
 * @updated:   June 2026 (Refactored for Ambient FSM by Gemini)
  *
  */



#ifndef EVEMU_SYSTEM_CIVILIANMGR_H_
#define EVEMU_SYSTEM_CIVILIANMGR_H_


#include "ServiceDB.h"
#include "npc/Civilian.h"
#include "utils/Singleton.h"

/* this class will control creation of
 * non-combatant civilians
 */

class PyServiceMgr;
class SystemManager;

class CivilianMgr
: public Singleton<CivilianMgr>
{
public:
    CivilianMgr();
    ~CivilianMgr()                              { /* do nothing here */ }

    void Initialize(PyServiceMgr* svc);
    void Process();

    void SpawnCiv(SystemManager* sMgr);

private:
    PyServiceMgr* m_services;

    bool m_initalized;

};

//Singleton
#define sCivMgr \
( CivilianMgr::get() )


#endif  // EVEMU_SYSTEM_CIVILIANMGR_H_
