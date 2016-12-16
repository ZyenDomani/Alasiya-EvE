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

#ifndef __SYSTEMDB_H_INCL__
#define __SYSTEMDB_H_INCL__

#include "ServiceDB.h"

class DBSystemEntity {
public:
    uint32 itemID = 0;
    uint32 typeID = 0;
    uint32 groupID = 0;
    uint32 categoryID = 0;  //TODO populate this for simple system entities. (recently added - 1Dec15)
    uint32 orbitID = 0;
    GPoint position = NULL_ORIGIN;
    double radius = 0;
    double security = 0;
    std::string itemName = "";
};

class DBSystemDynamicEntity {
public:
    uint32 itemID = 0;
    std::string itemName = "";
    uint32 typeID = 0;
    uint32 groupID = 0;
    uint32 categoryID = 0;
    uint32 ownerID = 0;
    uint32 corporationID = 0;
    uint32 allianceID = 0;
    uint32 factionID = 0;
    uint32 planetID = 0;
    double x = 0;
    double y = 0;
    double z = 0;
};

struct DBGPointEntity {
    uint8 idx = 0;
    uint32 itemID = 0;
    double radius = 0;
    GPoint position = NULL_ORIGIN;
    double x = 0;
    double y = 0;
    double z = 0;
};

class SystemDB
: public ServiceDB
{
public:
    bool LoadSystemStaticEntities(uint32 systemID, std::vector<DBSystemEntity>& into);
    bool LoadSystemDynamicEntities(uint32 systemID, std::vector<DBSystemDynamicEntity>& into);
    bool LoadPlayerDynamicEntities(uint32 systemID, std::vector<DBSystemDynamicEntity>& into);
    static bool GetWrecksToTypes(DBQueryResult& res);
    static void GetLootGroups(DBQueryResult& res);
    static void GetLootGroupTypes(DBQueryResult& res);
    static void GetSalvageGroups(DBQueryResult& res);
    static uint32 GetObjectLocationID( uint32 itemID );
    double GetItemTypeRadius( uint32 typeID );
    double GetCelestialRadius(uint32 itemID);

    PyObject* ListFactions();
    PyObject* ListJumps(uint32);

    void GetPlanets(uint32 systemID, std::vector<DBGPointEntity>* planetIDs, uint8* total);
	void GetMoons(uint32 systemID, std::vector<DBGPointEntity>* moonIDs, uint8* total);
    void GetGates(uint32 systemID, std::vector<DBGPointEntity>* gateIDs, uint8* total);
    void GetBelts(uint32 systemID, std::vector<DBGPointEntity>* beltIDs, uint8* total);

};


#endif