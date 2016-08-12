
 /**
  * @name TurrentFormulas.cpp
  *   forumlas for turrent tracking, to hit, and other specific things
  * @Author:         Allan
  * @date:   10 June 2015
  */


#include "character/Character.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "ship/Drone.h"
#include "ship/modules/weapon_modules/TurrentFormulas.h"
#include "ship/modules/TurrentModule.h"


float TurrentFormulas::GetToHit(ShipItemRef shipRef, TurrentModule* pMod, SystemEntity* pTarget)
{
    double range = pMod->GetMaxRange();
    double distance = shipRef->position().distance(pTarget->DestinyMgr()->GetPosition());

    _log(TARGET__MESSAGE, "Turrent::GetToHit - distance:%.2f, range:%.2f", distance, range);
    GPoint vel = pTarget->GetVelocity();
    double speed = vel.length();
    double angVelocity = (speed /distance);
    _log(TARGET__MESSAGE, "Turrent::GetToHit - speed/dist=angVelocity: %.3f / %.3f = %.3f", speed, distance, angVelocity);

    //  calculations for chance to hit
    /*     a =  angVelocity/(distance * tracking)
     *     b =  turrent sig res / target sig radius
     *     c =  (a * b) ^ 2
     *     d =  max(0, distance - optimal range)
     *     e =  (d / falloff) ^ 2
     * tohit =  0.5 ^ (c + e)    **NOTE**  e=0 when distance < range
     */
    double a = (angVelocity / (distance * pMod->GetTrackingSpeed()));
    double b = (pMod->GetSigRadius() / pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_float());
    double c = pow((a * b), 2);
    double e = 0;
    if (distance > range) {
        double d = _max(distance - range);
        e = pow((d /  pMod->GetFalloff()), 2);
    }

    float ChanceToHit = pow(0.5, c + e);
    double rNum = MakeRandomFloat(0.0, 1.0);
    _log(TARGET__MESSAGE, "Turrent::GetToHit - ChanceToHit:%f, Rand:%.3f", ChanceToHit, rNum);
    if (rNum <= 0.015)
        return 3.0f;
    else if (rNum < ChanceToHit)
        return (rNum + 0.49);
    else
        return 0;
}

float TurrentFormulas::GetNPCToHit(NPC* pNPC, SystemEntity* pTarget)
{
    uint16 range = pNPC->GetAIMgr()->GetMaxRange();
    double distance = pNPC->DestinyMgr()->GetPosition().distance(pTarget->DestinyMgr()->GetPosition());
    _log(TARGET__MESSAGE, "NPC::GetToHit - distance:%.2f, range:%.u", distance, range);

    GPoint vel = pTarget->GetVelocity();
    double speed = vel.length();
    double angVelocity = (speed /distance);
    _log(TARGET__MESSAGE, "NPC::GetToHit - speed/dist=angVelocity: %.3f / %.3f = %.3f", speed, distance, angVelocity);

    double a = (angVelocity / (distance * pNPC->GetAIMgr()->GetTrackingSpeed()));
    double b = (pNPC->GetSelf()->GetAttribute(AttrOptimalSigRadius).get_float() / pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_float());
    double c = pow((a * b), 2);
    double e = 0;
    if (distance > range) {
        double d = _max(distance - range);
        e = pow((d / pNPC->GetAIMgr()->GetFalloff()), 2);
    }

    float ChanceToHit = pow(0.5, c + e);
    double rNum = MakeRandomFloat(0.0, 1.0);
    _log(TARGET__MESSAGE, "NPC::GetToHit - ChanceToHit:%f, Rand:%.3f", ChanceToHit, rNum);
    if (rNum <= 0.015)
        return 3.0f;
    else if (rNum < ChanceToHit)
        return (rNum + 0.49);
    else
        return 0;
}

float TurrentFormulas::GetDroneToHit(Drone* pDrone, SystemEntity* pTarget)
{
    double range = pDrone->GetSelf()->GetAttribute(AttrEntityAttackRange).get_float();
    double distance = pDrone->DestinyMgr()->GetPosition().distance(pTarget->DestinyMgr()->GetPosition());
    _log(TARGET__MESSAGE, "Drone::GetToHit - distance:%.2f, range:%.2f", distance, range);

    GPoint vel = pTarget->GetVelocity();
    double speed = vel.length();
    double angVelocity = (speed /distance);
    _log(TARGET__MESSAGE, "Drone::GetToHit - speed/dist=angVelocity: %.3f / %.3f = %.3f", speed, distance, angVelocity);

    double a = (angVelocity / (distance * pDrone->GetSelf()->GetAttribute(AttrTrackingSpeed).get_float()));
    double b = (pDrone->GetSelf()->GetAttribute(AttrOptimalSigRadius).get_float() / pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_float());
    double c = pow((a * b), 2);
    double e = 0;
    if (distance > range) {
        double d = _max(distance - range);
        e = pow((d /  pDrone->GetSelf()->GetAttribute(AttrFalloff).get_float()), 2);
    }

    float ChanceToHit = pow(0.5, c + e);
    double rNum = MakeRandomFloat(0.0, 1.0);
    _log(TARGET__MESSAGE, "Drone::GetToHit - ChanceToHit:%f, Rand:%.3f", ChanceToHit, rNum);
    if (rNum <= 0.015)
        return 3.0f;
    else if (rNum < ChanceToHit)
        return (rNum + 0.49);
    else
        return 0;
}

double TurrentFormulas::_max(double x)
{
    if (x > 0)
        return x;
    else
        return 0;
}
