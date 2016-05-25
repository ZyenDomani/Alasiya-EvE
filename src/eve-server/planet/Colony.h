/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2011 The EVEmu Team
 *    For the latest information visit http://evemu.org
 *    ------------------------------------------------------------------------------------
 *    This program is free software; you can redistribute it and/or modify it under
 *    the terms of the GNU Lesser General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option) any later
 *    version.
 *
 *    This program is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License along with
 *    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 *    http://www.gnu.org/copyleft/lesser.txt.
 *    ------------------------------------------------------------------------------------
 *    Author:        Cometo
 */

#ifndef __COLONY_H_INCL__
#define __COLONY_H_INCL__

#include "PyCallable.h"

class Colony {

public:
    Colony(PyServiceMgr* mgr, uint32 charID, uint32 planetID);

    void Init();
    void Load();
    void Save();

    void UpgradeCommandCenter(uint32 pinID, uint32 level);

    bool CreatePin(uint32 pinID, uint32 typeID, float latitude, float longitude);
    bool RemovePin(uint32 pinID);
    bool CreateLink(uint32 src, uint32 dest, uint32 level, bool ccConnected);
    bool RemoveLink(uint32 src, uint32 dest, bool ccConnected);
    bool UpgradeLink(uint32 src, uint32 dest, uint32 level, bool ccConnected);
    bool CreateCommandPin(uint32 pinID, uint32 typeID, float latitude, float longitude);

    PyResult GetColony();

protected:
    struct Pin {
        int8 state = 0;

        uint32 id = 0;
        uint32 typeID = 0;
        uint32 ownerID = 0;

        float latitude = 0.0f;
        float longitude = 0.0f;

        uint64 lastRunTime = 0;

        bool isCommandCenter = false;
        bool isLaunchable = false;
        bool isProcess = false;
        bool isExtractor = false;

        // Command/Spaceport
        uint64 lastLaunchTime = 0;

        // Process
        uint32 schematicID = 0;
        bool hasRecievedInputs = false;
        bool recievedInputsLastCycle = false;

        //Extractor
        uint8 heads = 0;
        float headRadius = 0.0f;
        // -program data
        uint32 cycleTime = 0;
        uint32 programType = 0;
        uint32 qtyPerCycle = 0;
        uint64 expiryTime = 0;
        uint64 installTime = 0;
    };

    struct Link {
        bool commandCenterConnected;
        uint32 level;
        uint32 typeID;
        uint32 endpoint1;
        uint32 endpoint2;
    };

    struct Route {
        bool destIsCommandCenter;

        uint32 destID;
        uint32 comodityTypeID;
        uint32 commodityQuantity;
    };

    struct CommandCenterPin {
        uint32 level;
        uint64 currentSimTime;

        std::list<Pin> pins;
        std::list<Link> links;
        std::list<Route> routes;
    };


private:
    PyServiceMgr* svcMgr;
    PyDict* testContainer;

    uint32 charID;
    uint32 colonyID;
    uint32 planetID;

    CommandCenterPin ccPin;

    const int STATE_EDITMODE = -2;
    const int STATE_DISABLED = -1;
    const int STATE_IDLE = 0;
    const int STATE_ACTIVE = 1;

    /* event
     * STATE_NORMAL = 0
     * STATE_BUILDPIN = 1
     * STATE_CREATELINKSTART = 2
     * STATE_CREATELINKEND = 3
     * STATE_CREATEROUTE = 4
     * STATE_SURVEY = 5
     * SUBSTATE_NORMAL = 0
     * SUBSTATE_MOVEEXTRACTIONHEAD = 1
     */
};


#endif

