/*
 * EVE_Cosmic.h
 *
 *    -allan  21Jul26
 */

#ifndef _EVEMU_COSMIC_H_
#define _EVEMU_COSMIC_H_

#include "../eve-server/eve-server.h"


/* POD structure for asteroid */
struct AsteroidData {
    uint16 typeID=0;
    uint32 itemID=0;
    uint32 systemID=0;
    uint32 beltID=0;
    double quantity=0.0;
    double radius=0.0;
    Vector3d position = NULL_ORIGIN;
    std::string itemName = "none";
};

namespace BeltLayout {
    enum {
        HalfCircle          = 0,
        WeirdDiagonal       = 1,
        OddSlant            = 2,
        WTF_UFO             = 3,
        ElevatedArc         = 4,
        SphericalCloud      = 5,
        TripleHelixVortex   = 6, // YOUR HOLY GRAIL DESIGN! #1
        GravitationalOrbit  = 7  // YOUR HOLY GRAIL DESIGN! #2
    };
}

 /* POD structure for asteroid distribution methods by group */
struct OreTypeChance {
    uint16 typeID=0;
    float chance=0.0f;
};

/* POD structure for cosmic signatures/anomalies */
struct CosmicSignature {
    int8 dungeonType=0;          // internal for creation checks
    uint16 bubbleID=0;            // internal for .siglist command
    // typeID of signal
    uint16 sigTypeID=0;           // type name if scanGroupID is not sig or anom and certainty > 0.75
    // groupID of signal
    uint16 sigGroupID=0;          // group name if scanGroupID is not sig or anom and certainty > 0.25
    // groupID of signature...must be one of sig, anom, ship, drone, structure
    uint16 scanGroupID=0;         // ship,drone and structure uses sigGroupID for group name
    uint16 scanAttributeID=0;     // group naming data if scanGroupID is anom or sig and certainty > 0.25
    uint32 ownerID=0;
    uint32 systemID=0;
    uint32 sigItemID=0;           // itemID of this entry
    float sigStrength=0.0f;
    Vector3d position=NULL_ORIGIN;
    std::string sigID="";          // this is unique xxx-nnn id displayed in scanner.  can be other values
    std::string sigName="";        // site name if scanGroupID is sig or anom and certainty > 0.75
};

/* POD structure for systems. */
struct SystemData {
    uint32 systemID=0;
    uint32 constellationID=0;
    uint32 regionID=0;
    uint32 factionID=0;
    int64 radius=0;
    float security=0.0f;
    std::string name="";
    std::string securityClass="";
};

/* POD structure for solarsystem item. */
struct SolarSystemData {
    bool border=false;
    bool fringe=false;
    bool corridor=false;
    bool hub=false;
    bool international=false;
    bool region=false;
    bool constellation=false;
    uint32 systemID=0;
    uint32 constellationID=0;
    uint32 regionID=0;
    uint32 factionID=0;
    uint32 sunTypeID=0;
    int64 radius=0;
    float security=0.0f;
    float luminosity=0.0f;
    Vector3d position = NULL_ORIGIN;
    Vector3d minPosition = NULL_ORIGIN;
    Vector3d maxPosition = NULL_ORIGIN;
    std::string name="";
    std::string securityClass="";
};
struct SystemKillData {
    uint16 killsHour=0;
    uint16 kills24Hour=0;
    uint16 factionKills=0;
    uint16 factionKills24Hour=0;
    uint16 podKillsHour=0;
    uint16 podKills24Hour=0;

    int64 killsDateTime=0;
    int64 kills24DateTime=0;
    int64 factionDateTime=0;
    int64 faction24DateTime=0;
    int64 podDateTime=0;
    int64 pod24DateTime=0;
};

#endif  //_EVEMU_COSMIC_H_