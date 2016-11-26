
 /**
  * @name TurrentFormulas.cpp
  *   forumlas for turrent tracking, to hit, and other specific things
  * @Author:         Allan
  * @date:   10 June 2015
  */


#include "character/Character.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "npc/Drone.h"
#include "ship/modules/weapon_modules/TurrentFormulas.h"
#include "ship/modules/TurrentModule.h"


float TurrentFormulas::GetToHit(ShipItemRef shipRef, TurrentModule* pMod, SystemEntity* pTarget)
{
    if (!pTarget)
        return 0;
    uint32 falloff = pMod->GetFalloff();
    double range = pMod->GetMaxRange();
    double distance = shipRef->position().distance(pTarget->DestinyMgr()->GetPosition());

    _log(DAMAGE__TRACE, "Turrent::GetToHit - distance:%.2f, range:%.2f, falloff:%u", distance, range, falloff);
    GPoint vel = pTarget->GetVelocity();
    double speed = vel.length();
    double angVelocity = (speed /distance);
    _log(DAMAGE__TRACE, "Turrent::GetToHit - speed/dist=angVelocity: %.3f / %.3f = %.3f", speed, distance, angVelocity);

    //  calculations for chance to hit
    /*     a =  angVelocity/(distance * tracking)
     *     b =  turrent sig res / target sig radius
     *     c =  (a * b) ^ 2
     *     d =  max(0, distance - optimal range)
     *     e =  (d / falloff) ^ 2
     * tohit =  0.5 ^ (c + e)    **NOTE**  e=0 when distance < range
     */
    double a = (angVelocity / (distance * pMod->GetTrackingSpeed()));
    double b = (pMod->GetSigRadius() / pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_double());
    double c = pow((a * b), 2);
    double e = 0;
    if (distance > range) {
        double d = EvE::max(distance - range);
        e = pow((d / falloff), 2);
    }

    float ChanceToHit = pow(0.5, c + e);
    double rNum = MakeRandomFloat(0.0, 1.0);
    _log(DAMAGE__TRACE, "Turrent::GetToHit - ChanceToHit:%f, Rand:%.3f  (c:%.5f + e:%.5f)", ChanceToHit, rNum, c, e);
    if (rNum <= 0.02)
        return 3.0f;
    else if (rNum < ChanceToHit)
        return (rNum + 0.49);
    else
        return 0;
}

float TurrentFormulas::GetNPCToHit(NPC* pNPC, SystemEntity* pTarget)
{
    if (!pTarget)
        return 0;
    uint16 range = pNPC->GetAIMgr()->GetMaxRange();
    uint32 falloff = pNPC->GetAIMgr()->GetFalloff();
    double distance = pNPC->DestinyMgr()->GetPosition().distance(pTarget->DestinyMgr()->GetPosition());
    _log(DAMAGE__TRACE, "NPC::GetToHit - distance:%.2f, range:%.u, falloff:%u", distance, range, falloff);

    GPoint vel = pTarget->GetVelocity();
    double speed = vel.length();
    double angVelocity = (speed /distance);
    _log(DAMAGE__TRACE, "NPC::GetToHit - speed/distance=angVelocity: %.3f / %.3f = %.3f", speed, distance, angVelocity);

    double a = (angVelocity / (distance * pNPC->GetAIMgr()->GetTrackingSpeed()));
    double b = (pNPC->GetSelf()->GetAttribute(AttrOptimalSigRadius).get_double() / pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_double());
    double c = pow((a * b), 2);
    double e = 0;
    if (distance > range) {
        double d = EvE::max(distance - range);
        e = pow((d / falloff), 2);
    }

    float ChanceToHit = pow(0.5, c + e);
    double rNum = MakeRandomFloat(0.0, 1.0);
    _log(DAMAGE__TRACE, "NPC::GetToHit - ChanceToHit:%f, Rand:%.3f  (c:%.5f + e:%.5f)", ChanceToHit, rNum, c, e);
    if (rNum <= 0.015)
        return 3.0f;
    else if (rNum < ChanceToHit)
        return rNum;
    else
        return 0;
}

float TurrentFormulas::GetDroneToHit(Drone* pDrone, SystemEntity* pTarget)
{
    if (!pTarget)
        return 0;
    double range = pDrone->GetSelf()->GetAttribute(AttrEntityAttackRange).get_double();
    double falloff = pDrone->GetSelf()->GetAttribute(AttrFalloff).get_double();
    double distance = pDrone->DestinyMgr()->GetPosition().distance(pTarget->DestinyMgr()->GetPosition());
    _log(DAMAGE__TRACE, "Drone::GetToHit - distance:%.2f, range:%.2f, falloff:%.1f", distance, range, falloff);

    GPoint vel = pTarget->GetVelocity();
    double speed = vel.length();
    double angVelocity = (speed /distance);
    _log(DAMAGE__TRACE, "Drone::GetToHit - speed/dist=angVelocity: %.3f / %.3f = %.3f", speed, distance, angVelocity);

    double a = (angVelocity / (distance * pDrone->GetSelf()->GetAttribute(AttrTrackingSpeed).get_double()));
    double b = (pDrone->GetSelf()->GetAttribute(AttrOptimalSigRadius).get_double() / pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_double());
    double c = pow((a * b), 2);
    double e = 0;
    if (distance > range) {
        double d = EvE::max(distance - range);
        e = pow((d / falloff), 2);
    }

    float ChanceToHit = pow(0.5, c + e);
    double rNum = MakeRandomFloat(0.0, 1.0);
    _log(DAMAGE__TRACE, "Drone::GetToHit - ChanceToHit:%f, Rand:%.3f", ChanceToHit, rNum);
    if (rNum <= 0.03)
        return 3.0f;
    else if (rNum < ChanceToHit)
        return rNum;
    else
        return 0;
}

