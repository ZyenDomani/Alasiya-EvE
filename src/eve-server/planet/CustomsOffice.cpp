
/**
 * @name CustomsOffice.h
 *   Class for Customs Offices.
 *
 * @Author:         Allan
 * @date:   14 July 2019
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


#include "Client.h"
#include "EntityList.h"
#include "EVEServerConfig.h"
#include "StaticDataMgr.h"
#include "manufacturing/Blueprint.h"
#include "planet/CustomsOffice.h"
#include "pos/Structure.h"
#include "system/Container.h"
#include "system/Damage.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"


CustomsSE::CustomsSE(StructureItemRef sRef, PyServiceMgr &services, SystemManager* system, const FactionData& data)
: ObjectSystemEntity(sRef, services, system),
m_system(system)
{
    m_warID = data.factionID;
    m_allyID = data.allianceID;
    m_corpID = data.corporationID;
    m_ownerID = data.ownerID;

    sRef->SetAttribute(AttrIsGlobal, 1, false);

    // zero-init data
    m_data = EVEPOS::CustomsData();

    m_data.planetID = atoi(m_self->customInfo().c_str());
    m_planetSE = m_system->GetPlanet(m_data.planetID);

    _log(SE__DEBUG, "Created CustomsSE for item %s (%u).", sRef->itemName().c_str(), sRef->itemID());
}

void CustomsSE::Init()
{
    // init all data here.
    m_data.itemID = m_self->itemID();
    m_data.state = 252; // no clue...seen this in packet logs
    m_data.timestamp = 0;
    m_data.status = EVEPOS::StructureState::Online;
    GVector dir(m_self->position(), m_planetSE->GetPosition());
    dir.normalize();
    m_data.rotation = dir;

    m_data.taxRate = 0.05;

    m_self->SetFlag(flagStructureActive);

    /* later we'll save data, but for now, just create on Init()
    if (!m_db.GetCOData(m_data)) {
        _log(POS__MESSAGE, "CustomsSE::Init %s(%u) has no saved data.  Initalizing default set.", m_self->itemName().c_str(), m_data.itemID);
        InitData();
        m_db.SaveCOData(m_data);
    } */
}

void CustomsSE::InitData()
{
    m_data.itemID = m_self->itemID();
    m_data.state = 252; // no clue...seen this in packet logs
    m_data.timestamp = 0;
    m_data.status = EVEPOS::StructureState::Online;
    GVector dir(m_self->position(), m_planetSE->GetPosition());
    dir.normalize();
    m_data.rotation = dir;
    m_data.taxRate = 0.05;
}

void CustomsSE::Process() {
    /* called by EntityList::Process on every loop */
    /*  Enable base call to Process Targeting and Movement  */
    SystemEntity::Process();
}

void CustomsSE::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;
    //const uint16 miniballsCount = GetMiniBalls();
    BallHeader head = BallHeader();
        head.entityID = m_data.itemID;
        head.radius = GetRadius();
        head.x = x();
        head.y = y();
        head.z = z();
        head.mode = Ball::Mode::RIGID;
        head.flags = Ball::Flag::IsGlobal /*| Ball::Flag::IsMassive | HasMiniBalls*/;
    into.Append( head );

    RIGID_Struct main;
        main.formationID = 0xFF;
    into.Append( main );

    /* TODO  query and configure miniballs for entity
     * NOTE  MiniBalls are BROKEN!!!  DO NOT USE!
     *    into.Append( miniballsCount );
     *    MiniBall miniball;
     *    for (int16 i; i<miniballsCount; i++) {
     *        miniball.x = -7701.181;
     *        miniball.y = 8060.06;
     *        miniball.z = 27878.900;
     *        miniball.radius = 1639.241;
     *        into.Append( miniball );
     *        miniball.clear();
     *    }
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

    _log(SE__DESTINY, "CustomsSE::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

PyDict *CustomsSE::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for CustomsSE %u", m_data.itemID);
    _log(POS__SLIMITEM, "MakeSlimItem for CustomsSE %u", m_data.itemID);
    /** @todo (Allan) *Timestamp will need to be set to time current state is started. */
    PyDict *slim = new PyDict();
    slim->SetItemString("name",                 new PyString(m_self->itemName()));
    slim->SetItemString("nameID",               PyStatic.NewNone());
    slim->SetItemString("itemID",               new PyLong(m_data.itemID));
    slim->SetItemString("typeID",               new PyInt(m_self->typeID()));
    slim->SetItemString("ownerID",              new PyInt(m_ownerID));  //1000148 for interbus customs office (to be done on creation)
    slim->SetItemString("corpID",               new PyInt(m_corpID));  //1000148 for interbus customs office (to be done on creation)
    slim->SetItemString("allianceID",           new PyInt(m_allyID));
    slim->SetItemString("warFactionID",         new PyInt(m_warID));
    slim->SetItemString("level",                PyStatic.NewOne()); //{1-CUSTOMSOFFICE_SPACEPORT, 2-CUSTOMSOFFICE_SPACEELEVATOR}   this is for display model
    slim->SetItemString("orbitalTimestamp",     PyStatic.NewNone()); //new PyLong(m_data.timestamp));
    slim->SetItemString("planetID",             new PyInt(m_data.planetID));  // planetID for this orbital
    slim->SetItemString("orbitalState",         new PyInt(m_data.state));   // this needs to be ORBITAL state...not structure state
    PyTuple* tuple = new PyTuple(3);
        tuple->SetItem(0,                       new PyFloat(m_data.rotation.x));
        tuple->SetItem(1,                       new PyFloat(m_data.rotation.y));
        tuple->SetItem(2,                       new PyFloat(m_data.rotation.z));
    slim->SetItemString("dunRotation", tuple);  // direction to planet
    //  dunno what these are...
    slim->SetItemString("orbitalHackerProgress", PyStatic.NewNone());
    slim->SetItemString("orbitalHackerID",      PyStatic.NewNone());

    if (is_log_enabled(POS__SLIMITEM)) {
        _log( POS__SLIMITEM, "CustomsSE::MakeSlimItem() - %s(%u)", GetName(), m_data.itemID);
        slim->Dump(POS__SLIMITEM, "     ");
    }
    return slim;
}

void CustomsSE::Killed(Damage &fatal_blow) {
    if ((m_bubble == nullptr) or (m_destiny == nullptr) or (m_system == nullptr))
        return; // make error here?

    uint32 killerID = 0;
    Client* pClient(nullptr);
    SystemEntity* killer = fatal_blow.srcSE;

    if (killer->HasPilot()) {
        pClient = killer->GetPilot();
        killerID = pClient->GetCharacterID();
    } else if (killer->IsDroneSE()) {
        pClient = sEntityList.FindClientByCharID( killer->GetSelf()->ownerID() );
        if (pClient == nullptr) {
            sLog.Error("CustomsSE::Killed()", "killer == IsDrone and pPlayer == nullptr");
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
    /** @todo send pos mail/notification to corp members */
    CharKillData data = CharKillData();
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
        data.moonID = m_data.planetID;    /* denotes moonID for POS/Structure kills */

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

    GPoint wreckPosition = m_destiny->GetPosition();
    std::string wreck_name = m_self->itemName();
    wreck_name += " Wreck";
    const char* faction = itoa(m_allyID);
    ItemData wreckItemData(3962/*CO gantry*/, killerID, locationID, flagAutoFit, wreck_name.c_str(), wreckPosition, faction);
    WreckContainerRef wreckItemRef = sItemFactory.SpawnWreckContainer( wreckItemData );
    if (wreckItemRef.get() == nullptr) {
        sLog.Error("CustomsSE::Killed()", "Creating Wreck Item Failed for %s of type %u", wreck_name.c_str(), 3962);
        return;
    }

    if (is_log_enabled(PHYSICS__TRACE))
        _log(PHYSICS__TRACE, "Ship::Killed() - Ship %s(%u) Position: %.2f,%.2f,%.2f.  Wreck %s(%u) Position: %.2f,%.2f,%.2f.", \
        GetName(), GetID(), x(), y(), z(), wreckItemRef->itemName().c_str(), wreckItemRef->itemID(), wreckPosition.x, wreckPosition.y, wreckPosition.z);

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
        wreckEntity.typeID = 3962;
        wreckEntity.x = wreckPosition.x;
        wreckEntity.y = wreckPosition.y;
        wreckEntity.z = wreckPosition.z;

    if (!m_system->BuildDynamicEntity(wreckEntity, m_self->itemID())) {
        sLog.Error("CustomsSE::Killed()", "Spawning Wreck Failed: typeID or typeName not supported: '%u'", 3962);
        wreckItemRef->Delete();
        return;
    }
}
