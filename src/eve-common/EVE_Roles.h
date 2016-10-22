/*
 *
 *
 *
 */


 #ifndef EVE_ROLES_H
 #define EVE_ROLES_H

enum:uint64 {
    ROLE_DUST               = 1L,                      // 0x01                0b0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0001
    ROLE_BANNING            = 2L,                      // 0x02                0b0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0010
    ROLE_MARKET             = 4L,                      // 0x04                0b0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0100
    ROLE_MARKETH            = 8L,                      // 0x08                0b0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 1000
    ROLE_CSMADMIN           = 16L,                     // 0x010
    ROLE_CSMDELEGATE        = 32L,                     // 0x020
    ROLE_EXPOPLAYER         = 64L,                     // 0x040
    ROLE_PETITIONEE         = 256L,                    // 0x0100
    ROLE_CENTURION          = 2048L,                   // 0x0800
    // the client requires a module named "dna" for many menu items for the "worldmod" role
    //  we do not have this module, which leads to an error, and an inoperable rclick menu
    //  when ROLE_WORLDMOD is part of a client's roles
    ROLE_WORLDMOD           = 4096L,                   // 0x01000
    ROLE_DBA                = 16384L,
    ROLE_REMOTESERVICE      = 131072L,
    ROLE_LEGIONEER          = 262144L,
    ROLE_TRANSLATION        = 524288L,
    ROLE_CHTINVISIBLE       = 1048576L,
    ROLE_CHTADMINISTRATOR   = 2097152L,
    ROLE_HEALSELF           = 4194304L,
    ROLE_HEALOTHERS         = 8388608L,
    ROLE_NEWSREPORTER       = 16777216L,
    ROLE_TRANSLATIONADMIN   = 134217728L,
    ROLE_ACCOUNTMANAGEMENT  = 536870912L,
    ROLE_SPAWN              = 8589934592L,             // 0x0200000000
    ROLE_IGB                = 2147483648L,
    ROLE_TRANSLATIONEDITOR  = 4294967296L,
    ROLE_BATTLESERVER       = 17179869184L,
    ROLE_TRANSLATIONTESTER  = 34359738368L,
    ROLE_WIKIEDITOR         = 68719476736L,
    ROLE_TRANSFER           = 137438953472L,
    ROLE_GMS                = 274877906944L,
    ROLE_CL                 = 549755813888L,
    ROLE_CR                 = 1099511627776L,
    ROLE_CM                 = 2199023255552L,
    ROLE_BSDADMIN           = 35184372088832L,
    ROLE_PROGRAMMER         = 2251799813685248L,
    ROLE_QA                 = 4503599627370496L,
    ROLE_GMH                = 9007199254740992L,
    // the client requires a module named "dna" for many menu items for the "gml" role
    //  we do not have this module, which leads to an error, and an inoperable rclick menu
    //  when ROLE_GML is part of a client's roles
    ROLE_GML                = 18014398509481984L,
    ROLE_CONTENT            = 36028797018963968L,
    ROLE_ADMIN              = 72057594037927936L,
    ROLE_VIPLOGIN           = 144115188075855872L,
    ROLE_ROLEADMIN          = 288230376151711744L,
    ROLE_NEWBIE             = 576460752303423488L,
    ROLE_SERVICE            = 1152921504606846976L,        // can use station services without being docked.
    ROLE_PLAYER             = 2305843009213693952L, // 0x02
    ROLE_LOGIN              = 4611686018427387904L, // 0x04000000000000000  0b100 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000

    ROLE_ANY                = 18446744073709551615UL & ~ROLE_IGB,

    ROLE_STD                = ROLE_LOGIN | ROLE_PLAYER | ROLE_IGB,
    ROLE_VIP                = ROLE_STD | ROLE_VIPLOGIN | ROLE_HEALSELF,
    ROLE_TRANSAM            = ROLE_VIP | ROLE_TRANSLATION | ROLE_TRANSLATIONADMIN | ROLE_TRANSLATIONEDITOR,
    ROLE_SLASH              = ROLE_VIP /*| ROLE_GML*/ | ROLE_LEGIONEER | ROLE_SPAWN | ROLE_HEALOTHERS,
    ROLE_CREATOR            = ROLE_SLASH | ROLE_GMH /*| ROLE_WORLDMOD*/ | ROLE_CONTENT,
    ROLE_DEV                = ROLE_CREATOR | ROLE_QA | ROLE_PROGRAMMER,
    ROLE_BOSS               = ROLE_DEV | ROLE_ADMIN,
    ROLEMASK_ELEVATEDPLAYER = ROLE_VIP | ROLE_HEALOTHERS,
    ROLEMASK_VIEW           = ROLE_ADMIN | ROLE_CONTENT | ROLE_GML | ROLE_GMH | ROLE_QA
};

enum {
    corpRoleLocationTypeHQ = 1,
    corpRoleLocationTypeBase = 2,
    corpRoleLocationTypeOther = 3
};

typedef enum:uint64 {
    corpRoleDirector                        = 1L,
    corpRolePersonnelManager                = 128L,
    corpRoleAccountant                      = 256L,
    corpRoleSecurityOfficer                 = 512L,
    corpRoleFactoryManager                  = 1024L,
    corpRoleStationManager                  = 2048L,
    corpRoleAuditor                         = 4096L,
    corpRoleHangarCanTake1                  = 8192L,
    corpRoleHangarCanTake2                  = 16384L,
    corpRoleHangarCanTake3                  = 32768L,
    corpRoleHangarCanTake4                  = 65536L,
    corpRoleHangarCanTake5                  = 131072L,
    corpRoleHangarCanTake6                  = 262144L,
    corpRoleHangarCanTake7                  = 524288L,
    corpRoleHangarCanQuery1                 = 1048576L,
    corpRoleHangarCanQuery2                 = 2097152L,
    corpRoleHangarCanQuery3                 = 4194304L,
    corpRoleHangarCanQuery4                 = 8388608L,
    corpRoleHangarCanQuery5                 = 16777216L,
    corpRoleHangarCanQuery6                 = 33554432L,
    corpRoleHangarCanQuery7                 = 67108864L,
    corpRoleAccountCanTake1                 = 134217728L,
    corpRoleAccountCanTake2                 = 268435456L,
    corpRoleAccountCanTake3                 = 536870912L,
    corpRoleAccountCanTake4                 = 1073741824L,
    corpRoleAccountCanTake5                 = 2147483648L,
    corpRoleAccountCanTake6                 = 4294967296L,
    corpRoleAccountCanTake7                 = 8589934592L,
    corpRoleDiplomat                        = 17179869184L,
    corpRoleEquipmentConfig                 = 2199023255552L,
    corpRoleContainerCanTake1               = 4398046511104L,
    corpRoleContainerCanTake2               = 8796093022208L,
    corpRoleContainerCanTake3               = 17592186044416L,
    corpRoleContainerCanTake4               = 35184372088832L,
    corpRoleContainerCanTake5               = 70368744177664L,
    corpRoleContainerCanTake6               = 140737488355328L,
    corpRoleContainerCanTake7               = 281474976710656L,
    corpRoleCanRentOffice                   = 562949953421312L,
    corpRoleCanRentFactorySlot              = 1125899906842624L,
    corpRoleCanRentResearchSlot             = 2251799813685248L,
    corpRoleJuniorAccountant                = 4503599627370496L,
    corpRoleStarbaseConfig                  = 9007199254740992L,
    corpRoleTrader                          = 18014398509481984L,
    corpRoleChatManager                     = 36028797018963968L,
    corpRoleContractManager                 = 72057594037927936L,
    corpRoleInfrastructureTacticalOfficer   = 144115188075855872L,
    corpRoleStarbaseCaretaker               = 288230376151711744L,
    corpRoleFittingManager                  = 576460752303423488L,
    corpRoleAll                             = 1152921504606846975L,

    //Some Combos
    corpRoleAllHangar   = corpRoleHangarCanTake1|corpRoleHangarCanTake2|corpRoleHangarCanTake3|corpRoleHangarCanTake4|corpRoleHangarCanTake5|corpRoleHangarCanTake6|corpRoleHangarCanTake7|corpRoleHangarCanQuery1|corpRoleHangarCanQuery2|corpRoleHangarCanQuery3|corpRoleHangarCanQuery4|corpRoleHangarCanQuery5|corpRoleHangarCanQuery6|corpRoleHangarCanQuery7,
    corpRoleAllAccount  = corpRoleJuniorAccountant|corpRoleAccountCanTake1|corpRoleAccountCanTake2|corpRoleAccountCanTake3|corpRoleAccountCanTake4|corpRoleAccountCanTake5|corpRoleAccountCanTake6|corpRoleAccountCanTake7,
    corpRoleAllContainer= corpRoleContainerCanTake1|corpRoleContainerCanTake2|corpRoleContainerCanTake3|corpRoleContainerCanTake4|corpRoleContainerCanTake5|corpRoleContainerCanTake6|corpRoleContainerCanTake7,
    corpRoleAllOffice   = corpRoleCanRentOffice|corpRoleCanRentFactorySlot|corpRoleCanRentResearchSlot,
    corpRoleAllStarbase = corpRoleStarbaseCaretaker|corpRoleStarbaseConfig
} CorpRoleFlags;

//  -allan 5Aug14
typedef enum {
    fleetJobNone        = 0,
    fleetJobScout       = 1,
    fleetJobCreator     = 2
} FleetJobs;

//  -allan 5Aug14
typedef enum {
    fleetRoleLeader     = 1,
    fleetRoleWingCmdr   = 2,
    fleetRoleSquadCmdr  = 3,
    fleetRoleMember     = 4
} FleetRoles;

//  -allan 5Aug14
typedef enum {
    fleetBoosterNone    = 0,
    fleetBoosterFleet   = 1,
    fleetBoosterWing    = 2,
    fleetBoosterSquad   = 3
} FleetBoosters;

#endif  //EVE_ROLES_H