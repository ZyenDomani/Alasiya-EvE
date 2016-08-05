/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
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
    Author:        Allan
*/

#ifndef EVEMU_SHIP_FLEETSVC_H_
#define EVEMU_SHIP_FLEETSVC_H_

#include <unordered_map>
#include "eve-common.h"

#include "Client.h"
#include "packets/Fleet.h"

class FleetService
{
public:
    uint32 CreateFleet(Client* pClient);
    PyObject* CreateWing(Client* pClient);
    PyObject* CreateSquad(Client* pClient);
    PyRep* GetAvailableFleets();
    PyObject* Init(Client* pClient);

protected:
    static uint32 m_fleetID;

    struct avalibleFleets {
        float local_minSecurity;
        std::string description;
        float public_minStanding;
        std::string fleetName;
        int64 advertTime;
        int64 dateCreated;
        bool joinNeedsApproval;
        PyString local_allowedEntities;
        float local_minStanding;
        uint16 numMembers;
        bool hideInfo;
        float public_minSecurity;
        uint32 fleetID;
        int public_allowedEntities;
        uint8 inviteScope;
        uint32 solarSystemID;
        uint32 leaderID;
    };

private:
    //list containing all active fleets by ID
    std::list<uint32> m_fleets;

    //vector containing fleetID, Character* for all members of particular fleet.
    // CharacterRef will contain FleetData with fleet data for that member.
    std::unordered_multimap<uint32, Character*> m_fleetMembers;
    std::map<uint32, avalibleFleets*> m_avalibleFleetsMap;

};

#endif  // EVEMU_SHIP_FLEETSVC_H_

//Pilots who are disconnected and reconnect within 2 minutes, will automatically rejoin their fleet.

/*Note that the maximum size of a fleet is 256 pilots:
 * 1 Fleet Commander,
 * 5 Wing Commanders,
 * 25 10-man squadrons (5 Squads of 9 Pilots + Squad Commander per Wing Commander).
 *
 * To put it another way the maximum size of:
 *   a squadron is 10 members including the commander,
 *   a wing is five squadrons,
 *   a fleet is 5 wings.
 */

/*
fleetGroupingRange = 300
fleetJobNone = 0
fleetJobScout = 1
fleetJobCreator = 2

fleetLeaderRole = 1

fleetRoleLeader = 1
fleetRoleWingCmdr = 2
fleetRoleSquadCmdr = 3
fleetRoleMember = 4

fleetBoosterNone = 0
fleetBoosterFleet = 1
fleetBoosterWing = 2
fleetBoosterSquad = 3

rejectFleetInviteTimeout = 1
rejectFleetInviteAlreadyInFleet = 2
*/
