/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2008 The EVEmu Team
    For the latest information visit http://evemu.mmoforge.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Aknor Jaden
    Updates:    Allan
*/

#ifndef __DRONE__H__INCL__
#define __DRONE__H__INCL__

#include "Client.h"
#include "npc/DroneAI.h"
#include "system/SystemEntity.h"

/**
 * ObjectSystemEntity which represents drone object in space
 */

class PyServiceMgr;
class Item;
class DestinyManager;
class SystemManager;
class ServiceDB;
class NPCAIMgr;
class Damage;

class DroneSE
: public ObjectSystemEntity
{
public:
  friend DroneAIMgr;
    DroneSE(InventoryItemRef drone, PyServiceMgr& services, SystemManager* pSystem, const FactionData& data);
    virtual ~DroneSE();

    /* class type pointer querys. */
    virtual DroneSE*    GetDroneSE()                    { return this; }
    /* class type tests. */
    virtual bool        IsDroneSE()                     { return true; }
    // this will initialize abandoned or offline drones during system load and launched drones into existing system
    virtual void        Init();
    /* SystemEntity interface */
    virtual void        Process();
    virtual void        EncodeDestiny(Buffer &into);
    virtual void        MakeDamageState(DoDestinyDamageState &into);
    virtual PyDict*     MakeSlimItem();

    // override because drones dont have actual destiny...fake it via AI
    //DroneAIMgr*         DestinyMgr()                    { return m_AI; }

    /* virtual functions default to base class and overridden as needed */
    virtual void        Killed(Damage &fatal_blow);
    virtual void        Abandon();     // reset all owner info and bubblecast new data
    virtual const GVector& GetVelocity()                { return m_AI->GetVelocity(); }


    virtual void        TargetAdded(SystemEntity* pSE);
    virtual void        TargetLost(SystemEntity* pSE);
    virtual void        TargetedAdd(SystemEntity* pSE);
    virtual void        TargetedLost(SystemEntity* pSE);

    /* virtual functions to be overridden in derived classes */
    virtual void        MissileLaunched(Missile* pMissile);  // tell AI a missile has been launched at us.  allows defender missile code
    virtual void        ReportDamage(uint8 type=0);

    /* specific functions handled here. */
    Client*             GetOwner()                      { return m_pClient; }
    DroneAIMgr*         GetAI()                         { return m_AI; }

    // assign drone to ship and add to system
    void                Launched(ShipSE* pShipSE);       // this is only for drone owner.
    void                Online(ShipSE* pShipSE=nullptr); // this is only for drone owner.
    // sent on every state or controller change
    void                StateChange();                  // droneAI.state must be set before calling this for client to get accurate state information

    void                SaveDrone();
    void                RemoveDrone();                  // this will delete the item and SE
    void                SetResists();
    void                AssignTo(Client* pClient);

    uint32              GetBounty() const               { return (m_pClient == nullptr ? 0 : m_pClient->GetChar()->bounty()); }

    float               GetThermal()                    { return m_therDamage; }
    float               GetEM()                         { return m_emDamage; }
    float               GetKinetic()                    { return m_kinDamage; }
    float               GetExplosive()                  { return m_expDamage; }
    float               GetSecurityRating() const       { return (m_pClient == nullptr ? 0.0f : m_pClient->GetChar()->GetSecurityRating()); }

    /* for destiny setstate */
    uint8               GetState()                      { return m_AI->GetState(); }
    uint32              GetControllerID()               { return m_controllerID; }
    uint32              GetControllerOwnerID()          { return m_controllerOwnerID; }
    uint32              GetTargetID();

    /* misc methods */
    bool                IsEnabled()                     { return m_online; }
    bool                IsDamaged()                     { return m_damaged; }
    uint32              GetControlDistance()            { return m_controlDistance; }

    // sets ship offline and removes assigned ship
    void                OfflineDrone();                 // also calls StateChange
    //sets ship offline but doesnt reset anything else
    void                DisableDrone();                 // also calls StateChange

    void                AssignShip(ShipSE* pShipSE);
    void                TargetDestroyed( SystemEntity* pSE );              // no need to clarify...drones have only one target

    ShipSE*             GetHomeShip()                   { return m_pShipSE; }

    void                ShipWarping(ShipSE* pShipSE);

    /* commanded methods */
    void                Reconnect(ShipSE* pShipSE, PyDict* dict);     // this is for previously abandonded drones
    void                ReturnBay(PyDict* dict);                      // return to owner's ship and dock in drone bay
    void                ReturnHome(PyDict* dict);                     // return to owner's ship and remain in range
    void                Engage(SystemEntity* pTarget, PyDict* dict);  // this is for fighting
    void                Assist(SystemEntity* pTarget, PyDict* dict);  // this is for assisting another pilot
    void                Guard(SystemEntity* pTarget, PyDict* dict);   // this is for guarding another pilot
    void                Delegate(SystemEntity* pTarget, PyDict* dict);// this is to give control to another pilot
    void                Mine(SystemEntity* pTarget, PyDict* dict, bool repeat=false); // should i explain?

    /* helper methods */
    bool                InControlDistance();            // returns true if drone within control distance
    void                CheckCommand(PyDict* dict);     // runs multiple checks and will throw on error
    void                ChargeShield();                 // shield recharging while in space
    float               CalculateRechargeRate(float Capacity, float RechargeTimeMS, float Current);

    void                SendBallData();

protected:
    SystemManager*      m_system;               //we do not own this
    DroneAIMgr*         m_AI;                   //we do own this
    Client*             m_pClient;              //our owner
    ShipSE*             m_pShipSE;              //owning ship (home ship)


private:
    Timer               m_processTimer;

    bool                m_online;               // is drone within ship's control range?
    bool                m_damaged;              // damage beyond % as defined in AttrIncapacitationRatio

    uint32              m_controlDistance;
    uint32              m_controllerID;
    uint32              m_controllerOwnerID;

    float               m_emDamage;
    float               m_expDamage;
    float               m_kinDamage;
    float               m_therDamage;
    float               m_hullDamage;
    float               m_armorDamage;
    float               m_shieldCharge;
    float               m_shieldCapacity;
};

#endif /* !__DRONE__H__INCL__ */


/*
 *    def OnDroneStateChange(self, itemID, ownerID, controllerID, activityState, typeID, controllerOwnerID, targetID):
 *        if session.charid != ownerID and session.shipid != controllerID:
 *            if self.stateByDroneID.has_key(itemID):
 *                del self.stateByDroneID[itemID]
 *            if self.activityByDrone.has_key(itemID):
 *                del self.activityByDrone[itemID]
 *            sm.ScatterEvent('OnDroneControlLost', itemID)
 *            return
 *        state = self.stateByDroneID.get(itemID, None)
 *        if state is None:
 *            oldActivityState = None
 *            self.stateByDroneID.UpdateLI([[itemID,
 *              ownerID,
 *              controllerID,
 *              activityState,
 *              typeID,
 *              controllerOwnerID,
 *              targetID]], 'droneID')
 *        else:
 *            state.ownerID = ownerID
 *            state.controllerID = controllerID
 *            state.controllerOwnerID = controllerOwnerID
 *            oldActivityState = state.activityState
 *            state.activityState = activityState
 *            state.targetID = targetID
 *        sm.ScatterEvent('OnDroneStateChange2', itemID, oldActivityState, activityState)
 *
 *    def OnDroneActivityChange(self, droneID, activityID, activity):
 *        if not activity:
 *            if self.activityByDrone.has_key(droneID):
 *                del self.activityByDrone[droneID]
 *        else:
 *            self.activityByDrone[droneID] = (activity, activityID)
 */