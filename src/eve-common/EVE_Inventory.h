/*
 *
 *
 *
 */


#ifndef EVE_INVENTORY_H
#define EVE_INVENTORY_H

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
        // updated for crucible.  -allan 16May16
        enum {
            Item          = 0,    //also ixLauncherCapacity?
            Type          = 1,    //also ixLauncherUsed = 1,
            Owner         = 2,    //also ixLauncherChargeItem?
            Location      = 3,
            Flag          = 4,
            Quantity      = 5,
            Group         = 6,
            Category      = 7,
            CustomInfo    = 8,
            StackSize     = 9,
            Singleton     = 10,
            Subitems      = 11        // not in client data
        };
    }

}

/*  AuditLogSecureContainer shit here....
*/
namespace ALSC {

    namespace Action {
        enum {
            Assemble        = 1,
            Repackage       = 2,
            SetName         = 3,
            Move            = 4,
            SetPassword     = 5,
            Add             = 6,
            Lock            = 7,
            Unlock          = 8,
            EnterPassword   = 9,
            Configure       = 10
        };
    }

    namespace NeedPass {
        enum {
            ToOpen          = 1,
            ToLock          = 2,
            ToUnlock        = 4,
            ToViewAuditLog  = 8
        };
    }
}

#endif  // EVE_INVENTORY_H
