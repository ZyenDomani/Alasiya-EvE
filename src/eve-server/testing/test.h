
 /**
  * @name test.h
  *   Code for testing miscellaneous items in Alasiya EvE
  *
  * @Author:        Allan
  * @date:          19 August 2020
  *
  */


#ifndef EVEMU_TESTING_TEST_H_
#define EVEMU_TESTING_TEST_H_

#include "character/Character.h"


class Client;

struct attrTestData {
    uint8 ancestryID;   // unique
    uint8 bloodlineID;
    uint8 aPerception;
    uint8 aWillpower;
    uint8 aCharisma;
    uint8 aMemory;
    uint8 aIntelligence;

    uint8 raceID;
    uint8 bPerception;
    uint8 bWillpower;
    uint8 bCharisma;
    uint8 bMemory;
    uint8 bIntelligence;

    uint8 cPerception;
    uint8 cWillpower;
    uint8 cCharisma;
    uint8 cMemory;
    uint8 cIntelligence;

    uint8 tPerception;
    uint8 tWillpower;
    uint8 tCharisma;
    uint8 tMemory;
    uint8 tIntelligence;

    std::string ancestryName;
    std::string bloodlineName;
    std::string typeName;
};

struct warpState {
    bool accel;
    bool cruise;
    bool decel;
    int64 warpSpeed;           //in m/s
    float warpTime;             //in s
    float accelFraction;
    float decelFraction;
    double accelDist;           //in m
    double cruiseDist;          //in m
    double decelDist;           //in m
};

class testing {
public:

    static void posTest(Client* pClient);
    void CharAttribTest();

    void WarpTest(uint8 type);

    int64  m_targetDistance;
    int64  m_accelDistance;
    int64  m_decelDistance;
    float m_shipWarpSpeed;
    float m_speedToLeaveWarp;
    void InitWarp();
    void WarpAccel(uint16 sec_into_warp);
    void WarpCruise(uint16 sec_into_warp);
    bool WarpDecel(uint16 sec_into_warp);
    void WarpUpdate(int64 currentShipSpeed, uint16 sec_into_warp, uint8 type);      // 0=error, 1=accel, 2=cruise, 3=decel



private:
    uint32 BUBBLE_RADIUS_METERS;
    bool inBubble;

    warpState wState;

    std::map<uint8, attrTestData> m_attribTest;         //ancestryID, data

};



#endif  // EVEMU_TESTING_TEST_H_