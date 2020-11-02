
 /**
  * @name FactoryDB.h
  *   db query methods for R.A.M. activities
  *
  * @Author:         Allan
  * @date:          9Jan18
  */


#ifndef EVE_MANUF_FACTORYDB_H
#define EVE_MANUF_FACTORYDB_H

#include "ServiceDB.h"
#include "../eve-common/EVE_RAM.h"
#include "packets/Manufacturing.h"
#include "inventory/InventoryItem.h"


class FactoryDB
: public ServiceDB
{
public:
    // client calls
    static PyRep* GetJobs2(const int32 ownerID, const bool completed);
    static PyRep* AssemblyLinesSelectPublic(const uint32 regionID);
    static PyRep* AssemblyLinesSelectPersonal(const uint32 charID);
    static PyRep* AssemblyLinesSelectPrivate(const uint32 charID);
    static PyRep* AssemblyLinesSelectCorporation(const uint32 corporationID);
    static PyRep* AssemblyLinesSelectAlliance(const int32 allianceID);
    static PyRep* AssemblyLinesGet(const uint32 containerID);
    static PyRep* GetMaterialCompositionOfItemType(const uint32 typeID);

    // for static data mgr
    static void GetRAMMaterials(DBQueryResult& res);
    static void GetBlueprintType(DBQueryResult& res);
    static void GetRAMRequirements(DBQueryResult& res);

    // InstallJob stuff
    static bool GetAssemblyLineProperties(const uint32 assemblyLineID, Rsp_InstallJob &into);
    static bool GetAssemblyLineVerifyProperties(const uint32 assemblyLineID, uint32& ownerID, double& minCharSecurity, double& maxCharSecurity, int8& restrictionMask, int8& activity);
    static bool InstallJob(const uint32 ownerID, const uint32 installerID, const uint32 assemblyLineID, const uint32 installedItemID, const int64 beginProductionTime, const int64 endProductionTime, const char* description, const int32 runs, const EVEItemFlags outputFlag, const uint32 installedInSolarSystem, const int32 licensedProductionRuns);

    // CompleteJob stuff
    static bool GetJobProperties(const uint32 jobID, uint32& installedItemID, uint32& ownerID, EVEItemFlags& outputFlag, int32& runs, int32& licensedProductionRuns, int8& activity);
    static bool GetJobVerifyProperties(const uint32 jobID, uint32& ownerID, int64& endProductionTime, int8& restrictionMask, int8& status);
    static bool CompleteJob(const uint32 jobID, const int8 completedStatus);

    // misc queries
    static bool DeleteBlueprint(uint32 blueprintID);
    static bool GetBlueprint(uint32 blueprintID, EvERam::bpData& into);
    static bool SaveBlueprintData(uint32 blueprintID, EvERam::bpData& data);
    static bool IsProducableBy(const uint32 assemblyLineID, const uint32 groupID);
    static bool GetMultipliers(const uint32 assemblyLineID, uint32 groupID, double &materialMultiplier, double &timeMultiplier);

    static uint32 CountManufacturingJobs(const uint32 installerID);
    static uint32 CountResearchJobs(const uint32 installerID);
    static uint32 GetTech2Blueprint(const uint32 blueprintTypeID);

    static int64 GetNextFreeTime(const uint32 assemblyLineID);

};

#endif  // EVE_MANUF_FACTORYDB_H

