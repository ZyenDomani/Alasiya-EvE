 /**
  * @name EVE_Swarm.h
  *     Rogue Drone (Swarm) system data for Alasiya EvEmu
  *
  * @Author:    Allan
  * @date:      14Jul26
  *
  */

#pragma once

#include "../eve-server.h"

class Client;

namespace Swarm {
    enum Size {
        None        = 0,
        Swarm       = 35,
        Frigate     = 50,
        Destroyer   = 100,
        Cruiser     = 200,
        BCruiser    = 300,
        BShip       = 350,
        Indy        = 450
    };

    // High-level behavioral states determining spatial movement vectors
    enum State {
        Offline         = 0,  // De-activated node, waiting for system grid activation
        MeshIdle        = 1,  // On grid, no active targets, holding orbital swarm formation
        MeshIntercept   = 2,  // Dynamic approach vector: swarming a target based on shared tracking profiles
        MeshScaver      = 3,  // Resource priority: broken away from combat to move toward a scrap-metal wreck
        MeshIntruding   = 4,  // Electronic superiority: locked in position while breaching a player control channel
        MeshFeral       = 5   // Isolated state: severed from the mesh network, behaving aggressively/erratically
    };


    // Low-level actions executing module ticks, damage types, and network hacks
    enum Action {
        Idle            = 0,  // Re-charging capacitors, maintaining baseline orbit
        ProbeVulnerability = 1,  // Initial cyclic attacks testing player ship resistance profiles
        ExploitFlaw     = 2,  // Hard combat focus: using optimal damage types/EWAR against discovered holes
        NaniteReclaim   = 3,  // Harvesting phase: firing tractor/repair packets onto a nearby rogue wreck
        NetworkIntrude  = 4,  // Hack phase: streaming electronic intrusion vectors against player drone channels
        SelfSacrifice   = 5   // Living shield phase: matching vectors to intercept incoming damage for a master node
    };


    enum Event {
        // --- 1. Global Mesh Priority Shifts ---
        Mesh_Coherence_Gained,
        Mesh_Coherence_Severed,
        Target_Analysis_Initiated,
        Target_Analysis_Complete,
        Optimal_Target_Pivot,

        // --- 2. Individual Drone Events ---
        Node_Damaged_Seeking_Scrap,
        Node_Assimilation_Success,
        Node_Intrusion_Attempt,
        Node_Intrusion_Success,
        Node_Terminal_Shatter,
        Node_Feral_Panic_Orbit
    };

	namespace bCast {
            enum {
        	SystemWide,   // Whole solar system local channel
        	GridBubble,   // Only players on the immediate active grid (BubbleCast)
        	DirectTarget  // Forcible packet injection directly into a target player's cockpit UI
            };
        }


    struct VulnerabilityProfile {
        float lowestResistValue = 1.0f;
        uint32_t lowestResistType = 0; // e.g., Thermal, EM
        float averageCapacitorDelta = 0.0f;
        float angularVelocityThreshold = 0.0f;
    };

    // Struct to store individual node chaos offsets inside the Master Mind memory bank
    struct FeralPanicVector {
        uint32_t droneID;
        uint32_t targetPlayerID;
        uint32_t frameSeed;         // Randomized offset generated at link sever
        float currentPitchAngle;    // Vertical oscillation tracking
        float currentYawAngle;      // Orbital tracking around player
    };

    struct PlayerVulnerabilityProfile {
        uint32_t targetID;
        // Algorithmic tracking state
        float analysisProgress = 0.0f; // Reaching 100.0f triggers complete analysis
        bool isAnalyzed = false;
        // Discovered weakness states
        uint32_t weakestDamageType = 0; // 0:EM, 1:Thermal, 2:Kinetic, 3:Explosive
        float lowestResistValue = 1.0f;
        // Environmental tracking
        uint32_t totalProbesReceived = 0;
    };

    struct HiveVulnerabilityProfile {
        float analysisProgress = 0.0f;
        bool isFullyMapped = false;
        uint32_t optimalDamageType = 0; // EM, Thermal, Kinetic, Explosive
        uint32_t sampleCount = 0;
    };

    struct ActiveHijackSession {
        uint32_t carrierID;
        uint32_t targetPlayerID;
        float firewallBreachProgress = 0.0f;
    };

    struct ActiveHarvestTether {
        uint32_t harvestingDroneID;
        uint32_t targetWreckID;
        uint32_t remainingTicks;
    };

    struct TargetWeightProfile {
        Client* pClient;
        float finalScore = 0.0f;
    };

    struct ActiveHijackSiege {
        uint32_t carrierDroneID;
        uint32_t targetPlayerID;
        float breachProgress = 0.0f; // Reaching 100.0f triggers absolute drone takeover
        uint32_t lastTickFrame;
    };

    struct SwarmMeshTelemetry {
        float cumulativeProcessingBonus = 1.0f;
        float globalTrackingMultiplier = 1.0f;
        float globalLockSpeedMultiplier = 1.0f;
        float hijackPenetrationBonus = 0.0f;
    };

    struct MatrixAnalysisNode {
        uint32_t targetPlayerID;
        uint32_t lastActiveFrame;
        float currentAnalysisPoints = 0.0f;
        bool isMappingLocked = false;
        uint32_t discoveredWeaknessDmgType = 0; // 0:EM, 1:Thermal, 2:Kinetic, 3:Explosive
        float lowestResistCached = 1.0f;
    };

    struct ActiveGridProfileSnapshot {
        uint32_t characterID;
        float currentEffectiveSigRadius;
        float currentAngularVelocity;
        bool isSpeedTanking;
        bool isEwarImmune; // e.g., Bastion Mode active
    };

    struct ShipAttributeSnapshot {
        float capacitorPercentage;
        float currentShieldHealth;
        float currentArmorHealth;
        float sensorStrengthValue;
        uint32_t activeWeaponCount;
        bool isShieldBoosterRunning;
    };

    struct ActiveHijackSiege {
        uint32_t carrierDroneID;
        uint32_t targetPlayerID;
        // Core Network Math variables
        float computationalPowerPool = 0.0f; // Accumulated Intrusion Points (IP)
        float playerFirewallThreshold = 100.0f; // Scaled by player attributes
        uint64_t sessionEstablishedFrame;
        uint64_t lastProcessedFrame;
        // Counter-Hack State variables
        bool isCounterHackActive = false;
        float counterHackProgress = 0.0f;   // Reaching 100.0f rescues the drone and drops loot
    };
}

