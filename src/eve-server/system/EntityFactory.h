 /**
  * @name EntityFactory.h
  *     Method to create dynamic entities for Alasiya EvEmu
  *     split from SystemManager
  *
  * @Author:        Allan
  * @date:          29 August 2025
  * @update:		28 July 2026
  *
  */

#pragma once

#include "system/BubbleManager.h"
#include "system/SolarSystem.h"
#include "system/SystemDB.h"

namespace EntityClass {
    // dataset to ease runtime lookups
    enum : uint8_t {
        Unknown = 0,
        NPC,
        Sentry,
        Container,
        Max_Classes
    };
}

class SystemManager;

class EntityFactory
: public Singleton< EntityFactory >
{
public:
    EntityFactory();
    ~EntityFactory();

    // call this on server start to build EntityFactory
    void Initialize();

    // you MUST call (your SystemManager)->AddEntity([this returned object]) after this to actually put the entity in space
    SystemEntity* BuildEntity( SystemManager& sysMgr, const DBSystemDynamicEntity& eData );

private:
    // The index is the GroupID, the value is the EntityClass enum.
    std::unordered_map<uint16, uint8> m_routingArray;
};

//Singleton
#define sEntityFactory \
    ( EntityFactory::get() )
