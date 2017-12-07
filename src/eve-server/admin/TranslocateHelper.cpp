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
    Author:        Groove
*/
/*

   std::string usageString =
    "Correct Usage:<br><br>\
    - General Notes:<br>\
    + tr is same as translocate command<br>\
    + object being teleported MUST be in space<br>\
    + destination object MUST be in space<br>\
    + 'entityID #1' MUST be a currently logged-in character<br>\
    + 'entityID #2' can be a character, ship, NPC, station, belt, stargate, or solar system<br>\
    + 'me' string is allowed for [character name] to indicate YOU being teleported<br><br>\
    .tr [entityID #1] - teleport YOU to 'entityID'<br>\
    .tr [character name] - teleport YOU to 'character name'<br>\
    .tr [locationID] - teleport YOU into 'locationID'<br>\
    .tr [solar system name] - teleport YOU into 'solar system name' system<br>\
    .tr [entityID #1|character name] [entityID #2|character name|locationID|solar system name] - teleport 'entityID #1' or 'character name' to 'entityID #2', 'character name' or solar system<br>\
    .tr x y z - teleport YOU to a specific (x,y,z) coordinate in the current solar system<br>\
    ";

    // Error if there are NO arguments past first string "tr" or "translocate":
    if (args.argCount() < 2) {
        throw PyException(MakeCustomError(usageString.c_str()));
    }

    // Argument Discovery
    Client* p_targetClient(nullptr);
    SystemEntity* destinationEntity(nullptr);
    int locationID = 0, trMode = 0;
    GPoint destinationPoint(NULL_ORIGIN);
    uint32 argsCount = args.argCount();
    std::string name1 = args.arg(1);
    std::string name2 = "";
    bool isFirstArgName = false, isSecondArgName = false;
    enum TR_MODE
    {
        TR_MODE_ME_TO_ENTITY = 1,
        TR_MODE_ME_TO_CHARACTER = 2,
        TR_MODE_ME_TO_SOLARSYSTEMID = 3,
        TR_MODE_ME_TO_SOLARSYSTEM = 4,
        TR_MODE_ENTITY_TO_ENTITY = 5,
        TR_MODE_ENTITY_TO_CHARACTER = 6,
        TR_MODE_ENTITY_TO_SOLARSYSTEMID = 7,
        TR_MODE_ENTITY_TO_SOLARSYSTEM = 8,
        TR_MODE_CHARACTER_TO_ENTITY = 9,
        TR_MODE_CHARACTER_TO_CHARACTER = 10,
        TR_MODE_CHARACTER_TO_SOLARSYSTEMID = 11,
        TR_MODE_CHARACTER_TO_SOLARSYSTEM = 12
    };

    isFirstArgName = args.isNumber(1) ? false : true;

    // First, determine nature of First argument
    if (isFirstArgName) {
        // First argument is a string, find out if it's a character or solar system:
        if ((name1 == "me") && (argsCount < 3))
            throw PyException(MakeCustomError(std::string(usageString+"<br><br>FIRST ARGUMENT WAS 'me' BUT MISSING SECOND ARGUMENT!").c_str()));

        if (name1 == "me")
            p_targetClient = who;
        else {
            // First argument is a string of a character or solar system:
            //TODO
        }
    } else {
        // First argument is a number, find out if it's a character, ship, NPC, station, belt, stargate, or solar system:
        //TODO
        p_targetClient = who;
        locationID = atoi(args.arg(1).c_str());
        SystemGPoint m_gp;
        if (IsSolarSystem(locationID))
            destinationPoint = m_gp.GetRandPointOnMoon(locationID);//GPoint(12457894200.0f, 17254864800.0f, 14851254800.0f);
           
        //} else if (IsPlayerItem(locationID)) {
        //    destinationPoint = who->SystemMgr()->GetSE(locationID)->GetPosition();
        //    locationID = who->GetLocationID();
        //}

        trMode = 1;
    }

    if (trMode == 0)
        throw PyException(MakeCustomError(std::string(usageString+"<br><br>UNABLE TO DETERMINE FORMAT OF ARGUMENTS 1 and 2!").c_str()));


    if (argsCount == 3) {
        // We are transporting either THIS client 'who' or some other entity or character to somewhere:
        name2 = args.arg(2);
        isSecondArgName = args.isNumber(2) ? false : true;

        // Determine nature of Second argument
        if (isSecondArgName) {
            // Second argument is a string, find out if it's a character or solar system:
            //TODO
            throw PyException(MakeCustomError(std::string(usageString+"<br><br>NOT SUPPORTED YET!").c_str()));
        } else {
            // Second argument is a number, find out if it's a character, ship, NPC, station, belt, stargate, or solar system:
            //TODO
            throw PyException(MakeCustomError(std::string(usageString+"<br><br>NOT SUPPORTED YET!").c_str()));
        }
    }

    if (argsCount == 4) {
        // SPECIAL CASE:  We are transporting ourselves to a specific (x,y,z) coordinate in the current solar system:
        p_targetClient = who;
        locationID = who->GetLocationID();
        if (!IsSolarSystem(locationID))
            throw PyException(MakeCustomError(std::string(usageString+"<br><br>YOU MUST BE IN SPACE!").c_str()));

        if (args.isNumber(1) && args.isNumber(2) && args.isNumber(3))
            destinationPoint = GPoint(atoll(args.arg(1).c_str()), atoll(args.arg(2).c_str()), atoll(args.arg(3).c_str()));
    }

    //  in case ap is set, unset it, as it will do odd things when undocking or loging in
    p_targetClient->SetAutoPilot(false);

    // We're still going, so we know now we have a target to translocate AND a destination solar system AND destination coordinates, so let's do the translocate:
    //    p_targetClient - target character in a ship to translocate (Client *)
    //    locationID - destination solar system ID
    //    destinationPoint - destination coordinates (GPoint)

    //if (!p_targetClient->GetShipSE())
    //    p_targetClient->CreateShipSE();
    if (IsSolarSystem(locationID) and p_targetClient->GetShipSE() and p_targetClient->GetShipSE()->DestinyMgr())
        p_targetClient->GetShipSE()->DestinyMgr()->SendJumpOutEffect("effects.JumpOut", locationID);

    p_targetClient->MoveToLocation(locationID, destinationPoint);
    //p_targetClient->SetClientTimer(ClientState::csJump, ClientTimers::JumpingTimer);
    if (p_targetClient->GetShipSE() and p_targetClient->GetShipSE()->DestinyMgr())
        p_targetClient->GetShipSE()->DestinyMgr()->SendJumpInEffect("effects.JumpIn");

    return new PyString("Translocation successful.");

 */

#define ERROR(_str) \
    throw PyException(MakeCustomError(_str));

#include "admin/TranslocateHelper.h"
#include "system/SystemGPoint.h"
#include "EntityList.h"
#include "log/logsys.h"

static bool translocate_to_solarsystem(TRData *data, uint32_t who, uint32_t dest) {
    SystemManager *system = sEntityList.FindOrBootSystem(dest);
    if (!system) {
        return false;
    }
    
    Client *client = sEntityList.FindClientByCharID(who);
    if (!client) {
        return false;
    }

    if (client->GetShipSE() != nullptr and client->GetShipSE()->DestinyMgr() and !client->GetShipSE()->DestinyMgr()->IsCloaked()) {
        client->GetShipSE()->DestinyMgr()->SendJumpOutEffect("effects.JumpOut", dest);
    }

    client->EnterSystem(dest);

    if (client->GetShipSE() != nullptr and client->GetShipSE()->DestinyMgr() and !client->GetShipSE()->DestinyMgr()->IsCloaked()) {
        client->GetShipSE()->DestinyMgr()->SendJumpInEffect("effects.JumpIn");
    }

    return true;
}

static bool translocate_to_station(TRData *data, uint32_t who, uint32_t dest) {
		codelog(COMMAND__ERROR, "translocate_to_station called\n");
		Client *c = sEntityList.FindClientByCharID(who);

		Ship *ship = c->GetShipSE();
		if (ship != nullptr and ship->DestinyMgr() != nullptr and !ship->DestinyMgr()->IsCloaked()) {
				ship->DestinyMgr()->SendJumpInEffect("effects.JumpIn");
		}

		c->MoveToLocation(dest, GPoint(NULL_ORIGIN));

		// No jump out. We're docking, probably..
		return true;
}

static bool translocate_to_characterID(TRData *data, uint32_t who, uint32_t dest) {
		// Since we're /tr to player.  The system is assumed to be booted
		//if (!IsCharType(dest)) {
		//		ERROR("/tr called with a non-character ID but was resolved to a character action");
		//}
		Client *dest_client = sEntityList.FindClientByCharID(dest);
		Client *victim_client = sEntityList.FindClientByCharID(who);
		if (dest_client == nullptr or victim_client == nullptr) {
				ERROR("/tr to or from offline players.");
		}

		if (dest_client->IsDocked()) {
				uint32 station_id = dest_client->GetStationID();
				codelog(COMMAND__ERROR, "stationID: %d\n", station_id);
				if (IsStation(station_id)) {
						return translocate_to_station(data, who, station_id);
				} else {
						return false;
				}
		}

		if (dest_client->IsInSpace()) {
				if (dest_client->GetShipSE() != nullptr) {

						Ship *ship = victim_client->GetShipSE();
						if (ship != nullptr and ship->DestinyMgr() != nullptr and !ship->DestinyMgr()->IsCloaked()) {
								ship->DestinyMgr()->SendJumpInEffect("effects.JumpIn");
						}

						GPoint position = dest_client->GetShip()->position();
						victim_client->MoveToLocation(dest_client->GetSystemID(), position);

						if (ship != nullptr and ship->DestinyMgr() != nullptr and 
										!ship->DestinyMgr()->IsCloaked()) {
								ship->DestinyMgr()->SendJumpOutEffect("effects.JumpOut", dest);
						}
						return true;
				} else {
						return false;
				}
		}

		return false;
}

static bool translocate_to_celestial(TRData *data, uint32_t who, uint32_t dest) {
		ERROR("/tr to celestials not implemented")
		return false;
}


bool translocate_to(TRData *data, uint32_t who, uint32_t dest, LocationTag tag) {
    switch (tag) {
    case LocationTag_Character: {
				codelog(COMMAND__ERROR, "translocate_to_characterID");
        return translocate_to_characterID(data, who, dest);
    } break;
    case LocationTag_SolarSystem: {
				codelog(COMMAND__ERROR, "translocate_to_solarysystem");
        return translocate_to_solarsystem(data, who, dest);
    } break;
    case LocationTag_Celestial: {
				codelog(COMMAND__ERROR, "translocate_to_celestial");
        return translocate_to_celestial(data, who, dest);
    } break;
    case LocationTag_Station: {
				codelog(COMMAND__ERROR, "translocate_to_station");
        return translocate_to_station(data, who, dest);
    } break;
		default:
    case LocationTag_Invalid: {
        codelog(COMMAND__ERROR, "Somehow translocate_to got called with LocationTag_Invalid.  Fix yo shit");
        return false;
    } break;
    }
		return false;
}

LocationTag translocate_resolve_id(TRData *data, uint32_t thing_id) {
    if (IsSolarSystem(thing_id)) {
        return LocationTag_SolarSystem;
    }
    if (IsCelestial(thing_id)) {
        return LocationTag_Celestial;
    }
    if (IsCharType(thing_id)) {
        return LocationTag_Character;
    }
    if (IsStation(thing_id)) {
        return LocationTag_Station;
    }
    ERROR("Error while resolving ID.  Was not one of the following: Solar System, Celestials, Character, Station");
}

LocationTag translocate_resolve_location_name(TRData *data, const char *location_name, uint32_t *thing_id) {
    if (thing_id == nullptr) {
        return LocationTag_Invalid;
    }
    if (strcmp(location_name, "me") == 0) {
        *thing_id = data->who->GetCharacterID();
    }
    // Currently translocate_resolve_location_name only works with
    // solar systems and players.  I don't see a purpose otherwise
		//
		Client *client = sEntityList.FindClientByName(location_name);
    if (client != nullptr) {
        *thing_id = client->GetCharacterID();
				return LocationTag_Character;
    }
    uint32_t id = data->db->GetSolarSystem(location_name);
    if (id != 0) {
        *thing_id = id;
        return LocationTag_SolarSystem;
    }

		codelog(COMMAND__ERROR, "FindClientByName with name: '%s' returned no client\n", location_name);
    return LocationTag_Invalid;
}



