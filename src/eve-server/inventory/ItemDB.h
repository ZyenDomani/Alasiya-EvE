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

#ifndef EVE_INVENTORY_ITEM_DB
#define EVE_INVENTORY_ITEM_DB

#include "ServiceDB.h"
#include "inventory/ItemType.h"

/* POD structure for saving items */
struct SaveData {
    bool            contraband :1;
    bool            singleton :1;
    EVEItemFlags    flag;
    uint16          typeID;
    uint32          itemID;
    uint32          ownerID;
    uint32          locationID;
    uint32          quantity;
    GPoint          position;
    std::string     customInfo;
};

/* POD structure for saving attribute data */
struct AttrData {
    uint16 attrID;
    uint32 itemID;
    int64 valueInt;
    double valueFloat;
};


class ItemDB
{
public:
    // get item data based on itemID
    static bool GetItem(uint32 itemID, ItemData &into);
    static bool DeleteItem(uint32 itemID);

    static void UpdateLocation(uint32 itemID, uint32 locationID, EVEItemFlags flag);

    static uint32 NewItem(const ItemData &data);

    static bool SaveItem(uint32 itemID, const ItemData &data);
    static void SaveItems(std::vector< SaveData > &data);
    static void SaveAttributes(bool isChar, std::vector< AttrData > &data);

    static void GetItems(uint16 catID, std::map<uint16, std::string> &typeIDs);

};


#endif  // EVE_INVENTORY_ITEM_DB
