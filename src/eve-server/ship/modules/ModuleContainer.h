
 /**
  * @name ModuleContainer.h
  *     module container class for ship's module manager
  *
  * @Author:    Allan
  * @date:      6Feb18  (split from moduleMgr code)
  *
  */


#ifndef EVE_SHIP_MODULES_MODULECONTAINER_H
#define EVE_SHIP_MODULES_MODULECONTAINER_H


#include "ship/modules/GenericModule.h"
#include "ship/modules/ModuleDefs.h"

class ShipItem;
class SystemEntity;

// Container for all ships modules
class ModuleContainer
{
public:
    ModuleContainer(ShipItem* pShip);
    ~ModuleContainer();

    void LoadOnline();
    // this will populate the module map for all slots with nullptr
    void ClearModMap();

    GenericModule* GetRandModule();
    //faster than GetModule(itemID)
    GenericModule* GetModule(EVEItemFlags flag);
    //slower than GetModule(flag)
    GenericModule* GetModule(uint32 itemID);

    // basic methods
    void SaveModules();
    void ShipWarping();
    void GetWeapons(std::list<GenericModule*>& weaponList);
    // low, mid, hi, rig, subsys
    void GetModuleListOfRefsAsc(std::vector<InventoryItemRef>& modVec);
    // subsys, rig, hi, mid, low
    void GetModuleListOfRefsDec(std::vector<InventoryItemRef>& modVec);
    // subsys, rig, low, mid, hi
    void GetModuleListOfRefsOrdered(std::vector< InventoryItemRef >& modVec);
    // hi, mid, low, rig, subsys
    void GetModuleListOfRefsOrderedRev(std::vector<InventoryItemRef>& modVec);

    // returns vector of fitted GenericModule* in specified flag's bank
    void GetModulesInBank(EVEItemFlags flag, std::vector<GenericModule*>& modVec);

    bool AddModule(EVEItemFlags flag, GenericModule* mod);
    bool RemoveModule(EVEItemFlags flag);
    bool RemoveModule(uint32 itemID);

    //batch processes handlers
    void AbortCycle();
    void Process();
    void OfflineAll();
    void OnlineAll();
    void DeactivateAll();
    void UnloadAll();
    void RepairAll();
    void UnloadWeapons();
    void RemoveTarget(SystemEntity* pSE);
    void CargoFull();

    // called by MM::fitModule and MM::VerifySlotExchange
    bool isSlotOccupied(EVEItemFlags flag); // flag is not checked in this call

    //useful accessors
    uint8 GetLowSlotCount()                             { return m_LowSlots; }
    uint8 GetMedSlotCount()                             { return m_MediumSlots; }
    uint8 GetHighSlotCount()                            { return m_HighSlots; }
    uint8 GetRigSlotCount()                             { return m_RigSlots; }
    uint8 GetSubSysCount()                              { return m_SubSystemSlots; }
    uint8 GetFittedModuleCountByGroup(uint16 groupID);

    uint16 GetAvailableSlotInBank(EVEEffectID slotBank);

private:

    void deleteModuleRef(EVEItemFlags flag, GenericModule* mod);

    uint8 m_LowSlots;
    uint8 m_MediumSlots;
    uint8 m_HighSlots;
    uint8 m_RigSlots;
    uint8 m_SubSystemSlots;

    // map of all module slots by flag
    std::map<uint8, GenericModule*> m_modules;      // k,v of flag, module
    std::map<uint32, uint32> m_ModulesFittedByGroupID;
};

#endif  // EVE_SHIP_MODULES_MODULECONTAINER_H