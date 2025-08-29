 /**
  * @name DynamicEntityFactory.h
  *     Method to create dynamic entities for Alasiya EvEmu
  *     split from SystemManager
  *
  * @Author:        Allan
  * @date:          29 August 2025
  *
  */


#ifndef _EVE_SERVER_SYSTEM_ENTITY_FACTORY_H_
#define _EVE_SERVER_SYSTEM_ENTITY_FACTORY_H_


#include "system/BubbleManager.h"
#include "system/SolarSystem.h"
#include "system/SystemDB.h"


class SystemManager;

class DynamicEntityFactory {
public:
    // you MUST call (your SystemManager)->AddEntity([this returned object]) after this to actually put the entity in space
    static SystemEntity* BuildEntity(SystemManager& pSysMgr, const DBSystemDynamicEntity& data);
};


#endif  // _EVE_SERVER_SYSTEM_ENTITY_FACTORY_H_