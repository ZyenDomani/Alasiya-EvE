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
    Update:     Captnoord   - Juni 2010
    Rewrite:    Allan       - 6Feb17
*/

#include "eve-server.h"

#include "Client.h"
#include "EntityList.h"
#include "StaticDataMgr.h"
#include "inventory/AttributeMap.h"
#include "inventory/InventoryDB.h"
#include "inventory/InventoryItem.h"


/************************************************************************/
/* Start of new attribute system                                        */
/************************************************************************/
AttributeMap::AttributeMap( InventoryItem& item)
: mItem(item),
  mChanged(true)
{
}

bool AttributeMap::Load() {
    /* First, we load default attributes values from typeattrmgr.*/
    std::vector< DmgTypeAttribute > typeAttrVec;
    sDataMgr.GetDgmTypeAttrVec(mItem.typeID(), typeAttrVec);
    for (auto cur : typeAttrVec) {
        SetAttribute(cur.attributeID, cur.value, false);
    }

    /* Then we load item damage from the db, if any, to update the defaults */
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT  attributeID, valueInt, valueFloat FROM entity_attributes WHERE itemID='%u'", mItem.itemID())) {
        _log(DATABASE__ERROR, "AttributeMap", "Error in db load query: %s", res.error.c_str());
    }

    DBResultRow row;
    EvilNumber value = 0;
    while (res.GetRow(row)) {
        if (row.IsNull(1))
            value = row.GetDouble(2);
        else
            value = row.GetInt64(1);
        SetAttribute(row.GetUInt(0), value, false);
    }
    /* item has it's own attribute map, and is deleted when item object is destroyed */
    _log(ITEM__DEBUG, "AttributeMap::Load()  Loaded %u attribs for %s.", mAttributes.size(), mItem.itemName().c_str());
    return true;
}

bool AttributeMap::Save() {
    /** @todo update this
     * we are saving:
     *   all attribs for skills
     *   all attribs for ISEs and CSEs, where applicable
     *   damage for ships/modules/charges, where applicable
     */
    if (mItem.itemID() >= EVEMU_NPC_ID) return true;    // not saving npc attribs
    if (mItem.itemID() < EVEMU_MINIMUM_ID) return true; // not saving static object attribs
    bool success = false;

    /* if nothing changed... it means this action has previously been successful we return true... */
    if (!mChanged)
        return true;

    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO entity_attributes (itemID, attributeID, valueInt, valueFloat) ";
    bool first = true;
    AttrMapItr itr = mAttributes.begin();
    for (; itr != mAttributes.end(); itr++) {
        if (first) {
            Inserts << "VALUES";
            first = false;
        } else
            Inserts << ", ";
        Inserts << "(" << mItem.itemID() << ", " << itr->first << ", ";
        if ( itr->second.get_type() == evil_number_int ) {
            if (IsNaN(itr->second.get_int())) {
                _log(INV__ERROR, "AttributeMap::Save() - int == NaN for itemID:%u", mItem.itemID());
                return false;
            }
            Inserts << itr->second.get_int() << ", NULL)";
        } else {
            if (IsNaN(itr->second.get_double())) {
                _log(INV__ERROR, "AttributeMap::Save() - float == NaN for itemID:%u", mItem.itemID());
                return false;
            }
            Inserts << " NULL, " << itr->second.get_double() << ")";
        }
    }

    if (!first) {
        Inserts << "ON DUPLICATE KEY UPDATE ";
        Inserts << "valueInt=VALUES(valueInt), ";
        Inserts << "valueFloat=VALUES(valueFloat)";
        // execute the command.
        /** @todo  take this outta here.  copy from itemfactory.save() */
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str())) {
            _log(DATABASE__ERROR, "AttributeMap - unable to save attributes - %s", err.c_str());
            return false;
        }
    }

    mChanged = false;
    return true;
}


bool AttributeMap::SetAttribute( uint32 attributeId, EvilNumber& num, bool notify /*true*/ )
{
    AttrMapItr itr = mAttributes.find(attributeId);

    /* most attribute have default values which are related to the item type */
    if (itr == mAttributes.end()) {
        mAttributes.insert(std::make_pair(attributeId, num));
        if (notify)
            Add(attributeId, num);
        return (mChanged = true);
    }

    if (itr->second == num)
        return true;

    // notify dogma of attribute change
    if (notify)
        Change(attributeId, itr->second, num);

    itr->second = num;
    return (mChanged = true);
}

EvilNumber AttributeMap::GetAttribute( const uint32 attributeId ) const
{
    AttrMapConstItr itr = mAttributes.find(attributeId);
    if (itr != mAttributes.end())
        return itr->second;
    return EvilNumber(0);
}

bool AttributeMap::HasAttribute(const uint32 attributeID) const
{
    AttrMapConstItr itr = mAttributes.find(attributeID);
    if (itr != mAttributes.end())
        return true;
    return false;
}

bool AttributeMap::HasAttribute(const uint32 attributeID, EvilNumber &value) const
{
    AttrMapConstItr itr = mAttributes.find(attributeID);
    if (itr != mAttributes.end()) {
        value = itr->second;
        return true;
    }
    return false;
}

bool AttributeMap::Change( uint32 attributeID, EvilNumber& old_val, EvilNumber& new_val ) {
    if (old_val == new_val) return true;
    Notify_OnModuleAttributeChange modChange;
        modChange.ownerID = mItem.ownerID();
        modChange.itemKey = mItem.itemID();
        modChange.attributeID = attributeID;
        modChange.time = Win32TimeNow();
        modChange.newValue = new_val.GetPyObject();
        modChange.oldValue = old_val.GetPyObject();
	return SendAttributeChanges(modChange.Encode());
}

bool AttributeMap::Add( uint32 attributeID, EvilNumber& num ) {
    Notify_OnModuleAttributeChange modChange;
        modChange.ownerID = mItem.ownerID();
        modChange.itemKey = mItem.itemID();
        modChange.attributeID = attributeID;
        modChange.time = Win32TimeNow();
        modChange.newValue = num.GetPyObject();
        modChange.oldValue = new PyInt(0);
    return SendAttributeChanges(modChange.Encode());
}

bool AttributeMap::SendAttributeChanges( PyTuple* attrChange ) {
    if (!attrChange) return true;
    Client* client(nullptr);

    if (IsCharType(mItem.typeID()))
        client = sEntityList.FindClientByCharID(mItem.itemID());
    else if ((mItem.ownerID() == 1) || IsNPCCorp(mItem.ownerID()))
        return true;
    else
        client = sEntityList.FindClientByCharID(mItem.ownerID());

    if (client) {
        if (is_log_enabled(CLIENT__TRACE))
            attrChange->Dump(CLIENT__TRACE, "");
        client->QueueDestinyEvent(&attrChange);
    } else {
        _log(PLAYER__WARNING, "AttributeMap::SendAttributeChanges() - ownerID for %u not found", mItem.itemID() );
        return false;
    }
    return true;
}

bool AttributeMap::ResetAttribute(uint32 attrID, bool notify) {
    /** @todo update this */
    EvilNumber value = mItem.GetDefaultAttribute(attrID);
    return SetAttribute(attrID, value, notify);
}

bool AttributeMap::SaveAttributes() {
    return Save();
}

void AttributeMap::SaveShipState()
{
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "REPLACE INTO entity_attributes ";
    Inserts << " (itemID, attributeID, valueInt, valueFloat) VALUES";
    bool shield = false, armor = false, hull = false;
    AttrMap::iterator cur = mAttributes.find(AttrShieldCharge);
    if (cur != mAttributes.end()) shield = true;
    if (shield) {
        Inserts << "(" << mItem.itemID() << ", " << cur->first << ", ";
        if ( cur->second.get_type() == evil_number_int ) {
            Inserts << cur->second.get_int() << ", NULL)";
        } else {
            Inserts << " NULL, " << cur->second.get_double() << ")";
        }
    }
    cur = mAttributes.find(AttrArmorDamage);
    if (cur != mAttributes.end()) armor = true;
    if (armor) {
        if (shield) Inserts << ",";
        Inserts << "(" << mItem.itemID() << ", " << cur->first << ", ";
        if ( cur->second.get_type() == evil_number_int ) {
            Inserts << cur->second.get_int() << ", NULL)";
        } else {
            Inserts << " NULL, " << cur->second.get_double() << ")";
        }
    }
    cur = mAttributes.find(AttrDamage);
    if (cur != mAttributes.end()) hull = true;
    if (hull) {
        if (shield or armor) Inserts << ",";
        Inserts << "(" << mItem.itemID() << ", " << cur->first << ", ";
        if ( cur->second.get_type() == evil_number_int ) {
            Inserts << cur->second.get_int() << ", NULL)";
        } else {
            Inserts << " NULL, " << cur->second.get_double() << ")";
        }
    }

    if (shield or armor or hull) {
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str())) {
            _log(DATABASE__ERROR, "SaveShipState - unable to save attributes for %u - %s", mItem.itemID(), err.c_str());
        }
    }
}

bool AttributeMap::Delete() {
    DBerror err;
    if (!sDatabase.RunQuery(err, "DELETE FROM entity_attributes WHERE itemID = %u", mItem.itemID())) {
        _log(DATABASE__ERROR, "AttributeMap - unable to delete attributes - %s", err.c_str());
        return false;
    }

	mAttributes.clear();
	mChanged = false; // just synced with database, no need to save
    return true;
}

bool AttributeMap::DeleteAttribute(uint32 attributeID) {
    DBerror err;
    if (!sDatabase.RunQuery(err, "DELETE FROM entity_attributes WHERE itemID = %u", attributeID)) {
        _log(DATABASE__ERROR, "AttributeMap - unable to delete attributeID %u for itemID %u - %s", attributeID, mItem.itemID(), err.c_str());
        return false;
    }
    mChanged = false; // just synced with database, no need to save
    return true;
}

AttributeMap::AttrMapItr AttributeMap::begin() {
    return mAttributes.begin();
}

AttributeMap::AttrMapItr AttributeMap::end() {
    return mAttributes.end();
}
/************************************************************************/
/* End of new attribute system                                          */
/************************************************************************/