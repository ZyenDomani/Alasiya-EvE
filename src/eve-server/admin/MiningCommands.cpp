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
    Author:        Zhur
    Updates:    Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "PyCallable.h"
#include "admin/CommandDB.h"
#include "system/Asteroid.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"

uint32 GetAsteroidType( double p, const std::map<float, uint32>& roids );
void SpawnAsteroid( SystemManager* pSystem, uint32 typeID, double radius, const GPoint& position );

PyResult Command_roid( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
    if( !args.isNumber( 1 ) )
        throw PyException( MakeCustomError( "Argument 1 should be an item type ID" ) );
    const uint32 typeID = atoi( args.arg( 1 ).c_str() );

    if( !args.isNumber( 2 ) )
        throw PyException( MakeCustomError( "Argument 2 should be a radius" ) );
    const double radius = atof( args.arg( 2 ).c_str() );

    if( 0 >= radius )
        throw PyException( MakeCustomError( "Invalid radius." ) );

    if( !who->IsInSpace() )
        throw PyException( MakeCustomError( "You must be in space to spawn things." ) );

    sLog.White( "Command", "Roid %u of radius %f", typeID, radius );

    GPoint position( who->GetShipSE()->GetPosition() );
    position.x += radius + 1 + who->GetShipSE()->GetRadius();    //put it far enough away to not push us around.

    SpawnAsteroid( who->SystemMgr(), typeID, radius, position );

    return new PyString( "Spawn successful." );
}

PyResult Command_growbelt( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args )
{
    throw PyException( MakeCustomError( "Not implemented yet." ) );
}

PyResult Command_spawnbelt( Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args ) {
    if( !who->IsInSpace() )
        throw PyException( MakeCustomError( "You must be in space to spawn things." ) );

	bool makeIceBelt = false;
	bool makeRareIce = false;
	uint32 customCount = 0;
    if ( args.argCount() >= 2 ) {
        if ( !args.isNumber( 1 ) ) {
			if ( args.arg( 1 ) == "ice" )
				makeIceBelt = true;
		} else {
			if ( atoi(args.arg( 1 ).c_str()) > 15 )
				customCount = atoi(args.arg( 1 ).c_str());
			else
				PyException( MakeCustomError( "Argument 1 should be at least 15!" ) );
		}
	}

	if ( args.argCount() >= 3 ) {
		if ( args.isNumber( 2 ) ) {
			if ( atoi(args.arg( 2 ).c_str()) > 15 )
				customCount = atoi(args.arg(2).c_str());
			else
				PyException( MakeCustomError( "Argument 2 should be at least 15!" ) );
		} else {
			if( args.arg( 2 ) == "ice" )
				makeIceBelt = true;
			else if( args.arg( 2 ) == "rareice" ) {
				makeIceBelt = true;
				makeRareIce = true;
			}
        }
	}

	SystemManager* pSystem = who->SystemMgr();
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
            roidDist.insert(std::pair<float,uint32>(0.6,16264));		// Blue Ice
            roidDist.insert(std::pair<float,uint32>(0.4,17975));		// Thick Blue Ice
            roidDist.insert(std::pair<float,uint32>(0.3,28627));		// Azure Ice
            roidDist.insert(std::pair<float,uint32>(0.2,16262));		// Clear Icicle
            roidDist.insert(std::pair<float,uint32>(0.1,16267));		// Dark Glitter
		}
	} else {
        if( !db->GetRoidDist( pSystem->GetSystemSecurityClass(), roidDist ) ) {
            sLog.Error( "Command_spawnbelt", "Couldn't get roid list for system security class %s", pSystem->GetSystemSecurityClass() );
            throw PyException( MakeCustomError( "Couldn't get roid list for system security class %s", pSystem->GetSystemSecurityClass() ) );
		}
	}

    int8 pcs = 0, rand = MakeRandomInt( -10, 20 );
    if( customCount > 15 )
        pcs = customCount + rand;
    else
        pcs = 40 + rand;

    double radius = 22000;
    double security = 1.1 - db->GetSecurity(pSystem->GetID());
    radius += (radius *security);

    GPoint mposition = NULL_ORIGIN;
    double roidradius = 0, theta = 0;
    double degreeSeperation = (180/pcs);
    GPoint center = who->GetShipSE()->SysBubble()->GetCenter();

    for (uint32 i = 1; i < pcs; ++i) {
        roidradius = MakeRandomFloat( 3000.0, 8000.0 ) *security;
        theta = EvE_DegreesToRadians(degreeSeperation *i);
        mposition.x = radius * cos(theta);
        mposition.z = radius * sin(theta);
        mposition.y = MakeRandomFloat( -(radius/12), (radius/10) );
        SpawnAsteroid( pSystem, GetAsteroidType( MakeRandomFloat(), roidDist ), roidradius, (center +mposition) );
    }

    return new PyString( "Spawn successsful." );
}

uint32 GetAsteroidType( double p, const std::map<float, uint32>& roids )
{
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

void SpawnAsteroid( SystemManager* pSystem, uint32 typeID, double radius, const GPoint& position )
{
    ItemData idata( typeID,
                    1,
                    pSystem->GetID(),
                    flagAutoFit,
                    "",    //name
                    position );

    InventoryItemRef i = pSystem->itemFactory()->SpawnItem(idata);
    if (!i)
        throw PyException(MakeCustomError("Unable to spawn item of type %u.", typeID));

    //Amount of Ore = (25000*ln(Radius))-112404.8   V = 25000Ln(r) - 112407
    double quantity = ((25000 * log(radius)) - 112404.8);

	i->SetAttribute(AttrQuantity,  quantity);   // quantity in m^3
    i->SetAttribute(AttrRadius, (i->type().radius() * radius));  // Radius
    i->SetAttribute(AttrMass, (i->type().mass() * quantity));      // Mass
    i->SaveAttributes();

    AsteroidSE* se = new AsteroidSE(i, *(pSystem->GetServiceMgr()), pSystem);

    //TODO: check for a local asteroid belt object?
    //TODO: actually add this to the asteroid belt too...
    pSystem->AddEntity( se );
}
