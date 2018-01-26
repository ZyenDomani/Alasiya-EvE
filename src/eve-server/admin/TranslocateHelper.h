#ifndef __TRANSLOCATE_HELPER__H_
#define __TRANSLOCATE_HELPER__H_

#include "Client.h"
#include "ConsoleCommands.h"
#include "map/MapConnections.h"
#include <stdint.h>

// Some helper functions to make /tr suck less

struct TRData {
    Client *who;
    CommandDB *db;
    PyServiceMgr *services;
};

enum LocationTag {
    LocationTag_Character,
    LocationTag_SolarSystem,
    LocationTag_Celestial,
    LocationTag_Station,
    LocationTag_Invalid = -1,
};

bool translocate_to(TRData *data, uint32_t who, uint32_t dest, LocationTag tag);
LocationTag translocate_resolve_id(TRData *data, uint32_t thing_id);
LocationTag translocate_resolve_location_name(TRData *data, const char *location_name, uint32_t *thing_id);

#endif
