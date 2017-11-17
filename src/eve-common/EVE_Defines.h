
/*  EVE_Defines.h
 *   this file defines hard-coded values for game item classes
 *   we also define C-type macros for ease of readibilty in various item conditionals
 *
 */

/** @todo  these values need to be udpated (here and in server code) to match values expected/tested in client */

#ifndef EVE_DEFINES_H
#define EVE_DEFINES_H

// defined breakPoint
#define SrvPause()  do { printf("%s %d\n", __FILE__, __LINE__); getchar(); } while (0)

// bulkdata version
#define bulkDataBranch          4
#define bulkDataChangeID        70726

//  defines based on itemID, per client
#define maxNonCapitalModuleSize 500

#define minCharType             1373
#define maxCharType             1386

#define minEveMarketGroup       0
#define maxStaticChannel        1000
#define maxEveMarketGroup       350000
#define minDustMarketGroup      350001
#define maxDustMarketGroup      399999
#define minBMFolder             100000
#define maxBMFolder             300000
#define minFaction              500000
#define maxFaction              999999
#define minNPCCorporation       1000000
#define maxNPCCorporation       1000999
#define minAgent                3000000
#define maxAgent                3999999
#define minRegion               10000000
#define maxRegion               19999999
#define minWHRegion             11000000
#define maxWHRegion             11999999
#define minConstellation        20000000
#define maxConstellation        29999999
#define minWHConstellation      21000000
#define maxWHConstellation      21999999
#define minSolarSystem          30000000
#define maxSolarSystem          31999999
#define minWHSolarSystem        31000000
#define maxWHSolarSystem        31999999
#define minValidLocation        30000000
#define minValidShipLocation    30000000
#define minUniverseCelestial    40000000
#define maxUniverseCelestial    49999999
#define minStargate             50000000
#define maxStargate             59999999
#define minValidCharLocation    60000000
#define minStation              60000000
#define maxNPCStation           60999999
#define minOutpost              61000000
#define maxStation              63999999
#define minTradeCont            64000000
#define maxTradeCont            65999999
#define minOfficeFolder         66000000
#define maxOfficeFolder         67999999
#define minFactoryFolder        68000000
#define maxFactoryFolder        69999999
#define minUniverseAsteroid     70000000        // deco only
#define maxUniverseAsteroid     79999999
#define minControlBunker        80000000
#define maxControlBunker        80099999
#define maxNPCItem              89999999
#define minCharacter            90000000
#define maxCharacter            98000000
#define minAlliance             99000000
#define maxAlliance             99999999
#define minPlayerItem           100000000
#define minAsteroidItem         450000000
#define minPCCorporation        1600000000
#define maxEveItem              2147483647      // max short int32
#define maxPlayerItem           10000000000
#define minFleet                1000000888444
#define maxFleet                1990000888444

#define minFakeItem             9000000000000000000


/*
DSTLOCALBALLS = 0x0C0000000h  (3,221,225,472 decimal)      unknown where this is from
missile itemID's  dec = 9,000,000,000,000,000,000    hex = 0x7CE66C50E2840000h        from packet sniff
*/

/*
maxInt = 2147483647
maxBigint = 9223372036854775807L
minPlayerOwner = 90000000
maxPlayerOwner = 2147483647
minFakeClientItem = 17000000000000000000L

minDustUser = 1000000000
minDustCharacter = 2100000000
maxDustCharacter = 2130000000
*/

//  allan's static defines to ease code checks
//  * most of these arent implemented yet....client bracketmgr dont like them.
#define EVEMU_TEMP_ENTITY_ID          110000000
#define EVEMU_PLANET_PIN_ID           130000000
#define EVEMU_MINIMUM_DYNAMIC_ID      140000000
#define EVEMU_DRONE_ID                500000000
#define EVEMU_NPC_ID                  750000000
#define EVEMU_MISSILE_ID             1000000000
#define EVEMU_DUNGEON_ID             1200000000
#define EVEMU_PLAYER_CORP_ID         1600000000
#define EVEMU_MAX_SHORT_ID           2147483647
#define EVEMU_FLEET_ID            1000000888444
#define EVEMU_WING_ID             2000000888444
#define EVEMU_SQUAD_ID            3000000888444
#define EVEMU_MAX_LONG_ID   9223372036854775807     //this is max for a SIGNED int64.

#define IsTempPinID(pinID) \
 (pinID < 1000)

#define IsCharType(typeID) \
 ((typeID >= minCharType) && (typeID <= maxCharType))

#define IsCharacterLocation(itemID) \
 (itemID >= minValidCharLocation)

#define IsContainerLocation(itemID) \
 (itemID >= minValidShipLocation)

#define IsFleet(itemID) \
((itemID >= minFleet) && (itemID <= maxFleet))

#define IsCorp(itemID) \
((itemID >= minNPCCorporation) && (itemID <= maxNPCCorporation) \
|| ((itemID >= minPCCorporation) && (itemID < maxEveItem)))

#define IsNPCCorp(itemID) \
((itemID >= minNPCCorporation) && (itemID < maxNPCCorporation))

#define IsPlayerCorp(itemID) \
((itemID >= minPCCorporation) && (itemID < maxEveItem))

#define IsAlliance(itemID) \
((itemID >= minAlliance) && (itemID < maxAlliance))

#define IsAgent(itemID) \
((itemID >= minAgent) && (itemID < maxAgent))

#define IsFaction(itemID) \
((itemID >= minFaction) && (itemID < maxFaction))

// this covers ALL static celestial-type items
#define IsStaticMapItem(itemID) \
((itemID >= minRegion) && (itemID < maxNPCItem))

#define IsRegion(itemID) \
((itemID >= minRegion) && (itemID < minConstellation))

#define IsConstellation(itemID) \
((itemID >= minConstellation) && (itemID < minSolarSystem))

#define IsSolarSystem(itemID) \
((itemID >= minSolarSystem) && (itemID < maxSolarSystem))

#define IsKSpace(itemID) \
((itemID >= minSolarSystem) && (itemID < minWHSolarSystem))

#define IsWSpace(itemID) \
((itemID >= minWHSolarSystem) && (itemID < maxWHSolarSystem))

#define IsCelestial(itemID) \
((itemID >= minUniverseCelestial) && (itemID < minStargate))

#define IsStargate(itemID) \
((itemID >= minStargate) && (itemID < minStation))

#define IsStation(itemID) \
((itemID >= minStation) && (itemID < minTradeCont))

#define IsNPCStation(itemID) \
((itemID >= minStation) && (itemID < minOutpost))

#define IsOutpost(itemID) \
((itemID >= minOutpost) && (itemID < minTradeCont)

#define IsTrading(itemID) \
((itemID >= minTradeCont) && (itemID < minOfficeFolder))

#define IsOfficeFolder(itemID) \
((itemID >= minOfficeFolder) && (itemID < minFactoryFolder))

#define IsFactoryFolder(itemID) \
((itemID >= minFactoryFolder) && (itemID < minUniverseAsteroid))

#define IsAsteroid(itemID) \
((itemID >= minAsteroidItem) && (itemID < EVEMU_DRONE_ID))

#define IsUniverseAsteroid(itemID) \
((itemID >= minUniverseAsteroid) && (itemID < minControlBunker))

#define IsControlBunker(itemID) \
((itemID >= minControlBunker) and (itemID < 80100000))

#define IsScenarioItem(itemID) \
((itemID >= 90000000) && (itemID < EVEMU_MINIMUM_DYNAMIC_ID))

#define IsPlayerItem(itemID) \
((itemID > maxNPCItem) && (itemID < maxPlayerItem))

#define IsFakeItem(itemID) \
 (itemID >= minFakeItem)


#define FlagToSlot(flag) \
(flag - flagSlotFirst)

#define SlotToFlag(slot) \
((EVEItemFlags)(flagSlotFirst + slot))

#define IsModuleSlot(flag) \
(((flag >= flagLowSlot0) && (flag <= flagHiSlot7)) \
  || ((flag >= flagRigSlot0) && (flag <= flagRigSlot7)) \
  || ((flag >= flagSubSystem0) && (flag<=flagSubSystem7)))

#define IsCargoHoldFlag(flag) \
((flag == flagCargoHold) || (flag == flagDroneBay) || (flag == flagSecondaryStorage) || (flag == flagShipHangar) \
  || ((flag >= flagSpecializedFuelBay) && (flag <= flagSpecializedAmmoHold)))

#define IsHiSlot(flag) \
((flag >= flagHiSlot0) && (flag <= flagHiSlot7))

#define IsMidSlot(flag) \
((flag >= flagMedSlot0) && (flag <= flagMedSlot7))

#define IsLowSlot(flag) \
((flag >= flagLowSlot0) && (flag <= flagLowSlot7))

#define IsRigSlot(flag) \
((flag >= flagRigSlot0) && (flag <= flagRigSlot7))

#define IsSubSystem(flag) \
((flag >= flagSubSystem0) && (flag <= flagSubSystem7))


/*
def IsJunkLocation(locationID):
    if locationID >= 2000:
        return 0
    elif locationID in (6, 8, 10, 23, 25):
        return 1
    elif locationID > 1000 and locationID < 2000:
        return 1
    else:
        return 0

*/

#endif  //EVE_DEFINES_H