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
#include "system/BeltMgr.h"

/*  this class will keep track of all asteroid belts in a system
 * it is created on a per-system basis, and will also deal with
 * calling spawn/delete/grow functions for each belt.
 *
 * this class will also be in charge of belts in anomalies
 *
 *  a new iteration of this class is created for each system as that system
 * is booted.
 */


BeltMgr::BeltMgr(SystemManager* mgr, PyServiceMgr& svc)
: m_system(mgr),
  m_services(svc),
  m_growthTimer(m_growthTimer)
{
    m_growthTimer.Disable();
}

BeltMgr::~BeltMgr()
{
    _Clear();
}

void BeltMgr::_Clear() {
    for(auto cur : m_asteroids)
        SafeDelete(cur.second);
}

void BeltMgr::Process() {
    if (m_growthTimer.Check()) {
        _TriggerGrowth();
    }
}

void BeltMgr::_TriggerGrowth() {
    _log(SERVICE__ERROR, "Asteroid Growth not implemented yet.");
    for(auto cur : m_asteroids)
        cur.second->Grow();
}

bool BeltMgr::LoadState() {
    //load list of asteroids.
    //load next grow time
    for (auto cur : m_asteroids) {
        /** @todo (allan) do something useful here */
    }
    return false;    //until this is written.
}

bool BeltMgr::SaveState() {
    for (auto cur : m_asteroids) {
        /** @todo (allan) do something useful here */
    }
    return false;    //until this is written.
}

void BeltMgr::ForceGrowth() {
    for (auto cur : m_asteroids) {
        /** @todo (allan) do something useful here */
    }
    _TriggerGrowth();
    m_growthTimer.Start(ASTEROID_GROWTH_INTERVAL_MS);
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
    const double security = 1.1 - db->GetSecurity(sys->GetID());
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