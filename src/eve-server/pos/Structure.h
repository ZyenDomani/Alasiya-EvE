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
    Author:        Aknor Jaden (original)
    Updates:    Allan   (rewrite)
*/

#ifndef __STRUCTURE__H__INCL__
#define __STRUCTURE__H__INCL__


#include "inventory/InventoryItem.h"
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
    virtual void Delete();

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


    void AddItem(InventoryItemRef item);
    void RemoveItem(InventoryItemRef item);

protected:
    /*
     * Member functions:
     */
    using InventoryItem::_Load;
    virtual bool _Load();


    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem(ItemFactory &factory, uint32 structureID, const ItemType &type, const ItemData &data)
    {
        if ((type.categoryID() != EVEDB::invCategories::Structure)
            and (type.categoryID() != EVEDB::invCategories::Orbitals))
        {
            _log( ITEM__ERROR, "Trying to load %s as Structure %u.", type.category().name().c_str(), structureID);
            return RefPtr<_Ty>();
        }

        return _Ty::template _LoadStructure<_Ty>( factory, structureID, type, data );
    }

    // Actual loading stuff:
    template<class _Ty>
    static RefPtr<_Ty> _LoadStructure(ItemFactory &factory, uint32 structureID, const ItemType &itemType, const ItemData &data)
    {
        return StructureItemRef( new StructureItem( factory, structureID, itemType, data ) );
    }

    static uint32 CreateItemID(ItemFactory &factory, ItemData &data);

};


/**
 * ObjectSystemEntity which represents structure object in space
 */

class StructureSE
: public ObjectSystemEntity
{
public:
    StructureSE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& data);
    virtual ~StructureSE()                              { /* Do nothing here */ }

    void Init(StructureItemRef structure);

    /* class type pointer querys. */
    virtual StructureSE*        GetCOSE()               { return (m_co ? this : nullptr); }
    virtual StructureSE*        GetPOSSE()              { return (m_pos ? this : nullptr); }
    virtual StructureSE*        GetTCUSE()              { return (m_tcu ? this : nullptr); }
    virtual StructureSE*        GetSBUSE()              { return (m_sbu ? this : nullptr); }
    virtual StructureSE*        GetOutpostSE()          { return (m_bridge ? this : nullptr); }
    virtual StructureSE*        GetJumpBridgeSE()       { return (m_outpost ? this : nullptr); }

    /* class type tests. */
    virtual bool                IsCOSE()                { return m_co; }
    virtual bool                IsPOSSE()               { return m_pos; }
    virtual bool                IsTCUSE()               { return m_tcu; }
    virtual bool                IsSBUSE()               { return m_sbu; }
    virtual bool                IsOutpostSE()           { return m_outpost; }
    virtual bool                IsJumpBridgeSE()        { return m_bridge; }

    virtual bool                isGlobal()              { return m_co; }    // just in case item->isGlobal() fails here for customs offices...which it may

    /* SystemEntity interface */
    virtual void                Process();
    virtual void                EncodeDestiny( Buffer& into );
    virtual PyDict*             MakeSlimItem();

    /* virtual functions default to base class and overridden as needed */
    virtual void Killed(Damage &fatal_blow);

    /* specific functions handled in this class. */
    PyTuple*                    GetEffectState();
    uint8                       GetStructureState() const;
    SystemEntity*               GetMoonEntity()         { return m_moonSE; }

    inline void                SetPOSState(uint8 state) { m_state = state; }

    // for orbital infrastructure
    void                     SetPlanet(uint32 planetID) { m_planetID = planetID; }
    uint32                      GetPlanetID()           { return m_planetID; }
    void                        SetRotation(GPoint dir) { m_rotation = dir; }

    // for tower sentry
    void SetStanding(float set)                         { m_standing = set; }
    void SetStatus(float set)                           { m_status = set; }
    void SetStatusDrop(bool set)                        { m_statusDrop = set; }
    void SetCorpWar(bool set)                           { m_corpWar = set; }
    void SetStandingOwnerID(uint32 set)                 { m_standingOwnerID = set; }

    bool GetStatusDrop()                                { return m_statusDrop; }
    bool GetCorpWar()                                   { return m_corpWar; }
    float GetStanding()                                 { return m_standing;}
    float GetStatus()                                   { return m_status; }
    uint32 GetStandingOwnerID()                         { return m_standingOwnerID; }

private:
    SystemEntity* m_moonSE; /* moon this structure is stationed at.  used for killMail */

    bool m_co;
    bool m_tcu;
    bool m_pos;
    bool m_sbu;
    bool m_array;
    bool m_bridge;
    bool m_jammer;
    bool m_module;
    bool m_sentry;
    bool m_battery;
    bool m_outpost;

    int32 m_harmonic;       /* this tracks shield frequency for passing thru POS shields.  not sure how to use it yet..  -1 means "none" (i think) */

    uint8 m_state;          /* used to hold POS state (online, reinforced, operating, etc) */
    uint32 m_towerID;       /* this is the controlling towerID for POS modules */
    uint64 m_timestamp;     /* used to track start time on POS states (onlining, reinforced, etc) */

    // for orbital infrastructure (customs office)
    GPoint m_rotation;      /* direction to planet (for correct orientation) */
    uint32 m_planetID;

    // tower sentry settings
    float m_standing;
    float m_status;
    bool m_statusDrop :1;
    bool m_corpWar :1;
    uint32 m_standingOwnerID; // corp/ally
};

#endif /* !__STRUCTURE__H__INCL__ */


