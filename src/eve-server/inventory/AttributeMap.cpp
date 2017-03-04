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


AttributeMap::AttributeMap( InventoryItem& item)
: mItem(item)
{
    mAttributes.clear();
}

AttributeMap::~AttributeMap()
{
    mAttributes.clear();
}


bool AttributeMap::Load(bool reset/*false*/) {
    if (reset) {
        // this will allow total clearing of attribs to eliminate the necessity of 'removing' effects
        mAttributes.clear();
    }
    /* First, we load default attributes values from typeattrmgr.*/
    /* most attribute have default values which are related to the item type */
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
    /* item now has it's own attribute map, and is deleted when item object is destroyed or reset */
    _log(ITEM__DEBUG, "AttributeMap::Load()  Loaded %u attribs for %s.", mAttributes.size(), mItem.itemName().c_str());
    return true;
}

bool AttributeMap::SaveAttributes() {
    return Save();
}

bool AttributeMap::Save() {
    /** @todo update this
     * we are saving:
     *   all attribs for skills
     *   all attribs for ISEs and CSEs, where applicable
     *   damage for modules/charges, where applicable (ship damage saved separately)
     */
    if (mItem.itemID() >= EVEMU_NPC_ID) return true;    // not saving npc attribs
    if (mItem.itemID() < EVEMU_MINIMUM_ID) return true; // not saving static object attribs
    if (mItem.categoryID() == EVEDB::invCategories::Ship) // ship attribs saved in shipItem
        return true;

    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO entity_attributes (itemID, attributeID, valueInt, valueFloat) ";
    bool first = true;
    AttrMapItr itr = mAttributes.begin();
    /** @todo  test for and save the following:
     *  damage attributes for items (for persistance)
     *  sp/lvl attribs for skills
     *  size/etc for roids (should be in bms)
     *
     * ships saved separately
     */
    for (; itr != mAttributes.end(); itr++) {
        if (mItem.groupID() != EVEDB::invGroups::Character)
            if ((itr->first != AttrDamage) or (itr->first != AttrSkillPoints) or (itr->first != AttrSkillLevel))
                continue;

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

    return true;
}


void AttributeMap::SetAttribute( uint16 attrID, EvilNumber& num, bool nofity /*true*/ )
{
    AttrMapItr itr = mAttributes.find(attrID);

    if (itr == mAttributes.end()) {
        mAttributes.insert(std::make_pair(attrID, num));
        if (nofity)
            Add(attrID, num);
        return;
    }

    if (itr->second == num)
        return;

    if (nofity)
        Change(attrID, itr->second, num);

    itr->second = num;
    return;
}

EvilNumber AttributeMap::GetAttribute( const uint16 attrID ) const
{
    AttrMapConstItr itr = mAttributes.find(attrID);
    if (itr != mAttributes.end())
        return itr->second;
    return EvilNumber(0);
}

bool AttributeMap::HasAttribute(const uint16 attrID) const
{
    AttrMapConstItr itr = mAttributes.find(attrID);
    if (itr != mAttributes.end())
        return true;
    return false;
}

bool AttributeMap::HasAttribute(const uint16 attrID, EvilNumber &value) const
{
    AttrMapConstItr itr = mAttributes.find(attrID);
    if (itr != mAttributes.end()) {
        value = itr->second;
        return true;
    }
    return false;
}

bool AttributeMap::Change( uint16 attrID, EvilNumber& old_val, EvilNumber& new_val ) {
    if (old_val == new_val) return true;
    Notify_OnModuleAttributeChange modChange;
        modChange.ownerID = mItem.ownerID();
        modChange.itemKey = mItem.itemID();
        modChange.attributeID = attrID;
        modChange.time = Win32TimeNow();
        modChange.newValue = new_val.GetPyObject();
        modChange.oldValue = old_val.GetPyObject();
	return SendChanges(modChange.Encode());
}

bool AttributeMap::Add( uint16 attrID, EvilNumber& num ) {
    Notify_OnModuleAttributeChange modChange;
        modChange.ownerID = mItem.ownerID();
        modChange.itemKey = mItem.itemID();
        modChange.attributeID = attrID;
        modChange.time = Win32TimeNow();
        modChange.newValue = num.GetPyObject();
        modChange.oldValue = new PyNone();
    return SendChanges(modChange.Encode());
}

bool AttributeMap::SendChanges( PyTuple* attrChange ) {
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
        _log(PLAYER__WARNING, "AttributeMap::SendChanges() - ownerID for %u not found", mItem.itemID() );
        return false;
    }
    return true;
}

void AttributeMap::ResetAttribute(uint16 attrID, bool notify) {
    /** @todo update this */
    EvilNumber value = mItem.GetDefaultAttribute(attrID);
    SetAttribute(attrID, value, notify);
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

// Delete() only called from InventoryItem::Delete()
void AttributeMap::Delete() {
	mAttributes.clear();
}

void AttributeMap::DeleteAttribute(uint16 attrID) {
    AttrMapItr itr = mAttributes.find(attrID);
    if (itr != end())
        mAttributes.erase(itr);
}

AttrMapItr AttributeMap::begin() {
    return mAttributes.begin();
}

AttrMapItr AttributeMap::end() {
    return mAttributes.end();
}
