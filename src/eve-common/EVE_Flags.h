 /*
  *
  *
  *
  */


 #ifndef EVE_FLAGS_H
 #define EVE_FLAGS_H


//from invFlags DB table
enum EVEItemFlags {
    flagAutoFit                   = 0,
    flagWallet                    = 1,
    flagFactory                   = 2,
    flagWardrobe                  = 3,
    flagHangar                    = 4,
    flagCargoHold                 = 5,
    flagBriefcase                 = 6,
    flagSkill                     = 7,
    flagReward                    = 8,
    flagConnected                 = 9,    //Character in station connected
    flagDisconnected              = 10,    //Character in station offline

    //ship fittings:
    flagLowSlot0                  = 11,    //Low power slot 1
    flagLowSlot1                  = 12,
    flagLowSlot2                  = 13,
    flagLowSlot3                  = 14,
    flagLowSlot4                  = 15,
    flagLowSlot5                  = 16,
    flagLowSlot6                  = 17,
    flagLowSlot7                  = 18,    //Low power slot 8

    flagMedSlot0                  = 19,    //Medium power slot 1
    flagMedSlot1                  = 20,
    flagMedSlot2                  = 21,
    flagMedSlot3                  = 22,
    flagMedSlot4                  = 23,
    flagMedSlot5                  = 24,
    flagMedSlot6                  = 25,
    flagMedSlot7                  = 26,    //Medium power slot 8

    flagHiSlot0                   = 27,    //High power slot 1
    flagHiSlot1                   = 28,
    flagHiSlot2                   = 29,
    flagHiSlot3                   = 30,
    flagHiSlot4                   = 31,
    flagHiSlot5                   = 32,
    flagHiSlot6                   = 33,
    flagHiSlot7                   = 34,    //High power slot 8
    flagFixedSlot                 = 35,

    //factory stuff:
    flagFactoryBlueprint          = 36,
    flagFactoryMinerals           = 37,
    flagFactoryOutput             = 38,
    flagFactoryActive             = 39,

   //not real sure wtf this is, or what it's used for.  have not found refereces to any of these
    flagPromenadeSlot1            = 40,  //Promenade slot 1
    flagPromenadeSlot2            = 41,
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

    flagCapsule                   = 56,    //Capsule item in space
    flagPilot                     = 57,
    flagPassenger                 = 58,
    flagBoardingGate              = 59,
    flagCrew                      = 60,
    flagSkillInTraining           = 61,

    // these are flags for items in stations' corp offices
    flagCorpMarket                = 62,    //Corporation Market Deliveries / Returns  this item will be in location(officeID)
    flagLocked                    = 63,    //Locked item, can not be moved unless unlocked
    flagUnlocked                  = 64,
    flagOffice                    = 71,    // offices
    flagImpounded                 = 72,    // impounded or junk
    flagProperty                  = 74,    // property
    flagDelivery                  = 75,    // deliveries   cannot find where this is used.  items set to this flag do not show when loaded

    /*  they were OfficeSlot*, but i dont know where that data came from, as i cannot find any refreences to them
    flagUnknown1                   = 70,
    flagUnknown4                = 73,
    flagUnknown7                = 76,
    flagUnknown8                = 77,
    flagUnknown9                = 78,
    flagUnknown10               = 79,
    flagUnknown11               = 80,
    flagUnknown12               = 81,
    flagUnknown13               = 82,
    flagUnknown14               = 83,
    flagUnknown15               = 84,
    flagUnknown16               = 85,
    */
    flagBonus                      = 86,    //Char bonus/penalty
    flagDroneBay                   = 87,
    flagBooster                    = 88,
    flagImplant                    = 89,
    flagShipHangar                 = 90,
    flagShipOffline                = 91,

    flagRigSlot0                    = 92,    //Rig power slot 1
    flagRigSlot1                    = 93,    //Rig power slot 2
    flagRigSlot2                    = 94,    //Rig power slot 3
    flagRigSlot3                    = 95,    //Rig power slot 4
    flagRigSlot4                    = 96,    //Rig power slot 5
    flagRigSlot5                    = 97,    //Rig power slot 6
    flagRigSlot6                    = 98,    //Rig power slot 7
    flagRigSlot7                    = 99,    //Rig power slot 8

    flagFactoryOperation            = 100,   // dunno what this is for

    //  these are flags for items in corp hangars (station, container, ship), by divisionID
    flagCorpHangar2    = 116,   // formerly corpSAG* or Security Access Group.
    flagCorpHangar3    = 117,
    flagCorpHangar4    = 118,
    flagCorpHangar5    = 119,
    flagCorpHangar6    = 120,
    flagCorpHangar7    = 121,

    flagSecondaryStorage            = 122,    //Secondary Storage  (strontium bay on POS)
    flagCaptainsQuarters            = 123,    //Captains Quarters
    flagWisPromenade                = 124,    //Wis Promenade       Walking In Station
    // flagWorldSpace = 124

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

    // pi containers
    flagSpecializedCommandCenterHold = 148,
    flagSpecializedPlanetaryCommoditiesHold = 149,
    flagPlanetSurface               = 150,
    flagSpecializedMaterialBay      = 151,        // customs office hold

    flagDustCharacterBackpack       = 152,
    flagDustCharacterBattle         = 153,
    flagQuafeBay                    = 154,
    flagFleetHangar                 = 155,

    flagResearchFacilitySlotFirst   = 200,
    flagResearchFacilitySlotLast    = 255,

    flagMissile                     = 300,

    flagClone                       = 400,

    flagIllegal                     = 999

    /*  actual flags defined in client.....
     flagAutoFit = 0
     flagBonus = 86
     flagBooster = 88
     flagBriefcase = 6
     flagCapsule = 56
     flagCargo = 5
     flagCorpMarket = 62
     flagCorpSAG2 = 116
     flagCorpSAG3 = 117
     flagCorpSAG4 = 118
     flagCorpSAG5 = 119
     flagCorpSAG6 = 120
     flagCorpSAG7 = 121
     flagDroneBay = 87
     flagDustBackpack = 152
     flagDustBattle = 153
     flagCorpSAGs = (flagCorpSAG2,
     flagCorpSAG3,
     flagCorpSAG4,
     flagCorpSAG5,
     flagCorpSAG6,
     flagCorpSAG7)
     flagFactoryOperation = 100
     flagFixedSlot = 35
     flagHangar = 4
     flagHangarAll = 1000
     flagHiSlot0 = 27
     flagHiSlot1 = 28
     flagHiSlot2 = 29
     flagHiSlot3 = 30
     flagHiSlot4 = 31
     flagHiSlot5 = 32
     flagHiSlot6 = 33
     flagHiSlot7 = 34
     flagImplant = 89
     flagLoSlot0 = 11
     flagLoSlot7 = 18
     flagLocked = 63
     flagMedSlot0 = 19
     flagMedSlot7 = 26
     flagNone = 0
     flagPilot = 57
     flagPlanetSurface = 150
     flagQuafeBay = 154
     flagReward = 8
     flagRigSlot0 = 92
     flagRigSlot1 = 93
     flagRigSlot2 = 94
     flagRigSlot3 = 95
     flagRigSlot4 = 96
     flagRigSlot5 = 97
     flagRigSlot6 = 98
     flagRigSlot7 = 99
     flagSecondaryStorage = 122
     flagShipHangar = 90
     flagShipOffline = 91
     flagSkill = 7
     flagSkillInTraining = 61
     flagSpecializedFuelBay = 133
     flagSpecializedOreHold = 134
     flagSpecializedGasHold = 135
     flagSpecializedMineralHold = 136
     flagSpecializedSalvageHold = 137
     flagSpecializedShipHold = 138
     flagSpecializedSmallShipHold = 139
     flagSpecializedMediumShipHold = 140
     flagSpecializedLargeShipHold = 141
     flagSpecializedIndustrialShipHold = 142
     flagSpecializedAmmoHold = 143
     flagSpecializedCommandCenterHold = 148
     flagSpecializedPlanetaryCommoditiesHold = 149
     flagSpecializedMaterialBay = 151
     flagSlotFirst = 11
     flagSlotLast = 35
     flagStructureActive = 144
     flagStructureInactive = 145
     flagWorldSpace = 124
     flagSubSystemSlot0 = 125
     flagSubSystemSlot1 = 126
     flagSubSystemSlot2 = 127
     flagSubSystemSlot3 = 128
     flagSubSystemSlot4 = 129
     flagSubSystemSlot5 = 130
     flagSubSystemSlot6 = 131
     flagSubSystemSlot7 = 132
     flagUnlocked = 64
     flagWallet = 1
     flagJunkyardReprocessed = 146
     flagJunkyardTrashed = 147
     flagWardrobe = 3
     */
};

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

#endif  // EVE_FLAGS_H
