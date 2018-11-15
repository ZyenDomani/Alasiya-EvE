
/*
 *  EVE_Missions.h
 *   mission-specific enumerators
 *
 */

#ifndef EVE_MISSIONS_H
#define EVE_MISSIONS_H

struct MissionData {
        bool important;
        uint8 level;
        uint8 typeID;
        uint8 range;
        uint16 missionID;
        uint32 briefingID;
        uint32 constellationID;
        uint32 corporationID;
        uint32 dungeonID;
        std::string name;
};

class PyList;
struct MissionOffer {
    bool important;
    bool storyline;
    bool remoteOfferable;
    bool remoteCompletable;
    uint8 stateID;
    uint8 typeID;
    uint8 range;
    uint16 missionID;           // this is mission title messageID for locale
    uint16 rewardLP;
    uint16 rewardItemID;
    uint16 rewardItemQty;
    uint16 courierItemID;
    uint16 courierAmount;
    uint16 destinationTypeID;
    uint32 offerID;
    uint32 agentID;
    uint32 briefingID;          // this is mission briefing messageID for locale
    //uint32 contentID;           // on live, this is specific char data for mission keywords.  we're not using it
    uint32 characterID;
    uint32 rewardISK;
    uint32 bonusISK;
    uint32 originID;
    uint32 originOwnerID;
    uint32 originSystemID;
    uint32 destinationID;
    uint32 destinationOwnerID;
    uint32 destinationSystemID;
    uint32 dungeonLocationID;
    uint32 dungeonSolarSystemID;
    uint32 acceptFee;
    float courierItemVolume;
    double bonusTime;
    double expiryTime;
    double dateIssued;
    double dateAccepted;
    double dateCompleted;
    std::string name;
    PyList* bookmarks;
};

struct CourierData {
    bool important;
    bool storyline;
    uint8 level;
    uint8 typeID;
    uint8 range;
    uint16 missionID;
    uint16 itemTypeID;
    uint16 itemQty;
    uint16 rewardItemID;
    uint16 rewardItemQty;
    uint32 briefingID;
    uint32 rewardISK;
    uint32 bonusISK;
    uint32 bonusTime;
    float itemVolume;
    std::string name;
};

namespace Mission {
    namespace State {
        enum {
            Allocated   = 0,
            Offered     = 1,
            Accepted    = 2,
            Failed      = 3,
            //added
            Completed   = 4,
            Rejected    = 5,
            Defered     = 6
        };
    }

    namespace Status {
        enum {
            Incomplete  = 0,
            Complete    = 1,
            Cheat       = 2
        };
    }

    namespace Type {
        enum {
            // i think these are arbitrary
            Tutorial    = 1,
            Encounter   = 2,
            Courier     = 3,
            Trade       = 4,
            Mining      = 5,
            Research    = 6,
            Data        = 7,
            Storyline   = 8,  // After every 15 regular missions completed you will be offered a storyline mission.
            Cosmos      = 9,
            Arc         = 10, //Throughout the arc, you will be offered choices which will branch the arc in one or more directions, and thus the arcs have different outcomes depending on your choices. The missions that make up these arcs typically have very good ISK rewards and the last mission of the arc typically carries a handsome reward. There are seven Epic Mission Arcs. Most players begin with The Blood-Stained Stars, an arc that can be completed in a T1 frigate and gives a boost in standings withe Sisters of Eve. Seasoned L4 runners will be doing the four empire epic arcs while the fearless pilots can do the two pirate epic arcs. Epic arcs can be repeated once every three months.
            Anomic      = 11, //optional security missions that are given out by level 4 agents. They can always be declined without penalty. Anomic missions present a different and higher challenge compared to other security missions. You will encounter a small number of very powerful adversaries and you are restricted in ship size.
            Burner      = 12  //Miscellanous offers that can be completed in T1 frig/dessy, that have no bearing on corp/ally standings.  these are purely personal agent requests.  all agents have a chance to give these "courier" missions, which can be decliened without penalty.
        };
    }

}

#endif  // EVE_MISSIONS_H