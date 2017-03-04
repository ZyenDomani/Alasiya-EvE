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

#include "./eve-common.h"

typedef std::map<uint16, EvilNumber>    AttrMap;
typedef AttrMap::iterator               AttrMapItr;
typedef AttrMap::const_iterator         AttrMapConstItr;

/** @todo update/finish this class */
class PyTuple;
class InventoryItem;
class AttributeMap
{
public:
    /**
     * we store our keeper so we can use it in the various functions.
     * @param[in] item reference to the InventoryItem for which attributes will be managed
     * @param[in] isDefault boolean indicating whether this attribute map uses 'default' itemType attributes or saved-per-item 'entity_attributes' table
     */
    AttributeMap(InventoryItem& item);
    ~AttributeMap();

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
    void SetAttribute(uint16 attrID, EvilNumber& num, bool nofity = true);

    EvilNumber GetAttribute(const uint16 attrID) const;

    /*
     * HasAttribute
     *
     * returns true if this item has the attribute 'attrID', false if it does not have this attribute
     *
     * @note this function should be used very infrequently and only for specific reasons
     */
    bool HasAttribute(const uint16 attrID) const;
    bool HasAttribute(const uint16 attrID, EvilNumber &value) const;

    /* ATM we don't load or save as we assume that all attribute modifiers are calculated on the fly
     * except charge attributes but we won't handle them for now
     */
    bool Save();

	// Allow users to force changed flag to true, indicating that Save() should really save attributes to the database
	void ForceChanged() { ; }//mChanged = true; }

    void Delete();
    void DeleteAttribute(uint16 attrID);

    // load the default attributes that come with the item's typeID
    bool Load(bool reset=false);

    /* only save the ship damage other attribs are calculated when ship activated */
    void SaveShipState();
    /**
     * SaveAttributes
     *
     * @note this function always saves everything and doesn't take into account the fact that we should only save the
     * changes attributes.
     */
    bool SaveAttributes();

    /*
     * ResetAttribute
     *
     *@note this function will force reload the default value for the specified attribute
     */
    void ResetAttribute(uint16 attrID, bool notify);

    /**
     * @brief return the begin iterator of the AttributeMap
     * @return the begin iterator of the AttributeMap
     * @note this way to solve the attribute system problems are quite hacky... but atm its needed
     */
    AttrMapItr begin();

    /**
     * @brief return the end iterator of the AttributeMap
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
    bool Change(uint16 attrID, EvilNumber& old_val, EvilNumber& new_val);

    /**
     * @brief internal function to handle adding attributes.
     *
     * @param[in] attributeId the attribute id that needs to be added.
     * @param[in] num the number the attribute needs to be set to.
     *
     * @retval true  The attribute has successfully been added and queued.
     * @retval false The attribute addition has not been queued and not been changed.
     */
    bool Add(uint16 attrID, EvilNumber& num);

    /**
     * @brief queue the attribute changes into the QueueDestinyEvent system.
     *
     * @param[in] attrChange the attribute id that needs to be added.
     *
     * @retval true  The attribute has successfully been added and queued.
     * @retval false The attribute addition has not been queued and not been changed.
     */
    bool SendChanges(PyTuple* attrChange);

    /** we belong to this item..
     * @note possible design flaw because only items contain AttributeMap's so
     *       we don't need to store this.
     */
    InventoryItem& mItem;

    /**
     * @note possible design flaw, stack corruption because of a enormous amount
     *       of 'EvilNumber' objects not fitting into the stack.
     */
    AttrMap mAttributes;
};

#endif /* __EVE_ATTRIBUTE_MGR__H__INCL__ */
