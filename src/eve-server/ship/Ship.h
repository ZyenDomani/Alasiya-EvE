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
*/

#ifndef __SHIP__H__INCL__
#define __SHIP__H__INCL__

#include "inventory/ItemType.h"
#include "inventory/Inventory.h"
#include "inventory/InventoryItem.h"
#include "system/SystemEntity.h"
#include "ship/modules/ModuleManager.h"
#include "ship/modules/GenericModule.h"
#include "ship/ShipDB.h"

/**
 * Basic container for raw ship type data.
 */
class ShipTypeData {
public:
    ShipTypeData(
        uint32 _weaponTypeID = 0,
        uint32 _miningTypeID = 0,
        uint32 _skillTypeID = 0
    );

    // Content:
    uint32 mWeaponTypeID;
    uint32 mMiningTypeID;
    uint32 mSkillTypeID;
};

/**
 * Class managing ship type data.
 */
class ShipType
: public ItemType
{
    friend class ItemType; // to let them construct us
public:
    /**
     * Loads ship type.
     *
     * @param[in] factory
     * @param[in] shipTypeID ID of ship type to load.
     * @return Pointer to new ShipType object; NULL if failed.
     */
    static ShipType *Load(ItemFactory &factory, uint32 shipTypeID);

    /*
     * Access methods:
     */
    const ItemType *weaponType() const { return m_weaponType; }
    const ItemType *miningType() const { return m_miningType; }
    const ItemType *skillType() const { return m_skillType; }

protected:
    ShipType(
        uint32 _id,
        // ItemType stuff:
        const ItemGroup &_group,
        const TypeData &_data,
        // ShipType stuff:
        const ItemType *_weaponType,
        const ItemType *_miningType,
        const ItemType *_skillType,
        const ShipTypeData &stData
    );

    /*
     * Member functions:
     */
    using ItemType::_Load;

    // Template loader:
    template<class _Ty>
    static _Ty *_LoadType(ItemFactory &factory, uint32 shipTypeID,
        // ItemType stuff:
        const ItemGroup &group, const TypeData &data)
    {
        // verify it's a ship
        if( group.categoryID() != EVEDB::invCategories::Ship ) {
            _log( ITEM__ERROR, "Tried to load %u (%s) as a Ship.", shipTypeID, group.category().name().c_str() );
            return NULL;
        }

        // load additional ship type stuff
        ShipTypeData stData;
        if( !factory.db().GetShipType(shipTypeID, stData) )
            return NULL;

        // try to load weapon type
        const ItemType *weaponType = NULL;
        if( stData.mWeaponTypeID != 0 ) {
            weaponType = factory.GetType( stData.mWeaponTypeID );
            if( weaponType == NULL )
                return NULL;
        }

        // try to load mining type
        const ItemType *miningType = NULL;
        if( stData.mMiningTypeID != 0 ) {
            miningType = factory.GetType( stData.mMiningTypeID );
            if( miningType == NULL )
                return NULL;
        }

        // try to load skill type
        const ItemType *skillType = NULL;
        if( stData.mSkillTypeID != 0 ) {
            skillType = factory.GetType( stData.mSkillTypeID );
            if( skillType == NULL )
                return NULL;
        }

        // continue with load
        return _Ty::template _LoadShipType<_Ty>( factory, shipTypeID, group, data, weaponType, miningType, skillType, stData );
    }

    // Actual loading stuff:
    template<class _Ty>
    static _Ty *_LoadShipType(ItemFactory &factory, uint32 shipTypeID,
        // ItemType stuff:
        const ItemGroup &group, const TypeData &data,
        // ShipType stuff:
        const ItemType *weaponType, const ItemType *miningType, const ItemType *skillType, const ShipTypeData &stData
    );

    /*
     * Data content:
     */
    const ItemType *m_weaponType;
    const ItemType *m_miningType;
    const ItemType *m_skillType;
};

/**
 * InventoryItem which represents ShipItem.
 */

class ShipItem
: public InventoryItem
{
    friend class InventoryItem;    // to let it construct us

protected:
    ShipItem(
        ItemFactory &_factory,
        uint32 _shipID,
        // InventoryItem stuff:
        const ShipType &_shipType,
        const ItemData &_data
    );
    virtual ~ShipItem();

public:
    void Init();
    void InitPod();
    static ShipItemRef Load(ItemFactory &factory, uint32 shipID);
    static ShipItemRef Spawn(ItemFactory &factory, ItemData &data);

    virtual void SetPlayer(Client* pClient);
    virtual bool HasPilot()                                     { return (m_pilot ? true : false); }
    virtual Client* GetPilot()                                  { return m_pilot; }

    bool HasModuleManager()                                     { return (m_ModuleManager ? true : false); }
    ModuleManager* GetModuleManager()                           { return m_ModuleManager; }

    void Delete();

    double GetRemainingVolumeByFlag(EVEItemFlags flag) const;
    bool ValidateAddItem(EVEItemFlags flag, InventoryItemRef item);
    bool ValidateItemSpecifics(InventoryItemRef equip);

    const ShipType & type() const { return static_cast<const ShipType &>(InventoryItem::type()); }

    bool IsInvul() {return false;}      /** @todo finish this, and find what it's used for */

    std::string GetShipDNA();

    /*
     * Primary public packet builders:
     */
    PyDict* ShipGetInfo();
    PyDict* GetShipInfo();
    PyDict* GetShipState();
    PyList* ShipGetModuleList();
    PyDict* GetChargeState();

    /*
     * Validates boarding ship
     */
    bool ValidateBoardShip(ShipItemRef ship, CharacterRef who);

    /*
     * Saves the ship state
     */
    void SaveShip();

    /*
     * Inform Ship that a state change is taking place
     */
    void Dock();
    void Heal();
    void Jump();
    void Warp();
    void Undock();
    void AddModuleToOnlineVec(uint32 moduleID);

    /* begin new module manager interface */
    void ProcessModules();
    InventoryItemRef GetModule(EVEItemFlags flag);
    InventoryItemRef GetModule(uint32 itemID);
    EVEItemFlags FindAvailableModuleSlot( InventoryItemRef item );
    EvilNumber GetMaxTurrentHardpoints() { return GetAttribute(AttrTurretSlotsLeft); }
    EvilNumber GetMaxLauncherHardpoints() { return GetAttribute(AttrLauncherSlotsLeft); }
    uint32 AddItem( EVEItemFlags flag, InventoryItemRef item);
    void AddItem(InventoryItemRef item);
    void RemoveItem( InventoryItemRef item/*, uint32 inventoryID, EVEItemFlags flag*/ );
    void UpdateModules();
    void UpdateModules(EVEItemFlags flag);
    void UnloadModule(uint32 itemID);
    void UnloadAllModules();
    void MoveModuleSlot(EVEItemFlags slot1, EVEItemFlags slot2);
    void RepairModules();
    void Online(uint32 moduleID);
    void Offline(uint32 moduleID);
    void Activate(int32 itemID, std::string effectName, int32 targetID, int32 repeat);
    void Deactivate(int32 itemID, std::string effectName);
    void Overload();
    void CancelOverloading();
    void ReplaceCharges(EVEItemFlags flag, InventoryItemRef newCharge);
    void RemoveRig(InventoryItemRef item);
    void DeactivateAllModules();
    void OnlineAll();
    void OfflineAll();
    void StripFitting();

    // Tactical Interface:
    void SetShipShield(double fraction);
    void SetShipArmor(double fraction);
    void SetShipHull(double fraction);
    void SetShipCapacitorLevel(double fraction);
    double GetShipHullHP() { return GetAttribute(AttrHP).get_float(); }
    double GetShipArmorHP() { return GetAttribute(AttrArmorHP).get_float(); }
    double GetShipPGLevel() { return GetAttribute(AttrPowerOutput).get_float(); }
    double GetShipCPULevel() { return GetAttribute(AttrCpuOutput).get_float(); }
    double GetShipShieldHP() { return GetAttribute(AttrShieldCharge).get_float(); }
    double GetShipCapacitorLevel() { return GetAttribute(AttrCapacitorCharge).get_float(); }
    EvilNumber GetShipHullPercent() { return 1 -(GetAttribute(AttrDamage) / GetAttribute(AttrHP)); }
    EvilNumber GetShipCPUPercent() { return 1 -(GetAttribute(AttrCpuLoad) / GetAttribute(AttrCpuOutput)); }
    EvilNumber GetShipPGPercent() { return 1 -(GetAttribute(AttrPowerLoad) / GetAttribute(AttrPowerOutput)); }
    EvilNumber GetShipArmorPercent() { return 1 -(GetAttribute(AttrArmorDamage) / GetAttribute(AttrArmorHP)); }
    EvilNumber GetShipShieldPercent() { return (GetAttribute(AttrShieldCharge) / GetAttribute(AttrShieldCapacity)); }
    EvilNumber GetShipCapacitorPercent() { return (GetAttribute(AttrCapacitorCharge) / GetAttribute(AttrCapacitorCapacity)); }

    void UpdateHoldsUsedVolume();

    // External Methods For use by hostile entities directing effects to this entity:
    int32 ApplyRemoteEffect() { assert(true); }     // DO NOT CALL THIS YET!!!  This function needs to call down to ModuleManager::ApplyRemoteEffect with the proper argument list.
    int32 RemoveRemoteEffect() { assert(true); }    // DO NOT CALL THIS YET!!!  This function needs to call down to ModuleManager::RemoveRemoteEffect with the proper argument list.

    using InventoryItem::_Load;
    virtual bool _Load();

protected:
    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem(ItemFactory &factory, uint32 shipID, const ItemType &type, const ItemData &data) {
        if( type.categoryID() != EVEDB::invCategories::Ship ) {
            _log( ITEM__ERROR, "Trying to load %s as ShipItem.", type.category().name().c_str() );
            return RefPtr<_Ty>();
        }

        const ShipType &shipType = static_cast<const ShipType &>( type );
        return _Ty::template _LoadShip<_Ty>( factory, shipID, shipType, data );
    }

    // Actual loading stuff:
    template<class _Ty>
    static RefPtr<_Ty> _LoadShip(ItemFactory &factory, uint32 shipID,
                                 // InventoryItem stuff:
                                 const ShipType &shipType, const ItemData &data
    );

    //bool LoadAttributes();
    bool m_IsLoaded;

    static uint32 CreateItemID(ItemFactory &factory, ItemData &data);

    void _IncreaseCargoHoldsUsedVolume(EVEItemFlags flag, double volumeToConsume);  // To release cargo space, make 'volumeToConsume' negative
    void _DecreaseCargoHoldsUsedVolume(EVEItemFlags flag, double volumeToConsume);  // To release cargo space, make 'volumeToConsume' negative

private:
    Client* m_pilot;

    //the ship's module manager.  We own this
    ModuleManager* m_ModuleManager;

    std::vector<uint32> m_onlineModuleVec;
};

/**
 * DynamicSystemEntity which represents ship object in space
 */
class PyServiceMgr;
class InventoryItem;
class DestinyManager;
class SystemManager;
class ServiceDB;

class Ship
: public DynamicSystemEntity
{
public:
    Ship(InventoryItemRef self, PyServiceMgr& services, SystemManager* pSystem);
    virtual ~Ship();

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
    virtual void Killed(Damage &fatal_blow);    /* This method is defined in Damage.cpp */

    /* virtual functions in base to allow common interface calls specific to ship entities */
    virtual void SetPilot(Client* pClient);
    virtual bool HasPilot()                             { return (m_pilot ? true : false); }
    virtual Client* GetPilot()                          { return m_pilot; }

    /* specific functions handled here. */
    void PayInsurance();
    void ResetShipSystemMgr(SystemManager* pSystem);
    void SetPodShipID(uint32 shipID)                    { m_podShipID = shipID; }

    uint32 GetPodShipID()                               { return m_podShipID; }

    double CalculateRechargeRate(double Capacity, double RechargeTimeMS, double Current);

protected:
    Client* m_pilot;
    ShipItemRef m_shipRef;

    const uint32 m_processTimerTick;

private:
    ShipDB m_db;

    Timer m_processTimer;

    uint32 m_podShipID;

};

#endif /* !__SHIP__H__INCL__ */


