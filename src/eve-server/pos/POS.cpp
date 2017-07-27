
/**
 * @name POS.cpp
 *   Specific Class for POS entities.
 *
 * @Author:         Allan
 * @date:   24July17
 */


#include "Client.h"
#include "EntityList.h"
#include "EVEServerConfig.h"
#include "pos/POS.h"
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


TowerSE::TowerSE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& fData)
: StructureSE(structure, services, system, fData)
{
    // ct will anchor in the middle of the grid that you warp-in to.
    data.status = 0.0f;
    data.standing = 0.0f;
    data.standingOwnerID = 0;
    data.corpWar = false;
    data.statusDrop = false;
    data.showInCalendar = false;
    data.sendFuelNotifications = false;

    /** @todo  may need to save this itemID, instead of calculating every time. */
    m_moonSE = m_system->GetNearestMoon(GetPosition());
}

void TowerSE::Init(StructureItemRef structure)
{
    EVEPOS::SaveData data;
    m_db.GetPOSData(data);

    m_harmonic = EVEPOS::ForceField::inactive; // or whatever the harmonic is for this tower....

    // create and add force field to tower
    /** @todo  this will need to be based on structure state */
    //ItemData( uint32 _typeID, uint32 _ownerID, uint32 _locationID, EVEItemFlags _flag, uint32 _quantity, const char *_customInfo = "", bool _contraband = false);
    ItemData idata(EVEDB::invTypes::typeForceField, m_corpID, m_system->GetID(), flagAutoFit, m_ownerID);
    InventoryItemRef iRef = m_services.item_factory->SpawnItem(idata);
    if (iRef.get() == nullptr)
        ;  // we'll get over it
    iRef->Relocate(GetPosition());
    iRef->SetAttribute(AttrRadius, m_self->GetAttribute(AttrShieldRadius));
    ItemSystemEntity* iSE = new ItemSystemEntity(iRef, m_services, m_system);
    m_system->AddEntity(iSE);
}

void TowerSE::Process()
{
    /* called by EntityList::Process on every loop */
    /*  Enable base call to Process state changes  */
    StructureSE::Process();

}

PyDict* TowerSE::MakeSlimItem()
{
    _log(SE__SLIMITEM, "MakeSlimItem for TowerSE %u", m_self->itemID());
    _log(POS__SLIMITEM, "MakeSlimItem for TowerSE %u", m_self->itemID());

    PyDict *slim = new PyDict();
    slim->SetItemString("name",                     new PyString(m_self->itemName()));
    slim->SetItemString("nameID",                   new PyNone());
    slim->SetItemString("itemID",                   new PyLong(m_self->itemID()));
    slim->SetItemString("typeID",                   new PyInt(m_self->typeID()));
    slim->SetItemString("ownerID",                  new PyInt(m_self->ownerID()));
    slim->SetItemString("corpID",                   new PyInt(m_corpID));
    slim->SetItemString("allianceID",               new PyInt(m_allyID));
    slim->SetItemString("warFactionID",             new PyInt(m_warID));
    slim->SetItemString("posTimestamp",             new PyLong((m_timestamp > 0) ? m_timestamp : 0));
    slim->SetItemString("posState",                 new PyInt(GetStructureState()));
    slim->SetItemString("incapacitated",            new PyInt((m_state == EVEPOS::StructureState::Incapacitated) ? 1 : 0));
    // this is time shown in structure status (time left until current state completes)
    if (m_delayTime)
        slim->SetItemString("posDelayTime",         new PyInt(m_delayTime));

    if (is_log_enabled(POS__DEBUG)) {
        _log( POS__DEBUG, "TowerSE::MakeSlimItem()", "%s(%u)", GetName(), GetID());
        slim->Dump(POS__DEBUG, "     ");
    }
    return slim;
}

void TowerSE::UpdateSentry()
{
    m_db.UpdatePOSSentry(m_towerID, data);
}

void TowerSE::UpdateNotify()
{
    m_db.UpdatePOSNotify(m_towerID, data);
}

void TowerSE::UpdatePermission()
{
    m_db.UpdatePOSPermission(m_towerID, data);
}

void TowerSE::UpdateTimeStamp()
{
    m_db.UpdatePOSTimeStamp(m_towerID, m_timestamp);
}

/*  for updating structure data
 *
 * EVEPOS::SaveData data;
 * m_db.UpdatePOSData(data);
 */

/** @todo (Allan) set/get control tower id for modules in/from customInfo field of db */
ArraySE::ArraySE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& data)
: StructureSE(structure, services, system, data)
{

}

void ArraySE::Init(StructureItemRef structure)
{

}

void ArraySE::Process()
{
    StructureSE::Process();
}


WeaponSE::WeaponSE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& data)
: StructureSE(structure, services, system, data)
{

}

void WeaponSE::Init(StructureItemRef structure)
{

}

void WeaponSE::Process()
{
    /* called by EntityList::Process on every loop */
    /*  Enable base call to Process state changes  */
    StructureSE::Process();
    /** @todo (Allan)  will need some form of AI to engage defensive modules if/when any structure is attacked */
}


BatterySE::BatterySE(StructureItemRef structure, PyServiceMgr& services, SystemManager* system, const FactionData& data)
: StructureSE(structure, services, system, data)
{

}

void BatterySE::Init(StructureItemRef structure)
{

}

void BatterySE::Process()
{
    /* called by EntityList::Process on every loop */
    /*  Enable base call to Process state changes  */
    StructureSE::Process();
    /** @todo (Allan)  will need some form of AI to engage defensive modules if/when any structure is attacked */
}
