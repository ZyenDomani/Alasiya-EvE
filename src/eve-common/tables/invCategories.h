#ifndef EVEDB_invCategories_H__
#define EVEDB_invCategories_H__

namespace EVEDB {
    namespace invCategories {
        typedef enum {
            _System = 0,
            Character = 1,
            Celestial = 2,
            Station = 3,
            Material = 4,
            Accessories = 5,
            Ship = 6,
            Module = 7,
            Charge = 8,
            Blueprint = 9,
            Trading = 10,
            Entity = 11,
            Bonus = 14,
            Skill = 16,
            Commodity = 17,
            Drone = 18,
            Implant = 20,
            Deployable = 22,
            Structure = 23,
            Reaction = 24,
            Asteroid = 25,
            WorldSpace = 26,
            Abstract = 29,
            Apparel = 30,
            Subsystem = 32,
            AncientRelics = 34,
            Decryptors = 35,
            StructureUpgrade = 39,
            SovereigntyStructure = 40,
            PlanetaryInteraction = 41,
            PlanetaryResources = 42,
            PlanetaryCommodities = 43,
            Orbitals = 46,
            Placeables = 49,
            Effects = 53,
            Lights = 54,
            Cells = 59
        } invCategories;
    }
}

#endif


/** @note categories from client defs...
 * categoryAbstract = 29
 * categoryAccessories = 5
 * categoryAncientRelic = 34
 * categoryApparel = 30
 * categoryAsteroid = 25
 * categoryWorldSpace = 26
 * categoryBlueprint = 9
 * categoryBonus = 14
 * categoryCatma = 350001
 * categoryCelestial = 2
 * categoryCharge = 8
 * categoryCommodity = 17
 * categoryDecryptors = 35
 * categoryDeployable = 22
 * categoryDrone = 18
 * categoryEntity = 11
 * categoryImplant = 20
 * categoryMaterial = 4
 * categoryModule = 7
 * categoryOrbital = 46
 * categoryOwner = 1
 * categoryPlaceables = 49
 * categoryPlanetaryCommodities = 43
 * categoryPlanetaryInteraction = 41
 * categoryPlanetaryResources = 42
 * categoryReaction = 24
 * categoryShip = 6
 * categorySkill = 16
 * categorySovereigntyStructure = 40
 * categoryStation = 3
 * categoryStructure = 23
 * categoryStructureUpgrade = 39
 * categorySubSystem = 32
 * categorySystem = 0
 * categoryTrading = 10
 */