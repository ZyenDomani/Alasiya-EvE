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
    Author:     Zhur
*/

#ifndef __EVE_ATTRIBUTE_MGR__H__INCL__
#define __EVE_ATTRIBUTE_MGR__H__INCL__

/*
 * This file contains EVE-specific derivations of attribute managers.
 * it is also incomplete.  revisit and update as needed.
 */
/** @todo finish these classes */

#include "inventory/AttributeMgr.h"
#include "ship/dgmtypeattributeinfo.h"

class PyRep;
class PyTuple;
class ItemType;
class ItemFactory;
class InventoryItem;
class InventoryDB;

class ItemAttributeMgr;

/**
 * Base EVE AttributeManager, defines types to use.
 */
class EVEAttributeMgr : virtual public AttributeMgr<int, double>
{
public:
    /**
     * Returns attribute value as PyRep.
     *
     * @param[in] attr Attribute which value should be retrieved.
     * @return Pointer to new PyRep object; NULL if fails.
     */
    PyRep *PyGet(Attr attr) const;

    /**
     * Builds Int dictionary from attributes.
     *
     * @param[in] into Int dictionary into which values are saved.
     */
    virtual void EncodeAttributes(std::map<int32, PyRep *> &into) const;

    /**
     * Checks whether the attribute is persistent.
     *
     * @param[in] attr Attribute to be checked.
     * @return True if attribute is persistent, false if not.
     */
    static bool IsPersistent(Attr attr) {
        _LoadPersistent();
        return m_persistent[attr];
    }

protected:
    // Turns given value into proper PyRep
    static PyRep *_PyGet(const real_t &v);

    /*
     * Persistent stuff
     */
    static void _LoadPersistent();

    static bool m_persistentLoaded;
    static bool m_persistent[Invalid_Attr];
};

/**
 * Base EVE AdvancedAttributeMgr, defines type to use.
 */
class EVEAdvancedAttributeMgr : public AdvancedAttributeMgr<int, double>, public EVEAttributeMgr
{
public:
    // Uses PyGet instead of _PyGet to include income of attribute value.
    void EncodeAttributes(std::map<int32, PyRep *> &into) const;

    /*
     * These kill warnings about inheritance's dominance
     */
    real_t GetReal(Attr attr) const { return AdvancedAttributeMgr<int_t, real_t>::GetReal(attr); }

    void SetReal(Attr attr, const real_t &v) { AdvancedAttributeMgr<int_t, real_t>::SetReal(attr, v); }
    void SetInt(Attr attr, const int_t &v) { AdvancedAttributeMgr<int_t, real_t>::SetInt(attr, v); }

    void Clear(Attr attr) { AdvancedAttributeMgr<int_t, real_t>::Clear(attr); }
};

/**
 * Attribute manager for type attributes. (still used)
 */
class TypeAttributeMgr : public EVEAttributeMgr
{
    friend class ItemAttributeMgr;  // for access to _Get
public:
    TypeAttributeMgr(const ItemType &type) : m_type(type) {}

    /**
     * @return ItemType which this manager is bound to.
     */
    const ItemType &type() const { return(m_type); }

    /**
     * Loads attributes from DB.
     *
     * @param[in] db Database to use.
     * @return True if load was successful, false if not.
     */
    bool Load(InventoryDB &db);

protected:
    const ItemType &m_type;
};

// small map that does the magic of item attributes..
//class EvilNumber;

/**
 * @brief rewrite of the item attribute system.
 *
 * @author Captnoord.
 * @date Juni 2010
 * @note keeping track of the base value of the attribute is not implemented.
 * Besides the fact in increases memory concumption its unclear how to design it
 * at this moment.
 */
class AttributeMap
{
public:
    /**
     * we store our keeper so we can use it in the various functions.
     * @param[in] item reference to the InventoryItem for which attributes will be managed
     * @param[in] bDefaultMap boolean indicating whether this attribute map uses 'entity_default_attributes' table or 'entity_attributes' table
     */
    AttributeMap(InventoryItem &item, bool bDefaultMap=false);

	/**
     * @brief set the attribute with @num
     *
     * set the attribute with @num
     *
     * @param[in] attributeId the attribute id that needs to be changed.
     * @param[in] num the number the attribute needs to be changed in.
     *
     * @retval true  The attribute has successfully been set and queued.
     * @retval false The attribute change has not been queued but has not been changed.
     */
    bool SetAttribute(uint32 attributeId, EvilNumber &num, bool nofity = true);

    EvilNumber GetAttribute(const uint32 attributeId) const;

    /*
     * HasAttribute
     *
     * returns true if this item has the attribute 'attributeID', false if it does not have this attribute
     *
     * @note this function should be used very infrequently and only for specific reasons
     */
    bool HasAttribute(const uint32 attributeID) const;
    bool HasAttribute(const uint32 attributeID, EvilNumber &value) const;

    /* ATM we don't load or save as we assume that all attribute modifiers are calculated on the fly
     * except charge attributes but we won't handle them for now
     */
    bool Save();

	// Allow users to force changed flag to true, indicating that Save() should really save attributes to the database
	void ForceChanged() { ; }//mChanged = true; }

    bool Delete();
    bool DeleteAttribute(uint32 attributeID);

    // load the default attributes that come with the itemID


    typedef std::map<uint32, EvilNumber>    AttrMap;
    typedef AttrMap::iterator               AttrMapItr;
    typedef AttrMap::const_iterator         AttrMapConstItr;

    bool Load();

    /* only save the ship damage other attribs are calculated when ship activated */
    void SaveShipState();
    /**
     * SaveAttributes
     *
     * @note this function always saves everything and doesn't take into account the fact that we should only save the
     * changes attributes.
     */
    bool SaveAttributes();
    bool SaveIntAttribute(uint32 attributeID, int64 value);
    bool SaveFloatAttribute(uint32 attributeID, double value);

    /*
     * ResetAttribute
     *
     *@note this function will force reload the default value for the specified attribute
     */
    bool ResetAttribute(uint32 attrID, bool notify);

    //void set_item(InventoryItem *item) {mItem = item;}

    /**
     * @brief return the begin iterator of the AttributeMap
     *
     *
     *
     * @return the begin iterator of the AttributeMap
     * @note this way to solve the attribute system problems are quite hacky... but atm its needed
     */
    AttrMapItr begin();

    /**
     * @brief return the end iterator of the AttributeMap
     *
     *
     *
     * @return the end iterator of the AttributeMap
     * @note this way to solve the attribute system problems are quite hacky... but atm its needed
     */
    AttrMapItr end();

protected:
    /**
     * @brief internal function to handle the change.
     *
     * @param[in] attributeId the attribute id that needs to be changed.
     * @param[in] num the number the attribute needs to be changed in.
     *
     * @retval true  The attribute change has successfully been set and queued.
     * @retval false The attribute change has not been queued but has been changed.
     */
    bool Change(uint32 attributeID, EvilNumber& old_val, EvilNumber& new_val);

    /**
     * @brief internal function to handle adding attributes.
     *
     * @param[in] attributeId the attribute id that needs to be added.
     * @param[in] num the number the attribute needs to be set to.
     *
     * @retval true  The attribute has successfully been added and queued.
     * @retval false The attribute addition has not been queued and not been changed.
     */
    bool Add(uint32 attributeID, EvilNumber& num);

    /**
     * @brief queue the attribute changes into the QueueDestinyEvent system.
     *
     * @param[in] attrChange the attribute id that needs to be added.
     *
     * @retval true  The attribute has successfully been added and queued.
     * @retval false The attribute addition has not been queued and not been changed.
     */
    bool SendAttributeChanges(PyTuple* attrChange);

    bool SaveIntAttribute(uint32 attributeID);
    bool SaveFloatAttribute(uint32 attributeID);

    /** we belong to this item..
     * @note possible design flaw because only items contain AttributeMap's so
     *       we don't need to store this.
     */
    InventoryItem &mItem;

    /**
     * @note possible design flaw, stack corruption because of a enormous amount
     *       of 'EvilNumber' objects not fitting into the stack.
     */
    AttrMap mAttributes;

    /**
     * we set and we clear this flag when we change attributes of this item....
     * @note we should improve this idea... and only save the changed attributes...
     *       but that is for the future.
     */
    bool mChanged;

    /**
     * we set this true to tell the class methods to use attributes from 'entity_default_attributes' table
	 * instead of the normal 'entity_attributes' table
     */
    bool mDefault;
};

#endif /* __EVE_ATTRIBUTE_MGR__H__INCL__ */
