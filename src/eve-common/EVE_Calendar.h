
//  -allan 7Jul14   UD 30Oct20 (still not implemented)


#ifndef EVE_CALENDAR_H
#define EVE_CALENDAR_H

namespace Calendar {
    namespace Day {
        enum {
            Monday = 0,
            Tuesday = 1,
            Wednesday = 2,
            Thursday = 3,
            Friday = 4,
            Saturday = 5,
            Sunday = 6,
            DaysInWeek = 7
        };
    }
    namespace Month {
        enum {
            January = 1,
            February = 2,
            March = 3,
            April = 4,
            May = 5,
            June = 6,
            July = 7,
            August = 8,
            September = 9,
            October = 10,
            November = 11,
            December = 12
        };
    }

    namespace Flag {
        enum {
            Invalid = 0,
            Personal = 1,
            Corp = 2,
            Alliance = 4,
            CCP = 8,
            Automated = 16,
        };
    }

    namespace Response {
        enum {
            Uninvited = 0,
            Deleted = 1,
            Declined = 2,
            Undecided = 3,
            Accepted = 4,
            Maybe = 5
        };
    }
    enum {
        AutoEventPosFuel = 1,
        ViewRangeInMonths = 12,
        MaxTitleSize = 40,
        MaxInvitees = 50,
        MaxInviteeDisplayed = 100,
        MaxDescrSize = 500,
        StartYear = 2003,
    };
}


#endif  // EVE_CALENDAR_H