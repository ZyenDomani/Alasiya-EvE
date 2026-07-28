/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Rewrite:    Allan
*/

#ifndef _EVE_SERVER_SYSTEM_ENTITY_H_
#define _EVE_SERVER_SYSTEM_ENTITY_H_

#include "../../eve-common/EVE_Dungeon.h"

#include "SystemDB.h"
#include "inventory/InventoryItem.h"
#include "pos/PosMgrDB.h"
#include "system/DestinyManager.h"
#include "system/TargetManager.h"

class BeltMgr;
class Character;
class Client;
class Concord;
class ContainerSE;
class Damage;
class DroneSE;
class NPC;
class Sentry;
class SystemBubble;
class SystemManager;
class WreckSE;
class FieldSE;
class ProbeSE;
class FieldSystemEntity;
class CynoSE;


class StationSE;
class StaticSystemEntity;
class PlanetSE;
class MoonSE;
class StargateSE;
class BeltSE;
class DynamicSystemEntity;
class ItemSystemEntity;
class ObjectSystemEntity;
class WormholeSE;
class AnomalySE;
class StructureSE;
class CustomsSE;
class DeployableSE;
class AsteroidSE;
class ShipSE;
class DungeonEditSE;

class TowerSE;
class TCUSE;
class SBUSE;
class IHubSE;
class ArraySE;
class BatterySE;
class ModuleSE;
class WeaponSE;
class ReactorSE;
class JumpBridgeSE;
class PlatformSE;
class OutpostSE;

// complete rewrite of entity class system  - allan  9 January 2016
// finally added rule of 5.  -allan 4Nov21
//   my piss-poor understanding of rule of 5 has been challenged...currently updating c'tors  21Oct25


/* SE - base class for all SystemEntities
 * - no TargetMgr, no DestinyMgr
 * - provides base method calls
 */
class SystemEntity {
    friend class SystemBubble;    /* only to update m_bubble */
public:
    // default c'tor
    SystemEntity() =delete;
    // main c'tor
    SystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    // copy c'tor
    SystemEntity(const SystemEntity& oth) =delete;
    // move c'tor
    SystemEntity(SystemEntity&& oth) noexcept =default;
    // copy assignment
    SystemEntity& operator=(const SystemEntity& oth) =delete;
    // move assignment
    SystemEntity& operator=(SystemEntity&& oth) =default;
    // d'tor
    virtual ~SystemEntity()                             { /* do nothing here */ }

    /* Process Calls - Overridden as needed in derived classes */
    virtual void                Process();
    virtual bool                ProcessTic()            { return true; }   // not used yet

    /* (Allan) the next two sections eliminate the overhead of RTTI static casting.  */
    /* class type pointer querys, grouped by base class.  public for anyone to access. */
    /* Base */
    virtual SystemEntity*       GetSE()                 { return this; }
    virtual const char*         GetSEType()             { return "Base SE"; }
    /* Static */
    virtual StaticSystemEntity* GetStaticSE()           { return nullptr; }
    virtual StationSE*          GetStationSE()          { return nullptr; }
    virtual PlanetSE*           GetPlanetSE()           { return nullptr; }
    virtual MoonSE*             GetMoonSE()             { return nullptr; }
    virtual StargateSE*         GetGateSE()             { return nullptr; }
    virtual BeltSE*             GetBeltSE()             { return nullptr; }
	/* Field */
    virtual FieldSystemEntity*  GetFieldEntity()        { return nullptr; }
    virtual FieldSE*            GetFieldSE()            { return nullptr; }
    virtual CynoSE*             GetCynoSE()             { return nullptr; }
    virtual DungeonEditSE*      GetDungeonEditSE()      { return nullptr; }
    /* Item */
    virtual ItemSystemEntity*   GetItemSE()             { return nullptr; }
    virtual ContainerSE*        GetContSE()             { return nullptr; }
    virtual WreckSE*            GetWreckSE()            { return nullptr; }
    virtual AnomalySE*          GetAnomalySE()          { return nullptr; }
    virtual WormholeSE*         GetWormholeSE()         { return nullptr; }
    virtual ProbeSE*            GetProbeSE()            { return nullptr; }
    /* Object */
    virtual ObjectSystemEntity* GetObjectSE()           { return nullptr; }
    virtual AsteroidSE*         GetAsteroidSE()         { return nullptr; }
    virtual StructureSE*        GetPOSSE()              { return nullptr; }
    virtual StructureSE*        GetJammerSE()           { return nullptr; }
    virtual JumpBridgeSE*       GetJumpBridgeSE()       { return nullptr; }
    virtual OutpostSE*          GetOutpostSE()          { return nullptr; }
    virtual PlatformSE*         GetPlatformSE()         { return nullptr; }
    virtual TowerSE*            GetTowerSE()            { return nullptr; }
    virtual ArraySE*            GetArraySE()            { return nullptr; }
    virtual WeaponSE*           GetWeaponSE()           { return nullptr; }
    virtual BatterySE*          GetBatterySE()          { return nullptr; }
    virtual DeployableSE*       GetDeployableSE()       { return nullptr; }
    virtual Sentry*             GetSentrySE()           { return nullptr; }
    virtual ModuleSE*           GetModuleSE()           { return nullptr; }
    virtual ReactorSE*          GetReactorSE()          { return nullptr; }
    virtual CustomsSE*          GetCOSE()               { return nullptr; }
    virtual TCUSE*              GetTCUSE()              { return nullptr; }
    virtual SBUSE*              GetSBUSE()              { return nullptr; }
    virtual IHubSE*             GetIHubSE()             { return nullptr; }
    /* Dynamic */
    virtual DynamicSystemEntity* GetDynamicSE()         { return nullptr; }
    virtual NPC*                GetNPCSE()              { return nullptr; }
    virtual DroneSE*            GetDroneSE()            { return nullptr; }
    virtual Missile*            GetMissileSE()          { return nullptr; }
    virtual ShipSE*             GetShipSE()             { return nullptr; }
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
	/* Field */
    virtual bool                IsFieldEntity()         { return false; }
    virtual bool                IsFieldSE()             { return false; }
    virtual bool                IsCynoSE()              { return false; }
    virtual bool                IsDungeonEditSE()       { return false; }
    /* Item */
    virtual bool                IsItemEntity()          { return false; }
    virtual bool                IsAnomalySE()           { return false; }
    virtual bool                IsWormholeSE()          { return false; }
    virtual bool                IsCelestialSE()         { return false; }
    virtual bool                IsContainerSE()         { return false; }
    virtual bool                IsProbeSE()             { return false; }
    /* Object */
    virtual bool                IsObjectEntity()        { return false; }
    virtual bool                IsSentrySE()            { return false; }
    virtual bool                IsPOSSE()               { return false; }
    virtual bool                IsCOSE()                { return false; }
    virtual bool                IsTCUSE()               { return false; }
    virtual bool                IsSBUSE()               { return false; }
    virtual bool                IsIHubSE()              { return false; }
    virtual bool                IsTowerSE()             { return false; }
    virtual bool                IsArraySE()             { return false; }
    virtual bool                IsJammerSE()            { return false; }
    virtual bool                IsWeaponSE()            { return false; }
    virtual bool                IsBatterySE()           { return false; }
    virtual bool                IsModuleSE()            { return false; }
    virtual bool                IsMoonMiner()           { return false; }
    virtual bool                IsOutpostSE()           { return false; }
    virtual bool                IsPlatformSE()          { return false; }
    virtual bool                IsAsteroidSE()          { return false; }
    virtual bool                IsDeployableSE()        { return false; }
    virtual bool                IsJumpBridgeSE()        { return false; }
    virtual bool                IsReactorSE()           { return false; }
    virtual bool                IsOperSE()              { return false; }
    /* Dynamic */
    virtual bool                IsDynamicEntity()       { return false; }
    virtual bool                IsLogin()               { return false; }
    virtual bool                IsInvul()               { return false; }
    virtual bool                IsFrozen()              { return false; }
    virtual bool                IsNPCSE()               { return false; }
    virtual bool                IsDroneSE()             { return false; }
    virtual bool                IsWreckSE()             { return false; }
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
    uint16                      GetTypeID()             { return m_self->typeID(); }
    uint32                      GetGroupID()            { return m_self->groupID(); }
    uint8                       GetCategoryID()         { return m_self->categoryID(); }
    EVEItemFlags                GetFlag()               { return m_self->flag(); }
    uint32                      GetID()                 { return m_self->itemID(); }
    int32                       GetRadius()             { return m_self->radius(); }
    uint32                      GetLocationID()         { return m_self->locationID(); }
    const char*                 GetName() const         { return m_self->name(); }
    const Vector3d&             GetPosition() const     { return m_self->position(); }
    void                        SetPosition(const Vector3d &pos)
                                                        { m_self->SetPosition(pos); }
    inline double               x()                     { return m_self->position().x; }
    inline double               y()                     { return m_self->position().y; }
    inline double               z()                     { return m_self->position().z; }
    int32                       GetAllianceID()         { return m_allyID; }
    int32                       GetWarFactionID()       { return m_warID; }
    uint32                      GetCorporationID()      { return m_corpID; }
    uint32                      GetOwnerID()            { return m_ownerID; }
    uint32                      GetFleetID()            { return m_fleetID; }
    void                     SetFleetID(uint32 fleetID) { m_fleetID = fleetID; }

    int32                       GetHarmonic()           { return m_harmonic; }
    void                        SetHarmonic(int8 set)   { m_harmonic = set; }
    bool                        IsAbandoned()           { return m_abandoned; }
    void                        SetAbandoned(bool set)  { m_abandoned = set; }

    /* public generic functions handled in base class. */
    void                        DropLoot(WreckContainerRef wreckRef, uint32 groupID, uint32 ownerID);
    void                        AwardSecurityStatus(InventoryItemRef iRef, Character* pChar);
    bool                        ApplyDamage(Damage &d); /* This method is defined in Damage.cpp */
    bool                        IsImmuneToEWar()        { return m_self->GetAttribute(AttrEwImmuneTarget).get_bool(); }
    double                      DistanceTo2(const SystemEntity* other);
    PyTuple*                    MakeDamageState();

    /* public specific functions handled in base class. */
    virtual void                Abandon();
    virtual void                SendDamageStateChanged();  // this uses targetMgr update to send to all interested parties
    virtual void                MakeSlimItemChange();      // sends updated item info to players in bubble

    /* generic functions handled here, but set elsewhere */
    bool                        IsDead()                { return m_killed; }
    // this is virtual to allow drones to override it (they dont have DestinyMgr)
    virtual const Vector3d&      GetVelocity()           { return (m_destiny != nullptr ? m_destiny->GetVelocity() : NULL_ORIGIN); }

    /* virtual functions default to base class and overridden as needed */
    virtual void                Killed(Damage &damage);
    virtual void                EncodeDestiny(Buffer& into);
    virtual void                MakeDamageState(DoDestinyDamageState &into);
    virtual PyDict*             MakeSlimItem();

    /* virtual functions to be overridden in derived classes */
    virtual void     MissileLaunched(Missile* pMissile) { /* Do nothing here */ }
    virtual void                UpdateDamage()          { /* Do nothing here */ }
    virtual bool                LoadExtras()            { return true; }
    // this will remove SE* from system and call Delete() on it's itemRef for non-containers.
    //caller MUST call SafeDelete() on SE after this returns.
    virtual void                Delete();

    /* virtual functions in base to allow common interface calls specific to ship entities */
    virtual void              SetPilot(Client* pClient) { /* Do nothing here */ }
    virtual bool                HasPilot()              { return false; }
    virtual Client*             GetPilot()              { return nullptr; }

    /* virtual functions for npc/drone AI and player reporting */
    virtual void                ReportDamage(uint8 type=0, SystemEntity* pSourceSE=nullptr)
                                                        { /* do nothing here */ }
    // we have acquired a target lock
    virtual void      TargetAdded(SystemEntity* pTargetSE) { /* do nothing here */ }
    // this is call to inform us of yellowbox
    virtual void      TargetedAdd(SystemEntity* pSourceSE) { /* do nothing here */ }
    virtual void       TargetLost(SystemEntity* pTargetSE) { /* do nothing here */ }
    virtual void     TargetedLost(SystemEntity* pSourceSE) { /* do nothing here */ }
    // this is call to inform us of redbox
    virtual void     ShipTargeted(SystemEntity* pSourceSE) { /* do nothing here */ }
    virtual void     ShipAttacked(SystemEntity* pSourceSE) { /* do nothing here */ }
    // make sure you check for nullptr here...ships firing missiles can be killed before missile hits
    virtual void ShipTakingDamage(SystemEntity* pSourceSE) { /* do nothing here */ }
    // our assigned ship was killed...wtf we do now?
    virtual void       ShipKilled(SystemEntity* pSourceSE) { /* do nothing here */ }



protected:
    SystemBubble*               m_bubble;               /* we do not own this. never NULL in space */
    SystemManager*              m_system;               /* we do not own this  never NULL in space */
    TargetManager*              m_targMgr;              /* we do not own this. only Destructible items will have it */
    DestinyManager*             m_destiny;              /* we do not own this. only mobile items will have it */

    PyServiceMgr&               m_services;

    InventoryItemRef            m_self;

    bool                        m_killed;
    bool                        m_abandoned;
    bool                        m_damageReported;

    /* this is POS ForceField status/value */
    int32                       m_harmonic;

    /* ease of access to common data for ownable objects */
    int32                       m_warID;
    int32                       m_allyID;   // this is salvage factionID for npc wrecks
    uint32                      m_corpID;
    uint32                      m_fleetID;
    uint32                      m_ownerID;
};


/* SSE - Static, Non-Mobile, Non-Destructable
 * - Suns, Planets, Moons, Belts, Gates, NPC Stations
 * - no TargetMgr, no DestinyMgr
 */
class StaticSystemEntity : public SystemEntity {
public:
    // default c'tor
    StaticSystemEntity() =delete;
    // main c'tor
    StaticSystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    // copy c'tor
    StaticSystemEntity(const StaticSystemEntity* oth) =delete;
    // move c'tor
    StaticSystemEntity(StaticSystemEntity&& oth) noexcept =default;
    // copy assignment
    StaticSystemEntity& operator=(StaticSystemEntity& oth) =delete;
    // move assignment
    StaticSystemEntity& operator=(StaticSystemEntity&& oth) =default;
    // d'tor
    virtual ~StaticSystemEntity()                       { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual const char*         GetSEType()             { return "Static SE"; }
    virtual StaticSystemEntity* GetStaticSE()           { return this; }
    /* class type tests. */
    /* Base */
    virtual bool                isGlobal()              { return true; }    // just in case item->isGlobal() fails here...which it may
    virtual bool                IsSystemEntity()        { return false; }
    virtual bool                IsInanimateSE()         { return true; }
    /* Static */
    virtual bool                IsStaticEntity()        { return true; }
    /* SystemEntity interface */
    virtual void                EncodeDestiny(Buffer& into);
    virtual PyDict*             MakeSlimItem();

    /* virtual functions to be overridden in derived classes */
    virtual bool                LoadExtras();
};

class BeltSE
: public StaticSystemEntity
{
public:
    // default c'tor
    BeltSE() =delete;
    // main c'tor
    BeltSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    // copy c'tor
    BeltSE(const BeltSE* oth) =delete;
    // move c'tor
    BeltSE(BeltSE&& oth) noexcept =default;
    // copy assignment
    BeltSE& operator=(BeltSE& oth) =delete;
    // move assignment
    BeltSE& operator=(BeltSE&& oth) =default;
    // d'tor
    virtual ~BeltSE()                                   { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual const char*         GetSEType()             { return "Belt SE"; }
    virtual BeltSE*             GetBeltSE()             { return this; }
    /* class type tests. */
    virtual bool                IsSystemEntity()        { return true; }
    virtual bool                IsBeltSE()              { return true; }
    /* virtual functions to be overridden in derived classes */
    virtual bool                LoadExtras();

    /* generic access functions handled here */
    BeltMgr*                    GetBeltMgr()            { return m_beltMgr; }

    /* specific functions handled in this class. */
    void                   SetBeltMgr(BeltMgr* beltMgr) { m_beltMgr = beltMgr; }

protected:
    BeltMgr*                    m_beltMgr;		// we dont own this
};

class StargateSE
: public StaticSystemEntity
{
public:
    // default c'tor
    StargateSE() =delete;
    // main c'tor
    StargateSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    // copy c'tor
    StargateSE(const StargateSE* oth) =delete;
    // move c'tor
    StargateSE(StargateSE&& oth) noexcept =default;
    // copy assignment
    StargateSE& operator=(StargateSE& oth) =delete;
    // move assignment
    StargateSE& operator=(StargateSE&& oth) =default;
    // d'tor
    virtual ~StargateSE()                               { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual const char*         GetSEType()             { return "Stargate SE"; }
    virtual StargateSE*         GetGateSE()             { return this; }
    /* class type tests. */
    virtual bool                IsSystemEntity()        { return true; }
    virtual bool                IsGateSE()              { return true; }
    /* SystemEntity interface */
    virtual PyDict*             MakeSlimItem();
    /* virtual functions to be overridden in derived classes */
    virtual bool                LoadExtras();

    /* specific functions handled in this class. */
    StructureSE* GetMySBU()                             { return m_sbuSE; }
    bool HasSBU()                                       { return (m_sbuSE != nullptr); }
    void SetSBU(StructureSE* pSE)                       { m_sbuSE = pSE; }

protected:
    PyRep*                      m_jumps;
    StructureSE*                m_sbuSE;
};

/* FSE - Non-Static, Non-Mobile, Non-Destructible
 * - ForceFields, CynoFields
 * - no TargetMgr, no DestinyMgr
 */
class FieldSystemEntity : public SystemEntity {
public:
    // default c'tor
    FieldSystemEntity() =delete;
    // main c'tor
    FieldSystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    // copy c'tor
    FieldSystemEntity(const FieldSystemEntity* oth) =delete;
    // move c'tor
    FieldSystemEntity(FieldSystemEntity&& oth) noexcept =default;
    // copy assignment
    FieldSystemEntity& operator=(FieldSystemEntity& oth) =delete;
    // move assignment
    FieldSystemEntity& operator=(FieldSystemEntity&& oth) =default;
    // d'tor
    virtual ~FieldSystemEntity()                        { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual const char*         GetSEType()             { return "Field SE"; }
    virtual FieldSystemEntity*  GetFieldEntity()        { return this; }
    /* class type tests. */
    virtual bool                IsSystemEntity()        { return false; }
    virtual bool                IsFieldEntity()         { return true; }
    /* Base */
    virtual bool                isGlobal()              { return false; }
    virtual bool                IsInanimateSE()         { return true; }
    /* SystemEntity interface */
    virtual void                EncodeDestiny(Buffer& into);
    virtual PyDict*             MakeSlimItem();
};

/* POS ForceField */
class FieldSE
: public FieldSystemEntity
{
public:
    // default c'tor
    FieldSE() =delete;
    // main c'tor
    FieldSE(InventoryItemRef self, PyServiceMgr& services, SystemManager* system, const FactionData& data);
    // copy c'tor
    FieldSE(const FieldSE* oth) =delete;
    // move c'tor
    FieldSE(FieldSE&& oth) noexcept =default;
    // copy assignment
    FieldSE& operator=(FieldSE& oth) =delete;
    // move assignment
    FieldSE& operator=(FieldSE&& oth) =default;
    // d'tor
    virtual ~FieldSE()                                  { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual const char*         GetSEType()             { return "Forcefield SE"; }
    virtual FieldSE*            GetFieldSE()            { return this; }
    /* class type tests. */
    virtual bool                IsFieldSE()             { return true; }
    /* Base */
    virtual bool                isGlobal()              { return false; }
    /* SystemEntity interface */
    virtual void                EncodeDestiny(Buffer& into);
    virtual PyDict*             MakeSlimItem();
};

/* Cynosural Field */
class CynoSE
: public FieldSystemEntity
{
public:
    // default c'tor
    CynoSE() =delete;
    // main c'tor
    CynoSE(InventoryItemRef self, PyServiceMgr& services, SystemManager* system, const FactionData& data);
    // copy c'tor
    CynoSE(const CynoSE* oth) =delete;
    // move c'tor
    CynoSE(CynoSE&& oth) noexcept =default;
    // copy assignment
    CynoSE& operator=(CynoSE& oth) =delete;
    // move assignment
    CynoSE& operator=(CynoSE&& oth) =default;
    // d'tor
    virtual ~CynoSE()                                   { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual const char*         GetSEType()             { return "Cyno SE"; }
    virtual CynoSE*             GetCynoSE()             { return this; }
    /* class type tests. */
    virtual bool                IsCynoSE()              { return true; }
    /* Base */
    virtual bool                isGlobal()              { return true; }
    /* SystemEntity interface */
    //virtual void                EncodeDestiny(Buffer& into);
    //virtual PyDict*             MakeSlimItem();
};

class DungeonEditSE
: public FieldSystemEntity
{
public:
    DungeonEditSE() =delete;
    // main c'tor
    DungeonEditSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system, Dungeon::RoomObject data);
    // copy c'tor
    DungeonEditSE(const DungeonEditSE* oth) =delete;
    // move c'tor
    DungeonEditSE(DungeonEditSE&& oth) noexcept =default;
    // copy assignment
    DungeonEditSE& operator=(DungeonEditSE& oth) =delete;
    // move assignment
    DungeonEditSE& operator=(DungeonEditSE&& oth) =default;
    virtual ~DungeonEditSE()                            { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual DungeonEditSE*      GetDungeonEditSE()      { return this; }
    /* class type tests. */
    /* Base */
    virtual bool                IsDungeonEditSE()       { return true; }
    Dungeon::RoomObject         GetData()               { return m_data; }
    /* SystemEntity interface */
    //virtual void                EncodeDestiny( Buffer& into );
    virtual PyDict*             MakeSlimItem();

private:
    Dungeon::RoomObject m_data;
};



/* ISE - Non-Static, Mobile, Non-Destructible
 * - Containers, DeadSpace
 * - no TargetMgr, has DestinyMgr
 */
class ItemSystemEntity : public SystemEntity {
public:
    // default c'tor
    ItemSystemEntity() =delete;
    // main c'tor
    ItemSystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    // copy c'tor
    ItemSystemEntity(const ItemSystemEntity* oth) =delete;
    // move c'tor
    ItemSystemEntity(ItemSystemEntity&& oth) noexcept =default;
    // copy assignment
    ItemSystemEntity& operator=(ItemSystemEntity& oth) =delete;
    // move assignment
    ItemSystemEntity& operator=(ItemSystemEntity&& oth) =default;
    // d'tor
    virtual ~ItemSystemEntity()                         { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual const char*         GetSEType()             { return "Item SE"; }
    virtual ItemSystemEntity*   GetItemSE()             { return this; }
    /* class type tests. */
    virtual bool                IsSystemEntity()        { return false; }
    virtual bool                IsItemEntity()          { return true; }
    /* Base */
    //virtual bool                isGlobal()              { return false; }
    virtual bool                IsInanimateSE()         { return true; }
    /* SystemEntity interface */
    virtual void                EncodeDestiny(Buffer& into);
    virtual void                MakeDamageState(DoDestinyDamageState &into);
    virtual PyDict*             MakeSlimItem();

private:
    uint16 m_keyType;           //TrainingComplex/Deadspace Passkey   (group - Acceleration_Gate_Keys)
};


/* OSE - Non-Static, Non-Mobile, Destructible
 * - POS Structures, Outposts, Deployables, empty Ships, Asteroids
 * - has TargetMgr,  no DestinyMgr
 */
class ObjectSystemEntity : public SystemEntity {
public:
    // default c'tor
    ObjectSystemEntity() =delete;
    // main c'tor
    ObjectSystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    // copy c'tor
    ObjectSystemEntity(const ObjectSystemEntity* oth) =delete;
    // move c'tor
    ObjectSystemEntity(ObjectSystemEntity&& oth) noexcept =default;
    // copy assignment
    ObjectSystemEntity& operator=(ObjectSystemEntity& oth) =delete;
    // move assignment
    ObjectSystemEntity& operator=(ObjectSystemEntity&& oth) =default;
    // d'tor
    virtual ~ObjectSystemEntity();

    /* class type pointer querys. */
    virtual const char*         GetSEType()             { return "Object SE"; }
    virtual ObjectSystemEntity* GetObjectSE()           { return this; }
    /* class type tests. */
    virtual bool                IsSystemEntity()        { return false; }
    virtual bool                IsObjectEntity()        { return true; }
    /* Base */
    //virtual bool                isGlobal()              { return false; }
    virtual bool                IsInanimateSE()         { return true; }
    /* SystemEntity interface */
    virtual void                UpdateDamage();
    virtual void                EncodeDestiny(Buffer& into);
    virtual void                MakeDamageState(DoDestinyDamageState &into);
    virtual PyDict*             MakeSlimItem();
    /* virtual functions default to base class and overridden as needed */
    virtual void                Killed(Damage &damage);
    virtual bool                IsInvul()               { return m_invul; }

    /* specific functions handled here. */
    void                    SetInvul(bool invul=false)  { m_invul = invul; }

private:
    bool m_invul;
};

/* Mobile Warp Disruptors */
class DeployableSE
: public ObjectSystemEntity
{
public:
    // default c'tor
    DeployableSE() =delete;
    // main c'tor
    DeployableSE(InventoryItemRef self, PyServiceMgr& services, SystemManager* system, const FactionData& data);
    // copy c'tor
    DeployableSE(const DeployableSE* oth) =delete;
    // move c'tor
    DeployableSE(DeployableSE&& oth) noexcept =default;
    // copy assignment
    DeployableSE& operator=(DeployableSE& oth) =delete;
    // move assignment
    DeployableSE& operator=(DeployableSE&& oth) =default;
    // d'tor
    virtual ~DeployableSE()                             { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual const char*         GetSEType()             { return "Deployable SE"; }
    virtual DeployableSE*       GetDeployableSE()       { return this; }
    /* class type tests. */
    virtual bool                IsSystemEntity()        { return false; }
    virtual bool                IsDeployableSE()        { return true; }
};


/* DSE - Non-Static, Mobile, Destructible
 * - Drones, Ships, Missiles, Wrecks
 * - has TargetMgr, has DestinyMgr
 */
class DynamicSystemEntity : public SystemEntity {
public:
    // default c'tor
    DynamicSystemEntity() =delete;
    // main c'tor
    DynamicSystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system);
    // copy c'tor
    DynamicSystemEntity(const DynamicSystemEntity* oth) =delete;
    // move c'tor
    DynamicSystemEntity(DynamicSystemEntity&& oth) noexcept =default;
    // copy assignment
    DynamicSystemEntity& operator=(DynamicSystemEntity& oth) =delete;
    // move assignment
    DynamicSystemEntity& operator=(DynamicSystemEntity&& oth) =default;
    // d'tor
    virtual ~DynamicSystemEntity();

    /* class type pointer querys. */
    virtual const char*         GetSEType()             { return "Dynamic SE"; }
    virtual DynamicSystemEntity* GetDynamicSE()         { return this; }
    /* class type tests. */
    virtual bool                IsSystemEntity()        { return false; }
    virtual bool                IsDynamicEntity()       { return true; }
    /* Base */
    //virtual bool                isGlobal()              { return false; }
    /* SystemEntity interface */
    virtual void                UpdateDamage();
    virtual void                EncodeDestiny(Buffer& into);
    virtual void                MakeDamageState(DoDestinyDamageState &into);
    virtual PyDict*             MakeSlimItem();
    /* virtual functions default to base class and overridden as needed */
    virtual bool                Load()                  { return true; }
    virtual bool                IsInvul()               { return m_invul; }
    virtual bool                IsFrozen()              { return m_frozen; }

    /* specific functions handled here. */
    void                        AwardBounty(Client* pClient);
    void                    SetInvul(bool invul=false)  { m_invul = invul; }
    void                   SetFrozen(bool frozen=false) { m_frozen = frozen; }

private:
    bool m_invul;
    bool m_frozen;
};


#endif  // _EVE_SERVER_SYSTEM_ENTITY_H_
