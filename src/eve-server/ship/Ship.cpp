
#include "Client.h"
#include "EVEServerConfig.h"
#include "Profile.h"
#include "character/Character.h"
#include "ship/DestinyManager.h"
#include "ship/modules/ModuleManager.h"
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
    if (_weaponType != NULL)
        assert(_weaponType->id() == stData.mWeaponTypeID);
    if (_miningType != NULL)
        assert(_miningType->id() == stData.mMiningTypeID);
    if (_skillType != NULL)
        assert(_skillType->id() == stData.mSkillTypeID);
}

ShipType *ShipType::Load(ItemFactory &factory, uint32 shipTypeID)
{
    return ItemType::Load<ShipType>( factory, shipTypeID );
}

template<class _Ty>
_Ty *ShipType::_LoadShipType(ItemFactory &factory, uint32 shipTypeID,
                             // ItemType stuff:
                             const ItemGroup &group, const TypeData &data,
                             // ShipType stuff:
                             const ItemType *weaponType, const ItemType *miningType, const ItemType *skillType, const ShipTypeData &stData)
{
    // we have all the data, let's create new object
    return new ShipType(shipTypeID, group, data, weaponType, miningType, skillType, stData );
}

/*
 * ShipItem
 */
ShipItem::ShipItem(ItemFactory &_factory, uint32 _shipID, const ShipType &_shipType, const ItemData &_data)
: InventoryItem(_factory, _shipID, _shipType, _data),
m_processTimerTick(SHIP_PROCESS_TICK_MS),   //5s
m_processTimer(m_processTimerTick),
m_pilot(nullptr),
m_ModuleManager(nullptr)
{
    m_IsLoaded = false;
    m_processTimer.Start(m_processTimerTick);
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

template<class _Ty>
RefPtr<_Ty> ShipItem::_LoadShip(ItemFactory &factory, uint32 shipID, const ShipType &shipType, const ItemData &data)
{
    return ShipItemRef( new ShipItem(factory, shipID, shipType, data ));
}

ShipItemRef ShipItem::Spawn(ItemFactory &factory, ItemData &data) {
    uint32 shipID = ShipItem::CreateItemID( factory, data );
    if ( shipID == 0 )
        return ShipItemRef();

    ShipItemRef sShipRef = ShipItem::Load( factory, shipID );

    // Create default dynamic attributes in the AttributeMap:
    //sShipRef->SetAttribute(AttrIsOnline,                            false, false);
    sShipRef->SetAttribute(AttrArmorDamage,                         0.0, false);
    sShipRef->SetAttribute(AttrInertia,                             1, false);
    sShipRef->SetAttribute(AttrMass,                                sShipRef->type().mass(), false);
    sShipRef->SetAttribute(AttrRadius,                              sShipRef->type().radius(), false);
    sShipRef->SetAttribute(AttrVolume,                              sShipRef->type().volume(), false);
    sShipRef->SetAttribute(AttrCapacity,                            sShipRef->type().capacity(), false);
    sShipRef->SetAttribute(AttrShieldCharge,                        sShipRef->GetAttribute(AttrShieldCapacity), false);
    sShipRef->SetAttribute(AttrCapacitorCharge,                     sShipRef->GetAttribute(AttrCapacitorCapacity), false);

    // Check for existence of some attributes that may or may not have already been loaded and set them
    // to default values:
    if (!sShipRef->HasAttribute(AttrDamage))                        sShipRef->SetAttribute(AttrDamage, 0.0f, false );
    if (!sShipRef->HasAttribute(AttrMaximumRangeCap))               sShipRef->SetAttribute(AttrMaximumRangeCap, ((double)BUBBLE_RADIUS_METERS), false);
    if (!sShipRef->HasAttribute(AttrArmorMaxDamageResonance))       sShipRef->SetAttribute(AttrArmorMaxDamageResonance, 1.0f, false);
    if (!sShipRef->HasAttribute(AttrShieldMaxDamageResonance))      sShipRef->SetAttribute(AttrShieldMaxDamageResonance, 1.0f, false);
    if (!sShipRef->HasAttribute(AttrWarpSpeedMultiplier))           sShipRef->SetAttribute(AttrWarpSpeedMultiplier, 1.0f, false);
    // Warp Scramble Status of the ship (most ships have zero warp scramble status, but some already have it defined):
    if (!sShipRef->HasAttribute(AttrWarpScrambleStatus))            sShipRef->SetAttribute(AttrWarpScrambleStatus, 0.0f, false);

    // Shield Resonance
    if (!sShipRef->HasAttribute(AttrShieldEmDamageResonance))       sShipRef->SetAttribute(AttrShieldEmDamageResonance, 1.0, false);
    if (!sShipRef->HasAttribute(AttrShieldExplosiveDamageResonance)) sShipRef->SetAttribute(AttrShieldExplosiveDamageResonance, 1.0, false);
    if (!sShipRef->HasAttribute(AttrShieldKineticDamageResonance))  sShipRef->SetAttribute(AttrShieldKineticDamageResonance, 1.0, false);
    if (!sShipRef->HasAttribute(AttrShieldThermalDamageResonance))  sShipRef->SetAttribute(AttrShieldThermalDamageResonance, 1.0, false);
    if (!sShipRef->HasAttribute(AttrArmorEmDamageResonance))        sShipRef->SetAttribute(AttrArmorEmDamageResonance, 1.0, false);
    if (!sShipRef->HasAttribute(AttrArmorExplosiveDamageResonance)) sShipRef->SetAttribute(AttrArmorExplosiveDamageResonance, 1.0, false);
    if (!sShipRef->HasAttribute(AttrArmorKineticDamageResonance))   sShipRef->SetAttribute(AttrArmorKineticDamageResonance, 1.0, false);
    if (!sShipRef->HasAttribute(AttrArmorThermalDamageResonance))   sShipRef->SetAttribute(AttrArmorThermalDamageResonance, 1.0, false);
    if (!sShipRef->HasAttribute(AttrEmDamageResonance))             sShipRef->SetAttribute(AttrEmDamageResonance, 1.0, false);
    if (!sShipRef->HasAttribute(AttrExplosiveDamageResonance))      sShipRef->SetAttribute(AttrExplosiveDamageResonance, 1.0, false);
    if (!sShipRef->HasAttribute(AttrKineticDamageResonance))        sShipRef->SetAttribute(AttrKineticDamageResonance, 1.0, false);
    if (!sShipRef->HasAttribute(AttrThermalDamageResonance))        sShipRef->SetAttribute(AttrThermalDamageResonance, 1.0, false);
    if (!sShipRef->HasAttribute(AttrTurretSlotsLeft))               sShipRef->SetAttribute(AttrTurretSlotsLeft, 0, false);
    if (!sShipRef->HasAttribute(AttrLauncherSlotsLeft))             sShipRef->SetAttribute(AttrLauncherSlotsLeft, 0, false);

    sShipRef->SetAttribute(AttrCpuLoad, 0.0f, false);
    sShipRef->SetAttribute(AttrPowerLoad, 0.0f, false);

    sShipRef->SaveAttributes();

    return sShipRef;
}

uint32 ShipItem::CreateItemID(ItemFactory &factory, ItemData &data) {
    // make sure it's a ship
    const ShipType *st = factory.GetShipType(data.typeID);
    if (!st) return 0;

    return InventoryItem::CreateItemID(factory, data);
}

bool ShipItem::_Load()
{
    if (typeID() == EVEDB::invTypes::typeCapsule) return true;
    if (m_IsLoaded && m_ModuleManager) return true;
    // load attributes
    if (!InventoryItem::_Load()) return false;
    // load contents
    if (!m_inventory->LoadContents(&m_factory))  return false;

    /** @todo  apply ship and skill bonuses to hold capacities here */

	// fill cargo holds data here:
	if ( HasAttribute(AttrCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagCargoHold,mAttributeMap.GetAttribute(AttrCapacity).get_float()));
	if ( HasAttribute(AttrDroneCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagDroneBay,mAttributeMap.GetAttribute(AttrDroneCapacity).get_float()));
	if ( HasAttribute(AttrSpecialFuelBayCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedFuelBay,mAttributeMap.GetAttribute(AttrSpecialFuelBayCapacity).get_float()));
	if ( HasAttribute(AttrSpecialOreHoldCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedOreHold,mAttributeMap.GetAttribute(AttrSpecialOreHoldCapacity).get_float()));
	if ( HasAttribute(AttrSpecialGasHoldCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedGasHold,mAttributeMap.GetAttribute(AttrSpecialGasHoldCapacity).get_float()));
	if ( HasAttribute(AttrSpecialMineralHoldCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedMineralHold,mAttributeMap.GetAttribute(AttrSpecialMineralHoldCapacity).get_float()));
	if ( HasAttribute(AttrSpecialSalvageHoldCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedSalvageHold,mAttributeMap.GetAttribute(AttrSpecialSalvageHoldCapacity).get_float()));
	if ( HasAttribute(AttrSpecialShipHoldCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedShipHold,mAttributeMap.GetAttribute(AttrSpecialShipHoldCapacity).get_float()));
	if ( HasAttribute(AttrSpecialSmallShipHoldCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedSmallShipHold,mAttributeMap.GetAttribute(AttrSpecialSmallShipHoldCapacity).get_float()));
	if ( HasAttribute(AttrSpecialLargeShipHoldCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedLargeShipHold,mAttributeMap.GetAttribute(AttrSpecialLargeShipHoldCapacity).get_float()));
	if ( HasAttribute(AttrSpecialIndustrialShipHoldCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedIndustrialShipHold,mAttributeMap.GetAttribute(AttrSpecialIndustrialShipHoldCapacity).get_float()));
	if ( HasAttribute(AttrSpecialAmmoHoldCapacity))
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedAmmoHold,mAttributeMap.GetAttribute(AttrSpecialAmmoHoldCapacity).get_float()));

	UpdateHoldsUsedVolume();

    return (m_IsLoaded = true);
}

void ShipItem::Init()
{
    Character* pChar = m_pilot->GetChar().get();
    if (!pChar) {
        _log(SHIP__WARNING, "ShipItem %s(%u) does not have a pilot.", itemName().c_str(), itemID());
        return;
    }

    /** @todo These all still need to have ship bonuses applied */
    /** @todo this will need to be changed to use skill modifiers when i get them working.... */
    double pg = GetDefaultAttribute(AttrPowerOutput).get_int();
    double cpu = GetDefaultAttribute(AttrCpuOutput).get_float();
    double hullHP = GetDefaultAttribute(AttrHP).get_int();
    double armorHP = GetDefaultAttribute(AttrArmorHP).get_float();
    double capCapacity = GetDefaultAttribute(AttrCapacitorCapacity).get_float();  // default value from db
    double capChargeRate = GetDefaultAttribute(AttrRechargeRate).get_float(); // default value from db
    double shieldCapacity = GetDefaultAttribute(AttrShieldCapacity).get_float();
    double shieldChargeRate = GetDefaultAttribute(AttrShieldRechargeRate).get_float();

    pg *=  (1 + (0.05 * (pChar->GetSkillLevel(skillEngineering, true))));                       // 5% increase
    cpu *=  (1 + (0.05 * (pChar->GetSkillLevel(skillElectronics, true))));                      // 5% increase
    hullHP *=  (1 + (0.05 * (pChar->GetSkillLevel(skillMechanics, true))));                     // 5% increase
    armorHP *=  (1 + (0.05 * (pChar->GetSkillLevel(skillHullUpgrades, true))));                 // 5% increase
    capCapacity *=  (1 + (0.05 * (pChar->GetSkillLevel(skillEnergyManagement, true))));         // 5% increase
    capChargeRate *=  (1 - (0.05 * (pChar->GetSkillLevel(skillEnergySystemsOperation, true)))); // 5% decrease
    shieldCapacity *=  (1 + (0.05 * (pChar->GetSkillLevel(skillShieldManagement, true))));      // 5% increase
    shieldChargeRate *=  (1 - (0.05 * (pChar->GetSkillLevel(skillShieldOperation, true))));     // 5% decrease

    // add checks for implants here.
    //  ship bonuses are found in dgmShipBonusModifiers
    //  skill bonuses are found in dgmSkillBonusModifiers

    /* to reset for new pilot:
     * offline all modules
     * reset ship attribs
     * add new pilot skills
     * online all modules
     * save current attribs
     */
    /* this should probably be done in CharacterLeavingShip()
    if (m_ModuleManager)
        m_ModuleManager->OfflineAll(); */

    // reset basic ship attribs before updating modules   this is catchall incase of server crash (and subsequent data corruption)
    ResetAttribute(AttrCpuLoad);
    ResetAttribute(AttrPowerLoad);
    ResetAttribute(AttrUpgradeLoad);
    ResetAttribute(AttrUpgradeSlotsLeft);
    ResetAttribute(AttrShieldEmDamageResonance);
    ResetAttribute(AttrShieldExplosiveDamageResonance);
    ResetAttribute(AttrShieldKineticDamageResonance);
    ResetAttribute(AttrShieldThermalDamageResonance);
    ResetAttribute(AttrArmorEmDamageResonance);
    ResetAttribute(AttrArmorExplosiveDamageResonance);
    ResetAttribute(AttrArmorKineticDamageResonance);
    ResetAttribute(AttrArmorThermalDamageResonance);
    ResetAttribute(AttrEmDamageResonance);
    ResetAttribute(AttrExplosiveDamageResonance);
    ResetAttribute(AttrKineticDamageResonance);
    ResetAttribute(AttrThermalDamageResonance);

    SetAttribute(AttrHP, hullHP);
    SetAttribute(AttrArmorHP, armorHP);
    SetAttribute(AttrCpuOutput, cpu);
    SetAttribute(AttrPowerOutput, pg);
    SetAttribute(AttrRechargeRate, capChargeRate);
    SetAttribute(AttrShieldCapacity, shieldCapacity);
    SetAttribute(AttrCapacitorCharge, capCapacity);
    SetAttribute(AttrShieldRechargeRate,shieldChargeRate );

    // allocate the module manager, only the first time:
    if (!m_ModuleManager)
        m_ModuleManager = new ModuleManager(this);

    m_ModuleManager->Initialize();

    /** @todo need to check for ship damage status BEFORE or INSTEAD of calling this.
     */
    //set everything to full AFTER modules possibably update ship stats
    if (sConfig.server.testServer)
        Heal();
}

void ShipItem::InitPod() {
    // allocate the module manager, only the first time:
    if (!m_ModuleManager) {
        m_ModuleManager = new ModuleManager(this);
        m_ModuleManager->Initialize();
    }
    if (sConfig.server.testServer)
        Heal();
}

void ShipItem::SetPlayer(Client* pClient) {
    if (!pClient)
        if (m_ModuleManager)
            m_ModuleManager->CharacterLeavingShip();
    m_pilot = pClient;
    if (!m_pilot)
        return;
    Init();
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

void ShipItem::_IncreaseCargoHoldsUsedVolume(EVEItemFlags flag, double volumeToConsume)
{
    if ( m_cargoHoldsUsedVolumeByFlag.find(flag) != m_cargoHoldsUsedVolumeByFlag.end())
        m_cargoHoldsUsedVolumeByFlag.find(flag)->second += volumeToConsume;
    else {
        _log(SHIP__ERROR, "HoldsUsedVolume(+) given flag not found in current map - %u", flag);
        throw PyException( MakeCustomError( "ERROR!  Illegal flag '%u' specified!", flag ));
    }
}

void ShipItem::_DecreaseCargoHoldsUsedVolume(EVEItemFlags flag, double volumeToConsume)
{
    if ( m_cargoHoldsUsedVolumeByFlag.find(flag) != m_cargoHoldsUsedVolumeByFlag.end())
        m_cargoHoldsUsedVolumeByFlag.find(flag)->second -= volumeToConsume;
    else {
        _log(SHIP__ERROR, "HoldsUsedVolume(-) given flag not found in current map - %u", flag);
        throw PyException( MakeCustomError( "ERROR!  Illegal flag '%u' specified!", flag ));
    }
}

void ShipItem::Delete()
{
    // delete contents first
    m_inventory->DeleteContents( m_factory );

    InventoryItem::Delete();
}

double ShipItem::GetRemainingVolumeByFlag(EVEItemFlags flag) const
{   /** @todo should this be run thru inventory?   -- yes!  put on list todo later...*/
    switch( flag ) {
        case flagAutoFit:
        case flagCargoHold:
            return (GetAttribute(AttrCapacity).get_float() - m_cargoHoldsUsedVolumeByFlag.find(flagCargoHold)->second);
            break;

        case flagDroneBay:
            return (GetAttribute(AttrDroneCapacity).get_float() - m_cargoHoldsUsedVolumeByFlag.find(flagDroneBay)->second);
            break;

        case flagSpecializedFuelBay:
            return (GetAttribute(AttrSpecialFuelBayCapacity).get_float() - m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedFuelBay)->second);
            break;

        case flagSpecializedOreHold:
            return (GetAttribute(AttrSpecialOreHoldCapacity).get_float() - m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedOreHold)->second);
            break;

        case flagSpecializedGasHold:
            return (GetAttribute(AttrSpecialGasHoldCapacity).get_float() - m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedGasHold)->second);
            break;

        case flagSpecializedMineralHold:
            return (GetAttribute(AttrSpecialMineralHoldCapacity).get_float() - m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedMineralHold)->second);
            break;

        case flagSpecializedSalvageHold:
            return (GetAttribute(AttrSpecialSalvageHoldCapacity).get_float() - m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedSalvageHold)->second);
            break;

        case flagSpecializedShipHold:
            return (GetAttribute(AttrSpecialShipHoldCapacity).get_float() - m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedShipHold)->second);
            break;

        case flagSpecializedSmallShipHold:
            return (GetAttribute(AttrSpecialSmallShipHoldCapacity).get_float() - m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedSmallShipHold)->second);
            break;

        case flagSpecializedLargeShipHold:
            return (GetAttribute(AttrSpecialLargeShipHoldCapacity).get_float() - m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedLargeShipHold)->second);
            break;

        case flagSpecializedIndustrialShipHold:
            return (GetAttribute(AttrSpecialIndustrialShipHoldCapacity).get_float() - m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedIndustrialShipHold)->second);
            break;

        case flagSpecializedAmmoHold:
            return (GetAttribute(AttrSpecialAmmoHoldCapacity).get_float() - m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedAmmoHold)->second);
            break;

        default:
            return 0.0;
            break;
    }
}

bool ShipItem::ValidateAddItem(EVEItemFlags flag, InventoryItemRef item)
{
    CharacterRef character = m_pilot->GetChar();

    if (flag == flagDroneBay) {
        if ( item->categoryID() != EVEDB::invCategories::Drone ) {
            throw PyException( MakeCustomError( "Item Cannot be stowed in the Drone Bay" ));
            return false;
        }
    } else if (flag == flagShipHangar) {
        if (GetAttribute(AttrHasShipMaintenanceBay) != 0) {
            throw PyException( MakeCustomError( "%s has no ship maintenance bay.", item->itemName().c_str()) );
            return false;
        }
        if (item->categoryID() != EVEDB::invCategories::Ship) {
            throw PyException( MakeCustomError( "Only ships may be placed into ship maintenance bay." ));
            return false;
        }
    } else if (flag == flagHangar) {
        if (GetAttribute(AttrHasCorporateHangars) != 0) {
            throw PyException( MakeCustomError( "%s has no corporate hangars.", item->itemName().c_str()) );
            return false;
        }
    } else if ((flag >= flagLowSlot0) && (flag <= flagHiSlot7)) {
        if (m_pilot->IsClient())
            if (!Skill::FitModuleSkillCheck(item, character)) {
                throw PyException( MakeCustomError( "You do not have the required skills to fit this \n%s", item->itemName().c_str()) );
                return false;
            }
            if (!ValidateItemSpecifics(item)) {
                throw PyException( MakeCustomError( "Your ship cannot equip this module" ));
                return false;
            }
            if (item->categoryID() == EVEDB::invCategories::Charge) {
                if (m_ModuleManager->GetModule(flag)) {
                    InventoryItemRef module = m_ModuleManager->GetModule(flag)->getItem();
                    if (module->GetAttribute(AttrChargeSize) != item->GetAttribute(AttrChargeSize)) {
                        sLog.Error("Ship::ValidateAddItem", "Charge size %u for %s does not match Module size %u for %s.",
                                   item->GetAttribute(AttrChargeSize).get_int(), item->itemName().c_str(),
                                   module->GetAttribute(AttrChargeSize).get_int(), module->itemName().c_str()
                        );
                        throw PyException( MakeCustomError( "The charge is not the correct size for this module." ));
                        return false;
                    }
                    if (module->GetAttribute(AttrChargeGroup1) != item->groupID() &&
                        module->GetAttribute(AttrChargeGroup2) != item->groupID() &&
                        module->GetAttribute(AttrChargeGroup3) != item->groupID() &&
                        module->GetAttribute(AttrChargeGroup4) != item->groupID() &&
                        module->GetAttribute(AttrChargeGroup5) != item->groupID()) {
                        throw PyException( MakeCustomError( "Incorrect charge type for this module."));
                    return false;
                        }
                        // NOTE: Module Manager will check for actual room to load charges and make stack splits, or reject loading altogether
                } else {
                    throw PyException( MakeCustomError( "Module at flag '%u' does not exist!", flag ));
                    return false;
                }
            } else {
                if (m_ModuleManager->IsSlotOccupied(flag)) {
                    throw PyException( MakeUserError( "SlotAlreadyOccupied" ));
                    return false;
                }
            }
    } else if ((flag >= flagRigSlot0) && (flag <= flagRigSlot7)) {
        if (m_pilot->IsClient()) {
            if (!Skill::FitModuleSkillCheck(item, character)) {
                throw PyException( MakeCustomError( "You do not have the required skills to fit this \n%s", item->itemName().c_str()) );
                return false;
            }
            if (GetAttribute(AttrRigSize) != item->GetAttribute(AttrRigSize)) {
                throw PyException( MakeCustomError( "Your ship cannot fit this size module" ));
                return false;
            }
            if (GetAttribute(AttrUpgradeLoad) + item->GetAttribute(AttrUpgradeCost) > GetAttribute(AttrUpgradeCapacity)) {
                throw PyException( MakeCustomError( "Your ship cannot handle the extra calibration" ));
                return false;
            }
        }
    } else if ((flag >= flagSubSystem0) && (flag <= flagSubSystem7)) {
        if (m_pilot->IsClient())
            if (!Skill::FitModuleSkillCheck(item, character)) {
                throw PyException( MakeCustomError( "You do not have the required skills to fit this \n%s", item->itemName().c_str()) );
                return false;
            }
    } else {
        // Handle any other flag, legal or not by virtue of GetRemainingVolumeByFlag() and GetCapacity() that handle supported capacity types:
        // (unsupported or illegal flags report capacity of 0.0, so are automatically rejected)
        if ((GetRemainingVolumeByFlag(flag) < (item->GetAttribute(AttrVolume).get_float() * item->quantity()))) {
            throw PyException( MakeCustomError( "Not enough cargo space!<br><br>flag = %u", (uint32)flag));
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
        if (cur->Populate(entry)) {
            if (cur->groupID() == EVEDB::invCategories::Charge) {
                PyTuple* tuple = new PyTuple(3);
                tuple->SetItem(0, new PyInt(cur->itemID()));
                tuple->SetItem(1, new PyInt(cur->flag()));
                tuple->SetItem(2, new PyInt(cur->typeID()));
                result->SetItem(tuple, new PyObject("util.KeyVal", entry.Encode()));
            } else {
                result->SetItem(new PyInt(cur->itemID()), new PyObject("util.KeyVal", entry.Encode()));
            }
        } else
            _log( SHIP__ERROR, "%s(%u): Failed to load item %u for ShipGetInfo", itemName().c_str(), itemID(), cur->itemID());
    }

    return result;
}

PyDict* ShipItem::GetShipState() {
    if (!m_inventory->LoadContents(&m_factory)) {
        _log(INV__ERROR, "%s(%u): Failed to load contents for GetShipState", itemName().c_str(), itemID());
        return nullptr;
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
    if (!m_inventory->LoadContents(&m_factory)) {
        _log(INV__ERROR, "%s(%u): Failed to load contents for GetChargeState", itemName().c_str(), itemID());
        return nullptr;
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
    if ( item->flag() >= flagSlotFirst &&
        item->flag() <= flagSlotLast &&
        item->categoryID() != EVEDB::invCategories::Charge)
    {
        // make singleton
        item->ChangeSingleton( true );
    }
    m_inventory->AddItem( item );
}

bool ShipItem::ValidateBoardShip(ShipItemRef ship, CharacterRef character) {
    SkillRef requiredSkill;
    EvilNumber skillTypeID;

    if ( ship->HasAttribute(AttrRequiredSkill1, skillTypeID))
        if (!character->HasSkillTrainedToLevel( skillTypeID.get_int(), ship->GetAttribute(AttrRequiredSkill1Level).get_int())) return false;
    if ( ship->HasAttribute(AttrRequiredSkill2, skillTypeID))
        if (!character->HasSkillTrainedToLevel( skillTypeID.get_int(), ship->GetAttribute(AttrRequiredSkill2Level).get_int())) return false;
    if ( ship->HasAttribute(AttrRequiredSkill3, skillTypeID))
        if (!character->HasSkillTrainedToLevel( skillTypeID.get_int(), ship->GetAttribute(AttrRequiredSkill3Level).get_int())) return false;
    if ( ship->HasAttribute(AttrRequiredSkill4, skillTypeID))
        if (!character->HasSkillTrainedToLevel( skillTypeID.get_int(), ship->GetAttribute(AttrRequiredSkill4Level).get_int())) return false;
    if ( ship->HasAttribute(AttrRequiredSkill5, skillTypeID))
        if (!character->HasSkillTrainedToLevel( skillTypeID.get_int(), ship->GetAttribute(AttrRequiredSkill5Level).get_int())) return false;
    if ( ship->HasAttribute(AttrRequiredSkill6, skillTypeID))
        if (!character->HasSkillTrainedToLevel( skillTypeID.get_int(), ship->GetAttribute(AttrRequiredSkill6Level).get_int())) return false;
    return true;
}

void ShipItem::SaveShip()
{
    SaveItem();                         // Save ship info
    mAttributeMap.SaveShipState();      // save ship damage
    m_ModuleManager->SaveModules();     // Save item info for modules fitted to this ship
}

bool ShipItem::ValidateItemSpecifics(InventoryItemRef equip) {

    uint32 groupID = m_pilot->GetShip()->groupID();
    uint32 typeID = m_pilot->GetShip()->typeID();

    EvilNumber canFitShipGroup1, canFitShipGroup2, canFitShipGroup3, canFitShipGroup4;
    EvilNumber canFitShipType1, canFitShipType2, canFitShipType3, canFitShipType4;

    // If a ship group restriction is specified the item
    // must be able to fit to at least one ship group.

    if (equip->HasAttribute(AttrCanFitShipGroup1, canFitShipGroup1) ||
        equip->HasAttribute(AttrCanFitShipGroup2, canFitShipGroup2) ||
        equip->HasAttribute(AttrCanFitShipGroup3, canFitShipGroup3) ||
        equip->HasAttribute(AttrCanFitShipGroup4, canFitShipGroup4)){
        _log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - Beginning the validation:");
    _log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - AttrCanFitShipGroup1 = %s", equip->HasAttribute(AttrCanFitShipGroup1, canFitShipGroup1) ? "True":"False");
    _log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - AttrCanFitShipGroup2 = %s", equip->HasAttribute(AttrCanFitShipGroup2, canFitShipGroup2) ? "True":"False");
    _log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - AttrCanFitShipGroup3 = %s", equip->HasAttribute(AttrCanFitShipGroup3, canFitShipGroup3) ? "True":"False");
    _log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - AttrCanFitShipGroup4 = %s", equip->HasAttribute(AttrCanFitShipGroup4, canFitShipGroup4) ? "True":"False");
    if ( (canFitShipGroup1 != groupID) && (canFitShipGroup2 != groupID) && (canFitShipGroup3 != groupID) && (canFitShipGroup4 != groupID)){
        _log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - No attribute found. groupID = %i", groupID);
        return false;
    }
    else
        _log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - Validation passed. Fitting the module");
        }

        // If a ship type restriction is specified the item
        // must be able to fit to at least one ship type.
        if (equip->HasAttribute(AttrCanFitShipType1, canFitShipType1) ||
            equip->HasAttribute(AttrCanFitShipType2, canFitShipType2) ||
            equip->HasAttribute(AttrCanFitShipType3, canFitShipType3) ||
            equip->HasAttribute(AttrCanFitShipType4, canFitShipType4)){
            _log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - Beginning the validation:");
        _log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - AttrCanFitShipType1 = %s", equip->HasAttribute(AttrCanFitShipType1, canFitShipType1) ? "True":"False");
        _log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - AttrCanFitShipType2 = %s", equip->HasAttribute(AttrCanFitShipType2, canFitShipType2) ? "True":"False");
        _log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - AttrCanFitShipType3 = %s", equip->HasAttribute(AttrCanFitShipType3, canFitShipType3) ? "True":"False");
        _log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - AttrCanFitShipType4 = %s", equip->HasAttribute(AttrCanFitShipType4, canFitShipType4) ? "True":"False");
        if ( (canFitShipType1 != typeID) && (canFitShipType2 != typeID) && (canFitShipType3 != typeID) && (canFitShipType4 != typeID)){
            _log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - No attribute found. typeID = %i", typeID);
            return false;
        }
        else
            _log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - Validation passed. Fitting the module");
            }

            return true;
}

void ShipItem::Dock() {
    DeactivateAllModules();
}

void ShipItem::Undock() {
    if (sConfig.server.testServer) {
        // Heal Ship completely on test server
        Heal();
    } else {
        // live server will ONLY Recharge shields and cap
        SetShipShield(1.0);
        SetShipCapacitorLevel(1.0);
    }
    //get list of modules to activate from ShipBound::Handle_Undock()
    if (m_onlineModuleVec.size() < 1) return;
    for (auto cur : m_onlineModuleVec) {
        m_ModuleManager->Online(cur);
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
InventoryItemRef ShipItem::GetModule(EVEItemFlags flag)
{
	if ( m_ModuleManager->GetModule(flag) != NULL )
		return (m_ModuleManager->GetModule(flag))->getItem();
	else
		return InventoryItemRef();
}

InventoryItemRef ShipItem::GetModule(uint32 itemID)
{
	if ( m_ModuleManager->GetModule(itemID) != NULL )
		return (m_ModuleManager->GetModule(itemID))->getItem();
	else
		return InventoryItemRef();
}

EVEItemFlags ShipItem::FindAvailableModuleSlot(InventoryItemRef item) {
    uint32 slotFound = flagIllegal;
    // 1) get slot bank (low, med, high, rig, subsystem) from dgmTypeEffects using item->itemID()
    // 2) query this ship's ModuleManager to determine if there are any free slots in that bank,
    //    it should return a slot flag number for the next available slot starting at the lowest number
    //    for that bank
    // 3) return that slot flag number
    if (item->type().HasEffect(Effect_loPower)) {
        slotFound = m_ModuleManager->GetAvailableSlotInBank(Effect_loPower);
    } else if (item->type().HasEffect(Effect_medPower)) {
        slotFound = m_ModuleManager->GetAvailableSlotInBank(Effect_medPower);
    } else if (item->type().HasEffect(Effect_hiPower)) {
        slotFound = m_ModuleManager->GetAvailableSlotInBank(Effect_hiPower);
    } else if (item->type().HasEffect(Effect_subSystem)) {
        slotFound = m_ModuleManager->GetAvailableSlotInBank(Effect_subSystem);
    } else if (item->type().HasEffect(Effect_rigSlot)) {
        slotFound = m_ModuleManager->GetAvailableSlotInBank(Effect_rigSlot);
    } else {
        // ERROR: This is not a module that fits in any of the slot banks
    }

    return (EVEItemFlags)slotFound;
}

uint32 ShipItem::AddItem(EVEItemFlags flag, InventoryItemRef item)
{
    if (!ValidateAddItem(flag, item)) return 0;
    if (IsModuleSlot(flag)) {
        if (item->categoryID() == EVEDB::invCategories::Charge) {
            m_ModuleManager->LoadCharge(item, flag);
            InventoryItemRef loadedChargeOnModule = m_ModuleManager->GetLoadedChargeOnModule(flag);
            if (loadedChargeOnModule)
                return loadedChargeOnModule->itemID();
            else
                return 0;
        } else if (item->categoryID() == EVEDB::invCategories::Module) {
            item->PutOffline();
            // rigs are classed in the module category.  check here and call approprate method as needed.
            if ((item->groupID() >= 773 && item->groupID() <= 782) || item->groupID() == 786) {
                if (!m_ModuleManager->InstallRig(item, flag))
                    return 0;
            } else if (!m_ModuleManager->FitModule(item, flag))
                return 0;
        } else if (item->categoryID() == EVEDB::invCategories::Subsystem) {
            item->PutOffline();
            if (!m_ModuleManager->InstallSubSystem(item, flag))
                return 0;
        }
    } else {
        _IncreaseCargoHoldsUsedVolume( flag, (item->GetAttribute(AttrVolume).get_float() * item->quantity()));
	}

    item->Move(itemID(), flag);
	if (IsModuleSlot(flag)) {
        item->PutOnline();
        m_ModuleManager->Online(item->itemID());
        UpdateModules(flag);
    }

	return item->itemID();
}

void ShipItem::RemoveItem(InventoryItemRef item/*, uint32 inventoryID, EVEItemFlags flag*/)
{
    // check to see if item is currently in a module slot.  going by category is NOT working after _ExecAdd() updates.
    if (IsModuleSlot(item->flag())) {
        // if item being removed IS a charge, it needs to be removed via Module Manager so modules know charge is removed,
        // BUT, only if it is loaded into a module in one of the 3 slot banks, so we also check its flag value:
        if ((item->categoryID() == EVEDB::invCategories::Charge)
                && ((item->flag() >= flagLowSlot0) && (item->flag() <= flagHiSlot7))) {
            m_ModuleManager->UnloadCharge(item->flag());
            return;
        } else if ((item->categoryID() == EVEDB::invCategories::Module) || (item->categoryID() == EVEDB::invCategories::Subsystem)) {
            Deactivate( item->itemID(), "offline" );
            if (((item->flag() >= flagLowSlot0) && (item->flag() <= flagHiSlot7))
                || ((item->flag() >= flagSubSystem0) && (item->flag() <= flagSubSystem7))) {
                // item is a module and it's being removed from a slot:
                m_ModuleManager->UnfitModule(item->itemID());
                return;
            } else if ((item->flag() >= flagRigSlot0) && (item->flag() <= flagRigSlot7)) {
                // item is a rig and it's being removed from a slot:
                m_ModuleManager->UninstallRig(item->itemID());
                return;
            }
        }
    } else
        _DecreaseCargoHoldsUsedVolume( item->flag(), (item->GetAttribute(AttrVolume).get_float() * item->quantity()));
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
    modItemRef1->Move(itemID(), flagCargoHold);

    if (m_ModuleManager->IsSlotOccupied(slot2)) {
        // dropped-on slot is occupied.  procede with moving the module currently in this slot.
        InventoryItemRef modItemRef2 = GetModule(slot2);
        InventoryItemRef chargeItemRef2 = m_ModuleManager->GetLoadedChargeOnModule(slot2);
        if (chargeItemRef2)
            m_ModuleManager->UnloadCharge(slot2);
        modItemRef2->Move(itemID(), flagCargoHold);

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
    //may not look like it, but just moving this item will call ModuleManager::UninstallRig().
    item->Move(itemID(), flagCargoHold);
}

double ShipItem::CalculateRechargeRate(double Capacity, double Current, double RechargeTimeMS)
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

void ShipItem::Process() {
    double profileStartTime = 0.0;
    if (sConfig.server.UseProfiling)
        profileStartTime = GetTimeUSeconds();
    // Do Automatic Shield and Capacitor Recharge:
    if (m_processTimer.Check()) {
        // shield
        double Charge = GetAttribute(AttrShieldCharge).get_float();
        double Capacity = GetAttribute(AttrShieldCapacity).get_float();
        if (Charge < Capacity) {
            double newCharge = Charge + ((m_processTimerTick /1000) * CalculateRechargeRate(Capacity, Charge, GetAttribute(AttrShieldRechargeRate).get_float()));
            if (newCharge > Capacity)
                newCharge = Capacity;
            else if ((Capacity - newCharge) < 0.15)
                newCharge = Capacity;
            SetAttribute(AttrShieldCharge, newCharge);
            _log(SHIP__MESSAGE, "ShipItem::Process(): %s(%u) - New Shield Charge: %f",\
                    m_pilot->GetName(), m_pilot->GetShip().get()->itemID(), newCharge );
        }

        // capacitor
        Charge = GetAttribute(AttrCapacitorCharge).get_float();
        Capacity = GetAttribute(AttrCapacitorCapacity).get_float();
        if (Charge < Capacity) {
            double newCharge = Charge + ((m_processTimerTick /1000) * CalculateRechargeRate(Capacity, Charge, GetAttribute(AttrRechargeRate).get_float()));
            if (newCharge > Capacity)
                newCharge = Capacity;
            else if ((Capacity - newCharge) < 0.15)
                newCharge = Capacity;
            SetAttribute(AttrCapacitorCharge, newCharge);
            _log(SHIP__MESSAGE, "ShipItem::Process(): %s(%u) - New Cap Charge: %f",\
                        m_pilot->GetName(), m_pilot->GetShip().get()->itemID(), newCharge );
        }
    }

    // profile timer for JUST the ship recharge shit
    if (sConfig.server.UseProfiling)
        sProfile.AddTime(_shipProfile, GetTimeUSeconds() - profileStartTime);

    // now, process the modules.
    // Do this last so repair modules don't degrade shield recharge rate.
    // Although, cap recharge would benefit from the power use by modules.
    m_ModuleManager->Process();
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

void ShipItem::StripFitting()
{
    std::vector<InventoryItemRef> modList;
    m_ModuleManager->GetModuleListOfRefs(&modList);
    for (auto cur : modList)
        m_ModuleManager->UnfitModule(cur->itemID());
}

/* End new Module Manager Interface */

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
    dna << subSys << modHi << modMid << modLow << modRig << charges << drones;

    _log(SHIP__INFO, "ShipDNA has compiled DNA of \"%s\" for %s(%u) ", dna.str().c_str(), itemName().c_str(), itemID());
    return dna.str();
}


/* DynamicSystemEntity representing ship object in space */
Ship::Ship(InventoryItemRef self, PyServiceMgr &services, SystemManager* system)
: DynamicSystemEntity(self, services, system)
{
    m_destiny = new DestinyManager(this, system);
    m_podShipID = 0;
    m_player = nullptr;
    _log(SHIP__INFO, "Created ShipSE %p for item %u", this, self->itemID());
}

Ship::~Ship() {
    m_targMgr->DoDestruction();
    SafeDelete(m_destiny);
}

void Ship::Process() {
    SystemEntity::Process();
}

void Ship::PayInsurance() {
    m_player->GetChar()->AlterBalance(m_db.GetShipInsurancePayout(GetSelf()->itemID()));
    m_db.DeleteInsuranceByShipID(GetSelf()->itemID());
}

void Ship::SetPilot(Client* pClient) {
    m_player = pClient;
    m_self->SetPlayer(pClient);
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

    BallHeader head;
        head.entityID = GetID();
        head.mode = mode;
        head.radius = GetRadius();
        head.x = x();
        head.y = y();
        head.z = z();
        head.flags = IsMassive | IsFree;
        into.Append( head );
    MassSector mass;
        mass.mass = GetMass();
        mass.cloak = 0;
        mass.Harmonic = -1.0f;
        mass.corporationID = GetCorporationID();
        mass.allianceID = GetAllianceID();
        into.Append( mass );
    ShipSector ship;
        ship.maxVelocity = GetMaxVelocity();
        ship.velocity_x = GetVelocity().x;
        ship.velocity_y = GetVelocity().y;
        ship.velocity_z = GetVelocity().z;
        ship.agility = GetAgility();
        ship.speedfraction = m_destiny->GetSpeedFraction();
        into.Append( ship );
    if (mode == DSTBALL_WARP) {
        GPoint target = m_destiny->GetTargetPoint();
        DSTBALL_WARP_Struct warp;
            warp.effectStamp = -1;   //unknown value  seen many -1, few other random 4-5 digits
            warp.unknown_x = target.x;
            warp.unknown_y = target.y;
            warp.unknown_z = target.z;
            warp.ownerID = m_destiny->GetWarpSpeed();       //ship warp speed x10  (dont ask...this is what it is...more dumb ccp shit)
            warp.unk_1 = 0;      //unknown 64bit number.  seen 4666723172467343360 once....others are 0
            warp.unk_2 = 0;         //unknown 64bit number
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

    _log(COMMON__WARNING, "Ship::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
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
        slim->SetItemString("charID",               new PyInt(m_player ? m_player->GetCharacterID() : 0));
        slim->SetItemString("corpID",               new PyInt(m_player ? m_player->GetCorporationID() : GetCorporationID()));
        slim->SetItemString("allianceID",           new PyInt(m_player ? m_player->GetAllianceID() : GetAllianceID()));
        slim->SetItemString("warFactionID",         new PyInt(m_player ? m_player->GetWarFactionID() : GetWarFactionID()));
        slim->SetItemString("bounty",               new PyFloat(m_player ? m_player->GetBounty() : 0));
        slim->SetItemString("securityStatus",       new PyFloat(m_player ? m_player->GetSecurityRating() : 0.0));
    if (m_self->typeID() == itemTypeCapsule) {
        slim->SetItemString("launcherID",       new PyInt(GetPodShipID()));
        return slim;
    } else {
        slim->SetItemString("categoryID",       new PyInt(m_self->categoryID()));
        slim->SetItemString("groupID",          new PyInt(m_self->groupID()));
    }

    //encode the hiSlot and Subsystem modules list ONLY
    std::vector<InventoryItemRef> items;
    m_self->GetInventory()->FindByFlagRange(flagHiSlot0, flagHiSlot7, items);
    //m_self->GetInventory()->FindByFlagRange(flagSubSystem0, flagSubSystem7, items);
    if (!items.empty()) {
        PyList *l = new PyList();
        for (auto cur : items) {
            PyTuple* t = new_tuple(cur->itemID(), cur->typeID());
            l->AddItem(t);
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
