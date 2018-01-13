
 /**
  * @name RamMethods.h
  *   methods for R.A.M. activities
  *
  * @Author:         Allan
  * @date:          9Jan18
  */


#ifndef EVE_MANUF_RAM_METHODS_H
#define EVE_MANUF_RAM_METHODS_H


#include "manufacturing/RamProxyDB.h"

class Client;

class RamMethods {
public:
    RamMethods();
    virtual ~RamMethods();

    static void ActivityCheck(Client* const pClient, const Call_InstallJob& args, InventoryItemRef installedItem);
    static void JobsCheck(Character* pChar, const Call_InstallJob& args);
    static void InstallationCheck(Client* const pClient, int32 installationContainerID);
    static void AssemblyLineCheck(Client* const pClient, const Call_InstallJob& args);
    static void ItemLocationCheck(Client* const pClient, const Call_InstallJob& args, InventoryItemRef installedItem);
    static void ItemPermissionCheck(Client* const pClient, const Call_InstallJob& args, InventoryItemRef installedItem);

    static void LocationRolesCheck(Client* const pClient, int16 flagID);

    static void ProductionTimeCheck(uint32 productionTime);
    static void MaterialSkillsCheck(Client* const pClient, uint32 runs, const PathElement& bomLocation, const Rsp_InstallJob& rsp, const std::vector< EvERam::RequiredItem >& reqItems);

    static void CompleteJob(const Call_CompleteJob &args, Client *const c);

    static bool Calculate(const Call_InstallJob &args, InventoryItemRef installedItem, Client *const c, Rsp_InstallJob &into);
    static void EncodeBillOfMaterials(const std::vector< EvERam::RequiredItem >& reqItems, double materialMultiplier, double charMaterialMultiplier, uint32 runs, BillOfMaterials& into);
    static void EncodeMissingMaterials(const std::vector< EvERam::RequiredItem >& reqItems, const PathElement& bomLocation, Client*const pClient, double materialMultiplier, double charMaterialMultiplier, int32 runs, std::map< int32, PyRep* >& into);

    static void GetBOMItems(const PathElement &bomLocation, std::vector<InventoryItemRef> &into);
    static bool GetMultipliers(const uint32 assemblyLineID, const uint32 productGroupID, double &materialMultiplier, double &timeMultiplier);

    static void GetAdjustedRamRequiredMaterials();
    static std::string GetActivityName(int8 activityID);
};


#endif  // EVE_MANUF_RAM_METHODS_H