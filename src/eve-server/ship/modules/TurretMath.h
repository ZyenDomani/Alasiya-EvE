
 /**
  * @name TurretMath.h
  *   forumlas for turret tracking, to hit, and other specific things
  * @Author:         Allan
  * @date:   10 June 2015
  * @update:    Allan 13Nov26
  */

#pragma once

#include "eve-compat.h"
#include "ship/Ship.h"

class NPC;
class Sentry;
class DroneSE;
class TurretModule;
class Vector3d;

class TurretMath {
public:
    TurretMath() = delete;
    ~TurretMath() = delete;

    static float GetToHit(ShipItemRef shipRef, TurretModule* pMod, SpaceEntity* pTarget);
    static float GetNPCToHit(NPC* pNPC, SpaceEntity* pTarget);
    static float GetDroneToHit(DroneSE* pDrone, SpaceEntity* pTarget);
    static float GetSentryToHit(Sentry* pSentry, SpaceEntity* pTarget);

private:
    struct HitInputs {
        Vector3d shooterPos;
        Vector3d shooterVel;
        Vector3d targetPos;
        Vector3d targetVel;
        double   targSig;
        double   range;
        double   falloff;
        double   trackSpeed;
        double   sigRes;
        bool     scaleSigDamage; // true for Sentries
    };

    struct HitResult {
        double hitChance;
        double damageMod;
        double distance;
        double radialV;
        double transversalV;
        double angularVel;
    };

    static HitResult ComputeHitChance(const HitInputs& in);
};
