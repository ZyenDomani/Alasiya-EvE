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

/*
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
