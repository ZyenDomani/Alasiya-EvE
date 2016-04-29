
 /**
  * @name TurrentFormulas.cpp
  *   forumlas for turrent tracking, to hit, and other specific things
  * @Author:         Allan
  * @date:   10 June 2015
  */


#include "ship/modules/weapon_modules/TurrentFormulas.h"


float TurrentFormulas::GetToHit(ShipItemRef shipRef, InventoryItemRef ItemRef, SystemEntity* pTarget)
{
    float ChanceToHit = 1.0f;

    SystemEntity* pSE = shipRef->GetPilot()->GetShipSE();
    double range = ItemRef->GetAttribute(AttrMaxRange).get_float();
    double distance = shipRef->position().distance(pTarget->GetSelf()->position());

    double trackingSpeed = ItemRef->GetAttribute(AttrTrackingSpeed).get_float();
    double falloff = ItemRef->GetAttribute(AttrFalloff).get_float();

    Character* pChar = shipRef->GetPilot()->GetChar().get();
    range *= (1 + ( 0.05 * (pChar->GetSkillLevel(skillSharpshooter, true))));      //  5% increase in optimal range
    falloff *= (1 + ( 0.05 * (pChar->GetSkillLevel(skillTrajectoryAnalysis, true))));  //  5% increase in falloff

    if (distance <= range)
        return ChanceToHit;

    //TODO no fukin clue how to get module modifiers to falloff and range and implement here....

    GPoint vel = pTarget->GetVelocity();
    double speed = vel.length();
    double angVelocity = (speed /distance);

    //  calculations for chance to hit
    /*     a =  angVelocity/(distance * tracking)
     *     b =  turrent sig res / target sig radius
     *     c =  (a * b) ^ 2
     *     d =  max(0, distance - optimal range)
     *     e =  (d / falloff) ^ 2
     * tohit =  0.5 ^ (c + e)
     */
    double a = (angVelocity / (distance * trackingSpeed));
    double b = (ItemRef->GetAttribute(AttrOptimalSigRadius).get_float() / pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_float());
    double c = pow((a * b), 2);

    double d = _max(distance - range);
    double e = pow((d / falloff), 2);

    ChanceToHit = (pow(0.5, c) + pow(0.5, e));

    double rnd_number = MakeRandomFloat(0.0, 1.0);
    if (rnd_number <= 0.015)
        return 3.0f;
    else if (rnd_number < ChanceToHit)
        return (rnd_number + 0.49);
    else
        return 0;
}

float TurrentFormulas::GetNPCToHit(NPC* pNPC, SystemEntity* pTarget)
{
    double range = pNPC->GetSelf()->GetAttribute(AttrMaxRange).get_float();
    range += pNPC->GetSelf()->GetAttribute(AttrEntityFlyRange).get_float();
    float ChanceToHit = 1.0f;

    double distance = pNPC->GetSelf()->position().distance(pTarget->GetSelf()->position());

    if (distance <= range)
        return ChanceToHit;

    double trackingSpeed = pNPC->GetSelf()->GetAttribute(AttrTrackingSpeed).get_float();
    double falloff = pNPC->GetSelf()->GetAttribute(AttrFalloff).get_float();

    GPoint vel = pTarget->GetVelocity();
    double speed = vel.length();
    double angVelocity = (speed /distance);

    double a = (angVelocity / (distance * trackingSpeed));
    double b = (pNPC->GetSelf()->GetAttribute(AttrOptimalSigRadius).get_float() / pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_float());
    double c = pow((a * b), 2);

    double d = _max(distance - range);
    double e = pow((d / falloff), 2);

    ChanceToHit = (pow(0.5, c) + pow(0.5, e));

    double rnd_number = MakeRandomFloat(0.0, 1.0);
    if (rnd_number <= 0.015)
        return 3.0f;
    else if (rnd_number < ChanceToHit)
        return (rnd_number + 0.49);
    else
        return 0;
}

float TurrentFormulas::GetDroneToHit(Drone* pDrone, SystemEntity* pTarget)
{
    double range = pDrone->GetSelf()->GetAttribute(AttrMaxRange).get_float();
    range += pDrone->GetSelf()->GetAttribute(AttrEntityFlyRange).get_float();
    float ChanceToHit = 1.0f;

    double distance = pDrone->GetSelf()->position().distance(pTarget->GetSelf()->position());

    if (distance <= range)
        return ChanceToHit;

    double trackingSpeed = pDrone->GetSelf()->GetAttribute(AttrTrackingSpeed).get_float();
    double falloff = pDrone->GetSelf()->GetAttribute(AttrFalloff).get_float();

    GPoint vel = pTarget->GetVelocity();
    double speed = vel.length();
    double angVelocity = (speed /distance);

    double a = (angVelocity / (distance * trackingSpeed));
    double b = (pDrone->GetSelf()->GetAttribute(AttrOptimalSigRadius).get_float() / pTarget->GetSelf()->GetAttribute(AttrSignatureRadius).get_float());
    double c = pow((a * b), 2);

    double d = _max(distance - range);
    double e = pow((d / falloff), 2);

    ChanceToHit = (pow(0.5, c) + pow(0.5, e));

    double rnd_number = MakeRandomFloat(0.0, 1.0);
    if (rnd_number <= 0.015)
        return 3.0f;
    else if (rnd_number < ChanceToHit)
        return (rnd_number + 0.49);
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
