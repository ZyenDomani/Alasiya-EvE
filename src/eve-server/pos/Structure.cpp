
/**
 * @name Structure.cpp
 *   Generic Base Class for POS items and entities.
 *
 * @Author:         Allan
 * @date:   unknown
 */

/*
 * POS__ERROR
 * POS__WARNING
 * POS__MESSAGE
 * POS__DUMP
 * POS__DEBUG
 * POS__DESTINY
 * POS__SLIMITEM
 * POS__TRACE
 */


#include "eve-server.h"

#include "Client.h"
#include "EntityList.h"
#include "EVEServerConfig.h"
#include "StaticDataMgr.h"
#include "manufacturing/Blueprint.h"
#include "planet/Moon.h"
#include "pos/Tower.h"
#include "pos/Structure.h"
#include "system/Container.h"
#include "system/Damage.h"
#include "system/SystemManager.h"

/*
 * Base Structure Item for all POS types
 */
StructureItem::StructureItem(uint32 _structureID, const ItemType &_itemType, const ItemData &_data)
: InventoryItem(_structureID, _itemType, _data)
{
    pInventory = new Inventory(InventoryItemRef(this));
    _log(ITEM__TRACE, "Created StructureItem for %s (%u).", itemName().c_str(), itemID());
    _log(POS__TRACE, "Created StructureItem for %s (%u).", itemName().c_str(), itemID());
}

StructureItem::~StructureItem()
{
    SafeDelete(pInventory);
}

StructureItemRef StructureItem::Load(uint32 structureID)
{
    return InventoryItem::Load<StructureItem>(structureID );
}

bool StructureItem::_Load()
{
    if (!pInventory->LoadContents())
        return false;

    return InventoryItem::_Load();
}

StructureItemRef StructureItem::Spawn(ItemData &data)
{
    uint32 structureID = InventoryItem::CreateItemID(data);
    if (structureID < 1)
        return StructureItemRef(nullptr);
    StructureItemRef sRef = StructureItem::Load(structureID );
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
    if (!sRef->HasAttribute(AttrDamage))                        sRef->SetAttribute(AttrDamage, 0.0f, false);
    if (!sRef->HasAttribute(AttrArmorDamage))                   sRef->SetAttribute(AttrArmorDamage, 0.0f, false);
    if (!sRef->HasAttribute(AttrArmorMaxDamageResonance))       sRef->SetAttribute(AttrArmorMaxDamageResonance, 1.0f, false);
    if (!sRef->HasAttribute(AttrShieldMaxDamageResonance))      sRef->SetAttribute(AttrShieldMaxDamageResonance, 1.0f, false);

    // Shield Resonance
    if (!sRef->HasAttribute(AttrShieldEmDamageResonance))       sRef->SetAttribute(AttrShieldEmDamageResonance, 1.0, false);
    if (!sRef->HasAttribute(AttrShieldExplosiveDamageResonance)) sRef->SetAttribute(AttrShieldExplosiveDamageResonance, 1.0, false);
    if (!sRef->HasAttribute(AttrShieldKineticDamageResonance))  sRef->SetAttribute(AttrShieldKineticDamageResonance, 1.0, false);
    if (!sRef->HasAttribute(AttrShieldThermalDamageResonance))  sRef->SetAttribute(AttrShieldThermalDamageResonance, 1.0, false);
    if (!sRef->HasAttribute(AttrArmorEmDamageResonance))        sRef->SetAttribute(AttrArmorEmDamageResonance, 1.0, false);
    if (!sRef->HasAttribute(AttrArmorExplosiveDamageResonance)) sRef->SetAttribute(AttrArmorExplosiveDamageResonance, 1.0, false);
    if (!sRef->HasAttribute(AttrArmorKineticDamageResonance))   sRef->SetAttribute(AttrArmorKineticDamageResonance, 1.0, false);
    if (!sRef->HasAttribute(AttrArmorThermalDamageResonance))   sRef->SetAttribute(AttrArmorThermalDamageResonance, 1.0, false);
    if (!sRef->HasAttribute(AttrEmDamageResonance))             sRef->SetAttribute(AttrEmDamageResonance, 1.0, false);
    if (!sRef->HasAttribute(AttrExplosiveDamageResonance))      sRef->SetAttribute(AttrExplosiveDamageResonance, 1.0, false);
    if (!sRef->HasAttribute(AttrKineticDamageResonance))        sRef->SetAttribute(AttrKineticDamageResonance, 1.0, false);
    if (!sRef->HasAttribute(AttrThermalDamageResonance))        sRef->SetAttribute(AttrThermalDamageResonance, 1.0, false);

    return sRef;
}

void StructureItem::Delete()
{
    // delete contents first
    pInventory->DeleteContents();

    InventoryItem::Delete();
}

PyObject *StructureItem::StructureGetInfo()
{
    if (!pInventory->LoadContents()) {
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


StructureSE::StructureSE(StructureItemRef structure, PyServiceMgr &services, SystemManager* system, const FactionData& data)
: ObjectSystemEntity(structure, services, system),
  m_moonSE(nullptr),
  m_towerSE(nullptr),
  m_procTimer(10000) // arbitrary default
{
    m_co = false;
    m_tcu = false;
    m_sbu = false;
    m_miner = false;
    m_tower = false;
    m_bridge = false;
    m_jammer = false;
    m_module = false;
    m_outpost = false;
    m_reactor = false;

    m_duration = 0;

    m_procTimer.Disable();
    m_procState = EVEPOS::ProcState::Invalid;

    /** @todo  this is direction from customs office to planet and set when co is created */
    m_rotation = NULL_ORIGIN;
    m_planetID = 0;
    m_delayTime = 0;

    m_warID = data.factionID;
    m_allyID = data.allianceID;
    m_corpID = data.corporationID;
    m_ownerID = data.ownerID;

    _log(SE__DEBUG, "Created StructureSE for item %s (%u).", structure->itemName().c_str(), structure->itemID());
}

void StructureSE::InitData(SystemBubble* pBubble) {
    // not sure if we need to reinit to 0...probably not
    m_data.use = 0;
    m_data.view = 0;
    m_data.take = 0;
    m_data.moonID = 0;
    m_data.towerID = 0;
    m_data.status = 0.0f;
    m_data.timestamp = 0;
    m_data.state = EVEPOS::StructureState::Unanchored;

    if (m_module) {
        bool found = false;
        // this item is a module.  get towerID and save
        std::vector<SystemEntity*> seVec;
        pBubble->GetEntities(seVec);
        for (auto cur : seVec) {
            if (cur->IsTowerSE()) {
                found = true;
                m_data.towerID = cur->GetID();
            }
            if (found)
                break;
        }
    }

    if (m_bridge) {
        /** @todo figure out how/where to store this data.  */
        EVEPOS::JumpBridgeData data;
        data.itemID = m_data.itemID;
        data.towerID = m_data.towerID;
        data.corpID = m_corpID;
        data.allyID = m_allyID;
        data.systemID = m_system->GetID();
        data.toItemID = 0;
        data.toTypeID = 0;
        data.toSystemID = 0;
        data.password = "";
        data.allowCorp = false;
        data.allowAlliance = false;
        m_db.SaveBridgeData(data);
    }

    m_moonSE = m_system->GetClosestMoonSE(GetPosition())->GetMoonSE();
    m_data.moonID = m_moonSE->GetID();
}

void StructureSE::Init(InventoryItemRef iRef, SystemBubble* pBubble)
{
    // init all data to 0 here.
    m_data.use = 0;
    m_data.view = 0;
    m_data.take = 0;
    m_data.moonID = 0;
    m_data.towerID = 0;
    m_data.status = 0.0f;
    m_data.timestamp = 0;
    m_data.itemID = iRef->itemID();
    m_data.state = EVEPOS::StructureState::Unanchored;
    iRef->SetFlag(flagStructureInactive);

    switch(iRef->groupID()) {
        case EVEDB::invGroups::Orbital_Infrastructure: {
            m_co = true;
            m_planetID = atoi(m_self->customInfo().c_str());
        } break;
        case EVEDB::invGroups::Sovereignty_Blockade_Units: {
            m_sbu = true;
        } break;
        case EVEDB::invGroups::Control_Tower: {
            m_tower = true;
            m_data.towerID = m_data.itemID;
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
        case EVEDB::invGroups::Silo:
        case EVEDB::invGroups::Mobile_Reactor: {
            m_module = true;
            m_reactor = true;
        } break;
        case EVEDB::invGroups::Moon_Mining: {
            m_miner = true;
            m_module = true;
            m_reactor = true;
        } break;
        default: {
            m_module = true;
        }
    }

    if (!m_db.GetBaseData(m_data)) {
        _log(POS__TRACE, "StructureSE %s(%u) has no saved data.  Initalizing default set.", iRef->itemName().c_str(), m_data.itemID);
        // invalid data....init to 0 as this will only hit for currently-launching items (or errors)
        if (pBubble != nullptr)
            InitData(pBubble);
        else   // make error here.  this should never hit.
            _log(POS__TRACE, "StructureSE %s(%u) called with null bubble.", iRef->itemName().c_str(), m_data.itemID);
        m_db.SaveBaseData(m_data);
    }

    if (m_data.moonID == 0) {
        // make error here.  this should never hit.
        _log(POS__TRACE, "StructureSE %s(%u) has no moonID.", iRef->itemName().c_str(), m_data.itemID);
        iRef->Delete(); // really delete this?
        return;
    }

    if (m_moonSE == nullptr) {
        SystemEntity* pSE = m_system->GetSE(m_data.moonID);
        if (pSE == nullptr) {
            iRef->Delete(); // really delete this?
            return;
        }
        m_moonSE = pSE->GetMoonSE();
    }

    if (m_co or m_miner) {
        GVector dir(m_self->position(), m_moonSE->GetPosition());
        dir.normalize();
        m_rotation = dir;
    }

    // all POS have 1h duration
    if (m_tower) {
        m_duration = m_self->GetAttribute(AttrPosControlTowerPeriod).get_int();
    } else if (m_module) {
        SystemEntity* pSE = m_system->GetSE(m_data.towerID);
        if (pSE == nullptr) {
            iRef->Delete(); // really delete this?
            return;
        }
        m_towerSE = pSE->GetTowerSE();
        m_towerSE->AddModule(this);
        m_duration = 3600000;
    }

    /** @todo check for timestamp here and see if any processes are running.
     * if so, set varaibles accordingly and continue.
     */
    if (m_data.timestamp > 0) {
        // do something constructive here.
    }

    if (m_data.state > EVEPOS::StructureState::Anchored)
        m_self->SetFlag(flagStructureActive);
}

void StructureSE::Process() {
    /* called by EntityList::Process on every loop */
    /*  Enable base call to Process Targeting and Movement  */
    SystemEntity::Process();

    if (m_procTimer.Check(false)) {
    	m_procTimer.Disable();
        m_delayTime = 0;

        _log(POS__DEBUG, "Module %s(%u) Processing State '%s'", GetName(), m_data.itemID, GetProcStateName(m_procState).c_str());

        using namespace EVEPOS;
        switch (m_procState) {
            case ProcState::Unanchoring: {
                SendSlimUpdate();
                /** @todo  change SendSpecialEffect to SendSpecialEffect10 */
                m_destiny->SendSpecialEffect(m_data.itemID,m_data.itemID,m_self->typeID(),0,0,"effects.AnchorLift",0,0,0,-1,0);
                m_db.UpdateBaseData(m_data);
            } break;
            case ProcState::Anchoring: {
                SendSlimUpdate();
                m_destiny->SendSpecialEffect(m_data.itemID,m_data.itemID,m_self->typeID(),0,0,"effects.AnchorDrop",0,0,0,-1,0);
                if (m_tower)
                    m_moonSE->SetTower(this);

                m_db.UpdateBaseData(m_data);
            } break;
            case ProcState::Offlining: {
                m_data.state = StructureState::Anchored;
                SetOffline();
                SendSlimUpdate();
                m_db.UpdateBaseData(m_data);
            } break;
            case ProcState::Onlining: {
                m_data.state = StructureState::Online;
                SetOnline();
                SendSlimUpdate();
                m_db.UpdateBaseData(m_data);
            } break;
            case ProcState::Online: {
                Online();
            } break;
            case ProcState::Operating: {
                // this is virtual, overridden in specific class' for their needs
                // take resources or whatever needs to be done
                Operating();
            } break;
            // those below are not coded yet
            case ProcState::Reinforcing:
            case ProcState::SheildReinforcing:
            case ProcState::ArmorReinforcing: {
                m_self->SetFlag(flagStructureInactive);
                m_data.state = StructureState::Invulnerable;
                //m_data.state = StructureState::Reinforced;
                m_db.UpdateBaseData(m_data);
                // set timer for time to come out of reinforced
            } break;
            default:
            case ProcState::Invalid: {
                _log(POS__WARNING, "Module %s(%u) Processing State '%s'", GetName(), m_data.itemID, GetProcStateName(m_procState).c_str());
            } break;
        }

        // this will need more work
        m_data.timestamp = 0;

        if (m_procState < ProcState::Online)
            m_procState = ProcState::Invalid;
    }
}
/*{'messageKey': 'CantObjectInsideForceField', 'dataID': 17885214, 'suppressable': False, 'bodyID': 260153, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 399}
 * {'messageKey': 'CantOnlineAnotherClaimMarkerOnlining', 'dataID': 17876439, 'suppressable': False, 'bodyID': 256876, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3091}
 * {'messageKey': 'CantOnlineDamagedGoods', 'dataID': 17885104, 'suppressable': False, 'bodyID': 260116, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 400}
 * {'messageKey': 'CantOnlineDisruptorAnchored', 'dataID': 17877056, 'suppressable': False, 'bodyID': 257107, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3116}
 * {'messageKey': 'CantOnlineDisruptorNotClaimed', 'dataID': 17877530, 'suppressable': False, 'bodyID': 257286, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2931}
 * {'messageKey': 'CantOnlineDisruptorOutpostProtecting', 'dataID': 17876457, 'suppressable': False, 'bodyID': 256882, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3106}
 * {'messageKey': 'CantOnlineDisruptorsOnline', 'dataID': 17877687, 'suppressable': False, 'bodyID': 257346, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2932}
 * {'messageKey': 'CantOnlineInfrastructureDontOwnFlag', 'dataID': 17876442, 'suppressable': False, 'bodyID': 256877, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3153}
 * {'messageKey': 'CantOnlineInfrastructureHubAlreadyInSystem', 'dataID': 17876267, 'suppressable': False, 'bodyID': 256810, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3056}
 * {'messageKey': 'CantOnlineInfrastructureHubNoSovereignty', 'dataID': 17876796, 'suppressable': False, 'bodyID': 257014, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3125}
 * {'messageKey': 'CantOnlineInfrastructurePresent', 'dataID': 17877059, 'suppressable': False, 'bodyID': 257108, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3115}
 * {'messageKey': 'CantOnlineMissingUpgrade', 'dataID': 17876891, 'suppressable': False, 'bodyID': 257047, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3080}
 * {'messageKey': 'CantOnlineNoInfrastructureHub', 'dataID': 17876280, 'suppressable': False, 'bodyID': 256815, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3078}
 * {'messageKey': 'CantOnlineNotLinked', 'dataID': 17879371, 'suppressable': False, 'bodyID': 257982, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2294}
 * {'messageKey': 'CantOnlineNotSovereign', 'dataID': 17876923, 'suppressable': False, 'bodyID': 257058, 'messageType': 'hint', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3144}
 * {'messageKey': 'CantOnlineRequireTower', 'dataID': 17885220, 'suppressable': False, 'bodyID': 260155, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 401}
 * {'messageKey': 'CantOnlineSovInWormhole', 'dataID': 17877461, 'suppressable': False, 'bodyID': 257260, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2935}
 * {'messageKey': 'CantOnlineSovereigntyAlreadyClaimed', 'dataID': 17876445, 'suppressable': False, 'bodyID': 256878, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3044}
 * {'messageKey': 'CantOnlineStructureGroupLimit', 'dataID': 17879595, 'suppressable': False, 'bodyID': 258065, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2407}
 * {'messageKey': 'CantOnlineStructureOwnerBad', 'dataID': 17885124, 'suppressable': False, 'bodyID': 260123, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 402}
 * {'messageKey': 'CantOnlineStructureRequiresSovereigntyLevel', 'dataID': 17879395, 'suppressable': False, 'bodyID': 257991, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2254}
 * {'messageKey': 'CantOnlineTowerLacksCpuResources', 'dataID': 17885223, 'suppressable': False, 'bodyID': 260156, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 403}
 * {'messageKey': 'CantOnlineTowerLacksPowerResources', 'dataID': 17885226, 'suppressable': False, 'bodyID': 260157, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 404}
 * {'messageKey': 'CantOnlineTowerLacksResources', 'dataID': 17885363, 'suppressable': False, 'bodyID': 260207, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 405}
 * {'messageKey': 'CantOnlineWithinGlobalDisruptor', 'dataID': 17876951, 'suppressable': False, 'bodyID': 257068, 'messageType': 'hint', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3180}
 */
void StructureSE::SetAnchor(Client* pClient, GPoint& pos)
{
    if (m_data.state > EVEPOS::StructureState::Unanchored) {
        pClient->SendErrorMsg("The %s is already anchored", m_self->itemName().c_str());
        return;  // make error here?
    }

    /* returns SetBallPosition for towers.
     *    ct will anchor in the middle of the grid that you warp-in to....supposed to, but i dont know how yet.
     */

    /** @todo  check for other modules changing state...only allow one at a time */

    if (IsTowerSE()) {
        // hack for warping to moons
        // this puts ship at Az: 0.785332, Ele: 0.615505, angle: 1.5708
        //warpToPoint -= (radius * 1.25);

        uint32 dist = BUBBLE_RADIUS_METERS + 10000/*m_self->GetAttribute(AttrMoonAnchorDistance).get_int()*/;
        uint32 radius = m_moonSE->GetRadius();
        float rad = EvE::Trig::Deg2Rad(90);

        pos = m_moonSE->GetPosition();
        pos.x += radius + dist * std::sin(rad);
        pos.z += radius + dist * std::cos(rad);
        m_destiny->SetPosition(pos);
        sBubbleMgr.Add(this);

        if (is_log_enabled(POS__TRACE))
            _log(POS__TRACE, "StructureSE::Anchor() - TowerSE %s(%u) new position %.2f, %.2f, %.2f", GetName(), m_data.itemID, pos.x, pos.y, pos.z);
    } else {
        if (!m_moonSE->HasTower()) {
            pClient->SendErrorMsg("There is no tower anchored at this moon.  You cannot anchor any structure without a tower");
            return;
        }
        /*  few attribs to look into...
         * maxStructureDistance
         * posStructureControlDistanceMax   (maybe multiply this by tower size, or some fraction thereof?)
         */
        m_destiny->SetPosition(pos);
    }

    m_self->SaveItem();

    m_procState = EVEPOS::ProcState::Anchoring;
    m_data.state = EVEPOS::StructureState::Anchored;
    m_delayTime = m_self->GetAttribute(AttrAnchoringDelay).get_int();
    m_procTimer.SetTimer(m_delayTime);
    m_data.timestamp = GetFileTimeNow();

    SendSlimUpdate();

    std::vector<PyTuple*> updates;
    SetBallFree sbf;
        sbf.entityID = m_self->itemID();
        sbf.is_free = false;
    updates.push_back(sbf.Encode());
    SetBallRadius sbr;
        sbr.entityID = m_self->itemID();
        sbr.radius = m_self->radius();
    updates.push_back(sbr.Encode());
    m_destiny->SendDestinyUpdate(updates); //consumed
    m_destiny->SendSpecialEffect(m_data.itemID,m_data.itemID,m_self->typeID(),0,0,"effects.AnchorDrop",0,1,1,-1,0);
}

void StructureSE::PullAnchor()
{
    if (m_data.state > EVEPOS::StructureState::Anchored)
        return;  // make error here?

        /** @todo  check for other modules changing state...only allow one at a time */

    /** @todo make sure there are NO modules within the SOI of tower if Unanchoring tower.  */
    //m_towerSE->GetSOI();

    m_procState = EVEPOS::ProcState::Unanchoring;
    m_data.state = EVEPOS::StructureState::Unanchored;
    m_delayTime = m_self->GetAttribute(AttrUnanchoringDelay).get_int();
    m_procTimer.SetTimer(m_delayTime);
    m_data.timestamp = GetFileTimeNow();

    SendSlimUpdate();

    /*
    if (m_tower)
        SendEffectUpdate(anchorLiftForStructures, true);
    else if (m_co)
        SendEffectUpdate(anchorLiftForOrbitals, true);
    else
        */
        m_destiny->SendSpecialEffect(m_data.itemID,m_data.itemID,m_self->typeID(),0,0,"effects.AnchorLift",0,1,1,-1,0);
}

/*
 * 556     anchoringDelay  1800000     NULL
 * 650     maxStructureDistance    50000   NULL
 * 676     unanchoringDelay    NULL    3600000
 * 677     onliningDelay   1800000     NULL
 * 680     shieldRadius    30000   NULL
 * 711     moonAnchorDistance  100000  NULL
 * 1214    posStructureControlDistanceMax  NULL    15000
 */

void StructureSE::Activate(int32 effectID)
{
    if (m_data.state > EVEPOS::StructureState::Anchored)
        return;  // make error here?

        /** @todo  check for other modules changing state...only allow one at a time */

    // check effectID, check current state, check current timer, set new state, update timer

    if (m_tower) {
        // if tower, check fuel quantity for onlining
        //  if qty sufficient, begin online proc and send OnSlimItemChange and OnSpecialFX

    } else if (m_module) {
        if (m_towerSE == nullptr) {
            _log(POS__ERROR, "POS Module %s(%u) has no TowerSE for tower %u", m_self->itemName().c_str(), m_data.itemID, m_data.towerID);
            return;
        }
        if (!m_towerSE->HasCPU(m_self->GetAttribute(AttrCpu).get_float())) {
            // throwing an error negates further processing
            float total = m_towerSE->GetSelf()->GetAttribute(AttrCpuOutput).get_float();
            float remaining = total - m_towerSE->GetCPULoad();
            std::map<std::string, PyRep *> args;
            args["total"] = new PyFloat(total);
            args["require"] = new PyFloat(m_self->GetAttribute(AttrCpu).get_float());
            args["remaining"] = new PyFloat(remaining);
            args["moduleType"] = new PyInt(m_self->typeID());
            throw PyException( MakeUserError("NotEnoughCpu", args));
        }
        if (!m_towerSE->HasPG(m_self->GetAttribute(AttrPower).get_float())) {
            // throwing an error negates further processing
            float total = m_towerSE->GetSelf()->GetAttribute(AttrPowerOutput).get_float();
            float remaining = total - m_towerSE->GetPGLoad();
            std::map<std::string, PyRep *> args;
            args["total"] = new PyFloat(total);
            args["require"] = new PyFloat(m_self->GetAttribute(AttrPower).get_float());
            args["remaining"] = new PyFloat(remaining);
            args["moduleType"] = new PyInt(m_self->typeID());
            throw PyException( MakeUserError("NotEnoughPower", args));
        }
    } else {
        ; // check for things that DONT use a tower.  not sure if we need anymore checks here.  yes....all sov structures will need checks for activation
    }
    m_data.state = EVEPOS::StructureState::Onlining;
    m_procState = EVEPOS::ProcState::Onlining;
    m_delayTime = m_self->GetAttribute(AttrOnliningDelay).get_int();
    m_procTimer.SetTimer(m_delayTime);
    m_data.timestamp = GetFileTimeNow();

    SendSlimUpdate();
    m_destiny->SendSpecialEffect(m_data.itemID,m_data.itemID,m_self->typeID(),0,0,"effects.StructureOnline",0,1,1,-1,0);

    if (m_module)
        m_towerSE->OnlineModule(this);

    //m_db.UpdateBaseData(m_data);
}

void StructureSE::Deactivate(int32 effectID)
{
    m_procState = EVEPOS::ProcState::Offlining;
    m_delayTime = 500 /*m_self->GetAttribute(AttrOnliningDelay).get_int()*/;
    m_procTimer.SetTimer(m_delayTime);
    m_data.timestamp = GetFileTimeNow();

    SendSlimUpdate();
    //m_destiny->SendSpecialEffect(m_data.itemID,m_data.itemID,m_self->typeID(),0,0,"effects.StructureOnline",0,0,0,-1,0);
}

void StructureSE::SetOnline()
{
    // this state is persistant until out of resources or changed
    m_self->SetFlag(flagStructureActive);
    m_procState = EVEPOS::ProcState::Online;
    m_data.state = EVEPOS::StructureState::Online;

    SetTimer(m_duration);
    m_db.UpdateBaseData(m_data);
}

void StructureSE::SetOffline()
{
    m_self->SetFlag(flagStructureInactive);
    if (m_module)
        m_towerSE->OfflineModule(this);
}

void StructureSE::Online()
{
    // structure online, but not operating
    // take resources or whatever needs to be done

    if (!m_tower) {
        // do module shit here....tower online methods handled in tower class
    }

    SetTimer(m_duration);
}

void StructureSE::SetOperating()
{
    // this state is persistant until out of resources or changed
    m_procState = EVEPOS::ProcState::Operating;
    m_data.state = EVEPOS::StructureState::Operating;
    m_data.timestamp = GetFileTimeNow();

    SetTimer(m_duration);

    SendSlimUpdate();

    m_db.UpdateBaseData(m_data);
}

void StructureSE::Operating()
{
    // structure operating
    // take resources, move items, process reactions or whatever needs to be done (follow PI proc code)

    if (!m_tower) {
        // do module shit here....tower operating methods handled in tower class
    }


    SetTimer(m_duration);
}

void StructureSE::SetUsageFlags(int8 view/*0*/, int8 take/*0*/, int8 use/*0*/)
{
    m_data.use = use;
    m_data.view = view;
    m_data.take = take;
    m_db.UpdateBaseData(m_data);
}

void StructureSE::SendSlimUpdate()
{
    PyDict *slim = new PyDict();
        slim->SetItemString("name",                     new PyString(m_self->itemName()));
        slim->SetItemString("itemID",                   new PyLong(m_data.itemID));
        slim->SetItemString("typeID",                   new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",                  new PyInt(m_ownerID));
        slim->SetItemString("corpID",                   new PyInt(m_corpID));
        slim->SetItemString("allianceID",               new PyInt(m_allyID));
        slim->SetItemString("warFactionID",             new PyInt(m_warID));
        slim->SetItemString("posTimestamp",             new PyLong(m_data.timestamp));
        slim->SetItemString("posState",                 new PyInt(m_data.state));
        slim->SetItemString("incapacitated",            new PyInt(0));
        slim->SetItemString("posDelayTime",             new PyInt(m_delayTime));
    PyTuple* shipData = new PyTuple(2);
        shipData->SetItem(0, new PyLong(m_data.itemID));
        shipData->SetItem(1, new PyObject( "foo.SlimItem", slim));
    PyTuple* sItem = new PyTuple(2);
        sItem->SetItem(0, new PyString("OnSlimItemChange"));
        sItem->SetItem(1, shipData);
    m_destiny->SendSingleDestinyUpdate(&sItem);   // consumed
}

void StructureSE::SendEffectUpdate(int16 effectID, bool active)
{
    GodmaEnvironment ge;
        ge.selfID = m_data.itemID;
        ge.charID = m_ownerID;
        ge.shipID = m_data.itemID;
        ge.targetID = 0;
        ge.other = PyStatic.NewNone();
        ge.area = new PyList();
        ge.effectID = effectID;
    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = effectID;
        shipEff.timeNow = GetFileTimeNow();
        shipEff.start = (active ? 1 : 0);
        shipEff.active = (active ? 1 : 0);
        shipEff.environment = ge.Encode();
        shipEff.startTime = shipEff.timeNow;    // do we need to adjust this?
        shipEff.duration = (active ? 0 : -1);
        shipEff.repeat = (active ? 1 : 0);
        shipEff.error = PyStatic.NewNone();
    PyList* events = new PyList();
        events->AddItem(shipEff.Encode());
    PyTuple* event = new PyTuple(1);
        event->SetItem(0, events);
    m_destiny->SendSingleDestinyEvent(&event);   // consumed
}


void StructureSE::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;
    //const uint16 miniballsCount = GetMiniBalls();
    BallHeader head;
        head.entityID = m_data.itemID;
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
    } else if (m_tower) {
        head.mode = DSTBALL_STOP;
        head.flags = (m_data.state < EVEPOS::StructureState::Anchored ? IsFree : 0)/*HasMiniBalls*/;
    } else {
        head.mode = DSTBALL_RIGID;
        //TODO check for miniballs and add here if found.
        head.flags = (m_data.state < EVEPOS::StructureState::Anchored ? IsFree : 0/*IsMassive*/) /*HasMiniBalls*/;
    }
    into.Append( head );

    /** @todo these may need more work....  */
    if (m_tcu or m_tower) {
        MassSector mass;
            mass.cloak = 0;
            mass.corporationID = m_corpID;
            mass.allianceID = (m_allyID > 0 ? m_allyID : -1);
            mass.harmonic = m_harmonic;
            mass.mass = m_self->type().mass();
        into.Append( mass );
    }

    if (m_data.state < EVEPOS::StructureState::Anchored) {
        DataSector data;
            data.inertia = 1;
            data.velocity_x = 0;
            data.velocity_y = 0;
            data.velocity_z = 0;
            data.maxVelocity = 1;
            data.speedfraction = 1;
        into.Append( data );
    }

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
    if (head.mode == DSTBALL_RIGID) {
        DSTBALL_RIGID_Struct main;
            main.formationID = 0xFF;
        into.Append( main );
    } else if (head.mode == DSTBALL_STOP) {
        DSTBALL_STOP_Struct main;
            main.formationID = 0xFF;
        into.Append( main );
    }

    _log(SE__DESTINY, "StructureSE::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
    _log(POS__DESTINY, "StructureSE::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

PyDict *StructureSE::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for StructureSE %u", m_data.itemID);
    _log(POS__SLIMITEM, "MakeSlimItem for StructureSE %u", m_data.itemID);
    /** @todo (Allan) *Timestamp will need to be set to time current state is started. */
    PyDict *slim = new PyDict();
        slim->SetItemString("name",                     new PyString(m_self->itemName()));
        slim->SetItemString("nameID",                   PyStatic.NewNone());
        slim->SetItemString("itemID",                   new PyLong(m_data.itemID));
        slim->SetItemString("typeID",                   new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",                  new PyInt(m_ownerID));  //1000148 for interbus customs office (to be done on creation)
        slim->SetItemString("corpID",                   new PyInt(m_corpID));  //1000148 for interbus customs office (to be done on creation)
        slim->SetItemString("allianceID",               new PyInt(m_allyID));
        slim->SetItemString("warFactionID",             new PyInt(m_warID));
        if (m_module or m_tower) {    // for control towers and structures
            slim->SetItemString("posTimestamp",         new PyLong(m_data.timestamp));
            slim->SetItemString("posState",             new PyInt(m_data.state));
            slim->SetItemString("incapacitated",        new PyInt((m_data.state == EVEPOS::StructureState::Incapacitated) ? 1 : 0));
            slim->SetItemString("posDelayTime",         new PyInt(m_delayTime));
        }
        if (m_outpost) {
            slim->SetItemString("startTimestamp",       new PyLong(m_data.timestamp));
            slim->SetItemString("structureState",       new PyInt(m_data.state));
            slim->SetItemString("posDelayTime",         new PyInt(m_delayTime));
        } else if (m_co) {
            slim->SetItemString("level",                new PyInt(1)); //{1-CUSTOMSOFFICE_SPACEPORT, 2-CUSTOMSOFFICE_SPACEELEVATOR}   this is for display model
            slim->SetItemString("orbitalTimestamp",     new PyLong(m_data.timestamp));
            slim->SetItemString("planetID",             new PyInt(m_planetID));  // planetID for this orbital
            slim->SetItemString("orbitalState",         new PyInt(m_data.state));   // this needs to be ORBITAL state...not structure state
            PyTuple* tuple = new PyTuple(3);
                tuple->SetItem(0,                       new PyFloat(m_rotation.x));
                tuple->SetItem(1,                       new PyFloat(m_rotation.y));
                tuple->SetItem(2,                       new PyFloat(m_rotation.z));
            slim->SetItemString("dunRotation", tuple);  // direction to planet
            //  dunno what these are...
            slim->SetItemString("orbitalHackerProgress", PyStatic.NewNone());
            slim->SetItemString("orbitalHackerID",      PyStatic.NewNone());
        } else if (m_tcu) {
            slim->SetItemString("posDelayTime",         new PyInt(m_delayTime));
        } else if (m_miner) {
            PyTuple* tuple = new PyTuple(3);
                tuple->SetItem(0,                       new PyFloat(m_rotation.x));
                tuple->SetItem(1,                       new PyFloat(m_rotation.y));
                tuple->SetItem(2,                       new PyFloat(m_rotation.z));
            slim->SetItemString("dunRotation", tuple);  // direction to moon
            slim->SetItemString("controlTowerID",       new PyLong(m_data.towerID));
        } else if (m_module) {
            slim->SetItemString("controlTowerID",       new PyLong(m_data.towerID));
        }

    if (is_log_enabled(POS__SLIMITEM)) {
        _log( POS__SLIMITEM, "StructureSE::MakeSlimItem() - %s(%u)", GetName(), m_data.itemID);
        slim->Dump(POS__SLIMITEM, "     ");
    }
    return slim;
}

/*  Log events...not sure what this is for yet.
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

std::string StructureSE::GetProcStateName(int8 state)
{
    using namespace EVEPOS;
    switch(state) {
        case ProcState::Invalid:            return "Invalid";
        case ProcState::Unanchoring:        return "Unanchoring";
        case ProcState::Anchoring:          return "Anchoring";
        case ProcState::Offlining:          return "Offlining";
        case ProcState::Onlining:           return "Onlining";
        case ProcState::Online:             return "Online";
        case ProcState::Operating:          return "Operating";
        case ProcState::Reinforcing:        return "Reinforcing";
        case ProcState::SheildReinforcing:  return "SheildReinforcing";
        case ProcState::ArmorReinforcing:   return "ArmorReinforcing";
        default:                                    return "Bad State";
    }
}

void StructureSE::GetEffectState(PyList& into) {
	// this is for sending structure state info in destiny state data
    if ((m_data.state != EVEPOS::StructureState::Online)
    and (m_data.state != EVEPOS::StructureState::Operating))
        return;

    std::vector<int32, std::allocator<int32> > area;
     OnSpecialFX13 effect;
        if (m_module)
            effect.entityID = m_data.towerID;            /* control tower id */
        else
            effect.entityID = m_data.itemID;     /* control tower id */
        if (!m_co) {
            effect.moduleID = m_data.itemID;         /* structure/module id as part of above tower system */
            effect.moduleTypeID = m_self->typeID();
            effect.targetID = 0;
            effect.chargeTypeID = 0;
            effect.duration_ms = -1;
        }
        effect.area = area;
        effect.guid = "effects.StructureOnline"; // this is sent in destiny::SetState.  check for actual effect of this pos
        effect.isOffensive = false;                     /** @todo (Allan) this should be boolean */
        effect.start = 1;
        effect.active = 1;
        effect.repeat = 0;
        effect.startTime = m_data.timestamp;             /* time this effect started */
    into.AddItem(effect.Encode());
}

void StructureSE::Killed(Damage &fatal_blow) {
    if ((m_bubble == nullptr) or (m_destiny == nullptr))
        return; // make error here?

    m_destiny->Halt();
    m_destiny->SendTerminalExplosion(m_self->itemID(), m_bubble->GetID());

    uint32 killerID = 0;
    Client* pClient(nullptr);
    SystemEntity* killer = fatal_blow.srcSE;

    if (killer->HasPilot()) {
        pClient = killer->GetPilot();
        killerID = pClient->GetCharacterID();
    } else if (killer->IsDroneSE()) {
        pClient = sEntityList.FindClientByCharID( killer->GetSelf()->ownerID() );
        if (pClient == nullptr) {
            sLog.Error("StructureSE::Killed()", "killer == IsDrone and pPlayer == nullptr");
        } else
            killerID = pClient->GetCharacterID();
    } else
        killerID = killer->GetID();

    std::stringstream blob;
    blob << "<items>";
    std::vector<InventoryItemRef> survivedItems;
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
                // move item to vector for insertion into wreck later on
                survivedItems.push_back(cur.second);
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
        data.killTime = GetFileTimeNow();
        data.moonID = m_moonSE->GetID();    /* denotes moonID for POS/Structure kills */

    ServiceDB::SaveKillOrLoss(data);

    uint32 locationID = GetLocationID();
    //  log faction kill in dynamic data   -allan
    MapDB::AddKillToDynamicData(locationID);
    MapDB::AddFactionKillToDynamicData(locationID);

    if (pClient != nullptr) {
        //award kill bounty.
        //AwardBounty( pClient );
        if (m_system->GetSystemSecurityRating() > 0)
            AwardSecurityStatus(m_self, pClient->GetChar().get());  // this awards secStatusChange for npcs in empire space
    }

    GPoint deadPOSPosition = m_destiny->GetPosition();
    uint32 wreckTypeID = sDataMgr.GetWreckID(m_self->typeID());
    if (!IsWreckTypeID(wreckTypeID)) {
        sLog.Error("StructureSE::Killed()", "Could not get wreckType for %s of type %u", m_self->itemName().c_str(), m_self->typeID());
        // default to generic frigate wreck till i get better checks and/or complete wreck data
        wreckTypeID = 26557;
    }

    std::string wreck_name = m_self->itemName();
    wreck_name += " Wreck";
    const char* faction = itoa(m_allyID);
    ItemData wreckItemData(wreckTypeID, killerID, locationID, flagAutoFit, wreck_name.c_str(), deadPOSPosition, faction);
    WreckContainerRef wreckItemRef = sItemFactory.SpawnWreckContainer( wreckItemData );
    if (wreckItemRef.get() == nullptr) {
        sLog.Error("StructureSE::Killed()", "Creating Wreck Item Failed for %s of type %u", wreck_name.c_str(), wreckTypeID);
        return;
    }

    DropLoot(wreckItemRef, m_self->groupID(), killerID);

    if (survivedItems.size())
        for (auto cur: survivedItems)
            cur->Move(wreckItemRef->itemID(), flagAutoFit); // populate wreck with items that survived

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
        wreckItemRef->Delete();
        return;
    }

    _log(PHYSICS__TRACE, "StructureSE::Killed() - Wreck %s(%u) Item Position: %.2f,%.2f,%.2f.  Destiny Position: %.2f,%.2f,%.2f.", \
            GetName(), m_data.itemID, x(), y(), z(), deadPOSPosition.x, deadPOSPosition.y, deadPOSPosition.z);
}


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
