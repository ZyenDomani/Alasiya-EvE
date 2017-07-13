/*  EVE_Defines.h
 *   this file defines hard-coded values for game item classes
 *   we also define C-type macros for ease of readibilty in various item conditionals
 *
 */

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
#define maxDustMarketGroup      999999
#define minBMFolder             100000
#define maxBMFolder             300000
#define minFaction              500000
#define maxFaction              599999
#define minNPCCorporation       1000000
#define maxNPCCorporation       1000999
#define maxCorporation          1999999
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
#define maxStation              69999999
#define minUniverseAsteroid     70000000        // deco only
#define maxUniverseAsteroid     79999999        // deco only
#define minFleet                80000000
#define maxFleet                80009999
#define minAlliance             99000000
#define maxAlliance             99900000
#define minPlayerItem           140000000
#define maxEveItem              2147483647      // max short int32
/*
DSTLOCALBALLS = 0x0C0000000h  (3,221,225,472 decimal)      unknown where this is from
missile itemID's  dec = 9,000,000,000,000,000,000    hex = 0x7CE66C50E2840000h        from packet sniff
*/

/*
maxInt = 2147483647
maxBigint = 9223372036854775807L
minPlayerOwner = 90000000
maxPlayerOwner = 2147483647
minFakeItem = 9000000000000000000L
minFakeClientItem = 17000000000000000000L
*/

//  allan's static defines to ease code checks
//  **update these**
#define EVEMU_OUTPOST_ID               61000000
#define EVEMU_SCENERIO_ID              90000000
#define EVEMU_MAXIMUM_STATIC_ID        99900000
#define EVEMU_ASTEROID_ID             100000000
#define EVEMU_TEMP_ENTITY_ID          110000000
#define EVEMU_PLANET_PIN_ID           130000000
#define EVEMU_MINIMUM_DYNAMIC_ID      140000000
#define EVEMU_DRONE_ID                500000000
#define EVEMU_NPC_ID                  750000000
#define EVEMU_MISSILE_ID             1000000000
#define EVEMU_DUNGEON_ID             1200000000
#define EVEMU_MAX_SHORT_ID           2147483647
#define EVEMU_MAX_LONG_ID   9223372036854775807     //this is max for a SIGNED int64.

#define IsTempPinID(pinID) \
 (pinID < 1000)

#define IsTradeCont(itemID) \
 ((itemID > 1000) && (itemID < 2000))

#define IsCharType(typeID) \
 ((typeID >= minCharType) && (typeID <= maxCharType))

#define IsCharacterLocation(itemID) \
 (itemID >= minValidCharLocation)

#define IsContainerLocation(itemID) \
 (itemID >= minValidShipLocation)

#define IsFleet(itemID) \
((itemID >= minFleet) && (itemID <= maxFleet))

#define IsCorp(itemID) \
((itemID >= minNPCCorporation) && (itemID <= maxCorporation))

#define IsNPCCorp(itemID) \
((itemID >= minNPCCorporation) && (itemID < maxNPCCorporation))

#define IsPlayerCorp(itemID) \
((itemID >= maxNPCCorporation) && (itemID < maxCorporation))

#define IsAlliance(itemID) \
((itemID >= minAlliance) && (itemID < maxAlliance))

#define IsAgent(itemID) \
((itemID >= minAgent) && (itemID < maxAgent))

#define IsFaction(itemID) \
((itemID >= minFaction) && (itemID < maxFaction))

// this covers ALL static celestial-type items
#define IsStaticMapItem(itemID) \
((itemID >= minRegion) && (itemID < maxUniverseAsteroid))

#define IsRegion(itemID) \
((itemID >= 10000000) && (itemID < 20000000))

#define IsConstellation(itemID) \
((itemID >= 20000000) && (itemID < 30000000))

#define IsSolarSystem(itemID) \
((itemID >= minSolarSystem) && (itemID < maxSolarSystem))

#define IsKSpace(itemID) \
((itemID >= minSolarSystem) && (itemID < minWHSolarSystem))

#define IsWSpace(itemID) \
((itemID >= minWHSolarSystem) && (itemID < maxWHSolarSystem))

#define IsCelestial(itemID) \
((itemID >= 40000000) && (itemID < 50000000))

#define IsStargate(itemID) \
((itemID >= 50000000) && (itemID < 60000000))

#define IsStation(itemID) \
((itemID >= minStation) && (itemID < maxStation))

#define IsNPCStation(itemID) \
((itemID >= minStation) && (itemID <= maxNPCStation))

//#define IsOutpost(itemID)
// ((itemID > maxNPCStation) && (itemID < maxStation))

#define IsTrading(itemID) \
((itemID >= 64000000) && (itemID < 66000000))

#define IsOfficeFolder(itemID) \
((itemID >= 66000000) && (itemID < 68000000))

#define IsFactoryFolder(itemID) \
((itemID >= 68000000) && (itemID < 70000000))

#define IsUniverseAsteroid(itemID) \
((itemID >= 70000000) && (itemID < 80000000))

#define IsScenarioItem(itemID) \
((itemID >= 90000000) && (itemID < EVEMU_MINIMUM_DYNAMIC_ID))

#define IsNotStaticItem(itemID) \
(itemID >= EVEMU_MINIMUM_DYNAMIC_ID)

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
((flag >= flagSubSystem0) && (flag<=flagSubSystem7))


/*
def IsSystem(ownerID):
    return ownerID <= 10000


def IsNPC(ownerID):
    return ownerID < 90000000 and ownerID > 10000


def IsNPCCorporation(ownerID):
    return ownerID < 2000000 and ownerID >= 1000000


def IsNPCCharacter(ownerID):
    return ownerID < 4000000 and ownerID >= 3000000


def IsSystemOrNPC(ownerID):
    return ownerID < 90000000


def IsFaction(ownerID):
    if ownerID >= 500000 and ownerID < 1000000:
        return 1
    else:
        return 0


def IsCorporation(ownerID):
    if ownerID >= 1000000 and ownerID < 2000000:
        return 1
    if ownerID < 98000000 or ownerID > 2147483647:
        return 0
    if ownerID < 99000000:
        return 1
    if ownerID < 100000000:
        return 0
    if boot.role == 'server' and sm.GetService('standing2').IsKnownToBeAPlayerCorp(ownerID):
        return 1
    try:
        return cfg.eveowners.Get(ownerID).IsCorporation()
    except KeyError:
        return 0


def IsCharacter(ownerID):
    if ownerID >= 3000000 and ownerID < 4000000:
        return 1
    if ownerID < 90000000 or ownerID > 2147483647:
        return 0
    if ownerID < 98000000:
        return 1
    if ownerID < 100000000:
        return 0
    if boot.role == 'server' and sm.GetService('standing2').IsKnownToBeAPlayerCorp(ownerID):
        return 0
    try:
        return cfg.eveowners.Get(ownerID).IsCharacter()
    except KeyError:
        return 0


def IsPlayerAvatar(itemID):
    return IsCharacter(itemID)


def IsOwner(ownerID, fetch = 1):
    if ownerID >= 500000 and ownerID < 1000000 or ownerID >= 1000000 and ownerID < 2000000 or ownerID >= 3000000 and ownerID < 4000000:
        return 1
    if IsNPC(ownerID):
        return 0
    if ownerID < 90000000 or ownerID > 2147483647:
        return 0
    if ownerID < 100000000:
        return 1
    if fetch:
        try:
            oi = cfg.eveowners.Get(ownerID)
        except KeyError:
            return 0

        if oi.groupID in (const.groupCharacter, const.groupCorporation):
            return 1
        else:
            return 0
    else:
        return 0


def IsAlliance(ownerID):
    if ownerID < 99000000 or ownerID > 2147483647:
        return 0
    if ownerID < 100000000:
        return 1
    if boot.role == 'server' and sm.GetService('standing2').IsKnownToBeAPlayerCorp(ownerID):
        return 0
    try:
        return cfg.eveowners.Get(ownerID).IsAlliance()
    except KeyError:
        return 0


def IsRegion(itemID):
    return itemID >= 10000000 and itemID < 20000000


def IsConstellation(itemID):
    return itemID >= 20000000 and itemID < 30000000


def IsSolarSystem(itemID):
    return itemID >= 30000000 and itemID < 40000000


def IsCelestial(itemID):
    return itemID >= 40000000 and itemID < 50000000


def IsWormholeSystem(itemID):
    return itemID >= const.mapWormholeSystemMin and itemID < const.mapWormholeSystemMax


def IsWormholeConstellation(constellationID):
    return constellationID >= const.mapWormholeConstellationMin and constellationID < const.mapWormholeConstellationMax


def IsWormholeRegion(regionID):
    return regionID >= const.mapWormholeRegionMin and regionID < const.mapWormholeRegionMax


def IsCelestial(itemID):
    return itemID >= const.minUniverseCelestial and itemID <= const.maxUniverseCelestial


def IsStargate(itemID):
    return itemID >= 50000000 and itemID < 60000000


def IsStation(itemID):
    return itemID >= 60000000 and itemID < 64000000


def IsWorldSpace(itemID):
    return itemID >= const.mapWorldSpaceMin and itemID < const.mapWorldSpaceMax


def IsOutpost(itemID):
    return itemID >= 61000000 and itemID < 64000000


def IsTrading(itemID):
    return itemID >= 64000000 and itemID < 66000000


def IsOfficeFolder(itemID):
    return itemID >= 66000000 and itemID < 68000000


def IsFactoryFolder(itemID):
    return itemID >= 68000000 and itemID < 70000000


def IsUniverseAsteroid(itemID):
    return itemID >= 70000000 and itemID < 80000000


def IsJunkLocation(locationID):
    if locationID >= 2000:
        return 0
    elif locationID in (6, 8, 10, 23, 25):
        return 1
    elif locationID > 1000 and locationID < 2000:
        return 1
    else:
        return 0


def IsControlBunker(itemID):
    return itemID >= 80000000 and itemID < 80100000


def IsPlayerItem(itemID):
    return itemID >= const.minPlayerItem and itemID < const.minFakeItem


def IsFakeItem(itemID):
    return itemID > const.minFakeItem


def IsNewbieSystem(itemID):
    default = [30002547,
     30001392,
     30002715,
     30003489,
     30005305,
     30004971,
     30001672,
     30002505,
     30000141,
     30003410,
     30005042,
     30001407]
    optional = [30001722,
     30002518,
     30003388,
     30003524,
     30005015,
     30010141,
     30011392,
     30011407,
     30011672,
     30012505,
     30012547,
     30012715,
     30013410,
     30013489,
     30014971,
     30015042,
     30015305,
     30020141,
     30021392,
     30021407,
     30021672,
     30022505,
     30022547,
     30022715,
     30023410,
     30023489,
     30024971,
     30025042,
     30025305,
     30030141,
     30031392,
     30031407,
     30031672,
     30032505,
     30032547,
     30032715,
     30033410,
     30033489,
     30034971,
     30035042,
     30035305,
     30040141,
     30041392,
     30041407,
     30041672,
     30042505,
     30042547,
     30042715,
     30043410,
     30043489,
     30044971,
     30045042,
     30045305]
    if boot.region == 'optic':
        return itemID in default + optional
    return itemID in default


def IsStructure(categoryID):
    return categoryID in (const.categorySovereigntyStructure, const.categoryStructure)


def IsOrbital(categoryID):
    return categoryID == const.categoryOrbital


def IsPreviewable(typeID):
    type = cfg.invtypes.GetIfExists(typeID)
    if type is None:
        return False
    groupID = type.groupID
    categoryID = type.categoryID
    return categoryID in const.previewCategories or groupID in const.previewGroups


def IsPlaceable(typeID):
    type = cfg.invtypes.GetIfExists(typeID)
    if type is None:
        return False
    return const.categoryPlaceables == type.categoryID


def IsEveUser(userID):
    if userID < const.minDustUser:
        return True
    return False


def IsDustUser(userID):
    if userID > const.minDustUser:
        return True
    return False


def IsDustCharacter(characterID):
    if characterID > const.minDustCharacter and characterID < const.maxDustCharacter:
        return True
    return False
*/

#endif  //EVE_DEFINES_H