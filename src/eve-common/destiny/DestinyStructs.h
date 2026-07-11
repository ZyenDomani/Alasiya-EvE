/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
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
    Rewrite:    Allan
*/

#ifndef __DESTINY_STRUCTS_H__
#define __DESTINY_STRUCTS_H__

namespace Destiny {

    static const uint8 MAX_DSTBALL = 12;

    namespace Ball {
        namespace Mode {
            enum {
                GOTO        = 0,    // AlignTo / Primary trajectory point translation
                FOLLOW      = 1,    // Linear trailing chase logic
                STOP        = 2,    // Rapid dampening / absolute brake step
                WARP        = 3,    // High-speed collisionless absolute vector pathing
                ORBIT       = 4,    // Perpendicular tangent vector shifting loop (Rule 2)
                MISSILE     = 5,    // Speed-scaled dynamic mass turning tracking (Rule 4)
                MUSHROOM    = 6,    // Radial gravity/area expansion envelope (Smartbombs/AOE)
				// This is used when objects are spawned in proximity and need to resolve their initial positions before settling.
                BOID        = 7,    // Spatial swarm flock aggregation physics (Craig Reynolds' Bird-oid)
                TROLL       = 8,    // Inertially decaying unanchored physical drop (Wrecks/Cans dropped from moving objects)
                MINIBALL    = 9,    // Component multi-hitbox structural sub-mesh
                FIELD       = 10,   // Non-blocking spatial event boundaries (POS shields)
                RIGID       = 11,   // Frozen, zero-calculation static structures (Stations/Gates)
                FORMATION   = 12    // Matrix-offset slave tracking relative to group leader
            };
        }

        namespace Flag {
            enum  {
                IsFree          = 0x01,   // Contains DataSector (dynamic physical calculation)
                IsGlobal        = 0x02,   // Universally distributed lifecycle node
                IsMassive       = 0x04,   // Subject to rigid body elastic boundary bumping
                IsInteractive   = 0x08,   // Targetable UI spatial object selector
                IsMoribund      = 0x10,   // Flagged for dynamic destruction sweep next tick
                HasMiniBalls    = 0x40,   // Trailing array stream contains structural sub-shapes
            };
        }

        namespace HookEvent {
            enum {
                DST_CREATE         = 1,   // Creation setup execution
                DST_DESTROY        = 2,   // Destructor memory sweep
                DST_PROXIMITY      = 3,   // Proximity threshold breach check
                DST_PRETICK        = 4,   // Impulse calculation staging clear
                DST_POSTTICK       = 5,   // Unified blend feedback evaluation
                DST_COLLISION      = 6,   // Elastic bumping response execution
                DST_RANGE          = 7,   // Outer radius exit/entry calculation
                DST_MODECHANGE     = 8,   // Core flight mode transition trigger
                DST_PARTITION      = 9,   // Grid cell border cross step
                DST_WARPACTIVATION = 10,  // Warp entry linear transformation staging
                DST_WARPEXIT       = 11   // Deceleration warp drop normalization
            };
        }
    }

/** @note  this file MUST use packed data and variable types must remain as-is
 * client will not recognize it with byte-ordered padding, and expects values to be at specific locations
 * (error: malformed packet)
 */
#pragma pack(1)


// ============================================================================
// CORE STRUCTS & SYSTEM HEADERS
// ============================================================================

struct AddBall_header {
    uint8 packet_type;            /* 0 = Full Absolute State Snapshot, 1 = Delta Frames */
    uint32 stamp;                 /* Monotonic server execution baseline tick stamp */
};

struct NameSector {
    uint8  name_len;              // Size of payload scaled in 16-bit word lengths
    uint16 name[1];               // UTF-16 representation array allocation boundary
};

struct BallHeader {
    int64 entityID;               // Unique database entity descriptor (0x08 Memory Offset)
    uint8 mode;                   // Target movement execution mode enum (0x198 Memory Offset)
    float radius;                 // Collision shell/bounding extent (0x18 / 0x60 Memory Offset)
    double posX;                  // Global absolute spatial matrix translations
    double posY;
    double posZ;
    uint8 flags;                  // Status logic switches bitfield mask (Ball::Flag)
};


/**
 * @brief Dynamic Mass Parameter Tracking Sector
 * @note CONDITIONAL PAYLOAD: Streamed ONLY if (BallHeader::mode != Ball::Mode::RIGID)
 */
struct MassSector {
    double mass;                  // Static baseline weight property (0x68 Memory Offset)
    uint8  cloak;                 // Active vision denial masking state flag (0x53 Memory Offset)
    int64 allianceID;             // Ownership entity identifier tracking blocks
    int32 corporationID;
    int32 harmonic;               // Spatial sensor resolution signature seed modifier
};


/**
 * @brief Active Translation Vector & Capability Telemetry
 * @note CONDITIONAL PAYLOAD: Streamed ONLY if (BallHeader::flags & Ball::Flag::IsFree)
 */
struct DataSector {
    float maxSpeed;               // Maximum allowed velocity scale roof ceiling (0x78 Memory Offset)
    double velX;                  // Flat Cartesian velocity vector components (0x120 Memory Offset)
    double velY;                  // Flat Cartesian velocity vector components (0x128 Memory Offset)
    double velZ;                  // Flat Cartesian velocity vector components (0x130 Memory Offset)
    float inertia;                // Dynamic acceleration scalar time factor (0x64 Memory Offset)
    float speedfraction;          // Requested performance step scaling profile throttle
};


// ============================================================================
// COMPOSITE MINIBALL BOUNDARY MESHES
// ============================================================================

struct MiniBall {
    double posX;                  // Offset translation component vector from parent global root
    double posY;
    double posZ;
    float radius;                 // Specific individual sub-mesh collision boundary limit
};

/**
 * @note CONDITIONAL PAYLOAD: Streamed ONLY if (BallHeader::flags & Ball::Flag::HasMiniBalls)
 */
struct MiniBallList {
    uint16 count;                 // Dynamic quantity layout iterator count limit
    MiniBall balls[1];            // Inline trailing contiguous data buffer
};


// ============================================================================
// MODE-SPECIFIC SUB-SECTOR STRUCTS
// ============================================================================

struct GOTO_Struct {
    uint8  formationID;           // Assigned tracking flight group descriptor context
    double x;                     // Target spatial destination matrix (Maps to 0x150)
    double y;
    double z;
};

struct FOLLOW_Struct {
    uint8  formationID;
    int64 followID;               // Target entity identifier to lock trajectories onto
    float followRange;            // Safe tracking gap constraint distance parameter
};

struct STOP_Struct {
    uint8  formationID;           // Forces rapid alignment-free speed reduction looping
};

struct WARP_Struct {
    uint8  formationID;
    double targX;                 // Termination exit coordinates for warp drop
    double targY;
    double targZ;
    int32 effectStamp;            // Initialization tick baseline timestamp for linear scaling
    double distance;              // Reinterpreted from int64: Distance parameters (-1.0 = Default)
    double trackingFlags;         // Reinterpreted from int64: Dynamic tracking parameters / flags
    int32 speed;                  // Configured target speed tier factor setting
};

struct ORBIT_Struct {
    uint8  formationID;
    uint32 targetID;              // Target entity to project perpendicular circular track around
    double followRange;           // Commanded orbit radius setting (Maps to 0x70 Memory Offset)
};

struct MISSILE_Struct {
    uint8  formationID;
    int64 targetID;               // Targeted object proxy tracker pointer
    float followRange;            // Containment blast payload detonate radius scale
    int64 ownerID;                // Launcher context identifier tracking parameter
    int32 effectStamp;            // Launch initialization execution timestamp reference
    double x;                     // Last known trajectory vector target metrics
    double y;
    double z;
};

struct MUSHROOM_Struct {
    uint8  formationID;
    float maxRadius;              // Absolute boundary ceiling of explosive expansion volume
    double waveFactor;            // Radial deployment speed wave factor multiplier
    int32 effectStamp;            // Volumetric deployment timeline step execution stamp
    int64 ownerID;                // Source tracking context attribution ID
};

struct TROLL_Struct {
    uint8  formationID;
    int32 effectStamp;            // Creation timestamp; triggers RIGID mutation when threshold hit
};

struct FIELD_Struct {
    uint8  formationID;           // Static geometry boundary conditional checking payload
};

struct RIGID_Struct {
    uint8  formationID;
    //uint16 visualStateKey;        // Optional state key; handles client mesh orientation overrides
};

struct FORMATION_Struct {		  // ONLY used by slaves with no other mode structure; linked via leaderID
    uint8  formationID;			  // formation index (0=Point, 1=Sphere, 2=Plane, 3=Wall, 4=Arrow, etc).
    int64 leaderID;               // formation leader (focus entity) tracking index
    float spacing;                // see notes below
    int32 syncIndex;              // see notes below
};

#pragma pack()


/*  spacing notes:
  group separation spacing in meters in addition to <formation> outline spacing
   Tighter spacing for small rat combat units: 400.0
   Civilian suggestion:  500.0 - 1500.0

*/

/*  syncIndex notes:
Structural orchestration syncing index
the syncIndex is a powerful tool to offload AI calculation straight to the player's computer

tracker for the group's flight elasticity.
Low Values (0 or 1):
Tells the client to enforce a Rigid Formation. The ships move like a solid geometric block. If the leader turns 45 degrees,
the entire fleet rotates in perfect synchronization. This is great for military patrols or visual parade flybys.
Dynamic Values / State IDs (e.g., 2, 5, or custom ticks):
Tells the client to switch to Loose Formation or Elastic Mode. The client's destiny rendering core will allow individual rat
escorts to visually "drift" or lagoon away from their strict matrix slots when tracking a player, performing orbital loops,
or executing weapon-alignment arcs.
Micro-Warp Drive (MWD) & Warp In/Out Orchestration
This is where the name "effectStamp" truly comes from.
When a group of NPCs decides to execute a coordinated group warp-in or fire their Micro-Warp Drives simultaneously to close
the gap on a player ship, the server increments this stamp.
Visual Triggering:
When the client catches an updated effectStamp value associated with a combat command state, it triggers the localized visual
environment shaders natively on the player's GPU.
The Result:
It forces the entire rat wing to play their MWD blue-engine flash effect or warp-entry trails in perfect, unified visual lockstep,
making the NPC squad look like a highly trained tactical unit

By passing an syncIndex of 2 or higher, the frigate rats will smoothly orbit and break formation visuals naturally on the
player's screen whenever combat commences, while still roughly maintaining their tactical group identity.

*/

} //end Destiny

#endif
