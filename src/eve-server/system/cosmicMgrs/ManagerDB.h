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


#include "POD_containers.h"
#include "system/SystemDB.h"

/* more data for signatures...
 * this will have to be checked and set in the code.
 * this is def for scanGroupID:
 *
typedef enum {
    ScanGroupScrap                = 1,
    ScanGroupSignature            = 4,
    ScanGroupShip                 = 8,
    ScanGroupStructure            = 16,
    ScanGroupDroneOrProbe         = 32,
    ScanGroupCelestial            = 64,
    ScanGroupAnomaly              = 128
} ScanGroup;
 *
 *  for strengthAttributeID, use these attributes to indicate site type:

 AttrScanRadarStrength = 208,
 AttrScanLadarStrength = 209,
 AttrScanMagnetometricStrength = 210,
 AttrScanGravimetricStrength = 211,
 AttrScanAllStrength = 1136     - unknown

    */

// this class is a singleton object to have a common place for all manager data
class MgrData
: public Singleton< MgrData >
{
public:
    MgrData();
    virtual ~MgrData() { /* nothing do to yet */ }

    // Initializes the Table:
    int Initialize();

    uint8 GetRegionQuarter(uint32 regionID);
    bool GetRoidDist(uint8& quarter, const char* sec, std::map<float, uint32>& roids);

protected:
    void _Populate();

private:
    std::map<uint32, uint32> m_regions;   // this simple map holds k,v of regionID/factionID
    std::map<std::string, OreBySSC> m_oreBySSC;
};

#define sMgrData \
    ( MgrData::get() )


class AsteroidData;
class ManagerDB {
public:

    /* db methods for all managers */
    void SaveAnomaly(CosmicSignature& sig);
    void GetAnomalyList(DBQueryResult& res);
    GPoint GetAnomalyPos(std::string& string);
    void GetSystemAnomalies(uint32 systemID, DBQueryResult& res);
    void GetSystemAnomalies(uint32 systemID, std::vector< CosmicSignature >& sigs);

    /* data manager */
    void GetOreBySSC(DBQueryResult& res);

    /* belt manager */
    void SaveSystemRoids(uint32 systemID, std::vector< AsteroidData >& roids);
    bool LoadSystemRoids(uint32 systemID, uint32& beltID, std::vector< AsteroidData >& into);
    void GetRegionFaction(DBQueryResult& res);

    /* spawn manager */
    void DeleteSpawnedRats();
    void GetSpawnClasses(DBQueryResult& res);
    void GetGroupTypeIDs(uint32 groupID, DBQueryResult& res);
    void GetFactionGroups(DBQueryResult& res);
    void GetRegionRatFaction(DBQueryResult& res);

    /* dungeon manager */
    void GetDunTemplates(DBQueryResult& res);
    void GetDunRoomInfo(DBQueryResult& res);
    void GetDunRoomData(DBQueryResult& res);
    void GetDunGroupData(DBQueryResult& res);
    void GetDunSpawnInfo(DBQueryResult& res);
    bool GetSavedDungeons(uint32 systemID, std::vector< ActiveDungeon >& into);
    void SaveActiveDungeon(ActiveDungeon& dun);
    void ClearDungeons();

    /* anomaly manager */

    /* wormhole manager */

protected:

private:


};



#endif  // _EVEMU_SYSTEM_COSMICMGRS_MANAGERDB_H