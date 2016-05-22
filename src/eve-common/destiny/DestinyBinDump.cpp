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
    Author:        Zhur
*/

#include "eve-common.h"

#include "destiny/DestinyBinDump.h"
#include "destiny/DestinyStructs.h"

namespace Destiny {

    const char *const DSTBALL_modeNames[] = {
    "GOTO",
    "FOLLOW",
    "STOP",
    "WARP",
    "ORBIT",
    "MISSILE",
    "MUSHROOM",
    "BOID",
    "TROLL",
    "MINIBALL",
    "FIELD",
    "RIGID",
    "FORMATION"
};

void DumpUpdate(LogType into, const uint8 *data, uint32 len) {
    const AddBall_header *global_head = (const AddBall_header *) data;
    _log(into, "AddBall: packet_type=%d, sequence=%d (len %d)", global_head->packet_type, global_head->eventStamp, len);
    data += sizeof(AddBall_header);
    len -= sizeof(AddBall_header);

    while(len > 0) {
        uint32 used = DumpBall(into, data, len);
        if (used == 0)
            return;    //error
        data += used;
        len -= used;
    }
}

uint32 DumpBall(LogType into, const uint8 *data, uint32 len) {
    uint32 init_len = len;

    const BallHeader *ballhead = (const BallHeader *) data;
    data += sizeof(BallHeader);
    len -= sizeof(BallHeader);

    if (ballhead->mode > MAX_DSTBALL) {
        _log(into, "Error: Invalid ball mode %d for ball %d", ballhead->mode, ballhead->entityID);
        return 0;
    }

    _log(into, "AddBall: entity=%d, mode=%s (%d) flags=0x%X",
        ballhead->entityID, DSTBALL_modeNames[ballhead->mode], ballhead->mode, ballhead->flags);
    _log(into, "   x=%.3f, y=%.3f, z=%.3f, radius=%.2f",
        ballhead->x, ballhead->y, ballhead->z, ballhead->radius);

    if (ballhead->mode != DSTBALL_RIGID) {
        const MassSector *masschunk = (const MassSector *) data;
        data += sizeof(MassSector);
        len -= sizeof(MassSector);

        _log(into, "   mass=%.2f, cloak=%d, harmonic=%f, corp=%u, alliance=%" PRIx64,
            masschunk->mass, masschunk->cloak, masschunk->Harmonic, masschunk->corporationID, masschunk->allianceID);
    }

    //this seems a little strange, but this is how it works...
    if (ballhead->flags & IsFree) {
        const ShipSector *shipchunk = (const ShipSector *) data;
        data += sizeof(ShipSector);
        len -= sizeof(ShipSector);

        _log(into, "   maxSpeed=%.2f, V=(%.3f, %.3f, %.3f) PS=%.4f, SF=%.3f",
            shipchunk->maxVelocity,
            shipchunk->velocity_x, shipchunk->velocity_y, shipchunk->velocity_z,
            shipchunk->agility,
            shipchunk->speedfraction);
    }

    _log(into, "   %s:", DSTBALL_modeNames[ballhead->mode]);
    switch(ballhead->mode) {
        case DSTBALL_BOID:
        case DSTBALL_MINIBALL: {
            _log(into, "       NOT ALLOWED IN STREAM!");
            return 0;
        } break;
        case DSTBALL_GOTO: {
            const DSTBALL_GOTO_Struct *b = (const DSTBALL_GOTO_Struct *) data;
            data += sizeof(DSTBALL_GOTO_Struct);
            len -= sizeof(DSTBALL_GOTO_Struct);
            _log(into, "       formID=%d, Goto=(%.3f, %.3f, %.3f)", b->formationID, b->x, b->y, b->z);
        } break;
        case DSTBALL_FOLLOW: {
            const DSTBALL_FOLLOW_Struct *b = (const DSTBALL_FOLLOW_Struct *) data;
            data += sizeof(DSTBALL_FOLLOW_Struct);
            len -= sizeof(DSTBALL_FOLLOW_Struct);
            _log(into, "       formID=%d, followID=%u, distance=%.1f", b->formationID, b->followID, b->followRange);
        } break;
        case DSTBALL_STOP: {
            const DSTBALL_STOP_Struct *b = (const DSTBALL_STOP_Struct *) data;
            data += sizeof(DSTBALL_STOP_Struct);
            len -= sizeof(DSTBALL_STOP_Struct);
            _log(into, "       formID=%d ", b->formationID);
        } break;
        case DSTBALL_WARP: {
            const DSTBALL_WARP_Struct *b = (const DSTBALL_WARP_Struct *) data;
            data += sizeof(DSTBALL_WARP_Struct);
            len -= sizeof(DSTBALL_WARP_Struct);
            _log(into, "       formID=%d, To=(%.3f, %.3f, %.3f) effectStamp=%u", b->formationID, b->x, b->y, b->z, b->effectStamp);
            _log(into, "       followRange=%.3f, followID=%u, ownerID=%u", b->followRange, b->followID, b->ownerID);
        } break;
        case DSTBALL_ORBIT: {
            const DSTBALL_ORBIT_Struct *b = (const DSTBALL_ORBIT_Struct *) data;
            data += sizeof(DSTBALL_ORBIT_Struct);
            len -= sizeof(DSTBALL_ORBIT_Struct);
            _log(into, "       formID=%d, orbitID=%u, distance=%.1f", b->formationID, b->followID, b->followRange);
        } break;
        case DSTBALL_MISSILE: {
            const DSTBALL_MISSILE_Struct *b = (const DSTBALL_MISSILE_Struct *) data;
            data += sizeof(DSTBALL_MISSILE_Struct);
            len -= sizeof(DSTBALL_MISSILE_Struct);
            _log(into, "       formID=%d, target=%u, followRange?=%.1f, ownerID=%u, effectStamp=%u", b->formationID, b->followID, b->followRange, b->ownerID, b->effectStamp);
            _log(into, "       u=(%.3f, %.3f, %.3f)", b->x, b->y, b->z);
        } break;
        case DSTBALL_MUSHROOM: {
            const DSTBALL_MUSHROOM_Struct *b = (const DSTBALL_MUSHROOM_Struct *) data;
            data += sizeof(DSTBALL_MUSHROOM_Struct);
            len -= sizeof(DSTBALL_MUSHROOM_Struct);
            _log(into, "       formID=%d, distance=%.3f, u125=%.3f, effectStamp=%u, ownerID=%u", b->formationID, b->followRange, b->unknown125, b->effectStamp, b->ownerID);
        } break;
        case DSTBALL_TROLL: {
            const DSTBALL_TROLL_Struct *b = (const DSTBALL_TROLL_Struct *) data;
            data += sizeof(DSTBALL_TROLL_Struct);
            len -= sizeof(DSTBALL_TROLL_Struct);
            _log(into, "       formID=%d, effectStamp=%u", b->formationID, b->effectStamp);
        } break;
        case DSTBALL_FIELD: {
            const DSTBALL_FIELD_Struct *b = (const DSTBALL_FIELD_Struct *) data;
            data += sizeof(DSTBALL_FIELD_Struct);
            len -= sizeof(DSTBALL_FIELD_Struct);
            _log(into, "       formID=%d ", b->formationID);
        } break;
        case DSTBALL_RIGID: {
            const DSTBALL_RIGID_Struct *b = (const DSTBALL_RIGID_Struct *) data;
            data += sizeof(DSTBALL_RIGID_Struct);
            len -= sizeof(DSTBALL_RIGID_Struct);
            _log(into, "       formID=%d ", b->formationID);
        } break;
        case DSTBALL_FORMATION: {
            const DSTBALL_FORMATION_Struct *b = (const DSTBALL_FORMATION_Struct *) data;
            data += sizeof(DSTBALL_FORMATION_Struct);
            len -= sizeof(DSTBALL_FORMATION_Struct);
            _log(into, "       formID=%d, followID=%u, followRange=%.3f, effectStamp=%u", b->formationID, b->followID, b->followRange, b->effectStamp);
        } break;
        default:
            _log(into, "Error: Unknown ball mode %d!", ballhead->mode);
            _hex(into, data-sizeof(BallHeader), (len>128)?128:(len+sizeof(BallHeader)));
            return 0;
    }

    if (ballhead->flags & HasMiniBalls) {
        const MiniBallList* mbl = (const MiniBallList*)data;
        data += sizeof( MiniBallList );
        len -= sizeof( MiniBallList );

        if (mbl->count) {
            _log( into, "    MiniBall Count: %d", mbl->count );
            for( uint16 r = 0; r < mbl->count; ++r ) {
                const MiniBall* mini = (const MiniBall*)data;
                data += sizeof( MiniBall );
                len -= sizeof( MiniBall );
                _log( into, "        [%d] pos (%.3f, %.3f, %.3f) radius %.2f", r, mini->x, mini->y, mini->z, mini->radius );
            }
        }
    }
    /*  not used
    const NameStruct *name = (const NameStruct *) data;
    data += sizeof(NameStruct);
    len -= sizeof(NameStruct);
    if (name->name_len > 0) {
        _log(into, "   Name: len=%u", name->name_len);
        _log(into, "    ~ %s", name->name);

        data += name->name_len*sizeof(uint16);
        len  -= name->name_len*sizeof(uint16);
    }
    */
    if (len > init_len) {
        _log(into, "ERROR: Consumed more bytes than given: had %u, used %u", init_len, len);
        return init_len;
    }
    return init_len - len;
}

}
