
 /**
  * @name TurretMath.cpp
  *   formulas for turret tracking, to hit, and other specific things
  * @Author:         Allan
  * @date:   10 June 2015
  * @update:    Allan 13Nov26
  */

 /* default crit chances in server config
  *      NPC  - 1.5%
  *   Player  - 2%
  *   Sentry  - 2%
  *    Drone  - 3%
  *  Concord  - 5%
  */

#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "npc/Drone.h"
#include "npc/Sentry.h"
#include "ship/modules/TurretMath.h"
#include "ship/modules/TurretModule.h"

float TurretMath::GetToHit(ShipItemRef shipRef, TurretModule* pMod, SpaceEntity* pTarget) {
    if (pTarget == nullptr)
        return 0;
    const double rNum = MakeRandomDouble();
    if (rNum <= sConfig.rates.PlayerCritChance)
        return 3.0f;

    HitInputs in;
    in.shooterPos     = shipRef->position();
    in.shooterVel     = shipRef->GetPilot()->GetShipSE()->GetVelocity();
    in.targetPos      = pTarget->GetPosition();
    in.targetVel      = pTarget->GetVelocity();
    in.targSig        = pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_double();
    in.range          = pMod->GetAttribute(AttrMaxRange).get_double();
    in.falloff        = pMod->GetAttribute(AttrFalloff).get_double();
    in.trackSpeed     = pMod->GetAttribute(AttrTrackingSpeed).get_double();
    in.sigRes         = pMod->GetAttribute(AttrOptimalSigRadius).get_double();
    in.scaleSigDamage = false;

    const HitResult res = ComputeHitChance(in);

    if (is_log_enabled(DAMAGE__TRACE)) {
        _log(DAMAGE__TRACE, "GetToHit - distance:%0.2f, range:%0.2f, falloff:%0.2f", res.distance, in.range, in.falloff);
        _log(DAMAGE__TRACE, "GetToHit - transversalV:%0.3f, angularV:%0.3f, tracking:%0.3f, targetSig:%0.1f, sigRes:%0.1f", \
                res.transversalV, res.angularVel, in.trackSpeed, in.targSig, in.sigRes);
        _log(DAMAGE__TRACE, "GetToHit - (hitChance:%0.4f > Rand:%0.4f) = %s", res.hitChance, rNum, \
                (rNum < res.hitChance ? "Hit" : "Miss"));
    }

    if (rNum < res.hitChance)
        return static_cast<float>(rNum + 0.49);
    return 0.0f;
}

float TurretMath::GetNPCToHit(NPC* pNPC, SpaceEntity* pTarget) {
    if (pTarget == nullptr)
        return 0;
    const double rNum = MakeRandomDouble();
    if (rNum <= sConfig.rates.DroneCritChance)
        return 3.0f;

    InventoryItemRef nRef = pNPC->GetSelf();

    HitInputs in;
    in.shooterPos     = pNPC->GetPosition();
    in.shooterVel     = pNPC->GetVelocity();
    in.targetPos      = pTarget->GetPosition();
    in.targetVel      = pTarget->GetVelocity();
    in.targSig        = pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_double();
    in.range          = nRef->GetAttribute(AttrEntityAttackRange).get_double();
    in.falloff        = nRef->GetAttribute(AttrFalloff).get_double();
    in.trackSpeed     = nRef->GetAttribute(AttrTrackingSpeed).get_double();
    in.sigRes         = nRef->GetAttribute(AttrOptimalSigRadius).get_double();
    in.scaleSigDamage = false;

    const HitResult res = ComputeHitChance(in);

    if (is_log_enabled(DAMAGE__TRACE_NPC)) {
        const Vector3d relativeVelocity = in.targetVel - in.shooterVel;
        _log(DAMAGE__TRACE_NPC, "GetToHit - distance:%0.2f, range:%0.1f, falloff:%0.1f", res.distance, in.range, in.falloff);
        _log(DAMAGE__TRACE_NPC, "GetToHit - myVel:%0.3f, %0.3f, %0.3f", in.shooterVel.x, in.shooterVel.y, in.shooterVel.z);
        _log(DAMAGE__TRACE_NPC, "GetToHit - targVel:%0.3f, %0.3f, %0.3f", in.targetVel.x, in.targetVel.y, in.targetVel.z);
        _log(DAMAGE__TRACE_NPC, "GetToHit - relativeVelocity:%0.3f, %0.3f, %0.3f", relativeVelocity.x, relativeVelocity.y, relativeVelocity.z);
        _log(DAMAGE__TRACE_NPC, "GetToHit - radialV:%0.3f", res.radialV);
        _log(DAMAGE__TRACE_NPC, "GetToHit - transversalV:%0.3f, angularVel:%0.3f tracking:%0.3f, targetSig:%0.1f, sigRes:%0.1f", \
                res.transversalV, res.angularVel, in.trackSpeed, in.targSig, in.sigRes);
        _log(DAMAGE__TRACE_NPC, "GetToHit - (hitChance:%0.4f > Rand:%0.4f) = %s", res.hitChance, rNum, \
                (rNum < res.hitChance ? "Hit" : "Miss"));
    }

    if (rNum < res.hitChance)
        return static_cast<float>(rNum + 0.49);
    return 0.0f;
}

float TurretMath::GetDroneToHit(DroneSE* pDrone, SpaceEntity* pTarget) {
    if (pTarget == nullptr)
        return 0;

    const double rNum = MakeRandomDouble();
    if (rNum <= sConfig.rates.DroneCritChance)
        return 3.0f;

    InventoryItemRef dRef = pDrone->GetSelf();

    HitInputs in;
    in.shooterPos     = pDrone->GetPosition();
    in.shooterVel     = pDrone->GetVelocity();
    in.targetPos      = pTarget->GetPosition();
    in.targetVel      = pTarget->GetVelocity();
    in.targSig        = pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_double();
    in.range          = dRef->GetAttribute(AttrEntityAttackRange).get_double();
    in.falloff        = dRef->GetAttribute(AttrFalloff).get_double();
    in.trackSpeed     = dRef->GetAttribute(AttrTrackingSpeed).get_double();
    in.sigRes         = dRef->GetAttribute(AttrOptimalSigRadius).get_double();
    in.scaleSigDamage = false;

    const HitResult res = ComputeHitChance(in);

    if (rNum < res.hitChance)
        return static_cast<float>(rNum + 0.49);
    // drones will have a minimum damage instead of zero
    return 0.1f;
}

float TurretMath::GetSentryToHit(Sentry* pSentry, SpaceEntity* pTarget) {
    if (pTarget == nullptr)
        return 0.0f;

    // 1. Roll first: if it crits, skip all attribute lookups and vector math!
    const double rNum = MakeRandomDouble();
    if (rNum <= sConfig.rates.SentryCritChance)
        return 3.0f;

    InventoryItemRef sRef = pSentry->GetSelf();

    HitInputs in;
    in.shooterPos     = pSentry->GetPosition();
    in.shooterVel     = NULL_ORIGIN;
    in.targetPos      = pTarget->GetPosition();
    in.targetVel      = pTarget->GetVelocity();
    in.targSig        = pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_double();
    in.range          = sRef->GetAttribute(AttrEntityAttackRange).get_double();
    in.falloff        = sRef->GetAttribute(AttrFalloff).get_double();
    in.trackSpeed     = sRef->GetAttribute(AttrTrackingSpeed).get_double();
    in.sigRes         = sRef->GetAttribute(AttrOptimalSigRadius).get_double();
    in.scaleSigDamage = true;

    const HitResult res = ComputeHitChance(in);

    if (rNum < res.hitChance)
        return static_cast<float>((rNum + 0.49) * res.damageMod);

    return 0.0f;
}


// Core Helper
TurretMath::HitResult TurretMath::ComputeHitChance(const HitInputs& in) {
    HitResult res { 0.0, 1.0, 0.0, 0.0, 0.0, 0.0 };

    if (in.trackSpeed <= 0.0 || in.targSig <= 0.0 || in.sigRes <= 0.0)
        return res;

    Vector3d lineOfSight = in.targetPos - in.shooterPos;
    res.distance = lineOfSight.Length();
    if (res.distance < 0.1)
        res.distance = 0.1;

    // Normalize using already-computed distance (avoids 2nd sqrt())
    lineOfSight *= (1.0 / res.distance);

    //  calculations for chance to hit
    /*     a =  angVelocity/(distance * tracking)
     *     b =  turret sig res / target sig radius
     *     c =  (a * b) ^ 2
     *     d =  max(0, distance - optimal range)
     *     e =  (d / falloff) ^ 2
     */
    Vector3d relativeVelocity = in.targetVel - in.shooterVel;
    // Isolate radial velocity component (along the line of sight)
    res.radialV = relativeVelocity.dotProduct(lineOfSight);
    // Transversal velocity vector is total relative velocity minus the radial component
    const Vector3d transversalVector = relativeVelocity - (lineOfSight * res.radialV);
    res.transversalV = transversalVector.Length();
    res.angularVel = res.transversalV / res.distance;

    if (res.angularVel < 0.0001) {
        //fallback hack for 20% chance when angularVel is effectively zero and target is within gun's range
        if (res.distance < in.range)
            res.hitChance = MakeRandomDouble(0.0, 0.5);
        return res;
    }

    double a = (res.angularVel * in.sigRes) / (in.trackSpeed * in.targSig);
    if (in.scaleSigDamage) {
        // enable damage-reduction block for more lower-damage hits (sentry only)
        const double b = in.sigRes / in.targSig;
        if (a < 1.0 && b > 1.0) {
            // Re-scale 'a' to use b = 1.0 and transfer excess ratio to damageMod
            a = res.angularVel / in.trackSpeed;
            res.damageMod = in.targSig / in.sigRes;
        }
    }

    const double c = a * a;
    const double d = EvE::max(res.distance - in.range, 0.0);
    double e = 0.0;
    if (d > 0.0 && in.falloff > 0.0) {
        const double falloffRatio = d / in.falloff;
        e = falloffRatio * falloffRatio;
    }

    res.hitChance = std::pow(0.5, c + e);
    return res;
}
