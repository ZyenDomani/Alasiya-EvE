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
#include "inventory/ItemType.h"
#include "inventory/InventoryItem.h"
#include "ship/ShipDB.h"
#include "ship/modules/ModuleManager.h"
#include "system/SystemEntity.h"

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
 ** @todo wtf is this for???  delete it.
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
    static _Ty *_LoadShipType(ItemFactory &factory, uint32 shipTypeID, const ItemGroup &group, const TypeData &data,
        const ItemType *weaponType, const ItemType *miningType, const ItemType *skillType, const ShipTypeData &stData)
    {
        return new ShipType(shipTypeID, group, data, weaponType, miningType, skillType, stData );
    }

    /*
     * Data content:
     */
    const ItemType *m_weaponType;
    const ItemType *m_miningType;
    const ItemType *m_skillType;
};

//////////////////////////////////////////////////////////////////////////////////
// Modifier classes containing all data to modify an attribute
/** @todo  wtf is this shit???  */

class Modifier
: public RefObject
{
public:
    Modifier(uint32 originatorID, uint32 targetAttributeID, uint32 targetID, bool penaltiesApply, double modifierValue, uint32 calcTypeID, uint32 revCalcTypeID)
    : RefObject( 0 )
    {
        m_OriginatorID = originatorID;
        m_TargetAttributeID = targetAttributeID;
        m_TargetID = targetID;
        m_bPenaltiesApply = penaltiesApply;
        m_ModifierValue = modifierValue;
        m_CalculationTypeID = calcTypeID;
        m_ReverseCalculationTypeID = revCalcTypeID;
    }

    ~Modifier();

    double GetModifierValue() { return m_ModifierValue; }
    void SetModifierValue(double newModifierValue) { m_ModifierValue = newModifierValue; }
    uint32 GetOriginatorID() { return m_OriginatorID; }

protected:
    uint32 m_OriginatorID;
    uint32 m_TargetAttributeID;
    uint32 m_TargetID;
    bool m_bPenaltiesApply;
    double m_ModifierValue;
    uint32 m_CalculationTypeID;
    uint32 m_ReverseCalculationTypeID;
};

typedef RefPtr<Modifier> ModifierRef;

typedef std::multimap<double, ModifierRef> ModifierMapType;     // The ModifierRef is NOT owned by the owner of instances of this type

class ModifierMap
{
public:
    ModifierMap() { m_MapIsDirty = false; }
    ~ModifierMap();

    bool m_MapIsDirty;
    ModifierMapType m_ModifierMap;   // Key= modifier value, Value= Modifier class object containing all data describing this modifier for this attribute
};

typedef std::map<uint32, ModifierMap *> ModifierMaps;   // Key= attributeID, Value= ModifierMap class object containing a map of all modifiers for this attribute

/////////////////////////////// END MODIFIER /////////////////////////////////////

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
    ShipItem(
        ItemFactory &_factory,
        uint32 _shipID,
        // InventoryItem stuff:
        const ShipType &_shipType,
        const ItemData &_data
    );
    virtual ~ShipItem();

public:
    /* class type pointer querys. */
    virtual ShipItem* GetShipItem()                     { return this; }
    /* class type tests. */
    virtual bool IsShipItem()                           { return true; }

    void Init();
    void InitPod();
    void InitAttribs();
    void LogOut();
    static ShipItemRef Load(ItemFactory &factory, uint32 shipID);
    static ShipItemRef Spawn(ItemFactory &factory, ItemData &data);

    virtual void SetPlayer(Client* pClient);
    virtual bool HasPilot()                             { return (m_pilot ? true : false); }
    virtual Client* GetPilot()                          { return m_pilot; }

    bool HasModuleManager()                             { return (m_ModuleManager ? true : false); }
    ModuleManager* GetModuleManager()                   { return m_ModuleManager; }

    virtual void Delete();

    double GetRemainingVolumeByFlag(EVEItemFlags flag) const;
    bool ValidateAddItem(EVEItemFlags flag, InventoryItemRef iRef);
    bool ValidateItemSpecifics(InventoryItemRef iRef);

    const ShipType & type() const                       { return static_cast<const ShipType &>(InventoryItem::type()); }

    bool IsInvul()                                      { return false; }      /** @todo finish this, and find what it's used for */

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
    void Online(uint32 moduleID);
    void Offline(uint32 moduleID);
    void OnlineAll();
    void OfflineAll();
    void Activate(int32 itemID, std::string effectName, int32 targetID, int32 repeat);
    void Deactivate(int32 itemID, std::string effectName);
    void DeactivateAllModules();
    void Overload();
    void CancelOverloading();
    void ReplaceCharges(EVEItemFlags flag, InventoryItemRef newCharge);
    void RemoveRig(InventoryItemRef iRef);
    void AddItem(InventoryItemRef iRef);
    void RemoveItem( InventoryItemRef iRef, uint32 qty=0/*, uint32 inventoryID, EVEItemFlags flag*/ );
    void UpdateModules();
    void UpdateModules(EVEItemFlags flag);
    void UnloadModule(uint32 itemID);
    void UnloadAllModules();
    void MoveModuleSlot(EVEItemFlags slot1, EVEItemFlags slot2);
    void RepairModules();
    void StripFitting();

    void AbortCycle()                                        { m_ModuleManager->AbortCycle(); }
    bool IsDocking()                                         { return m_isDocking; }
    bool IsUndocking()                                       { return m_isUndocking; }
    void SetUndocking(bool set=false)                        { m_isUndocking = set; }
    InventoryItemRef GetTargetRef()                          { return m_targetRef; }
    void ClearTargetRef()                                    { m_targetRef = InventoryItemRef(); }

    InventoryItemRef GetModuleRef(EVEItemFlags flag);
    InventoryItemRef GetModuleRef(uint32 itemID);
    EVEItemFlags FindAvailableModuleSlot( InventoryItemRef iRef );
    uint32 AddItem( EVEItemFlags flag, InventoryItemRef iRef);
    /* end new module manager interface */

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

    // template loading system
    using InventoryItem::_Load;
    virtual bool _Load();
protected:
    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem(ItemFactory &factory, uint32 shipID, const ItemType &type, const ItemData &data) {
        if (type.categoryID() != EVEDB::invCategories::Ship) {
            _log( ITEM__ERROR, "Trying to load %s as ShipItem.", type.category().name().c_str() );
            if (sConfig.server.StackTrace)
                EvE::traceStack();
            return RefPtr<_Ty>();
        }

        const ShipType &shipType = static_cast<const ShipType &>( type );
        return _Ty::template _LoadShip<_Ty>( factory, shipID, shipType, data );
    }

    // Actual loading stuff:
    template<class _Ty>
    static RefPtr<_Ty> _LoadShip(ItemFactory &factory, uint32 shipID, const ShipType &shipType, const ItemData &data)
    {
        return ShipItemRef( new ShipItem(factory, shipID, shipType, data ));
    }

    //bool LoadAttributes();
    bool m_IsLoaded;

    static uint32 CreateItemID(ItemFactory &factory, ItemData &data);

    void ModifyHoldVolumeByFlag(EVEItemFlags flag, double amount);

    /* new effects system */
public:
    void RemoveEffects();
    void UpdateEffects();
    void CharacterBoardingShip()                        { m_ModuleManager->CharacterBoardingShip(); }

private:
    Client* m_pilot;

    //the ship's module manager.  We own this
    ModuleManager* m_ModuleManager;

    InventoryItemRef m_targetRef;

    std::vector<uint32> m_onlineModuleVec;

    void ProcessEffects(bool add=false, bool update=false);
    void ProcessShipEffects(bool update=false);

    typedef std::map<InventoryItem*, double> iMap;
    std::map<uint16, ShipItem::iMap> m_stackMap;     // stacking attrib storage  attrib, map<InventoryItem*, double>

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
class SystemManager;
class ServiceDB;

class Ship
: public DynamicSystemEntity
{
public:
    Ship(InventoryItemRef self, PyServiceMgr& services, SystemManager* pSystem, const FactionData& data);
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
    virtual bool HasPilot()                             { return (m_shipRef ? m_shipRef->HasPilot() : false); }
    virtual Client* GetPilot()                          { return (m_shipRef ? m_shipRef->GetPilot() : nullptr); }

    /* specific functions handled here. */
    void PayInsurance();
    void SaveShip()                                     { m_shipRef->SaveShip(); }
    void ResetShipSystemMgr(SystemManager* pSystem);    // this is to reset system manager for jumps, etc.

    void AbortCycle()                                   { m_shipRef->AbortCycle(); }
    void SetPodShipID(uint32 shipID)                    { m_podShipID = shipID; }


    ShipItemRef GetShipItemRef()                        { return m_shipRef; }

    uint32 GetPodShipID()                               { return m_podShipID; }

    double CalculateRechargeRate(double Capacity, double RechargeTimeMS, double Current);

protected:
    ShipItemRef m_shipRef;

    const uint32 m_processTimerTick;

private:
    ShipDB m_db;

    Timer m_processTimer;

    uint32 m_podShipID;

};

#endif /* !__SHIP__H__INCL__ */
