
/**
 * @name POS.h
 *   Specific Class for managing POS-specific data.
 *
 * @Author:         Allan
 * @date:   24July17
 */

#ifndef EVEMU_POS_POS_H_
#define EVEMU_POS_POS_H_

#include "../eve-server.h"

#include "pos/Structure.h"

class TowerSE
: public StructureSE
{
public:
    TowerSE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& fData);
    virtual ~TowerSE()                                  { /* do nothing here */ }

    /* class type pointer querys. */
    virtual TowerSE*            GetTowerSE()            { return this; }

    /* class type tests. */
    virtual bool                IsTowerSE()             { return true; }

    /* SystemEntity interface */
    virtual void                Process();
    virtual PyDict*             MakeSlimItem();


    /* specific functions handled in this class. */
    void Init(StructureItemRef structure);

    void SetStanding(float set)                         { data.standing = set; }
    void SetStatus(float set)                           { data.status = set; }
    void SetStatusDrop(bool set)                        { data.statusDrop = set; }
    void SetCorpWar(bool set)                           { data.corpWar = set; }
    void SetStandingOwnerID(uint32 set)                 { data.standingOwnerID = set; }

    bool GetStatusDrop()                                { return data.statusDrop; }
    bool GetCorpWar()                                   { return data.corpWar; }
    float GetStanding()                                 { return data.standing; }
    float GetStatus()                                   { return data.status; }
    uint32 GetStandingOwnerID()                         { return data.standingOwnerID; }

    bool ShowInCalendar()                               { return data.showInCalendar; }
    bool SendFuelNotifications()                        { return data.sendFuelNotifications; }
    void SetShowInCalendar(bool set)                    { data.showInCalendar = set; }
    void SetSendFuelNotifications(bool set)             { data.sendFuelNotifications = set; }

    void GetTowerData(EVEPOS::TowerData& tData)         { tData = data; }

    void UpdateSentry();
    void UpdateNotify();
    void UpdatePermission();
    void UpdateTimeStamp();
    
private:
    EVEPOS::TowerData data;
};

class ArraySE
: public StructureSE
{
public:
    ArraySE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& data);
    virtual ~ArraySE()                                  { /* do nothing here */ }

    /* class type pointer querys. */
    virtual ArraySE*            GetArraySE()            { return this; }

    /* class type tests. */
    virtual bool                IsArraySE()             { return true; }

    /* SystemEntity interface */
    virtual void                Process();

    /* specific functions handled in this class. */
    void Init(StructureItemRef structure);


};

class BatterySE
: public StructureSE
{
public:
    BatterySE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& data);
    virtual ~BatterySE()                                { /* do nothing here */ }

    /* class type pointer querys. */
    virtual BatterySE*          GetBatterySE()          { return this; }

    /* class type tests. */
    virtual bool                IsBatterySE()           { return true; }

    /* SystemEntity interface */
    virtual void                Process();


    /* specific functions handled in this class. */
    void Init(StructureItemRef structure);


};

class WeaponSE
: public StructureSE
{
public:
    WeaponSE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& data);
    virtual ~WeaponSE()                                 { /* do nothing here */ }

    /* class type pointer querys. */
    virtual WeaponSE*           GetWeaponSE()           { return this; }

    /* class type tests. */
    virtual bool                IsWeaponSE()            { return true; }

    /* SystemEntity interface */
    virtual void                Process();


    /* specific functions handled in this class. */
    void Init(StructureItemRef structure);


};

#endif  // EVEMU_POS_POS_H_
