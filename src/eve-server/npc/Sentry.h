 /**
  * @name Sentry.h
  *   Sentry class
  * @Author:    Allan
  * @date:      23 April 2017
  */

#ifndef __NPC_SENTRY_H
#define __NPC_SENTRY_H

#include "system/SpaceEntity.h"

class PyServiceMgr;
class DestinyManager;
class InventoryItem;
class SentryAI;
class SystemManager;
class ServiceDB;

class Sentry
: public ObjectSpaceEntity
{
public:
    Sentry(InventoryItemRef self, PyServiceMgr& services, SystemManager* system, const FactionData& data);
    virtual ~Sentry();

    /* class type pointer querys. */
    virtual const char*         GetSEType()             { return "Sentry SE"; }
    virtual Sentry* GetSentrySE()                       { return this; }
    /* class type tests. */
    virtual bool IsSentrySE()                           { return true; }

    /* SpaceEntity interface */
    virtual void Process();
    virtual void TargetLost(SpaceEntity* who);
    virtual void TargetedAdd(SpaceEntity* who);
    virtual void EncodeDestiny(Buffer& into);

    /* virtual functions default to base class and overridden as needed */
    virtual void Killed(Damage &fatal_blow);

    /* specific functions handled here. */
    void SaveSentry();
    void RemoveSentry();
    void SetResists();

    float GetThermal()                                  { return m_therDamage; }
    float GetEM()                                       { return m_emDamage; }
    float GetKinetic()                                  { return m_kinDamage; }
    float GetExplosive()                                { return m_expDamage; }

    SentryAI* GetAI()                                { return m_AI; }

protected:
    SentryAI* m_AI;

private:
    float m_emDamage = 0;
    float m_expDamage = 0;
    float m_kinDamage = 0;
    float m_therDamage = 0;
    float m_hullDamage = 0;
    float m_armorDamage = 0;
    float m_shieldCharge = 0;
    float m_shieldCapacity = 0;
};

#endif  // __NPC_SENTRY_H
