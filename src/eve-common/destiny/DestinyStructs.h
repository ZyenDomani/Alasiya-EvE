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
    Updates:    Allan
*/

#ifndef __DESTINY_STRUCTS_H__
#define __DESTINY_STRUCTS_H__

namespace Destiny {

#pragma pack(1)

struct AddBall_header {
    uint8 packet_type;  /* 0 = full state, 1 = balls */
    uint32 eventStamp;  /* statestamp */
};

typedef enum {
    DSTBALL_GOTO        = 0,    // Also used for AlignTo
    DSTBALL_FOLLOW      = 1,
    DSTBALL_STOP        = 2,
    DSTBALL_WARP        = 3,
    DSTBALL_ORBIT       = 4,
    DSTBALL_MISSILE     = 5,
    DSTBALL_MUSHROOM    = 6,    //Expanding gravity wall
    DSTBALL_BOID        = 7,    //Swarm like behavior
    DSTBALL_TROLL       = 8,    //used for ejected cans. Free ball that will become fixed after a while.
    DSTBALL_MINIBALL    = 9,
    DSTBALL_FIELD       = 10,    //Force field ball
    DSTBALL_RIGID       = 11,   //A ball that will never move, stations, etc..
    DSTBALL_FORMATION   = 12
} BallMode;

static const uint8 MAX_DSTBALL = DSTBALL_FORMATION;

enum ball_flags
{
    IsFree = 0x01,          // set if ball is free to move, has extra BallData
    IsGlobal = 0x02,        // set if ball should be visible from all
    IsMassive = 0x04,       // set if ball is solid
    IsInteractive = 0x08,   // set if ball is interactive - i.e. piloted ships
    IsMoribund = 0x10,      // set if ball is dying.. this is a rough guess..
    HasMiniBalls = 0x40,    // if set, the client expects mini balls
};

/**
 * a orbital asteroid entity has flags IsFree | IsMassive
 */


struct BallHeader {
    uint64 entityID;
    uint8 mode;
    float radius;
    double x;
    double y;
    double z;
    uint8 flags;
};

//included if not a RIGID body
struct MassSector {
    double mass;
    uint8  cloak;        // indicates cloak
    uint64 allianceID;
    uint32 corporationID;
    float Harmonic;
};

//only included if the thing can move...
struct DataSector {
    float maxVelocity;
    double velocity_x;
    double velocity_y;
    double velocity_z;
    float intertia;
    float speedfraction;
};

struct MiniBall {
    double x;
    double y;
    double z;
    float radius;
};

struct MiniBallList {
    uint16 count;
    MiniBall balls[0];
};

struct NameStruct {
    uint8  name_len;        //in 16 bit increments
    uint16 name[0];         //utf16
};

struct DSTBALL_GOTO_Struct {
    uint8  formationID;
    double x;        //object+0xD0 (as a set of 3)
    double y;
    double z;
};

struct DSTBALL_FOLLOW_Struct {
    uint8  formationID;
    uint64 followID;
    float followRange;
};

struct DSTBALL_STOP_Struct {
    uint8  formationID;
};

struct DSTBALL_WARP_Struct {
    uint8  formationID;
    double x;             //object+0xD0 (as a set of 3)
    double y;
    double z;
    uint32 effectStamp;   //statestamp of when warp started
    uint64 followRange;   //unknown   -4616189618054758400 when warp is initiated.  calculation unknown for other values (used during warp)
    uint64 followID;      //unknown   4669471951536783360 when warp is initiated or ship enters new bubble (AddBalls), 0 otherwise
    uint64 ownerID;
};

struct DSTBALL_ORBIT_Struct {
    uint8  formationID;
    uint32 followID;    //orbitID
    double followRange;
};

struct DSTBALL_MISSILE_Struct {
    uint8  formationID;
    uint64 followID;    //targetID
    float followRange;
    uint64 ownerID;
    uint32 effectStamp;
    double x;
    double y;
    double z;
};

struct DSTBALL_MUSHROOM_Struct {
    uint8  formationID;
    float followRange;
    double unknown125;
    uint32 effectStamp;
    uint64 ownerID;
};

struct DSTBALL_TROLL_Struct {
    uint8  formationID;
    uint32 effectStamp;   //statestamp of when effect started
};

struct DSTBALL_FIELD_Struct {
    uint8  formationID;
};

struct DSTBALL_RIGID_Struct {
    uint8  formationID;
/* there is an optional 2 byte thing here, if it is non-zero. not sure what it
   is yet. */
};

struct DSTBALL_FORMATION_Struct {
    uint8  formationID;
    uint64 followID;
    float followRange;
    uint32 effectStamp;  //statestamp of when warp started
};

union SpecificSectors {
    struct DSTBALL_GOTO_Struct GOTO;
    struct DSTBALL_FOLLOW_Struct FOLLOW;
    struct DSTBALL_STOP_Struct STOP;
    struct DSTBALL_WARP_Struct WARP;
    struct DSTBALL_ORBIT_Struct ORBIT;
    struct DSTBALL_MISSILE_Struct MISSILE;
    struct DSTBALL_MUSHROOM_Struct MUSHROOM;
    struct DSTBALL_TROLL_Struct TROLL;
    struct DSTBALL_FIELD_Struct FIELD;
    struct DSTBALL_RIGID_Struct RIGID;
    struct DSTBALL_FORMATION_Struct FORMATION;
};

struct TotalStruct {
    struct BallHeader head;
    struct MassSector mass;
    struct DataSector ship;
    union SpecificSectors specific;
};


#pragma pack()


} //end Destiny

#endif
