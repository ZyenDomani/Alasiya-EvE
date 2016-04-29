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
    Author:        Aknor Jaden
*/

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "system/LootSystem.h"


// ////////////////////// DGM_Types_to_Wrecks_Table Class ////////////////////////////
DGM_Types_to_Wrecks_Table::DGM_Types_to_Wrecks_Table()
{
    m_WrecksToTypesMap.clear();
}

DGM_Types_to_Wrecks_Table::~DGM_Types_to_Wrecks_Table()
{
}

int DGM_Types_to_Wrecks_Table::Initialize()
{
    _Populate();

    return 1;
}

void DGM_Types_to_Wrecks_Table::_Populate()
{
    double start = GetTimeUSeconds();
    uint32 wreckID, typeID;

    //first get list of all effects from dgmEffects table
    DBQueryResult *res = new DBQueryResult();
    SystemDB::GetWrecksToTypes(*res);

	//go through and populate each effect
    DBResultRow row;
    while( res->GetRow(row) )
    {
        typeID = row.GetInt(0);
        wreckID = row.GetInt(1);
		m_WrecksToTypesMap.insert(std::pair<uint32, uint32>(typeID,wreckID));
    }

    sLog.Log("     Wrecks Table", "%u wreck objects loaded in %.3fms.",
             m_WrecksToTypesMap.size(), (GetTimeUSeconds() - start));

    //cleanup
    delete res;
    res = NULL;
}

uint32 DGM_Types_to_Wrecks_Table::GetWreckID(uint32 typeID)
{
    std::map<uint32, uint32>::iterator mWrecksMapIterator;

    if( (mWrecksMapIterator = m_WrecksToTypesMap.find(typeID)) == m_WrecksToTypesMap.end() )
        return 0;
    else
    {
        return mWrecksMapIterator->second;
    }
}



//////////////////////// DGM_Loot_Groups_Table Class ////////////////////////////
//  Author:     Allan

DGM_Loot_Groups_Table::DGM_Loot_Groups_Table()
{
    m_LootGroupMap.clear();
    m_LootGroupTypeMap.clear();
}

DGM_Loot_Groups_Table::~DGM_Loot_Groups_Table()
{
    m_LootGroupMap.clear();
    m_LootGroupTypeMap.clear();
}

int DGM_Loot_Groups_Table::Initialize()
{
    _Populate();
    return 1;
}

void DGM_Loot_Groups_Table::_Populate()
{
    double start = GetTimeUSeconds();
    DBQueryResult* res = new DBQueryResult();

    //first get all loot groups from LootGroup table
    m_db.GetLootGroups(*res);
    DBResultRow row;
    DBLootGroup LootGroup;
    while( res->GetRow(row) ) {
        //SELECT npcGroupID, itemGroupID, groupDropChance FROM lootGroup
        //LootGroup.groupID = row.GetInt(0);
        LootGroup.lootGroupID = row.GetInt(1);
        LootGroup.dropChance = row.GetDouble(2);
        m_LootGroupMap.emplace(row.GetInt(0), LootGroup);
    }

    res->Reset();

    //second get all types from LootGroupTypes table
    m_db.GetLootGroupTypes(*res);
    DBLootGroupType GroupType;
    while( res->GetRow(row) ) {
        //SELECT itemGroupID, itemID, itemMetaLevel, minAmount, maxAmount FROM lootItemGroup
        GroupType.lootGroupID = row.GetInt(0);
        GroupType.typeID =  row.GetInt(1);
        GroupType.metaLevel = row.GetInt(2);
        GroupType.minQuantity = row.GetInt(3);
        GroupType.maxQuantity = row.GetInt(4);
        m_LootGroupTypeMap.emplace(row.GetInt(0), GroupType);
    }

    //cleanup
    SafeDelete(res);

    sLog.Log("       Loot Table", "%u loot group buckets and %u definitions loaded in %.3fms.",
             (m_LootGroupMap.bucket_count() + m_LootGroupTypeMap.bucket_count()),
             (m_LootGroupMap.size() + m_LootGroupTypeMap.size()),
             (GetTimeUSeconds() - start));
}

void DGM_Loot_Groups_Table::GetLoot(uint32 groupID, LootListDef &lootList) {
    double profileStartTime = 0.0;
    if (sConfig.server.UseProfiling) profileStartTime = GetTimeUSeconds();

    double randChance = 0.0;
    uint8 metaLevel = 0;
    LootGroupTypeVec loot_group_list;
    LootList loot_list;

    // Finds a range containing all elements whose key is k.
    // pair<iterator, iterator> equal_range(const key_type& k)
    auto range = m_LootGroupMap.equal_range(groupID);
    for ( auto it = range.first; it != range.second; it++ ) {
        // make lootMap of lootGroupID's
        if (MakeRandomFloat(0, 1) < it->second.dropChance){
            randChance = MakeRandomFloat(0, 1);
            metaLevel = 0;
            if (randChance < 0.1) metaLevel = 4;
            else if (randChance < 0.25) metaLevel = 3;
            else if (randChance < 0.5) metaLevel = 2;
            else if (randChance < 0.6) metaLevel = 1;
            /*need to figure out how to get faction loot for faction wrecks
    elif meta_level == 7:
        drop_chance = 0.15   # Faction stuff = 15%
    elif meta_level == 8:
        drop_chance = 0.15   # Faction projectiles = 15%
    elif meta_level == 9:
        drop_chance = 0.15   # Faction SB's and Missile launchers
            */

            auto range2 = m_LootGroupTypeMap.equal_range(it->second.lootGroupID);
            for (auto it2 = range2.first; it2 != range2.second; it2++) {
                if (it2->second.metaLevel == metaLevel)
                    loot_group_list.push_back(it2->second);
            }

            if (!loot_group_list.empty()) {
                uint16 i = MakeRandomInt(0, loot_group_list.size());
                loot_list.itemID = loot_group_list[i].typeID;
                loot_list.minDrop = loot_group_list[i].minQuantity;
                loot_list.maxDrop = loot_group_list[i].maxQuantity;
                lootList.push_back(loot_list);
                loot_group_list.clear();
            }
        }
    }

    if (sConfig.server.UseProfiling) sProfile.AddTime(_lootProfile, GetTimeUSeconds() - profileStartTime);
}

// ////////////////////// DGM_Salvage_Table Class ////////////////////////////
//  Author:     Allan

DGM_Salvage_Table::DGM_Salvage_Table()
{
    m_SalvageMap.clear();
}

DGM_Salvage_Table::~DGM_Salvage_Table()
{
    m_SalvageMap.clear();
}

int DGM_Salvage_Table::Initialize()
{
    _Populate();
    return 1;
}

void DGM_Salvage_Table::_Populate()
{
    double start = GetTimeUSeconds();
    DBQueryResult* res = new DBQueryResult();

    //get all groups from salvage table
    m_db.GetSalvageGroups(*res);
    DBResultRow row;
    DBSalvageGroup salvage;
    while( res->GetRow(row) ) {
        //salvage.wreckTypeID = row.GetInt(0);
        salvage.salvageItemID = row.GetInt(1);
        salvage.groupID = row.GetInt(2);
        salvage.dropChance = row.GetDouble(3);
        salvage.minDrop = row.GetInt(4);
        salvage.maxDrop = row.GetInt(5);
        m_SalvageMap.emplace(row.GetInt(0), salvage);
    }

    //cleanup
    SafeDelete(res);

    sLog.Log("    Salvage Table", "%u salvage definitions loaded in %.3fms.",
             m_SalvageMap.size(), (GetTimeUSeconds() - start));
}

void DGM_Salvage_Table::GetSalvage(uint32 wreckTypeID, LootListDef &salvageList) {
    // currently disabled as no salvage module is working yet.
    //TODO  finish later.
    double start = GetTimeUSeconds();
    double profileStartTime = 0.0;
    if (sConfig.server.UseProfiling)
        profileStartTime = GetTimeUSeconds();
    double randChance = 0.0;

    LootList loot_list1;
/*
    //SalvageItr curGroupItr = m_SalvageMap.begin();

    while (curGroupItr != m_SalvageMap.end()) {
        if (curGroupItr->wreckTypeID == wreckTypeID) {
            randChance = gen_random_float(0.00, 0.30);      // FIXME adjust this later...use a config var maybe?  -used to determine initial loot groups
            if (randChance < curGroupItr->dropChance) {
                loot_list1.itemID = curGroupItr->salvageItemID;
                loot_list1.minDrop = curGroupItr->minDrop;
                loot_list1.maxDrop = curGroupItr->maxDrop;
                salvageList.push_back(loot_list1);
            }
        }
        ++curGroupItr;
    }
    */

    if (sConfig.server.UseProfiling)
        sProfile.AddTime(_salvageProfile, GetTimeUSeconds() - profileStartTime);
    sLog.Log("     GetSalvage()", "Took %fus to iterate thru %u loops, with %u items returned",
             (GetTimeUSeconds() - start), m_SalvageMap.size(), salvageList.size());

}
