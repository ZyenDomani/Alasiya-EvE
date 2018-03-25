
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
#define maxDustMarketGroup      399999
#define minBMFolder             100000
#define maxBMFolder             300000
#define maxEveMarketGroup       350000
#define minDustMarketGroup      350001
#define minFaction              500000
#define maxFaction              999999
#define minNPCCorporation       1000000
#define maxNPCCorporation       1100000
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
#define minUniverseAsteroid     70000000        // deco only - not targettable, no bracket in overview
#define maxUniverseAsteroid     79999999
#define minControlBunker        80000000
#define maxControlBunker        80099999
#define maxNPCItem              89999999
#define minCharacter            90000000        // client NPC def ends here
#define maxCharacter            97999999
#define minPCCorporation        98000000        // player corps start here
#define minAlliance             99000000        // alliances start here
#define minOffice               100000000
#define minTempItemID           110000000
#define minPIStructure          130000000
#define minCustomsOffice        135000000
#define minPlayerItem           140000000
#define maxPlayerItem           300000000
#define minAsteroidItem         450000000
#define minDroneItem            500000000
#define minBookmark             600000000
#define minNPC                  750000000
#define minFleet                950000000
#define maxFleet                959000000
#define minWing                 960000000
#define maxWing                 969000000
#define minSquad                970000000
#define maxSquad                979000000
#define minDungeon              1200000000
#define maxEveItem              2147483647      // max int32

#define maxHangarCapy           9000000000000000

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
#define staOfficeOffset                 6000000
#define EVEMU_TEMP_ENTITY_ID          110000000
#define EVEMU_PLANET_PIN_ID           130000000
#define EVEMU_DRONE_ID                500000000
#define EVEMU_NPC_ID                  750000000
#define EVEMU_FLEET_ID                950000000
#define EVEMU_WING_ID                 960000000
#define EVEMU_SQUAD_ID                970000000
#define EVEMU_MISSILE_ID             1000000000
#define EVEMU_DUNGEON_ID             1200000000
#define EVEMU_MAX_SHORT_ID           2147483647
#define EVEMU_MAX_LONG_ID   9223372036854775807     // max int64.

#define IsTempPinID(pinID) \
 (pinID <= 1000)

#define IsStaticChannel(itemID) \
 ((itemID >= 1) && (itemID <= maxStaticChannel))

#define IsCharType(typeID) \
 ((typeID >= minCharType) && (typeID <= maxCharType))

#define IsCharacter(itemID) \
 ((itemID >= minCharacter) && (itemID <= maxCharacter))

#define IsValidLocation(itemID) \
 (itemID >= minValidLocation)

#define IsCharacterLocation(itemID) \
 (itemID >= minValidCharLocation)

#define IsContainerLocation(itemID) \
 (itemID >= minValidShipLocation)

#define IsFleet(itemID) \
((itemID >= minFleet) && (itemID < maxFleet))

#define IsWing(itemID) \
((itemID >= minWing) && (itemID < maxWing))

#define IsSquad(itemID) \
((itemID >= minSquad) && (itemID < maxSquad))

#define IsCorp(itemID) \
((itemID >= minNPCCorporation) && (itemID <= maxNPCCorporation) \
|| ((itemID >= minPCCorporation) && (itemID < minAlliance)))

#define IsNPCCorp(itemID) \
((itemID >= minNPCCorporation) && (itemID < maxNPCCorporation))

#define IsPlayerCorp(itemID) \
((itemID >= minPCCorporation) && (itemID < minAlliance))

#define IsAlliance(itemID) \
((itemID >= minAlliance) && (itemID < minOffice))

#define IsAgent(itemID) \
((itemID >= minAgent) && (itemID < maxAgent))

#define IsFaction(itemID) \
((itemID >= minFaction) && (itemID < maxFaction))

#define IsOffice(itemID) \
((itemID >= minOffice) && (itemID < minTempItemID))

#define IsAsteroid(itemID) \
((itemID >= minAsteroidItem) && (itemID < EVEMU_DRONE_ID))

#define IsPlayerItem(itemID) \
((itemID > minPlayerItem) && (itemID < maxPlayerItem))

// this covers all static items
#define IsStaticItem(itemID) \
 (itemID <= maxNPCItem)

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

#define IsUniverseAsteroid(itemID) \
((itemID >= minUniverseAsteroid) && (itemID < minControlBunker))

#define IsControlBunker(itemID) \
((itemID >= minControlBunker) and (itemID < 80100000))

#define IsScenarioItem(itemID) \
((itemID > minCharacter) && (itemID < minPlayerItem))

#define IsFakeItem(itemID) \
 (itemID >= minFakeItem)

#define IsValidTarget(itemID) \
 (((itemID >= minStargate) && (itemID <= maxStation)) || (itemID >= minControlBunker))

#define IsTempItem(itemID) \
 ((itemID >= minTempItemID) && (itemID < minPIStructure))

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

#define IsHangarFlag(flag) \
((flag == flagHangar) || ((flag >= flagCorpHangar2) && (flag <= flagCorpHangar7)))

#define IsOfficeFlag(flag) \
((flag >= flagCorpMarket) && (flag <= flagDelivery))

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


#define IsCash(key) \
((key >= 1000) && (key <= 1006))

#define IsAUR(key) \
((key >= 1200) && (key <= 1206))

#define IsDustKey(key) \
(key >= 10000)



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