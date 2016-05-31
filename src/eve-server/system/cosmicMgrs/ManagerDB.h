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

#ifndef _EVEMU_SYSTEM_COSMICMGRS_MANAGERDB_H
#define _EVEMU_SYSTEM_COSMICMGRS_MANAGERDB_H

#include "system/SystemDB.h"

/* POD entry for asteroid */
class DBAsteroidSE {
public:
    uint32 itemID;
    std::string itemName;
    uint32 typeID;
    uint32 systemID;
    uint32 beltID;
    double quantity;
    double radius;
    double x;
    double y;
    double z;
};

class ManagerDB {
public:
    /* these are for belt manager */
    bool GetRoidDist(const char * sec, std::map<float, uint32> &roids);
    void SaveSystemRoids(uint32 systemID, std::vector<DBAsteroidSE> roids);
    bool LoadSystemRoids(uint32 systemID, uint32 beltID, std::vector<DBAsteroidSE>& into);

    /*  these are for spawn manager */
    void DeleteSpawnedRats();
    void GetSpawnClasses(DBQueryResult& res);
    void GetGroupTypeIDs(uint32 groupID, DBQueryResult& res);
    void GetFactionGroups(DBQueryResult& res);
    void GetRegionFactionInfo(DBQueryResult& res);

    /* these are for anomaly manager */

    /* these are for dungeon manager */

    /* these are for wormhole manager */

protected:

private:


};



#endif  // _EVEMU_SYSTEM_COSMICMGRS_MANAGERDB_H