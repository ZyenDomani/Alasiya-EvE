/*
 *
 *
 *
 */


#ifndef EVE_ROLES_H
#define EVE_ROLES_H

enum:int64_t {
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
    ROLE_LOGIN              = 4611686018427387904LL, // 0x04000000000000000  0b100 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000

    //ROLE_ANY                = 18446744073709551615LL & ~ROLE_IGB,     max uint64 invalid.  unsigned types removed from this version.  -allan 10Dec17

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

namespace Corp {
    namespace Role {
        enum:int64_t {
            Member                          = 0,
            Director                        = 1,
            PersonnelManager                = 128,
            Accountant                      = 256,
            SecurityOfficer                 = 512,
            FactoryManager                  = 1024,
            StationManager                  = 2048,
            Auditor                         = 4096,
            HangarCanTake1                  = 8192,
            HangarCanTake2                  = 16384,
            HangarCanTake3                  = 32768,
            HangarCanTake4                  = 65536,
            HangarCanTake5                  = 131072,
            HangarCanTake6                  = 262144,
            HangarCanTake7                  = 524288,
            HangarCanQuery1                 = 1048576,
            HangarCanQuery2                 = 2097152,
            HangarCanQuery3                 = 4194304,
            HangarCanQuery4                 = 8388608,
            HangarCanQuery5                 = 16777216,
            HangarCanQuery6                 = 33554432,
            HangarCanQuery7                 = 67108864,
            AccountCanTake1                 = 134217728,
            AccountCanTake2                 = 268435456,
            AccountCanTake3                 = 536870912,
            AccountCanTake4                 = 1073741824,
            AccountCanTake5                 = 2147483648L,
            AccountCanTake6                 = 4294967296L,
            AccountCanTake7                 = 8589934592L,
            Diplomat                        = 17179869184L,
            EquipmentConfig                 = 2199023255552L,
            ContainerCanTake1               = 4398046511104L,
            ContainerCanTake2               = 8796093022208L,
            ContainerCanTake3               = 17592186044416L,
            ContainerCanTake4               = 35184372088832L,
            ContainerCanTake5               = 70368744177664L,
            ContainerCanTake6               = 140737488355328L,
            ContainerCanTake7               = 281474976710656L,
            CanRentOffice                   = 562949953421312L,
            CanRentFactorySlot              = 1125899906842624L,
            CanRentResearchSlot             = 2251799813685248L,
            JuniorAccountant                = 4503599627370496L,
            StarbaseConfig                  = 9007199254740992L,
            Trader                          = 18014398509481984L,
            ChatManager                     = 36028797018963968L,
            ContractManager                 = 72057594037927936L,
            InfrastructureTacticalOfficer   = 144115188075855872L,
            StarbaseCaretaker               = 288230376151711744L,
            FittingManager                  = 576460752303423488L,
            Missing                         = 3458764513820540928L, //   0x3000000000000000     <-- seen in logs, but not defined in client  ** not used **

            // corpRoles data
            AllOffice   = CanRentOffice|CanRentFactorySlot|CanRentResearchSlot,
            AllStarbase = StarbaseCaretaker|StarbaseConfig|InfrastructureTacticalOfficer|EquipmentConfig,
            AllManager  = PersonnelManager|StationManager|FactoryManager|ChatManager|ContractManager|FittingManager,

            // rolesAt* data
            AllHangarTake   = HangarCanTake1|HangarCanTake2|HangarCanTake3|HangarCanTake4|HangarCanTake5|HangarCanTake6|HangarCanTake7,
            AllHangarQuery  = HangarCanQuery1|HangarCanQuery2|HangarCanQuery3|HangarCanQuery4|HangarCanQuery5|HangarCanQuery6|HangarCanQuery7,
            AllAccountTake  = AccountCanTake1|AccountCanTake2|AccountCanTake3|AccountCanTake4|AccountCanTake5|AccountCanTake6|AccountCanTake7,
            // this means the player can take containers out of the respective inventory
            AllContainerTake = ContainerCanTake1|ContainerCanTake2|ContainerCanTake3|ContainerCanTake4|ContainerCanTake5|ContainerCanTake6|ContainerCanTake7,

            AllHangar   = AllHangarTake | AllHangarQuery,
            AllAt       = AllHangar|AllAccountTake|AllContainerTake,
            AllAccount  = JuniorAccountant|Accountant,
            All         = AllHangar|AllAccount|AllOffice|AllStarbase|AllManager|Auditor|Diplomat,
            Admin       = All|Trader|SecurityOfficer|Director,

            None        = 0


            /* 18:05:31 G   Alasiya's EvEMu: Common Corp Roles:
             * 18:37:16 W          Role_All:  1134904941433847168(0xfbffe07fffffd80)
             * 18:37:16 W         Role_Cont:  558551906910208(0x1fc0000000000)
             * 18:37:16 W        Role_Admin:  1152919339943329665(0xffffe07ffffff81)
             * 18:37:16 W       Role_Hangar:  134209536(0x7ffe000)
             * 18:37:16 W      Role_Account:  4503616673022208(0x100003f8000100)
             * 18:37:16 W     Role_Starbase:  441352763482308608(0x620000000000000)
             */
        };
    }
}

#endif  //EVE_ROLES_H