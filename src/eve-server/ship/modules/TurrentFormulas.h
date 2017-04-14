
 /**
  * @name TurrentFormulas.h
  *   forumlas for turrent tracking, to hit, and other specific things
  * @Author:         Allan
  * @date:   10 June 2015
  */

#ifndef _EVE_SHIP_MOD_FORMULAS_H_
#define _EVE_SHIP_MOD_FORMULAS_H_

#include "ship/Ship.h"

class Drone;
class NPC;
class TurrentModule;
class TurrentFormulas {
public:
    //  returns damage modifier from hit, based on calculations made about ship, item, and target.
    //    return 0 is missed
    float GetToHit(ShipItemRef shipRef, TurrentModule* pMod, SystemEntity* pTarget);
    float GetNPCToHit(NPC* pNPC, SystemEntity* pTarget);
    float GetDroneToHit(Drone* pDrone, SystemEntity* pTarget);

};


#endif  //_EVE_SHIP_MOD_FORMULAS_H_