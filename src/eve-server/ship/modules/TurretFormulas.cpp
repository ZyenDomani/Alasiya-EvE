
 /**
  * @name TurretFormulas.cpp
  *   forumlas for turret tracking, to hit, and other specific things
  * @Author:         Allan
  * @date:   10 June 2015
  */

 /* current crit chances
  *      NPC  - 1.5%
  *   turret  - 2%
  *    Drone  - 3%
  */

#include "character/Character.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "npc/Drone.h"
#include "npc/Sentry.h"
#include "ship/modules/TurretFormulas.h"
#include "ship/modules/TurretModule.h"


float TurretFormulas::GetToHit(ShipItemRef shipRef, TurretModule* pMod, SystemEntity* pTarget)
{
    if (!pTarget)
        return 0;
    uint32 falloff = pMod->GetAttribute(AttrFalloff).get_int();
    double range = pMod->GetAttribute(AttrMaxRange).get_int();
    double distance = shipRef->position().distance(pTarget->DestinyMgr()->GetPosition());
    float modTrackSpeed = pMod->GetAttribute(AttrTrackingSpeed).get_float();
    _log(DAMAGE__TRACE, "Turret::GetToHit - distance:%.2f, range:%.2f, falloff:%u", distance, range, falloff);

    // calculate transversal from other data
    /* i have had problems finding exact data for transversal velocity
     * ideas/data taken from https://wiki.eveuniversity.org/Velocity
     * The transversal velocity is computed by subtracting the two velocity vectors from one another, and then finding the length of the vector.
     * angular velocity = transversal velocity / distance
     */
    GVector vel = pTarget->GetVelocity();
    double speed = vel.length();
    GVector vector = vel - shipRef->GetPilot()->GetShipSE()->GetVelocity();
    double transversalV = vector.length();
    double angularVel = transversalV / distance;
    float ChanceToHit = 0;
    if (!angularVel)
        angularVel = pTarget->DestinyMgr()->GetRadTic();
    _log(DAMAGE__TRACE, "Turret::GetToHit - angularVel:%.3f tracking:%.3f", angularVel, modTrackSpeed);
    if (!angularVel) {
        // cant get angular.  hack ToHit
        if (distance < range)
            ChanceToHit = MakeRandomFloat(0, 0.5);  // hack a 20% chance to hit when !angularVel and targ is inside weapon range
    } else {
        //  calculations for chance to hit  --UD 23April17
        /*     a =  angVelocity * 40000
         *     b =  turret tracking * target sig radius
         *     c =  (a / b) ^ 2
         *     d =  max(0, distance - optimal range)
         *     e =  (d / falloff) ^ 2
         * tohit =  0.5 ^ (c + e)
         */
        double a = (angularVel * 40000);
        double b = (modTrackSpeed * pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_float());
        double c = pow((a / b), 2);
        double d = EvE::max(distance - range);
        double e = pow((d / falloff), 2);
        ChanceToHit = pow(0.5, c + e);
        _log(DAMAGE__TRACE, "Turret::GetToHit - (%.3f/%.3f)^2 = %.5f + e:%.5f", a, b, c, e);
    }
    if (ChanceToHit == 0)
        return 0;
    double rNum = MakeRandomFloat(0.0, 1.0);
    _log(DAMAGE__TRACE, "Turret::GetToHit - ChanceToHit:%f, Rand:%.3f", ChanceToHit, rNum);
    if (rNum <= 0.02)
        return 3.0f;
    else if (rNum < ChanceToHit)
        return (rNum + 0.49);
    else
        return 0;
}

float TurretFormulas::GetNPCToHit(NPC* pNPC, SystemEntity* pTarget)
{
    if (!pTarget)
        return 0;
    uint16 range = pNPC->GetAIMgr()->GetMaxRange();
    uint32 falloff = pNPC->GetAIMgr()->GetFalloff();
    double distance = pNPC->DestinyMgr()->GetPosition().distance(pTarget->DestinyMgr()->GetPosition());
    float trackSpeed = pNPC->GetAIMgr()->GetTrackingSpeed();
    _log(DAMAGE__TRACE_NPC, "NPC::GetToHit - distance:%.2f, range:%.u, falloff:%u", distance, range, falloff);

    GVector vel = pTarget->GetVelocity();
    double speed = vel.length();
    GVector vector = vel - pNPC->GetVelocity();
    double transversalV = vector.length();
    double angularVel = transversalV / distance;
    float ChanceToHit = 0;
    if (!angularVel)
        angularVel = pTarget->DestinyMgr()->GetRadTic();
    _log(DAMAGE__TRACE_NPC, "NPC::GetToHit - angularVel:%.3f tracking:%.3f", angularVel, trackSpeed);
    if (!angularVel) {
        // cant get angular.  hack ToHit
        if (distance < range)
            ChanceToHit = MakeRandomFloat(0, 0.5);  // hack a 20% chance to hit when !angularVel and targ is inside weapon range
    } else {
        double a = (angularVel * 40000);
        double b = (trackSpeed * pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_float());
        double c = pow((a / b), 2);
        double d = EvE::max(distance - range);
        double e = pow((d / falloff), 2);
        ChanceToHit = pow(0.5, c + e);
        _log(DAMAGE__TRACE_NPC, "NPC::GetToHit - (%.3f/%.3f)^2 = %.5f + e:%.5f", a, b, c, e);
    }
    if (ChanceToHit == 0)
        return 0;
    double rNum = MakeRandomFloat(0.0, 1.0);
    _log(DAMAGE__TRACE_NPC, "NPC::GetToHit - ChanceToHit:%f, Rand:%.3f", ChanceToHit, rNum);
    if (rNum <= 0.015)
        return 3.0f;
    else if (rNum < ChanceToHit)
        return rNum;
    else
        return 0;
}

float TurretFormulas::GetDroneToHit(Drone* pDrone, SystemEntity* pTarget)
{
    if (!pTarget)
        return 0;
    double falloff = pDrone->GetSelf()->GetAttribute(AttrFalloff).get_double();
    double distance = pDrone->DestinyMgr()->GetPosition().distance(pTarget->DestinyMgr()->GetPosition());

    GVector vel = pTarget->GetVelocity();
    double speed = vel.length();
    double angularVelDest = pTarget->DestinyMgr()->GetRadTic();
    GVector vector = vel - pDrone->GetVelocity();
    double transversalV = vector.length();
    double angularVel = transversalV / distance;

    double a = (angularVel * 40000);
    double b = (pDrone->GetSelf()->GetAttribute(AttrTrackingSpeed).get_float() * pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_float());
    double c = pow((a / b), 2);
    double d = EvE::max(distance - pDrone->GetSelf()->GetAttribute(AttrEntityAttackRange).get_double());
    double e = pow((d / falloff), 2);

    float ChanceToHit = pow(0.5, c + e);
    if (ChanceToHit == 0)
        return 0;
    double rNum = MakeRandomFloat(0.0, 1.0);
    if (rNum <= 0.03)
        return 3.0f;
    else if (rNum < ChanceToHit)
        return rNum;
    else
        return 0;
}

float TurretFormulas::GetSentryToHit(Sentry* pSentry, SystemEntity* pTarget)
{
    if (!pTarget)
        return 0;
    double falloff = pSentry->GetSelf()->GetAttribute(AttrFalloff).get_double();
    double distance = pSentry->DestinyMgr()->GetPosition().distance(pTarget->DestinyMgr()->GetPosition());

    GVector vel = pTarget->GetVelocity();
    double speed = vel.length();
    double angularVelDest = pTarget->DestinyMgr()->GetRadTic();
    double angularVel = speed / distance;

    double a = (angularVel * 40000);
    double b = (pSentry->GetSelf()->GetAttribute(AttrTrackingSpeed).get_float() * pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_float());
    double c = pow((a / b), 2);
    double d = EvE::max(distance - pSentry->GetSelf()->GetAttribute(AttrEntityAttackRange).get_double());
    double e = pow((d / falloff), 2);

    float ChanceToHit = pow(0.5, c + e);
    if (ChanceToHit == 0)
        return 0;
    double rNum = MakeRandomFloat(0.0, 1.0);
    if (rNum <= 0.03)
        return 3.0f;
    else if (rNum < ChanceToHit)
        return rNum;
    else
        return 0;
}
