
/**
 * @name Structure.cpp
 *   Generic Base Class for POS items and entities.
 *
 * @Author:         Allan
 * @date:   unknown
 */

#include "eve-server.h"

#include <PyServiceMgr.h>
#include "Client.h"
#include "EntityList.h"
#include "EVEServerConfig.h"
#include "manufacturing/Blueprint.h"
#include "pos/POS.h"
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
: ObjectSystemEntity(structure, services, system),
  m_moonSE(nullptr),
  m_procTimer(10000) // arbitrary default
{
    m_co = false;
    m_tcu = false;
    m_sbu = false;
    m_bridge = false;
    m_jammer = false;
    m_module = false;
    m_outpost = false;

    m_towerID = 0;

    m_procTimer.Disable();

    m_state = EVEPOS::StructureState::Unanchored;
    /** @todo  this is direction from customs office to planet and set when co is created */
    m_rotation = NULL_ORIGIN;
    m_delayTime = 0;
    m_timestamp = 0;

    m_warID = data.factionID;
    m_allyID = data.allianceID;
    m_corpID = data.corporationID;
    m_ownerID = data.ownerID;

    _log(SE__DEBUG, "Created StructureSE for item %s (%u).", structure->itemName().c_str(), structure->itemID());
}

void StructureSE::InitData() {
    EVEPOS::SaveData data;
        data.itemID = GetID();
        data.timestamp = 0;
        data.harmonic = m_harmonic;     // set during base SE creation
        data.state = m_state;
        data.rotation = m_rotation;
        data.planetID = 0;
        data.status = 0.0f;
        data.standing = 0.0f;
        data.standingOwnerID = 0;
        data.corpWar = false;
        data.allowCorp = false;
        data.statusDrop = false;
        data.allowAlliance = false;
        data.showInCalendar = false;
        data.sendFuelNotifications = false;
    if (m_module) {
        bool found = false;
        // this item is a module.  get towerID and save
        std::vector<SystemEntity*> seVec;
        if (m_bubble == nullptr)
            if (m_system != nullptr)
                m_system->AddEntity(this);
            else
                ; // make error here for no SystemManager?
        m_bubble->GetEntities(seVec);
        for (auto cur : seVec) {
            if (cur->IsTowerSE()) {
                found = true;
                m_towerID = cur->GetID();
                // do we wanna save towerSE for each module?
            }
            if (found)
                break;
        }
    } else
        data.towerID = m_towerID;

    m_db.SavePOSData(data);
}

void StructureSE::Init(StructureItemRef structure)
{
    EVEPOS::SaveData data;
    data.itemID = structure->itemID();
    if (!m_db.GetPOSData(data)) {
        // invalid data....init to 0 as this will only hit for currently-launching items (or errors)
        InitData();
        data.towerID = 0;
    } else
        m_harmonic = data.harmonic;

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
        case EVEDB::invGroups::Jump_Portal_Array: {
            m_bridge = true;
            m_module = true;
        } break;
        case EVEDB::invGroups::Cynosural_System_Jammer: {
            /** @todo (Allan) do we need anything else here?  check for and set system-wide cyno jammer?
             *    as we're nowhere even close to needing/using cyno, this can wait
             */
            m_jammer = true;
            m_module = true;
        } break;
        default: {
            m_module = true;
        }
    }

    if (m_module)
        m_towerID = data.towerID;

    if ((m_state == EVEPOS::StructureState::Online) or (m_state == EVEPOS::StructureState::Operating))
        m_self->SetFlag(flagStructureActive, false);

}

void StructureSE::Process() {
    /* called by EntityList::Process on every loop */
    /*  Enable base call to Process Targeting and Movement  */
    SystemEntity::Process();

    using namespace EVEPOS;
    if (m_procTimer.Check(false)) {
    	m_procTimer.Disable();
        m_timestamp = 0;  // time state ends
        m_delayTime = 0;

        switch (m_state) {
            case StructureState::Unanchored: {
                m_state = StructureState::Anchored;
            } break;

            // those below are not coded yet
            case StructureState::Onlining: {
                m_self->SetFlag(flagStructureActive);
            } break;

            case StructureState::Anchored: {
                ; // unanchor
            } break;

            case StructureState::Operating: {
                // take resources or whatever needs to be done
            } break;

            case StructureState::SheildReinforced:
            case StructureState::ArmorReinforced: {
                m_delayTime = 0;
            } break;

            case StructureState::Reinforced: {
                m_self->SetFlag(flagStructureInactive, false);
            } break;
        }
    }
}

/*  for updating structure data
 *
 * EVEPOS::SaveData data;
 * m_db.UpdatePOSData(data);
 */

void StructureSE::Anchor(GPoint& pos)
{
    /* returns SetBallPosition for towers.
     *    ct will anchor in the middle of the grid that you warp-in to.
     */

    if (IsTowerSE() or pos.isZero()) {
        // set position away from current position and send new position to client
        uint32 dist = MakeRandomInt(200000, 250000);
        uint32 radius = GetPosition().distance(m_moonSE->GetPosition());
        float rad = radius / dist;

        GPoint newPos(GetPosition());
        newPos.x += radius * cos(rad);
        newPos.z += radius * sin(rad);

        m_destiny->SetPosition(newPos);
    }

    m_procTimer.SetTimer(m_self->GetAttribute(AttrAnchoringDelay).get_int());
    m_timestamp = Win32TimeNow();

}

void StructureSE::Activate(int32 effectID)
{
    // check effectID, check current state, check current timer, set new state, update timer

    /** @note  to change tower timers, the m_timestamp will have to be adjusted.
     *   the client calculates all pos timers internally
     */

    /** @todo somehow notify client with one of these effects:
     *  effectAnchorDrop = 649
     *  effectAnchorLift = 650
     *  effectAnchorDropForStructures = 1022
     *  effectAnchorLiftForStructures = 1023
     *
     ** @todo  many more effects to send for.....look into later.
     * effectOnlineForStructures = 901
     *
     ** @note  also note there are timers involved here...
     */

    // check fuel quantity for onlining
    //  if qty sufficient, set tower to online and send OnSlimItemChange and OnSpecialFX and

    m_self->SetFlag(flagStructureActive, false);
    m_state = EVEPOS::StructureState::Online;
}

void StructureSE::Deactivate(int32 effectID)
{
    // check effectID, check current state, check current timer, set new state, update timer

    m_self->SetFlag(flagStructureInactive, false);
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

    //if (m_tcu or m_pos) {
        MassSector mass;
            mass.cloak = 0;
            mass.corporationID = m_corpID;
            mass.allianceID = m_allyID;
            mass.harmonic = m_harmonic;
            mass.mass = m_self->type().mass();
        into.Append( mass );
    //}

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
    DSTBALL_RIGID_Struct main;
        main.formationID = 0xFF;
    into.Append( main );

    _log(SE__DESTINY, "StructureSE::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
    _log(POS__DESTINY, "StructureSE::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
/*
    if (is_log_enabled(POS__DEBUG)) {
        _log( POS__DEBUG, "StructureSE::EncodeDestiny()", "%s(%u)", GetName(), GetID());
        uint8* data(into.Get<uint8*>(0));
        Destiny::DumpUpdate( POS__DEBUG, data, (uint32)into.size());    <<-- this doesnt work right....dunno why
    } */
}

PyDict *StructureSE::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for StructureSE %u", m_self->itemID());
    _log(POS__SLIMITEM, "MakeSlimItem for StructureSE %u", m_self->itemID());
    /** @todo (Allan) *Timestamp will need to be set to time current state is started. */
    PyDict *slim = new PyDict();
        slim->SetItemString("name",                     new PyString(m_self->itemName()));
        slim->SetItemString("nameID",                   new PyNone());
        slim->SetItemString("itemID",                   new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",                   new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",                  new PyInt(m_self->ownerID()));  //1000148 for interbus customs office (to be done on creation)
        slim->SetItemString("corpID",                   new PyInt(m_corpID));  //1000148 for interbus customs office (to be done on creation)
        slim->SetItemString("allianceID",               new PyInt(m_allyID));
        slim->SetItemString("warFactionID",             new PyInt(m_warID));
        if (m_module) {    // for control towers and structures
            slim->SetItemString("posTimestamp",         new PyLong((m_timestamp > 0) ? m_timestamp : 0));
            slim->SetItemString("posState",             new PyInt(GetStructureState()));
            slim->SetItemString("incapacitated",        new PyInt((m_state == EVEPOS::StructureState::Incapacitated) ? 1 : 0));
            // this is time shown in structure status (time left until current state completes)
            if (m_delayTime)
                slim->SetItemString("posDelayTime",         new PyInt(m_delayTime));
        }
        if (m_outpost) {
            slim->SetItemString("startTimestamp",       new PyLong(m_timestamp));
            slim->SetItemString("structureState",       new PyInt(GetStructureState()));
            if (m_delayTime)
                slim->SetItemString("posDelayTime",         new PyInt(m_delayTime));
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
        } else if ((m_tcu) and (m_delayTime)) {
            slim->SetItemString("posDelayTime",         new PyInt(m_delayTime));
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


PyTuple *StructureSE::GetEffectState() {
	// this is for sending structure state info in destiny state data
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
        effect.guid = "effects.StructureOnline"; // this is sent in destiny::SetState.  check for actual effect of this pos
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
            data.moonID = m_moonSE->GetID();    /* denotes moonID for POS/Structure kills */

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
