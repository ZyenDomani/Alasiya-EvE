/*
 *
 *
 *
 */


#ifndef EVE_INVENTORY_H
#define EVE_INVENTORY_H

//these came from the 'constants' object:
// updated for change in crucible.  -allan 16May16
enum EVEItemChangeType {
    ixItemID        = 0,    //also ixLauncherCapacity?
    ixTypeID        = 1,    //also ixLauncherUsed = 1,
    ixOwnerID       = 2,    //also ixLauncherChargeItem?
    ixLocationID    = 3,
    ixFlag          = 4,
    ixQuantity      = 5,
    ixGroupID       = 6,
    ixCategoryID    = 7,
    ixCustomInfo    = 8,
    ixStackSize     = 9,
    ixSingleton     = 10,
    ixSubitems      = 11        // not in client data
};

enum EVEContainerTypes {
    containerWallet            = 10001,
    containerGlobal            = 10002,
    containerSolarSystem       = 10003,
    containerHangar            = 10004,
    containerScrapHeap         = 10005,
    containerFactory           = 10006,
    containerBank              = 10007,
    containerRecycler          = 10008,
    containerOffices           = 10009,
    containerStationCharacters = 10010,
    containerCharacter         = 10011,
    containerCorpMarket        = 10012
};

/** @todo finish implementing these... */
namespace Inv {

    namespace Container {
        enum {
            Wallet            = 10001,
            Global            = 10002,
            SolarSystem       = 10003,
            Hangar            = 10004,
            ScrapHeap         = 10005,
            Factory           = 10006,
            Bank              = 10007,
            Recycler          = 10008,
            Offices           = 10009,
            StationCharacters = 10010,
            Character         = 10011,
            CorpMarket        = 10012
        };
    }

    namespace Update {
        //these are used with the OnItemChange packet to update client's item data (and trigger other actions)
        // updated for change in crucible.  -allan 16May16
        enum {
            ItemID        = 0,    //also ixLauncherCapacity?
            TypeID        = 1,    //also ixLauncherUsed = 1,
            OwnerID       = 2,    //also ixLauncherChargeItem?
            LocationID    = 3,
            Flag          = 4,
            Quantity      = 5,
            GroupID       = 6,
            CategoryID    = 7,
            CustomInfo    = 8,
            StackSize     = 9,
            Singleton     = 10,
            Subitems      = 11        // not in client data
        };
    }

}

/*  AuditLogSecureContainer shit here....
    namespace ALSC {

    }
ALSCActionAdd = 6
ALSCActionAssemble = 1
ALSCActionConfigure = 10
ALSCActionEnterPassword = 9
ALSCActionLock = 7
ALSCActionMove = 4
ALSCActionRepackage = 2
ALSCActionSetName = 3
ALSCActionSetPassword = 5
ALSCActionUnlock = 8
ALSCPasswordNeededToLock = 2
ALSCPasswordNeededToOpen = 1
ALSCPasswordNeededToUnlock = 4
ALSCPasswordNeededToViewAuditLog = 8
*/


#endif  // EVE_INVENTORY_H
