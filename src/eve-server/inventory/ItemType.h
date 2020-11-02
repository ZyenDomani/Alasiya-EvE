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
    Author:        Bloody.Rabbit
*/

#ifndef __ITEM_TYPE__H__INCL__
#define __ITEM_TYPE__H__INCL__

#include <unordered_map>

#include "POD_containers.h"
#include "effects/EffectsData.h"
#include "inventory/AttributeMap.h"
#include "inventory/ItemFactory.h"

/*
 * LOADING INVOKATION EXPLANATION:
 * ItemType and InventoryItem classes and their children have special loading. Every such type has following methods:
 *
 *  static Load( <identifier>):
 *    Merges static and virtual loading trees.
 *    First calls static _Load() to create desired object and
 *    then calls its virtual _Load() (if the type has any).
 *
 *  static _Load( <identifier>[, <data-argument>, ...]):
 *    These functions gradually, one by one, load any data needed to create desired
 *    type and in the end they create the type object.
 *
 *  virtual _Load(ItemFactory &factory) (optional):
 *    Performs any post-construction loading.
 */

/*
 * Simple container for raw type data.
 */
class TypeData {
public:
    TypeData(
        uint16 _groupID = 0,
        const char *_name = "",
        const char *_desc = "",
        double _radius = 0.0,
        double _mass = 0.0,
        double _volume = 0.0,
        double _capacity = 0.0,
        uint32 _portionSize = 0,
        uint8 _race = 0,
        double _basePrice = 0.0,
        bool _published = false,
        uint32 _marketGroupID = 0,
        double _chanceOfDuplicating = 0.0
    );

    // Content:
    uint16 groupID;
    std::string name;
    std::string description;
    double radius;
    double mass;
    double volume;
    double capacity;
    uint32 portionSize;
    uint8 race;
    double basePrice;
    bool published;
    uint32 marketGroupID;
    double chanceOfDuplicating;
};

/*
 * Class which maintains type data.
 */
class ItemType {
public:

    /* Helper methods  */
    uint16 id() const                                   { return m_id; }
    uint16 groupID() const                              { return m_group.id; }
    const std::string &groupName() const                { return m_group.name; }
    uint8 categoryID() const                            { return m_group.catID; }

    const std::string &name() const                     { return m_name; }
    const std::string &description() const              { return m_description; }
    uint32 portionSize() const                          { return m_portionSize; }
    double basePrice() const                            { return m_basePrice; }
    bool published() const                              { return m_published; }
    uint32 marketGroupID() const                        { return m_marketGroupID; }
    double chanceOfDuplicating() const                  { return m_chanceOfDuplicating; }

    double radius() const                               { return m_radius; }
    double mass() const                                 { return m_mass; }
    double volume() const                               { return m_volume; }
    double capacity() const                             { return m_capacity; }
    uint8 race() const                                  { return m_raceID; }

    /* new attribute system */
    const bool HasAttribute(const uint16 attributeID) const;
    EvilNumber GetAttribute(const uint16 attributeID) const;
    const void CopyAttributes(InventoryItem& itemRef) const;

    /* new effects processing system */
    void GetEffectMap(const int8 state, std::map<uint16, Effect>& effectMap) const;
    uint16 GetDefaultEffect() const                     { return m_defaultID; }

    bool HasEffect(uint16 effectID) const;
    bool HasReqSkill(const uint16 skillID) const;

    // load method
    static ItemType* Load( uint16 typeID);

protected:
    ItemType(uint16 _id, const TypeData &_data);

    /*
     * Member functions
     */
    // Template helper:
    template<class _Ty>
    static _Ty *Load( uint16 typeID)
    {
        // static load
        _Ty *t = _Ty::template _Load<_Ty>(typeID );
        if (t == nullptr)
            return nullptr;

        // dynamic load
        if (!t->_Load()) {
            delete t;
            return nullptr;
        }

        return t;
    }

    // Template loader:
    template<class _Ty>
    static _Ty *_Load( uint16 typeID)
    {
        // pull data
        TypeData data;
        if( !sItemFactory.db()->GetType( typeID, data ) )
            return nullptr;
        /** @todo  this needs work.  other items we need are "non-published" */
        if (data.groupID > 23)  // gID < 23 are map items.  will need to search for others
            if (!data.published)
                return nullptr;

        return _Ty::template _LoadType<_Ty>(typeID, data );
    }

    // Actual loading stuff:
    template<class _Ty>
    static _Ty *_LoadType( uint16 typeID, const TypeData &data);

    virtual bool _Load();

    void LoadEffects();

public:
    // i dont like this......MUST fix later....
    // UD: ive no clue what i didnt like about it
    std::unordered_multimap<int8, Effect> m_stateFxMap; // k,v map of state, data   -to search by state

private:
    Inv::GrpData m_group;
    bool m_published;
    uint8 m_raceID;
    const uint16 m_id;
    uint16 m_defaultID;                 // default effectID
    uint32 m_portionSize;
    uint32 m_marketGroupID;
    double m_basePrice;
    double m_chanceOfDuplicating;
    double m_radius;
    double m_mass;
    double m_volume;
    double m_capacity;
    std::string m_name;
    std::string m_description;

    std::map<uint16, uint8> m_reqSkillMap;              // k,v map of required skill, level for this ItemType, if any.
    std::map<uint16, EvilNumber> m_AttributeMap;        // k,v map of attributeID, value

};

/*
 * Simple container for raw item data.
 */
class ItemData {
public:
    // Full + default constructor:
    ItemData( const char *_name = "", uint16 _typeID = 0, uint32 _ownerID = 0, uint32 _locationID = 0,
              EVEItemFlags _flag = flagAutoFit, bool _contraband = false, bool _singleton = false, uint32 _quantity = 0,
              const GPoint &_position = NULL_ORIGIN, const char *_customInfo = "");

    // Item friendly constructor:
    ItemData( uint16 _typeID, uint32 _ownerID, uint32 _locationID, EVEItemFlags _flag, uint32 _quantity,
              const char *_customInfo = "", bool _contraband = false);

    // Singleton friendly constructor:
    ItemData( uint16 _typeID, uint32 _ownerID, uint32 _locationID, EVEItemFlags _flag, const char *_name = "",
              const GPoint &_position = NULL_ORIGIN, const char *_customInfo = "", bool _contraband = false);

    // Content:
    bool            contraband :1;
    bool            singleton :1;
    EVEItemFlags    flag;
    uint16          typeID;
    uint32          ownerID;
    uint32          locationID;
    uint32          quantity;
    GPoint          position;
    std::string     name;
    std::string     customInfo;
};

#endif /* __ITEM_TYPE__H__INCL__ */



