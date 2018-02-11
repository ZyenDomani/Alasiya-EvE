
 /**
  * @name Sentry.cpp
  *   Sentry class
  * @Author:    Allan
  * @date:      23 April 2017
  */

#include "eve-server.h"

#include "Client.h"
#include "EntityList.h"
#include "EVEServerConfig.h"
#include "npc/Sentry.h"
#include "npc/SentryAI.h"
#include "system/Container.h"
#include "system/Damage.h"
#include "system/LootSystem.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"


Sentry::Sentry(InventoryItemRef self, PyServiceMgr& services, SystemManager* system, const FactionData& data)
: ObjectSystemEntity(self, services, system)
{
    m_allyID = data.allianceID;
    m_warID = data.factionID;
    m_corpID = data.corporationID;
    m_ownerID = data.ownerID;
    m_AI = new SentryAI(this);

    // Create default dynamic attributes in the AttributeMap:
    m_self->SetAttribute(AttrDamage,              0);
    m_self->SetAttribute(AttrArmorDamage,         0);
    m_self->SetAttribute(AttrMass,                m_self->type().mass());
    m_self->SetAttribute(AttrRadius,              m_self->type().radius());
    m_self->SetAttribute(AttrVolume,              m_self->type().volume());
    m_self->SetAttribute(AttrCapacity,            m_self->type().capacity());
    m_self->SetAttribute(AttrShieldCharge,        m_self->GetAttribute(AttrShieldCapacity), false);
    m_self->SetAttribute(AttrCapacitorCharge,     m_self->GetAttribute(AttrCapacitorCapacity), false);

    SetResists();

    /* Gets the value from the Sentry and put on our own vars */
    m_emDamage = m_self->GetAttribute(AttrEmDamage).get_float(),
    m_kinDamage = m_self->GetAttribute(AttrKineticDamage).get_float(),
    m_therDamage = m_self->GetAttribute(AttrThermalDamage).get_float(),
    m_expDamage = m_self->GetAttribute(AttrExplosiveDamage).get_float(),
    m_hullDamage = m_self->GetAttribute(AttrDamage).get_float();
    m_armorDamage = m_self->GetAttribute(AttrArmorDamage).get_float();
    m_shieldCharge = m_self->GetAttribute(AttrShieldCharge).get_float();
    m_shieldCapacity = m_self->GetAttribute(AttrShieldCapacity).get_float();

    // _log(Sentry__TRACE, "Created Sentry object for %s (%u)", m_self.get()->itemName().c_str(), m_self.get()->itemID());
}

Sentry::~Sentry() {
    SafeDelete(m_destiny);
    SafeDelete(m_AI);
}

void Sentry::Process() {
    double profileStartTime = 0.0;
    if (sConfig.debug.UseProfiling)
        profileStartTime = GetTimeUSeconds();

    /*  Enable base call to Process Targeting and Movement  */
    SystemEntity::Process();
    m_AI->Process();

    if (sConfig.debug.UseProfiling)
        sProfile.AddTime(_npcProfile, GetTimeUSeconds() - profileStartTime);
}

void Sentry::TargetLost(SystemEntity *who) {
    m_AI->TargetLost(who);
}

void Sentry::TargetedAdd(SystemEntity *who) {
    m_AI->Targeted(who);
}

void Sentry::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;

    BallHeader head;
        head.entityID = GetID();
        head.mode = DSTBALL_STOP;
        head.radius = GetRadius();
        head.x = x();
        head.y = y();
        head.z = z();
        head.flags = IsMassive;
    into.Append( head );
    MassSector mass;
        mass.mass = m_self->type().mass();
        mass.cloak = 0;
        mass.harmonic = m_harmonic;
        mass.corporationID = m_corpID;
        mass.allianceID = (m_allyID > 0 ? m_allyID : -1);
    into.Append( mass );
    DSTBALL_STOP_Struct main;
        main.formationID = 0xFF;
    into.Append( main );

    _log(SE__DESTINY, "Sentry::EncodeDestiny: %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void Sentry::SaveSentry()
{
    m_self->SaveItem();
}

void Sentry::RemoveSentry()
{
    //this is called from SystemManager::RemoveSentry() which calls other SE* methods as needed
    m_self->Delete();
}

void Sentry::SetResists() {
    /* fix for missing resist attribs -allan 18April16  */
    if (!m_self->HasAttribute(AttrShieldEmDamageResonance)) m_self->SetAttribute(AttrShieldEmDamageResonance, 1.0);
    if (!m_self->HasAttribute(AttrShieldExplosiveDamageResonance)) m_self->SetAttribute(AttrShieldExplosiveDamageResonance, 1.0);
    if (!m_self->HasAttribute(AttrShieldKineticDamageResonance)) m_self->SetAttribute(AttrShieldKineticDamageResonance, 1.0);
    if (!m_self->HasAttribute(AttrShieldThermalDamageResonance)) m_self->SetAttribute(AttrShieldThermalDamageResonance, 1.0);
    if (!m_self->HasAttribute(AttrArmorEmDamageResonance)) m_self->SetAttribute(AttrArmorEmDamageResonance, 1.0);
    if (!m_self->HasAttribute(AttrArmorExplosiveDamageResonance)) m_self->SetAttribute(AttrArmorExplosiveDamageResonance, 1.0);
    if (!m_self->HasAttribute(AttrArmorKineticDamageResonance)) m_self->SetAttribute(AttrArmorKineticDamageResonance, 1.0);
    if (!m_self->HasAttribute(AttrArmorThermalDamageResonance)) m_self->SetAttribute(AttrArmorThermalDamageResonance, 1.0);
    if (!m_self->HasAttribute(AttrEmDamageResonance)) m_self->SetAttribute(AttrEmDamageResonance, 1.0);
    if (!m_self->HasAttribute(AttrExplosiveDamageResonance)) m_self->SetAttribute(AttrExplosiveDamageResonance, 1.0);
    if (!m_self->HasAttribute(AttrKineticDamageResonance)) m_self->SetAttribute(AttrKineticDamageResonance, 1.0);
    if (!m_self->HasAttribute(AttrThermalDamageResonance)) m_self->SetAttribute(AttrThermalDamageResonance, 1.0);
}

void Sentry::Killed(Damage &fatal_blow) {
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
            sLog.Error("Sentry::Killed()", "killer == IsDrone and pPlayer == nullptr");
        } else
            killerID = pClient->GetCharacterID();
    } else
        killerID = killer->GetID();

    m_destiny->SendTerminalExplosion(GetID(), m_bubble->GetID());

    GPoint deadNPCPosition = m_destiny->GetPosition();
    uint32 wreckTypeID = sWreckData.GetWreckID(m_self->typeID());
    if (!wreckTypeID) {
        sLog.Error("Sentry::Killed()", "Could not get wreckType for %s of type %u", m_self->itemName().c_str(), m_self->typeID());
        // default to generic frigate wreck till i get better checks and/or complete wreck data
        wreckTypeID = 26557;
    }

    uint32 locationID = GetLocationID();
    std::string wreck_name = m_self->itemName();
    wreck_name += " Wreck";
    const char* faction = itoa(m_allyID);
    ItemData wreckItemData(wreckTypeID, killerID, locationID, flagAutoFit, wreck_name.c_str(), deadNPCPosition, faction);
    WreckContainerRef wreckItemRef = sItemFactory.SpawnWreckContainer( wreckItemData );
    if (!wreckItemRef) {
        sLog.Error("Sentry::Killed()", "Creating Wreck Item Failed for %s of type %u", wreck_name.c_str(), wreckTypeID);
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
        wreckEntity.x = deadNPCPosition.x;
        wreckEntity.y = deadNPCPosition.y;
        wreckEntity.z = deadNPCPosition.z;

    if (!m_system->BuildDynamicEntity(wreckEntity)) {
        sLog.Error("Sentry::Killed()", "Spawning Wreck Failed: typeID or typeName not supported: '%u'", wreckTypeID);
        return;
    }

    _log(PHYSICS__TRACE, "Sentry::Killed() - Wreck %s(%u) Item Position: %.2f,%.2f,%.2f.  Destiny Position: %.2f,%.2f,%.2f.", \
    GetName(), GetID(), x(), y(), z(), deadNPCPosition.x, deadNPCPosition.y, deadNPCPosition.z);

    // cleanup and removal of dead npc
    //AI()->ClearAllTargets();
    m_system->RemoveEntity(this);  //this also removes npc from db
}
