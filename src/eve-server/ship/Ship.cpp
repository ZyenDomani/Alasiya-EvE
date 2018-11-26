
#include "Client.h"
#include "EntityList.h"
#include "EVEServerConfig.h"
#include "Profile.h"
#include "StaticDataMgr.h"
#include "account/AccountService.h"
#include "character/Character.h"
#include "effects/EffectsProcessor.h"
#include "ship/Ship.h"
#include "station/Station.h"
#include "system/DestinyManager.h"
#include "system/BubbleManager.h"
#include "system/SolarSystem.h"

/*
 * ShipTypeData
 */
ShipTypeData::ShipTypeData( uint32 weaponTypeID, uint32 miningTypeID, uint32 skillTypeID)
: mWeaponTypeID(weaponTypeID),
mMiningTypeID(miningTypeID),
mSkillTypeID(skillTypeID) {}
/*
 * ShipType
 */
ShipType::ShipType(
    uint32 _id,
    // ItemType stuff:
    const ItemGroup &_group,
    const TypeData &_data,
    // ShipType stuff:
    const ItemType *_weaponType,
    const ItemType *_miningType,
    const ItemType *_skillType,
    const ShipTypeData &stData)
: ItemType(_id, _group, _data),
m_weaponType(_weaponType),
m_miningType(_miningType),
m_skillType(_skillType)
{
    // data consistency checks:
    if (_weaponType)
        assert(_weaponType->id() == stData.mWeaponTypeID);
    if (_miningType)
        assert(_miningType->id() == stData.mMiningTypeID);
    if (_skillType)
        assert(_skillType->id() == stData.mSkillTypeID);
}

ShipType *ShipType::Load(uint32 shipTypeID)
{
    return ItemType::Load<ShipType>(shipTypeID );
}

/*
 * ShipItem
 */
ShipItem::ShipItem(uint32 _shipID, const ShipType &_shipType, const ItemData &_data)
: InventoryItem(_shipID, _shipType, _data),
m_pilot(nullptr),
m_ModuleManager(nullptr)
{
    m_isPopped = false;
    m_IsLoaded = false;
    m_isDocking = false;
    m_isUndocking = false;
    m_onlineModuleVec.clear();
    m_targetRef = InventoryItemRef(nullptr);
    pInventory = new Inventory(InventoryItemRef(this));
    _log(ITEM__TRACE, "Created ShipItem for %s(%u).", itemName().c_str(), itemID());
}

ShipItem::~ShipItem()
{
    SafeDelete(pInventory);
    SafeDelete(m_ModuleManager);
}

ShipItemRef ShipItem::Load( uint32 shipID)
{
    return InventoryItem::Load<ShipItem>( shipID );
}

ShipItemRef ShipItem::Spawn( ItemData &data) {
    uint32 shipID = ShipItem::CreateItemID( data );
    if ( shipID == 0 )
        return ShipItemRef(nullptr);

    ShipItemRef sShipRef = ShipItem::Load( shipID );

    return sShipRef;
}

uint32 ShipItem::CreateItemID( ItemData &data) {
    return InventoryItem::CreateItemID(data);
}

bool ShipItem::_Load()
{
    // load attributes
    if (!InventoryItem::_Load())
        return false;
    if (type().id() == EVEDB::invTypes::typeCapsule)
        return (m_IsLoaded = true);
    if (m_IsLoaded and (m_ModuleManager != nullptr))
        return true;
    // load contents
    if (!pInventory->LoadContents())
        return false;

    return (m_IsLoaded = true);
}

void ShipItem::Init()
{
    if (m_type.groupID() == EVEDB::invGroups::Capsule) {
        InitPod();
        return;
    }
    if (! m_pilot->GetChar().get()) {
        _log(SHIP__WARNING, "ShipItem %s(%u) does not have a pilot.", itemName().c_str(), itemID());
        return;
    }

    ClearModifiers();
    InitAttribs();

    // create and initialize the module manager if not already done
    if (m_ModuleManager == nullptr)
        m_ModuleManager = new ModuleManager(this);
    m_ModuleManager->Initialize();
}

void ShipItem::InitPod() {
    // allocate the module manager, only the first time:
    if (m_ModuleManager == nullptr) {
        m_ModuleManager = new ModuleManager(this);
        m_ModuleManager->Initialize();
    }
    // pods have 57 attribs and 0 effects

    // pod will be full when activated
    if (m_pilot->IsInSpace())
        Heal();
}

void ShipItem::LogOut()
{
    // remove module effects
    //m_ModuleManager->OfflineAll();
    // reset ship effects and save ship data
    //ProcessEffects();

    SaveShip();

    // remove ship item from factory master list here, as *something* changes ship postion when saving items from factory.
    sItemFactory.RemoveItem(m_itemID);

    // remove ship item from its' container's inventory list also.
    Inventory* pInv(nullptr);
    if (IsStation(m_locationID))
        pInv = sItemFactory.GetStation(m_locationID)->GetMyInventory();
    else
        pInv = sItemFactory.GetSolarSystem(m_locationID)->GetMyInventory();

    if (pInv != nullptr)
        pInv->RemoveItem(pInv->GetByID(m_itemID));
}

void ShipItem::SetPlayer(Client* pClient) {
    if (m_pilot == pClient)
        return;
    /* to reset for new pilot:
     * offline all modules
     * reset ship attribs
     * add new pilot skills
     * online all modules
     */

    m_pilot = pClient;
    if (m_pilot == nullptr) {
        // remove ship effects and char skill effects for char leaving ship here.
        ProcessEffects(false);
        // should we check for cargo and damage after char leaves ship?  maybe later
        m_onlineModuleVec.clear();
        return;
    }

    Init();
    ProcessEffects(true, IsSolarSystem(m_locationID));
    OnlineAll();

    // this hits ONLY when boarding ship in space.  will not hit on Undock() (location is still station at this point of execution)
    if (IsSolarSystem(m_locationID))
        if (pClient->IsLogin()) {
            if (sConfig.debug.IsTestServer) {
                // Heal Ship completely on test server
                Heal();
            } else {
                // live server will Recharge shields and cap if session change isnt active
                if (!m_pilot->IsSessionChange()) {
                    SetShipShield(1.0);
                    SetShipCapacitorLevel(1.0);
                }
            }
        }
}

void ShipItem::InitAttribs()
{
    // Create default dynamic attributes in the AttributeMap
    SetAttribute(AttrVolume,                            GetPackagedVolume());
    SetAttribute(AttrCpuLoad,                           0);
    SetAttribute(AttrPowerLoad,                         0);
    SetAttribute(AttrUpgradeLoad,                       0); // rig shit

    // Check for existence of attributes.  if not loaded then set them to default values:
    if (!HasAttribute(AttrDamage))                      SetAttribute(AttrDamage, 0.0f);
    if (!HasAttribute(AttrArmorDamage))                 SetAttribute(AttrArmorDamage, 0.0f);
    // shield and cap are part of persistance, and loaded on attrib map initalization.  check for and set to full if no saved value found
    if (!HasAttribute(AttrShieldCharge))                SetAttribute(AttrDamage,  GetAttribute(AttrShieldCapacity));
    if (!HasAttribute(AttrCapacitorCharge))             SetAttribute(AttrDamage,  GetAttribute(AttrCapacitorCapacity));
    if (!HasAttribute(AttrMaximumRangeCap))             SetAttribute(AttrMaximumRangeCap, ((double)BUBBLE_RADIUS_METERS));
    // Warp Scramble Status of the ship (most ships have zero warp scramble status, but some (t2 indy) already have it defined):
    if (!HasAttribute(AttrWarpScrambleStatus))          SetAttribute(AttrWarpScrambleStatus, 0.0f);
    if (!HasAttribute(AttrWarpSpeedMultiplier))         SetAttribute(AttrWarpSpeedMultiplier, 1.0f);
    if (!HasAttribute(AttrArmorMaxDamageResonance))     SetAttribute(AttrArmorMaxDamageResonance, 1.0f);
    if (!HasAttribute(AttrShieldMaxDamageResonance))    SetAttribute(AttrShieldMaxDamageResonance, 1.0f);
    // hull res is stored in item type as AttrHull*Resonance for 6 ships.  set accordingly
    if (!HasAttribute(AttrEmDamageResonance))           SetAttribute(AttrEmDamageResonance,  GetAttribute(AttrHullEmDamageResonance));
    if (!HasAttribute(AttrExplosiveDamageResonance))    SetAttribute(AttrExplosiveDamageResonance,  GetAttribute(AttrHullExplosiveDamageResonance));
    if (!HasAttribute(AttrKineticDamageResonance))      SetAttribute(AttrKineticDamageResonance,  GetAttribute(AttrHullKineticDamageResonance));
    if (!HasAttribute(AttrThermalDamageResonance))      SetAttribute(AttrThermalDamageResonance,  GetAttribute(AttrHullThermalDamageResonance));
}

void ShipItem::Delete() {
    pInventory->DeleteContents();
    InventoryItem::Delete();
}

/** @todo this will need more work to correctly check hold capacity for offline/unloaded ships */
double ShipItem::GetRemainingVolumeByFlag(EVEItemFlags flag) const {
    // updated to use inventory  -allan 26Jul16  -fixed 22Nov18
    return (pInventory->GetCapacity(flag) - pInventory->GetStoredVolume(flag));
}

bool ShipItem::ValidateAddItem(EVEItemFlags flag, InventoryItemRef iRef, Client* pClient/*nullptr*/)
{
    // if *this ship isnt active, it wont have a pilot to send errors to.  test and set as needed.
    if (pClient == nullptr) {
        if (m_pilot == nullptr)
            return false;
        pClient = m_pilot;
    }

    switch (flag) {
        case flagDroneBay: {
            if ( iRef->categoryID() != EVEDB::invCategories::Drone ) {
                pClient->SendErrorMsg("Item Cannot be stowed in the Drone Bay");
                return false;
            }
        } break;
        case flagShipHangar: {    //AttrShipMaintenanceBayCapacity
            if (!HasAttribute(AttrHasShipMaintenanceBay)) {
                pClient->SendErrorMsg("Your %s has no ship maintenance bay.", itemName().c_str());
                return false;
            }
            if (iRef->categoryID() != EVEDB::invCategories::Ship) {
                pClient->SendErrorMsg("Only ships may be placed into the maintenance bay.");
                return false;
            }
        } break;

        // not sure if all of these flagSpecialized* are used.  if not, *may* update dgmData to add them....later.
        case flagSpecializedFuelBay: {    //  AttrSpecialFuelBayCapacity        [dunno on this one - AttrFuelCargoCapacity]
            if (iRef->groupID() != EVEDB::invGroups::FuelBlock) {
                pClient->SendErrorMsg("Only fuel blocks may be placed into the fuel bay.");
                return false;
            }
        } break;
        case flagSpecializedOreHold: {
            if (iRef->categoryID() != EVEDB::invCategories::Asteroid) {
                pClient->SendErrorMsg("Only mined ore may be placed into the ore hold.");
                return false;
            }
        } break;
/*
        case flagSpecializedGasHold: {
            if (iRef->categoryID() != EVEDB::invCategories::Ship) {
                pClient->SendErrorMsg("Only ships may be placed into ship maintenance bay.");
                return false;
            }
        } break;
*/
        case flagSpecializedMineralHold: {
            if (iRef->groupID() != EVEDB::invGroups::Mineral) {
                pClient->SendErrorMsg("Only refined minerals may be placed into the mineral hold.");
                return false;
            }
        } break;
        case flagSpecializedSalvageHold: {
            if (iRef->groupID() != EVEDB::invGroups::Salvage_Materials) {
                pClient->SendErrorMsg("Only salvaged materials may be placed into the salvage bay.");
                return false;
            }
        } break;
        case flagSpecializedShipHold: {
            if (iRef->categoryID() != EVEDB::invCategories::Ship) {
                pClient->SendErrorMsg("Only ships may be placed into the ship hold.");
                return false;
            }
        } break;

        /** @todo need to figure out how to separate ships into s/m/l/i for these.... */
        case flagSpecializedSmallShipHold: {
            if (iRef->categoryID() != EVEDB::invCategories::Ship) {
                pClient->SendErrorMsg("Only small ships may be placed into the ship's small ship hold.");
                return false;
            }
        } break;
        case flagSpecializedMediumShipHold: {
            if (iRef->categoryID() != EVEDB::invCategories::Ship) {
                pClient->SendErrorMsg("Only medium ships may be placed into the ship's medium ship hold.");
                return false;
            }
        } break;
        case flagSpecializedLargeShipHold: {
            if (iRef->categoryID() != EVEDB::invCategories::Ship) {
                pClient->SendErrorMsg("Only large ships may be placed into the ship's large ship hold.");
                return false;
            }
        } break;
        case flagSpecializedIndustrialShipHold: {
            if (iRef->categoryID() != EVEDB::invCategories::Ship) {
                pClient->SendErrorMsg("Only indy ships may be placed into the ship's industurial ship hold.");
                return false;
            }
        } break;
        case flagSpecializedAmmoHold: {
            if ((iRef->groupID() != EVEDB::invGroups::Ammo)
            and (iRef->groupID() != EVEDB::invGroups::Advanced_Artillery_Ammo)
            and (iRef->groupID() != EVEDB::invGroups::Advanced_Autocannon_Ammo)
            and (iRef->groupID() != EVEDB::invGroups::Advanced_Blaster_Ammo)
            and (iRef->groupID() != EVEDB::invGroups::Advanced_Railgun_Ammo)
            and (iRef->groupID() != EVEDB::invGroups::Hybrid_Ammo)) {
                pClient->SendErrorMsg("Only ammunition may be placed into the ammo bay.");
                return false;
            }
        } break;
        case flagHangar: {    //AttrCorporateHangarCapacity
            if (GetAttribute(AttrHasCorporateHangars) == 0) {
                pClient->SendErrorMsg("Your %s has no corporate hangars.", itemName().c_str());
                return false;
            }
        } break;
        default: {
            if (IsRigSlot(flag)) {
                if (pClient->IsClient())
                    if (!Skill::FitModuleSkillCheck(iRef, pClient->GetChar())) {
                        pClient->SendErrorMsg("You do not have the required skills to fit this %s", iRef->itemName().c_str());
                        return false;
                    }
            } else if (IsSubSystem(flag)) {
                if (pClient->IsClient())
                    if (!Skill::FitModuleSkillCheck(iRef, pClient->GetChar())) {
                        pClient->SendErrorMsg("You do not have the required skills to fit this %s", iRef->itemName().c_str());
                        return false;
                    }
            } else if (IsModuleSlot(flag)) {
                if (!Skill::FitModuleSkillCheck(iRef, pClient->GetChar())) {
                    pClient->SendErrorMsg("You do not have the required skills to fit this %s.  Ref: ServerError 25163.", iRef->itemName().c_str());
                    return false;
                }
                if (!ValidateItemSpecifics(iRef)) {
                    pClient->SendErrorMsg("Your ship cannot equip this %s.  Ref: ServerError 25165.", iRef->itemName().c_str());
                    return false;
                }
                if (iRef->categoryID() == EVEDB::invCategories::Charge) {
                    if (m_ModuleManager == nullptr)
                        return false;   // log error?
                    if (m_ModuleManager->GetModule(flag)) {
                        InventoryItemRef module = m_ModuleManager->GetModule(flag)->GetSelf();
                        if (module.get() == nullptr)
                            return false;
                        if (module->GetAttribute(AttrChargeSize) != iRef->GetAttribute(AttrChargeSize)) {
                            sLog.Error("Ship::ValidateAddItem", "Charge size %u for %s does not match Module size %u for %s.",\
                                iRef->GetAttribute(AttrChargeSize).get_int(), iRef->itemName().c_str(),\
                                module->GetAttribute(AttrChargeSize).get_int(), module->itemName().c_str());
                            pClient->SendErrorMsg("Incorrect charge size for this module.");
                            return false;
                        }
                        if ((module->GetAttribute(AttrChargeGroup1) != iRef->groupID())
                        and (module->GetAttribute(AttrChargeGroup2) != iRef->groupID())
                        and (module->GetAttribute(AttrChargeGroup3) != iRef->groupID())
                        and (module->GetAttribute(AttrChargeGroup4) != iRef->groupID())
                        and (module->GetAttribute(AttrChargeGroup5) != iRef->groupID())) {
                            pClient->SendErrorMsg("Incorrect charge type for this module.");
                            return false;
                        }
                    // NOTE: Module Manager will check for actual room to load charges and make stack splits, or reject loading altogether
                    } else {
                        pClient->SendErrorMsg("Module at flag '%u' does not exist.  Ref: ServerError 25162.", flag);
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

// this one is called from ShipGetInfo
PyDict* ShipItem::ShipGetInfo() {
    if ( !pInventory->LoadContents()) {
        _log( SHIP__ERROR, "%s(%u): Failed to load contents for ShipGetInfo", itemName().c_str(), itemID());
        return nullptr;
    }

    Rsp_CommonGetInfo_Entry entry;
    //first populate the ship.
    if ( !Populate( entry ))
        return nullptr;    //print already done.

    PyDict* result = new PyDict();
    result->SetItem(new PyInt( itemID()), new PyObject("util.KeyVal", entry.Encode()));
    //now encode contents...
    std::vector<InventoryItemRef> equipped;
    //find all the equipped items and rigs
    uint8 mod = pInventory->FindByFlagRange( flagLowSlot0, flagFixedSlot, equipped );
    uint8 rig = pInventory->FindByFlagRange( flagRigSlot0, flagRigSlot7, equipped );
    //encode an entry for each one.
    for (auto cur : equipped) {
        if (cur->Populate(entry))
            result->SetItem(new PyInt(cur->itemID()), new PyObject("util.KeyVal", entry.Encode()));
        else
            _log( SHIP__ERROR, "%s(%u): Failed to load item %u for ShipGetInfo", itemName().c_str(), itemID(), cur->itemID());
    }
    return result;
}

// this one is called from GetAllInfo
PyDict* ShipItem::GetShipInfo()
{
    if (!pInventory->LoadContents())  {
        _log( INV__ERROR, "%s(%u): Failed to load contents for ShipGetInfo", itemName().c_str(), itemID());
        return nullptr;
    }

    //first populate the ship.
    Rsp_CommonGetInfo_Entry entry;
    if ( !Populate( entry ))
        return nullptr;

    PyDict *result = new PyDict();
    result->SetItem(new PyInt( itemID()), new PyObject("util.KeyVal", entry.Encode()));

    //now encode contents...
    std::vector<InventoryItemRef> equipped;
    //find all the equipped items and rigs
    uint8 mod = pInventory->FindByFlagRange( flagLowSlot0, flagFixedSlot, equipped );
    uint8 rig = pInventory->FindByFlagRange( flagRigSlot0, flagRigSlot7, equipped );
    //encode an entry for each one.
    for (auto cur : equipped) {
        Rsp_CommonGetInfo_Entry entry2;
        if (cur->Populate(entry2)) {
            if (cur->groupID() == EVEDB::invCategories::Charge) {
                PyTuple* tuple = new PyTuple(3);
                    tuple->SetItem(0, new PyInt(cur->itemID()));
                    tuple->SetItem(1, new PyInt(cur->flag()));
                    tuple->SetItem(2, new PyInt(cur->typeID()));
                result->SetItem(tuple, new PyObject("util.KeyVal", entry2.Encode()));
            } else {
                result->SetItem(new PyInt(cur->itemID()), new PyObject("util.KeyVal", entry2.Encode()));
            }
        } else
            _log( SHIP__ERROR, "%s(%u): Failed to load item %u for ShipGetInfo", itemName().c_str(), itemID(), cur->itemID());
    }

    return result;
}

PyDict* ShipItem::GetShipState() {
    if (!pInventory->ContentsLoaded()) {
        if (!pInventory->LoadContents()) {
            _log(INV__ERROR, "%s(%u): Failed to load contents for GetShipState", itemName().c_str(), itemID());
            return nullptr;
        }
    }
    // Create new dictionary for shipState:
    PyDict *result = new PyDict();
    // Create entry for ShipItem itself:
    result->SetItem(new PyInt(itemID()), GetItemStatusRow());
    // Check for and Create entry for pilot:
    InventoryItemRef iRefPilot;
    if (pInventory->FindSingleByFlag(flagPilot, iRefPilot))
        result->SetItem(new PyInt(iRefPilot->itemID()), iRefPilot->GetItemStatusRow());

    if (m_ModuleManager == nullptr) {
        m_ModuleManager = new ModuleManager(this);
        m_ModuleManager->Initialize();
    }
    // Create entries for ALL modules, rigs, and subsystems present on ship:
    std::vector<InventoryItemRef> moduleList;
    m_ModuleManager->GetModuleListOfRefsAsc(moduleList);
    for (int i=0; i<moduleList.size(); ++i)
        result->SetItem(new PyInt(moduleList.at(i)->itemID()), moduleList.at(i)->GetItemStatusRow());

    return result;
}

PyList* ShipItem::ShipGetModuleList() {
    if (!pInventory->LoadContents()) {
        _log(INV__ERROR, "%s(%u): Failed to load contents for ShipGetModuleList", itemName().c_str(), itemID());
        return nullptr;
    }
    if (m_ModuleManager == nullptr) {
        m_ModuleManager = new ModuleManager(this);
        m_ModuleManager->Initialize();
    }

    PyList* result = new PyList();
    PyTuple* module = new PyTuple(2);
    // Create entries in "onslimitemchange" modules list for ALL modules, rigs, and subsystems present on ship:
    std::vector<InventoryItemRef> moduleList;
    m_ModuleManager->GetModuleListOfRefsAsc(moduleList);
    for (int i=0; i<moduleList.size(); ++i) {
        module->SetItem(0, new PyInt(moduleList.at(i)->typeID()));
        module->SetItem(1, new PyInt(moduleList.at(i)->itemID()));
        result->AddItem(module);
    }

    return result;
}

PyDict* ShipItem::GetChargeState() {
    /*  this is correct */
    if (!pInventory->ContentsLoaded()) {
        if (!pInventory->LoadContents()) {
            _log(INV__ERROR, "%s(%u): Failed to load contents for GetShipState", itemName().c_str(), itemID());
            return nullptr;
        }
    }
    if (m_ModuleManager == nullptr) {
        m_ModuleManager = new ModuleManager(this);
        m_ModuleManager->Initialize();
    }

    /* get list of charges loaded in ship modules (*all slots*) */
    std::map< EVEItemFlags, InventoryItemRef > charges;
    m_ModuleManager->GetLoadedCharges(charges);

    if (charges.empty())
        return new PyDict();

    // Create entries in "shipState" dictionary for loaded charges on ship:
    PyDict* chargeDict = new PyDict();
    for (auto cur : charges)
        chargeDict->SetItem(new PyInt((uint32)cur.first), cur.second->GetChargeStatusRow(itemID()));

    PyDict *result = new PyDict();
    result->SetItem(new PyInt(itemID()), chargeDict);
    return result;
}

void ShipItem::AddItem(InventoryItemRef iRef)
{
    if (IsModuleSlot(iRef->flag()) and (iRef->categoryID() != EVEDB::invCategories::Charge)) {
        // make singleton
        iRef->ChangeSingleton( true );
    }
    pInventory->AddItem( iRef );
}

bool ShipItem::ValidateBoardShip(CharacterRef character) {

    bool result = false;
    EvilNumber skillTypeID = 0;

    if (HasAttribute(AttrRequiredSkill1, skillTypeID)) {
        if (character->HasSkillTrainedToLevel( skillTypeID.get_int(), GetAttribute(AttrRequiredSkill1Level).get_int()))
            result = true;
        if (HasAttribute(AttrRequiredSkill2, skillTypeID)) {
            if (character->HasSkillTrainedToLevel( skillTypeID.get_int(), GetAttribute(AttrRequiredSkill2Level).get_int())) {
                result = true;
            } else {
                return false;
            }
            if (HasAttribute(AttrRequiredSkill3, skillTypeID)) {
                if (character->HasSkillTrainedToLevel( skillTypeID.get_int(), GetAttribute(AttrRequiredSkill3Level).get_int())) {
                    result = true;
                } else {
                    return false;
                }
                if (HasAttribute(AttrRequiredSkill4, skillTypeID)) {
                    if (character->HasSkillTrainedToLevel( skillTypeID.get_int(), GetAttribute(AttrRequiredSkill4Level).get_int())) {
                        result = true;
                    } else {
                        return false;
                    }
                    if (HasAttribute(AttrRequiredSkill5, skillTypeID)) {
                        if (character->HasSkillTrainedToLevel( skillTypeID.get_int(), GetAttribute(AttrRequiredSkill5Level).get_int())) {
                            result = true;
                        } else {
                            return false;
                        }
                        if (HasAttribute(AttrRequiredSkill6, skillTypeID)) {
                            if (character->HasSkillTrainedToLevel( skillTypeID.get_int(), GetAttribute(AttrRequiredSkill6Level).get_int())) {
                                result = true;
                            } else {
                                return false;
                            }
                        }
                    }
                }
            }
        }
    } else {
        result = true;
    }

    return result;
}

void ShipItem::SaveShip()
{
    SaveItem();                         // Save ship info
    pAttributeMap->SaveShipState();      // save ship damage
    if (m_ModuleManager != nullptr)
        m_ModuleManager->SaveModules();     // Save item info for modules fitted to this ship
}

bool ShipItem::ValidateItemSpecifics(InventoryItemRef iRef)
{
    bool result = false;
    EvilNumber fitID = 0;
    uint16 groupID = m_pilot->GetShip()->groupID();
    // If a ship group restriction is specified, the item must be able to fit to at least one ship group.
    _log(SHIP__TRACE, "Ship::ValidateItemSpecifics - Beginning the group validation for %s(%u):", iRef->itemName().c_str(), iRef->itemID());
    if (iRef->HasAttribute(AttrCanFitShipGroup1, fitID)) {
        if (fitID == groupID)
            result = true;
        if (iRef->HasAttribute(AttrCanFitShipGroup2, fitID)) {
            if (fitID == groupID)
                result = true;
            if (iRef->HasAttribute(AttrCanFitShipGroup3, fitID)) {
                if (fitID == groupID)
                    result = true;
                if (iRef->HasAttribute(AttrCanFitShipGroup4, fitID)) {
                    if (fitID == groupID)
                        result = true;
                    if (iRef->HasAttribute(AttrCanFitShipGroup5, fitID)) {
                        if (fitID == groupID)
                            result = true;
                        if (iRef->HasAttribute(AttrCanFitShipGroup6, fitID)) {
                            if (fitID == groupID)
                                result = true;
                            if (iRef->HasAttribute(AttrCanFitShipGroup7, fitID)) {
                                if (fitID == groupID)
                                    result = true;
                                if (iRef->HasAttribute(AttrCanFitShipGroup8, fitID)) {
                                    if (fitID == groupID)
                                        result = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    } else {
        result = true;
    }

    _log(SHIP__TRACE, "Ship::ValidateItemSpecifics - Group Validation returning %s.", (result ? "true" : "false"));
    _log(SHIP__TRACE, "Ship::ValidateItemSpecifics - Beginning the type validation for %s(%u):", iRef->itemName().c_str(), iRef->itemID());

    uint16 typeID = m_pilot->GetShip()->typeID();
    if (iRef->HasAttribute(AttrCanFitShipType1, fitID)) {
        result = false;
        if (fitID == groupID)
            result = true;
        if (iRef->HasAttribute(AttrCanFitShipType2, fitID)) {
            if (fitID == groupID)
                result = true;
            if (iRef->HasAttribute(AttrCanFitShipType3, fitID)) {
                if (fitID == groupID)
                    result = true;
                if (iRef->HasAttribute(AttrCanFitShipType4, fitID)) {
                    if (fitID == groupID)
                        result = true;
                }
            }
        }
    } else {
        result = true;
    }

    _log(SHIP__TRACE, "Ship::ValidateItemSpecifics - Type Validation returning %s.", (result ? "true" : "false"));
    return result;
}

void ShipItem::ProcessModules() {
    if (m_pilot->IsDocked())
        return;
    if (m_ModuleManager == nullptr){
        _log(SHIP__MODULE_ERROR, "ProcessModules() - %s(%u) has no module manager.", itemName().c_str(), itemID());
        EvE::traceStack();
        return;
    }

    m_ModuleManager->Process();
}

void ShipItem::Dock() {
    m_isDocking = true;
    DeactivateAllModules();
    m_onlineModuleVec.clear();
}

void ShipItem::Undock() {
    // apply ship effects, as all variables are set at this point.
    if (m_ModuleManager != nullptr) {
        // this is hacked to reset ship effects, as *something* isnt working right...
        m_ModuleManager->OfflineAll();
        UpdateEffects();
        //ClearModifiers();
        //ProcessEffects(true, IsSolarSystem(m_locationID));
        UpdateModules(); //FailedToOnlineModulesOnUndock
    } else {
        _log(SHIP__MODULE_ERROR, "Undock() - %s(%u) has no module manager.", itemName().c_str(), itemID());
        EvE::traceStack();
    }

    if (sConfig.debug.IsTestServer) {
        // Heal Ship completely on test server
        Heal();
    } else {
        // live server will Recharge shields and cap if session change isnt active (undocking too fast)
        if (!m_pilot->IsSessionChange()) {
            SetShipShield(1.0);
            SetShipCapacitorLevel(1.0);
        }
    }
}

void ShipItem::Warp() {
    if (m_ModuleManager != nullptr)
        m_ModuleManager->ShipWarping();
    else {
        _log(SHIP__MODULE_ERROR, "Warp() - %s(%u) has no module manager.", itemName().c_str(), itemID());
        EvE::traceStack();
    }
}

void ShipItem::Jump() {
    if (m_ModuleManager != nullptr)
        m_ModuleManager->ShipJumping();
    else {
        _log(SHIP__MODULE_ERROR, "Jump() - %s(%u) has no module manager.", itemName().c_str(), itemID());
        EvE::traceStack();
    }
}

void ShipItem::Heal()
{
    // Heal Ship and Fully Recharge Capacitor:
    SetShipShield(1.0);
    SetShipCapacitorLevel(1.0);
    SetShipArmor(1.0);
    SetShipHull(1.0);
}

void ShipItem::AddModuleToOnlineVec(uint32 moduleID)
{
    m_onlineModuleVec.push_back(moduleID);
    _log(SHIP__MODULE_INFO, "Added ModuleID %u to Online List", moduleID);
}

//  Updated fractional ship defense settings.  -allan 1Feb15
void ShipItem::SetShipCapacitorLevel(double fraction)
{
    if ( fraction > 1.0 ) fraction = 1.0;
    if ( fraction < 0.0 ) fraction = 0.0;

    EvilNumber newCapacitorCharge = 0.0;
    newCapacitorCharge = GetAttribute(AttrCapacitorCapacity) * fraction;
    if ( (newCapacitorCharge + 0.5) > GetAttribute(AttrCapacitorCapacity))
        newCapacitorCharge = GetAttribute(AttrCapacitorCapacity);
    if ( (newCapacitorCharge - 0.5) < 0 )
        newCapacitorCharge = 0;

    SetAttribute(AttrCapacitorCharge, newCapacitorCharge);
}

void ShipItem::SetShipShield(double fraction)
{
    if ( fraction > 1.0 ) fraction = 1.0;
    if ( fraction < 0.0 ) fraction = 0.0;

    EvilNumber newShieldCharge = 0.0;
    newShieldCharge = GetAttribute(AttrShieldCapacity) * fraction;
    if ( (newShieldCharge + 0.2) > GetAttribute(AttrShieldCapacity))
        newShieldCharge = GetAttribute(AttrShieldCapacity);
    if ( (newShieldCharge - 0.2) < 0 )
        newShieldCharge = 0;

    SetAttribute(AttrShieldCharge, newShieldCharge);
}

void ShipItem::SetShipArmor(double fraction)
{
    fraction = 1 - fraction;

    if ( fraction > 1.0 ) fraction = 1.0;
    if ( fraction < 0.0 ) fraction = 0.0;

    EvilNumber newArmorDamage = 0.0;
    newArmorDamage = GetAttribute(AttrArmorHP) * fraction;
    if ( (newArmorDamage + 0.2) > GetAttribute(AttrArmorHP))
        newArmorDamage = GetAttribute(AttrArmorHP);
    if ( (newArmorDamage - 0.2) < 0 )
        newArmorDamage = 0;

    SetAttribute(AttrArmorDamage, newArmorDamage);
}

void ShipItem::SetShipHull(double fraction)
{
    fraction = 1 - fraction;

    if ( fraction > 1.0 ) fraction = 1.0;
    if ( fraction < 0.0 ) fraction = 0.0;

    EvilNumber newHullDamage = 0.0;
    newHullDamage = GetAttribute(AttrHP) * fraction;
    if ( (newHullDamage + 0.2) > GetAttribute(AttrHP))
        newHullDamage = GetAttribute(AttrHP);
    if ( (newHullDamage - 0.2) < 0 )
        newHullDamage = 0;

    SetAttribute(AttrDamage, newHullDamage);
}

/* Begin new Module Manager Interface */
void ShipItem::GetModuleRefVec(std::vector< InventoryItemRef >& iRefVec)
{
    if (m_ModuleManager != nullptr)
        m_ModuleManager->GetModuleListOfRefsAsc(iRefVec);
}

InventoryItemRef ShipItem::GetModuleRef(EVEItemFlags flag)
{
    if ((m_ModuleManager != nullptr) and (m_ModuleManager->GetModule(flag) != nullptr) )
		return (m_ModuleManager->GetModule(flag))->GetSelf();
	else
        return InventoryItemRef(nullptr);
}

InventoryItemRef ShipItem::GetModuleRef(uint32 modID)
{
    if ((m_ModuleManager != nullptr) and (m_ModuleManager->GetModule(modID) != nullptr) )
		return (m_ModuleManager->GetModule(modID))->GetSelf();
	else
        return InventoryItemRef(nullptr);
}

void ShipItem::TryHoldCapacity(EVEItemFlags flag, InventoryItemRef iRef)
{
    // Handle any flag, legal or not, by virtue of GetRemainingVolumeByFlag() and GetCapacity() that handle supported capacity types:
    // (unsupported or illegal flags report capacity of 0.0, so are automatically rejected)
    // check for adding unpackaged ships to cargo of active ship...
    double volume = iRef->GetPackagedVolume();
    volume *= iRef->quantity();
    double capacity = GetRemainingVolumeByFlag(flag);
    if (capacity < volume) {
        std::map<std::string, PyRep *> args;
        args["available"] = new PyFloat(capacity);
        args["volume"] = new PyFloat(volume);
        throw PyException( MakeUserError( "NotEnoughCargoSpace", args));
    }
}

void ShipItem::TryModuleLimitChecks(EVEItemFlags flag, InventoryItemRef iRef)
{
    if (m_ModuleManager->IsSlotOccupied(flag))
        throw PyException( MakeUserError( "SlotAlreadyOccupied" ));

    m_ModuleManager->CheckSlotFitLimited(flag, iRef);
    m_ModuleManager->CheckGroupFitLimited(flag, iRef);

    if (IsHiSlot(flag)) {
        // check avalible turret/launcher slots
        if (iRef->type().HasEffect(EVEEffectID::turretFitted)) {
            if (GetAttribute(AttrTurretSlotsLeft) < 1) {
                std::map<std::string, PyRep *> args;
                args["moduleName"] = new PyString(iRef->itemName());
                throw PyException( MakeUserError("NotEnoughTurretSlots", args));
                /*u'NotEnoughTurretSlotsBody'}(u"You cannot fit the {moduleName} because your ship doesn't have any turret slots left for fitting, possibly because you have already filled your ship with turrets or that the ship simply can not be fitted with turrets.\r\n<br>
                 * <br>Turret slots represent how many weapons of a certain type can be fitted on a ship. The current design is over a hundred years old, and is modular enough to allow for a great leeway in the fitting of various weaponry.", None,
                 * {u'{moduleName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'moduleName'}})
                 */
            }
        } else if (iRef->type().HasEffect(EVEEffectID::launcherFitted)) {
            if (GetAttribute(AttrLauncherSlotsLeft) < 1) {
                std::map<std::string, PyRep *> args;
                args["moduleName"] = new PyString(iRef->itemName());
                throw PyException( MakeUserError("NotEnoughLauncherSlots", args));
                /*NotEnoughLauncherSlotsBody'}(u"You cannot fit the {moduleName} because your ship doesn't have any launcher slots left for fitting, possibly because you have already filled your ship with launchers or that the ship simply can not be fitted with launchers.<br>
                 * <br>Launcher slots represent how many weapons of a certain type can be fitted on a ship. The current design is over a hundred years old, and is modular enough to allow for a great leeway in the fitting of various weaponry.", None,
                 * {u'{moduleName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'moduleName'}})
                 */
            }
        }
    } else if (IsRigSlot(flag)) {
        if (GetAttribute(AttrRigSize) != iRef->GetAttribute(AttrRigSize)) {
            std::map<std::string, PyRep *> args;
            args["rigSize"] = new PyString(sDataMgr.GetRigSizeName(iRef->GetAttribute(AttrRigSize).get_int()));
            args["item"] = new PyString(iRef->itemName());
            args["shipRigSize"] = new PyString(sDataMgr.GetRigSizeName(GetAttribute(AttrRigSize).get_int()));
            throw PyException( MakeUserError("CannotFitRigWrongSize", args));
            /* CannotFitRigWrongSizeBody'}(u'{item} does not fit in this slot.
             * The slot takes size {shipRigSize} rigs, but the item is size {rigSize}.', None,
             * {u'{rigSize}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'rigSize'},
             * u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'},
             * u'{shipRigSize}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'shipRigSize'}})
             * check avalible rig slots and ship upgrade capy
             */
        }
        if (GetAttribute(AttrUpgradeSlotsLeft) < 1) {
            std::map<std::string, PyRep *> args;
            args["moduleType"] = new PyString(iRef->type().name());
            throw PyException( MakeUserError("NotEnoughUpgradeSlots", args));
            /*NotEnoughUpgradeSlotsBody'}(u"You cannot fit the {[item]moduleType.name} because your ship doesn't have any upgrade slots left for fitting, possibly because you have already filled your ship with upgrades or that the ship simply can not be fitted with upgrades.", None,
             * {u'{[item]moduleType.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleType'}})
             */
        }

        if ((GetAttribute(AttrUpgradeLoad) + iRef->GetAttribute(AttrUpgradeCost)) > GetAttribute(AttrUpgradeCapacity)) {
            std::map<std::string, PyRep *> args;
            args["moduleName"] = new PyString(iRef->itemName());
            throw PyException( MakeUserError("NotEnoughUpgradeCapacity", args));
            /*NotEnoughUpgradeCapacityBody'}(u'You cannot fit the {moduleName} because your ship cannot handle it. Your ship can only fit so many upgrades as each interferes with its calibration, and past a certain point your ship is rendered unusable.', None,
             * {u'{moduleName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'moduleName'}})
             */
        }
    }
}

EVEItemFlags ShipItem::FindAvailableModuleSlot(InventoryItemRef iRef) {
    // CantFitModuleToThatShip
    // u'CantFitModuleToThatShipBody'}(u"You can't fit {item} to {ship}", None, {u'{ship}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'ship'}, u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}})
    uint16 slotFound = flagIllegal;
    if (iRef->type().HasEffect(EVEEffectID::loPower)) {
        slotFound = m_ModuleManager->GetAvailableSlotInBank(EVEEffectID::loPower);
    } else if (iRef->type().HasEffect(EVEEffectID::medPower)) {
        slotFound = m_ModuleManager->GetAvailableSlotInBank(EVEEffectID::medPower);
    } else if (iRef->type().HasEffect(EVEEffectID::hiPower)) {
        slotFound = m_ModuleManager->GetAvailableSlotInBank(EVEEffectID::hiPower);
    } else if (iRef->type().HasEffect(EVEEffectID::subSystem)) {
        slotFound = m_ModuleManager->GetAvailableSlotInBank(EVEEffectID::subSystem);
    } else if (iRef->type().HasEffect(EVEEffectID::rigSlot)) {
        slotFound = m_ModuleManager->GetAvailableSlotInBank(EVEEffectID::rigSlot);
    } else {
        // ERROR: This is not a module that fits in any of the slot banks
        codelog(SHIP__ERROR, "ShipItem::FindAvailableModuleSlot() - iRef %s is not module type.", iRef->itemName().c_str());
    }

    return (EVEItemFlags)slotFound;
}

void ShipItem::LoadCharge(EVEItemFlags flag, InventoryItemRef iRef)
{
    if (iRef.get() == nullptr)
        return;  // make error here?

    if (ValidateAddItem(flag, iRef))
        m_ModuleManager->LoadCharge(iRef, flag);
}

uint32 ShipItem::AddItem(EVEItemFlags flag, InventoryItemRef iRef, Client* pClient/*nullptr*/)
{
    if (flag == flagAutoFit) {
        // make error.  nothing at this point should be "autoFit"
        codelog(SHIP__ERROR, "ShipItem::AddItem() - old_flag = flagAutoFit.");
        if (sConfig.debug.IsTestServer)
            EvE::traceStack();
    }

    if (!ValidateAddItem(flag, iRef, pClient))
        return 0;

    if (IsModuleSlot(flag)) {
        if (m_ModuleManager == nullptr) {
            _log(SHIP__MODULE_ERROR, "Ship::AddItem - %u - m_ModuleManager is null.", m_itemID );
            return 0;
        }
        //  need to verify if this is needed.  comment for now.
        //iRef->ClearModifiers();
        if (iRef->categoryID() == EVEDB::invCategories::Charge) {
            m_ModuleManager->LoadCharge(iRef, flag);
            InventoryItemRef loadedChargeOnModule = m_ModuleManager->GetLoadedChargeOnModule(flag);
            if (loadedChargeOnModule.get() != nullptr)
                return loadedChargeOnModule->itemID();
            else
                return 0;
        } else if (iRef->categoryID() == EVEDB::invCategories::Module) {
            iRef->ChangeSingleton(true, false);
            // rigs are classed in the module category.  check here and call approprate method as needed.
            if (IsRigSlot(flag)) {
                if (!m_ModuleManager->InstallRig(iRef, flag))
                    return 0;
            } else if (!m_ModuleManager->FitModule(iRef, flag))
                return 0;
        } else if (iRef->categoryID() == EVEDB::invCategories::Subsystem) {
            iRef->PutOffline();
            iRef->ChangeSingleton(true, false);
            if (!m_ModuleManager->InstallSubSystem(iRef, flag))
                return 0;
        }
        m_ModuleManager->UpdateModules(flag);
    }

    iRef->Move(m_itemID, flag, true);

	return iRef->itemID();
}

void ShipItem::RemoveItem(InventoryItemRef iRef)
{
    if (iRef.get() == nullptr)
        return;

    pInventory->RemoveItem(iRef);

    if (m_pilot == nullptr)
        return;

    // check to see if item is currently in a module slot.  going by category is NOT working after _ExecAdd() updates.
    if (IsModuleSlot(iRef->flag())) {
        if (m_ModuleManager == nullptr) {
            m_ModuleManager = new ModuleManager(this);
            m_ModuleManager->Initialize();
        }
        //iRef->ClearModifiers();
        // if item being removed is in a module slot, remove it via Module Manager here, and let invBound take care of the rest.
        if (iRef->categoryID() == EVEDB::invCategories::Charge) {
            m_ModuleManager->UnloadCharge(iRef->flag());
        } else if ((iRef->categoryID() == EVEDB::invCategories::Module) or (iRef->categoryID() == EVEDB::invCategories::Subsystem)) {
            if (IsRigSlot(iRef->flag()))
                m_ModuleManager->UninstallRig(iRef->itemID());
            else
                m_ModuleManager->UnfitModule(iRef->itemID());
        }
        /*
        if ((m_pilot != nullptr) and (m_pilot->IsInSpace()))
            UpdateEffects();
        */
        m_ModuleManager->UpdateModules(iRef->flag());
    }
}

uint32 ShipItem::RemoveCharge(EVEItemFlags fromFlag, EVEItemFlags toFlag)
{
    if (IsModuleSlot(fromFlag)) {
        if (m_ModuleManager == nullptr) {
            m_ModuleManager = new ModuleManager(this);
            m_ModuleManager->Initialize();
        }
        InventoryItemRef chargeRef = m_ModuleManager->GetModule(fromFlag)->GetLoadedChargeRef();
        m_ModuleManager->UnloadCharge(fromFlag);
        if (m_pilot->IsInSpace())
            chargeRef->Move(itemID(), toFlag, true);
        else
            chargeRef->Move(locationID(), toFlag, true);
        return chargeRef->itemID();
    } 
    return 0;
}


void ShipItem::MoveModuleSlot(EVEItemFlags slot1, EVEItemFlags slot2) {
    // slot1 is occupied, as this is location module is from.
    InventoryItemRef modItemRef1 = GetModuleRef(slot1);
    if (modItemRef1.get() == nullptr) {
        _log(SHIP__MODULE_TRACE, "Ship::MoveModuleSlot - modItemRef1 is null." );
        m_pilot->SendNotifyMsg("There was an internal error.  The module to move was not found.");
        return;
    }
    InventoryItemRef chargeItemRef1 = m_ModuleManager->GetLoadedChargeOnModule(slot1);
    if (chargeItemRef1.get() != nullptr)
        m_ModuleManager->UnloadCharge(slot1);
    RemoveItem(modItemRef1);

    if (m_ModuleManager->IsSlotOccupied(slot2)) {
        // dropped-on slot is occupied.  procede with moving the module currently in this slot.
        InventoryItemRef modItemRef2 = GetModuleRef(slot2);
        InventoryItemRef chargeItemRef2 = m_ModuleManager->GetLoadedChargeOnModule(slot2);
        if (chargeItemRef2.get() != nullptr)
            m_ModuleManager->UnloadCharge(slot2);
        RemoveItem(modItemRef2);

        AddItem(slot1, modItemRef2);
        if (chargeItemRef2.get() != nullptr)
            m_ModuleManager->LoadCharge(chargeItemRef2, slot1);
    }

    AddItem(slot2, modItemRef1);
    if (chargeItemRef1.get() != nullptr)
        m_ModuleManager->LoadCharge(chargeItemRef1, slot2);

    UpdateModules(slot1);
}

void ShipItem::UpdateModules()
{
    /* this is only called when ship is in space
     * this will call Online() on all modules, which will apply passive and online effects.
     */
    m_ModuleManager->UpdateModules(m_onlineModuleVec);
    m_onlineModuleVec.clear();
}

void ShipItem::UpdateModules(EVEItemFlags flag)
{
	// List of callees to put this function into context as to what it should be doing:
    // Ship::AddItem()
    // Ship::MoveModuleSlot()
    // Client::MoveItem()               - something has been moved into or out of the ship, recheck all modules for... some reason
    m_ModuleManager->UpdateModules(flag);
    /*
    if ((m_pilot != nullptr) and (m_pilot->IsInSpace()))
        UpdateEffects();
    */
}

void ShipItem::UnloadModule(uint32 itemID)
{
    m_ModuleManager->UnfitModule(itemID);
}

void ShipItem::UnloadAllModules()
{
    m_ModuleManager->UnloadAllModules();
}

// not used
void ShipItem::RepairShip(float fraction)
{
    if (fraction > 1)
        fraction = 1;

    if (fraction == 1) {
         SetAttribute(AttrDamage, EvilZero);
         SetAttribute(AttrArmorDamage, EvilZero);
        return;
    }

    uint32 cHull =  GetAttribute(AttrDamage).get_int();
    uint32 cArmor =  GetAttribute(AttrArmorDamage).get_int();
    uint32 damage = cHull + cArmor;
    EvilNumber amount = damage * fraction;
    // this will repair hull first, then armor
    if (amount > cHull) {
        amount -= cHull;
         SetAttribute(AttrDamage, EvilZero);
        if (amount >= cArmor) {
             SetAttribute(AttrArmorDamage, EvilZero);
        } else {
            amount = cArmor - amount;
             SetAttribute(AttrArmorDamage, amount);
        }
    } else
         SetAttribute(AttrDamage, amount);

}

// not used
void ShipItem::RepairModules(std::vector<InventoryItemRef>& itemRefVec, float fraction)
{
    /** @todo  this isnt right....needs update */
    EvilNumber amount = 0, damage = 0;
    for (auto cur : itemRefVec) {
        damage = cur->GetAttribute(AttrDamage);
        if (damage < 0.01)
            continue;
        amount = cur->GetAttribute(AttrDamage);
        if ((amount / cur->GetAttribute(AttrHP)) > fraction)
            amount = cur->GetAttribute(AttrHP) *  fraction;
        else
            amount = 1;
        m_ModuleManager->RepairModule(cur->itemID(), amount);
    }
}

void ShipItem::Online (uint32 moduleID)
{
    if (IsSolarSystem(m_locationID)) {
        ; // check for avalible cap, and drain accordingly
        /*
        float Charge = GetAttribute(AttrCapacitorCharge).get_float();
        float Capacity = GetAttribute(AttrCapacitorCapacity).get_float();
        float newCharge = 0;
        SetAttribute(AttrCapacitorCharge, newCharge);
        _log(SHIP__MESSAGE, "ShipItem::Online(): %s(%u) - New Cap Charge: %f", GetPilot()->GetName(), itemID(), newCharge );
        */
    }
	m_ModuleManager->Online(moduleID);
}

void ShipItem::Offline (uint32 moduleID)
{
	m_ModuleManager->Offline(moduleID);
}

void ShipItem::Activate(int32 itemID, std::string effectName, int32 targetID, int32 repeat)
{
    if (IsValidTarget(targetID))
        m_targetRef = sItemFactory.GetItem(targetID);
    else
        m_targetRef = InventoryItemRef(nullptr);

    m_ModuleManager->Activate( itemID, sFxDataMgr.GetEffectID(effectName), targetID, repeat );
}

void ShipItem::Deactivate(int32 itemID, std::string effectName)
{
    m_ModuleManager->Deactivate(itemID, effectName);
}

void ShipItem::Overload()
{
    // FIXME TODO get module flag(s) and send to function
    EVEItemFlags flag = flagNone;
    m_ModuleManager->Overload(flag);
}

void ShipItem::CancelOverloading()
{
    // FIXME TODO get module flag(s) and send to function
    EVEItemFlags flag = flagNone;
    m_ModuleManager->DeOverload(flag);
}

void ShipItem::RemoveRig(InventoryItemRef iRef) {
    //may not look like it, but just moving this item will call ModuleManager::UninstallRig().  not anymore.  fix this shit.
    m_ModuleManager->UninstallRig(iRef->itemID());
    iRef->Move(itemID(), flagCargoHold, true);
}

void ShipItem::OnlineAll()
{
    if (m_ModuleManager != nullptr)
        m_ModuleManager->OnlineAll();
    else {
        _log(SHIP__MODULE_ERROR, "OnlineAll() - %s(%u) has no module manager.", itemName().c_str(), itemID());
        EvE::traceStack();
    }
}

void ShipItem::OfflineAll()
{
    if (m_ModuleManager != nullptr)
        m_ModuleManager->OfflineAll();
    else {
        _log(SHIP__MODULE_ERROR, "OfflineAll() - %s(%u) has no module manager.", itemName().c_str(), itemID());
        EvE::traceStack();
    }
}

void ShipItem::ReplaceCharges(EVEItemFlags flag, InventoryItemRef newCharge)
{
    _log(SHIP__MODULE_ERROR, "ReplaceCharges() called by %s(%u).  It still needs to be written.", itemName().c_str(), itemID());
}

void ShipItem::DeactivateAllModules()
{
    if (m_ModuleManager != nullptr)
        m_ModuleManager->DeactivateAllModules();
}
/* End new Module Manager Interface */

void ShipItem::StripFitting()
{
    if (m_ModuleManager != nullptr) {
        std::vector<InventoryItemRef> moduleList;
        m_ModuleManager->GetModuleListOfRefsAsc(moduleList);
        for (auto cur : moduleList) {
            m_ModuleManager->UnfitModule(cur->itemID());
            cur->Move(m_pilot->GetLocationID(), flagHangar);
        }
    } else {
        _log(SHIP__MODULE_ERROR, "StripFitting() - %s(%u) has no module manager.", itemName().c_str(), itemID());
        EvE::traceStack();
    }
}

void ShipItem::LinkWeapon(uint32 masterID, uint32 slaveID)
{
    std::map<uint32, std::list<uint32>>::iterator itr = m_linkedWeapons.find(masterID);
    if (itr != m_linkedWeapons.end()) {
        itr->second.push_back(slaveID);
    } else {
        std::list<uint32> slaves;
        slaves.push_back(slaveID);
        m_linkedWeapons[masterID] = slaves;
    }
}

void ShipItem::LinkAllWeapons()
{
    std::vector< InventoryItemRef > moduleVec;
    m_ModuleManager->GetWeapons(moduleVec);
    /* check weapon types and charge types.
     * if types same, then link weapons
     */
}

void ShipItem::UnlinkWeapon(uint32 masterID, uint32 slaveID)
{
    std::map<uint32, std::list<uint32>>::iterator itr = m_linkedWeapons.find(masterID);
    if (itr != m_linkedWeapons.end()) {
        std::list<uint32>::iterator itr2 = itr->second.begin();
        while (itr2 != itr->second.end()) {
            if ((*itr2) == slaveID) {
                itr->second.erase(itr2);
                return;
            }
            ++itr2;
        }
    }
}

PyList* ShipItem::GetLinkedWeapons()
{
    PyList* result = new PyList();
    if (!m_linkedWeapons.empty()) {
        for (auto cur : m_linkedWeapons) {
            PyList* slaves = new PyList();
            for (auto slave : cur.second)
                slaves->AddItem(new PyInt(slave));
            PyTuple* master = new PyTuple(2);
            master->SetItem(0, new PyInt(cur.first));
            master->SetItem(1, slaves);
            result->AddItem(master);
        }
    }

    if (is_log_enabled(SHIP__MODULE_MESSAGE))
        result->Dump(SHIP__MODULE_MESSAGE, "    ");
    return result;
}

// new effects system.  wip
void ShipItem::ProcessEffects(bool add/*false*/, bool update/*false*/)
{
    /*
    Effects processing order...
        boosters   //char effect
        Implants   //char effect
        skills     //char effect
        Ship       //ship effect
        Subsystem  //module effect
        Rigs       //module effect
        Low        //module effect
        Mid        //module effect
        Hi         //module effect
    */
    if (add) {
        double start = GetTimeMSeconds();
        // char effects are processed when char is loaded.
        // apply char effects
        m_pilot->GetChar()->ResetModifiers();
        _log(EFFECTS__TRACE, "ShipItem::ProcessEffects() - Processing Char Effects");
        m_pilot->GetChar()->ProcessEffects();
        _log(EFFECTS__TRACE, "ShipItem::ProcessEffects() - Applying Char Effects");
        sFxProc.ApplyEffects(m_pilot->GetChar().get(), m_pilot->GetChar().get(), this, update);
        ProcessShipEffects(update);
        _log(EFFECTS__DEBUG, "ShipItem::ProcessEffects() - %u ship and char effects processed and applied in %.3fms", \
                (m_pilot->GetChar()->m_modifiers.size() + m_modifiers.size()), (GetTimeMSeconds() - start));
    } else {
        RemoveEffects();
    }
}

void ShipItem::ProcessShipEffects(bool update/*false*/)
{
    _log(EFFECTS__TRACE, "ShipItem::ProcessEffects():  Processing Ship Effects Processing.");
    fxData data;
    data.action = Effects::Action::dgmActInvalid;
    for (auto it : m_type.m_stateFxMap) {
        data.srcRef = static_cast<InventoryItemRef>(this);
        data.math = data.targLoc = data.targAttr = data.srcAttr = data.grpID = data.typeID = data.fxSrc = 0;
        sFxProc.ParseExpression(this, sFxDataMgr.GetExpression(it.second.preExpression), data);
    }
    _log(EFFECTS__TRACE, "Applying Ship Effects");
    // apply processed effects
    sFxProc.ApplyEffects(this, m_pilot->GetChar().get(), this, update);
}

void ShipItem::RemoveEffects()
{
    SaveShip();
    // clear also reloads default attribs
    ClearModifiers();
}

void ShipItem::UpdateEffects() {
    double start = GetTimeMSeconds();
    RemoveEffects();
    ProcessEffects(true, IsSolarSystem(m_locationID));
    _log(EFFECTS__DEBUG, "ShipItem::UpdateEffects() - Effects updated in %.3fms", (GetTimeMSeconds() - start));
}

std::string ShipItem::GetShipDNA()
{
    if (m_ModuleManager == nullptr) {
        _log(SHIP__MODULE_ERROR, "GetShipDNA() - %s(%u) has no module manager.", itemName().c_str(), itemID());
        EvE::traceStack();
    }

    /* ship dna is shorthand notation to describe a ship and it's fittings purely thru the use of typeIDs and quantities
     *
     * the format is as follows:
     * <shipTypeID>:
     *      <subsystemID>:
     *      <moduleType-Highslot>;<quantity>:
     *      <moduleType-Midslot>;<quantity>:
     *      <moduleType-Lowslot>;<quantity>:
     *      <moduleType-Rigslot>;<quantity>:
     *      <chargeType>;<quantity>:
     *      <droneType>;<quantity>
     *
     * Condensed version:
     *  Ship:Subsystem:Highs:Mids:Lows:Rigs:Charges:Drones
     *
     * [PyString "<url=fitting:24698:3841;2:2531;1:19812;1:23527;1:2410;7:1422;4:2547;1:31802;3:2301;1:2454;5::>Anchor</url>"]
     *
     *  current code returns this:
     * "587:8863;1:8863;1:8863;1:499;1:578;1:1798;1:6485;1:2046;1:8325;1:31788;1:31800;1:31788;1::"
     *  need to figure out how to group modules for correct condensed counts
     */
    if (type().id() == EVEDB::invTypes::typeCapsule) {
        std::stringstream dna;
        dna << type().id() << ":";
        _log(SHIP__INFO, "ShipDNA has compiled DNA of \"%s\" for %s(%u) ", dna.str().c_str(), itemName().c_str(), itemID());
        return dna.str();
    }

    /* find and encode the module typeIDs */
    std::stringstream modHi, modMid, modLow, subSys, modRig, charges, drones;

    std::vector<InventoryItemRef> moduleList;
    m_ModuleManager->GetModuleListOfRefsAsc(moduleList);

    for (auto cur : moduleList) {
        if (IsRigSlot(cur->flag()))
            modRig << cur->typeID() << ";" << cur->quantity() << ":";
        else if (IsHiSlot(cur->flag()))
            modHi << cur->typeID() << ";" << cur->quantity() << ":";
        else if (IsMidSlot(cur->flag()))
            modMid << cur->typeID() << ";" << cur->quantity() << ":";
        else if (IsLowSlot(cur->flag()))
            modLow << cur->typeID() << ";" << cur->quantity() << ":";
        else if (IsSubSystem(cur->flag()))
            subSys << cur->typeID() << ":";
        else
            ; // error?
    }

    std::map<EVEItemFlags, InventoryItemRef> chargeList;
    m_ModuleManager->GetLoadedCharges(chargeList);
    for (auto cur : chargeList)
        charges << cur.second->typeID() << ";" << cur.second->quantity() << ":";

    /* not sure how to get drones yet.  will work on later */
    drones << ":";

    /* build the dna stream */
    std::stringstream dna;
    dna << type().id() << ":";
    dna << subSys.str() << modHi.str() << modMid.str() << modLow.str() << modRig.str() << charges.str() << drones.str();

    _log(SHIP__INFO, "ShipDNA has compiled DNA of \"%s\" for %s(%u) ", dna.str().c_str(), itemName().c_str(), itemID());
    return dna.str();
}


/* DynamicSystemEntity representing ship object in space */
Ship::Ship(InventoryItemRef self, PyServiceMgr &services, SystemManager* pSystem, const FactionData& data)
: DynamicSystemEntity(self, services, pSystem),
m_shipRef(ShipItemRef::StaticCast(self)),
m_processTimerTick(SHIP_PROCESS_TICK_MS),   //5s
m_processTimer(m_processTimerTick)
{
    m_warID = data.factionID;
    m_allyID = data.allianceID;
    m_corpID = data.corporationID;
    m_ownerID = data.ownerID;

    ClearBoostData();

    m_towerPass = "";
    m_podShipID = 0;
    m_processTimer.Start(m_processTimerTick);
    _log(SHIP__INFO, "Created ShipSE %p for item %u", this, self->itemID());
}

Ship::~Ship()
{
}

double Ship::CalculateRechargeRate(double Capacity, double Current, double RechargeTimeMS)
{
    // C = Cmax * [ 1 + ( SQRT(C0/Cmax) - 1 ) * EXP((t0-t1)/tau) ] ^ 2
    // dC/dt = (SQRT(C/Cmax) - C/Cmax) * 2 * Cmax / tau
    // tau = "Cap Recharge Time" / 5.0

    // prevent divide by zero.
    RechargeTimeMS = (RechargeTimeMS < 1 ? 1 : RechargeTimeMS);
    Current = (Current < 1 ? 1 : Current);
    double Cmax = (Capacity < 1 ? 1 : Capacity);

    // tau = "cap recharge time" / 5.0
    double tau = (RechargeTimeMS / 5000.0);
    // (2*Cmax) / tau
    double Cmax2_tau = ((Cmax * 2) / tau);
    double C = Current;
    // C / Cmax
    double C_Cmax = (C / Cmax);
    // sqrt( C / Cmax )
    double sC_Cmax = sqrt(C_Cmax);
    // charge rate in Gj / sec
    return (Cmax2_tau * (sC_Cmax - C_Cmax));
}

void Ship::Process() {
    /*  Enable base call to Process Targeting and Movement  */
    SystemEntity::Process();

    // check to see if this is an empty ship, and exit if so.
    //  we're not worried about recharge and modules for empty ships (segfaults)
    /** @todo m_self is NOT being populated for non-piloted ships...check later */
    if ((!m_self) or (!m_self->HasPilot()))
        return;

    if (m_processTimer.Check()) {
        double profileStartTime = 0.0;
        if (sConfig.debug.UseProfiling)
            profileStartTime = GetTimeUSeconds();
        // shield
        double Charge = m_self->GetAttribute(AttrShieldCharge).get_float();
        double Capacity = m_self->GetAttribute(AttrShieldCapacity).get_float();
        if (Charge < Capacity) {
            double newCharge = Charge + ((m_processTimerTick /1000) * CalculateRechargeRate(Capacity, Charge, m_self->GetAttribute(AttrShieldRechargeRate).get_float()));
            if (newCharge > Capacity)
                newCharge = Capacity;
            else if ((Capacity - newCharge) < 0.3)
                newCharge = Capacity;
            m_self->SetAttribute(AttrShieldCharge, newCharge);
            _log(SHIP__MESSAGE, "Ship::Process(): %s(%u) - New Shield Charge: %f", m_self->GetPilot()->GetName(), m_self->itemID(), newCharge );
        }

        // cap
        Charge = m_self->GetAttribute(AttrCapacitorCharge).get_float();
        Capacity = m_self->GetAttribute(AttrCapacitorCapacity).get_float();
        if (Charge < Capacity) {
            double newCharge = Charge + ((m_processTimerTick /1000) * CalculateRechargeRate(Capacity, Charge, m_self->GetAttribute(AttrRechargeRate).get_float()));
            if (newCharge > Capacity)
                newCharge = Capacity;
            else if ((Capacity - newCharge) < 0.3)
                newCharge = Capacity;
            m_self->SetAttribute(AttrCapacitorCharge, newCharge);
            _log(SHIP__MESSAGE, "Ship::Process(): %s(%u) - New Cap Charge: %f", m_self->GetPilot()->GetName(), m_self->itemID(), newCharge );
        }
        // profile timer for the ship recharge shit
        if (sConfig.debug.UseProfiling)
            sProfile.AddTime(_shipProfile, GetTimeUSeconds() - profileStartTime);
    }

    // now, process the modules.
    m_shipRef->ProcessModules();
}

void Ship::DamageRandModule(float chance)
{
    if (chance == 0)
        return;
    if (chance > MakeRandomFloat())
        m_shipRef->DamageRandModule();
}

void Ship::PayInsurance() {
    std::string reason = "Insurance payment for loss of ship ";
    reason += m_self->itemName();
    AccountService::TranserFunds(ownerSCC, m_ownerID, m_db.GetShipInsurancePayout(GetSelf()->itemID()), reason, Journal::EntryType::Insurance);
    m_db.DeleteInsuranceByShipID(GetSelf()->itemID());
}

void Ship::ResetShipSystemMgr(SystemManager* pSystem)
{
    m_system = pSystem;
}

void Ship::SetPilot(Client* pClient) {
    m_self->SetPlayer(pClient);
    // set shipSE data
    m_allyID = pClient->GetAllianceID();
    m_corpID = pClient->GetCorporationID();
}

void Ship::EncodeDestiny( Buffer& into ) {
    using namespace Destiny;

    uint8 mode = DSTBALL_STOP;
    if (m_destiny->IsWarping())
        mode = DSTBALL_WARP;
    else if (m_destiny->IsFollowing())
        mode = DSTBALL_FOLLOW;
    else if (m_destiny->IsOrbiting())
        mode = DSTBALL_ORBIT;
    else if (m_destiny->IsMoving())
        mode = DSTBALL_GOTO;
/*
    NameStruct name;
        name.name = GetName();
        name.name_len = sizeof(name.name);
  */
    BallHeader head;
        head.entityID = GetID();
        head.mode = mode;
        head.radius = GetRadius();
        head.x = x();
        head.y = y();
        head.z = z();
        if (m_self->HasPilot())
            head.flags = IsInteractive | IsFree;
        else
            head.flags = IsFree;
    into.Append( head );
    MassSector mass;
        mass.mass = m_destiny->GetMass();
        mass.cloak = (m_destiny->IsCloaked() ? 1 : 0);
        mass.harmonic = m_harmonic;
        mass.corporationID = m_corpID;
        mass.allianceID = (m_allyID > 0 ? m_allyID : -1);
    into.Append( mass );
    DataSector data;
        data.inertia = m_destiny->GetInertia();
        data.maxVelocity = m_destiny->GetMaxVelocity();
        data.velocity_x = m_destiny->GetVelocity().x;
        data.velocity_y = m_destiny->GetVelocity().y;
        data.velocity_z = m_destiny->GetVelocity().z;
        data.speedfraction = m_destiny->GetSpeedFraction();
    into.Append( data );
    switch (mode) {
        case DSTBALL_WARP: {
            GPoint target = m_destiny->GetTargetPoint();
            DSTBALL_WARP_Struct warp;
                warp.formationID = 0xFF;
                warp.x = target.x;
                warp.y = target.y;
                warp.z = target.z;
                warp.ownerID = m_destiny->GetWarpSpeed();       //ship warp speed x10  (dont ask...this is what it is...more dumb ccp shit)
                // warp timing.  see Ship::EncodeDestiny() for notes/updates
                warp.effectStamp = -1; //m_destiny->GetStateStamp();   //timestamp when warp started
                warp.followRange = 0;   //this isnt right
                warp.followID = 0;  //this isnt right
            into.Append( warp );
        }  break;
        case DSTBALL_FOLLOW: {
            DSTBALL_FOLLOW_Struct follow;
                follow.followID = m_destiny->GetTargetID();
                follow.followRange = m_destiny->GetFollowDistance();
                follow.formationID = 0xFF;
            into.Append( follow );
        }  break;
        case DSTBALL_ORBIT: {
            DSTBALL_ORBIT_Struct orbit;
                orbit.followID = m_destiny->GetTargetID();
                orbit.followRange = m_destiny->GetFollowDistance();
                orbit.formationID = 0xFF;
            into.Append( orbit );
        }  break;
        case DSTBALL_GOTO: {
            GPoint target = m_destiny->GetTargetPoint();
            DSTBALL_GOTO_Struct go;
                go.formationID = 0xFF;
                go.x = target.x;
                go.y = target.y;
                go.z = target.z;
            into.Append( go );
        }  break;
        default: {
            DSTBALL_STOP_Struct main;
                main.formationID = 0xFF;
            into.Append( main );
        } break;
    }

    std::string modeStr = "Goto";
    switch (mode) {
        case 1: modeStr = "Follow"; break;
        case 2: modeStr = "Stop"; break;
        case 3: modeStr = "Warp"; break;
        case 4: modeStr = "Orbit"; break;
        case 5: modeStr = "Missile"; break;
        case 6: modeStr = "Mushroom"; break;
        case 7: modeStr = "Boid"; break;
        case 8: modeStr = "Troll"; break;
        case 9: modeStr = "Miniball"; break;
        case 10: modeStr = "Field"; break;
        case 11: modeStr = "Rigid"; break;
        case 12: modeStr = "Formation"; break;
    }

    _log(SE__DESTINY, "Ship::EncodeDestiny(): %s - id:%u, mode:%s, flags:0x%X, Vel:%.1f, %.1f, %.1f", \
            GetName(), head.entityID, modeStr.c_str(), head.flags, data.velocity_x, data.velocity_y, data.velocity_z);
}

void Ship::MakeDamageState(DoDestinyDamageState &into) {
    into.shield = (m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float() +7;
    into.timestamp = Win32TimeNow();
    into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float());
}

PyDict* Ship::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for Ship %s(%u)", m_self->itemName().c_str(), m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",               new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",               new PyInt(m_self->typeID()));
        slim->SetItemString("name",                 new PyString(m_self->itemName()));
        slim->SetItemString("ownerID",              new PyInt(m_ownerID));
        slim->SetItemString("charID",               new PyInt(m_self->GetPilot() ? m_self->GetPilot()->GetCharacterID() : 0));
        slim->SetItemString("corpID",               new PyInt(m_self->GetPilot() ? m_self->GetPilot()->GetCorporationID() : GetCorporationID()));
        slim->SetItemString("allianceID",           new PyInt(m_self->GetPilot() ? m_self->GetPilot()->GetAllianceID() : GetAllianceID()));
        slim->SetItemString("warFactionID",         new PyInt(m_self->GetPilot() ? m_self->GetPilot()->GetWarFactionID() : GetWarFactionID()));
        slim->SetItemString("bounty",               new PyFloat(m_self->GetPilot() ? m_self->GetPilot()->GetBounty() : 0));
        slim->SetItemString("securityStatus",       new PyFloat(m_self->GetPilot() ? m_self->GetPilot()->GetSecurityRating() : 0.0));
    if (m_self->typeID() == itemTypeCapsule) {
        slim->SetItemString("launcherID",           new PyInt(GetPodShipID()));
        return slim;
    } else {
        slim->SetItemString("categoryID",           new PyInt(m_self->categoryID()));
        slim->SetItemString("groupID",              new PyInt(m_self->groupID()));
    }

    //encode the hiSlot and Subsystem modules list ONLY
    std::vector<InventoryItemRef> items;
    m_self->GetMyInventory()->FindByFlagRange(flagHiSlot0, flagHiSlot7, items);
    //m_self->GetMyInventory()->FindByFlagRange(flagSubSystem0, flagSubSystem7, items);
    if (!items.empty()) {
        PyList *l = new PyList();
        for (auto cur : items) {
            l->AddItem(new_tuple(cur->itemID(), cur->typeID()));
        }

        slim->SetItemString("modules", l);
        //PySafeDecRef(l);
    }

    if (is_log_enabled(DESTINY__DEBUG)) {
        _log( DESTINY__DEBUG, "Ship::MakeSlimItem() - %s(%u)", GetName(), GetID());
        slim->Dump(DESTINY__DEBUG, "     ");
    }
    return slim;
}

void Ship::ClearBoostData()
{
    m_oldArmor       = 0;
    m_oldShield      = 0;
    m_oldScanRes     = 0;
    m_oldInertia     = 0;
    m_oldTargetRange = 0;

    m_boost.armored  = 0; // armor hit points
    m_boost.info     = 0; // targeting range
    m_boost.leader   = 0; // targeting speed
    m_boost.mining   = 0; // mining yield
    m_boost.siege    = 0; // shield capacity
    m_boost.skirmish = 0; // agility

    m_boosted = false;
}

void Ship::RemoveBoost()
{
    _log( FLEET__TRACE, "Ship::RemoveBoost() - %s(%u)", GetName(), GetID());

    m_shipRef->SetAttribute(AttrArmorHP, m_oldArmor);
    m_shipRef->SetAttribute(AttrInetia, m_oldInertia);
    m_shipRef->SetAttribute(AttrShieldCapacity, m_oldShield);

    m_destiny->SetShipCapabilities(m_shipRef);

    ClearBoostData();
}

void Ship::ApplyBoost(BoostData& bData)
{
    // note:  mining boost applied in mining module code

    // remove existing boost
    if (m_boosted)
        RemoveBoost();

    _log( FLEET__TRACE, "Ship::ApplyBoost() - %s(%u)", GetName(), GetID());

    m_boost = bData;
    m_oldArmor = m_shipRef->GetAttribute(AttrArmorHP).get_int();
    m_oldInertia = m_shipRef->GetAttribute(AttrInetia).get_float();
    m_oldShield = m_shipRef->GetAttribute(AttrShieldCapacity).get_int();
    m_oldScanRes = m_shipRef->GetAttribute(AttrScanResolution).get_int();
    m_oldTargetRange = m_shipRef->GetAttribute(AttrMaxTargetRange).get_int();

    uint16 armorHP = m_oldArmor * (1 + (0.02 * m_boost.armored)); // 2% increase/level
    uint16 shieldHP = m_oldShield * (1 + (0.02 *  m_boost.siege));// 2% increase/level
    uint16 scanRes = m_oldScanRes * (1 + (0.02 *  m_boost.leader));// 2% increase/level
    uint32 targRange = m_oldTargetRange * (1 + (0.02 *  m_boost.info));// 2% increase/level
    double inertia = m_oldInertia * (1 - (0.02 *  m_boost.skirmish));// 2% decrease/level

    m_shipRef->SetAttribute(AttrInetia, inertia);   // lower inertia = lower agility = faster ship
    m_shipRef->SetAttribute(AttrArmorHP, armorHP);
    m_shipRef->SetAttribute(AttrScanResolution, scanRes);  // higher scanRes = faster targeting
    m_shipRef->SetAttribute(AttrShieldCapacity, shieldHP);
    m_shipRef->SetAttribute(AttrMaxTargetRange, targRange);

    m_destiny->SetShipCapabilities(m_shipRef);

    m_boosted = true;
}

