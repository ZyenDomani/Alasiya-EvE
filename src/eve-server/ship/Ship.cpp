/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:     Bloody.Rabbit
    Updates:    Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "EVEServerConfig.h"
#include "Profile.h"
#include "character/Character.h"
#include "ship/DestinyManager.h"
#include "ship/Ship.h"
//#include "ship/ShipOperatorInterface.h"
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
 * Ship
 */
Ship::Ship(
    ItemFactory &_factory,
    uint32 _shipID,
    // InventoryItem stuff:
    const ShipType &_shipType,
    const ItemData &_data)
: InventoryItem(_factory, _shipID, _shipType, _data),
  m_processTimerTick(SHIP_PROCESS_TICK_MS),
  m_processTimer(SHIP_PROCESS_TICK_MS)
{
    m_ModuleManager = nullptr;
    m_pOperator = new ShipOperatorInterface();
    m_IsLoaded = false;

	m_processTimer.Start();

    _log(ITEM__TRACE, "Created Ship object for item %s (%u).", itemName().c_str(), itemID());
}

ShipRef Ship::Load(ItemFactory &factory, uint32 shipID)
{
    return InventoryItem::Load<Ship>( factory, shipID );
}

template<class _Ty>
RefPtr<_Ty> Ship::_LoadShip(ItemFactory &factory, uint32 shipID,
    // InventoryItem stuff:
    const ShipType &shipType, const ItemData &data)
{
    // we don't need any additional stuff
    return ShipRef( new Ship(factory, shipID, shipType, data ) );
}

ShipRef Ship::Spawn(ItemFactory &factory, ItemData &data) {
    uint32 shipID = Ship::_Spawn( factory, data );
    if ( shipID == 0 )
        return ShipRef();

    ShipRef sShipRef = Ship::Load( factory, shipID );

    // Create default dynamic attributes in the AttributeMap:
    sShipRef->SetAttribute(AttrIsOnline,            1, true);												// Is Online
    sShipRef->SetAttribute(AttrShieldCharge,        sShipRef->GetAttribute(AttrShieldCapacity), true);		// Shield Charge
    sShipRef->SetAttribute(AttrArmorDamage,         0.0, true);												// Armor Damage
    sShipRef->SetAttribute(AttrMass,                sShipRef->type().mass(), true);				// Mass
    sShipRef->SetAttribute(AttrRadius,              sShipRef->type().radius(), true);			// Radius
    sShipRef->SetAttribute(AttrVolume,              sShipRef->type().volume(), true);			// Volume
    sShipRef->SetAttribute(AttrCapacity,            sShipRef->type().capacity(), true);			// Capacity
    sShipRef->SetAttribute(AttrInertia,             1, true);												// Inertia
    sShipRef->SetAttribute(AttrCapacitorCharge,     sShipRef->GetAttribute(AttrCapacitorCapacity), true);	// Set Capacitor Charge to the Capacitor Capacity

    // Check for existence of some attributes that may or may not have already been loaded and set them
    // to default values:
	// Hull Damage
	if ( !(sShipRef->HasAttribute(AttrDamage)) )
        sShipRef->SetAttribute(AttrDamage, 0.0f, true );
    // Theoretical Maximum Targeting Range
    if ( !(sShipRef->HasAttribute(AttrMaximumRangeCap)) )
        sShipRef->SetAttribute(AttrMaximumRangeCap, ((double)BUBBLE_RADIUS_METERS), true );
    // Maximum Armor Damage Resonance
    if ( !(sShipRef->HasAttribute(AttrArmorMaxDamageResonance)) )
        sShipRef->SetAttribute(AttrArmorMaxDamageResonance, 1.0f, true);
    // Maximum Shield Damage Resonance
    if ( !(sShipRef->HasAttribute(AttrShieldMaxDamageResonance)) )
        sShipRef->SetAttribute(AttrShieldMaxDamageResonance, 1.0f, true);
    // Warp Speed Multiplier
    if ( !(sShipRef.get()->HasAttribute(AttrWarpSpeedMultiplier)) )
        sShipRef.get()->SetAttribute(AttrWarpSpeedMultiplier, 1.0f, true);
    // CPU Load of the ship (new ships have zero load with no modules fitted, of course):
    if ( !(sShipRef.get()->HasAttribute(AttrCpuLoad)) )
        sShipRef.get()->SetAttribute(AttrCpuLoad, 0.0f, true);
    // Power Load of the ship (new ships have zero load with no modules fitted, of course):
    if ( !(sShipRef.get()->HasAttribute(AttrPowerLoad)) )
        sShipRef.get()->SetAttribute(AttrPowerLoad, 0.0f, true);
	// Warp Scramble Status of the ship (most ships have zero warp scramble status, but some already have it defined):
	if ( !(sShipRef.get()->HasAttribute(AttrWarpScrambleStatus)) )
		sShipRef.get()->SetAttribute(AttrWarpScrambleStatus, 0.0f, true);

	// Shield Resonance
	// AttrShieldEmDamageResonance
	if ( !(sShipRef.get()->HasAttribute(AttrShieldEmDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrShieldEmDamageResonance, 1.0, true);
	// AttrShieldExplosiveDamageResonance
	if ( !(sShipRef.get()->HasAttribute(AttrShieldExplosiveDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrShieldExplosiveDamageResonance, 1.0, true);
	// AttrShieldKineticDamageResonance
	if ( !(sShipRef.get()->HasAttribute(AttrShieldKineticDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrShieldKineticDamageResonance, 1.0, true);
	// AttrShieldThermalDamageResonance
	if ( !(sShipRef.get()->HasAttribute(AttrShieldThermalDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrShieldThermalDamageResonance, 1.0, true);

	// Armor Resonance
	// AttrArmorEmDamageResonance
	if ( !(sShipRef.get()->HasAttribute(AttrArmorEmDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrArmorEmDamageResonance, 1.0, true);
	// AttrArmorExplosiveDamageResonance
	if ( !(sShipRef.get()->HasAttribute(AttrArmorExplosiveDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrArmorExplosiveDamageResonance, 1.0, true);
	// AttrArmorKineticDamageResonance
	if ( !(sShipRef.get()->HasAttribute(AttrArmorKineticDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrArmorKineticDamageResonance, 1.0, true);
	// AttrArmorThermalDamageResonance
	if ( !(sShipRef.get()->HasAttribute(AttrArmorThermalDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrArmorThermalDamageResonance, 1.0, true);

	// Hull Resonance
	// AttrHullEmDamageResonance
	if ( !(sShipRef.get()->HasAttribute(AttrHullEmDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrHullEmDamageResonance, 1.0, true);
	// AttrHullExplosiveDamageResonance
	if ( !(sShipRef.get()->HasAttribute(AttrHullExplosiveDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrHullExplosiveDamageResonance, 1.0, true);
	// AttrHullKineticDamageResonance
	if ( !(sShipRef.get()->HasAttribute(AttrHullKineticDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrHullKineticDamageResonance, 1.0, true);
	// AttrHullThermalDamageResonance
	if ( !(sShipRef.get()->HasAttribute(AttrHullThermalDamageResonance)) )
		sShipRef.get()->SetAttribute(AttrHullThermalDamageResonance, 1.0, true);

	// AttrTurretSlotsLeft
	if ( !(sShipRef.get()->HasAttribute(AttrTurretSlotsLeft)) )
		sShipRef.get()->SetAttribute(AttrTurretSlotsLeft, 0, true);
	// AttrLauncherSlotsLeft
	if ( !(sShipRef.get()->HasAttribute(AttrLauncherSlotsLeft)) )
		sShipRef.get()->SetAttribute(AttrLauncherSlotsLeft, 0, true);

    return sShipRef;
}

uint32 Ship::_Spawn(ItemFactory &factory, ItemData &data) {
    // make sure it's a ship
    const ShipType *st = factory.GetShipType(data.typeID);
    if (st == NULL)
        return 0;

    // store item data
    uint32 shipID = InventoryItem::_Spawn(factory, data);
    if (shipID == 0)
        return 0;

    // nothing additional

    return shipID;
}

bool Ship::_Load()
{
    if (m_IsLoaded && m_ModuleManager) return true;

    // load contents
    if ( !LoadContents( &m_factory ) )
        return false;

    if (!InventoryItem::_Load())      // Attributes are loaded here!
        return false;

	// fill cargo holds data here:
    //  NOTE  skill and ship bonuses will be applied AFTER the ship object is created/loaded, but BEFORE MM takes affect.
	if ( HasAttribute(AttrCapacity) )
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagCargoHold,mAttributeMap.GetAttribute(AttrCapacity).get_float()));
	if ( HasAttribute(AttrDroneCapacity) )
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagDroneBay,mAttributeMap.GetAttribute(AttrDroneCapacity).get_float()));
	if ( HasAttribute(AttrSpecialFuelBayCapacity) )
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedFuelBay,mAttributeMap.GetAttribute(AttrSpecialFuelBayCapacity).get_float()));
	if ( HasAttribute(AttrSpecialOreHoldCapacity) )
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedOreHold,mAttributeMap.GetAttribute(AttrSpecialOreHoldCapacity).get_float()));
	if ( HasAttribute(AttrSpecialGasHoldCapacity) )
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedGasHold,mAttributeMap.GetAttribute(AttrSpecialGasHoldCapacity).get_float()));
	if ( HasAttribute(AttrSpecialMineralHoldCapacity) )
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedMineralHold,mAttributeMap.GetAttribute(AttrSpecialMineralHoldCapacity).get_float()));
	if ( HasAttribute(AttrSpecialSalvageHoldCapacity) )
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedSalvageHold,mAttributeMap.GetAttribute(AttrSpecialSalvageHoldCapacity).get_float()));
	if ( HasAttribute(AttrSpecialShipHoldCapacity) )
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedShipHold,mAttributeMap.GetAttribute(AttrSpecialShipHoldCapacity).get_float()));
	if ( HasAttribute(AttrSpecialSmallShipHoldCapacity) )
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedSmallShipHold,mAttributeMap.GetAttribute(AttrSpecialSmallShipHoldCapacity).get_float()));
	if ( HasAttribute(AttrSpecialLargeShipHoldCapacity) )
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedLargeShipHold,mAttributeMap.GetAttribute(AttrSpecialLargeShipHoldCapacity).get_float()));
	if ( HasAttribute(AttrSpecialIndustrialShipHoldCapacity) )
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedIndustrialShipHold,mAttributeMap.GetAttribute(AttrSpecialIndustrialShipHoldCapacity).get_float()));
	if ( HasAttribute(AttrSpecialAmmoHoldCapacity) )
		m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagSpecializedAmmoHold,mAttributeMap.GetAttribute(AttrSpecialAmmoHoldCapacity).get_float()));

	_UpdateCargoHoldsUsedVolume();

    m_IsLoaded = true;
    return true;
}

void Ship::Init()
{
    // NOTE: These all still need to have ship bonuses applied
    //TODO FIXME  this will need to be changed to use skill modifiers when i get them working....
    double pg = GetDefaultAttribute(AttrPowerOutput).get_int();
    double cpu = GetDefaultAttribute(AttrCpuOutput).get_float();
    double hullHP = GetDefaultAttribute(AttrHP).get_int();
    double armorHP = GetDefaultAttribute(AttrArmorHP).get_float();
    double capCapacity = GetDefaultAttribute(AttrCapacitorCapacity).get_float();  // default value from db
    double capChargeRate = GetDefaultAttribute(AttrRechargeRate).get_float(); // default value from db
    double shieldCapacity = GetDefaultAttribute(AttrShieldCapacity).get_float();
    double shieldChargeRate = GetDefaultAttribute(AttrShieldRechargeRate).get_float();

    Character* pChar = GetOperator()->GetChar().get();

    pg *=  (1 + (0.05 * (pChar->GetSkillLevel(skillEngineering, true))));      //5% increase
    cpu *=  (1 + (0.05 * (pChar->GetSkillLevel(skillElectronics, true))));      // 5% increase
    hullHP *=  (1 + (0.05 * (pChar->GetSkillLevel(skillMechanics, true))));      //5% increase
    armorHP *=  (1 + (0.05 * (pChar->GetSkillLevel(skillHullUpgrades, true))));      // 5% increase
    capCapacity *=  (1 + (0.05 * (pChar->GetSkillLevel(skillEnergyManagement, true))));      // 5% increase
    capChargeRate *=  (1 - (0.05 * (pChar->GetSkillLevel(skillEnergySystemsOperation, true))));      //5% decrease
    shieldCapacity *=  (1 + (0.05 * (pChar->GetSkillLevel(skillShieldManagement, true))));      // 5% increase
    shieldChargeRate *=  (1 - (0.05 * (pChar->GetSkillLevel(skillShieldOperation, true))));      //5% decrease

    // add checks for implants here.
    //  ship bonuses are found in dgmShipBonusModifiers
    //  skill bonuses are found in dgmSkillBonusModifiers

    // reset cpu and pg loads to 0 before updating modules      -- should do this on client logout.  this is catchall for crash
    SetAttribute(AttrCpuLoad, 0);
    SetAttribute(AttrPowerLoad, 0);
    SetAttribute(AttrUpgradeLoad, 0);
    SetAttribute(AttrUpgradeSlotsLeft, AttrRigSlots);
    // set ship adjusted attributes and save.
    SetAttribute(AttrHP, hullHP);
    SetAttribute(AttrArmorHP, armorHP);
    SetAttribute(AttrCpuOutput, cpu);
    SetAttribute(AttrPowerOutput, pg);
    SetAttribute(AttrRechargeRate, capChargeRate);
    SetAttribute(AttrShieldCapacity, shieldCapacity);
    SetAttribute(AttrCapacitorCharge, capCapacity);
    SetAttribute(AttrShieldRechargeRate,shieldChargeRate );
    SaveAttributes();

    // allocate the module manager, only the first time:
    if (!m_ModuleManager)
        m_ModuleManager = new ModuleManager(this);

    m_ModuleManager->Initialize();
    //set everything to full AFTER modules possibably update ship stats
    /** @todo need to check for ship damage status BEFORE or INSTEAD of calling this.
     */
    Heal();
}

void Ship::InitPod()
{
    // allocate the module manager, only the first time:
    if (!m_ModuleManager)
        m_ModuleManager = new ModuleManager(this);

    m_ModuleManager->Initialize();
    Heal();
}

void Ship::_UpdateCargoHoldsUsedVolume()    //TODO FIXME  look into this....not working right.
{
	if ( HasAttribute(AttrCapacity) )
		_log(ITEM__TRACE, "Ship::_UpdateCargoHoldsUsedVolume() - flagCargoHold update values: m_cargoHoldsUsedVolumeByFlag = %lf, GetStoredVolume = %lf",
					m_cargoHoldsUsedVolumeByFlag.find(flagCargoHold)->second, GetStoredVolume(flagCargoHold));
		m_cargoHoldsUsedVolumeByFlag.find(flagCargoHold)->second = GetStoredVolume(flagCargoHold);
		_log(ITEM__TRACE, "Ship::_UpdateCargoHoldsUsedVolume() - flagCargoHold new values: m_cargoHoldsUsedVolumeByFlag = %lf",
							m_cargoHoldsUsedVolumeByFlag.find(flagCargoHold)->second);
	if ( HasAttribute(AttrDroneCapacity) )
		m_cargoHoldsUsedVolumeByFlag.find(flagDroneBay)->second = GetStoredVolume(flagDroneBay);
	if ( HasAttribute(AttrSpecialFuelBayCapacity) )
		m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedFuelBay)->second = GetStoredVolume(flagSpecializedFuelBay);
	if ( HasAttribute(AttrSpecialOreHoldCapacity) )
		m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedOreHold)->second = GetStoredVolume(flagSpecializedOreHold);
	if ( HasAttribute(AttrSpecialGasHoldCapacity) )
		m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedGasHold)->second = GetStoredVolume(flagSpecializedGasHold);
	if ( HasAttribute(AttrSpecialMineralHoldCapacity) )
		m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedMineralHold)->second = GetStoredVolume(flagSpecializedMineralHold);
	if ( HasAttribute(AttrSpecialSalvageHoldCapacity) )
		m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedSalvageHold)->second = GetStoredVolume(flagSpecializedSalvageHold);
	if ( HasAttribute(AttrSpecialShipHoldCapacity) )
		m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedShipHold)->second = GetStoredVolume(flagSpecializedShipHold);
	if ( HasAttribute(AttrSpecialSmallShipHoldCapacity) )
		m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedSmallShipHold)->second = GetStoredVolume(flagSpecializedSmallShipHold);
	if ( HasAttribute(AttrSpecialLargeShipHoldCapacity) )
		m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedLargeShipHold)->second = GetStoredVolume(flagSpecializedLargeShipHold);
	if ( HasAttribute(AttrSpecialIndustrialShipHoldCapacity) )
		m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedIndustrialShipHold)->second = GetStoredVolume(flagSpecializedIndustrialShipHold);
	if ( HasAttribute(AttrSpecialAmmoHoldCapacity) )
		m_cargoHoldsUsedVolumeByFlag.find(flagSpecializedAmmoHold)->second = GetStoredVolume(flagSpecializedAmmoHold);
}

void Ship::_IncreaseCargoHoldsUsedVolume(EVEItemFlags flag, double volumeToConsume)
{
	if ( m_cargoHoldsUsedVolumeByFlag.find(flag) != m_cargoHoldsUsedVolumeByFlag.end() )
		m_cargoHoldsUsedVolumeByFlag.find(flag)->second += volumeToConsume;
	else
		throw PyException( MakeCustomError( "ERROR!  Illegal flag '%u' specified!", flag ) );
}

void Ship::_DecreaseCargoHoldsUsedVolume(EVEItemFlags flag, double volumeToConsume)
{
	if ( m_cargoHoldsUsedVolumeByFlag.find(flag) != m_cargoHoldsUsedVolumeByFlag.end() )
		m_cargoHoldsUsedVolumeByFlag.find(flag)->second -= volumeToConsume;
	else
		throw PyException( MakeCustomError( "ERROR!  Illegal flag '%u' specified!", flag ) );
}

void Ship::Delete()
{
    // delete contents first
    DeleteContents( &m_factory );

    InventoryItem::Delete();
}

double Ship::GetCapacity(EVEItemFlags flag) const
{
    switch( flag ) {
		case flagAutoFit:
		case flagCargoHold:
			if ( HasAttribute(AttrCapacity) )
				return GetAttribute(AttrCapacity).get_float();
			break;

		case flagDroneBay:
			if ( HasAttribute(AttrDroneCapacity) )
				return GetAttribute(AttrDroneCapacity).get_float();
			break;

		case flagSpecializedFuelBay:
			if ( HasAttribute(AttrSpecialFuelBayCapacity) )
				return GetAttribute(AttrSpecialFuelBayCapacity).get_float();
			break;

		case flagSpecializedOreHold:
			if ( HasAttribute(AttrSpecialOreHoldCapacity) )
				return GetAttribute(AttrSpecialOreHoldCapacity).get_float();
			break;

		case flagSpecializedGasHold:
			if ( HasAttribute(AttrSpecialGasHoldCapacity) )
				return GetAttribute(AttrSpecialGasHoldCapacity).get_float();
			break;

		case flagSpecializedMineralHold:
			if ( HasAttribute(AttrSpecialMineralHoldCapacity) )
				return GetAttribute(AttrSpecialMineralHoldCapacity).get_float();
			break;

		case flagSpecializedSalvageHold:
			if ( HasAttribute(AttrSpecialSalvageHoldCapacity) )
				return GetAttribute(AttrSpecialSalvageHoldCapacity).get_float();
			break;

		case flagSpecializedShipHold:
			if ( HasAttribute(AttrSpecialShipHoldCapacity) )
				return GetAttribute(AttrSpecialShipHoldCapacity).get_float();
			break;

		case flagSpecializedSmallShipHold:
			if ( HasAttribute(AttrSpecialSmallShipHoldCapacity) )
				return GetAttribute(AttrSpecialSmallShipHoldCapacity).get_float();
			break;

		case flagSpecializedLargeShipHold:
			if ( HasAttribute(AttrSpecialLargeShipHoldCapacity) )
				return GetAttribute(AttrSpecialLargeShipHoldCapacity).get_float();
			break;

		case flagSpecializedIndustrialShipHold:
			if ( HasAttribute(AttrSpecialIndustrialShipHoldCapacity) )
				return GetAttribute(AttrSpecialIndustrialShipHoldCapacity).get_float();
			break;

		case flagSpecializedAmmoHold:
			if ( HasAttribute(AttrSpecialAmmoHoldCapacity) )
				return GetAttribute(AttrSpecialAmmoHoldCapacity).get_float();
			break;

        case flagShipHangar:
			if ( HasAttribute(AttrShipMaintenanceBayCapacity) )
				return GetAttribute(AttrShipMaintenanceBayCapacity).get_float();
			break;

        case flagHangar:
			if ( HasAttribute(AttrCorporateHangarCapacity) )
				return GetAttribute(AttrCorporateHangarCapacity).get_float();
			break;

		default:
			return 0.0;
			break;
	}

	// Handle all missing/unsupported/illegal flag by reporting available capacity of 0.0:
	return 0.0;
}

double Ship::GetRemainingVolumeByFlag(EVEItemFlags flag) const
{
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

bool Ship::ValidateAddItem(EVEItemFlags flag, InventoryItemRef item)
{
    CharacterRef character = m_pOperator->GetChar();

    if (flag == flagDroneBay) {
        if ( item->categoryID() != EVEDB::invCategories::Drone ) {
            throw PyException( MakeUserError( "Item Cannot be stowed in the Drone Bay" ) );
            return false;
        }
    } else if (flag == flagShipHangar) {
        if (m_pOperator->GetShip()->GetAttribute(AttrHasShipMaintenanceBay) != 0) {
            throw PyException( MakeCustomError( "%s has no ship maintenance bay.", item->itemName().c_str() ) );
            return false;
        }
        if (item->categoryID() != EVEDB::invCategories::Ship) {
            throw PyException( MakeCustomError( "Only ships may be placed into ship maintenance bay." ) );
            return false;
        }
    } else if (flag == flagHangar) {
        if (m_pOperator->GetShip()->GetAttribute(AttrHasCorporateHangars) != 0) {
            throw PyException( MakeCustomError( "%s has no corporate hangars.", item->itemName().c_str() ) );
            return false;
        }
    } else if ((flag >= flagLowSlot0) && (flag <= flagHiSlot7)) {
        if (m_pOperator->IsClient())
            if (!Skill::FitModuleSkillCheck(item, character)) {
                throw PyException( MakeCustomError( "You do not have the required skills to fit this \n%s", item->itemName().c_str() ) );
                return false;
            }
        if (!ValidateItemSpecifics(item)) {
            throw PyException( MakeCustomError( "Your ship cannot equip this module" ) );
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
                    throw PyException( MakeCustomError( "The charge is not the correct size for this module." ) );
                    return false;
                }
				if (module->GetAttribute(AttrChargeGroup1) != item->groupID() &&
					module->GetAttribute(AttrChargeGroup2) != item->groupID() &&
					module->GetAttribute(AttrChargeGroup3) != item->groupID() &&
					module->GetAttribute(AttrChargeGroup4) != item->groupID() &&
					module->GetAttribute(AttrChargeGroup5) != item->groupID()) {
                    	throw PyException( MakeCustomError( "Incorrect charge type for this module.") );
                    	return false;
                }
				// NOTE: Module Manager will check for actual room to load charges and make stack splits, or reject loading altogether
			} else {
                throw PyException( MakeCustomError( "Module at flag '%u' does not exist!", flag ) );
                return false;
            }
        } else {
			if (m_ModuleManager->IsSlotOccupied(flag)) {
                throw PyException( MakeUserError( "SlotAlreadyOccupied" ) );
                return false;
            }
		}
    } else if ((flag >= flagRigSlot0) && (flag <= flagRigSlot7)) {
        if (m_pOperator->IsClient()) {
            if (!Skill::FitModuleSkillCheck(item, character)) {
                throw PyException( MakeCustomError( "You do not have the required skills to fit this \n%s", item->itemName().c_str() ) );
                return false;
            }
            if (m_pOperator->GetShip()->GetAttribute(AttrRigSize) != item->GetAttribute(AttrRigSize)) {
                throw PyException( MakeCustomError( "Your ship cannot fit this size module" ) );
                return false;
            }
            if (m_pOperator->GetShip()->GetAttribute(AttrUpgradeLoad) + item->GetAttribute(AttrUpgradeCost) > m_pOperator->GetShip()->GetAttribute(AttrUpgradeCapacity) ) {
                throw PyException( MakeCustomError( "Your ship cannot handle the extra calibration" ) );
                return false;
            }
        }
    } else if ((flag >= flagSubSystem0) && (flag <= flagSubSystem7)) {
        if (m_pOperator->IsClient())
            if (!Skill::FitModuleSkillCheck(item, character)) {
                throw PyException( MakeCustomError( "You do not have the required skills to fit this \n%s", item->itemName().c_str() ) );
                return false;
            }
    } else {
		// Handle any other flag, legal or not by virtue of GetRemainingVolumeByFlag() and GetCapacity() that handle supported capacity types:
		// (unsupported or illegal flags report capacity of 0.0, so are automatically rejected)
		if ((GetRemainingVolumeByFlag(flag) < (item->GetAttribute(AttrVolume).get_float() * item->quantity()))) {
            throw PyException( MakeCustomError( "Not enough cargo space!<br><br>flag = %u", (uint32)flag) );
            return false;
        }
    }

	return true;
}

// this one is called from ShipGetInfo
PyDict* Ship::ShipGetInfo()
{
    /*
          [PyDict 14 kvp]
            [PyTuple 3 items]
              [PyIntegerVar 1002332770557]     << ship id
              [PyInt 32]                       << slot id (flag)
              [PyInt 21867]                    << type id
            [PyObjectData Name: util.KeyVal]
              [PyDict 5 kvp]
                [PyString "itemID"]
                [PyTuple 3 items]
                  [PyIntegerVar 1002332770557] << ship id
                  [PyInt 32]                   << slot id (flag)
                  [PyInt 21867]                << type id
                [PyString "attributes"]      << set in Populate()
                [PyDict 32 kvp]
                  [PyInt 644]
                  [PyFloat 1]
..............
                  [PyInt 4]
                  [PyFloat 1000]
                [PyString "invItem"]
                [PyNone]
                [PyString "time"]      << set in Populate()
                [PyIntegerVar 129520542423668225]
                [PyString "activeEffects"]      << set in Populate()
                [PyDict 0 kvp]
            */
    if ( !LoadContents( &m_factory ) )
    {
        codelog( ITEM__ERROR, "%s (%u): Failed to load contents for ShipGetInfo", itemName().c_str(), itemID() );
        return NULL;
    }

    PyDict* result = new PyDict;
    Rsp_CommonGetInfo_Entry entry;

    //first populate the ship.
    if ( !Populate( entry ) )
        return NULL;    //print already done.

        result->SetItem(new PyInt( itemID()), new PyObject("util.KeyVal", entry.Encode()));

    //now encode contents...
    std::vector<InventoryItemRef> equipped;
    std::vector<InventoryItemRef> integrated;
    //find all the equipped items and rigs
    FindByFlagRange( flagLowSlot0, flagFixedSlot, equipped );
    FindByFlagRange( flagRigSlot0, flagRigSlot7, integrated );
    //append them into one list
    equipped.insert(equipped.end(), integrated.begin(), integrated.end() );
    //encode an entry for each one.
    std::vector<InventoryItemRef>::iterator cur = equipped.begin();
    for(; cur != equipped.end(); cur++)
    {
        if ( !(*cur)->Populate( entry ) )
        {
            codelog( ITEM__ERROR, "%s (%u): Failed to load item %u for ShipGetInfo", itemName().c_str(), itemID(), (*cur)->itemID() );
        }
        else
            result->SetItem(new PyInt((*cur)->itemID()), new PyObject("util.KeyVal", entry.Encode()));
    }

    return result;

}

// this one is called from GetAllInfo, and may be a bit off in the response.
PyDict* Ship::GetShipInfo()
{
    /*
              [PyString "shipInfo"]
              [PyDict 14 kvp]
                [PyIntegerVar 1006132995446]
                [PyObjectData Name: util.KeyVal]
                  [PyDict 5 kvp]
                    [PyString "itemID"]
                    [PyIntegerVar 1006132995446]
                    [PyString "attributes"]
                    [PyDict 17 kvp]
                      [PyInt 161]
                      [PyFloat 5]
                      .........
                      [PyInt 565]
                      [PyFloat 0.56]
                    [PyString "invItem"]
                    [PyPackedRow 37 bytes]
                      ["itemID" => <1006132995446> [I8]]
                      ["typeID" => <16301> [I4]]
                      ["ownerID" => <1661059544> [I4]]
                      ["locationID" => <1006132945754> [I8]]
                      ["flagID" => <12> [I2]]
                      ["quantity" => <-1> [I4]]
                      ["groupID" => <315> [I4]]
                      ["categoryID" => <7> [I4]]
                      ["customInfo" => <empty string> [Str]]
                    [PyString "time"]
                    [PyIntegerVar 129773015518415424]
                    [PyString "activeEffects"]
                    [PyDict 1 kvp]
                      [PyInt 16]
                      [PyList 11 items]
                        [PyIntegerVar 1006132995446]
                        [PyIntegerVar 1661059544]
                        [PyIntegerVar 1006132945754]
                        [PyNone]
                        [PyNone]
                        [PyList 0 items]
                        [PyInt 16]
                        [PyIntegerVar 129773015508502912]
                        [PyInt -1]
                        [PyInt 1]
                        [PyNone]
                */
    if ( !LoadContents( &m_factory ) )
    {
        codelog( ITEM__ERROR, "%s (%u): Failed to load contents for ShipGetInfo", itemName().c_str(), itemID() );
        return NULL;
    }

    PyDict *result = new PyDict;
    Rsp_CommonGetInfo_Entry entry;

    //first populate the ship.
    if ( !Populate( entry ) )
        return NULL;    //print already done.

    result->SetItem(new PyInt( itemID()), new PyObject("util.KeyVal", entry.Encode()));

    //now encode contents...
    std::vector<InventoryItemRef> equipped;
    std::vector<InventoryItemRef> integrated;
    //find all the equipped items and rigs
    FindByFlagRange( flagLowSlot0, flagFixedSlot, equipped );
    FindByFlagRange( flagRigSlot0, flagRigSlot7, integrated );
    //append them into one list
    equipped.insert(equipped.end(), integrated.begin(), integrated.end() );
    //encode an entry for each one.
    std::vector<InventoryItemRef>::iterator cur = equipped.begin();
    for(; cur != equipped.end(); cur++)
    {
        if ( !(*cur)->Populate( entry ) )
        {
            codelog( ITEM__ERROR, "%s (%u): Failed to load item %u for ShipGetInfo", itemName().c_str(), itemID(), (*cur)->itemID() );
        }
        else
            result->SetItem(new PyInt((*cur)->itemID()), new PyObject("util.KeyVal", entry.Encode()));
    }

    return result;
}

PyDict* Ship::ShipGetState()
{
    if ( !LoadContents( &m_factory ) )
    {
        codelog( ITEM__ERROR, "%s (%u): Failed to load contents for ShipGetInfo", itemName().c_str(), itemID() );
        return NULL;
    }

	// Create new dictionary for "shipState":
    PyDict *result = new PyDict;

	// Create entry in "shipState" dictionary for Ship itself:
    result->SetItem(new PyInt(itemID()), GetItemStatusRow());

	// Create entries in "shipState" dictionary for ALL modules, rigs, and subsystems present on ship:
	std::vector<InventoryItemRef> moduleList;
	m_ModuleManager->GetModuleListOfRefs( &moduleList );

	for (int i=0; i<moduleList.size(); i++)
		result->SetItem(new PyInt(moduleList.at(i)->itemID()), moduleList.at(i)->GetItemStatusRow());

	return result;
}

PyList* Ship::ShipGetModuleList()
{
    if ( !LoadContents( &m_factory ) )
    {
        codelog( ITEM__ERROR, "%s (%u): Failed to load contents for ShipGetInfo", itemName().c_str(), itemID() );
        return NULL;
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

PyDict* Ship::ShipGetModuleInfo()
{
    if ( !LoadContents( &m_factory ) )
    {
        codelog( ITEM__ERROR, "%s (%u): Failed to load contents for ShipGetInfo", itemName().c_str(), itemID() );
        return NULL;
    }

    // Create new dictionary for "shipState":
    PyDict *result = new PyDict;
    PyDict *result2 = new PyDict;

    // Create entries in "shipState" dictionary for ALL ONLINE modules, rigs, and subsystems present on ship:
    std::vector<InventoryItemRef> moduleList;
    m_ModuleManager->GetModuleListOfRefs( &moduleList );

    for (int i=0; i<moduleList.size(); i++)
        if (moduleList.at(i)->IsOnline())
            result2->SetItem(new PyInt(moduleList.at(i)->flag()), moduleList.at(i)->GetModuleStatusRow());

    result->SetItem(new PyInt(itemID()), result2);
    return result;
}

PyDict* Ship::ShipGetWeaponInfo()
{
    if ( !LoadContents( &m_factory ) )
    {
        codelog( ITEM__ERROR, "%s (%u): Failed to load contents for ShipGetInfo", itemName().c_str(), itemID() );
        return NULL;
    }

    PyDict *result = new PyDict;
    PyDict *result2 = new PyDict;

    // Create entries in "shipState" dictionary for highslot modules present on ship:
    std::vector<InventoryItemRef> moduleList;
    m_ModuleManager->GetModuleListOfRefs( &moduleList );

    for (int i=0; i<moduleList.size(); i++)
        if ((moduleList.at(i)->flag() >= flagHiSlot0) && (moduleList.at(i)->flag() <= flagHiSlot7))
            result2->SetItem(new PyInt(moduleList.at(i)->flag()), moduleList.at(i)->GetModuleStatusRow());

        result->SetItem(new PyInt(itemID()), result2);
    return result;
}

void Ship::AddItem(InventoryItemRef item)
{
    InventoryEx::AddItem( item );

    if ( item->flag() >= flagSlotFirst &&
        item->flag() <= flagSlotLast &&
        item->categoryID() != EVEDB::invCategories::Charge)
    {
        // make singleton
        item->ChangeSingleton( true );
    }
}

bool Ship::ValidateBoardShip(ShipRef ship, CharacterRef character) {
    SkillRef requiredSkill;
    EvilNumber skillTypeID;

    if ( ship->HasAttribute(AttrRequiredSkill1, skillTypeID) )
        if ( !(character->HasSkillTrainedToLevel( skillTypeID.get_int(), ship->GetAttribute(AttrRequiredSkill1Level).get_int()) )) return false;
    if ( ship->HasAttribute(AttrRequiredSkill2, skillTypeID) )
        if ( !(character->HasSkillTrainedToLevel( skillTypeID.get_int(), ship->GetAttribute(AttrRequiredSkill2Level).get_int() ))) return false;
    if ( ship->HasAttribute(AttrRequiredSkill3, skillTypeID) )
        if ( !(character->HasSkillTrainedToLevel( skillTypeID.get_int(), ship->GetAttribute(AttrRequiredSkill3Level).get_int() ))) return false;
    if ( ship->HasAttribute(AttrRequiredSkill4, skillTypeID) )
        if ( !(character->HasSkillTrainedToLevel( skillTypeID.get_int(), ship->GetAttribute(AttrRequiredSkill4Level).get_int() ))) return false;
    if ( ship->HasAttribute(AttrRequiredSkill5, skillTypeID) )
        if ( !(character->HasSkillTrainedToLevel( skillTypeID.get_int(), ship->GetAttribute(AttrRequiredSkill5Level).get_int() ))) return false;
    if ( ship->HasAttribute(AttrRequiredSkill6, skillTypeID) )
        if ( !(character->HasSkillTrainedToLevel( skillTypeID.get_int(), ship->GetAttribute(AttrRequiredSkill6Level).get_int() ))) return false;
    return true;
}

void Ship::SaveShip()
{
    sLog.Debug( "Ship::SaveShip()", "Saving all 'entity' info and attribute info to DB for ship %s (%u)...", itemName().c_str(), itemID() );

    SaveItem();                         // Save all attributes and item info
    m_ModuleManager->SaveModules();     // Save all attributes and item info for all modules fitted to this ship
}

bool Ship::ValidateItemSpecifics(InventoryItemRef equip) {

    //declaring explicitly as int...not sure if this is needed or not
    int groupID = m_pOperator->GetShip()->groupID();
    int typeID = m_pOperator->GetShip()->typeID();
    /*
    EvilNumber canFitShipGroup1, canFitShipGroup2, canFitShipGroup3, canFitShipGroup4;
    EvilNumber canFitShipType1, canFitShipType2, canFitShipType3, canFitShipType4;
    */
    EvilNumber canFitShipGroup1;
    EvilNumber canFitShipGroup2;
    EvilNumber canFitShipGroup3;
    EvilNumber canFitShipGroup4;

    EvilNumber canFitShipType1;
    EvilNumber canFitShipType2;
    EvilNumber canFitShipType3;
    EvilNumber canFitShipType4;

    // If a ship group restriction is specified the item
    // must be able to fit to at least one ship group.

    if (equip->HasAttribute(AttrCanFitShipGroup1, canFitShipGroup1) ||
        equip->HasAttribute(AttrCanFitShipGroup2, canFitShipGroup2) ||
        equip->HasAttribute(AttrCanFitShipGroup3, canFitShipGroup3) ||
        equip->HasAttribute(AttrCanFitShipGroup4, canFitShipGroup4) ){
    	_log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - Beginning the validation:");
    	_log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - AttrCanFitShipGroup1 = %s", equip->HasAttribute(AttrCanFitShipGroup1, canFitShipGroup1) ? "True":"False");
    	_log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - AttrCanFitShipGroup2 = %s", equip->HasAttribute(AttrCanFitShipGroup2, canFitShipGroup2) ? "True":"False");
    	_log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - AttrCanFitShipGroup3 = %s", equip->HasAttribute(AttrCanFitShipGroup3, canFitShipGroup3) ? "True":"False");
    	_log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - AttrCanFitShipGroup4 = %s", equip->HasAttribute(AttrCanFitShipGroup4, canFitShipGroup4) ? "True":"False");
        if ( (canFitShipGroup1 != groupID) && (canFitShipGroup2 != groupID) && (canFitShipGroup3 != groupID) && (canFitShipGroup4 != groupID) ){
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
        equip->HasAttribute(AttrCanFitShipType4, canFitShipType4) ){
    	_log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - Beginning the validation:");
    	_log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - AttrCanFitShipType1 = %s", equip->HasAttribute(AttrCanFitShipType1, canFitShipType1) ? "True":"False");
    	_log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - AttrCanFitShipType2 = %s", equip->HasAttribute(AttrCanFitShipType2, canFitShipType2) ? "True":"False");
    	_log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - AttrCanFitShipType3 = %s", equip->HasAttribute(AttrCanFitShipType3, canFitShipType3) ? "True":"False");
    	_log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - AttrCanFitShipType4 = %s", equip->HasAttribute(AttrCanFitShipType4, canFitShipType4) ? "True":"False");
        if ( (canFitShipType1 != typeID) && (canFitShipType2 != typeID) && (canFitShipType3 != typeID) && (canFitShipType4 != typeID) ){
        	_log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - No attribute found. typeID = %i", typeID);
            return false;
        }
        else
        	_log(SHIP__MODULE_TRACE, "Ship::ValidateItemSpecifics - Validation passed. Fitting the module");
    }

    return true;
}

void Ship::Dock() {
    DeactivateAllModules();
}

void Ship::Undock() {
    if (sConfig.world.testServer) {
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

void Ship::Heal()
{
    // Heal Ship and Fully Recharge Capacitor:
    SetShipShield(1.0);
    SetShipCapacitorLevel(1.0);
    SetShipArmor(1.0);
    SetShipHull(1.0);
}

void Ship::AddModuleToOnlineVec(uint32 moduleID)
{
    m_onlineModuleVec.push_back(moduleID);
}

//  Updated fractional ship defense settings.  -allan 1Feb15
void Ship::SetShipCapacitorLevel(double fraction)
{
    if ( fraction > 1.0 ) fraction = 1.0;
    if ( fraction < 0.0 ) fraction = 0.0;

    EvilNumber newCapacitorCharge = 0.0;
    newCapacitorCharge = GetAttribute(AttrCapacitorCapacity) * fraction;
    if ( (newCapacitorCharge + 0.5) > GetAttribute(AttrCapacitorCapacity) )
        newCapacitorCharge = GetAttribute(AttrCapacitorCapacity);
    if ( (newCapacitorCharge - 0.5) < 0 )
        newCapacitorCharge = 0;

    SetAttribute(AttrCapacitorCharge, newCapacitorCharge);
}

void Ship::SetShipShield(double fraction)
{
    if ( fraction > 1.0 ) fraction = 1.0;
    if ( fraction < 0.0 ) fraction = 0.0;

    EvilNumber newShieldCharge = 0.0;
    newShieldCharge = GetAttribute(AttrShieldCapacity) * fraction;
    if ( (newShieldCharge + 0.2) > GetAttribute(AttrShieldCapacity) )
        newShieldCharge = GetAttribute(AttrShieldCapacity);
    if ( (newShieldCharge - 0.2) < 0 )
        newShieldCharge = 0;

    SetAttribute(AttrShieldCharge, newShieldCharge);
}

void Ship::SetShipArmor(double fraction)
{
    fraction = 1 - fraction;

    if ( fraction > 1.0 ) fraction = 1.0;
    if ( fraction < 0.0 ) fraction = 0.0;

    EvilNumber newArmorDamage = 0.0;
    newArmorDamage = GetAttribute(AttrArmorHP) * fraction;
    if ( (newArmorDamage + 0.2) > GetAttribute(AttrArmorHP) )
        newArmorDamage = GetAttribute(AttrArmorHP);
    if ( (newArmorDamage - 0.2) < 0 )
        newArmorDamage = 0;

    SetAttribute(AttrArmorDamage, newArmorDamage);
}

void Ship::SetShipHull(double fraction)
{
    fraction = 1 - fraction;

    if ( fraction > 1.0 ) fraction = 1.0;
    if ( fraction < 0.0 ) fraction = 0.0;

    EvilNumber newHullDamage = 0.0;
    newHullDamage = GetAttribute(AttrHP) * fraction;
    if ( (newHullDamage + 0.2) > GetAttribute(AttrHP) )
        newHullDamage = GetAttribute(AttrHP);
    if ( (newHullDamage - 0.2) < 0 )
        newHullDamage = 0;

    SetAttribute(AttrDamage, newHullDamage);
}

void Ship::PayInsurance() {
    GetOperator()->GetChar()->AlterBalance(m_db.GetShipInsurancePayout(itemID()));
    m_db.DeleteInsuranceByShipID(itemID());
}


/* Begin new Module Manager Interface */
InventoryItemRef Ship::GetModule(EVEItemFlags flag)
{
	if ( m_ModuleManager->GetModule(flag) != NULL )
		return (m_ModuleManager->GetModule(flag))->getItem();
	else
		return InventoryItemRef();
}

InventoryItemRef Ship::GetModule(uint32 itemID)
{
	if ( m_ModuleManager->GetModule(itemID) != NULL )
		return (m_ModuleManager->GetModule(itemID))->getItem();
	else
		return InventoryItemRef();
}

EVEItemFlags Ship::FindAvailableModuleSlot(InventoryItemRef item) {
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

uint32 Ship::AddItem(EVEItemFlags flag, InventoryItemRef item)
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
        _IncreaseCargoHoldsUsedVolume( flag, (item->GetAttribute(AttrVolume).get_float() * item->quantity()) );
	}

    item->Move(itemID(), flag);
	if (IsModuleSlot(flag)) {
        item->PutOnline();
        m_ModuleManager->Online(item->itemID());
        UpdateModules(flag);
    }

	return item->itemID();
}

void Ship::RemoveItem(InventoryItemRef item/*, uint32 inventoryID, EVEItemFlags flag*/)
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
        _DecreaseCargoHoldsUsedVolume( item->flag(), (item->GetAttribute(AttrVolume).get_float() * item->quantity()) );
}

void Ship::MoveModuleSlot(EVEItemFlags slot1, EVEItemFlags slot2) {
    // slot1 is occupied, as this is location module is from.
    InventoryItemRef modItemRef1 = GetModule(slot1);
    if (!modItemRef1) {
        _log(SHIP__MODULE_TRACE, "Ship::MoveModuleSlot - modItemRef1 is null." );
        m_pOperator->GetClient()->SendNotifyMsg("There was an internal error.  The module to move was not found.");
        return;
    }
    InventoryItemRef chargeItemRef1 = m_ModuleManager->GetLoadedChargeOnModule(slot1);
    if (chargeItemRef1)
        m_ModuleManager->UnloadCharge(slot1);
    //m_ModuleManager->UnfitModule(modItemRef1->itemID());
    modItemRef1->Move(itemID(), flagCargoHold);

    if (m_ModuleManager->IsSlotOccupied(slot2)) {
        // dropped slot is occupied.  procede with moving the module currently in this slot.
        InventoryItemRef modItemRef2 = GetModule(slot2);
        InventoryItemRef chargeItemRef2 = m_ModuleManager->GetLoadedChargeOnModule(slot2);
        if (chargeItemRef2)
            m_ModuleManager->UnloadCharge(slot2);
        //m_ModuleManager->UnfitModule(modItemRef2->itemID());
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

void Ship::UpdateModules()
{
    // List of callees to put this function into context as to what it should be doing:
    // Client::BoardShip()              - put modules online that are recorded with attributeID 2 as being online / skill check all modules and if any fail, keep those OFFLINE
    // InventoryBound::_ExecAdd()       - things have been added or removed, recheck all modules for... some reason
    // Client::MoveItem()               - something has been moved into or out of the ship, recheck all modules for... some reason
    m_ModuleManager->UpdateModules();
    //sLog.Error( "Ship::UpdateModules()", "We are currently not checking for modules that need to go online, or skill checking character for any modules of a newly boarded ship, or updating module states based on things being moved into or off the ship!" );
    //sLog.Error( "Ship::UpdateModules()", "This should really be a simple call to a function ModuleManager::UpdateModules() and the code put inside there." );
}

void Ship::UpdateModules(EVEItemFlags flag)
{
	// List of callees to put this function into context as to what it should be doing:
	// Client::BoardShip()				- put modules online that are recorded with attributeID 2 as being online / skill check all modules and if any fail, keep those OFFLINE
	// InventoryBound::_ExecAdd()		- things have been added or removed, recheck all modules for... some reason
	// Client::MoveItem()				- something has been moved into or out of the ship, recheck all modules for... some reason
    m_ModuleManager->UpdateModules(flag);
}

void Ship::UnloadModule(uint32 itemID)
{
    m_ModuleManager->UnfitModule(itemID);
}

void Ship::UnloadAllModules()
{
    m_ModuleManager->UnloadAllModules();
}

void Ship::RepairModules()
{
    // FIXME TODO get module IDs and send to function
    uint32 modID = 0;
    m_ModuleManager->RepairModule(modID);
}

void Ship::Online (uint32 moduleID)
{
	m_ModuleManager->Online(moduleID);
}

void Ship::Offline (uint32 moduleID)
{
	m_ModuleManager->Offline(moduleID);
}

void Ship::Activate(int32 itemID, std::string effectName, int32 targetID, int32 repeat)
{
    m_ModuleManager->Activate( itemID, effectName, targetID, repeat );
}

void Ship::Deactivate(int32 itemID, std::string effectName)
{
    m_ModuleManager->Deactivate(itemID, effectName);
}

void Ship::Overload()
{
    // FIXME TODO get module IDs and send to function
    uint32 modID = 0;
    m_ModuleManager->Overload(modID);
}

void Ship::CancelOverloading()
{
    // FIXME TODO get module IDs and send to function
    uint32 modID = 0;
    m_ModuleManager->DeOverload(modID);
}

void Ship::RemoveRig(InventoryItemRef item) {
    //may not look like it, but just moving this item will call ModuleManager::UninstallRig().
    item->Move(itemID(), flagCargoHold);
}

double Ship::CalculateRechargeRate(double Capacity, double RechargeTimeMS, double Current)
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
    double profileStartTime = 0.0;
    if (sConfig.misc.UseProfiling)
        profileStartTime = GetTimeUSeconds();
    // Do Automatic Shield and Capacitor Recharge:
    if (m_processTimer.Check()) {
        // Get the elapsed interval.
        double interval = m_processTimerTick / 1000.0;

        // shield
        double shieldCharge = GetAttribute(AttrShieldCharge).get_float();
        double shieldCapacity = GetAttribute(AttrShieldCapacity).get_float();
        if (shieldCharge < shieldCapacity) {
            double shieldRechargeRate = GetAttribute(AttrShieldRechargeRate).get_float();
            // 5% decrease in recharge time
            shieldRechargeRate *= (1 - ( 0.05 * (m_pOperator->GetChar()->GetSkillLevel(skillShieldOperation, true))));
            double newCharge = shieldCharge + (interval * CalculateRechargeRate(shieldCapacity, shieldRechargeRate, shieldCharge));
            if (newCharge > shieldCapacity)
                newCharge = shieldCapacity;
            // if capacity is very close to full charge set to full to prevent lots of VERY small updates.
            if ((shieldCapacity - newCharge) < 0.1)
                newCharge = shieldCapacity;
            SetAttribute(AttrShieldCharge, newCharge);
            _log(COMMON__MESSAGE, "Ship::Process(): %s(%u) - New Shield Charge: %f",\
                    m_pOperator->GetName(), m_pOperator->GetShip().get()->itemID(), newCharge );
        }

        // capacitor
        double capCharge = GetAttribute(AttrCapacitorCharge).get_float();
        double capCapacity = GetAttribute(AttrCapacitorCapacity).get_float();
        if (capCharge < capCapacity) {
            double capRechargeRate = GetAttribute(AttrRechargeRate).get_float();
            // 5% decrease in recharge time
            capRechargeRate *= (1 - ( 0.05 * (m_pOperator->GetChar()->GetSkillLevel(skillEnergySystemsOperation, true))));
            double newCharge = capCharge + (interval *CalculateRechargeRate(capCapacity, capRechargeRate, capCharge));
            if (newCharge > capCapacity)
                newCharge = capCapacity;
            // if capacity is very close to full charge set to full to prevent lots of VERY small updates.
            if ((capCapacity - newCharge) < 0.1)
                newCharge = capCapacity;
            SetAttribute(AttrCapacitorCharge, newCharge);
            _log(COMMON__MESSAGE, "Ship::Process(): %s(%u) - New Cap Charge: %f",\
                        m_pOperator->GetName(), m_pOperator->GetShip().get()->itemID(), newCharge );
        }
    }

    // profile timer for JUST the ship shit
    if (sConfig.misc.UseProfiling)
        sProfile.AddTime(_shipProfile, GetTimeUSeconds() - profileStartTime);

    // now, process the modules.
    // Do this last so repair modules don't degrade shield recharge rate.
    // Although, cap recharge would benefit from the power use by modules.
    m_ModuleManager->Process();
}

void Ship::OnlineAll()
{
    m_ModuleManager->OnlineAll();
}

void Ship::OfflineAll()
{
    m_ModuleManager->OfflineAll();
}


void Ship::ReplaceCharges(EVEItemFlags flag, InventoryItemRef newCharge)
{

}

void Ship::DeactivateAllModules()
{
    m_ModuleManager->DeactivateAllModules();
}

std::vector<GenericModule *> Ship::GetStackedItems(uint32 typeID, ModulePowerLevel level)
{
    return m_ModuleManager->GetStackedItems(typeID, level);
}

/* End new Module Manager Interface */

using namespace Destiny;

ShipEntity::ShipEntity(
    ShipRef ship,
    SystemManager *system,
    PyServiceMgr &services,
    const GPoint &position)
: DynamicSystemEntity(new DestinyManager(this, system), ship),
  m_system(system),
  m_services(services)
{
    _shipRef = ship;
    m_destiny->SetPosition(position, false);
    m_podShipID = 0;
}

ShipEntity::~ShipEntity()
{
}

void ShipEntity::Process()
{
    SystemEntity::Process();
}

void ShipEntity::ForcedSetPosition( const GPoint &pt ) {
    m_destiny->SetPosition(pt, false);
}

void ShipEntity::EncodeDestiny( Buffer& into ) const
{
    // this is an entity in space NOT owned by a player
    const GPoint& position = GetPosition();

    uint8 mode = Destiny::DSTBALL_STOP;
    if (Destiny()->IsWarping())
        mode = Destiny::DSTBALL_WARP;
    else if (Destiny()->IsFollowing())
        mode = Destiny::DSTBALL_FOLLOW;
    else if (Destiny()->IsOrbiting())
        mode = Destiny::DSTBALL_ORBIT;
    else if (Destiny()->IsMoving())
        mode = Destiny::DSTBALL_GOTO;

    Destiny::BallHeader head;
    head.entityID = GetID();
    head.mode = mode;
    head.radius = GetRadius();
    head.x = position.x;
    head.y = position.y;
    head.z = position.z;
    head.flags = Destiny::IsMassive | Destiny::IsFree;
    into.Append( head );

    Destiny::MassSector mass;
    mass.mass = GetMass();
    mass.cloak = 0;
    mass.Harmonic = -1.0f;
    mass.corporationID = GetCorporationID();
    mass.allianceID = GetAllianceID();
    into.Append( mass );

    Destiny::ShipSector ship;
    ship.maxVelocity = GetMaxVelocity();
    ship.velocity_x = GetVelocity().x;
    ship.velocity_y = GetVelocity().y;
    ship.velocity_z = GetVelocity().z;
    ship.agility = GetAgility();
    ship.speedfraction = m_destiny->GetSpeedFraction();
    into.Append( ship );

    if (mode == Destiny::DSTBALL_WARP) {
        GPoint target = m_destiny->GetTargetPoint();
        Destiny::DSTBALL_WARP_Struct warp;
        warp.effectStamp = -1;   //unknown value  seen many -1, few other random 4-5 digits
        warp.unknown_x = target.x;
        warp.unknown_y = target.y;
        warp.unknown_z = target.z;
        warp.ownerID = m_destiny->GetWarpSpeed();       //ship warp speed x10  (dont ask...this is what it is...more dumb ccp shit)
        warp.unk_1 = 0;      //unknown 64bit number.  seen 4666723172467343360 once....others are 0
        warp.unk_2 = 0;         //unknown 64bit number
        into.Append( warp );
    } else if (mode == Destiny::DSTBALL_FOLLOW) {
        Destiny::DSTBALL_FOLLOW_Struct follow;
        follow.followID = m_destiny->GetTargetID();
        follow.followRange = m_destiny->GetFollowDistance();
        follow.formationID = 0xFF;
        into.Append( follow );
    } else if (mode == Destiny::DSTBALL_ORBIT) {
        Destiny::DSTBALL_ORBIT_Struct orbit;
        orbit.followID = m_destiny->GetTargetID();
        orbit.followRange = m_destiny->GetFollowDistance();
        orbit.formationID = 0xFF;
        into.Append( orbit );
    } else if (mode == Destiny::DSTBALL_GOTO) {
        GPoint target = m_destiny->GetTargetPoint();
        Destiny::DSTBALL_GOTO_Struct go;
        go.x = target.x;
        go.y = target.y;
        go.z = target.z;
        into.Append( go );
    } else {
        Destiny::DSTBALL_STOP_Struct main;
        main.formationID = 0xFF;
        into.Append( main );
    }

    _log(COMMON__WARNING, "ShipEntity::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void ShipEntity::MakeDamageState(DoDestinyDamageState &into) const
{
    into.shield = (m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float() +7;
    into.timestamp = Win32TimeNow();
    into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float());
}

PyDict* ShipEntity::MakeSlimItem() const {
    _log(COMMON__WARNING, "MakeSlimItem for ShipEntity %s(%u)", m_self->itemName().c_str(), m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",           new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",           new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",          new PyInt(m_self->ownerID()));
        slim->SetItemString("name",             new PyString(m_self->itemName()));
        slim->SetItemString("corpID",           new PyInt(GetCorporationID()));
        slim->SetItemString("allianceID",       new PyInt(GetAllianceID()));
        slim->SetItemString("warFactionID",     new PyInt(GetWarFactionID()));
        slim->SetItemString("nameID",           new PyNone);
        slim->SetItemString("bounty",           new PyInt(GetOwnerBounty()));
        if (m_self->itemID() == itemTypeCapsule)
            slim->SetItemString("launcherID",   new PyInt(GetPodShipID()));
        else {
            slim->SetItemString("categoryID",       new PyInt(m_self->categoryID()));
            slim->SetItemString("groupID",          new PyInt(m_self->groupID()));
        }

    return (slim);
}
