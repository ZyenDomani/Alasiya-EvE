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


#ifndef EVEMU_PLANET_PLANETDB_H_
#define EVEMU_PLANET_PLANETDB_H_

#include "../eve-server.h"
#include "POD_containers.h"

class CommandCenterPin;
class PlanetDB
{
public:
    PyRep* GetPlanetsForChar(uint32 charID);
    PyRep* GetMyLaunchesDetails(uint32 charID);

    GPoint GetLaunchPos(uint32 launchID);

    void SavePins(PI_CCPin* ccPin);
    void SaveHeads(std::map< uint32, PI_Heads >& heads);
    void SaveLinks(PI_CCPin* ccPin);
    void SaveRoutes(PI_CCPin* ccPin);
    void RemovePin(uint32 pinID);
    void RemoveHead(uint32 pinID);
    void RemoveLink(uint32 linkID);
    void RemoveRoute(uint8 routeID);
    void DeleteColony(uint32 ccPinID, uint32 planetID, uint32 charID);
    void LoadPins(uint32 ccPinID, std::map<uint32, PI_Pin>& pins);
    void LoadLinks(uint32 ccPinID, std::map<uint32, PI_Link>& links);
    void LoadRoutes(uint32 ccPinID, std::map<uint32, PI_Route>& routes);
    void SaveCCLevel(uint32 pinID, uint8 level);
    void SavePinLevel(uint32 pinID, uint8 level);
    void SaveLinkLevel(uint32 linkID, uint8 level);
    void GetPlanetData(DBQueryResult& row);
    void SaveCommandCenter(uint32 pinID, uint32 charID, uint32 planetID, uint32 typeID, float latitude, float longitude);
    void GetExtractorsForPlanet(uint32 planetID, DBQueryResult& res);

    void SaveLaunch(uint32 charID, uint32 systemID, uint32 planetID, GPoint& pos);
    void UpdatePlanetsForChar(uint32 solarSystemID, uint32 planetID, uint32 charID, uint16 typeID, uint8 pins=1);

    bool LoadColony(uint32 charID, uint32 planetID, PI_CCPin* ccPin);

};

#endif  // EVEMU_PLANET_PLANETDB_H_
