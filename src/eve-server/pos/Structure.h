
/**
 * @name Structure.h
 *   Specific Class for POS items and entities.
 *
 * @Author:         Allan
 * @date:   unknown
 */


#ifndef EVEMU_POS_STRUCTURE_H_
#define EVEMU_POS_STRUCTURE_H_


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
class TowerSE;
class ArraySE;
class BatterySE;
class WeaponSE;
class StructureSE
: public ObjectSystemEntity
{
public:
    StructureSE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& data);
    virtual ~StructureSE()  { /* do nothing here */ }

    void Init(StructureItemRef structure);

    /* class type pointer querys. */
    virtual StructureSE*        GetPOSSE()              { return this; }
    virtual StructureSE*        GetCOSE()               { return (m_co ? this : nullptr); }
    virtual StructureSE*        GetTCUSE()              { return (m_tcu ? this : nullptr); }
    virtual StructureSE*        GetSBUSE()              { return (m_sbu ? this : nullptr); }
    virtual StructureSE*        GetJammerSE()           { return (m_jammer ? this : nullptr); }
    virtual StructureSE*        GetOutpostSE()          { return (m_bridge ? this : nullptr); }
    virtual StructureSE*        GetJumpBridgeSE()       { return (m_outpost ? this : nullptr); }
    virtual TowerSE*            GetTowerSE()            { return nullptr; }
    virtual ArraySE*            GetArraySE()            { return nullptr; }
    virtual BatterySE*          GetBatterySE()          { return nullptr; }
    virtual WeaponSE*           GetWeaponSE()           { return nullptr; }

    /* class type tests. */
    virtual bool                IsPOSSE()               { return true; }
    virtual bool                IsCOSE()                { return m_co; }
    virtual bool                IsTCUSE()               { return m_tcu; }
    virtual bool                IsSBUSE()               { return m_sbu; }
    virtual bool                IsJammerSE()            { return m_jammer; }
    virtual bool                IsOutpostSE()           { return m_outpost; }
    virtual bool                IsJumpBridgeSE()        { return m_bridge; }
    virtual bool                IsTowerSE()             { return false; }
    virtual bool                IsArraySE()             { return false; }
    virtual bool                IsBatterySE()           { return false; }
    virtual bool                IsWeaponSE()            { return false; }

    virtual bool                isGlobal()              { return m_co; }    // just in case item->isGlobal() fails here for customs offices...which it may

    /* SystemEntity interface */
    virtual void                Process();
    virtual void                EncodeDestiny( Buffer& into );
    virtual PyDict*             MakeSlimItem();

    /* virtual functions default to base class and overridden as needed */
    virtual void Killed(Damage &fatal_blow);

    /* specific functions handled in this class. */
    void                        InitData();
    void                        Activate(int32 effectID);
    void                        Deactivate(int32 effectID);

    PyTuple*                    GetEffectState();
    uint8                     GetStructureState() const { return m_state; }
    SystemEntity*               GetMoonEntity()         { return m_moonSE; }

    inline void                SetPOSState(uint8 state) { m_state = state; }

    // for orbital infrastructure
    void                     SetPlanet(uint32 planetID) { m_planetID = planetID; }
    uint32                      GetPlanetID()           { return m_planetID; }
    void                        SetRotation(GPoint dir) { m_rotation = dir; }

protected:
    PosMgrDB m_db;

    SystemEntity* m_moonSE; /* moon this structure is stationed at.  used for killMail */

    uint8 m_state;          /* used to hold POS state (online, reinforced, operating, etc) */
    uint32 m_towerID;       /* this is the controlling towerID for POS modules */
    uint32 m_delayTime;     /* dynamic - only used for reinforced states */
    uint64 m_timestamp;     /* used to track start time on POS states (onlining, reinforced, etc) */


    // for orbital infrastructure (customs office)
    GPoint m_rotation;      /* direction to planet (for correct orientation) */
    uint32 m_planetID;

private:
    Timer m_procTimer;

    bool m_co :1;
    bool m_tcu :1;
    bool m_sbu :1;
    bool m_bridge :1;
    bool m_jammer :1;
    bool m_module :1;
    bool m_outpost :1;

};

#endif  // EVEMU_POS_STRUCTURE_H_
