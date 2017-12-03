/*
 *
 *
 *
 */


 #ifndef EVE_ROLES_H
 #define EVE_ROLES_H

enum:uint64_t {
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
    ROLE_LEGIONEER          = 262144L,                  // get ALL corp chat channels (and is usually invis)
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
    ROLE_BATTLESERVER       = 17179869184LL,
    ROLE_TRANSLATIONTESTER  = 34359738368LL,
    ROLE_WIKIEDITOR         = 68719476736LL,
    ROLE_TRANSFER           = 137438953472LL,
    ROLE_GMS                = 274877906944LL,
    ROLE_CL                 = 549755813888LL,
    ROLE_CR                 = 1099511627776LL,
    ROLE_CM                 = 2199023255552LL,
    ROLE_BSDADMIN           = 35184372088832LL,
    ROLE_PROGRAMMER         = 2251799813685248LL,
    ROLE_QA                 = 4503599627370496LL,
    ROLE_GMH                = 9007199254740992LL,
    // the client requires a module named "dna" for many menu items for the "gml" role
    //  we do not have this module, which leads to an error, and an inoperable rclick menu
    //  when ROLE_GML is part of a client's roles
    ROLE_GML                = 18014398509481984LL,
    ROLE_CONTENT            = 36028797018963968LL,
    ROLE_ADMIN              = 72057594037927936LL,
    ROLE_VIPLOGIN           = 144115188075855872LL,
    ROLE_ROLEADMIN          = 288230376151711744LL,
    ROLE_NEWBIE             = 576460752303423488LL,
    ROLE_SERVICE            = 1152921504606846976LL,        // can use station services without being docked.
    ROLE_PLAYER             = 2305843009213693952LL, // 0x02
    ROLE_LOGIN              = 4611686018427387904ULL, // 0x04000000000000000  0b100 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000

    ROLE_ANY                = 18446744073709551615ULL & ~ROLE_IGB,

    ROLE_STD                = ROLE_LOGIN | ROLE_PLAYER | ROLE_IGB,
    ROLE_VIP                = ROLE_STD | ROLE_VIPLOGIN | ROLE_HEALSELF,
    ROLE_TRANSAM            = ROLE_VIP | ROLE_TRANSLATION | ROLE_TRANSLATIONADMIN | ROLE_TRANSLATIONEDITOR,
    ROLE_SLASH              = ROLE_VIP | ROLE_GML | ROLE_LEGIONEER | ROLE_SPAWN | ROLE_HEALOTHERS,
    ROLE_CREATOR            = ROLE_SLASH | ROLE_GMH | ROLE_WORLDMOD | ROLE_CONTENT,
    ROLE_DEV                = ROLE_CREATOR | ROLE_QA | ROLE_PROGRAMMER,
    ROLE_BOSS               = ROLE_DEV | ROLE_ADMIN,
    ROLE_ELEVATEDPLAYER     = ROLE_VIP | ROLE_HEALOTHERS,
    //ROLEMASK_ELEVATEDPLAYER = ROLE_ANY & ~(ROLE_LOGIN | ROLE_PLAYER | ROLE_NEWBIE | ROLE_VIPLOGIN),
    ROLE_VIEW               = ROLE_ADMIN | ROLE_CONTENT | ROLE_GML | ROLE_GMH | ROLE_QA
    /*
     * 23:54:52 W          ROLE_DEV:  7113435622181961728(0x62b8000280c40000)
     * 23:54:52 W          ROLE_STD:  6917529029788565504(0x6000000080000000)
     * 23:54:52 W          ROLE_VIP:  7061644217868615680(0x6200000080400000)
     * 23:54:52 W         ROLE_VIP+:  7061644217877004288(0x6200000080c00000)
     * 23:54:52 W         ROLE_VIEW:  139611588448485376(0x1f0000000000000)
     * 23:54:52 W         ROLE_BOSS:  7185493216219889664(0x63b8000280c40000)
     * 23:54:52 W        ROLE_SLASH:  7061644226467201024(0x6200000280c40000)
     * 23:54:52 W      ROLE_CREATOR:  7106680222740905984(0x62a0000280c40000)
     *
     */
};

enum:uint64_t {
    corpRoleDirector                        = 1,
    corpRolePersonnelManager                = 128,
    corpRoleAccountant                      = 256,
    corpRoleSecurityOfficer                 = 512,
    corpRoleFactoryManager                  = 1024,
    corpRoleStationManager                  = 2048,
    corpRoleAuditor                         = 4096,
    corpRoleHangarCanTake1                  = 8192,
    corpRoleHangarCanTake2                  = 16384,
    corpRoleHangarCanTake3                  = 32768,
    corpRoleHangarCanTake4                  = 65536,
    corpRoleHangarCanTake5                  = 131072,
    corpRoleHangarCanTake6                  = 262144,
    corpRoleHangarCanTake7                  = 524288,
    corpRoleHangarCanQuery1                 = 1048576,
    corpRoleHangarCanQuery2                 = 2097152,
    corpRoleHangarCanQuery3                 = 4194304,
    corpRoleHangarCanQuery4                 = 8388608,
    corpRoleHangarCanQuery5                 = 16777216,
    corpRoleHangarCanQuery6                 = 33554432,
    corpRoleHangarCanQuery7                 = 67108864,
    corpRoleAccountCanTake1                 = 134217728,
    corpRoleAccountCanTake2                 = 268435456,
    corpRoleAccountCanTake3                 = 536870912,
    corpRoleAccountCanTake4                 = 1073741824,
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
    corpRoleMissing                         = 3458764513820540928L, //   0x3000000000000000

    //Some Combos
    corpRoleAllHangar   = corpRoleHangarCanTake1|corpRoleHangarCanTake2|corpRoleHangarCanTake3|corpRoleHangarCanTake4|corpRoleHangarCanTake5|corpRoleHangarCanTake6|corpRoleHangarCanTake7|corpRoleHangarCanQuery1|corpRoleHangarCanQuery2|corpRoleHangarCanQuery3|corpRoleHangarCanQuery4|corpRoleHangarCanQuery5|corpRoleHangarCanQuery6|corpRoleHangarCanQuery7,
    corpRoleAllAccount  = corpRoleJuniorAccountant|corpRoleAccountCanTake1|corpRoleAccountCanTake2|corpRoleAccountCanTake3|corpRoleAccountCanTake4|corpRoleAccountCanTake5|corpRoleAccountCanTake6|corpRoleAccountCanTake7|corpRoleAccountant,
    corpRoleAllContainer= corpRoleContainerCanTake1|corpRoleContainerCanTake2|corpRoleContainerCanTake3|corpRoleContainerCanTake4|corpRoleContainerCanTake5|corpRoleContainerCanTake6|corpRoleContainerCanTake7,
    corpRoleAllOffice   = corpRoleCanRentOffice|corpRoleCanRentFactorySlot|corpRoleCanRentResearchSlot,
    corpRoleAllStarbase = corpRoleStarbaseCaretaker|corpRoleStarbaseConfig|corpRoleInfrastructureTacticalOfficer|corpRoleEquipmentConfig,
    corpRoleAllManager  = corpRolePersonnelManager|corpRoleStationManager|corpRoleFactoryManager|corpRoleChatManager|corpRoleContractManager|corpRoleFittingManager,

    corpRoleAll         = corpRoleAllHangar|corpRoleAllAccount|corpRoleAllContainer|corpRoleAllOffice|corpRoleAllStarbase|corpRoleAllManager|corpRoleAuditor|corpRoleDiplomat,
    corpRoleAdmin       = corpRoleAll|corpRoleTrader|corpRoleSecurityOfficer|corpRoleDirector|corpRoleMissing

    /* 18:05:31 G   Alasiya's EvEMu: Common Corp Roles:
     * 18:37:16 W          Role_All:  1134904941433847168(0xfbffe07fffffd80)
     * 18:37:16 W         Role_Cont:  558551906910208(0x1fc0000000000)
     * 18:37:16 W        Role_Admin:  1152919339943329665(0xffffe07ffffff81)
     * 18:37:16 W       Role_Hangar:  134209536(0x7ffe000)
     * 18:37:16 W      Role_Account:  4503616673022208(0x100003f8000100)
     * 18:37:16 W     Role_Starbase:  441352763482308608(0x620000000000000)
     */
};

#endif  //EVE_ROLES_H