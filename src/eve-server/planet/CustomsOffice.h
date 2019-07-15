
/**
 * @name CustomsOffice.h
 *   Class for Customs Offices.
 *
 * @Author:         Allan
 * @date:   14 July 2019
 */


#ifndef EVEMU_POS_CUSTOMS_OFFICE_H_
#define EVEMU_POS_CUSTOMS_OFFICE_H_

#include "../eve-server.h"

#include "inventory/InventoryItem.h"
#include "pos/PosMgrDB.h"
#include "system/SystemEntity.h"


class CustomsSE
: public ObjectSystemEntity
{
public:
    CustomsSE(StructureItemRef sRef, PyServiceMgr& services, SystemManager* system, const FactionData& data);
    virtual ~CustomsSE()  { /* do nothing here */ }


    /* class type pointer querys. */
    virtual CustomsSE*          GetCOSE()               { return this; }

    /* class type tests. */
    virtual bool                IsCOSE()                { return true; }
    virtual bool                isGlobal()              { return true; }

    /* SystemEntity interface */
    virtual void                Process();
    virtual void                EncodeDestiny( Buffer& into );
    virtual PyDict*             MakeSlimItem();

    /* virtual functions default to base class and overridden as needed */
    virtual void                Killed(Damage &fatal_blow);
    virtual void                InitData();
    virtual void                Init();

    /* virtual functions to be overridden in derived classes */
    virtual void     MissileLaunched(Missile* pMissile) { /* Do nothing here */ }

    /* specific functions handled in this class. */
    void                        Reinforced()            { /* do nothing here yet */ }
    void                        GetEffectState(PyList& into);
    uint8                     GetStructureState() const { return m_data.state; }
    float                       GetStatus()             { return m_data.status; }
    bool                        IsReinforced()          { return false; }   /** @todo  finish this...not sure how yet. */

    // for orbital infrastructure
    void                     SetPlanet(uint32 planetID) { m_data.planetID = planetID; }
    uint32                      GetPlanetID()           { return m_data.planetID; }
    void                        SetRotation(GPoint dir) { m_data.rotation = dir; }
    float                       GetTaxRate()            { return m_data.taxRate; }


protected:
    void                        SendSlimUpdate();
    void                        SendEffectUpdate(int16 effectID, bool active);

private:
    PosMgrDB                    m_db;

    EVEPOS::CustomsData         m_data;

    SystemManager*              m_system;
    SystemEntity*               m_planetSE;
};

#endif  // EVEMU_POS_CUSTOMS_OFFICE_H_
