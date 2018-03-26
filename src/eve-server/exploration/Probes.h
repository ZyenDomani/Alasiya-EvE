 /**
  * @name Probes.h
  *     Probe SE class for Alasiya EvEmu
  *
  * @Author:        Allan
  * @date:          10 March 2018
  *
  */

#ifndef EVEMU_EXPLORE_PROBES_H_
#define EVEMU_EXPLORE_PROBES_H_

/*  this class is in charge of creating/destroying and maintaining
 * anomaly types in it's system.
 *
 *  a new iteration of this class is created for each system as that system is booted.
 */


#include "EVEServerConfig.h"
#include "../../eve-common/EVE_Scanning.h"
#include "inventory/InventoryItem.h"
#include "system/SystemEntity.h"


/**
 * InventoryItem for generic celestial object.
 */
class ProbeItem
: public InventoryItem
{
    friend class InventoryItem; // to let it construct us
public:
    ProbeItem(uint32 itemID, const ItemType &_type, const ItemData &_data);
    virtual ~ProbeItem()                                { /* Do nothing here */ }

    static ProbeItemRef Load( uint32 itemID);
    static ProbeItemRef Spawn( ItemData &data);

protected:
    using InventoryItem::_Load;
    //virtual bool _Load();

    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem( uint32 itemID, const ItemType &type, const ItemData &data)
    {
        if ((type.groupID() != EVEDB::invGroups::Scanner_Probe)
        and (type.groupID() != EVEDB::invGroups::Survey_Probe)
        //and (type.groupID() != EVEDB::invGroups::Warp_Disruption_Probe)  this wont work here...
        and (type.groupID() != EVEDB::invGroups::Obsolete_Probes)) {
            _log( ITEM__ERROR, "Trying to load %s as Probe.", type.group().name().c_str() );
            if (sConfig.server.StackTrace)
                EvE::traceStack();
            return RefPtr<_Ty>();
        }

        return ProbeItemRef( new ProbeItem(itemID, type, data));
    }

    static uint32 CreateItemID( ItemData &data);
};



/**
 * DynamicSystemEntity which represents celestial object in space
 */
class PyServiceMgr;
class Scan;

class ProbeSE : public DynamicSystemEntity {
public:
    ProbeSE(ProbeItemRef self, PyServiceMgr& services, SystemManager* system, InventoryItemRef moduleRef, ShipItemRef shipRef);
    virtual ~ProbeSE()                                  { /* Do nothing here */ }

    /* Process Calls - Overridden as needed in derived classes */
    virtual void                Process();

    /* class type pointer querys. */
    virtual ProbeSE*            GetProbeSE()            { return this; }
    /* class type tests. */
    /* Base */
    virtual bool                IsProbeSE()             { return true; }

    /* virtual functions default to base class and overridden as needed */
    virtual void                MakeDamageState(DoDestinyDamageState &into);
    virtual PyDict*             MakeSlimItem();
    virtual void                Delete();

    /* specific functions handled in this class. */
    void RecoverProbe(PyList* list);
    void UpdateProbe(ProbeData& data);
    void SendNewProbe();
    void SendSlimChange();
    void SendStateChange(uint8 state);
    void SendRemoveProbe();
    void SendWarpStart(float travelTime);
    void SendWarpEnd();

    void SetScan(Scan* pScan)                           { m_scan = pScan; }
    void RemoveScan()                                   { m_scan = nullptr; }
    void StartStateTimer(uint16 time)                   { m_stateTimer.Start(time); }

    bool IsMoving();
    uint8 GetState()                                    { return m_state; }
    uint8 GetRangeStep()                                { return m_rangeStep; }

    // remaining move time in ds
    uint16 GetMoveTime()                                { return m_stateTimer.GetRemainingTime() /100; }
    int64 GetExpiryTime()                               { return m_expiry; }

    float GetDeviation();
    float GetScanRange()                                { return m_scanRange; }
    float GetRangeModifier(float dist);

    // total, modified probe scan strength, based on data modified by char skills, ship, launcher, distance and range
    float GetScanStrength();

    GPoint GetDestination()                             { return m_destination; }
    std::string GetStateName(uint8 state);


private:
    Timer m_lifeTimer;
    Timer m_returnTimer;
    Timer m_stateTimer;

    Scan* m_scan;
    Client* m_client;

    GPoint m_destination;

    ShipItemRef m_shipRef;
    InventoryItemRef m_moduleRef;

    uint8 m_state;
    uint8 m_rangeStep;

    int64 m_expiry;

    float m_secStatus;
    float m_scanRange;
    float m_scanStrength;
    float m_scanDeviation;

};

#endif  // EVEMU_EXPLORE_PROBES_H_
