/**
 * @file Civilian.h
 *   Civilian Non-Combatant class
 * @Author:    Allan
 * @date:      02 April 2017
 * @updated:   June 2026 (Refactored for Ambient FSM by Gemini)
 *
 * simple AI to simulate a 'civilian run' from point A to point B to simulate normal system traffic.
 * system proc() will hit on (config option) timer
 * NCs jumping will be deleted.
 * there is very minimal processing overhead.
 * system creation will create NCs, configure their routing and begin processing
 * system unloading will delete any NCs in that system
 *
 *  Finite State Machine (FSM) NOTE: this is a VISUAL ONLY system...no destiny proc required
 *
 *  as the system matures...
 *  'shady' npcs may travel to unknown areas (to do their deeds in secret locations)  this will require calls to anomalyMgr/dungeonMgr
 *    (the astute capsuleer will notice the ship NOT traveling to a planet, gate, or station,
 *			and will then know the general area/direction to scan)
 *  'jumping' civs xfered to target system (if active) to continue travels
 *		possible to span multiple routes, if players are in system
 */


#ifndef __CIVILIAN_H_INCL__
#define __CIVILIAN_H_INCL__

#include "../eve-server.h"
#include "inventory/ItemType.h"

namespace Civ {
    enum State {
        Idle            = 0,
        Arriving 	= 1,	// have left origin, will spawn in destination bubble and travel
        Departing       = 2,	// have arrived/spawned at origin and will travel to destinantion
        Undocking	= 3,
	Stopping	= 4,
        Completed 	= 5,
	Formation       = 6	// to send proper packet data and avoid Proc() tics
    };
}

class SystemBubble;
class SystemEntity;

class Civilian
{

public:
    Civilian(uint32 itemID, uint16 typeID);
    ~Civilian();

    void 		Init(SystemEntity* pOrig, SystemEntity* pDest);
    void                Process();

    // Getters
    uint8	 	GetState()  const 		{ return m_state; }
    uint32 		GetID()	const			{ return m_itemID; }
    uint32 		GetTypeID()   const 		{ return m_type->id(); }
    Civilian*		GetLeader()			{ return m_pLeader; }
    const char*         GetName()                       { return m_type->name().c_str(); }

    // Setters
    void		SetPos(Vector3d pos)		{ m_pos = pos; }
    void 		AddGuard(Civilian* pCiv)	{ m_guards.push_back(pCiv); }
    void		SetLeader(Civilian* pLeader)	{ m_pLeader = pLeader; m_state = Civ::State::Formation; }
    void 		SetFormID(uint8 formID)		{ m_formID = formID; }

    // Misc
    void 		Stop();
    void 		SetVectors();
    uint16	        GetAlignTime();

protected:
    void 		Undock();
    void 		Add(SystemBubble* pBubble);
    void 		Warp(SystemEntity* ptargSE);
    void 		Remove(SystemBubble* pBubble);
    void 		SendGateActivity(SystemEntity* pGateSE) const;

    /* (fake) SystemEntity interface */
    void                EncodeDestiny(Buffer& into);
    PyTuple*            MakeDamageState();
    PyDict*             MakeSlimItem();
    void                SendShipVars(SystemBubble* pBubble);

private:
    uint8               m_state;
    uint8               m_formID;
    uint16              m_timeLeft;
    uint32              m_itemID;
    uint32              m_corpID;

    Vector3d              m_pos;
    Vector3d              m_heading;
    Vector3d             m_velocity;

    Civilian*           m_pLeader;
    const ItemType*	m_type;
    SystemEntity*       m_origSE;
    SystemEntity*       m_destSE;

    std::vector<Civilian*> m_guards;
};

#endif

/*
Profile 0 (The Escorted V):
Leader (Group 297 Hauler): (0, 0, 0)
Escort 1 (Group 298 Sentry): (-500, -200, -300) (Left Wing, slightly back)
Escort 2 (Group 298 Sentry): (500, -200, -300) (Right Wing, slightly back)

Profile 1 (The Heavy Escort / Diamond):
Leader: (0, 0, 0)
Vanguard Escort: (0, 0, 800) (Out front clearing the path)
Wing Escorts: (-600, -100, -200) and (600, -100, -200)
*/

/*
10043   Peddler     297     Convoy
10044   Column      297     Convoy
10045   Vanguard    297     Convoy
10114   Tradesman   297     Convoy
10115   Merchant    297     Convoy
10116   Trafficker  297     Convoy
10117   Caravan     297     Convoy
10118   Flotilla    297     Convoy
10823   Retailer    297     Convoy
10824   Chafferer   297     Convoy
10825   Trailer     297     Convoy
10826   Hauler  297     Convoy
10827   Trader  297     Convoy
10828   Courier     297     Convoy
10829   Purveyor    297     Convoy
10830   Carrier     297     Convoy
10831   Hawker  297     Convoy
10832   Huckster    297     Convoy
10833   Patronager  297     Convoy
10834   Chandler    297     Convoy
20716   Vendor  297     Convoy
20717   Bursar  297     Convoy
20718   Auctioneer  297     Convoy
20719   Marketeer   297     Convoy
9869    Loiterer I  298     Convoy Drone
10999   Convoy Escort   298     Convoy Drone
11000   Convoy Protector    298     Convoy Drone
11001   Convoy Guard    298     Convoy Drone
11002   Convoy Sentry   298     Convoy Drone


*/
