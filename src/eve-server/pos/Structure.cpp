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

#include "inventory/AttributeEnum.h"
#include "pos/Structure.h"
#include "ship/DestinyManager.h"

/*
 * Structure
 */
StructureItem::StructureItem(
    ItemFactory &_factory,
    uint32 _structureID,
    // InventoryItem stuff:
    const ItemType &_itemType,
    const ItemData &_data)
: InventoryItem(_factory, _structureID, _itemType, _data)
{
    m_inventory = new Inventory(InventoryItemRef(this));
    _log(ITEM__TRACE, "Created StructureItem for %s (%u).", itemName().c_str(), itemID());
}

StructureItem::~StructureItem()
{
    SafeDelete(m_inventory);
}

StructureItemRef StructureItem::Load(ItemFactory &factory, uint32 structureID) {
    return InventoryItem::Load<StructureItem>( factory, structureID );
}

bool StructureItem::_Load() {
    if( !m_inventory->LoadContents( &m_factory ) )
        return false;

    return InventoryItem::_Load();
}

template<class _Ty>
RefPtr<_Ty> StructureItem::_LoadStructure(ItemFactory &factory, uint32 structureID, const ItemType &itemType, const ItemData &data) {
    // we don't need any additional stuff
    return StructureItemRef( new StructureItem( factory, structureID, itemType, data ) );
}

StructureItemRef StructureItem::Spawn(ItemFactory &factory, ItemData &data) {
    uint32 structureID = StructureItem::CreateItemID( factory, data );
    if (!structureID)
        return StructureItemRef();
    return StructureItem::Load( factory, structureID );
}

uint32 StructureItem::CreateItemID(ItemFactory &factory, ItemData &data) {
    // make sure it's a Structure
    const ItemType *st = factory.GetType(data.typeID);
    if (!st) return 0;

    uint32 structureID = InventoryItem::CreateItemID(factory, data);
    if (!structureID) return 0;

    return structureID;
}

void StructureItem::Delete()
{
    // delete contents first
    m_inventory->DeleteContents( m_factory );

    InventoryItem::Delete();
}

void StructureItem::ValidateAddItem(EVEItemFlags flag, InventoryItemRef item) const {
    EvilNumber capacityUsed(0);
    std::vector<InventoryItemRef> items;
    m_inventory->FindByFlag(flag, items);
    for (auto cur : items)
        capacityUsed += cur->GetAttribute(AttrVolume);
    capacityUsed += item->GetAttribute(AttrVolume);

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
            if( capacityUsed > GetAttribute(AttrAmmoCapacity) )
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
    if( !m_inventory->LoadContents( &m_factory ) )
    {
        codelog( ITEM__ERROR, "%s (%u): Failed to load contents for Structure", itemName().c_str(), itemID() );
        return NULL;
    }

    Rsp_CommonGetInfo result;
    Rsp_CommonGetInfo_Entry entry;

    //first populate the Structure.
    if( !Populate( entry ) )
        return NULL;

    result.items[ itemID() ] = entry.Encode();

    return result.Encode();
}

void StructureItem::AddItem( InventoryItemRef item )
{
    m_inventory->AddItem( item );
}


/** @todo (Allan) this class needs more research to finish
 *
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
    */
StructureSE::StructureSE(StructureItemRef structure, PyServiceMgr &services, SystemManager* system)
: ObjectSystemEntity(structure, services, system)
{
    /** @todo (Allan) fix this later...used for shield passage */
    m_harmonic = 0;
    m_timestamp = Win32TimeNow() - Win32Time_Day;
    m_state = STRUCTURE_ONLINE;
    /** @todo (Allan) fix this */
    m_corpID = m_self->ownerID();

    Init(structure);
}

void StructureSE::Init(StructureItemRef structure)
{
    switch(structure->typeID()) {
        case EVEDB::invGroups::Territorial_Claim_Units: {
            m_tcu = true;
        } break;
        case EVEDB::invGroups::Control_Tower: {
            m_pos = true;
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
    m_towerID = 0;
}

void StructureSE::Process() {
    /* called by EntityList::Process on every loop */
    /* this is empty default call */
    SystemEntity::Process();
    /** @todo (Allan)  will need some form of AI to engage defensive modules if/when any structure is attacked */
}

void StructureSE::ProcessOther() {
    /* called by EntityList::Process on each tic */
    /* this is empty default call */
    //ObjectSystemEntity::ProcessOther();
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
        } else {
            head.mode = DSTBALL_RIGID;
            head.flags = IsMassive | IsInteractive;        //TODO check for miniballs and add here if found.
        }
    into.Append( head );

    DSTBALL_RIGID_Struct main;
        main.formationID = 0xFF;
    into.Append( main );

    if (m_tcu) {
        MassSector mass;
            mass.cloak = 0;
            mass.corporationID = GetCorporationID();
            mass.allianceID = GetAllianceID();
            mass.Harmonic = m_harmonic;
            mass.mass = m_self->type().mass();
        into.Append( mass );
    }

    /** @todo (Allan) fix this when POS system is more operational */
    /* TODO  query and configure miniballs for entity
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
    _log(COMMON__WARNING, "StructureSE::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

PyDict *StructureSE::MakeSlimItem() {
    _log(COMMON__WARNING, "MakeSlimItem for StructureSE %u", m_self->itemID());
    /** @todo (Allan) *Timestamp will need to be set to time current state is started. */
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",                   new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",                   new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",                  new PyInt(m_self->ownerID()));

        if (m_outpost) {
            slim->SetItemString("startTimestamp",       new PyLong(m_timestamp));
            slim->SetItemString("structureState",       new PyInt(GetStructureState()));
            slim->SetItemString("delayTime",            new PyInt(0));/** @todo (Allan) fix this later - dont know what it is */
            return slim;
        }

        slim->SetItemString("corpID",                   new PyInt(m_corpID));
        slim->SetItemString("allianceID",               new PyInt(0));/** @todo (Allan) fix this later */
        slim->SetItemString("warFactionID",             new PyInt(0));/** @todo (Allan) fix this later */
        slim->SetItemString("posTimestamp",             new PyLong(m_timestamp));
        slim->SetItemString("posState",                 new PyInt(GetStructureState()));
        slim->SetItemString("incapacitated",            new PyInt(0)); /** @todo (Allan) fix this later....check for offline/vulnerable states */

        if (m_tcu) {
            slim->SetItemString("posDelayTime",         new PyInt(0));/** @todo (Allan) fix this later - dont know what it is */
            return slim;
        } else if (m_module) {
            slim->SetItemString("controlTowerID",       new PyLong(m_towerID));
        }

        slim->SetItemString("name",                     new PyString(""));
        slim->SetItemString("nameID",                   new PyNone);

    return slim;
}

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
    /* see file:///home/allan/Desktop/cruc/entities/pos_packets/control_tower_packets for more info */
    std::vector<int32, std::allocator<int32> > area;
    DoDestiny_OnSpecialFX13 effect;
        if (m_module)
            effect.entityID = m_towerID;            /* control tower id */
        else
            effect.entityID = m_self->itemID();     /* control tower id */
        effect.moduleID = m_self->itemID();         /* structure/module id as part of above tower system */
        effect.moduleTypeID = m_self->typeID();
        effect.targetID = m_self->itemID();
        effect.otherTypeID = 0;
        effect.area = area;
        effect.effect_type = "effects.StructureOnline";
        effect.isOffensive = 0;                     /** @todo (Allan) this should be boolean */
        effect.start = 1;
        effect.active = 1;
        effect.duration_ms = -1;
        effect.repeat = 0;
        effect.startTime = m_timestamp;             /* time this effect started */
    PyTuple *update = effect.Encode();
    return update;
}
    /*
                          [PyTuple 2 items]                         <<<< in AddBalls
                            [PyInt 14492]
                            [PyTuple 2 items]
                              [PyString "OnSpecialFX"]
                              [PyTuple 14 items]
                                [PyIntegerVar 1006353458454]        <<<< 365    Gallente Control Tower Small
                                [PyIntegerVar 1006353458454]
                                [PyInt 20064]
                                [PyNone]
                                [PyNone]
                                [PyList 0 items]
                                [PyString "effects.StructureOnline"]
                                [PyBool False]
                                [PyInt 1]
                                [PyInt 1]
                                [PyInt -1]
                                [PyInt 0]
                                [PyIntegerVar 129813018310933856]
                                [PyNone]

                            [PyString "effectStates"]               <<<< in SetState
                            [PyList 1 items]
                              [PyTuple 14 items]
                                [PyIntegerVar 1002330708702]        <<<< Territorial Claim Unit
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
                                [PyIntegerVar 129514900819404614]
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