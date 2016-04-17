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
    Author:        Allan
*/

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "Profile.h"
#include "PyServiceMgr.h"
#include "system/SystemBubble.h"
#include "system/SystemEntities.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/BeltMgr.h"


BeltMgr::BeltMgr(SystemManager* mgr, PyServiceMgr& svc)
: m_system(mgr),
  m_services(svc),
  m_growthTimer(m_growthTimer)
{
    m_initialized = false;

    m_growthTimer.Disable();

    m_systemID = m_system->GetID();
}

BeltMgr::~BeltMgr()
{
    Save();
    ClearAll();
}

void BeltMgr::Init()
{
    m_initialized = true;
    _log(COSMIC_MGR__MESSAGE, "BeltMgr Initialized for %s(%u)", m_system->GetName().c_str(), m_systemID);
}

void BeltMgr::RegisterBelt(InventoryItemRef itemRef)
{
    uint32 beltID = itemRef->itemID();
    m_belts.insert(std::pair<uint32, InventoryItemRef>(beltID, itemRef));
    m_spawned.insert(m_spawned.end(), std::pair<uint32, bool>(beltID, false));
    CheckSpawn(itemRef);
    SystemEntity *se = m_system->GetSEFromInventory(beltID);
    se->Bubble()->SetBelt(beltID);
}

void BeltMgr::ClearBelt()
{
    ClearAll();
}

bool BeltMgr::CheckSpawn(InventoryItemRef itemRef)
{
    if (IsSpawned(itemRef)) return true;
    /*  if there are already roids created for this belt, they will be loaded in Load()
     * and NOT LOADED in loadsystemdynamics from SystemManager.
     * if Load() has roids for this belt, this belt will have true already set, and checked in SpawnBelt()
     */
    if (!Load(itemRef->itemID()))
        SpawnBelt(itemRef);
}

bool BeltMgr::IsSpawned(InventoryItemRef itemRef)
{
    return IsSpawned(itemRef->itemID());
}

bool BeltMgr::IsSpawned(uint32 beltID)
{
    std::map<uint32, bool>::iterator itr = m_spawned.find(beltID);
    if (itr != m_spawned.end())
        return itr->second;
    return false;
}

void BeltMgr::Clear() {
    for(auto cur : m_asteroids)
        SafeDelete(cur.second);
    m_asteroids.clear();
}

void BeltMgr::ClearAll() {
    for(auto cur : m_asteroids)
        SafeDelete(cur.second);
    m_asteroids.clear();
    m_belts.clear();
}

void BeltMgr::Process() {
    if (m_growthTimer.Check()) {
        _TriggerGrowth();
    }
}

void BeltMgr::_TriggerGrowth() {
    for(auto cur : m_asteroids)
        cur.second->Grow();
}

bool BeltMgr::Load(uint32 beltID) {
    std::vector<DBAsteroidEntity> entities;
    entities.clear();
    m_db.LoadSystemRoids(m_systemID, beltID, entities);
    if (entities.empty()) return false;

    for(auto entity : entities) {
        InventoryItemRef asteroid = m_system->GetServiceMgr()->item_factory->GetItem( entity.itemID );
        if( !asteroid ) {
            _log(COSMIC_MGR__WARNING, "BeltMgr::Load() -  Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID);
            continue;
        }
        GPoint location(entity.x, entity.y, entity.z);
        AsteroidEntity* asteroidObj = new AsteroidEntity( asteroid, m_system, *(m_system->GetServiceMgr()), location );
        if( !asteroidObj ) {
            _log(COSMIC_MGR__WARNING, "BeltMgr::Load() -  Unable to spawn entity #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID);
            continue;
        }
        _log(COSMIC_MGR__TRACE, "BeltMgr::Load() - Loaded asteroid %u, type %u for %s(%u)", entity.itemID, entity.typeID, m_system->GetName().c_str(), m_systemID );
        m_system->bubbles.Add( asteroidObj );
        m_asteroids.emplace(std::pair<uint32, AsteroidEntity*>(beltID, asteroidObj));
    }
    std::map<uint32, bool>::iterator itr = m_spawned.find(beltID);
    if (itr == m_spawned.end())
        m_spawned.insert(std::pair<uint32, bool>(beltID, true));
    else
        itr->second = true;

    return true;
}

void BeltMgr::Save() {
    DBAsteroidEntity entry;
    std::vector<DBAsteroidEntity> roids;
    roids.clear();
    for (auto cur : m_asteroids) {
        entry.itemID = cur.second->GetID();
        entry.itemName = cur.second->GetName();
        entry.typeID = cur.second->Item()->typeID();
        entry.systemID = m_systemID;
        entry.beltID = cur.first;
        entry.radius = cur.second->GetRadius();
        entry.quantity = ((25000 * log(entry.radius)) - 112404.8);   // quantity in m^3
        entry.x = cur.second->x();
        entry.y = cur.second->y();
        entry.z = cur.second->z();
        roids.push_back(entry);
    }

    _log(COSMIC_MGR__TRACE, "BeltMgr::Save - Saving %u Asteroids in %s(%u) ", roids.size(), m_system->GetName().c_str(), m_systemID );
    m_db.SaveSystemRoids(m_systemID, roids);
}

void BeltMgr::ForceGrowth() {
    for (auto cur : m_asteroids) {
        /** @todo (allan) do something useful here */
    }
    _TriggerGrowth();
    m_growthTimer.Start(ASTEROID_GROWTH_INTERVAL_MS);
}

void BeltMgr::GetList(uint32 beltID, std::vector< AsteroidEntity* >& list)
{
    auto range = m_asteroids.equal_range(beltID);
    for (auto itr = range.first; itr != range.second; ++itr)
        list.push_back(itr->second);
}

void BeltMgr::SpawnBelt(InventoryItemRef itemRef)
{
    if (IsSpawned(itemRef)) return;

    bool makeIceBelt = false;
    bool makeRareIce = false;
    std::map<float, uint32> roidDist;
    if( makeIceBelt ) {
        // put this in db or mem map?
        if (makeRareIce) {
            roidDist.insert(std::pair<float,uint32>(0.9,16263));      // Glacial Mass
            roidDist.insert(std::pair<float,uint32>(0.8,16265));      // White Glaze
            roidDist.insert(std::pair<float,uint32>(0.7,16266));      // Glare Crust
            roidDist.insert(std::pair<float,uint32>(0.6,16268));      // Gelidus
            roidDist.insert(std::pair<float,uint32>(0.5,16269));      // Krystallos
            roidDist.insert(std::pair<float,uint32>(0.4,17976));      // Pristine White Glaze
            roidDist.insert(std::pair<float,uint32>(0.3,17977));      // Smooth Glacial Mass
            roidDist.insert(std::pair<float,uint32>(0.2,17978));      // Enriched Clear Icicle
            roidDist.insert(std::pair<float,uint32>(0.1,28628));      // Crystalline Icicle
        } else {
            roidDist.insert(std::pair<float,uint32>(0.6,16264));        // Blue Ice
            roidDist.insert(std::pair<float,uint32>(0.4,17975));        // Thick Blue Ice
            roidDist.insert(std::pair<float,uint32>(0.3,28627));        // Azure Ice
            roidDist.insert(std::pair<float,uint32>(0.2,16262));        // Clear Icicle
            roidDist.insert(std::pair<float,uint32>(0.1,16267));        // Dark Glitter
        }
    } else {
        if( !m_db.GetRoidDist( m_system->GetSystemSecurityClass(), roidDist ) ) {
            _log(COSMIC_MGR__ERROR, "BeltMgr::SpawnBelt - could not get roid distribution for sysSecClass %s", m_system->GetSystemSecurityClass() );
        }
    }

    int8 pcs = 15 + MakeRandomInt( -5, 10 );

    double radius = 15000;
    double security = 1.1 - m_system->GetSystemSecurityRating();
    radius += (radius *security);

    GPoint mposition = NULL_ORIGIN;
    double roidradius = 0, theta = 0;
    double degreeSeperation = (180/pcs);
    uint32 beltID = itemRef->itemID();
    SystemEntity *se = m_system->GetSEFromInventory(beltID);
    GPoint center = se->Bubble()->GetCenter();

    for (uint32 i = 1; i < pcs; ++i) {
        roidradius = MakeRandomFloat( 3000.0, 8000.0 ) *security;
        theta = EvE_DegreesToRadians(degreeSeperation *i);
        mposition.x = radius * cos(theta);
        mposition.z = radius * sin(theta);
        mposition.y = MakeRandomFloat( -(radius/12), (radius/10) );
        SpawnAsteroid(beltID, GetAsteroidType(MakeRandomFloat(), roidDist), roidradius, (center +mposition));
    }

    std::map<uint32, bool>::iterator itr = m_spawned.find(beltID);
    if (itr == m_spawned.end())
        m_spawned.insert(std::pair<uint32, bool>(beltID, true));
    else
        itr->second = true;

    _log(COSMIC_MGR__TRACE, "BeltMgr::SpawnBelt - Belt spawned with %u roids in beltID %u for %s(%u)", pcs, beltID, m_system->GetName().c_str(), m_systemID );
}

uint32 BeltMgr::GetAsteroidType(double p, const std::map<float, uint32>& roids) {
    std::map<float, uint32>::const_iterator cur = roids.begin();
    float chance = 0.0;
    for(; cur != roids.end(); ++cur ) {
        chance += cur->first;
        if ( chance > p )
            return cur->second;
    }
    --cur;

    return cur->second;
}

void BeltMgr::SpawnAsteroid(uint32 beltID, uint32 typeID, double radius, const GPoint& position) {
    ItemData idata( typeID, 1, m_systemID, flagAutoFit, "", position );
    InventoryItemRef i = m_system->itemFactory()->SpawnItem( idata );
    if (!i)
        return;

    //Amount of Ore = (25000*ln(Radius))-112404.8   V = 25000Ln(r) - 112407
    double quantity = ((25000 * log(radius)) - 112404.8);

    i->SetAttribute(AttrQuantity,  quantity);   // quantity in m^3
    i->SetAttribute(AttrRadius, (i->type().radius() * radius));  // Radius
    i->SetAttribute(AttrMass, (i->type().mass() * quantity));      // Mass
    i->SaveAttributes();

    AsteroidEntity* new_roid = new AsteroidEntity( i, m_system, *(m_system->GetServiceMgr()), position );
    m_system->bubbles.Add( new_roid );
    m_asteroids.emplace(std::pair<uint32, AsteroidEntity*>(beltID, new_roid));
}

/*          this gives random single point on sphere with radius of 'r'
 *
        double theta = MakeRandomFloat( 0.0, (2*M_PI) );
        double phi = MakeRandomFloat( 0.0, (2*M_PI) );
        x += r * sin(theta) * cos(phi);
        y += r * sin(theta) * sin(phi);
        z += r * cos(theta);

*/
/*          straight line from center
 *
        theta += angleSeperation;
        double theta = MakeRandomFloat( 0.0, (theta) );
        double phi = MakeRandomFloat( 0.0, (2*M_PI) );
        mposition.x += radius * sin(theta) * cos(phi);
        mposition.y += radius * sin(theta) * sin(phi);
        mposition.z += radius * cos(theta);
    */

/*      up-an-over arc from center
 *
        theta += angleSeperation;
        double theta = MakeRandomFloat( 0.0, (theta) );
        double phi = MakeRandomFloat( 0.0, (2*M_PI) );
        mposition.x += radius * sin(theta) * cos(phi);
        mposition.y += radius * sin(theta) * sin(phi);
        mposition.z += radius * cos(theta);
*/
/*      this makes triple helix design      -allan 7May15
 *
    int8 pcs = 0, rand = MakeRandomInt( -10, 20 );
    if( customCount > 15 )
        pcs = customCount + rand;
    else
        pcs = 30 + rand;

    float radius = 15000;
    const double security = 1.1 - m_db.GetSecurity(sys->GetID());
    radius += (radius * security) *2;

    GPoint mposition = NULL_ORIGIN;
    const GPoint position( who->Bubble()->GetCenter() );
    double roidradius = 0, thefloat = 0;
    uint32 theta = 0, angleSeperation = floor(180 /pcs);

    for (uint32 i = 0; i < pcs; ++i) {
        roidradius = 3000;

        if (makeIceBelt) {
            thefloat = MakeRandomFloat( 1.0, 5.0 );
            roidradius += thefloat * 1000;
            roidradius *= 5;
        } else {
            thefloat = MakeRandomFloat( 1.0, 7.0 );
            roidradius += thefloat * 1000;
        }
        roidradius *= security;

        theta += angleSeperation;
        mposition.x = cos(theta) *radius + (roidradius *2) + MakeRandomFloat( -1000, 5000 );
        mposition.z = sin(theta) *radius + (roidradius *2) + MakeRandomFloat( -3000, 5000 );
        mposition.y += MakeRandomFloat( -2000, 3000 ) + roidradius;

        SpawnAsteroid( sys, GetAsteroidType( MakeRandomFloat(), roidDist ), roidradius, position + mposition );
        sLog.Warning( "Command_spawnbelt", "roidradius: %.2f, mpos: %.2f, %.2f, %.2f, angleSeperation: %u, theta: %u",
                        roidradius, mposition.x, mposition.y, mposition.z, angleSeperation, theta );
    }

*/
/*
    for (uint32 i = 0; i < pcs; ++i) {
        roidradius = MakeRandomFloat( 1.0, 7.0 ) *1000 *security +secRad;
        theta = MakeRandomFloat( 0.0, M_PI );
        phi = MakeRandomFloat( 0.0, M_PI );
        mposition.x = radius * sin(theta) * cos(phi);
        mposition.z = radius * sin(theta) * sin(phi);
        mposition.y = MakeRandomInt(-secRad, secRad);
*/

    /*
     *  triple helix
        uint32 theta = 0, angleSeperation = floor(180 /pcs);
        theta += angleSeperation;
        mposition.x = cos(theta) *radius + (roidradius *2) + MakeRandomFloat( -1000, 5000 );
        mposition.z = sin(theta) *radius + (roidradius *2) + MakeRandomFloat( -3000, 5000 );
        mposition.y += MakeRandomFloat( -2000, 3000 ) + roidradius;
     */
    /*
     *  flat circle @ 50k
        uint32 theta = 0, angleSeperation = floor(180 /pcs);
        theta += angleSeperation;
        mposition.x = radius * cos(theta);
        mposition.z = radius * sin(theta);
     */
    /*
     *   ufo style
        uint32 theta = 0, angleSeperation = floor(180 /pcs);
        phi += angleSeperation;
        if (phi > 90)
            theta = phi - 180;
        else
            theta = phi;

        mposition.x = radius * sin(theta) + (roidradius /3);
        mposition.z = radius * cos(phi) + (roidradius /3);
        mposition.y = MakeRandomInt(-secRad, secRad);
    */
    /*
     * diagonal
        uint32 theta = 0, angleSeperation = floor(180 /pcs);
        phi += angleSeperation;
        if (phi > 90)
            theta = phi - 180;
        else
            theta = phi;

        mposition.x = radius * sin(theta);// + (roidradius /3);
        mposition.z = radius * cos(phi);// + (roidradius /3);
        mposition.y = MakeRandomInt(-secRad, secRad);
    */