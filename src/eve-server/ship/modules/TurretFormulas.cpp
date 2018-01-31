
 /**
  * @name TurretFormulas.cpp
  *   forumlas for turret tracking, to hit, and other specific things
  * @Author:         Allan
  * @date:   10 June 2015
  */

 /* current crit chances
  *      NPC  - 1.5%
  *   turret  - 2%
  *   Sentry  - 2%
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
    float sigRes = pMod->GetAttribute(AttrOptimalSigRadius).get_float();
    uint32 falloff = pMod->GetAttribute(AttrFalloff).get_int();
    uint32 range = pMod->GetAttribute(AttrMaxRange).get_int();
    float distance = shipRef->position().distance(pTarget->DestinyMgr()->GetPosition());
    float trackSpeed = pMod->GetAttribute(AttrTrackingSpeed).get_float();
    _log(DAMAGE__TRACE, "Turret::GetToHit - distance:%.2f, range:%.2f, falloff:%u", distance, range, falloff);

    // calculate transversal from other data
    /* i have had problems finding exact data for transversal velocity
     * ideas/data taken from https://wiki.eveuniversity.org/Velocity
     * The transversal velocity is computed by subtracting the two velocity vectors from one another, and then finding the length of the vector.
     * angular velocity = transversal velocity / distance
     */
    GVector vector = pTarget->GetVelocity() - shipRef->GetPilot()->GetShipSE()->GetVelocity();
    double transversalV = vector.length();
    double angularVel = transversalV / distance;
    float targSig = pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_float();
    _log(DAMAGE__TRACE, "Turret::GetToHit - transversalV:%.3f tracking:%.3f, targetSig:%.1f, sigRes:%.1f", transversalV, trackSpeed, targSig, sigRes);
    //  calculations for chance to hit  --UD 29May17
    /*ChanceToHit = 0.5 ^ ((((Transversal speed/(Range to target * Turret Tracking))*(Turret Signature Resolution / Target Signature Radius))^2)
     * + ((max(0, Range To Target - Turret Optimal Range))/Turret Falloff)^2)
     *
     *     a =  Transversal speed/(Range to target * Turret Tracking
     *     b =  Turret Signature Resolution / Target Signature Radius
     *     c =  (a * b) ^ 2
     *     d =  max(0, distance - optimal range)
     *     e =  (d / falloff) ^ 2
     * tohit =  0.5 ^ (c + e)
     */
    double a = (angularVel / trackSpeed);
    double b = (sigRes / targSig);
    float modifier = 0.0f;
    if ((a < 1) and (b > 1)) {
        /* in cases where weapon can track target, but sigRes > targSig, the weapon would not hit on live but *should* hit with reduced damage
         * modify formula to remove Signature varaible from equation, test toHit against tracking,
         * then use Signature variables to determine amount of damage reduction (i.e. large gun vs. small ship)
         */
        b = 1;
        modifier = (targSig / sigRes);
    }
    double c = pow((a * b), 2);
    double d = EvE::max(distance - range);
    double e = pow((d / falloff), 2);
    double x = pow(0.5, c);
    double y = pow(0.5, e);
    float ChanceToHit = x * y;
    _log(DAMAGE__TRACE, "Turret::GetToHit - (%.3f * %.3f)^2 = c:%.5f : (%.3f / %u)^2 = e:%.5f", a, b, c, d, falloff, e);
    float rNum = MakeRandomFloat(0.0, 1.0);
    _log(DAMAGE__TRACE, "Turret::GetToHit - %f * %f = %.5f  - Rand:%.3f  - %s", x, y, ChanceToHit, rNum, ((rNum <= 0.015) ? "Crit" : (rNum < ChanceToHit ? "Hit" : "Miss")));
    if (rNum <= 0.02)
        return 3.0f;
    else if (rNum < ChanceToHit) {
        if (modifier)
            return modifier;
        else
            return (rNum + 0.49);
    } else
        return 0;
}

float TurretFormulas::GetNPCToHit(NPC* pNPC, SystemEntity* pTarget)
{
    if (!pTarget)
        return 0;
    uint16 sigRes = pNPC->GetAIMgr()->GetSigRes();
    uint16 range = pNPC->GetAIMgr()->GetOptimalRange();
    uint32 falloff = pNPC->GetAIMgr()->GetFalloff();
    double distance = pNPC->DestinyMgr()->GetPosition().distance(pTarget->DestinyMgr()->GetPosition());
    float trackSpeed = pNPC->GetAIMgr()->GetTrackingSpeed();
    float targSig = pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_float();
    _log(DAMAGE__TRACE_NPC, "NPC::GetToHit - distance:%.2f, range:%.u, falloff:%u", distance, range, falloff);

    GVector vector = pTarget->GetVelocity() - pNPC->GetVelocity();
    double transversalV = vector.length();
    double angularVel = transversalV / distance;
    _log(DAMAGE__TRACE_NPC, "NPC::GetToHit - angularVel:%.3f tracking:%.3f, targetSig:%.1f, sigRes:%u", angularVel, trackSpeed, targSig, sigRes);

    double a = (angularVel / trackSpeed);
    double b = (sigRes / targSig);
    float modifier = 0.0f;
    if ((a < 1) and (b > 1)) {
        b = 1;
        modifier = (targSig / sigRes);
    }
    double c = pow((a * b), 2);
    double d = EvE::max(distance - range);
    double e = pow((d / falloff), 2);
    double x = pow(0.5, c);
    double y = pow(0.5, e);
    float ChanceToHit = x * y;
    _log(DAMAGE__TRACE_NPC, "NPC::GetToHit - (%.3f * %.3f)^2 = c:%.5f : (%.3f / %u)^2 = e:%.5f", a, b, c, d, falloff, e);
    _log(DAMAGE__TRACE_NPC, "NPC::GetToHit - %f * %f = %.5f", x, y, ChanceToHit);
    float rNum = MakeRandomFloat(0.0, 1.0);
    _log(DAMAGE__TRACE_NPC, "NPC::GetToHit - ChanceToHit:%f, Rand:%.3f - %s", ChanceToHit, rNum, ((rNum <= 0.015) ? "Crit" : (rNum < ChanceToHit ? "Hit" : "Miss")));
    if (rNum <= 0.015)
        return 3.0f;
    else if (rNum < ChanceToHit) {
        if (modifier)
            return modifier;
        else
            return (rNum + 0.49);
    } else
        return 0;
}

float TurretFormulas::GetDroneToHit(Drone* pDrone, SystemEntity* pTarget)
{
    if (!pTarget)
        return 0;
    double falloff = pDrone->GetSelf()->GetAttribute(AttrFalloff).get_double();
    double distance = pDrone->DestinyMgr()->GetPosition().distance(pTarget->DestinyMgr()->GetPosition());
    GVector vector = pTarget->GetVelocity() - pDrone->GetVelocity();
    double transversalV = vector.length();
    double a = (transversalV / (distance * pDrone->GetSelf()->GetAttribute(AttrTrackingSpeed).get_float()));
    double b = (pDrone->GetSelf()->GetAttribute(AttrOptimalSigRadius).get_float() / pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_float());
    double c = pow((a * b), 2);
    double d = EvE::max(distance - pDrone->GetSelf()->GetAttribute(AttrEntityAttackRange).get_double());
    double e = pow((d / falloff), 2);
    float ChanceToHit = pow(0.5, c + e);
    float rNum = MakeRandomFloat(0.0, 1.0);
    if (rNum <= 0.03)
        return 3.0f;
    else if (rNum < ChanceToHit)
        return (rNum + 0.49);
    else
        return 0;
}

float TurretFormulas::GetSentryToHit(Sentry* pSentry, SystemEntity* pTarget)
{
    if (!pTarget)
        return 0;
    float sigRes = pSentry->GetSelf()->GetAttribute(AttrOptimalSigRadius).get_float();
    double falloff = pSentry->GetSelf()->GetAttribute(AttrFalloff).get_double();
    double distance = pSentry->GetPosition().distance(pTarget->DestinyMgr()->GetPosition());
    float targSig = pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_float();
    double a = (pTarget->GetVelocity().length() / (distance * pSentry->GetSelf()->GetAttribute(AttrTrackingSpeed).get_float()));
    double b = (sigRes / targSig);
    float modifier = 0.0f;
    if ((a < 1) and (b > 1)) {
        b = 1;
        modifier = (targSig / sigRes);
    }
    double c = pow((a * b), 2);
    double d = EvE::max(distance - pSentry->GetSelf()->GetAttribute(AttrEntityAttackRange).get_double());
    double e = pow((d / falloff), 2);
    float ChanceToHit = pow(0.5, c + e);
    float rNum = MakeRandomFloat(0.0, 1.0);
    if (rNum <= 0.02)
        return 3.0f;
    else if (rNum < ChanceToHit) {
        if (modifier)
            return modifier;
        else
            return (rNum + 0.49);
    } else
        return 0;
}
