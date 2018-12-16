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
    Rewrite:    Allan
*/

#ifndef __EVE_ATTRIBUTE_MGR__H__INCL__
#define __EVE_ATTRIBUTE_MGR__H__INCL__

#include "./eve-common.h"

#include "inventory/InventoryDB.h"

typedef std::map<uint16, EvilNumber>    AttrMap;
typedef AttrMap::iterator               AttrMapItr;
typedef AttrMap::const_iterator         AttrMapConstItr;

class PyTuple;
class InventoryItem;

class AttributeMap
{
public:
    AttributeMap(InventoryItem& item);
    ~AttributeMap() noexcept;

    void SetAttribute(uint16 attrID, EvilNumber& num, bool nofity = true);
    void MultiplyAttribute(uint16 attrID, EvilNumber& num, bool nofity = false);

    EvilNumber GetAttribute(const uint16 attrID) const;

    bool HasAttribute(const uint16 attrID) const;
    bool HasAttribute(const uint16 attrID, EvilNumber &value) const;

    bool Save();

    void Delete();
    void DeleteAttribute(uint16 attrID);

    // load the default attributes that come with the item's typeID
    bool Load(bool reset=false);

    /* only save the ship damage. other attribs are calculated when ship activated */
    void SaveShipState();
    bool SaveAttributes();

    void ResetAttribute(uint16 attrID, bool notify);
    void CopyAttributes(std::map<uint16, EvilNumber>& attrMap);

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

    InventoryItem& mItem;

    AttrMap mAttributes;

private:
    InventoryDB m_db;

};

#endif /* __EVE_ATTRIBUTE_MGR__H__INCL__ */
