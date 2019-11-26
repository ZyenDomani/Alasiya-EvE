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
    Author:        Bloody.Rabbit
    Updates:    Allan
*/

#ifndef __SHIP__H__INCL__
#define __SHIP__H__INCL__


#include "effects/EffectsData.h"
#include "fleet/FleetData.h"
#include "inventory/ItemType.h"
#include "inventory/InventoryItem.h"
#include "ship/ShipDB.h"
#include "ship/modules/ModuleManager.h"
#include "system/SystemEntity.h"

/**
 * InventoryItem which represents ShipItem.
 */
class Client;
class GenericModule;

class ShipItem
: public InventoryItem
{
    friend class InventoryItem;    // to let it construct us

protected:
    ShipItem(uint32 shipID, const ItemType &type, const ItemData &data);
    ~ShipItem() noexcept;

public:
    /* class type pointer querys. */
    virtual ShipItem* GetShipItem()                     { return this; }
    /* class type tests. */
    virtual bool IsShipItem()                           { return true; }

    void Init();
    void InitPod();
    void InitAttribs();
    void LogOut();
    static ShipItemRef Load( uint32 shipID);
    static ShipItemRef Spawn( ItemData &data);

    virtual void SetPlayer(Client* pClient);
    virtual bool HasPilot()                             { return (m_pilot != nullptr); }
    virtual Client* GetPilot()                          { return m_pilot; }

    virtual void AddItem(InventoryItemRef iRef);
    virtual void RemoveItem( InventoryItemRef iRef);

    bool HasModuleManager()                             { return (m_ModuleManager != nullptr); }
    ModuleManager* GetModuleManager()                   { return m_ModuleManager; }

    virtual void Delete();

    double GetRemainingVolumeByFlag(EVEItemFlags flag) const;
    // this checks destination flag vs item type/group/category for proper placement
    bool ValidateAddItem(EVEItemFlags flag, InventoryItemRef iRef, Client* pClient=nullptr);     // this cannot throw.  must return bool
    bool ValidateItemSpecifics(InventoryItemRef iRef);

  //  const ShipType & type() const                       { return static_cast<const ShipType &>(InventoryItem::type()); }

    bool IsPopped()                                     { return m_isPopped; }
    void SetPopped(bool set=false)                      { m_isPopped = set; }
    std::string GetShipDNA();

    /*
     * Primary public packet builders:
     */
    PyDict* ShipGetInfo();
    PyDict* GetShipInfo();
    PyDict* GetShipState();
    PyList* ShipGetModuleList();
    PyDict* GetChargeState();

    bool ValidateBoardShip(CharacterRef who);
    void SaveShip();
    void RepairShip(float fraction);

    /* Inform Ship that a state change is taking place  */
    void Dock();
    void Heal();
    void Jump();
    void Warp();
    void Eject();
    void Undock();
    void AddModuleToOnlineVec(uint32 modID);

    /* begin new module manager interface */
    void ProcessModules();
    //this can throw
    void Online(uint32 modID);
    //this can throw
    void Offline(uint32 modID);
    void OnlineAll();
    void OfflineAll();
    void Activate(int32 itemID, std::string effectName, int32 targetID, int32 repeat);
    void Deactivate(int32 itemID, std::string effectName) { m_ModuleManager->Deactivate(itemID, effectName); }
    void DeactivateAllModules();
    void Overload(uint32 itemID)                        { m_ModuleManager->Overload(itemID); }
    void CancelOverloading(uint32 itemID)               { m_ModuleManager->DeOverload(itemID); }
    void ReplaceCharges(EVEItemFlags flag, InventoryItemRef newCharge);
    void RemoveRig(InventoryItemRef iRef);
    void UpdateModules();
    void UpdateModules(EVEItemFlags flag);
    void UnloadModule(uint32 itemID)                    { m_ModuleManager->UnloadModule(itemID); }
    void UnloadAllModules()                             { m_ModuleManager->UnloadAllModules();  }
    void MoveModuleSlot(EVEItemFlags slot1, EVEItemFlags slot2);
    void StripFitting();
    // this will remove all items from all ship cargo holds
    void EmptyCargo();

    void RepairModules(std::vector<InventoryItemRef>& itemRefVec, float fraction);

    void RepairModules()                                { m_ModuleManager->RepairModules(); }

    void AbortCycle()                                   { m_ModuleManager->AbortCycle(); }
    bool IsDocking()                                    { return m_isDocking; }
    bool IsUndocking()                                  { return m_isUndocking; }
    void SetDocked()                                    { m_isDocking = false; }
    void SetUndocking(bool set=false)                   { m_isUndocking = set; }
    InventoryItemRef GetTargetRef()                     { return m_targetRef; }
    void ClearTargetRef()                               { m_targetRef = InventoryItemRef(); }
    // this is for repairing modules with nanite paste
    PyRep* ModuleRepair(uint32 modID)                   { return m_ModuleManager->ModuleRepair(modID); }
    void StopModuleRepair(uint32 modID)                 { m_ModuleManager->StopModuleRepair(modID); }

    // OL and Heat damage shit
    void ProcessHeat();
    // heat generated per tic by ship's active modules (passive *may not* generate heat yet.)
    float GenerateHeat(uint16 attrID);
    // heat dissipated per tic by ship's design and pilots skills
    float DissipateHeat(uint16 attrID, float heat);
    // this method does just that....checks for heat-induced damage from overheated modules
    void HeatDamageCheck(GenericModule* pMod);
    void DamageModule(uint32 modID, float amt=1)        { m_ModuleManager->DamageModule(modID, amt); }
    void DamageRandModule()                             { m_ModuleManager->DamageRandModule(); }

    void GetModuleRefVec(std::vector<InventoryItemRef>& iRefVec);
    GenericModule* GetModule(EVEItemFlags flag)         { return m_ModuleManager->GetModule(flag); }
    InventoryItemRef GetModuleRef(EVEItemFlags flag);
    InventoryItemRef GetModuleRef(uint32 modID);

    // this also checks for fit-by-type and rig restrictions
    void TryModuleLimitChecks(EVEItemFlags flag, InventoryItemRef iRef);     // this must throw on error
    EVEItemFlags FindAvailableModuleSlot( InventoryItemRef iRef );
    // make sure this does NOT throw.
    // must return integer
    // will remove item from previous container
    uint32 AddItem( EVEItemFlags flag, InventoryItemRef iRef, Client* pClient=nullptr);
    // this can throw. returns nothing
    void LoadCharge(InventoryItemRef cRef, EVEItemFlags flag);
    // this can throw. returns nothing
    void LoadChargesToBank(EVEItemFlags flag, std::vector<int32>& chargeIDs);
    uint32 RemoveCharge(EVEItemFlags fromFlag, EVEItemFlags toFlag, bool merge=false);
    /* end new module manager interface */

    // Tactical Interface:
    void SetShipShield(float fraction);
    void SetShipArmor(float fraction);
    void SetShipHull(float fraction);
    void SetShipCapacitorLevel(float fraction);
    float GetShipHullHP()                               { return GetAttribute(AttrHP).get_float(); }
    float GetShipArmorHP()                              { return GetAttribute(AttrArmorHP).get_float(); }
    float GetShipPGLevel()                              { return GetAttribute(AttrPowerOutput).get_float(); }
    float GetShipCPULevel()                             { return GetAttribute(AttrCpuOutput).get_float(); }
    float GetShipShieldHP()                             { return GetAttribute(AttrShieldCharge).get_float(); }
    float GetShipCapacitorLevel()                       { return GetAttribute(AttrCapacitorCharge).get_float(); }
    EvilNumber GetShipHullPercent()                     { return 1 -(GetAttribute(AttrDamage) / GetAttribute(AttrHP)); }
    EvilNumber GetShipCPUPercent()                      { return 1 -(GetAttribute(AttrCpuLoad) / GetAttribute(AttrCpuOutput)); }
    EvilNumber GetShipPGPercent()                       { return 1 -(GetAttribute(AttrPowerLoad) / GetAttribute(AttrPowerOutput)); }
    EvilNumber GetShipArmorPercent()                    { return 1 -(GetAttribute(AttrArmorDamage) / GetAttribute(AttrArmorHP)); }
    EvilNumber GetShipShieldPercent()                   { return (GetAttribute(AttrShieldCharge) / GetAttribute(AttrShieldCapacity)); }
    EvilNumber GetShipCapacitorPercent()                { return (GetAttribute(AttrCapacitorCharge) / GetAttribute(AttrCapacitorCapacity)); }

    // template loading system
    using InventoryItem::_Load;
    virtual bool _Load();
protected:
    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem( uint32 shipID, const ItemType &type, const ItemData &data) {
        if (type.categoryID() != EVEDB::invCategories::Ship) {
            _log( ITEM__ERROR, "Trying to load %s(%u) as ShipItem.", type.category().name().c_str(), shipID);
            if (sConfig.server.StackTrace)
                EvE::traceStack();
            return RefPtr<_Ty>();
        }

       // const ShipType &shipType = static_cast<const ShipType &>( type );
        return ShipItemRef( new ShipItem(shipID, type, data ));
    }

    //bool LoadAttributes();
    bool m_loaded;

    static uint32 CreateItemID( ItemData &data);

public:
    /* new effects system */
    void RemoveEffects();
    void UpdateEffects();
    void CharacterBoardingShip()                        { m_ModuleManager->CharacterBoardingShip(); }

    /* linking weapons methods */
    uint8 GetLinkedCount(GenericModule* pMod);
    bool HasLinkedWeapons()                             { return (!m_linkedWeapons.empty()); }
    void LinkAllWeapons();
    void LinkWeapon(uint32 masterID, uint32 slaveID);       // this should throw if applicable
    void MergeModuleGroups(uint32 masterID, uint32 slaveID);  // this should throw if applicable
    uint32 UnlinkWeapon(uint32 moduleID);
    void UnlinkGroup(uint32 memberID);
    void UnlinkAllWeapons();
    PyRep* GetLinkedWeapons();
    void OfflineGroup(GenericModule* pMod);
    void DamageGroup(GenericModule* pMod);
    // to load with ammo
    void LoadLinkedWeapons(InventoryItemRef cRef, GenericModule* pMod);
    void LoadLinkedWeapons(GenericModule* pMod, std::vector<int32>& chargeIDs);

protected:
    /* linking weapons methods */
    void LinkWeapon(GenericModule* pMaster, GenericModule* pSlave);
    void UnlinkWeapon(uint32 masterID, uint32 slaveID);
    // to load saved linked weapons
    void LoadLinkedWeapons();
    // to save linked weapons
    void SaveLinkedWeapons();
    // this single iteration loop will link same type weapons, then remove the weapon from the list.
    //  non-linked weapons are kept in list to be checked upon return to caller.
    void LinkWeaponLoop(std::list< GenericModule* >& moduleVec);

private:
    Client* m_pilot;

    //the ship's module manager.  We own this
    ModuleManager* m_ModuleManager;

    InventoryItemRef m_targetRef;       // this is only used for module effects that require a target.  is here because of the ease of aquiring/sending (common code)

    std::vector<uint32> m_onlineModuleVec;      // for onlining modules when undocking
    std::map<GenericModule*, std::list<GenericModule*>> m_linkedWeapons; // masterID/data (slaveIDs)

    void ProcessEffects(bool add=false, bool update=false);
    void ProcessShipEffects(bool update=false);

    bool m_isPopped;
    bool m_isDocking;
    bool m_isUndocking;
};

/**
 * DynamicSystemEntity which represents ship object in space
 */
class PyServiceMgr;
class InventoryItem;
class DestinyManager;
class Missile;
class SystemManager;
class ServiceDB;

class Ship
: public DynamicSystemEntity
{
public:
    Ship(InventoryItemRef self, PyServiceMgr& services, SystemManager* pSystem, const FactionData& data);
    virtual ~Ship()                                     { /* do nothing here */ }

    /* class type pointer querys. */
    virtual Ship* GetShipSE()                           { return this; }
    /* class type tests. */
    virtual bool IsShipSE()                             { return true; }

    /* SystemEntity interface */
    virtual void Process();
    virtual void EncodeDestiny( Buffer& into );
    virtual void MakeDamageState(DoDestinyDamageState &into);
    virtual PyDict *MakeSlimItem();

    /* virtual functions default to base class and overridden as needed */
    virtual void Killed(Damage &fatal_blow);            /* This method is defined in Damage.cpp */

    /* virtual functions to be overridden in derived classes */
    virtual void MissileLaunched(Missile* pMissile)     { /* Do nothing here */ }
    virtual bool IsInvul()                              { return false; }      /** @todo finish this. client IsInvul coded. */
    virtual bool IsFrozen()                             { return false; }
    virtual bool IsLogin()                              { return false; }      /** @todo finish this. client IsLogin coded. */

    /* virtual functions in base to allow common interface calls specific to ship entities */
    virtual void SetPilot(Client* pClient);
    virtual bool HasPilot()                             { return ((m_shipRef.get() == nullptr) ? false : m_shipRef->HasPilot()); }
    virtual Client* GetPilot()                          { return ((m_shipRef.get() == nullptr) ? nullptr : m_shipRef->GetPilot()); }

    /* specific functions handled here. */
    void SetPassword(std::string pass)                  { m_towerPass = pass; }
    // fleet
    void ClearBoostData();
    bool IsBoosted()                                    { return m_boosted; }
    void SetBoost(bool set=false)                       { m_boosted = set; }
    void RemoveBoost();
    void ApplyBoost(BoostData& bData);
    uint8 GetMiningBoostAmount()                        { return m_boost.mining; }

    // misc
    void PayInsurance();
    void DamageModule(uint32 modID)                     { m_shipRef->DamageModule(modID); }
    void DamageRandModule(float chance);
    void ResetShipSystemMgr(SystemManager* pSystem);    // this is to reset system manager for jumps, etc.
    void SaveShip()                                     { m_shipRef->SaveShip(); }

    //cancel all active modules
    void AbortCycle()                                   { m_shipRef->AbortCycle(); }

    // this is the itemID of the ship that ejected the pod, for setting in slimItemUpdate (launcherID)
    void SetLauncherID(uint32 shipID)                   { m_podShipID = shipID; }
    uint32 GetLauncherID()                              { return m_podShipID; }

    ShipItemRef GetShipItemRef()                        { return m_shipRef; }

    double CalculateRechargeRate(double Capacity, double RechargeTimeMS, double Current);

protected:
    ShipItemRef m_shipRef;

    const uint32 m_processTimerTick;

private:
    ShipDB m_db;

    Timer m_processTimer;

    uint32 m_podShipID;

    /*  boost data */
    BoostData m_boost;
    bool m_boosted;
    uint16 m_oldArmor;
    uint16 m_oldShield;
    uint16 m_oldScanRes;
    uint32 m_oldTargetRange;
    double m_oldInertia;

    /*  for POS field */
    std::string m_towerPass;

};

#endif /* !__SHIP__H__INCL__ */
