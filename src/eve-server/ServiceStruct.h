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
    Author:      Allan
*/

#ifndef EVE_SERVER_SERVICESTRUCT_H__
#define EVE_SERVER_SERVICESTRUCT_H__

#include "eve-server.h"


/* structs for system, station, and static items.
 *  this is to avoid db hits EVERYTIME the shit is called...which is often (jump, login, dock, undock)
 *  -allan 1July15
 *
 * this group of methods may become complicated.
 * i'll just have to see how it works out.
 *
 */

struct SystemInfo
{
    uint32 constellationID;
    uint32 regionID;
    float securityRating;
    std::string name;
    std::string securityClass;
};

struct StaticInfo
{
    uint32 systemID;
    uint32 constellationID;
    uint32 regionID;
    GPoint position;
};

struct StationInfo
{
    uint32 systemID;
    uint32 constellationID;
    uint32 regionID;
    GPoint position;
    GPoint dockPosition;
    GVector dockOrientation;
};

class ServiceStruct
: public Singleton< ServiceStruct >
{
public:
    ServiceStruct();
    ~ServiceStruct();

    void Init() { }

private:
    std::map<uint32, SystemInfo> m_systemInfo;
    std::map<uint32, StaticInfo> m_staticInfo;
    std::map<uint32, StationInfo> m_stationInfo;
};

//Singleton
#define sServiceStruct \
    ( ServiceStruct::get() )


#endif  // EVE_SERVER_SERVICESTRUCT_H__
