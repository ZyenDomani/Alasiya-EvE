
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

// Container for all ships modules
class ModuleContainer
{
public:
    ModuleContainer(ShipItem* pShip);
    ~ModuleContainer();

    void ClearModMap();

    GenericModule* GetModule(EVEItemFlags flag); //faster than GetModule(itemID)
    GenericModule* GetModule(uint32 itemID); //slower than GetModule(flag)
    GenericModule* GetRandModule();

    // basic methods
    void SaveModules();
    void ShipWarping();
    void GetModuleListOfRefsAsc(std::vector<InventoryItemRef>& pModuleList);
    void GetModuleListOfRefsDec(std::vector<InventoryItemRef>& pModuleList);

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

    //useful accessors
    bool isSlotOccupied(EVEItemFlags flag);

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