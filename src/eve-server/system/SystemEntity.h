/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Zhur
    Updates:    Allan (complete rewrite)
*/

#ifndef __SYSTEMENTITY_H_INCL__
#define __SYSTEMENTITY_H_INCL__

#include "SystemDB.h"
#include "inventory/InventoryItem.h"
#include "system/DestinyManager.h"
#include "system/TargetManager.h"
#include "pos/PosMgrDB.h"

class AsteroidBeltMgr;
class Character;
class Client;
class Concord;
class ContainerSE;
class Damage;
class DBSystemEntity;
class Drone;
class NPC;
class Player;
class Sentry;
class SystemBubble;
class SystemManager;
class WreckSE;

class StationSE;
class StaticSystemEntity;
class PlanetSE;
class MoonSE;
class StargateSE;
class BeltSE;
class DynamicSystemEntity;
class ItemSystemEntity;
class ObjectSystemEntity;
class StructureSE;
class DeployableSE;
class AsteroidSE;
class Ship;
class DungeonSE;

class TowerSE;
class ArraySE;
class BatterySE;
class WeaponSE;

/*
 * base class for all SystemEntities
 * complete rewrite of entity class system  - allan  9 January 2016
 */
class SystemEntity {
    friend class SystemBubble;    /* only to update m_bubble */
public:
    SystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~SystemEntity();

    /* Process Calls - Overridden as needed in derived classes */
    virtual void                Process();

    /* (Allan) the next two sections eliminate the overhead of RTTI static casting.  */
    /* class type pointer querys, grouped by base class.  public for anyone to access. */
    /* Base */
    virtual SystemEntity*       GetSE()                 { return this; }
    /* Static */
    virtual StaticSystemEntity* GetStaticSE()           { return nullptr; }
    virtual StationSE*          GetStationSE()          { return nullptr; }
    virtual PlanetSE*           GetPlanetSE()           { return nullptr; }
    virtual MoonSE*             GetMoonSE()             { return nullptr; }
    virtual StargateSE*         GetGateSE()             { return nullptr; }
    virtual BeltSE*             GetBeltSE()             { return nullptr; }
    /* Item */
    virtual ItemSystemEntity*   GetItemSE()             { return nullptr; }
    virtual ContainerSE*        GetContSE()             { return nullptr; }
    virtual WreckSE*            GetWreckSE()            { return nullptr; }
    virtual DungeonSE*          GetDungeonSE()          { return nullptr; }
    /* Object */
    virtual ObjectSystemEntity* GetObjectSE()           { return nullptr; }
    virtual AsteroidSE*         GetAsteroidSE()         { return nullptr; }
    virtual StructureSE*        GetPOSSE()              { return nullptr; }
    virtual StructureSE*        GetJammerSE()           { return nullptr; }
    virtual StructureSE*        GetJumpBridgeSE()       { return nullptr; }
    virtual StructureSE*        GetOutpostSE()          { return nullptr; }
    virtual StructureSE*        GetCOSE()               { return nullptr; }
    virtual StructureSE*        GetTCUSE()              { return nullptr; }
    virtual StructureSE*        GetSBUSE()              { return nullptr; }
    virtual TowerSE*            GetTowerSE()            { return nullptr; }
    virtual ArraySE*            GetArraySE()            { return nullptr; }
    virtual WeaponSE*           GetWeaponSE()           { return nullptr; }
    virtual BatterySE*          GetBatterySE()          { return nullptr; }
    virtual DeployableSE*       GetDeployableSE()       { return nullptr; }
    virtual Sentry*             GetSentrySE()           { return nullptr; }
    /* Dynamic */
    virtual DynamicSystemEntity* GetDynamicSE()         { return nullptr; }
    virtual NPC*                GetNPCSE()              { return nullptr; }
    virtual Drone*              GetDroneSE()            { return nullptr; }
    virtual Missile*            GetMissileSE()          { return nullptr; }
    virtual Ship*               GetShipSE()             { return nullptr; }
    virtual Concord*            GetConcordSE()          { return nullptr; }

    /* class type tests, grouped by base class.  public for anyone to access. */
    /* Base */
    virtual bool                isGlobal()              { return m_self->isGlobal(); }    // not all items have this attribute set
    virtual bool                IsSystemEntity()        { return true; }
    virtual bool                IsInanimateSE()         { return false; }
    /* Static */
    virtual bool                IsStaticEntity()        { return false; }
    virtual bool                IsBeltSE()              { return false; }
    virtual bool                IsGateSE()              { return false; }
    virtual bool                IsPlanetSE()            { return false; }
    virtual bool                IsMoonSE()              { return false; }
    virtual bool                IsStationSE()           { return false; }
    /* Item */
    virtual bool                IsItemEntity()          { return false; }
    virtual bool                IsWreckSE()             { return false; }
    virtual bool                IsDungeonSE()           { return false; }
    virtual bool                IsCelestialSE()         { return false; }
    virtual bool                IsContainerSE()         { return false; }
    /* Object */
    virtual bool                IsObjectEntity()        { return false; }
    virtual bool                IsSentrySE()            { return false; }
    virtual bool                IsPOSSE()               { return false; }
    virtual bool                IsCOSE()                { return false; }
    virtual bool                IsTCUSE()               { return false; }
    virtual bool                IsSBUSE()               { return false; }
    virtual bool                IsTowerSE()             { return false; }
    virtual bool                IsArraySE()             { return false; }
    virtual bool                IsJammerSE()            { return false; }
    virtual bool                IsWeaponSE()            { return false; }
    virtual bool                IsBatterySE()           { return false; }
    virtual bool                IsOutpostSE()           { return false; }
    virtual bool                IsAsteroidSE()          { return false; }
    virtual bool                IsDeployableSE()        { return false; }
    virtual bool                IsJumpBridgeSE()        { return false; }
    /* Dynamic */
    virtual bool                IsDynamicEntity()       { return false; }
    virtual bool                IsLogin()               { return false; }
    virtual bool                IsInvul()               { return false; }
    virtual bool                IsNPCSE()               { return false; }
    virtual bool                IsDroneSE()             { return false; }
    virtual bool                IsMissileSE()           { return false; }
    virtual bool                IsShipSE()              { return false; }
    virtual bool                IsConcord()             { return false; }

    /* generic functions handled here */
    PyServiceMgr&               GetServices()           { return m_services; }
    SystemBubble*               SysBubble()             { return m_bubble; }
    SystemManager*              SystemMgr()             { return m_system; }
    TargetManager*              TargetMgr()             { return m_targMgr; }
    DestinyManager*             DestinyMgr()            { return m_destiny; }

    /* common functions for all entities handled here */
    /* public data queries  */
    InventoryItemRef            GetSelf()               { return m_self; }
    uint32                      GetTypeID()             { return m_self->typeID(); }
    uint32                      GetGroupID()            { return m_self->groupID(); }
    EVEItemCategories           GetCategoryID()         { return m_self->categoryID(); }
    EVEItemFlags                GetFlag()               { return m_self->flag(); }
    uint32                      GetID()                 { return m_self->itemID(); }
    double                      GetRadius()             { return m_radius; }
    uint32                      GetLocationID()         { return m_self->locationID(); }
    const char*                 GetName() const         { return m_self->itemName().c_str(); }
    const GPoint&               GetPosition() const     { return m_self->position(); }
    void                  SetPosition(const GPoint &pos){ m_self->Relocate(pos); }
    inline double               x()                     { return m_self->position().x; }
    inline double               y()                     { return m_self->position().y; }
    inline double               z()                     { return m_self->position().z; }
    uint32                      GetCorporationID()      { return m_corpID; }
    uint32                      GetAllianceID()         { return m_allyID; }
    uint32                      GetWarFactionID()       { return m_warID; }
    uint32                      GetOwnerID()            { return m_ownerID; }

    /* public generic functions handled in base class. */
    void                        DropLoot(WreckContainerRef wreckRef, uint32 groupID, uint32 owner);
    void                        AwardSecurityStatus(InventoryItemRef m_self, Character* pChar);
    void                        SendDamageStateChanged(SystemEntity* source);
    bool                        ApplyDamage(Damage &d); /* This method is defined in Damage.cpp */
    double                      DistanceTo2(const SystemEntity* other);
    PyTuple*                    MakeDamageState();

    /* public specific functions handled in base class. */
    virtual void                Abandon();


    /* generic functions handled here, but set elsewhere */
    const GVector&              GetVelocity()           { return (m_destiny ? m_destiny->GetVelocity() : NULL_ORIGIN_V); }

    /* virtual functions default to base class and overridden as needed */
    virtual void                Killed(Damage &fatal_blow) { /* Do nothing here */ }
    virtual void                EncodeDestiny(Buffer& into);
    virtual void                MakeDamageState(DoDestinyDamageState &into);
    virtual PyDict*             MakeSlimItem();

    /* virtual functions to be overridden in derived classes */
    virtual void                UpdateDamage()          { /* Do nothing here */ }
    virtual bool                LoadExtras(SystemDB *db){ return true; }

    /* virtual functions in base to allow common interface calls specific to ship entities */
    virtual void                SetPilot(Client* pClient){ /* Do nothing here */ }
    virtual bool                HasPilot()              { return false; }
    virtual Client*             GetPilot()              { return nullptr; }
    virtual void                Delete()                { /* Do nothing here */ }  // this is only for asteroids and missiles and containers/wrecks (so far...)

protected:
    SystemBubble*               m_bubble;               /* we do not own this. never NULL in space */
    TargetManager*              m_targMgr;              /* we do not own this. never NULL in space */
    SystemManager*              m_system;               /* we do not own this  never NULL in space */
    DestinyManager*             m_destiny;              /* we do not own this. never NULL in space */

    PyServiceMgr&               m_services;

    InventoryItemRef            m_self;

    double                      m_radius;

    /* this is POS ForceField status */
    int32 m_harmonic;

    /* ease of access to common data for ownable objects */
    uint32                      m_warID;
    uint32                      m_corpID;
    uint32                      m_allyID;
    uint32                      m_ownerID;

};


/* Static / Non-Mobile / Non-Destructable / Celestial Objects - Suns, Planets, Moons, Belts, Gates, Stations */
class StaticSystemEntity : public SystemEntity {
public:
    StaticSystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~StaticSystemEntity()                       { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual StaticSystemEntity* GetStaticSE()           { return this; }
    /* class type tests. */
    /* Base */
    virtual bool                IsInanimateSE()         { return true; }
    /* Static */
    virtual bool                isGlobal()              { return true; }    // just in case item->isGlobal() fails here...which it may
    virtual bool                IsStaticEntity()        { return true; }

    /* SystemEntity interface */
    virtual void                EncodeDestiny( Buffer& into );
    virtual PyDict*             MakeSlimItem();

    /* virtual functions to be overridden in derived classes */
    virtual bool                LoadExtras(SystemDB *db);

};

class BeltSE
: public StaticSystemEntity
{
public:
    BeltSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~BeltSE()                                   { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual BeltSE*             GetBeltSE()             { return this; }
    /* class type tests. */
    virtual bool                IsBeltSE()              { return true; }

    /* virtual functions to be overridden in derived classes */
    virtual bool                LoadExtras(SystemDB *db);

    /* generic access functions handled here */
    AsteroidBeltMgr*            BeltMgr()               { return m_beltMgr; }

    /* specific functions handled in this class. */
    void          SetBeltMgr(AsteroidBeltMgr* beltMgr)  { m_beltMgr = beltMgr; }

protected:
    AsteroidBeltMgr*            m_beltMgr;

};

class StargateSE
: public StaticSystemEntity
{
public:
    StargateSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~StargateSE()                               { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual StargateSE*         GetGateSE()             { return this; }
    /* class type tests. */
    virtual bool                IsGateSE()              { return true; }

    /* SystemEntity interface */
    virtual PyDict*             MakeSlimItem();

    /* virtual functions to be overridden in derived classes */
    virtual bool                LoadExtras(SystemDB *db);

protected:
    PyRep*                      m_jumps;

};


/* Non-Static / Non-Mobile / Non-Destructable / Celestial Objects - Containers, Wrecks, DeadSpace */
class ItemSystemEntity : public SystemEntity {
public:
    ItemSystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~ItemSystemEntity()                         { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual ItemSystemEntity*   GetItemSE()             { return this; }
    /* class type tests. */
    /* Base */
    virtual bool                IsInanimateSE()         { return true; }
    /* Item */
    virtual bool                IsItemEntity()          { return true; }

    /* SystemEntity interface */
    virtual void                EncodeDestiny( Buffer& into );
    virtual void                MakeDamageState(DoDestinyDamageState &into);

    virtual PyDict*             MakeSlimItem();
};

class DungeonSE : public ItemSystemEntity {
public:
    DungeonSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~DungeonSE()                                { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual DungeonSE*          GetDungeonSE()          { return this; }
    /* class type tests. */
    /* Base */
    virtual bool                IsDungeonSE()           { return true; }

    /* SystemEntity interface */
    virtual void                EncodeDestiny( Buffer& into );

    virtual PyDict*             MakeSlimItem();
};

/* Non-Static / Non-Mobile / Destructable / Celestial Objects - POS Structures, Outposts, Asteroids, Deployables */
class ObjectSystemEntity : public SystemEntity {
public:
    ObjectSystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~ObjectSystemEntity()                       { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual ObjectSystemEntity* GetObjectSE()           { return this; }
    /* class type tests. */
    /* Base */
    virtual bool                IsInanimateSE()         { return true; }
    /* Object */
    virtual bool                IsObjectEntity()        { return true; }

    /* SystemEntity interface */
    virtual void                UpdateDamage();
    virtual void                EncodeDestiny( Buffer& into );
    virtual void                MakeDamageState(DoDestinyDamageState &into);

    virtual PyDict*             MakeSlimItem();

    /* virtual functions default to base class and overridden as needed */
    virtual void                Killed(Damage &fatal_blow);
};

/* Mobile Warp Disruptors */
class DeployableSE
: public ObjectSystemEntity
{
public:
    DeployableSE(InventoryItemRef self, PyServiceMgr& services, SystemManager* system, const FactionData& data);
    virtual ~DeployableSE();

    /* class type pointer querys. */
    virtual DeployableSE*       GetDeployableSE()       { return nullptr; }
    /* class type tests. */
    virtual bool                IsDeployableSE()        { return true; }
};


/* Non-Static / Mobile / Destructable / Celestial Objects - PC's, NPC's, Drones, Ships, Missiles */
class DynamicSystemEntity : public SystemEntity {
public:
    DynamicSystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~DynamicSystemEntity()                      { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual DynamicSystemEntity* GetDynamicSE()         { return this; }
    /* class type tests. */
    /* Dynamic */
    virtual bool                IsDynamicEntity()       { return true; }

    /* SystemEntity interface */
    virtual void                UpdateDamage();
    virtual void                EncodeDestiny( Buffer& into );
    virtual void                MakeDamageState(DoDestinyDamageState &into);

    virtual PyDict*             MakeSlimItem();

    /* virtual functions default to base class and overridden as needed */
    virtual void                Killed(Damage &fatal_blow);

    /* specific functions handled here. */
    bool                        Load();
    void                        AwardBounty(Client* pClient);
};


#endif
