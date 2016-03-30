 /*
  *
  *
  *
  */


 #ifndef EVE_FLAGS_H
 #define EVE_FLAGS_H


//from invFlags DB table
typedef enum EVEItemFlags {
    flagAutoFit                        = 0,
    flagWallet                        = 1,
    flagFactory                        = 2,
    flagWardrobe                    = 3,
    flagHangar                        = 4,
    flagCargoHold                    = 5,
    flagBriefcase                    = 6,
    flagSkill                        = 7,
    flagReward                        = 8,
    flagConnected                    = 9,    //Character in station connected
    flagDisconnected                = 10,    //Character in station offline

    //ship fittings:
    flagLowSlot0                    = 11,    //Low power slot 1
    flagLowSlot1                    = 12,
    flagLowSlot2                    = 13,
    flagLowSlot3                    = 14,
    flagLowSlot4                    = 15,
    flagLowSlot5                    = 16,
    flagLowSlot6                    = 17,
    flagLowSlot7                    = 18,    //Low power slot 8

    flagMedSlot0                    = 19,    //Medium power slot 1
    flagMedSlot1                    = 20,
    flagMedSlot2                    = 21,
    flagMedSlot3                    = 22,
    flagMedSlot4                    = 23,
    flagMedSlot5                    = 24,
    flagMedSlot6                    = 25,
    flagMedSlot7                    = 26,    //Medium power slot 8

    flagHiSlot0                        = 27,    //High power slot 1
    flagHiSlot1                        = 28,
    flagHiSlot2                        = 29,
    flagHiSlot3                        = 30,
    flagHiSlot4                        = 31,
    flagHiSlot5                        = 32,
    flagHiSlot6                        = 33,
    flagHiSlot7                        = 34,    //High power slot 8
    flagFixedSlot                    = 35,

    //factory stuff:
    flagFactoryBlueprint            = 36,
    flagFactoryMinerals                = 37,
    flagFactoryOutput                = 38,
    flagFactoryActive                = 39,

    flagPromenadeSlot1            = 40,  //Promenade slot 1
    flagPromenadeSlot2            = 41,     //not real sure wtf this is, or what it's used for.
    flagPromenadeSlot3            = 42,
    flagPromenadeSlot4            = 43,
    flagPromenadeSlot5            = 44,
    flagPromenadeSlot6            = 45,
    flagPromenadeSlot7            = 46,
    flagPromenadeSlot8            = 47,
    flagPromenadeSlot9            = 48,
    flagPromenadeSlot10           = 49,
    flagPromenadeSlot11           = 50,
    flagPromenadeSlot12           = 51,
    flagPromenadeSlot13           = 52,
    flagPromenadeSlot14           = 53,
    flagPromenadeSlot15           = 54,
    flagPromenadeSlot16           = 55,  //Promenade slot 16

    flagCapsule                        = 56,    //Capsule item in space
    flagPilot                        = 57,
    flagPassenger                    = 58,
    flagBoardingGate                = 59,
    flagCrew                        = 60,
    flagSkillInTraining                = 61,
    flagCorpMarket                    = 62,    //Corporation Market Deliveries / Returns
    flagLocked                        = 63,    //Locked item, can not be moved unless unlocked
    flagUnlocked                    = 64,

    flagOfficeSlot1                = 70,
    flagOfficeSlot2                = 71,
    flagOfficeSlot3                = 72,
    flagOfficeSlot4                = 73,
    flagOfficeSlot5                = 74,
    flagOfficeSlot6                = 75,
    flagOfficeSlot7                = 76,
    flagOfficeSlot8                = 77,
    flagOfficeSlot9                = 78,
    flagOfficeSlot10               = 79,
    flagOfficeSlot11               = 80,
    flagOfficeSlot12               = 81,
    flagOfficeSlot13               = 82,
    flagOfficeSlot14               = 83,
    flagOfficeSlot15               = 84,
    flagOfficeSlot16               = 85,

    flagBonus                        = 86,    //Char bonus/penalty
    flagDroneBay                    = 87,
    flagBooster                        = 88,
    flagImplant                        = 89,
    flagShipHangar                    = 90,
    flagShipOffline                    = 91,

    flagRigSlot0                    = 92,    //Rig power slot 1
    flagRigSlot1                    = 93,    //Rig power slot 2
    flagRigSlot2                    = 94,    //Rig power slot 3
    flagRigSlot3                    = 95,    //Rig power slot 4
    flagRigSlot4                    = 96,    //Rig power slot 5
    flagRigSlot5                    = 97,    //Rig power slot 6
    flagRigSlot6                    = 98,    //Rig power slot 7
    flagRigSlot7                    = 99,    //Rig power slot 8

    flagFactoryOperation            = 100,

    flagCorpSecurityAccessGroup2    = 116,
    flagCorpSecurityAccessGroup3    = 117,
    flagCorpSecurityAccessGroup4    = 118,
    flagCorpSecurityAccessGroup5    = 119,
    flagCorpSecurityAccessGroup6    = 120,
    flagCorpSecurityAccessGroup7    = 121,

    flagSecondaryStorage            = 122,    //Secondary Storage  (strontium bay on POS)
    flagCaptainsQuarters            = 123,    //Captains Quarters
    flagWisPromenade                = 124,    //Wis Promenade       Walking In Station

    flagSubSystem0                    = 125,    //Sub system slot 0
    flagSubSystem1                    = 126,    //Sub system slot 1
    flagSubSystem2                    = 127,    //Sub system slot 2
    flagSubSystem3                    = 128,    //Sub system slot 3
    flagSubSystem4                    = 129,    //Sub system slot 4
    flagSubSystem5                    = 130,    //Sub system slot 5
    flagSubSystem6                    = 131,    //Sub system slot 6
    flagSubSystem7                    = 132,    //Sub system slot 7

    flagSpecializedFuelBay          = 133,
    flagSpecializedOreHold          = 134,
    flagSpecializedGasHold          = 135,
    flagSpecializedMineralHold      = 136,
    flagSpecializedSalvageHold      = 137,
    flagSpecializedShipHold         = 138,
    flagSpecializedSmallShipHold    = 139,
    flagSpecializedMediumShipHold   = 140,
    flagSpecializedLargeShipHold    = 141,
    flagSpecializedIndustrialShipHold = 142,
    flagSpecializedAmmoHold         = 143,

    flagStructureActive             = 144,
    flagStructureInactive           = 145,

    flagJunkyardReprocessed         = 146,
    flagJunkyardTrashed             = 147,

    flagSpecializedCommandCenterHold = 148,
    flagSpecializedPlanetaryCommoditiesHold = 149,
    flagPlanetSurface               = 150,
    flagSpecializedMaterialBay      = 151,

    flagDustCharacterBackpack       = 152,
    flagDustCharacterBattle         = 153,
    flagQuafeBay                    = 154,
    flagFleetHangar                 = 155,

    flagResearchFacilitySlotFirst    = 200,
    flagResearchFacilitySlotLast    = 255,

    flagMissile                     = 300,

    flagClone                        = 400,

    flagIllegal                     = 9999
} EVEItemFlags;

//for use in the new module manager
typedef enum {
    NaT                  = 0,
    slotTypeSubSystem    = 1,
    slotTypeRig          = 2,
    slotTypeLowPower     = 3,
    slotTypeMedPower     = 4,
    slotTypeHiPower      = 5
} EVEItemSlotType;

//some alternative names for entries above.
static const EVEItemFlags flagSlotFirst = flagLowSlot0;    //duplicate values
static const EVEItemFlags flagSlotLast = flagFixedSlot;
static const EVEItemFlags flagNone = flagAutoFit;

static const EVEItemFlags flagAnywhere = flagAutoFit;
static const uint8 MAX_MODULE_COUNT = flagSlotLast - flagSlotFirst + 1;
static const uint8 MAX_HIGH_SLOT_COUNT = flagHiSlot7 - flagHiSlot0 + 1;
static const uint8 MAX_MEDIUM_SLOT_COUNT = flagMedSlot7 - flagMedSlot0 + 1;
static const uint8 MAX_LOW_SLOT_COUNT = flagLowSlot7 - flagLowSlot0 + 1;
static const uint8 MAX_RIG_COUNT = flagRigSlot7 - flagRigSlot0 + 1;
static const uint8 MAX_ASSEMBLY_COUNT = flagSubSystem7 - flagSubSystem0 + 1;

#define FlagToSlot(flag) \
    (flag - flagSlotFirst)
#define SlotToFlag(slot) \
    ((EVEItemFlags)(flagSlotFirst + slot))

#endif  // EVE_FLAGS_H
