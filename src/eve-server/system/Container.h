/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2008 The EVEmu Team
    For the latest information visit http://evemu.mmoforge.org
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
    Author:        Aknor Jaden
    Updates:    Allan
*/

#ifndef __CONTAINER__H__INCL__
#define __CONTAINER__H__INCL__

#include "inventory/AttributeEnum.h"
#include "inventory/Inventory.h"
#include "inventory/InventoryItem.h"
#include "system/SystemEntity.h"

/**
 * InventoryItem which represents cargo container.
 */
class CargoContainer
: public InventoryItem
{
    friend class InventoryItem;    // to let it construct us
public:
    CargoContainer(
        ItemFactory &_factory,
        uint32 _containerID,
        // InventoryItem stuff:
        const ItemType &_containerType,
        const ItemData &_data
    );
    virtual ~CargoContainer()                           { /* Do nothing here */ }

    /**
     * Loads CargoContainer from DB.
     *
     * @param[in] factory
     * @param[in] containerID ID of container to load.
     * @return Pointer to CargoContainer object; NULL if failed.
     */
    static CargoContainerRef Load(ItemFactory &factory, uint32 containerID);
    /**
     * Spawns new CargoContainer.
     *
     * @param[in] factory
     * @param[in] data Item data for CargoContainer.
     * @return Pointer to new CargoContainer object; NULL if failed.
     */
    static CargoContainerRef Spawn(ItemFactory &factory, ItemData &data);

    /*
     * Primary public interface:
     */
    void Delete();

    double GetCapacity(EVEItemFlags flag) const;
    /*
     * _ExecAdd validation interface:
     */
    void ValidateAddItem(EVEItemFlags flag, InventoryItemRef item) const;

    void AddItem(InventoryItemRef item);
    void RemoveItem(InventoryItemRef item);

    bool IsAnchored()                                   { return m_isAnchored; }
    void SetAnchor(bool set=false)                      { m_isAnchored = set; }

    /*
     * Public fields:
     */
    const ItemType &type() const                        { return InventoryItem::type(); }

    /*
     * Primary public packet builders:
     */
    PyObject *CargoContainerGetInfo();

    virtual void MakeDamageState(DoDestinyDamageState &into) const;

    bool IsEmpty()                                      { return GetInventory()->IsEmpty(); }

protected:
    bool m_isAnchored;

    /*
     * Member functions:
     */
    using InventoryItem::_Load;
    virtual bool _Load();

    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem(ItemFactory &factory, uint32 containerID, const ItemType &type, const ItemData &data) {
        if( (type.groupID() != EVEDB::invGroups::Cargo_Container)
            && (type.groupID() != EVEDB::invGroups::Audit_Log_Secure_Container)
            && (type.groupID() != EVEDB::invGroups::Freight_Container)
            && (type.groupID() != EVEDB::invGroups::Secure_Cargo_Container)
            && (type.groupID() != EVEDB::invGroups::Spawn_Container) )
        {
            _log( ITEM__ERROR, "CargoContainer::_LoadItem()  Trying to load category=%s, group=%s as CargoContainer.", type.category().name().c_str(), type.group().name().c_str() );
            return RefPtr<_Ty>();
        }
        return _Ty::template _LoadCargoContainer<_Ty>( factory, containerID, type, data );
    }

    // Actual loading stuff:
    template<class _Ty>
    static RefPtr<_Ty> _LoadCargoContainer(ItemFactory &factory, uint32 containerID, const ItemType &itemType, const ItemData &data);

    static uint32 CreateItemID(ItemFactory &factory, ItemData &data);

    virtual PyRep* GetItem() const                      { return GetItemRow(); }

};


/**
 * ContainerEntity which represents container object in space
 */
class PyServiceMgr;
class Item;
class DestinyManager;
class SystemManager;
class ServiceDB;

class ContainerSE
: public ItemSystemEntity
{
public:
    ContainerSE(CargoContainerRef self, PyServiceMgr &services, SystemManager *system);
    virtual ~ContainerSE();

    /* class type pointer querys. */
    virtual ContainerSE* GetContSE()                    { return this; }
    /* class type tests. */
    virtual bool IsContainerSE()                        { return true; }

    /* SystemEntity interface */
    virtual void Process();
    virtual void EncodeDestiny(Buffer& into);
    virtual PyDict* MakeSlimItem();
    virtual void MakeDamageState(DoDestinyDamageState &into);

    /* specific functions handled in this class. */
    void AnchorContainer();
    bool IsEmpty()                                      { return _containerRef->IsEmpty(); }


protected:
    CargoContainerRef _containerRef;

    Timer m_deleteTimer;

    double m_shieldCharge;
    double m_armorDamage;
    double m_hullDamage;
};

/**
 * InventoryItem which represents wreck container.
 * Author:  Allan
 */
class WreckContainer
: public InventoryItem
{
    friend class InventoryItem;    // to let it construct us
public:
    WreckContainer(
        ItemFactory &_factory,
        uint32 _containerID,
        // InventoryItem stuff:
        const ItemType &_containerType,
        const ItemData &_data
    );
    virtual ~WreckContainer()                           { /* Do nothing here */ }

    /**
     * Loads WreckContainer from DB.
     *
     * @param[in] factory
     * @param[in] containerID ID of container to load.
     * @return Pointer to WreckContainer object; NULL if failed.
     */
    static WreckContainerRef Load(ItemFactory &factory, uint32 containerID);
    /**
     * Spawns new WreckContainer.
     *
     * @param[in] factory
     * @param[in] data Item data for WreckContainer.
     * @return Pointer to new WreckContainer object; NULL if failed.
     */
    static WreckContainerRef Spawn(ItemFactory &factory, ItemData &data);

    /*
     * Primary public interface:
     */
    void Delete();

    double GetCapacity(EVEItemFlags flag) const;
    /*
     * _ExecAdd validation interface:
     */
    void ValidateAddItem(EVEItemFlags flag, InventoryItemRef item) const;

    void AddItem(InventoryItemRef item);
    void RemoveItem(InventoryItemRef item);
    /*
     * Public fields:
     */
    const ItemType &type() const                        { return InventoryItem::type(); }

    /*
     * Primary public packet builders:
     */
    PyObject *WreckContainerGetInfo();

    bool IsEmpty()                                      { return GetInventory()->IsEmpty(); }
    void MakeSlimItemChange();
    void SetMySE(SystemEntity* pSE)                     { mySE = pSE;}

protected:
    /*
     * Member functions:
     */
    using InventoryItem::_Load;
    virtual bool _Load();

    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem(ItemFactory &factory, uint32 containerID, const ItemType &type, const ItemData &data) {
        if (type.groupID() != EVEDB::invGroups::Wreck) {
            _log( ITEM__ERROR, "WreckContainer::_LoadItem()  Trying to load category=%s, group=%s as Wreck.", type.category().name().c_str(), type.group().name().c_str() );
            return RefPtr<_Ty>();
        }
        return _Ty::template _LoadWreck<_Ty>( factory, containerID, type, data );
    }

    // Actual loading stuff:
    template<class _Ty>
    static RefPtr<_Ty> _LoadWreck(ItemFactory &factory, uint32 containerID, const ItemType &itemType, const ItemData &data );

    static uint32 CreateItemID(ItemFactory &factory, ItemData &data );

private:
    SystemEntity* mySE;
};

/**
 * ItemSystemEntity which represents wreck object in space
 * Author:  Allan
 */

class WreckSE
: public ItemSystemEntity
{
public:
    WreckSE(WreckContainerRef self, PyServiceMgr &services, SystemManager *system/*, uint32 launcherID*/);
    virtual ~WreckSE();

    /* class type pointer querys. */
    virtual WreckSE* GetWreckSE()                       { return this; }
    /* class type tests. */
    virtual bool IsWreckSE()                            { return true; }

    /* SystemEntity interface */
    virtual void Process();
    virtual void EncodeDestiny(Buffer& into);
    virtual PyDict* MakeSlimItem();
    void MakeWreckState(DoDestinyDamageState3 &into);

    /* specific functions handled in this class. */
    void SetLaunchedByID(uint32 launcherID)             { m_launchedByID = launcherID; }
    bool IsEmpty()                                      { return _containerRef->IsEmpty(); }

    /** @todo (allan) finish this */
    double GetOwnerBounty()                             { return 0; }

protected:
    WreckContainerRef _containerRef;

    Timer m_deleteTimer;

    uint32 m_launchedByID;

};

#endif /* !__CONTAINER__H__INCL__ */


