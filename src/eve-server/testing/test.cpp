
 /**
  * @name test.cpp
  *   Code for testing miscellaneous items in Alasiya EvE
  *
  * @Author:        Allan
  * @date:          19 August 2020
  *
  */

#include "iostream"

#include "../eve-server.h"
#include "../../eve-core/math/Trig.h"

#include "Client.h"
#include "StaticDataMgr.h"
#include "system/SystemEntity.h"
#include "testing/test.h"

void testing::posTest(Client* pClient) {
    SystemEntity* mySE(pClient->GetShipSE());

    sLog.Warning("\ttesting","Test competed (did nothing)");
}

void testing::UpdateCharOwners() {
    // characters should be in eveStaticOwners data...they werent, but are added now.
    // this is fix to add existing chars to data
    DBQueryResult res;
    DBResultRow row;
    DBerror err;
    std::string safename;
    sDatabase.RunQuery(res,"SELECT characterID, characterName, typeID FROM chrCharacters");
    while (res.GetRow(row)) {
        sDatabase.DoEscapeString(safename, row.GetText(1));
        sDatabase.RunQuery(err,
            "INSERT INTO eveStaticOwners (ownerID,ownerName,typeID)"
            " VALUES (%u, '%s', %u)",
                row.GetUInt(0), safename.c_str(), row.GetUInt(2));
    }
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

        m_attribTest[row.GetUInt8(0)] = data;
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

void testing::GetSkills() {

    std::map<uint32, uint8> startingSkills;
    //  Base Skills
    CharacterDB::GetBaseSkills(startingSkills); //9
    for (auto cur : startingSkills)
        sLog.Green("Base: ", "%u:%u (%s)", cur.first, cur.second, sDataMgr.GetTypeName(cur.first));

    //  Race Skills
    startingSkills.clear();
    CharacterDB::GetSkillsByRace(1, startingSkills); //32
    for (auto skill : startingSkills)
        sLog.Green("Race: ", "%s: %u:%u (%s)", sDataMgr.GetRaceName(1), skill.first, skill.second, sDataMgr.GetTypeName(skill.first));
    startingSkills.clear();
    CharacterDB::GetSkillsByRace(2, startingSkills); //32
    for (auto skill : startingSkills)
        sLog.Green("Race: ", "%s: %u:%u (%s)", sDataMgr.GetRaceName(2), skill.first, skill.second, sDataMgr.GetTypeName(skill.first));
    startingSkills.clear();
    CharacterDB::GetSkillsByRace(4, startingSkills); //32
    for (auto skill : startingSkills)
        sLog.Green("Race: ", "%s: %u:%u (%s)", sDataMgr.GetRaceName(4), skill.first, skill.second, sDataMgr.GetTypeName(skill.first));
    startingSkills.clear();
    CharacterDB::GetSkillsByRace(8, startingSkills); //32
    for (auto skill : startingSkills)
        sLog.Green("Race: ", "%s: %u:%u (%s)", sDataMgr.GetRaceName(8), skill.first, skill.second, sDataMgr.GetTypeName(skill.first));

    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT raceID, careerID, careerName FROM careers WHERE raceID < 10"); //15
    DBResultRow row;
    while (res.GetRow(row)) {
        //  Career Skills
        startingSkills.clear();
        CharacterDB::GetSkillsByCareer(row.GetUInt(1), startingSkills); //162
        for (auto skill : startingSkills)
            sLog.Green("Career: ", "%s/%s(%u): %u:%u (%s)", sDataMgr.GetRaceName(row.GetUInt(0)), \
            row.GetText(2), row.GetUInt(1), skill.first, skill.second, sDataMgr.GetTypeName(skill.first));
    }
}

// 11jan25
void testing::WarpTest(uint8 type) {
    double profileStartTime = GetTimeUSeconds();

    BUBBLE_RADIUS_METERS = 300000;

    inBubble = true;

    wState = warpState();

    m_speedToLeaveWarp = 265.0f;
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
        case 6:
            m_targetDistance = 6731904181500; //40 au
            break;
        case 7:
            m_targetDistance = 17951744484000; //120 au
            break;
        default:
            m_targetDistance = 897587224200; //6 au
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

    //sLog.Green("WarpTest", "actual accel distance: %lli.", m_accelDistance);
    //sLog.Green("WarpTest", "actual decel distance: %lli.", m_decelDistance);
    sLog.Cyan("WarpTest", "runtime: %.3fus.", GetTimeUSeconds() - profileStartTime);
}


void testing::InitWarp() {
    //  150km - 15s, 1mkm - 23s, 1au - 29s base + ship's wsm
    double decelTime(1.0f), cruiseTime(0.0f);
    int64 accelDistance(0), decelDistance(0), cruiseDistance(0);
    int64 warpSpeedInMeters(m_shipWarpSpeed * ONE_AU_IN_METERS);
    // set times and distances based on target distance
    if (m_targetDistance < (warpSpeedInMeters * 3)) {
        //  short warp....no cruise
        // accel = 1/3 decel
        accelDistance = (m_targetDistance / 3);
        m_accelTime = log(accelDistance / 3) / 3;
        //decelTime += m_accelTime * 2;
        warpSpeedInMeters = accelDistance;
    } else {
        accelDistance = warpSpeedInMeters;       // ship warp speed in meters
        //m_accelTime = (27 + (m_shipWarpSpeed / 3)) / 3;
        m_accelTime = log(accelDistance / 3) / 3;
        //decelTime += (m_accelTime * 2);
        cruiseDistance = ((double)m_targetDistance - (accelDistance * 3));
        cruiseTime = EvE::max(cruiseDistance / warpSpeedInMeters, 1);
    }

    uint16 intAccel = m_accelTime;
    wState.accelFraction = (m_accelTime - intAccel);

    decelDistance = accelDistance * 2;

    m_decelTime = log(decelDistance);
    //m_decelTime += adjust;
    uint16 intDecel = decelTime;
    wState.decelFraction = (decelTime - intDecel);

    //  set total warp time based on above math.
    wState.warpTime = m_accelTime + m_decelTime + cruiseTime;

    decelTime = m_decelTime;
    double speed(0.0f);
    bool run(true);
    uint16 step(0);
    while (run) {
        speed = exp(--decelTime);
        ++step;
        if (speed < m_speedToLeaveWarp) {
            run = false;
        }
    }

    double distance(0.0f);
    while (step > 0) {
        distance += exp(decelTime++);
        --step;
    }

    decelDistance = distance;

    float distanceAU = (m_targetDistance / ONE_AU_IN_METERS);

    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        _log(DESTINY__WARP_TRACE, "testing::InitWarp():Calculate - Warp will accelerate for %.1fs, cruise for %.1fs, then decelerate for %.1fs, with total time of %.2fs and warp speed of %lli m/s.", \
                m_accelTime, cruiseTime, decelTime, wState.warpTime, warpSpeedInMeters);
        if (wState.cruiseDist > 0.0) {
            _log(DESTINY__WARP_TRACE, "testing::InitWarp():Calculate - Accel distance is %lli  Cruise distance is %lli   Decel distance is %lli.  Total distance is %lli (%.2f au).", \
                    accelDistance, cruiseDistance, decelDistance, m_targetDistance, distanceAU);
        } else {
            int64 cruiseDist = m_targetDistance - accelDistance - decelDistance;
            _log(DESTINY__WARP_TRACE, "testing::InitWarp():Calculate - Accel distance is %lli  Cruise distance is %lli   Decel distance is %lli.  Total distance is %lli (%.2f au).", \
                    accelDistance, cruiseDist, decelDistance, m_targetDistance, distanceAU);
        }
    }

    wState.accel = true;
    wState.decel = false;
    wState.cruise = false;
    wState.warpSpeed = warpSpeedInMeters;     //in m/s
    wState.accelDist = accelDistance;         //in m
    wState.decelDist = decelDistance;         //in m
    wState.cruiseDist = cruiseDistance;       //in m
}

void testing::WarpAccel(uint16 sec_into_warp) {
    float accelTime(sec_into_warp + wState.accelFraction);
    int64 currentDistance = exp(3 * accelTime);

    if (accelTime >= m_accelTime) {
        wState.accel = false;
        currentDistance = wState.accelDist - m_accelDistance;

        if (wState.cruiseDist > 0.0) {
            wState.cruise = true;
        } else {
            wState.decel = true;
            m_targetDistance = wState.decelDist + currentDistance;
        }
    }

    m_accelDistance += currentDistance;
    WarpUpdate(currentDistance, sec_into_warp, 1);

    if (inBubble)
        if (m_accelDistance > BUBBLE_RADIUS_METERS) {
            // this will use actual InBubble() check
            inBubble = false;
            sLog.Error("WarpTest", "remove ship from bubble.");
        }

    //sLog.Cyan("WarpTest", "accelTime: %.3f  distance: %lli  totalDistance: %lli", \
            accelTime, currentDistance, m_accelDistance);
}

void testing::WarpCruise(uint16 sec_into_warp) {
    // in cruise....only updating position data.
    wState.cruiseDist -= wState.warpSpeed;
    //sLog.Cyan("WarpTest", "cruiseDist: %.3f", wState.cruiseDist);

    WarpUpdate(wState.warpSpeed, sec_into_warp, 2);

    if (m_targetDistance - wState.warpSpeed < wState.decelDist) {
        m_targetDistance = wState.decelDist;
        wState.cruise = false;
        wState.decel = true;
    }
}

bool testing::WarpDecel(uint16 sec_into_warp) {
    /* For deceleration, k = -1
     * distance = e^(k*s)
     * speed = k*e^(k*s)
     */

    double decelTime = --m_decelTime;
    int64 currentShipSpeed = exp(decelTime);

    m_decelDistance += currentShipSpeed;

    //sLog.Cyan("WarpTest", "decelTime: %.3f  currentShipSpeed: %lli  totalDistance: %lli", \
            decelTime, currentShipSpeed, m_decelDistance);

    WarpUpdate(currentShipSpeed, sec_into_warp, 3);

    if (!inBubble)
        if (m_targetDistance < BUBBLE_RADIUS_METERS) {
            // this will use actual InBubble() check
            inBubble = true;
            sLog.Green("WarpTest", "add ship to bubble.");
        }

    if (currentShipSpeed <= m_speedToLeaveWarp) {
        sLog.Warning("WarpTest", "speed < m_speedToLeaveWarp");
        return true;
    }

    return false;
}

void testing::WarpUpdate(int64 currentDistance, uint16 sec_into_warp, uint8 type/*0*/) {
    //  track position and velocity for all stages.
    m_targetDistance -= currentDistance;

    switch (type) {
        case 1: {
            if (is_log_enabled(DESTINY__WARP_TRACE))
                _log(DESTINY__WARP_TRACE, "testing::WarpAccel(): Warp Accelerating(%us): \t velocity \t %lli m/s.  \t %lli m remaining.", \
                        sec_into_warp, (currentDistance * 3), m_targetDistance);
        } break;
        case 2: {
            if (is_log_enabled(DESTINY__WARP_TRACE))
                _log(DESTINY__WARP_TRACE, "testing::WarpCruise(): Warp Crusing(%us): \t velocity \t %lli m/s. \t %lli m remaining.", \
                        sec_into_warp, currentDistance, m_targetDistance);
        } break;
        case 3: {
            if (is_log_enabled(DESTINY__WARP_TRACE))
                _log(DESTINY__WARP_TRACE, "testing::WarpDecel(): Warp Decelerating(%us): \t velocity \t %lli m/s. \t %lli m remaining.", \
                        sec_into_warp, currentDistance, m_targetDistance);
        } break;
    }
}

//22jan25
void testing::TurnTest(uint8 type) {
    double profileStartTime = GetTimeUSeconds();
    m_stop = false;
    m_agility = 18.4f;
    m_alignTime = 25.50871f;
    m_maxShipSpeed = 140;
    m_turnMinFraction = 0.0;
    m_prevSpeedFraction = 0.0;
    m_userSpeedFraction = 1.0f;
    m_activeSpeedFraction = 0.652;

    switch (type) {
        case 1:
            degrees = 15.0;
            break;
        case 2:
            degrees = 30.0;
            break;
        case 3:
            degrees = 45.0;
            break;
        case 4:
            degrees = 60.0;
            break;
        case 5:
            degrees = 90.0;
            break;
        case 6:
            degrees = 135.0;
            break;
    }

    InitTurn();

    while (++m_turnTime < m_alignTime) {
        Turn();
    }
    sLog.Cyan("TurnTest", "runtime: %.3fus.", GetTimeUSeconds() - profileStartTime);
}

void testing::InitTurn() {
    m_turnAccel = false;
    m_turnDecel = false;
    m_wasDecel = false;
    m_turnTime = 0;

    //  calc min speed for this turn as absolute percent of shipMaxSpeed
    float radians = EvE::Trig::Deg2Rad(degrees);
    m_turnMinFraction = sqrt((cos(radians) + 1) / 2);

    m_turnPct = 1.0f / m_alignTime;

    // check speed for changes and set vars accordingly
    m_prevSpeedFraction = m_activeSpeedFraction;
    if (m_activeSpeedFraction > m_turnMinFraction) {
        m_turnDecel = true;
        m_wasDecel = true;
    } else {
        m_turnAccel = true;
    }

    sLog.Green("TurnTest", "%.0f degree test init. start speed: %.1f  max speed: %.1f", \
            degrees, m_activeSpeedFraction * m_maxShipSpeed, m_maxShipSpeed);
    sLog.Green("TurnTest", "asf %.3f mtsf %.3f, pct:%.2f, alignTime:%.1f", \
            m_activeSpeedFraction, m_turnMinFraction, m_turnPct, m_alignTime);
}

void testing::Turn() {
    std::string move = "";
    float speed(0.0f);
    float change(m_turnPct * m_turnTime);

    // update tf for this tic
    m_timeFraction = (1 - exp(-m_turnTime / m_agility));

    // accel/decel in turn act different. ignore MoveObject() speed and set per turn change here (+5-9us)
    if (m_turnDecel) {
        // our speed is above min for this turn.
        //if (m_turnTime > (m_alignTime * 0.5f)) {
        if (m_activeSpeedFraction > m_turnMinFraction) {
            m_turnDecel = false;
            m_turnAccel = true;
            // we are now resuming accel after 1/2 turn
            m_activeSpeedFraction = getPctf(m_turnMinFraction, m_userSpeedFraction, 1.0f - (change * 2));
            //speed = m_maxShipSpeed * m_activeSpeedFraction;
            move = "continue accel in turn";
        } else {
            m_activeSpeedFraction = getPctf(m_prevSpeedFraction, m_turnMinFraction, 1.0f - (change * 2));
            //speed = m_maxShipSpeed * m_activeSpeedFraction;
            move = "decel in turn";
        }
        // at this point, asf < mtsf @ InitTurn().
        // however, we need to make sure asf < mtsf during first half of turn
        if (m_turnTime < (m_alignTime * 0.5f)) {
            sLog.Warning("TurnTest", "turn time %u < 1/2 align time %u.  asf %.3f mtsf %.3f", \
                    m_turnTime, m_alignTime, m_activeSpeedFraction, m_turnMinFraction);
            if (m_activeSpeedFraction > m_turnMinFraction) {
                sLog.Warning("Turn()", "asf > mtsf during first half. ");
            }
        }
        // have to figure out how to keep this down and everything sane at same time.
    } else if (m_turnAccel) {
        // we are below mtsf, either begin accel for align or resuming accel after 1/2 turn
        if (m_wasDecel) {
            m_activeSpeedFraction = getPctf(m_turnMinFraction, m_userSpeedFraction, change * 2);
        } else {
            m_activeSpeedFraction = getPctf(m_prevSpeedFraction, m_userSpeedFraction, change);
        }
        move = "accel in turn";
    } else {
        // error.  see if we can recover and continue
        move = "error";
    }

    speed = m_maxShipSpeed * m_activeSpeedFraction;

    sLog.Cyan("TurnTest", "%s at %u", move.c_str(), m_turnTime);
    sLog.Cyan("TurnTest", "accel:%s, decel:%s, change: %.4f,  speed: %.1f  asf %.3f, tf: %.3f", \
            m_turnAccel?"true":"false", m_turnDecel?"true":"false", change, \
            speed, m_activeSpeedFraction, m_timeFraction);
}

void testing::RunDroneAttribs() {
    // get drone movement attribs and calculate agility and friends

    double start(GetTimeUSeconds());
    // get drone ids from db  138 total
    std::vector<InventoryItemRef> droneRefs;
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT typeID FROM invTypes WHERE groupID IN (SELECT groupID  FROM invGroups WHERE categoryID = 18)");
    DBResultRow row, row2;
    while (res.GetRow(row)) {
        ItemData data = ItemData();
        data.ownerID = 1;
        data.locationID = locTemp;
        data.quantity = 1;
        data.flag = flagAutoFit;
        data.customInfo = "Drone Test";
        data.typeID = row.GetUInt(0);
        InventoryItemRef iRef = sItemFactory.SpawnTempItem(data);
        if (iRef.get() != nullptr)
            if (iRef->type().published())
                droneRefs.push_back(iRef);
    }

    double mass(0.0), inertiaMod(0.0), agility(0.0), alignTime(0.0), turnPct(0.0), accelTime(0.0);
    // loop thru drone refs and perform math as directed
    for (auto &cur : droneRefs) {
        sLog.Yellow("   ", "%s(%u) [%s]:  AttrMaxVelocity: %u, AttrEntityCruiseSpeed: %u", \
                cur->name(), cur->typeID(), cur->type().groupName().c_str(), cur->GetAttribute(AttrMaxVelocity).get_uint32(), \
                cur->GetAttribute(AttrEntityCruiseSpeed).get_uint32());

        /*
        sLog.Yellow("   ", "%s(%u) [%s]:  AttrMaxRange: %u, AttrOrbitRange: %u, AttrEntityAttackRange: %u, AttrFalloff: %u, AttrEntityChaseMaxDistance: %u, AttrProximityRange: %u", \
                cur->name(), cur->typeID(), cur->type().groupName().c_str(), cur->GetAttribute(AttrMaxRange).get_uint32(), \
                cur->GetAttribute(AttrOrbitRange).get_uint32(), cur->GetAttribute(AttrEntityAttackRange).get_uint32(), \
                cur->GetAttribute(AttrFalloff).get_uint32(), cur->GetAttribute(AttrEntityChaseMaxDistance).get_uint32(), \
                cur->GetAttribute(AttrProximityRange).get_uint32());
        */
        /*
        mass = cur->mass();
        inertiaMod = cur->GetAttribute(AttrInertiaMod).get_double();
        agility = mass * inertiaMod / 1000000;
        alignTime = 1.386294 * agility;
        turnPct = 1.0f / alignTime;
        accelTime = (-log(ASF_CHECK) * agility);

        // print out data
        sLog.Yellow("   ", "%s:  mass: %.3f, inertiaMod: %.3f, agility: %.3f, alignTime: %.3f, turnPct: %.3f, accelTime: %.3f ", \
                cur->name(), mass, inertiaMod, agility, alignTime, turnPct, accelTime);
                */
    }

    sLog.Cyan("dronetest runtime", "%u items processed in %.5fus", (uint8)droneRefs.size(), GetTimeUSeconds() - start);
}

// 5Feb25
/*
17:09:39 G   Alasiya's EvEMu: NumberTest in progress:
17:09:39 C f runtime:   1497.25000us
17:09:39 C d runtime:   1601.50000us
17:09:39 C i runtime:   968.75000us
17:09:39 C u runtime:   788.75000us
17:09:39 C b runtime:   870.25000us

speed order fastest -> slowest
uint32 -> int64 -> int32 -> float -> double
*/
void testing::NumberTest() {
    // time ops with diff variable types.  30k iterations
    float f(0.01f), ft(0.0f);
    double d(0.01), dt(0.0);
    int i(0), it(0);
    uint32 u(0), ut(0);
    int64 b(0), bt(0);

    uint32 run(0);

    double start(GetTimeUSeconds());
    while (run < 30000) {
        f += 5;
        f *= 15;
        f /= 3;

        ft = f;
        ft += f / 6;
        ft = f * 8 / 30;
        ft *= 184;
        ++run;
    }
    sLog.Cyan("f runtime", "  %.3fus", GetTimeUSeconds() - start);

    run = 0;
    start = GetTimeUSeconds();
    while (run < 30000) {
        d += 5;
        d *= 15;
        d /= 3;

        dt = d;
        dt += d / 6;
        dt = d * 8 / 30;
        dt *= 184;
        ++run;
    }
    sLog.Cyan("d runtime", "  %.3fus", GetTimeUSeconds() - start);

    run = 0;
    start = GetTimeUSeconds();
    while (run < 30000) {
        i += 5;
        i *= 15;
        i /= 3;

        it = i;
        it += i / 6;
        it = i * 8 / 30;
        it *= 184;
        ++run;
    }
    sLog.Cyan("i runtime", "  %.3fus", GetTimeUSeconds() - start);

    run = 0;
    start = GetTimeUSeconds();
    while (run < 30000) {
        u += 5;
        u *= 15;
        u /= 3;

        ut = u;
        ut += u / 6;
        ut = u * 8 / 30;
        ut *= 184;
        ++run;
    }
    sLog.Cyan("u runtime", "  %.3fus", GetTimeUSeconds() - start);

    run = 0;
    start = GetTimeUSeconds();
    while (run < 30000) {
        b += 5;
        b *= 15;
        b /= 3;

        bt = b;
        bt += b / 6;
        bt = b * 8 / 30;
        bt *= 184;
        ++run;
    }
    sLog.Cyan("b runtime", "  %.3fus", GetTimeUSeconds() - start);

}


void testing::GetAgentPics() {
    /* after not finding agent pictures and not wanting to keep hitting live,
     *  i am tryin this....dl the pics from live once and store them on my server for disto
     *  there's 10977 agents.  this may take a minute...(took 2.3h)
     *
     * http://images.evetech.net/Character/<charID>_64.jpg
     */

    std::vector<uint32> agentIDs;
    DBQueryResult res;
    DBResultRow row;
    sDatabase.RunQuery(res, "SELECT agentID FROM agtAgents");
    while (res.GetRow(row)) {
        //SELECT agentID, locationID FROM agtAgents
        agentIDs.push_back(row.GetInt(0));
    }

    for (auto &cur : agentIDs) {
        std::cout << "wget http://images.evetech.net/Character/" << cur << "_256.jpg -P /srv/games/eve/Alasiya-EvE/image_cache/Agent/" << std::endl;
    }
}

void testing::FixDungeonGroupData() {
    std::vector<uint16> typeIDs;
    DBerror err;
    DBQueryResult res;
    DBResultRow row;
    /*
    sDatabase.RunQuery(res, "SELECT itemTypeID FROM dunGroupData");
    while (res.GetRow(row)) {
        Inv::TypeData data = Inv::TypeData();
        sDataMgr.GetType(row.GetInt(0), data);
        sDatabase.RunQuery(err, "UPDATE dunGroupData SET itemName='%s',itemGroupID=%u  WHERE itemTypeID=%u",
            data.name.c_str(), data.groupID, data.id);
    }
*/
    sDatabase.RunQuery(res, "SELECT dunGroupID, itemGroupID FROM dunGroupData");
    while (res.GetRow(row)) {
        std::string name;
        if (row.GetInt(0) > 1000) {
            name = sDataMgr.GetGroupName(row.GetInt(1));
        } else if ((row.GetInt(0) > 619) and (row.GetInt(0) < 700)) {
            name = sDataMgr.GetGroupName(row.GetInt(1));
        } else {
            name = GetDungeonGroupName(row.GetInt(0));
        }
        std::string safename;
        sDatabase.DoEscapeString(safename, name);

        sDatabase.RunQuery(err, "UPDATE dunGroupData SET groupName='%s' WHERE dunGroupID=%u",
            name.c_str(), row.GetInt(0));
    }

}

const char* testing::GetDungeonGroupName(uint16 grpID) {
    return "none";
}

/*
 *    templateID = (sig.dungeonType * 10000) + (sec * 1000) + (type * 100) + (level * 10) + factionID;
 *
 *    uint8 factionID   = templateID % 10;
 *    uint8 level       = templateID / 10 % 10;
 *    uint8 type        = templateID / 100 % 10;
 *    uint8 sec         = templateID / 1000 % 10;
 *    uint8 dungeonType = templateID / 10000 % 10;
 */
void testing::UpdateDungeons() {
    DBerror err;
    DBQueryResult res;
    DBResultRow row;
    uint8 typeID(0);
    uint32 factionID(0);
    uint8 archetypeID(0);
    sDatabase.RunQuery(res, "SELECT templateID, templateName, dunRoomID FROM dunTemplates");
    while (res.GetRow(row)) {
        std::string name = row.GetText(1);
        std::string safename;
        sDatabase.DoEscapeString(safename, name);

        factionID = row.GetInt(0) % 10;
        typeID = row.GetInt(0) / 100 % 10;
        archetypeID = row.GetInt(0) / 10000 % 10;

        switch (factionID) {
            // A = site - 1:mission, 2:grav, 3:mag, 4:radar, 5:ladar, 6:ded, 7:anomaly, 8:unrated, 9:escalation
            // E = faction - 0=code defined, 1=Serpentis, 2=Angel, 3=Blood, 4=Guristas, 5=Sansha, 6=Drones
            case 1:  factionID = factionSerpentis;  break;
            case 2:  factionID = factionAngel;  break;
            case 3:  factionID = factionBloodRaider;  break;
            case 4:  factionID = factionGuristas;  break;
            case 5:  factionID = factionSanshas;  break;
            case 6:  factionID = factionUnknown;  break;
        }

        sDatabase.RunQuery(err, "INSERT INTO dunDungeons("
            "dungeonID, dungeonName, dungeonStatus, dungeonNameID, factionID, archetypeID, dungeonTemplateID) "
            " VALUES (%u,'%s',2,null,%u,%u,%u)",
                           row.GetInt(0), safename.c_str(), factionID, archetypeID, row.GetInt(0));

        //insert room into dunRooms table
        sDatabase.RunQuery(err, "INSERT INTO dunRooms(dungeonID, roomID, roomOrdinal, roomName)"
        " VALUES (%u, %u, 1, '%s')", row.GetInt(0), row.GetInt(2), safename.c_str());
    }


    sLog.Cyan("UpdateDungeons", " completed");
}
