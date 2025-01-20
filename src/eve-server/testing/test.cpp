
 /**
  * @name test.cpp
  *   Code for testing miscellaneous items in Alasiya EvE
  *
  * @Author:        Allan
  * @date:          19 August 2020
  *
  */

#include "eve-server.h"

#include "Client.h"
#include "StaticDataMgr.h"
#include "system/SystemEntity.h"
#include "testing/test.h"

void testing::posTest(Client* pClient) {
    SystemEntity* mySE(pClient->GetShipSE());

    sLog.Warning("\ttesting","Test competed");
}

void testing::CharAttribTest() {
    /* this is for testing and setting default character attributes
     * it will need names, Base, Ancestry.Bonus, BL.Bonus
     */

    DBQueryResult res, res2, res3, res4;
    DBResultRow row, row2, row3, row4;

    // ancestryID is unique.  get it first.  (42)
    sDatabase.RunQuery(res,
    "SELECT ancestryID, ancestryName, bloodlineID, perception, willpower, charisma, memory, intelligence "
    " FROM chrAncestries");

    while (res.GetRow(row)) {
        attrTestData data = attrTestData();
        data.ancestryID = row.GetUInt8(0);   // unique
        data.ancestryName = row.GetText(1);
        data.bloodlineID = row.GetUInt8(2);
        data.aPerception = row.GetUInt8(3);
        data.aWillpower = row.GetUInt8(4);
        data.aCharisma = row.GetUInt8(5);
        data.aMemory = row.GetUInt8(6);
        data.aIntelligence = row.GetUInt8(7);

        // bloodline is based on ancestry  (14)
        sDatabase.RunQuery(res2,
        "SELECT bloodlineName, raceID, perception, willpower, charisma, memory, intelligence"
        " FROM chrBloodlines"
        " WHERE bloodlineID = %u", row.GetUInt8(2));
        res2.GetRow(row2);

        data.bloodlineName = row2.GetText(0);
        data.raceID = row2.GetUInt8(1);
        data.bPerception = row2.GetUInt8(2);
        data.bWillpower = row2.GetUInt8(3);
        data.bCharisma = row2.GetUInt8(4);
        data.bMemory = row2.GetUInt8(5);
        data.bIntelligence = row2.GetUInt8(6);

        sDatabase.RunQuery(res3, "SELECT typeID FROM bloodlineTypes WHERE bloodlineID = %u", row.GetUInt8(2));
        res3.GetRow(row3);

        const ItemType *iType = sItemFactory.GetType(row3.GetUInt(0));

        data.typeName = iType->name();
        data.cPerception = iType->GetAttribute(AttrPerception).get_uint32();
        data.cWillpower = iType->GetAttribute(AttrWillpower).get_uint32();
        data.cCharisma = iType->GetAttribute(AttrCharisma).get_uint32();
        data.cMemory = iType->GetAttribute(AttrMemory).get_uint32();
        data.cIntelligence = iType->GetAttribute(AttrIntelligence).get_uint32();

        data.tPerception = data.aPerception + data.bPerception + data.cPerception;
        data.tWillpower = data.aWillpower + data.bWillpower + data.cWillpower;
        data.tCharisma = data.aCharisma + data.bCharisma + data.cCharisma;
        data.tMemory = data.aMemory + data.bMemory + data.cMemory;
        data.tIntelligence = data.aIntelligence + data.bIntelligence + data.cIntelligence;

        m_attribTest[row.GetUInt8(0)] = std::move(data);
    }

    sLog.Green("Character Base Attributes", "%u Ancestry ID's registered.", m_attribTest.size());
    for (auto& cur : m_attribTest) {
        sLog.Blue("Character ", "%s", sDataMgr.GetRaceName(cur.second.raceID));
        sLog.Yellow("        ", "Per: %u, Wil: %u, Cha: %u, Mem: %u, Int: %u", \
                cur.second.tPerception, cur.second.tWillpower, cur.second.tCharisma, \
                cur.second.tMemory, cur.second.tIntelligence);


        sLog.Blue("          ", "Ancestry: %s", cur.second.ancestryName.c_str());
        sLog.Yellow("        ", "Per: %u, Wil: %u, Cha: %u, Mem: %u, Int: %u", \
                cur.second.aPerception, cur.second.aWillpower, cur.second.aCharisma, \
                cur.second.aMemory, cur.second.aIntelligence);


        sLog.Blue("          ", "Bloodline: %s", cur.second.bloodlineName.c_str());
        sLog.Yellow("        ", "Per: %u, Wil: %u, Cha: %u, Mem: %u, Int: %u", \
                cur.second.bPerception, cur.second.bWillpower, cur.second.bCharisma, \
                cur.second.bMemory, cur.second.bIntelligence);
    }
}

void testing::WarpTest(uint8 type) {
    double profileStartTime(GetTimeUSeconds());

    BUBBLE_RADIUS_METERS = 300000;

    inBubble = true;

    wState = warpState();

    m_speedToLeaveWarp = 265;
    // warp speed 6au/s
    // ONE_AU_IN_METERS = 149597870700
    m_shipWarpSpeed = 6;

    m_accelDistance = 0;
    m_decelDistance = 0;
    m_targetDistance = 0;

    switch (type) {
        case 1:
            m_targetDistance = 150000;  //150km
            break;
        case 2:
            m_targetDistance = 1000000000;  //1mkm
            break;
        case 3:
            m_targetDistance = 149597870700;  //1 au
            break;
        case 4:
            m_targetDistance = 299195741400;  //2 au
            break;
        case 5:
            m_targetDistance = 1645576577700; //11 au
            break;
        default:
            m_targetDistance = 6731904181500; //40 au
            break;
    }

    InitWarp();

    uint8 curTime(0);
    while (1 /*curTime < wState.warpTime*/) {
        if (wState.accel) {
            WarpAccel(curTime);
        } else if (wState.cruise) {
            WarpCruise(curTime);
        } else if (wState.decel) {
            if (WarpDecel(curTime)) {
                sLog.Warning("WarpTest", "Done");
                break;
            }
        } else {
            sLog.Error("WarpTest", "Done with error");
        }

        ++curTime;
    }

    sLog.Green("WarpTest", "actual accel distance: %lli.", m_accelDistance);
    sLog.Green("WarpTest", "actual decel distance: %lli.", m_decelDistance);
    sLog.Cyan("WarpTest", "runtime: %.3fus.", GetTimeUSeconds() - profileStartTime);
}


void testing::InitWarp() {
    //  150km - 15s, 1mkm - 23s, 1au - 29s base + ship's wsm
    double accelTime(0), decelTime(1.0), cruiseTime(0.0), cruiseDistance(0.0);
    double accelDistance(0), decelDistance(0);
    int64 warpSpeedInMeters(m_shipWarpSpeed * ONE_AU_IN_METERS);
    // set times and distances based on target distance
    if (m_targetDistance < (warpSpeedInMeters * 3)) {
        //  short warp....no cruise
        // accel = 1/3 decel
        accelDistance = (m_targetDistance / 3);
        decelDistance = (m_targetDistance - accelDistance);
        warpSpeedInMeters = accelDistance;
        accelTime = log(accelDistance / 3) / 3;
        decelTime += accelTime * 2;
    } else {
        accelTime = (27 + (m_shipWarpSpeed / 3)) / 3;
        accelDistance = warpSpeedInMeters;       // ship warp speed in meters
        decelTime += (accelTime * 2);
        decelDistance = (accelDistance * 2);
        cruiseDistance = ((double)m_targetDistance - accelDistance - decelDistance);
        cruiseTime = EvE::max(cruiseDistance / warpSpeedInMeters, 1.0);
    }

    //  set total warp time based on above math.
    wState.warpTime = accelTime + decelTime + cruiseTime;

    uint16 intAccel = accelTime;
    wState.accelFraction = (accelTime - intAccel);
    uint16 intDecel = decelTime;
    wState.decelFraction = (decelTime - intDecel);
    //wState.decelFraction += wState.accelFraction;

    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        _log(DESTINY__WARP_TRACE, "testing::InitWarp():Calculate - Warp will accelerate for %.1fs, cruise for %.1fs, then decelerate for %.1fs, with total time of %.2fs and warp speed of %lli m/s.", \
                accelTime, cruiseTime, decelTime, wState.warpTime, warpSpeedInMeters);
        _log(DESTINY__WARP_TRACE, "testing::InitWarp():Calculate - Accel distance is %.1f  Cruise distance is %.1f   Decel distance is %.1f.  Total distance is %lli (%.2f au).", \
                accelDistance, cruiseDistance, decelDistance, m_targetDistance, (double)(m_targetDistance / ONE_AU_IN_METERS));
    }

    wState.accel = true;
    wState.decel = false;
    wState.cruise = false;
    wState.warpSpeed = warpSpeedInMeters;     //in m/s
    wState.accelDist = accelDistance;         //in m
    wState.cruiseDist = cruiseDistance;       //in m
    wState.decelDist = decelDistance;         //in m
}

void testing::WarpAccel(uint16 sec_into_warp) {
    float accelTime(sec_into_warp + wState.accelFraction);
    //accelTime -= wState.decelFraction;
    int64 currentDistance = exp(3 * accelTime);
    int64 currentShipSpeed = (3 * currentDistance);

    int64 testDist = exp(3 * (accelTime + 1));
    if ((testDist * 3) > wState.warpSpeed) {
        wState.accel = false;
        currentShipSpeed = wState.warpSpeed;
        currentDistance = wState.accelDist - m_accelDistance;

        if (wState.cruiseDist > 0) {
            wState.cruise = true;
        } else {
            wState.decel = true;
        }
    }

    m_accelDistance += currentDistance;

    sLog.Cyan("WarpTest", "accelTime: %.3f  distance: %lli  totalDistance: %lli", \
            accelTime, currentDistance, m_accelDistance);

    if (inBubble)
        if (m_accelDistance > BUBBLE_RADIUS_METERS) {
            inBubble = false;
            sLog.Error("WarpTest", "remove ship from bubble.");
        }

    WarpUpdate(currentShipSpeed, sec_into_warp, 1);
}

void testing::WarpCruise(uint16 sec_into_warp) {
    // in cruise....only updating position data.
    WarpUpdate(wState.warpSpeed, sec_into_warp, 2);

    wState.cruiseDist -= wState.warpSpeed;

    if ((m_targetDistance - wState.warpSpeed) < wState.decelDist) {
        m_targetDistance = wState.decelDist;
        wState.cruise = false;
        wState.decel = true;
    }

    sLog.Cyan("WarpTest", "cruiseDist: %.3f", wState.cruiseDist);
}

bool testing::WarpDecel(uint16 sec_into_warp) {
    /* For deceleration, k = -1
     * distance = e^(k*s)
     * speed = k*e^(k*s)
     */
    /*
    float decelTime(sec_into_warp - m_decelTime);
    uint16 loopCount = floor(decelTime);
    switch (loopCount) {
        case 0:  // first loop
            decelTime = 0.0f;
            break;
        case 1:  // second loop
            //decelTime = wState.accelFraction;           // 0.240        39316 m/s
            //decelTime = wState.decelFraction;           // 0.481        30915 m/s
            decelTime = 1.0f - wState.decelFraction;    // 0.519        29749 m/s
            //decelTime += 0;                             // 0.759        23392 m/s  **default w/o change
            break;
        default:
            //--decelTime += wState.decelFraction;        // 1.240         14469 m/s
            --decelTime -= wState.decelFraction;        // 1.279         13918 m/s
            //--decelTime += wState.accelFraction;        // 1.480         11381 m/s
            //--decelTime -= wState.accelFraction;        // 1.519         10944 m/s
            //--decelTime += 0;                           // 1.759          8610 m/s
            break;
    }
    //sLog.Cyan("WarpTest", "loopCount: %u  decelTime: %.3f", loopCount, decelTime);

    //int64 currentShipSpeed = (wState.warpSpeed * exp(-decelTime));
    */

    float decelTime = --wState.warpTime;
    int64 currentShipSpeed = exp(decelTime);

    m_decelDistance += currentShipSpeed;

    WarpUpdate(currentShipSpeed, sec_into_warp, 3);

    sLog.Cyan("WarpTest", "decelTime: %.3f  currentShipSpeed: %lli  currentDistance: %lli", \
            decelTime, currentShipSpeed, m_decelDistance);

    if (!inBubble)
        if (m_targetDistance < BUBBLE_RADIUS_METERS) {
            inBubble = true;
            sLog.Green("WarpTest", "add ship to bubble.");
        }

    if (currentShipSpeed <= m_speedToLeaveWarp) {
        sLog.Warning("WarpTest", "speed < m_speedToLeaveWarp");
        return true;
    }

    return false;
}

void testing::WarpUpdate(int64 currentShipSpeed, uint16 sec_into_warp, uint8 type/*0*/) {
    //  track position and velocity for all stages.
    m_targetDistance -= currentShipSpeed;

    switch (type) {
        case 1: {
            if (is_log_enabled(DESTINY__WARP_TRACE))
                _log(DESTINY__WARP_TRACE, "testing::WarpAccel(): Warp Accelerating(%us): \t velocity \t %lli m/s.  \t %lli m remaining.", \
                        sec_into_warp, currentShipSpeed, m_targetDistance);
        } break;
        case 2: {
            if (is_log_enabled(DESTINY__WARP_TRACE))
                _log(DESTINY__WARP_TRACE, "testing::WarpCruise(): Warp Crusing(%us): \t velocity \t %lli m/s. \t %lli m remaining.", \
                        sec_into_warp, currentShipSpeed, m_targetDistance);
        } break;
        case 3: {
            if (is_log_enabled(DESTINY__WARP_TRACE))
                _log(DESTINY__WARP_TRACE, "testing::WarpDecel(): Warp Decelerating(%us): \t velocity \t %lli m/s. \t %lli m remaining.", \
                        sec_into_warp, currentShipSpeed, m_targetDistance);
        } break;
    }
}
