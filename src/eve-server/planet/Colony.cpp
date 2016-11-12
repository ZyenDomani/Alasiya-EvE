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
    Author:        Cometo
    Updates:    Allan
*/

#include "eve-server.h"
#include "Colony.h"
#include "PyServiceMgr.h"
#include "inventory/ItemType.h"
#include <Client.h>

Colony::Colony(PyServiceMgr* mgr, Client* pclient, uint32 pID)
:svcMgr(mgr),
m_client(pclient),
m_planetID(pID)
{
    ccPin = new CommandCenterPin();
    ccContents = new PyDict();
    //ccContents->SetItem(new PyInt(2268), new PyInt(1));  //Aqueous Liquids
}

Colony::~Colony()
{
    SafeDelete(ccPin);
}

void Colony::Init()
{
    // check for and load colony if the char has one on this planet
}

void Colony::Load()
{

}
void Colony::Save()
{

}

void Colony::AbandonColony()
{
    m_db.DeleteColony(m_colonyID);
    SafeDelete(ccPin);
    ccPin = new CommandCenterPin();
    ccContents = new PyDict();
    m_colonyID = 0;
}


bool Colony::CreateCommandPin(uint32 itemID, uint32 typeID, float latitude, float longitude) {
    /**
     * create itemID for Pin
     * ??
     */
    InventoryItemRef iRef = svcMgr->item_factory->GetItem(itemID);
    m_client->GetShip()->RemoveItem(iRef);
    iRef->Delete();
    m_colonyID = m_db.MakeCommandCenter(m_client->GetCharacterID(), m_planetID, typeID, latitude, longitude);
    Pin cc;
        cc.id = m_colonyID;
        cc.typeID = typeID;
        cc.latitude = latitude;
        cc.longitude = longitude;
        cc.ownerID = m_client->GetCharacterID();
        cc.state = PINSTATE_IDLE;
        cc.lastRunTime = 0;
        cc.lastLaunchTime = 0;
        cc.isLaunchable = true;
        cc.isCommandCenter = true;
    ccPin->level = 0;
    ccPin->currentSimTime = Win32TimeNow();
    ccPin->pins.push_back(cc);
    return true;
}

bool Colony::CreatePin(uint32 pinID, uint32 typeID, float latitude, float longitude) {
    uint32 groupID = svcMgr->item_factory->GetType(typeID)->groupID();
    Pin pin;
        pin.id = pinID;
        pin.typeID = typeID;
        pin.latitude = latitude;
        pin.longitude = longitude;
        pin.ownerID = m_client->GetCharacterID();
        pin.state = PINSTATE_IDLE;
        pin.lastRunTime = 0;

    switch(groupID) {
        case  EVEDB::invGroups::Processors: {
            pin.isProcess = true;
            pin.schematicID = 0;
            pin.hasRecievedInputs = 0;
            pin.recievedInputsLastCycle = 0;
        } break;
        case EVEDB::invGroups::Extractor_Control_Units: {
            pin.heads = 0;
            pin.programType = 0;
            pin.cycleTime = 0;
            pin.expiryTime = 0;
            pin.qtyPerCycle = 0;
            pin.headRadius = 0.0;
            pin.installTime = 0;
        } break;
        case EVEDB::invGroups::Spaceports: {
            pin.isLaunchable = true;
            pin.lastLaunchTime = 0;
        } break;
        case EVEDB::invGroups::Extractors: {
            pin.isExtractor = true;
            /* nothing to do yet */
        } break;
        case EVEDB::invGroups::Planetary_Links: {
            /* nothing to do yet */
        } break;
        case EVEDB::invGroups::Storage_Facilities: {
            /* nothing to do yet */
        } break;
    }

    ccPin->pins.push_back(pin);
    return true;
}

bool Colony::CreateLink(uint32 src, uint32 dest, uint32 level, bool ccConnected) {
    Link link;
        link.level = level;
        link.endpoint1 = src;
        link.endpoint2 = dest;
        link.typeID = 2280; // Only link type in the game.
        link.commandCenterConnected = ccConnected;
    ccPin->links.push_back(link);
    return true;
}

void Colony::UpgradeCommandCenter(uint32 pinID, uint32 level) {
    ccPin->level = level;
}

bool Colony::UpgradeLink(uint32 src, uint32 dest, uint32 level, bool ccConnected) {
    bool rtn = false;
    for (int i = 0; i < ccPin->links.size(); i++) {
        Link tmp = ccPin->links.front();
        ccPin->links.pop_front();
        if (((tmp.endpoint1 == src) or (tmp.endpoint1 == dest)) and ((tmp.endpoint2 == src) or (tmp.endpoint2 == dest))) {
            rtn = true;
            tmp.level = level;
        }
        ccPin->links.push_back(tmp);
    }
    return rtn;
}

bool Colony::RemovePin(uint32 pinID) {
    bool rtn = false;
    for (int i = 0;i < ccPin->pins.size();i++) {
        Pin tmp = ccPin->pins.front();
        ccPin->pins.pop_front();
        if (tmp.id != pinID)
            ccPin->pins.push_back(tmp);
        else
            rtn = true;
    }
    return rtn;
}

bool Colony::RemoveLink(uint32 src, uint32 dest, bool ccConnected) {
    bool rtn = false;
    for (int i = 0; i < ccPin->links.size(); i++) {
        Link tmp = ccPin->links.front();
        ccPin->links.pop_front();
        if (((tmp.endpoint1 == src) or (tmp.endpoint1 == dest)) and ((tmp.endpoint2 == src) or (tmp.endpoint2 == dest))) {
            rtn = true;
            continue;
        }
        ccPin->links.push_back(tmp);
    }
    return rtn;
}

PyRep* Colony::GetColony() {
    PyTuple *pins = new PyTuple(ccPin->pins.size());
    PyTuple *links = new PyTuple(ccPin->links.size());
    PyTuple *routes = new PyTuple(ccPin->routes.size());
    int index = 0; // used by each for loop.

    for (auto i : ccPin->pins) {
        PyDict *dict = new PyDict();
        if (i.isCommandCenter)
            dict->SetItem("id", new PyInt(i.id));
        else
            dict->SetItem("id", new_tuple(m_colonyID, i.id));
        dict->SetItem("typeID", new PyInt(i.typeID));
        dict->SetItem("ownerID", new PyInt(i.ownerID));
        dict->SetItem("latitude", new PyFloat(i.latitude));
        dict->SetItem("longitude", new PyFloat(i.longitude));
        dict->SetItem("lastRunTime", new PyULong(i.lastRunTime));
        dict->SetItem("state", new PyInt(i.state));
        dict->SetItem("contents", ccContents);

        if (i.isLaunchable) {
            dict->SetItem("lastLaunchTime", new PyULong(i.lastLaunchTime));
        } else if (i.isProcess) {
            dict->SetItem("schematicID", new PyInt(i.schematicID));
            dict->SetItem("hasRecievedInputs", new PyBool(i.hasRecievedInputs));
            dict->SetItem("recievedInputsLastCycle", new PyBool(i.recievedInputsLastCycle));
        } else if (i.isExtractor) {
            dict->SetItem("installTime", new PyULong(i.installTime));
            dict->SetItem("programType", new PyInt(i.programType));
            dict->SetItem("qtyPerCycle", new PyInt(i.qtyPerCycle));
            dict->SetItem("headRadius", new PyFloat(i.headRadius));
            dict->SetItem("expiryTime", new PyULong(i.expiryTime));
            dict->SetItem("cycleTime", new PyInt(i.cycleTime));
            dict->SetItem("heads", new PyInt(i.heads));
        }

        PyObject *obj = new PyObject("util.KeyVal", dict);
        pins->SetItem(index++, obj);
    }
    index = 0;

    for (auto i : ccPin->links) {
        PyDict *dict = new PyDict();

        if (i.commandCenterConnected)
            dict->SetItem("endpoint1", new PyInt(i.endpoint1));
        else
            dict->SetItem("endpoint1", new_tuple(1, i.endpoint1));
        dict->SetItem("endpoint2", new_tuple(1, i.endpoint2));
        dict->SetItem("level", new PyInt(i.level));
        dict->SetItem("typeID", new PyInt(i.typeID));

        PyObject *obj = new PyObject("util.KeyVal", dict);
        links->SetItem(index++, obj);
    }
    index = 0;

    for (auto i : ccPin->routes) {
        PyDict *dict = new PyDict();

        if (i.destIsCommandCenter)
            dict->SetItem("destID", new PyInt(i.destID));
        else
            dict->SetItem("destID", new_tuple(1, i.destID));
        dict->SetItem("comodityTypeID", new PyInt(i.comodityTypeID));
        dict->SetItem("commodityQuantity", new PyInt(i.commodityQuantity));

        PyObject *obj = new PyObject("util.KeyVal", dict);
        routes->SetItem(index++, obj);
    }

    PyDict *args = new PyDict();
        args->SetItem("level", new PyInt(ccPin->level));
        args->SetItem("pins", pins);
        args->SetItem("links", links);
        args->SetItem("routes", routes);
        args->SetItem("currentSimTime", new PyULong(ccPin->currentSimTime));
    PyObject *rtn = new PyObject("util.KeyVal", args);

    _log(PLANET__DEBUG, "Colony::GetColony() Dump");
    rtn->Dump(PLANET__GC_DUMP, "    ");
    return rtn;
}
