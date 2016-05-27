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

/*  NOTE   this is NEW StructureItem and StructureSE code from SE rewrite.  */


#ifndef __STRUCTURE__H__INCL__
#define __STRUCTURE__H__INCL__

#include "inventory/Inventory.h"
#include "system/SystemEntity.h"

// TODO: We may need to create StructureTypeData and StructureType classes just as Ship.h/Ship.cpp
// has in order to load up type data specific to structures.  For now, the generic ItemType class is used.

/**
 * InventoryItem which represents Structure.
 */
class StructureItem
: public InventoryItem
{
    friend class InventoryItem;    // to let it construct us

protected:
    StructureItem(
        ItemFactory &_factory,
        uint32 _structureID,
        // InventoryItem stuff:
        const ItemType &_itemType,
        const ItemData &_data
    );
    virtual ~StructureItem();

public:
    /**
     * Loads Structure from DB.
     *
     * @param[in] factory
     * @param[in] structureID ID of Structure to load.
     * @return Pointer to Structure object; NULL if failed.
     */
    static StructureItemRef Load(ItemFactory &factory, uint32 structureID);
    /**
     * Spawns new Structure.
     *
     * @param[in] factory
     * @param[in] data Item data for Structure.
     * @return Pointer to new Structure object; NULL if failed.
     */
    static StructureItemRef Spawn(ItemFactory &factory, ItemData &data);

    /*
     * Primary public interface:
     */
    void Delete();

    /*
     * _ExecAdd validation interface:
     */
    void ValidateAddItem(EVEItemFlags flag, InventoryItemRef item) const;

    /** @todo (Allan)  will probably need to rewrite all these, too.
     * Public fields:
     */
    const ItemType &    type() const { return InventoryItem::type(); }

    /*
     * Primary public packet builders:
     */
    PyObject *StructureGetInfo();


protected:
    /*
     * Member functions:
     */
    using InventoryItem::_Load;
    virtual bool _Load();


    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem(ItemFactory &factory, uint32 structureID,
        // InventoryItem stuff:
        const ItemType &type, const ItemData &data)
    {
        // check if it's a structure
        if( type.categoryID() != EVEDB::invCategories::Structure )
        {
            _log( ITEM__ERROR, "Trying to load %s as Structure.", type.category().name().c_str() );
            return RefPtr<_Ty>();
        }
        //// cast the type
        //const ItemType &itemType = static_cast<const ItemType &>( type );

        // no additional stuff

        return _Ty::template _LoadStructure<_Ty>( factory, structureID, type, data );
    }

    // Actual loading stuff:
    template<class _Ty>
    static RefPtr<_Ty> _LoadStructure(ItemFactory &factory, uint32 structureID,
        // InventoryItem stuff:
        const ItemType &itemType, const ItemData &data
    );

    static uint32 CreateItemID(ItemFactory &factory,
        // InventoryItem stuff:
        ItemData &data
    );


    void AddItem(InventoryItemRef item);

};


/**
 * ObjectSystemEntity which represents structure object in space
 */

class StructureSE
: public ObjectSystemEntity
{
public:
    StructureSE(StructureItemRef structure, PyServiceMgr &services, SystemManager* system);
    virtual ~StructureSE()                              { /* Do nothing here */ }

    void Init(StructureItemRef structure);

    /* class type pointer querys. */
    virtual StructureSE* GetPOSSE()                     { return (m_pos ? this : nullptr); }
    virtual StructureSE* GetTCUSE()                     { return (m_tcu ? this : nullptr); }
    virtual StructureSE* GetSBUSE()                     { return (m_sbu ? this : nullptr); }
    virtual StructureSE* GetOutpostSE()                 { return (m_bridge ? this : nullptr); }
    virtual StructureSE* GetJumpBridgeSE()              { return (m_outpost ? this : nullptr); }

    /* class type tests. */
    virtual bool IsPOSSE()                              { return m_pos; }
    virtual bool IsTCUSE()                              { return m_tcu; }
    virtual bool IsSBUSE()                              { return m_sbu; }
    virtual bool IsOutpostSE()                          { return m_outpost; }
    virtual bool IsJumpBridgeSE()                       { return m_bridge; }

    /* SystemEntity interface */
    virtual void Process();
    virtual void EncodeDestiny( Buffer& into );
    virtual PyDict *MakeSlimItem();

    /* specific functions handled in this class. */
    inline void SetPOSState(uint8 state)                       { m_state = state; }

    uint8 GetStructureState() const;

    PyTuple *GetEffectState();

private:
    bool m_tcu = false;
    bool m_pos = false;
    bool m_sbu = false;
    bool m_array = false;
    bool m_bridge = false;
    bool m_jammer = false;
    bool m_module = false;
    bool m_sentry = false;
    bool m_battery = false;
    bool m_outpost = false;

    uint8 m_state;          /* used to hold POS state (online, reinforced, operating, etc) */
    uint32 m_harmonic;      /* this tracks shield frequency for passing thru POS shields.  not sure how to use it yet.... */
    uint32 m_towerID;       /* this is the controlling towerID for POS modules */
    uint64 m_timestamp;     /* used to track time on POS states (onlining, reinforced, etc) */

};

#endif /* !__STRUCTURE__H__INCL__ */


