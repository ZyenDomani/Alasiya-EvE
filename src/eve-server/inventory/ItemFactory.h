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
#ifndef EVE_ITEM_FACTORY_H
#define EVE_ITEM_FACTORY_H

//#include "eve-compat.h"

/** @todo look into making this a singleton to avoid multiple redirection calls when factory is needed (and it's single-instance code) */

#include "inventory/InventoryDB.h"

class ItemCategory;
class ItemGroup;
class ItemType;
class BlueprintType;
class CharacterType;
class ShipType;
class StationType;
class Missile;
class Client;
class EntityList;
class Inventory;

class ItemFactory
{
    friend class InventoryItem;    //only for access to _DeleteItem
public:
    ItemFactory();
    ~ItemFactory();

    void SaveItems();
    void RemoveItem(uint32 itemID);
    void SetUsingClient(Client *pClient)                { m_pClient = pClient; }
    void UnsetUsingClient()                             { m_pClient = nullptr; }

    uint32 Count()                                      { return m_itemCount; }

    InventoryDB& db()                                   { return m_db; }

    Client* GetUsingClient()                            { return m_pClient; }
    Inventory* GetInventoryFromId(uint32 inventoryID, bool load=true);

    /**
     * these load type, cache it and return it.
     *
     * @param[in] typeID  type to be returned.
     * @return Pointer to type data container; NULL if fails.
     */
    const ItemCategory*     GetCategory(EVEItemCategories category);
    const ItemGroup*        GetGroup(uint32 groupID);
    const ItemType*         GetType(uint32 typeID);
    const ShipType*         GetShipType(uint32 shipTypeID);
    const StationType*      GetStationType(uint32 stationTypeID);
    const CharacterType*    GetCharacterType(uint32 characterTypeID);
    const BlueprintType*    GetBlueprintType(uint32 blueprintTypeID);
    const CharacterType*    GetCharacterTypeByBloodline(uint32 bloodlineID);


    /**
     * these load an InventoryItem of requested type and returns a RefPtr.
     *
     * @param[in]  ID of _item to load.
     * @return RefPtr to _Ty; NULL if load failed.
     */
    SkillRef                GetSkill(uint32 skillID);
    ShipItemRef             GetShip(uint32 shipID);
    StationItemRef          GetStation(uint32 stationID);
    BlueprintRef            GetBlueprint(uint32 blueprintID);
    CharacterRef            GetCharacter(uint32 characterID);
    SolarSystemRef          GetSolarSystem(uint32 solarSystemID);
    //AsteroidItemRef         GetAsteroid(uint32 asteroidID);
    StructureItemRef        GetStructure(uint32 structureID);
    InventoryItemRef        GetItem(uint32 itemID);
    InventoryItemRef        GetItemContainer(uint32 itemID, bool load=true);
    InventoryItemRef        GetInventoryItemFromID(uint32 itemID, bool load=true);
    CargoContainerRef       GetCargoContainer(uint32 containerID);
    WreckContainerRef       GetWreckContainer(uint32 containerID);
    CelestialObjectRef      GetCelestialObject(uint32 celestialID);


    /**
     * creates new InventoryItem, saves to db, caches it and returns reference.
     *
     * @param[in] data Item data (for entity table).
     * @param[in] charData Character data.
     * @param[in] appData Character's appearance.
     * @param[in] corpData Character's corporation-membership data.
     * @return RefPtr to _Ty; NULL if load failed.
     */
    SkillRef                SpawnSkill(ItemData &data);
    ShipItemRef             SpawnShip(ItemData &data);
    CharacterRef            SpawnCharacter(ItemData &data, CharacterData &charData, CorpData &corpData);
    BlueprintRef            SpawnBlueprint(ItemData &data, BlueprintData &bpData);
    InventoryItemRef        SpawnItem(ItemData &data);
    //AsteroidItemRef         SpawnAsteroid(ItemData &idata, AsteroidData& aData);
    StructureItemRef        SpawnStructure(ItemData &data);
    CargoContainerRef       SpawnCargoContainer(ItemData &data);
    WreckContainerRef       SpawnWreckContainer(ItemData &data);
    /** @todo  add PI item spawners here */

    /*
	 * ID Authority Functions:
     */
    uint32                  GetNextEntityID();
    uint32                  GetNextAsteroidID();
    uint32                  GetNextMissileID();
    uint32                  GetNextNPCID();


protected:
    InventoryDB m_db;
    Client* m_pClient;     // pointer to client currently using the ItemFactory, we do not own this

    std::map<EVEItemCategories, ItemCategory*> m_categories;
    std::map<uint32, ItemGroup*> m_groups;
    std::map<uint32, ItemType*> m_types;
    std::map<uint32, InventoryItemRef> m_items;

    template<class _Ty>
    const _Ty *_GetType(uint32 typeID);

    template<class _Ty>
    RefPtr<_Ty> _GetItem(uint32 itemID);

private:
    // ID Authority:
    // these hold the next valid ID for in-memory only objects
    static uint32 m_nextEntityID;
    static uint32 m_nextAsteroidID;
    static uint32 m_nextMissileID;
    static uint32 m_nextNPCID;

    //item to hold current number of currently loaded items
    uint32 m_itemCount;
};

#endif

