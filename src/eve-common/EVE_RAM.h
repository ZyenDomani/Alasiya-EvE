
/*
 * EVE_RAM.h
 *  enums for R.A.M.
 *
 */


#ifndef EVE_RAM_ENUMS_H
#define EVE_RAM_ENUMS_H

/*
BASE_INVENTION_ME = -4.0
BASE_INVENTION_PE = -4.0
*/

/*  for indy */
namespace EvERam {
    namespace Activity {
        //from table 'ramActivities'
        enum  {
            Manufacturing = 1,
            ResearchTech = 2,
            ResearchTime = 3,
            ResearchMaterial = 4,
            Copying = 5,
            Duplicating = 6,
            ReverseEngineering = 7,
            Invention = 8
        };
    }

    namespace CompletedStatus {
        //from table 'ramCompletedStatuses'
        enum  {
            InProgress = 0,
            Delivered = 1,
            Abort = 2,
            GMAbort = 3,
            Unanchor = 4,
            Destruction = 5
        };
    }

    namespace RestrictionMask {
        //restrictionMask from table 'ramAssemblyLines'
        enum  {
            None = 0,
            BySecurity = 1,
            ByStanding = 2,
            ByCorp = 4,
            ByAlliance = 8
        };
    }

    /* POD structure for blueprint ram requirements */
    struct RamRequirements {
        bool extra;
        uint8 activityID;
        uint16 requiredTypeID;
        uint32 quantity;
        float damagePerJob;
    };

    /* POD structure for blueprint item materials  */
    struct RamMaterials {
        uint16 materialTypeID;
        uint32 quantity;
    };

    /* POD structure for blueprint required materials  */
    struct RequiredItem {
        RequiredItem(uint16 _typeID, uint32 _quantity, double _damagePerJob, bool _isSkill, bool _extra)
        : typeID(_typeID), quantity(_quantity), damagePerJob(_damagePerJob), isSkill(_isSkill), extra(_extra) {}

        bool extra;
        uint16 typeID;
        uint32 quantity;
        double damagePerJob;
        bool isSkill;
    };

    /* POD structure for indy job data  */
    struct JobProperties {
        int8 activity;
        int16 jobRuns;
        int16 licensedRuns;
        uint32 bpID;
        uint32 ownerID;
        EVEItemFlags outputFlag;
    };

}

#endif  //EVE_RAM_ENUMS_H