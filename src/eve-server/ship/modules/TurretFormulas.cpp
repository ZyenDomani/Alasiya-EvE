
 /**
  * @name TurretFormulas.cpp
  *   formulas for turret tracking, to hit, and other specific things
  * @Author:         Allan
  * @date:   10 June 2015
  */

 /* default crit chances - can change in server config
  *      NPC  - 1.5%
  *   Player  - 2%
  *   Sentry  - 2%
  *    Drone  - 3%
  *  Concord  - 5%
  */

#include "character/Character.h" // this sets compat includes for this file
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "npc/Drone.h"
#include "npc/Sentry.h"
#include "ship/modules/TurretFormulas.h"
#include "ship/modules/TurretModule.h"


float TurretFormulas::GetToHit(ShipItemRef shipRef, TurretModule* pMod, SystemEntity* pTarget) {
    if (pTarget == nullptr)
        return 0;

    double range = pMod->GetAttribute(AttrMaxRange).get_double();
    double sigRes = pMod->GetAttribute(AttrOptimalSigRadius).get_double();
    double falloff = pMod->GetAttribute(AttrFalloff).get_double();
    double targSig = pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_double();
    double trackSpeed = pMod->GetAttribute(AttrTrackingSpeed).get_double();
    Vector3d lineOfSight =  pTarget->GetPosition() - shipRef->position();
    double distance = lineOfSight.Length();
    if (distance < 0.1)
        distance = 0.1;
    lineOfSight.Normalize();

    Vector3d relativeVelocity = pTarget->GetVelocity() - shipRef->GetPilot()->GetShipSE()->GetVelocity();
    // Isolate radial velocity component (along the line of sight)
    double radialV = relativeVelocity.dotProduct(lineOfSight);
    // Transversal velocity vector is total relative velocity minus the radial component
    Vector3d transversalVector = relativeVelocity - (lineOfSight * radialV);
    double transversalV = transversalVector.Length();
    double angularVel = transversalV / distance;
    //  calculations for chance to hit
    /*     a =  angVelocity/(distance * tracking)
     *     b =  turret sig res / target sig radius
     *     c =  (a * b) ^ 2
     *     d =  max(0, distance - optimal range)
     *     e =  (d / falloff) ^ 2
     */
    double hitChance = 0.0;
    if (angularVel < 0.0001) {
        //fallback hack for 20% chance when angularVel is effectively zero
        if (distance < range)
            hitChance = MakeRandomDouble(0.0, 0.5);
    } else {
        double a = angularVel / (trackSpeed * targSig);
        double c = a * a;
        double d = EvE::max(distance - range, 0.0);
        double e = 0.0;
        if (d > 0)
            e = (d / falloff) * (d / falloff);
        hitChance = std::pow(0.5, c + e);
    }

    double rNum = MakeRandomDouble();
    if (is_log_enabled(DAMAGE__TRACE)) {
        _log(DAMAGE__TRACE, "GetToHit - distance:%0.2f, range:%0.2f, falloff:%0.2f", distance, range, falloff);
        _log(DAMAGE__TRACE, "GetToHit - transversalV:%0.3f, angularV:%0.3f, tracking:%0.3f, targetSig:%0.1f, sigRes:%0.1f", \
                transversalV, angularVel, trackSpeed, targSig, sigRes);
        _log(DAMAGE__TRACE, "GetToHit - (hitChance:%0.4f > Rand:%0.4f) = %s", hitChance, rNum, \
                ((rNum <= sConfig.rates.PlayerCritChance) ? "Crit" : (rNum < hitChance ? "Hit" : "Miss")));
    }
    if (rNum <= sConfig.rates.PlayerCritChance)
        return 3.0f;
    if (rNum < hitChance)
        return static_cast<float>(rNum + 0.49);
    return 0.0f;
}

float TurretFormulas::GetNPCToHit(NPC* pNPC, SystemEntity* pTarget) {
    if (pTarget == nullptr)
        return 0;

    InventoryItemRef iRef = pNPC->GetSelf();
    double sigRes = iRef->GetAttribute(AttrOptimalSigRadius).get_double();
    double range = iRef->GetAttribute(AttrMaxRange).get_double();
    double falloff = iRef->GetAttribute(AttrFalloff).get_double();
    double trackSpeed = iRef->GetAttribute(AttrTrackingSpeed).get_double();
    double targSig = pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_double();
    Vector3d lineOfSight =  pTarget->GetPosition() - pNPC->GetPosition();
    double distance = lineOfSight.Length();
    if (distance < 0.1)
        distance = 0.1;
    lineOfSight.Normalize();

    Vector3d targVel = pTarget->GetVelocity();
    Vector3d myVel = pNPC->GetVelocity();
    // calculate transversal from other data
    Vector3d relativeVelocity = targVel - myVel;
    // Isolate radial velocity component (along the line of sight)
    double radialV = relativeVelocity.dotProduct(lineOfSight);
    // Transversal velocity vector is total relative velocity minus the radial component
    Vector3d transversalVector = relativeVelocity - (lineOfSight * radialV);
    double transversalV = transversalVector.Length();
    double angularVel = transversalV / distance;

    // revert to original ToHit (circa. 2015) code...
    double hitChance = 0.0;
    if (angularVel < 0.0001) {
        //fallback hack for 20% chance when angularVel is effectively zero
        if (distance < range)
            hitChance = MakeRandomDouble(0.0, 0.5);
    } else {
        double a = angularVel / (trackSpeed * targSig);
        double c = a * a;
        double d = EvE::max(distance - range, 0.0);
        double e = 0.0;
        if (d > 0)
            e = (d / falloff) * (d / falloff);
        hitChance = std::pow(0.5, c + e);
    }

    double rNum = MakeRandomDouble();
    if (is_log_enabled(DAMAGE__TRACE_NPC)) {
        _log(DAMAGE__TRACE_NPC, "GetToHit - distance:%0.2f, range:%0.1f, falloff:%0.1f", distance, range, falloff);
        _log(DAMAGE__TRACE_NPC, "GetToHit - myVel:%0.3f, %0.3f, %0.3f", myVel.x, myVel.y, myVel.z);
        _log(DAMAGE__TRACE_NPC, "GetToHit - targVel:%0.3f, %0.3f, %0.3f", targVel.x, targVel.y, targVel.z);
        _log(DAMAGE__TRACE_NPC, "GetToHit - relativeVelocity:%0.3f, %0.3f, %0.3f", relativeVelocity.x, relativeVelocity.y, relativeVelocity.z);
        _log(DAMAGE__TRACE_NPC, "GetToHit - radialV:%0.3f", radialV);
        _log(DAMAGE__TRACE_NPC, "GetToHit - transversalV:%0.3f, angularVel:%0.3f tracking:%0.3f, targetSig:%0.1f, sigRes:%0.1f", \
                transversalV, angularVel, trackSpeed, targSig, sigRes);
        _log(DAMAGE__TRACE_NPC, "GetToHit - (hitChance:%0.4f > Rand:%0.4f) = %s", hitChance, rNum, \
                ((rNum <= sConfig.rates.NpcCritChance) ? "Crit" : (rNum < hitChance ? "Hit" : "Miss")));
    }
    if (rNum <= sConfig.rates.NpcCritChance)
        return 3.0f;
    if (rNum < hitChance)
        return static_cast<float>(rNum + 0.49);
    return 0.0f;
}

float TurretFormulas::GetDroneToHit(DroneSE* pDrone, SystemEntity* pTarget) {
    if (pTarget == nullptr)
        return 0;
    InventoryItemRef dRef = pDrone->GetSelf();
    double falloff = dRef->GetAttribute(AttrFalloff).get_double();
    Vector3d delta =  pTarget->GetPosition() - pDrone->GetPosition();
    double distance = delta.Length();
    Vector3d vector = pTarget->GetVelocity() - pDrone->GetVelocity();
    double transversalV = vector.Length();
    double a = (transversalV / (distance * dRef->GetAttribute(AttrTrackingSpeed).get_double()));
    double b = (dRef->GetAttribute(AttrOptimalSigRadius).get_double() / pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_float());
    double c = (a * b) * (a * b);
    double d = EvE::max(distance - dRef->GetAttribute(AttrEntityAttackRange).get_double(), 0.0f);
    double e = (d / falloff) * (d / falloff);
    double ChanceToHit = (std::pow(0.5, c + e));
    double rNum = MakeRandomDouble();
    if (rNum <= sConfig.rates.DroneCritChance)
        return 3.0f;
    if (rNum < ChanceToHit)
        return static_cast<float>(rNum + 0.49);
    // drones will have a minimum damage instead of zero
    return 0.1f;
}

float TurretFormulas::GetSentryToHit(Sentry* pSentry, SystemEntity* pTarget) {
    if (pTarget == nullptr)
        return 0;
    double sigRes = pSentry->GetSelf()->GetAttribute(AttrOptimalSigRadius).get_double();
    double falloff = pSentry->GetSelf()->GetAttribute(AttrFalloff).get_double();
    Vector3d delta =  pTarget->GetPosition() - pSentry->GetPosition();
    double distance = delta.Length();
    double targSig = pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_double();
    double a = (pTarget->GetVelocity().Length() / (distance * pSentry->GetSelf()->GetAttribute(AttrTrackingSpeed).get_double()));
    double b = (sigRes / targSig);
    double modifier = 1.0;
    if ((a < 1) and (b > 1)) {
        b = 1;
        modifier = (targSig / sigRes);
    }
    double c = (a * b) * (a * b);
    double d = EvE::max(distance - pSentry->GetSelf()->GetAttribute(AttrEntityAttackRange).get_double(), 0.0);
    double e = (d / falloff) * (d / falloff);
    double ChanceToHit = std::pow(0.5, c + e);
    double rNum = MakeRandomDouble();
    if (rNum <= sConfig.rates.SentryCritChance)
        return 3.0f;
    if (rNum < ChanceToHit)
        return static_cast<float>((rNum + 0.49) * modifier);
    return 0.0f;
}
