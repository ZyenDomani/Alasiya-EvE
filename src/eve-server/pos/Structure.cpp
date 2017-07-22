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
    Author:     Aknor Jaden
    Updates:    Allan
*/

#include "eve-server.h"

#include <PyServiceMgr.h>
#include "Client.h"
#include "EntityList.h"
#include "EVEServerConfig.h"
#include "manufacturing/Blueprint.h"
#include "pos/Structure.h"
#include "system/Container.h"
#include "system/Damage.h"
#include "system/LootSystem.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
/*
POS__WARNING=1
POS__MESSAGE=0
POS__DEBUG=1
POS__DESTINY=0
POS__SLIMITEM=0
POS__TRACE=0
*/
/*
 * Structure
 */
StructureItem::StructureItem(ItemFactory &_factory, uint32 _structureID, const ItemType &_itemType, const ItemData &_data)
: InventoryItem(_factory, _structureID, _itemType, _data)
{
    m_inventory = new Inventory(InventoryItemRef(this));
    _log(ITEM__TRACE, "Created StructureItem for %s (%u).", itemName().c_str(), itemID());
    _log(POS__TRACE, "Created StructureItem for %s (%u).", itemName().c_str(), itemID());
}

StructureItem::~StructureItem()
{
    SafeDelete(m_inventory);
}

StructureItemRef StructureItem::Load(ItemFactory &factory, uint32 structureID)
{
    return InventoryItem::Load<StructureItem>( factory, structureID );
}

bool StructureItem::_Load()
{
    if( !m_inventory->LoadContents( &m_factory ) )
        return false;

    return InventoryItem::_Load();
}

StructureItemRef StructureItem::Spawn(ItemFactory &factory, ItemData &data)
{
    uint32 structureID = StructureItem::CreateItemID( factory, data );
    if (!structureID)
        return StructureItemRef();
    StructureItemRef sRef = StructureItem::Load( factory, structureID );
    // check for customs offices and set global flag
    if ((data.typeID == EVEDB::invTypes::typeInterbusCustomsOffice)
        or (data.typeID == EVEDB::invTypes::typePlanetaryCustomsOffice)) {
        sRef->SetAttribute(AttrIsGlobal,                        1);
    }
    // Create default dynamic attributes in the AttributeMap:
    sRef->SetAttribute(AttrMass,                                sRef->type().mass());
    sRef->SetAttribute(AttrRadius,                              sRef->type().radius());
    sRef->SetAttribute(AttrVolume,                              sRef->type().volume());
    sRef->SetAttribute(AttrCapacity,                            sRef->type().capacity());
    sRef->SetAttribute(AttrShieldCharge,                        sRef->GetAttribute(AttrShieldCapacity));

    // Check for existence of some attributes that may or may not have already been loaded and set them
    // to default values:
    if (!sRef->HasAttribute(AttrDamage))                        sRef->SetAttribute(AttrDamage, 0.0f, false );
    if (!sRef->HasAttribute(AttrArmorDamage))                   sRef->SetAttribute(AttrArmorDamage, 0.0f, false );
    if (!sRef->HasAttribute(AttrArmorMaxDamageResonance))       sRef->SetAttribute(AttrArmorMaxDamageResonance, 1.0f);
    if (!sRef->HasAttribute(AttrShieldMaxDamageResonance))      sRef->SetAttribute(AttrShieldMaxDamageResonance, 1.0f);

    // Shield Resonance
    if (!sRef->HasAttribute(AttrShieldEmDamageResonance))       sRef->SetAttribute(AttrShieldEmDamageResonance, 1.0);
    if (!sRef->HasAttribute(AttrShieldExplosiveDamageResonance)) sRef->SetAttribute(AttrShieldExplosiveDamageResonance, 1.0);
    if (!sRef->HasAttribute(AttrShieldKineticDamageResonance))  sRef->SetAttribute(AttrShieldKineticDamageResonance, 1.0);
    if (!sRef->HasAttribute(AttrShieldThermalDamageResonance))  sRef->SetAttribute(AttrShieldThermalDamageResonance, 1.0);
    if (!sRef->HasAttribute(AttrArmorEmDamageResonance))        sRef->SetAttribute(AttrArmorEmDamageResonance, 1.0);
    if (!sRef->HasAttribute(AttrArmorExplosiveDamageResonance)) sRef->SetAttribute(AttrArmorExplosiveDamageResonance, 1.0);
    if (!sRef->HasAttribute(AttrArmorKineticDamageResonance))   sRef->SetAttribute(AttrArmorKineticDamageResonance, 1.0);
    if (!sRef->HasAttribute(AttrArmorThermalDamageResonance))   sRef->SetAttribute(AttrArmorThermalDamageResonance, 1.0);
    if (!sRef->HasAttribute(AttrEmDamageResonance))             sRef->SetAttribute(AttrEmDamageResonance, 1.0);
    if (!sRef->HasAttribute(AttrExplosiveDamageResonance))      sRef->SetAttribute(AttrExplosiveDamageResonance, 1.0);
    if (!sRef->HasAttribute(AttrKineticDamageResonance))        sRef->SetAttribute(AttrKineticDamageResonance, 1.0);
    if (!sRef->HasAttribute(AttrThermalDamageResonance))        sRef->SetAttribute(AttrThermalDamageResonance, 1.0);

    return sRef;
}

uint32 StructureItem::CreateItemID(ItemFactory &factory, ItemData &data)
{
    return InventoryItem::CreateItemID( factory, data );
}

void StructureItem::Delete()
{
    // delete contents first
    m_inventory->DeleteContents();

    InventoryItem::Delete();
}

void StructureItem::ValidateAddItem(EVEItemFlags flag, InventoryItemRef item) const
{
    /** @todo update this to new inventory system  */
    EvilNumber capacityUsed(0);
    std::vector<InventoryItemRef> items;
    m_inventory->FindByFlag(flag, items);
    for (auto cur : items)
        capacityUsed += cur->GetAttribute(AttrVolume);
    capacityUsed += item->GetAttribute(AttrVolume);

    /*
     * 1351,1352
    AttrSpecialFuelBayCapacity = 1549,
    AttrSpecialOreHoldCapacity = 1556,
    AttrSpecialGasHoldCapacity = 1557,
    AttrSpecialMineralHoldCapacity = 1558,
    AttrSpecialSalvageHoldCapacity = 1559,
    AttrSpecialShipHoldCapacity = 1560,
    AttrSpecialSmallShipHoldCapacity = 1561,
    AttrSpecialMediumShipHoldCapacity = 1562,
    AttrSpecialLargeShipHoldCapacity = 1563,
    AttrSpecialIndustrialShipHoldCapacity = 1564,
    AttrSpecialAmmoHoldCapacity = 1573,
    */

    /** @todo  check for throwable status here */
    switch (flag) {
        case flagCargoHold: {
            if( capacityUsed > GetAttribute(AttrCapacity) )
                throw PyException( MakeCustomError( "Not enough cargo space!") );
        }
        case flagSecondaryStorage: {
            if( capacityUsed > GetAttribute(AttrCapacitySecondary) )
                throw PyException( MakeCustomError( "Not enough Secondary Storage space!") );
        }
        case flagSpecializedAmmoHold: {
            if( capacityUsed > GetAttribute(AttrSpecialAmmoHoldCapacity) )
                throw PyException( MakeCustomError( "Not enough Ammo Storage space!") );
        }
        case flagSpecializedFuelBay: {
            if( capacityUsed > GetAttribute(AttrSpecialFuelBayCapacity) )
                throw PyException( MakeCustomError( "Not enough Fuel Storage space!") );
        }
    }
}

PyObject *StructureItem::StructureGetInfo()
{
    if (!m_inventory->LoadContents(&m_factory)) {
        codelog( ITEM__ERROR, "%s (%u): Failed to load contents for Structure", itemName().c_str(), itemID() );
        return NULL;
    }

    Rsp_CommonGetInfo result;
    Rsp_CommonGetInfo_Entry entry;

    //first populate the Structure.
    if (!Populate(entry))
        return NULL;

    result.items[ itemID() ] = entry.Encode();

    return result.Encode();
}

void StructureItem::AddItem( InventoryItemRef item )
{
    m_inventory->AddItem( item );
}

void StructureItem::RemoveItem(InventoryItemRef item)
{
    m_inventory->RemoveItem( item );
}


/** @todo (Allan) this class needs more research to finish
 * see pics in ::GamePC/G/games/EvE/misc/POS
    flagStructureActive             = 144,
    flagStructureInactive           = 145,


    AttrOperationConsumptionRate = 687,
    AttrReinforcedConsumptionRate = 688,
    AttrResourceReinforced1Type = 694,
    AttrResourceReinforced2Type = 695,
    AttrResourceReinforced3Type = 696,
    AttrResourceReinforced4Type = 697,
    AttrResourceReinforced5Type = 698,
    AttrResourceReinforced1Quantity = 699,
    AttrResourceReinforced2Quantity = 700,
    AttrResourceReinforced3Quantity = 701,
    AttrResourceReinforced4Quantity = 703,
    AttrResourceReinforced5Quantity = 704,
    AttrResourceOnline1Type = 705,
    AttrResourceOnline2Type = 706,
    AttrResourceOnline3Type = 707,
    AttrResourceOnline4Type = 708,

    ***  many other attributes for towers and their modules.....

    AttrControlTowerMissileVelocityBonus = 792,
    AttrControlTowerSize = 1031,
    AttrAnchoringSecurityLevelMax = 1032,
    AttrAnchoringRequiresSovereignty = 1033,
    AttrControlTowerMinimumDistance = 1165,
    AttrPosPlayerControlStructure = 1167,
    AttrIsIncapacitated = 1168,
    AttrPosStructureControlAmount = 1174,
    AttrOnliningRequiresSovereigntyLevel = 1185,
    AttrPosAnchoredPerSolarSystemAmount = 1195,
    AttrPosStructureControlDistanceMax = 1214,
    AttrAnchoringRequiresSovereigntyLevel = 1215,
    AttrHarvesterType = 709,
    AttrHarvesterQuality = 710,
    AttrMoonAnchorDistance = 711,
    AttrUsageDamagePercent = 712,
    AttrConsumptionType = 713,
    AttrConsumptionQuantity = 714,
    AttrMaxOperationalDistance = 715,
    AttrMaxOperationalUsers = 716,
    AttrRefiningYieldMultiplier = 717,
    AttrOperationalDuration = 719,
    AttrRefineryCapacity = 720,
    AttrRefiningDelayMultiplier = 721,
    AttrPosControlTowerPeriod = 722,
    AttrMoonMiningAmount = 726,
    AttrControlTowerLaserDamageBonus = 728,
    AttrControlTowerLaserOptimalBonus = 750,
    AttrControlTowerHybridOptimalBonus = 751,
    AttrControlTowerProjectileOptimalBonus = 752,
    AttrControlTowerProjectileFallOffBonus = 753,
    AttrControlTowerProjectileROFBonus = 754,
    AttrControlTowerMissileROFBonus = 755,
    AttrControlTowerMoonHarvesterCPUBonus = 756,
    AttrControlTowerSiloCapacityBonus = 757,
    AttrControlTowerLaserProximityRangeBonus = 760,
    AttrControlTowerProjectileProximityRangeBonus = 761,
    AttrControlTowerHybridProximityRangeBonus = 762,
    AttrMaxGroupActive = 763,
    AttrControlTowerEwRofBonus = 764,
    AttrScanRange = 765,
    AttrControlTowerHybridDamageBonus = 766,
    AttrTrackingSpeedBonus = 767,
    AttrMaxRangeBonus2 = 769,
    AttrControlTowerEwTargetSwitchDelayBonus = 770,
    AttrAmmoCapacity = 771,
    AttrActivationBlocked = 1349,
    AttrActivationBlockedStrenght = 1350,
    AttrPosCargobayAcceptType = 1351,
    AttrPosCargobayAcceptGroup = 1352,
    */
StructureSE::StructureSE(StructureItemRef structure, PyServiceMgr &services, SystemManager* system, const FactionData& data)
: ObjectSystemEntity(structure, services, system)
{
    m_co = false;
    m_tcu = false;
    m_pos = false;
    m_sbu = false;
    m_array = false;
    m_bridge = false;
    m_jammer = false;
    m_module = false;
    m_sentry = false;
    m_battery = false;
    m_outpost = false;
    /** @todo  hacked state...need to fix */
    //res = sm.StartService('pwn').GetStructureState(slimItem)[0] in ('online', 'invulnerable', 'vulnerable', 'reinforced')
    m_state = STRUCTURE_ONLINE;
    /** @todo  hacked moonID...need to fix */
    m_moonID = 0;
    /** @todo (Allan) fix this later...used for shield passage */
    m_harmonic = -1;
    m_timestamp = Win32TimeNow() - Win32Time_Day;
    /** @todo (Allan) fix this */
    m_warID = data.factionID;
    m_allyID = data.allianceID;
    m_corpID = data.corporationID;
    m_ownerID = data.ownerID;

    Init(structure);
    _log(SE__DEBUG, "Created StructureSE for item %s (%u).", structure->itemName().c_str(), structure->itemID());
}

void StructureSE::Init(StructureItemRef structure)
{
    switch(structure->groupID()) {
        case EVEDB::invGroups::Orbital_Infrastructure: {
            m_co = true;
            m_planetID = atoi(m_self->customInfo().c_str());
        } break;
        case EVEDB::invGroups::Sovereignty_Blockade_Units: {
            m_sbu = true;
        } break;
        case EVEDB::invGroups::Territorial_Claim_Units: {
            m_tcu = true;
        } break;
        case EVEDB::invGroups::Control_Tower: {
            m_pos = true;
            // create and add force field to tower
            /** @todo  this will need to be based on structure state */
            //ItemData( uint32 _typeID, uint32 _ownerID, uint32 _locationID, EVEItemFlags _flag, uint32 _quantity, const char *_customInfo = "", bool _contraband = false);
            ItemData idata(EVEDB::invTypes::typeForceField, m_corpID, m_system->GetID(), flagAutoFit, m_ownerID);
            InventoryItemRef iRef = m_services.item_factory->SpawnItem(idata);
            if (!iRef)
                break;  // we'll get over it
            iRef->Relocate(GetPosition());
            iRef->SetAttribute(AttrRadius, m_self->GetAttribute(AttrShieldRadius));
            ItemSystemEntity* iSE = new ItemSystemEntity(iRef, m_services, m_system);
            m_system->AddEntity(iSE);
        } break;
        case EVEDB::invGroups::Jump_Portal_Array: {
            m_bridge = true;
            m_module = true;
        } break;
        case EVEDB::invGroups::Mobile_Missile_Sentry:
        case EVEDB::invGroups::Mobile_Projectile_Sentry:
        case EVEDB::invGroups::Mobile_Laser_Sentry:
        case EVEDB::invGroups::Mobile_Hybrid_Sentry: {
            m_sentry = true;
            m_module = true;
        } break;
        case EVEDB::invGroups::Electronic_Warfare_Battery:
        case EVEDB::invGroups::Sensor_Dampening_Battery:
        case EVEDB::invGroups::Stasis_Webification_Battery:
        case EVEDB::invGroups::Warp_Scrambling_Battery:
        case EVEDB::invGroups::Energy_Neutralizing_Battery:
        case EVEDB::invGroups::Target_Painting_Battery: {
            m_battery = true;
            m_module = true;
        } break;
        case EVEDB::invGroups::Refining_Array:
        case EVEDB::invGroups::Ship_Maintenance_Array:
        case EVEDB::invGroups::Assembly_Array:
        case EVEDB::invGroups::Shield_Hardening_Array:
        case EVEDB::invGroups::Force_Field_Array:
        case EVEDB::invGroups::Corporate_Hangar_Array:
        case EVEDB::invGroups::Stealth_Emitter_Array:
        case EVEDB::invGroups::Scanner_Array:
        case EVEDB::invGroups::Logistics_Array:
        case EVEDB::invGroups::Cynosural_Generator_Array:
        case EVEDB::invGroups::Structure_Repair_Array: {
            m_array = true;
            m_module = true;
        } break;
        case EVEDB::invGroups::Cynosural_System_Jammer: {
            /** @todo (Allan) do we need anything else here?  check for and set system-wide cyno jammer? */
            m_jammer = true;
            m_module = true;
        } break;
        default: {
            m_module = true;
        }
    }

    /** @todo (Allan) set/get control tower id for modules in/from customInfo field of db */
    if (m_module)
        m_towerID = atoi(m_self->customInfo().c_str());
}

/*
 * fuel bay = flag 0
 * strot bay = flag 122 (2nd storage)
 */
void StructureSE::Process() {
    /* called by EntityList::Process on every loop */
    /*  Enable base call to Process Targeting and Movement  */
    SystemEntity::Process();
    /** @todo (Allan)  will need some form of AI to engage defensive modules if/when any structure is attacked */
}

void StructureSE::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;

    //const uint16 miniballsCount = GetMiniBalls();

    BallHeader head;
        head.entityID = GetID();
        head.radius = GetRadius();
        head.x = x();
        head.y = y();
        head.z = z();
    if (m_tcu) {
        head.mode = DSTBALL_STOP;
        head.flags = IsGlobal;
    } else if (m_co) {
        head.mode = DSTBALL_RIGID;
        head.flags = IsGlobal /*| HasMiniBalls*/;
    } else {
        head.mode = DSTBALL_RIGID;
        head.flags = IsMassive | IsInteractive /*| HasMiniBalls*/;        //TODO check for miniballs and add here if found.
    }
    into.Append( head );

    DSTBALL_RIGID_Struct main;
        main.formationID = 0xFF;
    into.Append( main );

    if (m_tcu or m_pos) {
        MassSector mass;
            mass.cloak = 0;
            mass.corporationID = GetCorporationID();
            mass.allianceID = GetAllianceID();
            mass.harmonic = m_harmonic;
            mass.mass = m_self->type().mass();
        into.Append( mass );
    }

    /** @todo (Allan) fix this when POS system is more operational */
    /* TODO  query and configure miniballs for entity
     * NOTE  MiniBalls are BROKEN!!!  DO NOT USE!
    into.Append( miniballsCount );

    MiniBall miniball;
    for (int16 i; i<miniballsCount; i++) {
        miniball.x = -7701.181;
        miniball.y = 8060.06;
        miniball.z = 27878.900;
        miniball.radius = 1639.241;
        into.Append( miniball );
        miniball.clear();
    }
                                    [MiniBall]
                                      [Radius: 963.8593]
                                      [Offset: (0, -2302, 1)]
                                    [MiniBall]
                                      [Radius: 1166.27]
                                      [Offset: (0, 1298, 1)]
                                    [MiniBall]
                                      [Radius: 876.2357]
                                      [Offset: (0, -502, 1)]
                                    [MiniBall]
                                      [Radius: 796.5781]
                                      [Offset: (0, 2598, 1)]

                                      */
    _log(SE__DESTINY, "StructureSE::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
    _log(POS__DESTINY, "StructureSE::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

PyDict *StructureSE::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for StructureSE %u", m_self->itemID());
    _log(POS__SLIMITEM, "MakeSlimItem for StructureSE %u", m_self->itemID());
    /** @todo (Allan) *Timestamp will need to be set to time current state is started. */
    PyDict *slim = new PyDict();
        slim->SetItemString("name",                     new PyString(""));
        slim->SetItemString("nameID",                   new PyNone());
        slim->SetItemString("itemID",                   new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",                   new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",                  new PyInt(m_self->ownerID()));  //1000148 for interbus customs office (to be done on creation)

        slim->SetItemString("corpID",                   new PyInt(m_corpID));  //1000148 for interbus customs office (to be done on creation)
        slim->SetItemString("allianceID",               new PyInt(m_allyID));/** @todo (Allan) fix this later */
        slim->SetItemString("warFactionID",             new PyInt(m_warID));/** @todo (Allan) fix this later */
        if (!m_co) {
            slim->SetItemString("posTimestamp",         new PyLong(m_timestamp));
            slim->SetItemString("posState",             new PyInt(GetStructureState()));
            // this is only checked when state == (STRUCTURE_SHIELD_REINFORCE || STRUCTURE_ARMOR_REINFORCE)
            slim->SetItemString("posDelayTime",         new PyInt(GetStructureState()));
            // this is boolean and ONLY included if structure is incapacitated
            if (m_state == STATE_INCAPACITATED)
                slim->SetItemString("incapacitated",    new PyInt(1));
        }
        if (m_outpost) {
            slim->SetItemString("startTimestamp",       new PyLong(m_timestamp));
            slim->SetItemString("structureState",       new PyInt(GetStructureState()));
            slim->SetItemString("delayTime",            new PyInt(0));/** @todo (Allan) fix this later - dont know what it is */
            return slim;
        } else if (m_co) {
            slim->SetItemString("level",                new PyInt(1)); //{1-CUSTOMSOFFICE_SPACEPORT, 2-CUSTOMSOFFICE_SPACEELEVATOR}   this is for display model
            slim->SetItemString("orbitalTimestamp",     new PyLong(m_timestamp));
            slim->SetItemString("planetID",             new PyInt(m_planetID));  // planetID for this orbital
            slim->SetItemString("orbitalState",         new PyInt(GetStructureState()));
            PyTuple* tuple = new PyTuple(3);
                tuple->SetItem(0,                       new PyFloat(m_rotation.x));
                tuple->SetItem(1,                       new PyFloat(m_rotation.y));
                tuple->SetItem(2,                       new PyFloat(m_rotation.z));
            slim->SetItemString("dunRotation", tuple);  // direction to planet
            //  dunno what these are...
            slim->SetItemString("orbitalHackerProgress", new PyNone());
            slim->SetItemString("orbitalHackerID",      new PyNone());
        } else if (m_tcu) {
            slim->SetItemString("posDelayTime",         new PyInt(0));/** @todo (Allan) fix this later - dont know what it is */
            return slim;
        } else if (m_module) {
            slim->SetItemString("controlTowerID",       new PyLong(m_towerID));
        }

    if (is_log_enabled(POS__DEBUG)) {
        _log( POS__DEBUG, "StructureSE::MakeSlimItem()", "%s(%u)", GetName(), GetID());
        slim->Dump(POS__DEBUG, "     ");
    }
    return slim;
}

/*  Log events
eventTCUExploded = 280
eventTCUInvulnerable = 283
eventTCUOffline = 259
eventTCUOnline = 258
eventTCUVulnerable = 282
eventControlTowerAnchored = 364
eventControlTowerUnanchored = 365
eventControlTowerDestroyed = 366
eventSovereigntyClaimed = 197
eventSovereigntyLost = 194
eventSBUExploded = 279
eventSBUOffline = 257
eventSBUOnline = 256
*/
uint8 StructureSE::GetStructureState() const {
    /** @todo (Allan) fix this when POS system is more operational */
    /*
     STRUCTURE_UNANCHORED = 0,
     STRUCTURE_ANCHORED = 1,
     STRUCTURE_ONLINING = 2,
     STRUCTURE_REINFORCED = 3,
     STRUCTURE_ONLINE = 4,
     STRUCTURE_OPERATING = 5,
     STRUCTURE_VULNERABLE = 6,
     STRUCTURE_SHIELD_REINFORCE = 7,
     STRUCTURE_ARMOR_REINFORCE = 8,
     STRUCTURE_INVULNERABLE = 9
    */
    return m_state;   /* hack for pos online */
}

PyTuple *StructureSE::GetEffectState() {
    /** @todo (Allan) fix this when POS system is more operational */
    /* see file:///home/allan/Desktop/cruc/entities/pos_packets/control_tower_packets and cruc/entities/structures for more info */

    std::vector<int32, std::allocator<int32> > area;

    DoDestiny_OnSpecialFX13 effect;
        if (m_module)
            effect.entityID = m_towerID;            /* control tower id */
        else
            effect.entityID = m_self->itemID();     /* control tower id */
        if (!m_co) {
            effect.moduleID = m_self->itemID();         /* structure/module id as part of above tower system */
            effect.moduleTypeID = m_self->typeID();
            effect.targetID = m_self->itemID();
            effect.chargeTypeID = 0;
            effect.duration_ms = -1;
        }
        effect.area = area;
        effect.guid = "effects.StructureOnline";
        effect.isOffensive = 0;                     /** @todo (Allan) this should be boolean */
        effect.start = 1;
        effect.active = 1;
        effect.repeat = 0;
        effect.startTime = m_timestamp;             /* time this effect started */
    PyTuple *update = effect.Encode();
    return update;
}

void StructureSE::Killed(Damage &fatal_blow) {
    if (!m_bubble or !m_destiny) return;

    m_destiny->Halt();

    SystemEntity *killer = fatal_blow.srcSE;
    Client* pClient = nullptr;
    uint32 killerID = 0;

    if (killer->HasPilot()) {
        pClient = killer->GetPilot();
        killerID = pClient->GetCharacterID();
    } else if (killer->IsDroneSE()) {
        pClient = sEntityList.FindClientByCharID( killer->GetSelf()->ownerID() );
        if (!pClient ) {
            sLog.Error("StructureSE::Killed()", "killer == IsDrone and pPlayer == nullptr");
        } else
            killerID = pClient->GetCharacterID();
    } else
        killerID = killer->GetID();

    m_destiny->SendTerminalExplosion(GetID(), m_bubble->GetID());

    GPoint deadPOSPosition = m_destiny->GetPosition();
    uint32 wreckTypeID = sDGM_Types_to_Wrecks_Table.GetWreckID(m_self->typeID());
    if (!wreckTypeID) {
        sLog.Error("StructureSE::Killed()", "Could not get wreckType for %s of type %u", m_self->itemName().c_str(), m_self->typeID());
        // default to generic frigate wreck till i get better checks and/or complete wreck data
        wreckTypeID = 26557;
    }

    uint32 locationID = GetLocationID();
    std::string wreck_name = m_self->itemName();
    wreck_name += " Wreck";
    const char* faction = itoa(m_allyID);
    ItemData wreckItemData(wreckTypeID, killerID, locationID, flagAutoFit, wreck_name.c_str(), deadPOSPosition, faction);
    WreckContainerRef wreckItemRef = m_self->GetItemFactory()->SpawnWreckContainer( wreckItemData );
    if (!wreckItemRef) {
        sLog.Error("StructureSE::Killed()", "Creating Wreck Item Failed for %s of type %u", wreck_name.c_str(), wreckTypeID);
        return;
    }

    if ( pClient ) {
        DropLoot(wreckItemRef, m_self->groupID(), pClient->GetCharacterID());
        //award kill bounty.
        //AwardBounty( pClient );
        //  log faction kill in dynamic data   -allan
        Character* pChar = pClient->GetChar().get();
        pChar->chkDynamicSystemID(locationID);
        pChar->AddKillToDynamicData(locationID);
        pChar->AddFactionKillToDynamicData(locationID);
        if (m_system->GetSystemSecurityRating() > 0)
            AwardSecurityStatus(m_self, pChar);
    } else
        DropLoot(wreckItemRef, m_self->groupID(), killerID);

    DBSystemDynamicEntity wreckEntity;
        wreckEntity.allianceID = killer->GetAllianceID();
        wreckEntity.categoryID = EVEDB::invCategories::Celestial;
        wreckEntity.corporationID = killer->GetCorporationID();
        wreckEntity.factionID = m_warID;
        wreckEntity.groupID = EVEDB::invGroups::Wreck;
        wreckEntity.itemID = wreckItemRef->itemID();
        wreckEntity.itemName = wreck_name;
        wreckEntity.ownerID = killerID;
        wreckEntity.typeID = wreckTypeID;
        wreckEntity.x = deadPOSPosition.x;
        wreckEntity.y = deadPOSPosition.y;
        wreckEntity.z = deadPOSPosition.z;

    if (!m_system->BuildDynamicEntity(wreckEntity)) {
        sLog.Error("StructureSE::Killed()", "Spawning Wreck Failed: typeID or typeName not supported: '%u'", wreckTypeID);
        return;
    }

        std::stringstream blob;
        blob << "<items>";
        std::map<uint32, InventoryItemRef> deadShipInventory;
        deadShipInventory.clear();
        m_self->GetMyInventory()->GetInventoryList(deadShipInventory);
        if (!deadShipInventory.empty()) {
            uint32 s = 0, d = 0, x = 0;
            for (auto cur : deadShipInventory) {
                d = 0;
                x = cur.second->quantity();
                s = (cur.second->singleton() ? 1 : 0);
                if (cur.second->categoryID() == EVEDB::invCategories::Blueprint) {
                    // singleton for bpo = 1, bpc = 2.
                    BlueprintRef bpRef = BlueprintRef::StaticCast(cur.second);
                    s = (bpRef->copy() ? 2 : s);
                }
                blob << "<i t=" << cur.second->typeID() << " f=" << cur.second->flag() << " s=" << s ;
                // all items have 50% chance of drop, even from popped ship
                if (IsEven(MakeRandomInt(0, 100))) {
                    // item survived.  check qty for drop
                    if (x > 1) {
                        d = MakeRandomInt(0, x);
                        x -= d;
                    }
                    // move item to wreck
                    cur.second->Move(wreckItemRef->itemID(),flagAutoFit);
                }
                blob << " d=" << d << " x=" << x << "/>";
            }
        }
        blob << "</items>";

        /* populate kill data for killMail and save to db  -allan 01May16  --updated 13July17 */
        /** @todo  check for tower/tcu/sbu/jammer and make killmail */
        CharKillData data;
            data.solarSystemID = m_system->GetID();
            data.victimCharacterID = 0; // charID = 0 means strucuture/item
            data.victimCorporationID = m_corpID;
            data.victimAllianceID = m_allyID;
            data.victimFactionID = m_warID;
            data.victimShipTypeID = GetTypeID();

            data.finalCharacterID = killerID;
            data.finalCorporationID = killer->GetCorporationID();
            data.finalAllianceID = killer->GetAllianceID();
            data.finalFactionID = killer->GetWarFactionID();
            data.finalShipTypeID = killer->GetTypeID();
            data.finalWeaponTypeID = fatal_blow.weaponRef->typeID();
            data.finalSecurityStatus = 0;  /* fix this */
            data.finalDamageDone = fatal_blow.GetTotal();

        uint32 totalHP = m_self->GetAttribute(AttrHP).get_int();
            totalHP += m_self->GetAttribute(AttrArmorHP).get_int();
            totalHP += m_self->GetAttribute(AttrShieldCapacity).get_int();
            data.victimDamageTaken = totalHP;

            data.killBlob = blob.str().c_str();
            data.killTime = Win32TimeNow();
            data.moonID = m_moonID;    /* denotes moonID for POS/Structure kills */

        ServiceDB::SaveKillOrLoss(data);

    _log(PHYSICS__TRACE, "StructureSE::Killed() - Wreck %s(%u) Item Position: %.2f,%.2f,%.2f.  Destiny Position: %.2f,%.2f,%.2f.", \
    GetName(), GetID(), x(), y(), z(), deadPOSPosition.x, deadPOSPosition.y, deadPOSPosition.z);

    // cleanup and removal
    m_system->RemoveEntity(this);
}

/*
 *    [PyTuple 1 items]
 *      [PyTuple 2 items]
 *        [PyInt 0]
 *        [PySubStream 160 bytes]
 *          [PyTuple 2 items]
 *            [PyInt 0]
 *            [PyTuple 2 items]
 *              [PyInt 1]
 *              [PyTuple 3 items]
 *                [PyList 1 items]
 *                  [PyTuple 2 items]
 *                    [PyInt 12193]
 *                    [PyTuple 2 items]
 *                      [PyString "PackagedAction"]
 *                      [PySubStream 123 bytes]
 *                        [PyList 1 items]
 *                          [PyTuple 2 items]
 *                            [PyInt 12193]
 *                            [PyTuple 2 items]
 *                              [PyString "AddBalls2"]
 *                              [PyTuple 1 items]
 *                                [PyTuple 2 items]
 *                                  [Destiny Header]
 *                                    [PacketType: 1]
 *                                    [Stamp: 12193]
 *                                  [Ball]
 *                                    [Name: ]
 *                                    [FormationId: 255]
 *                                    [Header]
 *                                      [ItemId: 9000000000000038313]
 *                                      [Mode: Stop (2)]
 *                                      [Flags: 0 (0)]
 *                                      [Radius: 30000]
 *                                      [Location: (-29259571200, -487710720, 55060439040)]
 *                                    [ExtraHeader]
 *                                      [AllianceId: -1]
 *                                      [CorporationId: 98038978]
 *                                      [CloakMode: 0]
 *                                      [Harmonic: NaN]
 *                                      [Mass: 10000000000]
 *
 *                                  [PyList 1 items]
 *                                    [PyDict 3 kvp]
 *                                      [PyString "itemID"]
 *                                      [PyIntegerVar 9000000000000038313]
 *                                      [PyString "typeID"] (groupID 411)
 *                                      [PyInt 16103]                       <<<  POS tower force field?
 *                                      [PyString "ownerID"]
 *                                      [PyInt 98038978]
 *                [PyBool True]
 *                [PyList 0 items]
 *    [PyDict 1 kvp]
 *      [PyString "sn"]
 *      [PyIntegerVar 85]
 *
 *    [PyTuple 1 items]
 *      [PyTuple 2 items]
 *        [PyInt 0]
 *        [PySubStream 127 bytes]
 *          [PyTuple 2 items]
 *            [PyInt 0]
 *            [PyTuple 2 items]
 *              [PyInt 1]
 *              [PyTuple 2 items]
 *                [PyList 3 items]
 *                  [PyTuple 2 items]
 *                    [PyInt 12193]
 *                    [PyTuple 2 items]
 *                      [PyString "SetBallHarmonic"]
 *                      [PyTuple 5 items]
 *                        [PyIntegerVar 1002330621081]              <<  setting ship ID
 *                        [PyIntegerVar 8039077077960405911]        <<  allianceID
 *                        [PyInt 98038978]                          <<  corpID
 *                        [PyInt -1]
 *                        [PyInt 0]
 *                  [PyTuple 2 items]
 *                    [PyInt 12193]
 *                    [PyTuple 2 items]
 *                      [PyString "SetBallMassive"]
 *                      [PyTuple 2 items]
 *                        [PyIntegerVar 9000000000000038313]        <<  force field ID
 *                        [PyInt 1]
 *                  [PyTuple 2 items]
 *                    [PyInt 12193]
 *                    [PyTuple 2 items]
 *                      [PyString "SetBallHarmonic"]
 *                      [PyTuple 5 items]
 *                        [PyIntegerVar 9000000000000038313]        <<  force field ID
 *                        [PyIntegerVar 8039077077960405911]        <<  allianceID
 *                        [PyInt 98038978]                          <<  corpID
 *                        [PyInt -1]
 *                        [PyInt 1]
 *                [PyBool False]
 *    [PyDict 1 kvp]
 *      [PyString "sn"]
 *      [PyIntegerVar 86]
 *
 *
 *


                      [PyString "SetState"]
                      [PyTuple 1 items]
                        [PyObjectData Name: util.KeyVal]
                          [PyDict 10 kvp]
                            [PyString "stamp"]
                            [PyInt 4480]
                            [PyString "damageState"]
                            [PyDict 110 kvp]
                              [PyIntegerVar 1002330708702]
                              [PyList 3 items]
                                [PyTuple 3 items]
                                  [PyFloat 0.995216219742903]
                                  [PyFloat 40000000]
                                  [PyIntegerVar 129527416407184682]
                                [PyFloat 1]
                                [PyFloat 1]
                            [PyString "effectStates"]
                            [PyList 1 items]
                              [PyTuple 14 items]
                                [PyIntegerVar 1002330708702]
                                [PyIntegerVar 1002330708702]
                                [PyInt 32226]
                                [PyNone]
                                [PyNone]
                                [PyList 0 items]
                                [PyString "effects.StructureOnline"]
                                [PyBool False]
                                [PyInt 1]
                                [PyInt 1]
                                [PyInt -1]
                                [PyInt 0]
                                [PyIntegerVar 129527371609182416]
                                [PyNone]
                        */


    /*      GetHybridBridgeMenu
     *      GetAllianceBeaconSubMenu
     * lots of other bridge/beacon/fleet menus in
  * /eve/client/script/ui/services/menusvc.py
  */
/*
 * class BasicOrbital(spaceObject.PlayerOwnedStructure):
 *
 *    def Assemble(self):
 *        self.SetStaticRotation()
 *
 *    def OnSlimItemUpdated(self, slimItem):
 *        orbitalState = getattr(slimItem, 'orbitalState', None)
 *        orbitalTimestamp = getattr(slimItem, 'orbitalTimestamp', blue.os.GetSimTime())
 *        fxSequencer = sm.GetService('FxSequencer')
 *        if not hasattr(self, 'orbitalState'):
 *            if orbitalState in (entities.STATE_ANCHORING, entities.STATE_ANCHORED):
 *                uthread.pool('SpaceObject::BasicOrbital::OnSlimItemUpdated', fxSequencer.OnSpecialFX, slimItem.itemID, slimItem.itemID, None, None, None, [], 'effects.AnchorDrop', 0, 1, 0)
 *            elif orbitalState in (entities.STATE_IDLE, entities.STATE_OPERATING):
 *                uthread.pool('SpaceObject::BasicOrbital::OnSlimItemUpdated', fxSequencer.OnSpecialFX, slimItem.itemID, slimItem.itemID, None, None, None, [], 'effects.StructureOnlined', 0, 1, 0)
 *        else:
 *            if orbitalState == entities.STATE_ANCHORING and self.orbitalState == entities.STATE_UNANCHORED:
 *                uthread.pool('SpaceObject::BasicOrbital::OnSlimItemUpdated', fxSequencer.OnSpecialFX, slimItem.itemID, slimItem.itemID, None, None, None, [], 'effects.AnchorDrop', 0, 1, 0)
 *            if orbitalState == entities.STATE_ONLINING and self.orbitalState == entities.STATE_ANCHORED:
 *                uthread.pool('SpaceObject::BasicOrbital::OnSlimItemUpdated', fxSequencer.OnSpecialFX, slimItem.itemID, slimItem.itemID, None, None, None, [], 'effects.StructureOnline', 0, 1, 0)
 *            if orbitalState == entities.STATE_IDLE and self.orbitalState == entities.STATE_ONLINING:
 *                uthread.pool('SpaceObject::BasicOrbital::OnSlimItemUpdated', fxSequencer.OnSpecialFX, slimItem.itemID, slimItem.itemID, None, None, None, [], 'effects.StructureOnlined', 0, 1, 0)
 *            if orbitalState == entities.STATE_ANCHORED and self.orbitalState in (entities.STATE_OFFLINING, entities.STATE_IDLE, entities.STATE_OPERATING):
 *                uthread.pool('SpaceObject::BasicOrbital::OnSlimItemUpdated', fxSequencer.OnSpecialFX, slimItem.itemID, slimItem.itemID, None, None, None, [], 'effects.StructureOffline', 0, 1, 0)
 *            if orbitalState == entities.STATE_UNANCHORING and self.orbitalState == entities.STATE_ANCHORED:
 *                uthread.pool('SpaceObject::BasicOrbital::OnSlimItemUpdated', fxSequencer.OnSpecialFX, slimItem.itemID, slimItem.itemID, None, None, None, [], 'effects.AnchorLift', 0, 1, 0)
 *        self.orbitalState = orbitalState
 *        self.orbitalTimestamp = orbitalTimestamp
 *
 *    def IsAnchored(self):
 *        self.LogInfo('Anchor State = ', not self.isFree)
 *        return not self.isFree
 *
 *    def IsOnline(self):
 *        slimItem = sm.StartService('michelle').GetBallpark().GetInvItem(self.id)
 *        return slimItem.orbitalState is not None and slimItem.orbitalState in (entities.STATE_OPERATING, entities.STATE_IDLE)
 *
 */