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

#include "inventory/InventoryItem.h"
//#include "inventory/ItemRef.h"
#include "ship/DestinyManager.h"
#include "ship/TargetManager.h"
#include "SystemDB.h"

class AsteroidBeltManager;
class Character;
class Client;
class Concord;
class ContainerSE;
class Damage;
class DBSystemEntity;
class Drone;
class NPC;
class Player;
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

/*
 * base class for all SystemEntities
 * complete rewrite of entity class system  - allan  9 January 2016
 */
class SystemEntity {
    friend class SystemBubble;    /* only to update m_bubble */
public:
    SystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~SystemEntity()                             { /* Do nothing here */ }

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
    /** @todo  these will have to be adjusted when the classes are finished */
    virtual ItemSystemEntity*   GetItemSE()             { return nullptr; }
    virtual ContainerSE*        GetContSE()             { return nullptr; }
    virtual WreckSE*            GetWreckSE()            { return nullptr; }
    virtual DungeonSE*          GetDungeonSE()          { return nullptr; }
    /* Object */
    /** @todo  these will have to be adjusted when the classes are finished */
    virtual ObjectSystemEntity* GetObjectSE()           { return nullptr; }
    virtual AsteroidSE*         GetAsteroidSE()         { return nullptr; }
    virtual StructureSE*        GetJumpBridgeSE()       { return nullptr; }
    virtual StructureSE*        GetOutpostSE()          { return nullptr; }
    virtual StructureSE*        GetPOSSE()              { return nullptr; }
    virtual StructureSE*        GetTCUSE()              { return nullptr; }
    virtual StructureSE*        GetSBUSE()              { return nullptr; }
    virtual DeployableSE*       GetDeployableSE()       { return nullptr; }
    /* Dynamic */
    virtual DynamicSystemEntity* GetDynamicSE()         { return nullptr; }
    virtual NPC*                GetNPCSE()              { return nullptr; }
    virtual Drone*              GetDroneSE()            { return nullptr; }
    virtual Missile*            GetMissileSE()          { return nullptr; }
    virtual Ship*               GetShipSE()             { return nullptr; }
    virtual Concord*            GetConcordSE()          { return nullptr; }

    /* class type tests, grouped by base class.  public for anyone to access. */
    /* Base */
    virtual bool                IsSystemEntity()        { return true; }
    virtual bool                IsInanimateSE()         { return false; }
    /* Static */
    virtual bool                IsStaticEntity()        { return false; }
    virtual bool                IsVisibleSystemWide()   { return false; }
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
    virtual bool                IsPOSSE()               { return false; }
    virtual bool                IsTCUSE()               { return false; }
    virtual bool                IsSBUSE()               { return false; }
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

    /* generic access functions handled here */
    PyServiceMgr&               GetServices()           { return m_services; }
    SystemBubble*               SysBubble()             { return m_bubble; }
    SystemManager*              SystemMgr()             { return m_system; }
    TargetManager*              TargetMgr()             { return m_targMgr; }
    DestinyManager*             DestinyMgr()            { return m_destiny; }

    /* common functions for all entities handled here */
    /* public data queries  */
    virtual InventoryItemRef    GetSelf()               { return m_self; }
    virtual uint32              GetID()                 { return m_self->itemID(); }
    virtual double              GetRadius();            /* too long to put here */
    virtual uint32              GetLocationID()         { return m_self->locationID(); }
    virtual const char*         GetName() const         { return m_self->itemName().c_str(); }
    virtual const GPoint&       GetPosition() const     { return m_self->position(); }
    virtual void                SetPosition(GPoint &pos){ m_self->Relocate(pos); }
    inline virtual double       x()                     { return m_self->position().x; }
    inline virtual double       y()                     { return m_self->position().y; }
    inline virtual double       z()                     { return m_self->position().z; }

    /* public-access generic functions handled in base class. */
    void                        DropLoot(uint32 groupID, uint32 owner, uint32 locationID);
    void                        AwardSecurityStatus(InventoryItemRef m_self, Character* pChar);
    void                        SendDamageStateChanged(SystemEntity* source);
    bool                        ApplyDamage(Damage &d); /* This method is defined in Damage.cpp */
    double                      DistanceTo2(const SystemEntity* other);
    PyTuple*                    MakeDamageState();

    /* generic access functions handled here, but set elsewhere */
    virtual const GVector&      GetVelocity()           { return (m_destiny ? m_destiny->GetVelocity() : NULL_ORIGIN_V); }

    /* virtual functions default to base class and overridden as needed */
    virtual void                Process()               { /* Do nothing here */ }
    virtual void                Killed(Damage &fatal_blow) { /* Do nothing here */ }
    virtual void                EncodeDestiny(Buffer& into);
    virtual void                MakeDamageState(DoDestinyDamageState &into);
    virtual PyDict*             MakeSlimItem();

    /* virtual functions to be overridden in derived classes */
    virtual void                UpdateDamage()          { /* Do nothing here */ }
    virtual void                ProcessDestiny()        { /* Do nothing here */ }
    virtual void                ProcessOther()          { /* Do nothing here */ }
    virtual void                QueueDestinyUpdate(PyTuple **du) { /* Do nothing here */ }
    virtual void                QueueDestinyEvent(PyTuple **de)  { /* Do nothing here */ }
    virtual bool                LoadExtras(SystemDB *db){ return true; }

    /* virtual functions in base to allow common interface calls */
    /** @todo these need to be moved to target manager. */
    virtual void TargetLost(SystemEntity *who)          { /* Do nothing here */ }
    virtual void TargetAdded(SystemEntity *who)         { /* Do nothing here */ }
    virtual void TargetedAdd(SystemEntity *who)         { /* Do nothing here */ }
    virtual void TargetedLost(SystemEntity *who)        { /* Do nothing here */ }
    virtual void TargetsCleared()                       { /* Do nothing here */ }

    virtual uint32              GetTypeID()             { return m_self->typeID(); }
    virtual uint32              GetGroupID()            { return m_self->groupID(); }
    virtual EVEItemCategories   GetCategoryID()         { return m_self->categoryID(); }
    virtual EVEItemFlags        GetFlag()               { return m_self->flag(); }

    /** @todo (allan)  finish these ... not sure how yet.  */
    virtual uint32              GetCorporationID()      { return m_corpID; };
    virtual uint32              GetAllianceID()         { return m_allyID; };
    virtual uint32              GetWarFactionID()       { return m_warID; }

    /* virtual functions in base to allow common interface calls specific to ship entities */
    virtual void                SetPilot(Client* pClient){ /* Do nothing here */ }
    virtual bool                HasPilot()              { return false; }
    virtual Client*             GetPilot()              { return nullptr; }

    /* specific functions handled in this class. */

protected:
    SystemBubble*               m_bubble = nullptr;     /* we do not own this. never NULL in space */
    TargetManager*              m_targMgr = nullptr;    /* we do not own this. never NULL in space */
    SystemManager*              m_system = nullptr;     /* we do not own this  never NULL in space */
    DestinyManager*             m_destiny = nullptr;    /* we do not own this. never NULL in space */

    PyServiceMgr&               m_services;

    InventoryItemRef            m_self = InventoryItemRef();

    uint32                      m_warID = 0;
    uint32                      m_corpID = 0;
    uint32                      m_allyID = 0;

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
    virtual bool                IsStaticEntity()        { return true; }
    virtual bool                IsVisibleSystemWide()   { return true; }

    /* SystemEntity interface */
    virtual void                EncodeDestiny( Buffer& into );
    virtual PyDict*             MakeSlimItem();

    /** @todo (allan)  finish these ... not sure how yet.  */
    //virtual uint32 GetCorporationID()                   { return m_data.corporationID; };
    //virtual uint32 GetAllianceID()                      { return m_data.allianceID; };

    /* virtual functions to be overridden in derived classes */
    virtual bool                LoadExtras(SystemDB *db);

    /* specific functions handled in this class. */
    virtual double              GetRadius()             { return m_radius; }

private:
    double                      m_radius;

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
    AsteroidBeltManager*        BeltMgr()               { return m_beltMgr; }

protected:
    AsteroidBeltManager*        m_beltMgr;
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
    PyRep* m_jumps;
};


/* Non-Static / Non-Mobile / Non-Destructable / Celestial Objects - Containers, Wrecks, DeadSpace */
class ItemSystemEntity : public SystemEntity {
public:
    ItemSystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~ItemSystemEntity()                         { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual ItemSystemEntity* GetItemSE()               { return this; }
    /* class type tests. */
    /* Base */
    virtual bool IsInanimateSE()                        { return true; }
    /* Item */
    virtual bool IsItemEntity()                         { return true; }

    /* SystemEntity interface */
    virtual PyDict *MakeSlimItem();
    virtual void MakeDamageState(DoDestinyDamageState &into);
};

class DungeonSE : public ItemSystemEntity {
public:
    DungeonSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~DungeonSE()                                { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual DungeonSE* GetDungeonSE()                   { return this; }
    /* class type tests. */
    /* Base */
    virtual bool IsDungeonSE()                          { return true; }

    /* SystemEntity interface */
    virtual void EncodeDestiny( Buffer& into );

    virtual PyDict *MakeSlimItem();
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
    virtual bool IsInanimateSE()                        { return true; }
    /* Object */
    virtual bool IsObjectEntity()                       { return true; }

    /* SystemEntity interface */
    virtual void UpdateDamage();
    virtual void EncodeDestiny( Buffer& into );
    virtual void MakeDamageState(DoDestinyDamageState &into);

    virtual PyDict *MakeSlimItem();

    /* virtual functions default to base class and overridden as needed */
    virtual void Process();
    virtual void ProcessOther()                         { /* Do nothing here */ }
    virtual void Killed(Damage &fatal_blow);

};

class DeployableSE
: public ObjectSystemEntity
{
public:
    DeployableSE(
        InventoryItemRef structure,
        PyServiceMgr &services,
        SystemManager *system);
    virtual ~DeployableSE()                             { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual DeployableSE* GetDeployableSE()             { return nullptr; }
    /* class type tests. */
    virtual bool IsDeployableSE()                       { return true; }

};


/* Non-Static / Mobile / Destructable / Celestial Objects - PC's, NPC's, Drones, Ships, Missiles */
class DynamicSystemEntity : public SystemEntity {
public:
    DynamicSystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~DynamicSystemEntity();

    /* class type pointer querys. */
    virtual DynamicSystemEntity* GetDynamicSE()         { return this; }
    /* class type tests. */
    /* Dynamic */
    virtual bool IsDynamicEntity()                      { return true; }

    /* SystemEntity interface */
    virtual void UpdateDamage();
    virtual void ProcessDestiny();
    virtual void EncodeDestiny( Buffer& into );
    virtual void MakeDamageState(DoDestinyDamageState &into);

    virtual PyDict *MakeSlimItem();

    /* DynamicSystemEntity interface */
    virtual double GetMass();
    virtual double GetMaxVelocity();
    virtual double GetAgility();

    /* virtual functions default to base class and overridden as needed */
    virtual void Process();
    virtual void ProcessOther()                         { /* Do nothing here */ }
    virtual void Killed(Damage &fatal_blow);

    /* specific functions handled here. */
    bool Load(ServiceDB& from);
    void AwardBounty(Client* pClient);

};


#endif
