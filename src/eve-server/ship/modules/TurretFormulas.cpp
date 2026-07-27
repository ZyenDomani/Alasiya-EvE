
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
    double falloff = (range - pMod->GetAttribute(AttrFalloff).get_double());
    Vector3d lineOfSight =  pTarget->GetPosition() - shipRef->position();
    double distance = lineOfSight.Length();
    lineOfSight.Normalize();

    if (distance < 0.1)
        distance = 0.1;

    // calculate transversal from other data
    Vector3d relativeVelocity = pTarget->GetVelocity() - shipRef->GetPilot()->GetShipSE()->GetVelocity();
    // Isolate radial velocity component (along the line of sight)
    double radialV = relativeVelocity.dotProduct(lineOfSight);
    // Transversal velocity vector is total relative velocity minus the radial component
    Vector3d transversalVector = relativeVelocity - (lineOfSight * radialV);
    double transversalV = transversalVector.Length();
    double angularVel = transversalV / distance;
    double targSig = pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_double();
    double sigRes = pMod->GetAttribute(AttrOptimalSigRadius).get_double();
    double trackSpeed = pMod->GetAttribute(AttrTrackingSpeed).get_double();
    //  calculations for chance to hit  --UD 29May17
    /*ChanceToHit = 0.5 ^ ((((Transversal speed/(Range to target * Turret Tracking))*(Turret Signature Resolution / Target Signature Radius))^2)
     *                  + ((max(0, Range To Target - Turret Optimal Range))/Turret Falloff)^2)
     *
     *     a =  Transversal speed/(Range to target * Turret Tracking)
     *     b =  Turret Signature Resolution / Target Signature Radius
     *     c =  (a * b) ^ 2
     *     d =  max(0, distance - optimal range)
     *     e =  (d / falloff) ^ 2
     * tohit =  0.5 ^ (c + e)
     */
    float a = (angularVel / trackSpeed);
    float b = (sigRes / targSig);
    float modifier = 1.0f;
    if ((a < 1) and (b > 1)) {
        /* in cases where weapon can track target, but sigRes > targSig, the weapon would not hit on live but *should* hit with reduced damage
         * modify formula to remove Signature variable from equation, test toHit against tracking,
         * then use Signature variables to determine amount of damage reduction (i.e. large gun vs. small ship)
         */
        b = 1;
        modifier = (targSig / sigRes);
    }
    float c = (a * b) * (a * b);
    float d = EvE::max(distance - range, 0.0f);
    float e = (d / falloff) * (d / falloff);
    float ChanceToHit = std::pow(0.5, c + e);
    float rNum = MakeRandomFloat();
    if (is_log_enabled(DAMAGE__TRACE)) {
        _log(DAMAGE__TRACE, "Turret::GetToHit - distance:%0.2f, range:%0.2f, falloff:%0.2f", distance, range, falloff);
        _log(DAMAGE__TRACE, "Turret::GetToHit - transversalV:%.3f, angularV:%.3f, tracking:%.3f, targetSig:%.1f, sigRes:%.1f", \
                transversalV, angularVel, trackSpeed, targSig, sigRes);
        _log(DAMAGE__TRACE, "Turret::GetToHit - (%0.3f * %0.3f)^2 = c:%0.5f : (%0.3f / %0.1f)^2 = e:%0.5f", a, b, c, d, falloff, e);
        _log(DAMAGE__TRACE, "Turret::GetToHit - ChanceToHit: %0.5f  - Rand:%0.3f  - %s", ChanceToHit, rNum, \
                ((rNum <= sConfig.rates.PlayerCritChance) ? "Crit" : (rNum < ChanceToHit ? "Hit" : "Miss")));
    }
    if (rNum <= sConfig.rates.PlayerCritChance)
        return 3.0f;
    if (modifier < 0.01f)
        modifier = 1.0f;
    if (rNum < ChanceToHit)
        return (rNum + 0.49f) * modifier;
    return 0.0f;
}

float TurretFormulas::GetNPCToHit(NPC* pNPC, SystemEntity* pTarget) {
    if (pTarget == nullptr)
        return 0;

    double sigRes = pNPC->GetSelf()->GetAttribute(AttrOptimalSigRadius).get_double();
    uint16 range = pNPC->GetAI()->GetOptimalRange();
    int32 falloff = pNPC->GetAI()->GetFalloff();
    double trackSpeed = pNPC->GetAI()->GetTrackingSpeed();
    double targSig = pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_double();
    Vector3d lineOfSight =  pTarget->GetPosition() - pNPC->GetPosition();
    double distance = lineOfSight.Length();
    lineOfSight.Normalize();

    if (distance < 0.1)
        distance = 0.1;

    // calculate transversal from other data
    Vector3d relativeVelocity = pTarget->GetVelocity() - pNPC->GetVelocity();
    // Isolate radial velocity component (along the line of sight)
    float radialV = relativeVelocity.dotProduct(lineOfSight);
    // Transversal velocity vector is total relative velocity minus the radial component
    Vector3d transversalVector = relativeVelocity - (lineOfSight * radialV);
    double transversalV = transversalVector.Length();
    double angularVel = transversalV / distance;

    float a = (angularVel / trackSpeed);
    float b = (sigRes / targSig);
    float modifier = 1.0f;
    if ((a < 1) and (b > 1)) {
        b = 1;
        modifier = (targSig / sigRes);
    }
    float c = (a * b) * (a * b);
    float d = EvE::max(distance - range, 0.0f);
    float e = (d / falloff) * (d / falloff);
    float ChanceToHit = std::pow(0.5, c + e);
    float rNum = MakeRandomFloat();
    if (is_log_enabled(DAMAGE__TRACE_NPC)) {
        _log(DAMAGE__TRACE_NPC, "NPC::GetToHit - distance:%.2f, range:%.u, falloff:%i", distance, range, falloff);
        _log(DAMAGE__TRACE_NPC, "NPC::GetToHit - transversalV:%.3f, angularVel:%.3f tracking:%.3f, targetSig:%.1f, sigRes:%u", \
                transversalV, angularVel, trackSpeed, targSig, sigRes);
        _log(DAMAGE__TRACE_NPC, "NPC::GetToHit - (%.3f * %.3f)^2 = c:%.5f : (%.3f / %i)^2 = e:%.5f", a, b, c, d, falloff, e);
        _log(DAMAGE__TRACE_NPC, "NPC::GetToHit - ChanceToHit:%f, Rand:%.3f - %s", ChanceToHit, rNum, \
                ((rNum <= sConfig.rates.NpcCritChance) ? "Crit" : (rNum < ChanceToHit ? "Hit" : "Miss")));
    }
    if (rNum <= sConfig.rates.NpcCritChance)
        return 3.0f;
    if (modifier < 0.01f)
        modifier = 1.0f;
    if (rNum < ChanceToHit)
        return (rNum + 0.49f) * modifier;
    return 0.0f;
}

float TurretFormulas::GetDroneToHit(DroneSE* pDrone, SystemEntity* pTarget) {
    if (pTarget == nullptr)
        return 0;
    InventoryItemRef dRef = pDrone->GetSelf();
    float falloff(dRef->GetAttribute(AttrFalloff).get_float());
    Vector3d delta =  pTarget->GetPosition() - pDrone->GetPosition();
    double distance = delta.Length();
    Vector3d vector = pTarget->GetVelocity() - pDrone->GetVelocity();
    double transversalV = vector.Length();
    double a = (transversalV / (distance * dRef->GetAttribute(AttrTrackingSpeed).get_float()));
    float b = (dRef->GetAttribute(AttrOptimalSigRadius).get_float() / pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_float());
    float c = (a * b) * (a * b);
    double d(EvE::max(distance - dRef->GetAttribute(AttrEntityAttackRange).get_float(), 0.0f));
    float e = (d / falloff) * (d / falloff);
    float ChanceToHit = (std::pow(0.5, c + e));
    float rNum = (MakeRandomFloat(0.0, 1.0));
    if (rNum <= sConfig.rates.DroneCritChance)
        return 3.0f;
    if (rNum < ChanceToHit)
        return (rNum + 0.49);
    // drones will have a minimum damage instead of zero
    return 0.1;
}

float TurretFormulas::GetSentryToHit(Sentry* pSentry, SystemEntity* pTarget) {
    if (pTarget == nullptr)
        return 0;
    float sigRes = pSentry->GetSelf()->GetAttribute(AttrOptimalSigRadius).get_float();
    float falloff = pSentry->GetSelf()->GetAttribute(AttrFalloff).get_float();
    Vector3d delta =  pTarget->GetPosition() - pSentry->GetPosition();
    double distance = delta.Length();
    float targSig = pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_float();
    double a = (pTarget->GetVelocity().Length() / (distance * pSentry->GetSelf()->GetAttribute(AttrTrackingSpeed).get_float()));
    float b = (sigRes / targSig);
    float modifier = 1.0f;
    if ((a < 1) and (b > 1)) {
        b = 1;
        modifier = (targSig / sigRes);
    }
    float c = (a * b) * (a * b);
    float d = EvE::max(distance - pSentry->GetSelf()->GetAttribute(AttrEntityAttackRange).get_float(), 0.0f);
    float e = (d / falloff) * (d / falloff);
    float ChanceToHit = std::pow(0.5, c + e);
    float rNum = MakeRandomFloat(0.0, 1.0);
    if (rNum <= sConfig.rates.SentryCritChance)
        return 3.0f;
    if (rNum < ChanceToHit)
        return (rNum + 0.49) * modifier;
    return 0;
}
