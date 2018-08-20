
/*
 *  EVE_Missions.h
 *   mission-specific enumerators
 *
 */

#ifndef EVE_MISSIONS_H
#define EVE_MISSIONS_H

struct MissionData {
        bool important;
        uint16 id;
        uint8 level;
        uint8 typeID;
        uint16 contentID;
        uint32 constellationID;
        uint32 corporationID;
        uint32 dungeonID;
        std::string name;
};

struct MissionOffer {
    uint8 stateID;
    uint16 missionID;
    uint16 rewardLP;
    uint16 rewardItemID;
    uint16 rewardItemQty;
    uint16 courierItemID;
    uint16 courierAmount;
    uint32 offerID;
    uint32 agentID;
    uint32 characterID;
    uint32 rewardISK;
    uint32 originID;
    uint32 destinationID;
    uint32 acceptFee;
    double expiryTime;
    double dateIssued;
    double dateAccepted;
    double dateCompleted;

};

struct CourierData {
    bool important;
    bool storyline;
    uint8 level;
    uint8 typeID;
    uint16 id;
    uint16 descID;
    uint16 itemTypeID;
    uint16 itemQty;
    uint16 rewardItemID;
    uint16 rewardItemQty;
    uint32 rewardISK;
    uint32 bonusISK;
    uint32 bonusTime;
    std::string name;
};

namespace Mission {
    namespace State {
        enum {
            Allocated   = 0,
            Offered     = 1,
            Accepted    = 2,
            Failed      = 3,
            Completed   = 4 //added
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
            Storyline   = 8,    // After every 15 regular missions completed you will be offered a storyline mission.
            Cosmos      = 9,
            Arc         = 10, //Throughout the arc, you will be offered choices which will branch the arc in one or more directions, and thus the arcs have different outcomes depending on your choices. The missions that make up these arcs typically have very good ISK rewards and the last mission of the arc typically carries a handsome reward. There are seven Epic Mission Arcs. Most players begin with The Blood-Stained Stars, an arc that can be completed in a T1 frigate and gives a boost in standings withe Sisters of Eve. Seasoned L4 runners will be doing the four empire epic arcs while the fearless pilots can do the two pirate epic arcs. Epic arcs can be repeated once every three months.
            Anomic      = 11 //optional security missions that are given out by level 4 agents. They can always be declined without penalty. Anomic missions present a different and higher challenge compared to other security missions. You will encounter a small number of very powerful adversaries and you are restricted in ship size.
        };
    }

}



#endif  // EVE_MISSIONS_H