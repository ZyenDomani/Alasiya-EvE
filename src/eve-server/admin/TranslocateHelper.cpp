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


    client->SetAutoPilot(false);
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
        codelog(COMMAND__ERROR, "translocate_to_characterID: who: %d dest: %d", who, dest);
        return translocate_to_characterID(data, who, dest);
    } break;
    case LocationTag_SolarSystem: {
        codelog(COMMAND__ERROR, "translocate_to_solarysystem: who: %d dest: %d", who, dest);
        return translocate_to_solarsystem(data, who, dest);
    } break;
    case LocationTag_Celestial: {
        codelog(COMMAND__ERROR, "translocate_to_celestial: who: %d dest: %d", who, dest);
        return translocate_to_celestial(data, who, dest);
    } break;
    case LocationTag_Station: {
        codelog(COMMAND__ERROR, "translocate_to_station: who: %d dest: %d", who, dest);
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
    codelog(COMMAND__ERROR, "Error resolving ID: %d", thing_id);
    ERROR("Error while resolving ID.  Was not one of the following: Solar System, Celestials, Character, Station");
}

LocationTag translocate_resolve_location_name(TRData *data, const char *location_name, uint32_t *thing_id) {
    if (thing_id == nullptr) {
        return LocationTag_Invalid;
    }
    if (strcmp(location_name, "me") == 0) {
        *thing_id = data->who->GetCharacterID();
        return LocationTag_Character;
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



