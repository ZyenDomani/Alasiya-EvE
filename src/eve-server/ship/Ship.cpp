
#include "Client.h"
#include "EVEServerConfig.h"
#include "Profile.h"
#include "character/Character.h"
#include "effects/EffectsProcessor.h"
#include "system/DestinyManager.h"
#include "ship/Ship.h"
#include "system/BubbleManager.h"

/*
 * ShipTypeData
 */
ShipTypeData::ShipTypeData( uint32 weaponTypeID, uint32 miningTypeID, uint32 skillTypeID) : mWeaponTypeID(weaponTypeID),
mMiningTypeID(miningTypeID), mSkillTypeID(skillTypeID) {}
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

ShipType *ShipType::Load(ItemFactory &factory, uint32 shipTypeID)
{
    return ItemType::Load<ShipType>( factory, shipTypeID );
}

/*
 * ShipItem
 */
ShipItem::ShipItem(ItemFactory &_factory, uint32 _shipID, const ShipType &_shipType, const ItemData &_data)
: InventoryItem(_factory, _shipID, _shipType, _data),
m_pilot(nullptr),
m_ModuleManager(nullptr)
{
    m_IsLoaded = false;
    m_effectsApplied = false;
    m_stackMap.clear();
    m_attribMap.clear();
    m_inventory = new Inventory(InventoryItemRef(this));
    _log(ITEM__TRACE, "Created ShipItem for %s(%u).", itemName().c_str(), itemID());
}

ShipItem::~ShipItem()
{
    SafeDelete(m_ModuleManager);
    SafeDelete(m_inventory);
}

ShipItemRef ShipItem::Load(ItemFactory &factory, uint32 shipID)
{
    return InventoryItem::Load<ShipItem>( factory, shipID );
}

ShipItemRef ShipItem::Spawn(ItemFactory &factory, ItemData &data) {
    uint32 shipID = ShipItem::CreateItemID( factory, data );
    if ( shipID == 0 )
        return ShipItemRef();

    ShipItemRef sShipRef = ShipItem::Load( factory, shipID );

    return sShipRef;
}

uint32 ShipItem::CreateItemID(ItemFactory &factory, ItemData &data) {
    return InventoryItem::CreateItemID(factory, data);
}

bool ShipItem::_Load()
{
    // load attributes
    if (!InventoryItem::_Load())
        return false;
    if (type().id() == EVEDB::invTypes::typeCapsule)
        return (m_IsLoaded = true);
    if (m_IsLoaded and m_ModuleManager)
        return true;
    // load contents
    if (!m_inventory->LoadContents(&m_factory))
        return false;

    // Create default dynamic attributes in the AttributeMap
    SetAttribute(AttrVolume,                            GetPackagedVolume());

    // Check for existence of attributes.  if not loaded then set them to default values:
    if (!HasAttribute(AttrDamage))                      SetAttribute(AttrDamage, 0.0f);
    if (!HasAttribute(AttrCpuLoad))                     SetAttribute(AttrCpuLoad, 0);
    if (!HasAttribute(AttrPowerLoad))                   SetAttribute(AttrPowerLoad, 0);
    if (!HasAttribute(AttrArmorDamage))                 SetAttribute(AttrArmorDamage, 0.0f);
    if (!HasAttribute(AttrUpgradeLoad))                 SetAttribute(AttrUpgradeLoad, 0);
    // shield and cap are part of persistance, and loaded on attrib map initalization.  check for and set to full if no saved value found
    if (!HasAttribute(AttrShieldCharge))                SetAttribute(AttrDamage, mAttributeMap.GetAttribute(AttrShieldCapacity));
    if (!HasAttribute(AttrCapacitorCharge))             SetAttribute(AttrDamage, mAttributeMap.GetAttribute(AttrCapacitorCapacity));
    if (!HasAttribute(AttrMaximumRangeCap))             SetAttribute(AttrMaximumRangeCap, ((double)BUBBLE_RADIUS_METERS));
    // Warp Scramble Status of the ship (most ships have zero warp scramble status, but some (t2 indy) already have it defined):
    if (!HasAttribute(AttrWarpScrambleStatus))          SetAttribute(AttrWarpScrambleStatus, 0.0f);
    if (!HasAttribute(AttrWarpSpeedMultiplier))         SetAttribute(AttrWarpSpeedMultiplier, 1.0f);
    if (!HasAttribute(AttrArmorMaxDamageResonance))     SetAttribute(AttrArmorMaxDamageResonance, 1.0f);
    if (!HasAttribute(AttrShieldMaxDamageResonance))    SetAttribute(AttrShieldMaxDamageResonance, 1.0f);

    /*  Resonance  - these should be set in item type
    if (!HasAttribute(AttrShieldEmDamageResonance))       SetAttribute(AttrShieldEmDamageResonance, 1.0);
    if (!HasAttribute(AttrShieldExplosiveDamageResonance)) SetAttribute(AttrShieldExplosiveDamageResonance, 1.0);
    if (!HasAttribute(AttrShieldKineticDamageResonance))  SetAttribute(AttrShieldKineticDamageResonance, 1.0);
    if (!HasAttribute(AttrShieldThermalDamageResonance))  SetAttribute(AttrShieldThermalDamageResonance, 1.0);
    if (!HasAttribute(AttrArmorEmDamageResonance))        SetAttribute(AttrArmorEmDamageResonance, 1.0);
    if (!HasAttribute(AttrArmorExplosiveDamageResonance)) SetAttribute(AttrArmorExplosiveDamageResonance, 1.0);
    if (!HasAttribute(AttrArmorKineticDamageResonance))   SetAttribute(AttrArmorKineticDamageResonance, 1.0);
    if (!HasAttribute(AttrArmorThermalDamageResonance))   SetAttribute(AttrArmorThermalDamageResonance, 1.0);
    if (!HasAttribute(AttrTurretSlotsLeft))               SetAttribute(AttrTurretSlotsLeft, 0);
    if (!HasAttribute(AttrLauncherSlotsLeft))             SetAttribute(AttrLauncherSlotsLeft, 0);
    */
    // hull res is stored in item type as AttrHull*Resonance.  set accordingly
    if (!HasAttribute(AttrEmDamageResonance))           SetAttribute(AttrEmDamageResonance, mAttributeMap.GetAttribute(AttrHullEmDamageResonance));
    if (!HasAttribute(AttrExplosiveDamageResonance))    SetAttribute(AttrExplosiveDamageResonance, mAttributeMap.GetAttribute(AttrHullExplosiveDamageResonance));
    if (!HasAttribute(AttrKineticDamageResonance))      SetAttribute(AttrKineticDamageResonance, mAttributeMap.GetAttribute(AttrHullKineticDamageResonance));
    if (!HasAttribute(AttrThermalDamageResonance))      SetAttribute(AttrThermalDamageResonance, mAttributeMap.GetAttribute(AttrHullThermalDamageResonance));

	// set cargo holds data here:
	if (HasAttribute(AttrCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagCargoHold,mAttributeMap.GetAttribute(AttrCapacity).get_float()));
	if (HasAttribute(AttrDroneCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagDroneBay,mAttributeMap.GetAttribute(AttrDroneCapacity).get_float()));
	if (HasAttribute(AttrSpecialFuelBayCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedFuelBay,mAttributeMap.GetAttribute(AttrSpecialFuelBayCapacity).get_float()));
	if (HasAttribute(AttrSpecialOreHoldCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedOreHold,mAttributeMap.GetAttribute(AttrSpecialOreHoldCapacity).get_float()));
	if (HasAttribute(AttrSpecialGasHoldCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedGasHold,mAttributeMap.GetAttribute(AttrSpecialGasHoldCapacity).get_float()));
	if (HasAttribute(AttrSpecialMineralHoldCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedMineralHold,mAttributeMap.GetAttribute(AttrSpecialMineralHoldCapacity).get_float()));
	if (HasAttribute(AttrSpecialSalvageHoldCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedSalvageHold,mAttributeMap.GetAttribute(AttrSpecialSalvageHoldCapacity).get_float()));
	if (HasAttribute(AttrSpecialShipHoldCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedShipHold,mAttributeMap.GetAttribute(AttrSpecialShipHoldCapacity).get_float()));
	if (HasAttribute(AttrSpecialSmallShipHoldCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedSmallShipHold,mAttributeMap.GetAttribute(AttrSpecialSmallShipHoldCapacity).get_float()));
	if (HasAttribute(AttrSpecialLargeShipHoldCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedLargeShipHold,mAttributeMap.GetAttribute(AttrSpecialLargeShipHoldCapacity).get_float()));
	if (HasAttribute(AttrSpecialIndustrialShipHoldCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedIndustrialShipHold,mAttributeMap.GetAttribute(AttrSpecialIndustrialShipHoldCapacity).get_float()));
	if (HasAttribute(AttrSpecialAmmoHoldCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedAmmoHold,mAttributeMap.GetAttribute(AttrSpecialAmmoHoldCapacity).get_float()));

	UpdateHoldsUsedVolume();

    return (m_IsLoaded = true);
}

void ShipItem::Init()
{
    if (m_type.groupID() == EVEDB::invGroups::Capsule) {
        InitPod();
        return;
    }
    Character* pChar = m_pilot->GetChar().get();
    if (!pChar) {
        _log(SHIP__WARNING, "ShipItem %s(%u) does not have a pilot.", itemName().c_str(), itemID());
        return;
    }

    /** @todo These all still need to have ship bonuses applied */
    // these get default from item type.  specific non-damage attrs are NOT saved.
    float pg = GetDefaultAttribute(AttrPowerOutput).get_int();
    float cpu = GetDefaultAttribute(AttrCpuOutput).get_float();
    float hullHP = GetDefaultAttribute(AttrHP).get_int();
    float armorHP = GetDefaultAttribute(AttrArmorHP).get_float();
    float capCapacity = GetDefaultAttribute(AttrCapacitorCapacity).get_float();
    float shipInertia = GetDefaultAttribute(AttrInetia).get_float();
    double warpCapNeed = GetDefaultAttribute(AttrWarpCapacitorNeed).get_float();
    float capChargeRate = GetDefaultAttribute(AttrRechargeRate).get_float();
    float shieldCapacity = GetDefaultAttribute(AttrShieldCapacity).get_float();
    float shipMaxVelocity = GetDefaultAttribute(AttrMaxVelocity).get_float();
    float shieldChargeRate = GetDefaultAttribute(AttrShieldRechargeRate).get_float();

    pg *=  (1 + (0.05 * (pChar->GetSkillLevel(skillEngineering, true))));                       // 5% increase
    cpu *=  (1 + (0.05 * (pChar->GetSkillLevel(skillElectronics, true))));                      // 5% increase
    hullHP *=  (1 + (0.05 * (pChar->GetSkillLevel(skillMechanics, true))));                     // 5% increase
    armorHP *=  (1 + (0.05 * (pChar->GetSkillLevel(skillHullUpgrades, true))));                 // 5% increase
    capCapacity *=  (1 + (0.05 * (pChar->GetSkillLevel(skillEnergyManagement, true))));         // 5% increase
    shipInertia *= pChar->GetAgilitySkills(HasAttribute(AttrIsCapitalSize));                    // multiple skill effects
    warpCapNeed *=  (1 - (0.1 * ( pChar->GetSkillLevel(skillWarpDriveOperation, true))));       // 10% decrease
    capChargeRate *=  (1 - (0.05 * (pChar->GetSkillLevel(skillEnergySystemsOperation, true)))); // 5% decrease
    shieldCapacity *=  (1 + (0.05 * (pChar->GetSkillLevel(skillShieldManagement, true))));      // 5% increase
    shipMaxVelocity *= (1 + (0.05 * ( pChar->GetSkillLevel(skillNavigation, true))));           // 5% increase
    shieldChargeRate *=  (1 - (0.05 * (pChar->GetSkillLevel(skillShieldOperation, true))));     // 5% decrease

    // add checks for implants here.

    /* to reset for new pilot:
     * offline all modules
     * reset ship attribs
     * add new pilot skills
     * online all modules
     */

    SetAttribute(AttrHP, hullHP);
    SetAttribute(AttrMass, type().mass());   // no default mass in ship item.
    SetAttribute(AttrInetia, shipInertia);
    SetAttribute(AttrArmorHP, armorHP);
    SetAttribute(AttrCpuOutput, cpu);
    SetAttribute(AttrPowerOutput, pg);
    SetAttribute(AttrMaxVelocity, shipMaxVelocity);
    SetAttribute(AttrRechargeRate, capChargeRate);
    SetAttribute(AttrShieldCapacity, shieldCapacity);
    SetAttribute(AttrCapacitorCharge, capCapacity);
    SetAttribute(AttrWarpCapacitorNeed, warpCapNeed);
    SetAttribute(AttrShieldRechargeRate,shieldChargeRate );
    // will eventually code checks for char boarding a tackled ship.  warpScram will be updated then
    SetAttribute(AttrWarpScrambleStatus, 0);

    /* AttrMass = 4,    (largest mass = Leviathan(3764) @ 2,430,000,000kg)
     * AttrMassLimit = 622,
     * AttrMassAddition = 796,
     * AttrMassMultiplier = 1471,
     */
    /*   look into these, too...
     * AttrWarpSBonus(624) [rigs and implants]
     * AttrWarpFactor(21) [all are 0]
     * AttrWarpInhibitor(29) [default is null]
     */

    // create and initialize the module manager if not already done
    if (!m_ModuleManager)
        m_ModuleManager = new ModuleManager(this);

    m_ModuleManager->Initialize();

    if (sConfig.server.IsTestServer) {
        // if test server, update shield and cap (simulate idle charging)
        if (m_pilot->IsInSpace()) {
            SetShipShield(1.0);
            SetShipCapacitorLevel(1.0);
        }
    }
}

void ShipItem::InitPod() {
    // allocate the module manager, only the first time:
    if (!m_ModuleManager) {
        m_ModuleManager = new ModuleManager(this);
        m_ModuleManager->Initialize();
    }
    // pod will be full when activated
    if (m_pilot->IsInSpace())
        Heal();
}

void ShipItem::SetPlayer(Client* pClient) {
    if (m_pilot == pClient)
        return;
    m_pilot = pClient;
    if (!m_pilot) {
        // remove ship effects and char skill effects for char leaving ship here.
        ProcessEffects(false);
        RemoveEffects();
        // should we check for cargo and damage after char leaves ship?  maybe later
        if (m_ModuleManager)
            m_ModuleManager->CharacterLeavingShip();
        return;
    }
    Init();
    ProcessEffects(true);
    if (IsSolarSystem(m_locationID))
        ApplyEffects();
    m_ModuleManager->CharacterBoardingShip();
}

void ShipItem::UpdateHoldsUsedVolume()    /** @todo (allan)  look into this....not working right. */
{
    if (HasAttribute(AttrCapacity)) {
        _log(SHIP__TRACE, "flagCargoHold current values: m_cargoHoldsUsedVolumeByFlag = %lf, GetStoredVolume = %lf", \
                m_cargoHoldsUsedVolumeByFlag.find(flagCargoHold)->second, m_inventory->GetStoredVolume(flagCargoHold));
        m_cargoHoldsUsedVolumeByFlag.find(flagCargoHold)->second = m_inventory->GetStoredVolume(flagCargoHold);
        _log(SHIP__TRACE, "flagCargoHold new values: m_cargoHoldsUsedVolumeByFlag = %lf", \
                m_cargoHoldsUsedVolumeByFlag.find(flagCargoHold)->second);
    }
    if ( HasAttribute(AttrDroneCapacity))
        m_cargoHoldsUsedVolumeByFlag.find(flagDroneBay)->second = m_inventory->GetStoredVolume(flagDroneBay);
    if ( HasAttribute(AttrSpecialFuelBayCapacity))
        m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedFuelBay)->second = m_inventory->GetStoredVolume(flagSpecializedFuelBay);
    if ( HasAttribute(AttrSpecialOreHoldCapacity))
        m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedOreHold)->second = m_inventory->GetStoredVolume(flagSpecializedOreHold);
    if ( HasAttribute(AttrSpecialGasHoldCapacity))
        m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedGasHold)->second = m_inventory->GetStoredVolume(flagSpecializedGasHold);
    if ( HasAttribute(AttrSpecialMineralHoldCapacity))
        m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedMineralHold)->second = m_inventory->GetStoredVolume(flagSpecializedMineralHold);
    if ( HasAttribute(AttrSpecialSalvageHoldCapacity))
        m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedSalvageHold)->second = m_inventory->GetStoredVolume(flagSpecializedSalvageHold);
    if ( HasAttribute(AttrSpecialShipHoldCapacity))
        m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedShipHold)->second = m_inventory->GetStoredVolume(flagSpecializedShipHold);
    if ( HasAttribute(AttrSpecialSmallShipHoldCapacity))
        m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedSmallShipHold)->second = m_inventory->GetStoredVolume(flagSpecializedSmallShipHold);
    if ( HasAttribute(AttrSpecialLargeShipHoldCapacity))
        m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedLargeShipHold)->second = m_inventory->GetStoredVolume(flagSpecializedLargeShipHold);
    if ( HasAttribute(AttrSpecialIndustrialShipHoldCapacity))
        m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedIndustrialShipHold)->second = m_inventory->GetStoredVolume(flagSpecializedIndustrialShipHold);
    if ( HasAttribute(AttrSpecialAmmoHoldCapacity))
        m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedAmmoHold)->second = m_inventory->GetStoredVolume(flagSpecializedAmmoHold);
}

void ShipItem::ModifyHoldVolumeByFlag(EVEItemFlags flag, double amount) {
    if ( m_cargoHoldsUsedVolumeByFlag.find(flag) != m_cargoHoldsUsedVolumeByFlag.end()) {
        m_cargoHoldsUsedVolumeByFlag.find(flag)->second += amount;
    } else {
        _log(SHIP__ERROR, "ModifyContVolumeByFlag() - given flag not found in current map: %u", flag);
        if (m_pilot)
            m_pilot->SendErrorMsg("Item not moved.  Ref: ServerError 65282");
    }
}

void ShipItem::Delete() {
    m_inventory->DeleteContents();
    InventoryItem::Delete();
}

double ShipItem::GetRemainingVolumeByFlag(EVEItemFlags flag) const {
    // updated to use inventory  -allan 26Jul16
    if (flag == flagAutoFit)
        return (m_inventory->GetCapacity(flag) - m_cargoHoldsUsedVolumeByFlag.find(flagCargoHold)->second);
    return (m_inventory->GetCapacity(flag) - m_cargoHoldsUsedVolumeByFlag.find(flag)->second);
}

bool ShipItem::ValidateAddItem(EVEItemFlags flag, InventoryItemRef item)
{
    /** @todo this will need more work to correctly check hold capacity for offline ships */
    if (!m_pilot)
        return true;

    CharacterRef character = m_pilot->GetChar();

    if (flag == flagDroneBay) {
        if ( item->categoryID() != EVEDB::invCategories::Drone ) {
            m_pilot->SendErrorMsg("Item Cannot be stowed in the Drone Bay");
            return false;
        }
    } else if (flag == flagShipHangar) {
        if (GetAttribute(AttrHasShipMaintenanceBay) != 0) {
            m_pilot->SendErrorMsg("%s has no ship maintenance bay.", item->itemName().c_str());
            return false;
        }
        if (item->categoryID() != EVEDB::invCategories::Ship) {
            m_pilot->SendErrorMsg("Only ships may be placed into ship maintenance bay.");
            return false;
        }
    } else if (flag == flagHangar) {
        if (GetAttribute(AttrHasCorporateHangars) != 0) {
            m_pilot->SendErrorMsg("%s has no corporate hangars.", itemName().c_str());
            return false;
        }
    } else if ((flag >= flagLowSlot0) and (flag <= flagHiSlot7)) {
        if (m_pilot->IsClient()) {      // why this check?  does it really matter here?  i dont think so...
            if (!Skill::FitModuleSkillCheck(item, character)) {
                m_pilot->SendErrorMsg("You do not have the required skills to fit this %s.  Ref: ServerError 25163.", item->itemName().c_str());
                return false;
            }
            if (!ValidateItemSpecifics(item)) {
                m_pilot->SendErrorMsg("Your ship cannot equip this %s.  Ref: ServerError 25165.", item->itemName().c_str());
                return false;
            }
            if (item->categoryID() == EVEDB::invCategories::Charge) {
                if (m_ModuleManager and m_ModuleManager->GetModule(flag)) {
                    InventoryItemRef module = m_ModuleManager->GetModule(flag)->getItem();
                    if (module->GetAttribute(AttrChargeSize) != item->GetAttribute(AttrChargeSize)) {
                        sLog.Error("Ship::ValidateAddItem", "Charge size %u for %s does not match Module size %u for %s.",
                                   item->GetAttribute(AttrChargeSize).get_int(), item->itemName().c_str(),
                                   module->GetAttribute(AttrChargeSize).get_int(), module->itemName().c_str()
                        );
                        m_pilot->SendErrorMsg("The charge is not the correct size for this module.");
                        return false;
                    }
                    if ((module->GetAttribute(AttrChargeGroup1) != item->groupID())
                        and (module->GetAttribute(AttrChargeGroup2) != item->groupID())
                        and (module->GetAttribute(AttrChargeGroup3) != item->groupID())
                        and (module->GetAttribute(AttrChargeGroup4) != item->groupID())
                        and (module->GetAttribute(AttrChargeGroup5) != item->groupID())) {
                            m_pilot->SendErrorMsg("Incorrect charge type for this module.");
                            return false;
                    }
                    // NOTE: Module Manager will check for actual room to load charges and make stack splits, or reject loading altogether
                } else {
                    m_pilot->SendErrorMsg("Module at flag '%u' does not exist.  Ref: ServerError 25162.", flag);
                    return false;
                }
            } else {
                if (m_ModuleManager and m_ModuleManager->IsSlotOccupied(flag)) {
                    if (m_pilot->CanThrow())
                        throw PyException( MakeUserError( "SlotAlreadyOccupied" ));
                    return false;
                }
            }
        }
    } else if ((flag >= flagRigSlot0) and (flag <= flagRigSlot7)) {
        if (m_pilot->IsClient()) {
            if (!Skill::FitModuleSkillCheck(item, character)) {
                m_pilot->SendErrorMsg("You do not have the required skills to fit this %s", item->itemName().c_str());
                return false;
            }
            if (GetAttribute(AttrRigSize) != item->GetAttribute(AttrRigSize)) {
                m_pilot->SendErrorMsg("Your ship cannot fit this size module");
                return false;
            }
            if (GetAttribute(AttrUpgradeLoad) + item->GetAttribute(AttrUpgradeCost) > GetAttribute(AttrUpgradeCapacity)) {
                m_pilot->SendErrorMsg("Your ship cannot handle the extra calibration");
                return false;
            }
        }
    } else if ((flag >= flagSubSystem0) and (flag <= flagSubSystem7)) {
        if (m_pilot->IsClient())
            if (!Skill::FitModuleSkillCheck(item, character)) {
                m_pilot->SendErrorMsg("You do not have the required skills to fit this %s", item->itemName().c_str());
                return false;
            }
    } else {
        // Handle any other flag, legal or not by virtue of GetRemainingVolumeByFlag() and GetCapacity() that handle supported capacity types:
        // (unsupported or illegal flags report capacity of 0.0, so are automatically rejected)
        // check for adding unpackaged ships to cargo of active ship...
        double volume = item->GetPackagedVolume();
        if ((GetRemainingVolumeByFlag(flag) < (volume * item->quantity()))) {
            m_pilot->SendErrorMsg("Not enough cargo space");
            return false;
        }
    }

    return true;
}

// this one is called from ShipGetInfo
PyDict* ShipItem::ShipGetInfo() {
    if ( !m_inventory->LoadContents( &m_factory )) {
        _log( SHIP__ERROR, "%s(%u): Failed to load contents for ShipGetInfo", itemName().c_str(), itemID());
        return nullptr;
    }

    Rsp_CommonGetInfo_Entry entry;
    //first populate the ship.
    if ( !Populate( entry ))
        return nullptr;    //print already done.

    PyDict* result = new PyDict;
    result->SetItem(new PyInt( itemID()), new PyObject("util.KeyVal", entry.Encode()));
    //now encode contents...
    std::vector<InventoryItemRef> equipped;
    //find all the equipped items and rigs
    uint8 mod = m_inventory->FindByFlagRange( flagLowSlot0, flagFixedSlot, equipped );
    uint8 rig = m_inventory->FindByFlagRange( flagRigSlot0, flagRigSlot7, equipped );
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
    if (!m_inventory->LoadContents(&m_factory))  {
        _log( INV__ERROR, "%s(%u): Failed to load contents for ShipGetInfo", itemName().c_str(), itemID());
        return nullptr;
    }

    PyDict *result = new PyDict;
    Rsp_CommonGetInfo_Entry entry;

    //first populate the ship.
    if ( !Populate( entry ))
        return nullptr;

    result->SetItem(new PyInt( itemID()), new PyObject("util.KeyVal", entry.Encode()));

    //now encode contents...
    std::vector<InventoryItemRef> equipped;
    //find all the equipped items and rigs
    uint8 mod = m_inventory->FindByFlagRange( flagLowSlot0, flagFixedSlot, equipped );
    uint8 rig = m_inventory->FindByFlagRange( flagRigSlot0, flagRigSlot7, equipped );
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
    if (!m_inventory->ContentsLoaded()) {
        if (!m_inventory->LoadContents(&m_factory)) {
            _log(INV__ERROR, "%s(%u): Failed to load contents for GetShipState", itemName().c_str(), itemID());
            return nullptr;
        }
    }
    // Create new dictionary for shipState:
    PyDict *result = new PyDict;
    // Create entry for ShipItem itself:
    result->SetItem(new PyInt(itemID()), GetItemStatusRow());
    // Check for and Create entry for pilot:
    InventoryItemRef pilot;
    if (m_inventory->FindSingleByFlag(flagPilot, pilot))
        result->SetItem(new PyInt(pilot->itemID()), pilot->GetItemStatusRow());

    if (m_ModuleManager) {
        // Create entries for ALL modules, rigs, and subsystems present on ship:
        std::vector<InventoryItemRef> moduleList;
        m_ModuleManager->GetModuleListOfRefs( &moduleList );
        for (int i=0; i<moduleList.size(); i++)
            result->SetItem(new PyInt(moduleList.at(i)->itemID()), moduleList.at(i)->GetItemStatusRow());
    } else
        _log(SHIP__MODULE_ERROR, "GetShipState() - %s(%u) has no module manager.", itemName().c_str(), itemID());

    return result;
}

PyList* ShipItem::ShipGetModuleList() {
    if (!m_inventory->LoadContents(&m_factory)) {
        _log(INV__ERROR, "%s(%u): Failed to load contents for ShipGetModuleList", itemName().c_str(), itemID());
        return nullptr;
    }
    if (!m_ModuleManager) {
        _log(SHIP__MODULE_ERROR, "ShipGetModuleList() - %s(%u) has no module manager.", itemName().c_str(), itemID());
        return nullptr;
    }

    PyList* result = new PyList;
    PyTuple* module = new PyTuple(2);
    // Create entries in "onslimitemchange" modules list for ALL modules, rigs, and subsystems present on ship:
    std::vector<InventoryItemRef> moduleList;
    m_ModuleManager->GetModuleListOfRefs( &moduleList );
    for (int i=0; i<moduleList.size(); i++) {
        module->SetItem(0, new PyInt(moduleList.at(i)->typeID()));
        module->SetItem(1, new PyInt(moduleList.at(i)->itemID()));
        result->AddItem(module);
    }

    return result;
}

PyDict* ShipItem::GetChargeState() {
    /*  this is correct */
    if (!m_inventory->ContentsLoaded()) {
        if (!m_inventory->LoadContents(&m_factory)) {
            _log(INV__ERROR, "%s(%u): Failed to load contents for GetShipState", itemName().c_str(), itemID());
            return nullptr;
        }
    }
    if (!m_ModuleManager) {
        _log(SHIP__MODULE_ERROR, "GetChargeState() - %s(%u) has no module manager.", itemName().c_str(), itemID());
        return nullptr;
    }

    /* get list of charges loaded in ship modules (*all slots*) */
    std::map< EVEItemFlags, InventoryItemRef > charges;
    m_ModuleManager->GetLoadedCharges(charges);

    if (charges.empty()) {
        PyDict *result = new PyDict;
        //result->SetItem(new PyInt(itemID()), new BuiltinSet());
        return result;
    }

    // Create entries in "shipState" dictionary for loaded charges on ship:
    uint32 shipID = itemID();
    PyDict* chargeDict = new PyDict;
    for (auto cur : charges)
        chargeDict->SetItem(new PyInt((uint32)cur.first), cur.second->GetChargeStatusRow(shipID));

    PyDict *result = new PyDict;
    result->SetItem(new PyInt(itemID()), chargeDict);
    return result;
}

void ShipItem::AddItem(InventoryItemRef item)
{
    if (( item->flag() >= flagSlotFirst)
        and (item->flag() <= flagSlotLast)
        and (item->categoryID() != EVEDB::invCategories::Charge)) {
            // make singleton
            item->ChangeSingleton( true );
        }
    m_inventory->AddItem( item );
}

bool ShipItem::ValidateBoardShip(ShipItemRef ship, CharacterRef character) {
    SkillRef requiredSkill;
    EvilNumber skillTypeID;

    if ( ship->HasAttribute(AttrRequiredSkill1, skillTypeID))
        if (!character->HasSkillTrainedToLevel( skillTypeID.get_int(), ship->GetAttribute(AttrRequiredSkill1Level).get_int()))
            return false;
    if ( ship->HasAttribute(AttrRequiredSkill2, skillTypeID))
        if (!character->HasSkillTrainedToLevel( skillTypeID.get_int(), ship->GetAttribute(AttrRequiredSkill2Level).get_int()))
            return false;
    if ( ship->HasAttribute(AttrRequiredSkill3, skillTypeID))
        if (!character->HasSkillTrainedToLevel( skillTypeID.get_int(), ship->GetAttribute(AttrRequiredSkill3Level).get_int()))
            return false;
    if ( ship->HasAttribute(AttrRequiredSkill4, skillTypeID))
        if (!character->HasSkillTrainedToLevel( skillTypeID.get_int(), ship->GetAttribute(AttrRequiredSkill4Level).get_int()))
            return false;
    if ( ship->HasAttribute(AttrRequiredSkill5, skillTypeID))
        if (!character->HasSkillTrainedToLevel( skillTypeID.get_int(), ship->GetAttribute(AttrRequiredSkill5Level).get_int()))
            return false;
    if ( ship->HasAttribute(AttrRequiredSkill6, skillTypeID))
        if (!character->HasSkillTrainedToLevel( skillTypeID.get_int(), ship->GetAttribute(AttrRequiredSkill6Level).get_int()))
            return false;
    return true;
}

void ShipItem::SaveShip()
{
    SaveItem();                         // Save ship info
    mAttributeMap.SaveShipState();      // save ship damage
    if (m_ModuleManager)
        m_ModuleManager->SaveModules();     // Save item info for modules fitted to this ship
}

bool ShipItem::ValidateItemSpecifics(InventoryItemRef item) {
    uint32 groupID = m_pilot->GetShip()->groupID();

    EvilNumber canFitShipGroup1=0, canFitShipGroup2=0, canFitShipGroup3=0, canFitShipGroup4=0;
    // If a ship group restriction is specified, the item must be able to fit to at least one ship group.
    if (item->HasAttribute(AttrCanFitShipGroup1, canFitShipGroup1)
        or item->HasAttribute(AttrCanFitShipGroup2, canFitShipGroup2)
        or item->HasAttribute(AttrCanFitShipGroup3, canFitShipGroup3)
        or item->HasAttribute(AttrCanFitShipGroup4, canFitShipGroup4)) {
            _log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - Beginning the group validation for %s(%u):", item->itemName().c_str(), item->itemID());
            _log(SHIP__MODULE_TRACE, "Has AttrCanFitShipGroup1 = %s", item->HasAttribute(AttrCanFitShipGroup1, canFitShipGroup1) ? "True":"False");
            _log(SHIP__MODULE_TRACE, "Has AttrCanFitShipGroup2 = %s", item->HasAttribute(AttrCanFitShipGroup2, canFitShipGroup2) ? "True":"False");
            _log(SHIP__MODULE_TRACE, "Has AttrCanFitShipGroup3 = %s", item->HasAttribute(AttrCanFitShipGroup3, canFitShipGroup3) ? "True":"False");
            _log(SHIP__MODULE_TRACE, "Has AttrCanFitShipGroup4 = %s", item->HasAttribute(AttrCanFitShipGroup4, canFitShipGroup4) ? "True":"False");
            if ((canFitShipGroup1 != groupID)
                and (canFitShipGroup2 != groupID)
                and (canFitShipGroup3 != groupID)
                and (canFitShipGroup4 != groupID)) {
                    _log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - No attribute found. groupID = %i", groupID);
                    return false;
                } else
                _log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - Group Validation passed.");
        }

    uint32 typeID = m_pilot->GetShip()->typeID();
    EvilNumber canFitShipType1=0, canFitShipType2=0, canFitShipType3=0, canFitShipType4=0;
    // If a ship type restriction is specified, the item must be able to fit to at least one ship type.
    if (item->HasAttribute(AttrCanFitShipType1, canFitShipType1)
        or item->HasAttribute(AttrCanFitShipType2, canFitShipType2)
        or item->HasAttribute(AttrCanFitShipType3, canFitShipType3)
        or item->HasAttribute(AttrCanFitShipType4, canFitShipType4)) {
            _log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - Beginning the type validation for %s(%u):", item->itemName().c_str(), item->itemID());
            _log(SHIP__MODULE_TRACE, "Has AttrCanFitShipType1 = %s", item->HasAttribute(AttrCanFitShipType1, canFitShipType1) ? "True":"False");
            _log(SHIP__MODULE_TRACE, "Has AttrCanFitShipType2 = %s", item->HasAttribute(AttrCanFitShipType2, canFitShipType2) ? "True":"False");
            _log(SHIP__MODULE_TRACE, "Has AttrCanFitShipType3 = %s", item->HasAttribute(AttrCanFitShipType3, canFitShipType3) ? "True":"False");
            _log(SHIP__MODULE_TRACE, "Has AttrCanFitShipType4 = %s", item->HasAttribute(AttrCanFitShipType4, canFitShipType4) ? "True":"False");
            if ( (canFitShipType1 != typeID) and (canFitShipType2 != typeID) and (canFitShipType3 != typeID) and (canFitShipType4 != typeID)){
                _log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - No attribute found. typeID = %i", typeID);
                return false;
            } else
                _log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - Item Validation passed.");
        }

    _log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - Validation passed. Fitting the module");
    return true;
}

void ShipItem::ProcessModules() {
    if (m_pilot->IsDocked())
        return;
    if (m_ModuleManager)
        m_ModuleManager->Process();
}

void ShipItem::Dock() {
    DeactivateAllModules();
}

void ShipItem::Undock() {
    if (sConfig.server.IsTestServer) {
        // Heal Ship completely on test server
        Heal();
    } else {
        // live server will Recharge shields and cap if session change isnt active
        if (!m_pilot->IsSessionChange()) {
            SetShipShield(1.0);
            SetShipCapacitorLevel(1.0);
        }
    }
    // apply ship effects
    ApplyEffects();
    //get list of modules to activate from ShipBound::Handle_Undock()
    // calling Online() on modules will also apply their state 0 effects
    for (auto cur : m_onlineModuleVec) {
        if (m_ModuleManager)
            m_ModuleManager->Online(cur);
    }
}

void ShipItem::Warp() {
    if (m_ModuleManager)
        m_ModuleManager->ShipWarping();
}

void ShipItem::Jump() {
    if (m_ModuleManager)
        m_ModuleManager->ShipJumping();
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
InventoryItemRef ShipItem::GetModule(EVEItemFlags flag)
{
    if (m_ModuleManager and m_ModuleManager->GetModule(flag) )
		return (m_ModuleManager->GetModule(flag))->getItem();
	else
		return InventoryItemRef();
}

InventoryItemRef ShipItem::GetModule(uint32 itemID)
{
    if (m_ModuleManager and m_ModuleManager->GetModule(itemID) )
		return (m_ModuleManager->GetModule(itemID))->getItem();
	else
		return InventoryItemRef();
}

EVEItemFlags ShipItem::FindAvailableModuleSlot(InventoryItemRef item) {
    uint16 slotFound = flagIllegal;
    // 1) get slot bank (low, med, high, rig, subsystem) from dgmTypeEffects using item->itemID()
    // 2) query this ship's ModuleManager to determine if there are any free slots in that bank,
    //    it should return a slot flag number for the next available slot starting at the lowest number
    //    for that bank
    // 3) return that slot flag number
    if (item->type().HasEffect(effectLoPower)) {
        slotFound = m_ModuleManager->GetAvailableSlotInBank(effectLoPower);
    } else if (item->type().HasEffect(effectMedPower)) {
        slotFound = m_ModuleManager->GetAvailableSlotInBank(effectMedPower);
    } else if (item->type().HasEffect(effectHiPower)) {
        slotFound = m_ModuleManager->GetAvailableSlotInBank(effectHiPower);
    } else if (item->type().HasEffect(effectSubSystem)) {
        slotFound = m_ModuleManager->GetAvailableSlotInBank(effectSubSystem);
    } else if (item->type().HasEffect(effectRigSlot)) {
        slotFound = m_ModuleManager->GetAvailableSlotInBank(effectRigSlot);
    } else {
        // ERROR: This is not a module that fits in any of the slot banks
    }

    return (EVEItemFlags)slotFound;
}

uint32 ShipItem::AddItem(EVEItemFlags flag, InventoryItemRef item)
{
    if (!ValidateAddItem(flag, item))
        return 0;

    if (IsModuleSlot(flag)) {
        if (!m_ModuleManager)
            return 0;
        if (item->categoryID() == EVEDB::invCategories::Charge) {
            m_ModuleManager->LoadCharge(item, flag);
            InventoryItemRef loadedChargeOnModule = m_ModuleManager->GetLoadedChargeOnModule(flag);
            if (loadedChargeOnModule)
                return loadedChargeOnModule->itemID();
            else
                return 0;
        } else if (item->categoryID() == EVEDB::invCategories::Module) {
            item->PutOffline();
            item->ChangeSingleton(true, false);
            // rigs are classed in the module category.  check here and call approprate method as needed.
            if ((item->groupID() >= 773 and item->groupID() <= 782) or item->groupID() == 786) {
                if (!m_ModuleManager->InstallRig(item, flag))
                    return 0;
            } else if (!m_ModuleManager->FitModule(item, flag))
                return 0;
        } else if (item->categoryID() == EVEDB::invCategories::Subsystem) {
            item->PutOffline();
            item->ChangeSingleton(true, false);
            if (!m_ModuleManager->InstallSubSystem(item, flag))
                return 0;
        }
    } else {
        ModifyHoldVolumeByFlag( flag, (item->GetAttribute(AttrVolume).get_float() * item->quantity()));
	}

    item->Move(itemID(), flag);
	if (IsModuleSlot(flag)) {
        // may not need this call.  is redundant, but has redundant check built-in...
        m_ModuleManager->Online(item->itemID());
        UpdateModules(flag);
    }

	return item->itemID();
}

void ShipItem::RemoveItem(InventoryItemRef item, uint32 qty/*0*/)
{
    if (!m_pilot)
        return;

    // check to see if item is currently in a module slot.  going by category is NOT working after _ExecAdd() updates.
    if (IsModuleSlot(item->flag())) {
        if (!m_ModuleManager)
            return;
        // if item being removed is in a module slot, remove it via Module Manager here, and let invBound take care of the rest.
        if (item->categoryID() == EVEDB::invCategories::Charge) {
            m_ModuleManager->UnloadCharge(item->flag());
        } else if ((item->categoryID() == EVEDB::invCategories::Module) or (item->categoryID() == EVEDB::invCategories::Subsystem)) {
            Deactivate( item->itemID(), "offline" );
            if ((item->flag() >= flagRigSlot0) and (item->flag() <= flagRigSlot7))
                m_ModuleManager->UninstallRig(item->itemID());
            else
                m_ModuleManager->UnfitModule(item->itemID());
        }
    } else
        ModifyHoldVolumeByFlag( item->flag(), -(item->GetAttribute(AttrVolume).get_float() * (qty ? qty : item->quantity())));
}

void ShipItem::MoveModuleSlot(EVEItemFlags slot1, EVEItemFlags slot2) {
    // slot1 is occupied, as this is location module is from.
    InventoryItemRef modItemRef1 = GetModule(slot1);
    if (!modItemRef1) {
        _log(SHIP__MODULE_TRACE, "Ship::MoveModuleSlot - modItemRef1 is null." );
        m_pilot->SendNotifyMsg("There was an internal error.  The module to move was not found.");
        return;
    }
    InventoryItemRef chargeItemRef1 = m_ModuleManager->GetLoadedChargeOnModule(slot1);
    if (chargeItemRef1)
        m_ModuleManager->UnloadCharge(slot1);
    RemoveItem(modItemRef1);

    if (m_ModuleManager->IsSlotOccupied(slot2)) {
        // dropped-on slot is occupied.  procede with moving the module currently in this slot.
        InventoryItemRef modItemRef2 = GetModule(slot2);
        InventoryItemRef chargeItemRef2 = m_ModuleManager->GetLoadedChargeOnModule(slot2);
        if (chargeItemRef2)
            m_ModuleManager->UnloadCharge(slot2);
        RemoveItem(modItemRef2);

        AddItem(slot1, modItemRef2);
        if (chargeItemRef2)
            m_ModuleManager->LoadCharge(chargeItemRef2, slot1);
    }

    AddItem(slot2, modItemRef1);
    if (chargeItemRef1)
        m_ModuleManager->LoadCharge(chargeItemRef1, slot2);

    UpdateModules(slot1);
}

void ShipItem::UpdateModules()
{
    // List of callees to put this function into context as to what it should be doing:
    // Client::BoardShip()              - put modules online that are recorded with attributeID 2 as being online / skill check all modules and if any fail, keep those OFFLINE
    // InventoryBound::_ExecAdd()       - things have been added or removed, recheck all modules for... some reason
    // Client::MoveItem()               - something has been moved into or out of the ship, recheck all modules for... some reason
    m_ModuleManager->UpdateModules();
    //sLog.Error( "Ship::UpdateModules()", "We are currently not checking for modules that need to go online, or skill checking character for any modules of a newly boarded ship, or updating module states based on things being moved into or off the ship!" );
    //sLog.Error( "Ship::UpdateModules()", "This should really be a simple call to a function ModuleManager::UpdateModules() and the code put inside there." );
}

void ShipItem::UpdateModules(EVEItemFlags flag)
{
	// List of callees to put this function into context as to what it should be doing:
	// Client::BoardShip()				- put modules online that are recorded with attributeID 2 as being online / skill check all modules and if any fail, keep those OFFLINE
	// InventoryBound::_ExecAdd()		- things have been added or removed, recheck all modules for... some reason
	// Client::MoveItem()				- something has been moved into or out of the ship, recheck all modules for... some reason
    m_ModuleManager->UpdateModules(flag);
}

void ShipItem::UnloadModule(uint32 itemID)
{
    m_ModuleManager->UnfitModule(itemID);
}

void ShipItem::UnloadAllModules()
{
    m_ModuleManager->UnloadAllModules();
}

void ShipItem::RepairModules()
{
    // FIXME TODO get module IDs and send to function
    uint32 modID = 0;
    m_ModuleManager->RepairModule(modID);
}

void ShipItem::Online (uint32 moduleID)
{
    if (IsSolarSystem(locationID())) {
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
    m_ModuleManager->Activate( itemID, effectName, targetID, repeat );
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

void ShipItem::RemoveRig(InventoryItemRef item) {
    //may not look like it, but just moving this item will call ModuleManager::UninstallRig().  not anymore.  fix this shit.
    m_ModuleManager->UninstallRig(item->itemID());
    item->Move(itemID(), flagCargoHold);
}

void ShipItem::OnlineAll()
{
    m_ModuleManager->OnlineAll();
}

void ShipItem::OfflineAll()
{
    m_ModuleManager->OfflineAll();
}


void ShipItem::ReplaceCharges(EVEItemFlags flag, InventoryItemRef newCharge)
{

}

void ShipItem::DeactivateAllModules()
{
    m_ModuleManager->DeactivateAllModules();
}
/* End new Module Manager Interface */

void ShipItem::StripFitting()
{
    std::vector<InventoryItemRef> modList;
    m_ModuleManager->GetModuleListOfRefs(&modList);
    for (auto cur : modList) {
        m_ModuleManager->UnfitModule(cur->itemID());
        cur->Move(m_pilot->GetLocationID(), flagHangar);
    }
}

// stacking penality system   -allan   (UD 29Jul16)
double ShipItem::GetEffectiveness(uint16 attrib, ModuleStates state)
{
    uint8 count = 1;
    /** @note this is no longer valid....
    std::map<uint16, uint8>::iterator itr = m_stackMap.find(attrib);
    if (itr != m_stackMap.end()) {
        switch (state) {
            case MOD_ONLINE:
            case MOD_ACTIVATED: {
                count = ++(itr->second);
            } break;
            case MOD_OFFLINE:
            case MOD_DEACTIVATING: {
                count = itr->second;
                if (itr->second == 1)
                    m_stackMap.erase(itr);
                else
                    --(itr->second);
            } break;
            default: {
                codelog(SHIP__MODULE_ERROR, "ShipItem::GetEffectiveness() -  module has invalid state %u", state);
                if (m_pilot)
                    m_pilot->SendErrorMsg("Internal Server Error - module has invalid state.  Ref: ServerError 15611");
            } break;
        }
    } else {
        m_stackMap.emplace(attrib, 1);
    }
    */
    //stacking calculation fixed  -allan  20Dec15
    double effectiveness = exp(-pow(((count - 1)/2.67),2));

    if (effectiveness <= 0) {   /* this should never happen */
        codelog(SHIP__MODULE_ERROR, "ShipItem::GetEffectiveness() -  effectiveness <= 0");
        if (m_pilot)
            m_pilot->SendErrorMsg("Internal Server Error - module has 0 effectiveness.  Hacking to 1.  Ref: ServerError 15610");
        effectiveness = 1;
    }

    return effectiveness;
}

// resist cap system    -allan 26Dec16
void ShipItem::InitStackingMap()
{
    /*
    m_resistMap[AttrKineticDamageResonance] = GetAttribute(AttrKineticDamageResonance).get_float();
    m_resistMap[AttrThermalDamageResonance] = GetAttribute(AttrThermalDamageResonance).get_float();
    m_resistMap[AttrExplosiveDamageResonance] = GetAttribute(AttrExplosiveDamageResonance).get_float();
    m_resistMap[AttrEmDamageResonance] = GetAttribute(AttrEmDamageResonance).get_float();
    m_resistMap[AttrArmorEmDamageResonance] = GetAttribute(AttrArmorEmDamageResonance).get_float();
    m_resistMap[AttrArmorExplosiveDamageResonance] = GetAttribute(AttrArmorExplosiveDamageResonance).get_float();
    m_resistMap[AttrArmorKineticDamageResonance] = GetAttribute(AttrArmorKineticDamageResonance).get_float();
    m_resistMap[AttrArmorThermalDamageResonance] = GetAttribute(AttrArmorThermalDamageResonance).get_float();
    m_resistMap[AttrShieldEmDamageResonance] = GetAttribute(AttrShieldEmDamageResonance).get_float();
    m_resistMap[AttrShieldExplosiveDamageResonance] = GetAttribute(AttrShieldExplosiveDamageResonance).get_float();
    m_resistMap[AttrShieldKineticDamageResonance] = GetAttribute(AttrShieldKineticDamageResonance).get_float();
    m_resistMap[AttrShieldThermalDamageResonance] = GetAttribute(AttrShieldThermalDamageResonance).get_float();

    // this is not resist...cannot use cap method on these...
    m_resistMap[AttrScanResolution] = GetAttribute(AttrScanResolution).get_float();
    m_resistMap[AttrSignatureRadius] = GetAttribute(AttrSignatureRadius).get_float();
    */
    /** these also have char skills that will need to be recalculated if the attrib is reset to base
    m_resistMap[AttrMaxVelocity] = GetAttribute(AttrMaxVelocity).get_float();
    m_resistMap[AttrInetia] = GetAttribute(AttrInetia).get_float();
    m_resistMap[AttrShieldRechargeRate] = GetAttribute(AttrShieldRechargeRate).get_float();
    m_resistMap[AttrMaxTargetRange] = GetAttribute(AttrMaxTargetRange).get_float();
    */
}

void ShipItem::SetTrueResist(uint16 attrib, EvilNumber& value)
{
}

void ShipItem::GetTrueResist(uint16 attrib, EvilNumber& value)
{
}

void ShipItem::CheckStacking(uint16 attrib, Effects::Association type, ModuleStates state, EvilNumber& value)
{
}


std::string ShipItem::GetShipDNA()
{
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

    std::vector<InventoryItemRef> modList;
    m_ModuleManager->GetModuleListOfRefs(&modList);

    for (auto cur : modList) {
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

// new effects system.  wip
void ShipItem::ProcessEffects(bool add/*true*/)
{
    fxData data;
    data.assoc = data.env = data.targAttr = data.srcAttr = data.grpID = data.typeID = data.domain = 0;
    auto itr = m_stateFxMap.equal_range(0);
    for (auto it = itr.first; it != itr.second; it++) {
        if (add)
            ParseExpression(sFxDataMgr.GetExpression(it->second.preExpression), data);
        else
            ParseExpression(sFxDataMgr.GetExpression(it->second.postExpression), data);
    }
}

void ShipItem::ApplyEffects()
{
    if (!m_ModuleManager) {
        m_effectsApplied = false;
        return;
    }
    using namespace Effects;
    for (auto cur : m_modifiers) {  // k,v of assoc, data<assoc, domain, env, targAttr, srcAttr, grpID, typeID>
        // get env
        InventoryItem* envItem(nullptr);
        switch (cur.second.env) {
            case dgmEnvInvalid: {   // null
                // if environment is invalid, just continue.  make error later
            } break;
            case dgmEnvShip:        // should be self.
            case dgmEnvSelf: {      // ship
                envItem = this;
            } break;
            case dgmEnvChar: {      // pilot
                envItem = m_pilot->GetChar().get();
            } break;
            case dgmEnvOther: {     // charges?
            } break;
            case dgmEnvTarget:      // not used for ship effects
            case dgmEnvArea: {      // not used for ship effects
            } break;
        }

        // get srcAttr, check for nerf, modify value as needed
        EvilNumber srcAttr = mAttributeMap.GetAttribute(cur.second.srcAttr);

        EvilNumber targAttr = 0;
        // test for location domain
        if (cur.second.domain) {
            std::vector<InventoryItemRef> itemVec;
            switch (cur.second.domain) {
                case dgmDomainGroup: {
                    std::vector<InventoryItemRef> moduleList;
                    m_ModuleManager->GetModuleListOfRefs(&moduleList);
                    // location group defined by data.grpID
                    // get modules beloning to 'grpID'
                    for (auto mod : moduleList)
                        if (mod->groupID() == cur.second.grpID)
                            itemVec.push_back(mod);
                } break;
                case dgmDomainSkill: {
                    // location group defined by data.typeID (for getting items based on skill requirement)
                    // get modules that require skill 'typeID'
                    m_ModuleManager->GetModuleListByReqSkill(cur.second.typeID, &itemVec);
                } break;
            }
            for (auto item : itemVec) {
                // get targAttr
                targAttr = item->GetAttribute(cur.second.targAttr);
                // send data to calculator
                EvilNumber newAttr = sFxProc.CalculateAttributeValue(targAttr, srcAttr, cur.first);
                // set new calculated value for target attribute
                item->SetAttribute(cur.second.targAttr, newAttr, false);
            }
        } else {
            // location domain is self.
            // get targAttr
            if (envItem)
                targAttr = envItem->GetAttribute(cur.second.targAttr);
            // send data to calculator
            EvilNumber newAttr = sFxProc.CalculateAttributeValue(targAttr, srcAttr, cur.first);
            // set new calculated value for target attribute
            if (envItem)
                envItem->SetAttribute(cur.second.targAttr, newAttr, false); // no, dont notifiy client of change.  will be updated after undock, when requesting ship info
        }
    }
    m_effectsApplied = true;
}

void ShipItem::RemoveEffects()
{
    SaveShip();
    mAttributeMap.Load(true);
    m_effectsApplied = false;
}

void ShipItem::AddEffect(uint16 attributeID, InventoryItemRef iRef)
{

}

void ShipItem::RemoveEffect(uint16 attributeID, InventoryItemRef iRef)
{

}

void ShipItem::ApplyModifiers()
{

}


/*
# Items belonging to these categories never have
# their effects stacking penalized
PENALTY_IMMUNE_CATEGORIES = (
    Category.ship,
    Category.charge,
    Category.skill,
    Category.implant,
    Category.subsystem
)

# Stacking penalty base constant, used in attribute calculations
PENALTY_BASE = 1 / exp((1 / 2.67) ** 2)

# Tuple with penalizable operators
PENALIZABLE_OPERATORS = (
    Operator.pre_mul,
    Operator.post_mul,
    Operator.post_percent,
    Operator.pre_div,
    Operator.post_div
)

*/

void ShipItem::ParseExpression(Expression expression, fxData& data)
{
    using namespace Effects;
    switch(expression.operandID) {
        // trivial attribute operations
        case operandATT: {      //'%(arg1)s->%(arg2)s'      (domain:attribID)
            if (expression.arg1)
                ParseExpression(sFxDataMgr.GetExpression(expression.arg1), data);
            if (expression.arg2)
                ParseExpression(sFxDataMgr.GetExpression(expression.arg2), data);
        } break;

        // these return the given expressionValue
        case operandDEFBOOL:    // this evaulates to 'true' (Bool(1))
        case operandDEFINT: {   // this is used as  0,1,2,{raceID}
            // not sure what to do here
            expression.expressionValue;
        } break;
        case operandDEFASSOCIATION: {
            data.assoc = sFxProc.GetAssociationEnum(expression.expressionValue);
        } break;
        case operandDEFENVIDX: {
            data.env = sFxProc.GetEnvironmentEnum(expression.expressionValue);
        } break;

        // these provide the given expression*ID
        case operandDEFATTRIBUTE: {    // check this....
            if (expression.expressionAttributeID) {
                if (data.targAttr)
                    data.srcAttr = expression.expressionAttributeID;
                else
                    data.targAttr = expression.expressionAttributeID;
            }
        } break;
        case operandDEFGROUP: {    //
            if (expression.expressionGroupID)
                data.grpID = expression.expressionGroupID;
        } break;
        case operandDEFTYPEID: {    //
            if (expression.expressionTypeID)
                data.typeID = expression.expressionTypeID;
        } break;

        // do as stated
        case operandCOMBINE: { // executes two statements  '%(arg1)s); (%(arg2)s'
            //data1.assoc = data1.env = data1.targAttr = data1.srcAttr = data1.grpID = data1.typeID = data.domain = 0;
            if (expression.arg1)
                ParseExpression(sFxDataMgr.GetExpression(expression.arg1), data);
            fxData data1;
            data1.assoc = data1.env = data1.targAttr = data1.srcAttr = data1.grpID = data1.typeID = data.domain = 0;
            if (expression.arg2)
                ParseExpression(sFxDataMgr.GetExpression(expression.arg2), data1);
        } break;
        case operandEFF: {      //31, '(%(arg2)s).(%(arg1)s)'       --define association type
            ParseExpression(sFxDataMgr.GetExpression(expression.arg1), data);
            ParseExpression(sFxDataMgr.GetExpression(expression.arg2), data);
        } break;

        // these function calls are a bit more complicated...will need more work and better understanding
        case operandGM: {    //'%(arg1)s.GetModule(%(arg2)s)'      --used by subsystems as (GetModule(Ship.201):55)
            ParseExpression(sFxDataMgr.GetExpression(expression.arg1), data);
            ParseExpression(sFxDataMgr.GetExpression(expression.arg2), data);
        } break;
        case operandGETTYPE: {    //'%(arg1)s.GetTypeID()'  --used by SRLG in AORSM/RORSM
            ParseExpression(sFxDataMgr.GetExpression(expression.arg1), data);
            ParseExpression(sFxDataMgr.GetExpression(expression.arg2), data);
        } break;
        case operandLG: {    //48, '%(arg1)s.LocationGroup.%(arg2)s'  -- specify a group by grpID in a location'
            data.domain = dgmDomainGroup;   //preliminary....will need work later.
            ParseExpression(sFxDataMgr.GetExpression(expression.arg1), data);   //domain
            ParseExpression(sFxDataMgr.GetExpression(expression.arg2), data);   //groupID
        } break;
        case operandLS: {    //49, '%(arg1)s.SkillRequiredLocationGroup[%(arg2)s]'  --  specify a group by skillID in a location...not sure how to do this
            data.domain = dgmDomainSkill;   //preliminary....will need work later.
            ParseExpression(sFxDataMgr.GetExpression(expression.arg1), data);   //domain
            ParseExpression(sFxDataMgr.GetExpression(expression.arg2), data);   //skillID
        } break;

        // effect function calls.
        // here is where we'll actually add the modifier data to the map
        case operandAIM: {    //'AddItemModifier(env,%(arg1)s, %(arg2)s)'
            Expression arg1Expression = sFxDataMgr.GetExpression(expression.arg1);
            ParseExpression(sFxDataMgr.GetExpression(arg1Expression.arg1), data);
            ParseExpression(sFxDataMgr.GetExpression(arg1Expression.arg2), data);
            ParseExpression(sFxDataMgr.GetExpression(expression.arg2), data);
            m_modifiers.emplace(std::pair<uint8, fxData>(data.assoc, data));
        } break;
        // these arent completely correct yet.  testing
        case operandALGM: {    //7,(%(arg1)s).AddLocationGroupModifier (%(arg2)s)
            ParseExpression(sFxDataMgr.GetExpression(expression.arg1), data);
            ParseExpression(sFxDataMgr.GetExpression(expression.arg2), data);
            m_modifiers.emplace(std::pair<uint8, fxData>(data.assoc, data));
        } break;
        case operandALM: {    //8,(%(arg1)s).AddLocationModifier (%(arg2)s)
            ParseExpression(sFxDataMgr.GetExpression(expression.arg1), data);
            ParseExpression(sFxDataMgr.GetExpression(expression.arg2), data);
            m_modifiers.emplace(std::pair<uint8, fxData>(data.assoc, data));
        } break;
        case operandALRSM: {    //9,(%(arg1)s).AddLocationRequiredSkillModifier(%(arg2)s)
            ParseExpression(sFxDataMgr.GetExpression(expression.arg1), data);
            ParseExpression(sFxDataMgr.GetExpression(expression.arg2), data);
            m_modifiers.emplace(std::pair<uint8, fxData>(data.assoc, data));
        } break;
        case operandAORSM: {    //11,(%(arg1)s).AddOwnerRequiredSkillModifier(%(arg2)s)
            ParseExpression(sFxDataMgr.GetExpression(expression.arg1), data);
            ParseExpression(sFxDataMgr.GetExpression(expression.arg2), data);
            m_modifiers.emplace(std::pair<uint8, fxData>(data.assoc, data));
        } break;
        /** @todo  will have to figure out how to remove modifiers and delete from the map(s) */
        // why?  just reset everything....
        case operandRIM: {    //'RemoveItemModifier(env,%(arg1)s, %(arg2)s)'
            Expression arg1Expression = sFxDataMgr.GetExpression(expression.arg1);
            ParseExpression(sFxDataMgr.GetExpression(arg1Expression.arg1), data);
            ParseExpression(sFxDataMgr.GetExpression(arg1Expression.arg2), data);
            ParseExpression(sFxDataMgr.GetExpression(expression.arg2), data);
            //m_modifiers.erase();
        } break;
        case operandRLGM: {    //59,(%(arg1)s).RemoveLocationGroupModifier (%(arg2)s)
            ParseExpression(sFxDataMgr.GetExpression(expression.arg1), data);
            ParseExpression(sFxDataMgr.GetExpression(expression.arg2), data);
        } break;
        case operandRLM: {    //60, (%(arg1)s).RemoveLocationModifier (%(arg2)s)
            ParseExpression(sFxDataMgr.GetExpression(expression.arg1), data);
            ParseExpression(sFxDataMgr.GetExpression(expression.arg2), data);
        } break;
        case operandRLRSM: {    //61,(%(arg1)s).RemoveLocationRequiredSkillModifier(%(arg2)s)
            ParseExpression(sFxDataMgr.GetExpression(expression.arg1), data);
            ParseExpression(sFxDataMgr.GetExpression(expression.arg2), data);
        } break;
        case operandRORSM: {    //62, (%(arg1)s).RemoveOwnerRequiredSkillModifier(%(arg2)s)
            ParseExpression(sFxDataMgr.GetExpression(expression.arg1), data);
            ParseExpression(sFxDataMgr.GetExpression(expression.arg2), data);
        } break;
        default: {              // in case the op hasnt been defined, make a note here (should not hit)
            std::ostringstream ret;
            Operand operand = sFxDataMgr.GetOperand(expression.operandID);
            ret << "Operand id:" << expression.operandID << " key:" << operand.operandKey;
            if (operand.format == "")
                ret << " - has not been defined";
            else                // % {'arg1': arg1, 'arg2': arg2, 'value': expression.expressionValue}
                ret << " *needsWork*";
            sLog.Error("Ship::ParseExpression", "%s", ret.str().c_str());
        } break;
    }
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
    m_destiny = new DestinyManager(this);
    m_podShipID = 0;
    m_processTimer.Start(m_processTimerTick);
    _log(SHIP__INFO, "Created ShipSE %p for item %u", this, self->itemID());
}

Ship::~Ship() {
    m_targMgr->DoDestruction();
    SafeDelete(m_destiny);
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
        if (sConfig.server.UseProfiling)
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
        if (sConfig.server.UseProfiling)
            sProfile.AddTime(_shipProfile, GetTimeUSeconds() - profileStartTime);
    }

    // now, process the modules.
    m_shipRef->ProcessModules();
}

void Ship::PayInsurance() {
    m_self->GetPilot()->GetChar()->AlterBalance(m_db.GetShipInsurancePayout(GetSelf()->itemID()));
    m_db.DeleteInsuranceByShipID(GetSelf()->itemID());
}

void Ship::ResetShipSystemMgr(SystemManager* pSystem)
{
    m_system = pSystem;
    //SafeDelete(m_destiny);
    //m_destiny = new DestinyManager(this);
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
        mass.cloak = 0;
        mass.Harmonic = -1.0f;
        mass.corporationID = GetCorporationID();
        mass.allianceID = GetAllianceID();
        into.Append( mass );
    DataSector data;
        data.intertia = m_destiny->GetInertia();
        data.maxVelocity = m_destiny->GetMaxVelocity();
        data.velocity_x = m_destiny->GetVelocity().x;
        data.velocity_y = m_destiny->GetVelocity().y;
        data.velocity_z = m_destiny->GetVelocity().z;
        data.speedfraction = m_destiny->GetSpeedFraction();
        into.Append( data );
    if (mode == DSTBALL_WARP) {
        GPoint target = m_destiny->GetTargetPoint();
        DSTBALL_WARP_Struct warp;
            warp.formationID = 0xFF;
            warp.effectStamp = -1; // m_destiny->GetStateStamp();   //timestamp when warp started...not working right yet.
            warp.x = target.x;
            warp.y = target.y;
            warp.z = target.z;
            warp.ownerID = m_destiny->GetWarpSpeed();       //ship warp speed x10  (dont ask...this is what it is...more dumb ccp shit)
            warp.followRange = 0; //m_destiny->GetDistance();
            warp.followID = (m_destiny->GetTargetID() ? m_destiny->GetTargetID() : 0);
        into.Append( warp );
    } else if (mode == DSTBALL_FOLLOW) {
        DSTBALL_FOLLOW_Struct follow;
            follow.followID = m_destiny->GetTargetID();
            follow.followRange = m_destiny->GetFollowDistance();
            follow.formationID = 0xFF;
        into.Append( follow );
    } else if (mode == DSTBALL_ORBIT) {
        DSTBALL_ORBIT_Struct orbit;
            orbit.followID = m_destiny->GetTargetID();
            orbit.followRange = m_destiny->GetFollowDistance();
            orbit.formationID = 0xFF;
        into.Append( orbit );
    } else if (mode == DSTBALL_GOTO) {
        GPoint target = m_destiny->GetTargetPoint();
        DSTBALL_GOTO_Struct go;
            go.x = target.x;
            go.y = target.y;
            go.z = target.z;
        into.Append( go );
    } else {
        DSTBALL_STOP_Struct main;
            main.formationID = 0xFF;
        into.Append( main );
    }

    std::string modeStr = "Goto";
    switch (head.mode) {
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

    _log(SHIP__INFO, "Ship::EncodeDestiny(): %s - id:%u, mode:%s, flags:0x%X", GetName(), head.entityID, modeStr.c_str(), head.flags);
}

void Ship::MakeDamageState(DoDestinyDamageState &into) {
    into.shield = (m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float() +7;
    into.timestamp = Win32TimeNow();
    into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float());
}

PyDict* Ship::MakeSlimItem() {
    _log(COMMON__WARNING, "MakeSlimItem for Ship %s(%u)", m_self->itemName().c_str(), m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",               new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",               new PyInt(m_self->typeID()));
        slim->SetItemString("name",                 new PyString(m_self->itemName()));
        slim->SetItemString("ownerID",              new PyInt(m_self->ownerID()));
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
    m_self->GetInventory()->FindByFlagRange(flagHiSlot0, flagHiSlot7, items);
    //m_self->GetInventory()->FindByFlagRange(flagSubSystem0, flagSubSystem7, items);
    if (!items.empty()) {
        PyList *l = new PyList();
        for (auto cur : items) {
            l->AddItem(new_tuple(cur->itemID(), cur->typeID()));
        }

        slim->SetItemString("modules", l);
        PySafeDecRef(l);
    }

    if (is_log_enabled(DESTINY__DEBUG)) {
        _log( DESTINY__DEBUG, "Ship::MakeSlimItem()", "%s(%u)", GetName(), GetID());
        slim->Dump(DESTINY__DEBUG, "     ");
    }
    return slim;
}
