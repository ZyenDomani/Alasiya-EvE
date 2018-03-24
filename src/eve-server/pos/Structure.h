
/**
 * @name Structure.h
 *   Generic Base Class for POS items and entities.
 *
 * @Author:         Allan
 * @date:   unknown
 */


#ifndef EVEMU_POS_STRUCTURE_H_
#define EVEMU_POS_STRUCTURE_H_

#include "../eve-server.h"

#include "packets/POS.h"
#include "EVEServerConfig.h"
#include "inventory/InventoryItem.h"
#include "pos/PosMgrDB.h"
#include "system/SystemBubble.h"
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
    StructureItem(uint32 _structureID, const ItemType &_itemType, const ItemData &_data);
    virtual ~StructureItem();

public:
    static StructureItemRef Load( uint32 structureID);
    static StructureItemRef Spawn( ItemData &data);

    virtual void Delete();

    /** @todo (Allan)  will probably need to rewrite all these, too.
     * Public fields:
     */
    const ItemType &    type() const { return InventoryItem::type(); }

    PyObject *StructureGetInfo();

protected:
    using InventoryItem::_Load;
    virtual bool _Load();

    template<class _Ty>
    static RefPtr<_Ty> _LoadItem( uint32 structureID, const ItemType &type, const ItemData &data)
    {
        if ((type.categoryID() != EVEDB::invCategories::Structure)
        and (type.categoryID() != EVEDB::invCategories::Orbitals)
        and (type.categoryID() != EVEDB::invCategories::SovereigntyStructure)
        and (type.categoryID() != EVEDB::invCategories::StructureUpgrade)) {
            _log( ITEM__ERROR, "Trying to load %s as Structure %u.", type.category().name().c_str(), structureID);
            if (sConfig.server.StackTrace)
                EvE::traceStack();
            return RefPtr<_Ty>();
        }

        return StructureItemRef( new StructureItem(structureID, type, data ) );
    }
};


/**
 * ObjectSystemEntity which represents structure object in space
 */
class Client;
class Missile;
class MoonSE;
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
    virtual ReactorSE*          GetReactorSE()          { return nullptr; }

    /* class type tests. */
    virtual bool                IsPOSSE()               { return true; }
    virtual bool                IsCOSE()                { return m_co; }
    virtual bool                IsTCUSE()               { return m_tcu; }
    virtual bool                IsSBUSE()               { return m_sbu; }
    virtual bool                IsJammerSE()            { return m_jammer; }
    virtual bool                IsMoonMiner()           { return m_miner; }
    virtual bool                IsOutpostSE()           { return m_outpost; }
    virtual bool                IsJumpBridgeSE()        { return m_bridge; }
    virtual bool                IsTowerSE()             { return false; }
    virtual bool                IsArraySE()             { return false; }
    virtual bool                IsBatterySE()           { return false; }
    virtual bool                IsWeaponSE()            { return false; }
    virtual bool                IsReactorSE()           { return false; }

    virtual bool                isGlobal()              { return m_co; }    // just in case item->isGlobal() fails here for customs offices...which it may

    /* SystemEntity interface */
    virtual void                Process();
    virtual void                EncodeDestiny( Buffer& into );
    virtual PyDict*             MakeSlimItem();

    /* virtual functions default to base class and overridden as needed */
    virtual void                Killed(Damage &fatal_blow);
    virtual void                Init(InventoryItemRef iRef, SystemBubble* pBubble);
    virtual void                InitData(SystemBubble* pBubble);

    /* virtual functions to be overridden in derived classes */
    virtual void     MissileLaunched(Missile* pMissile) { /* Do nothing here */ }

    /* basic structure processing methods */
    virtual void                SetOnline();
    virtual void                SetOffline();
    virtual void                Online();
    virtual void                SetOperating();
    virtual void                Operating();

    /* basic tower processing methods */
    virtual void                Reinforced()            { /* do nothing here yet */ }

    /* specific functions handled in this class. */
    void                        Anchor();
    void                        Offline();
    void                        PullAnchor();
    void                        SetAnchor(Client* pClient, GPoint& pos);
    void                        Activate(int32 effectID);
    void                        Deactivate(int32 effectID);
    void                        GetEffectState(PyList& into);
    uint8                     GetStructureState() const { return m_data.state; }
    float                       GetStatus()             { return m_data.status; }
    MoonSE*                     GetMoonSE()             { return m_moonSE; }

    inline void                SetPOSState(uint8 state) { m_data.state = state; }
    inline void                 SetTimer(uint32 time)   { m_procTimer.SetTimer(time); }
    inline void                 SetStatus(float set)    { m_data.status = set; }

    void                        SetUsageFlags(int8 view=0, int8 take=0, int8 use=0);
    inline int8                 CanUse()                { return m_data.use; }
    inline int8                 CanView()               { return m_data.view; }
    inline int8                 CanTake()               { return m_data.take; }

    // for orbital infrastructure
    void                     SetPlanet(uint32 planetID) { m_planetID = planetID; }
    uint32                      GetPlanetID()           { return m_planetID; }
    void                        SetRotation(GPoint dir) { m_rotation = dir; }

    // structure update methods....may not use like this
    void                        UpdateTimeStamp()       { m_db.UpdateTimeStamp(m_data.itemID, m_data); }
    void                        UpdateUsageFlags()      { m_db.UpdateUsageFlags(m_data.itemID, m_data); }

    // for targetMgr
    bool                        IsReinforced()          { return false; }   /** @todo  finish this...not sure how yet. */

protected:
    void                        SendSlimUpdate();
    void                        SendEffectUpdate(int16 effectID, bool active);

    PosMgrDB                    m_db;

    MoonSE*                     m_moonSE;               /* moonSE this structure is orbiting. */
    TowerSE*                    m_towerSE;              /* controlling towerSE for this structure */

    EVEPOS::StructureData       m_data;

    int8                        m_procState;            /* internal state data for processing mode changes */

    // this is time shown in structure status (time left until current state completes)
    uint32 m_delayTime;

    // for orbital infrastructure (customs office and moon miner)
    GVector m_rotation;      /* direction to planet (for correct orientation) */
    uint32 m_planetID;

private:
    std::string GetProcStateName(int8 state);

    uint32 m_duration;              // module duration in ms

    Timer m_procTimer;              // module state timer

    bool m_co :1;
    bool m_tcu :1;
    bool m_sbu :1;
    bool m_tower :1;
    bool m_miner :1;
    bool m_bridge :1;
    bool m_jammer :1;
    bool m_module :1;
    bool m_reactor :1;
    bool m_outpost :1;

};

#endif  // EVEMU_POS_STRUCTURE_H_
