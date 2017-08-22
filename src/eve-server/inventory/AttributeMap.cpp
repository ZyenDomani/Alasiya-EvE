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
    /* First, we copy default attributes values from our itemType */
    mItem.type().CopyAttributes(mItem);

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
    /** @note
     * we are saving:
     *   all attribs for characters
     *   level and sp attribs for skills
     *   all attribs for ISEs and CSEs, where applicable
     *   damage for modules/charges, where applicable (ship damage saved separately)
     */
    if (mItem.itemID() >= EVEMU_NPC_ID)    // not saving npc attribs
        return true;
    else if (mItem.itemID() < EVEMU_MINIMUM_DYNAMIC_ID)  // not saving static object attribs
        return true;

    bool skill = false, damage = false, owner = false;
    switch (mItem.categoryID()) {
        case EVEDB::invCategories::Asteroid:    // asteroids and blueprints are NOT saved here
        case EVEDB::invCategories::Blueprint: {
            return false;
        } break;
        case EVEDB::invCategories::Ship: {      // ship attribs saved in shipItem, not here.
            return true;
        } break;
        case EVEDB::invCategories::Skill: {     // save SP and Level for skills
            skill = true;
        } break;
        case EVEDB::invCategories::Owner:       // save all attribs for these
        case EVEDB::invCategories::Celestial:
        case EVEDB::invCategories::Structure:
        case EVEDB::invCategories::StructureUpgrade:
        case EVEDB::invCategories::SovereigntyStructure:
        case EVEDB::invCategories::Orbitals:
        case EVEDB::invCategories::Deployable: {
            owner = true;
        } break;
        case EVEDB::invCategories::Module:      // save damage for these
        case EVEDB::invCategories::Charge:
        case EVEDB::invCategories::Subsystem:
        case EVEDB::invCategories::Drone: {
            damage = true;
        } break;
    }
    EvilNumber zero = 0;
    bool first = true, save = false;
    std::vector<AttrData> items;
    items.clear();
    AttrMapItr itr = mAttributes.begin();
    for (; itr != mAttributes.end(); ++itr) {
        save = false;
        if (skill)
            if ((itr->first == AttrSkillPoints) or (itr->first == AttrSkillLevel) or (itr->first == AttrExpiryTime))
                save = true;
        if (damage)
            if (itr->first == AttrDamage)
                save = true;
        if (owner)
            save = true;
        if (save)
            if (itr->second != zero) {
                AttrData data;
                data.itemID = mItem.itemID();
                data.attrID = itr->first;
                if ( itr->second.get_type() == evil_number_int ) {
                    data.valueInt = itr->second.get_int();
                    data.valueFloat = 0;
                } else {
                    data.valueInt = 0;
                    data.valueFloat = itr->second.get_double();
                }
                items.push_back(data);
            }
    }

    m_db.SaveAttributes(items);

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
    Client* pClient(nullptr);

    if (IsCharType(mItem.typeID()))
        pClient = sEntityList.FindClientByCharID(mItem.itemID());
    else if ((mItem.ownerID() == 1) || IsNPCCorp(mItem.ownerID()))
        return true;
    else
        pClient = sEntityList.FindClientByCharID(mItem.ownerID());

    if (pClient != nullptr) {
        if (is_log_enabled(CLIENT__TRACE))
            attrChange->Dump(CLIENT__TRACE, "");
        pClient->QueueDestinyEvent(&attrChange);
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

void AttributeMap::CopyAttributes(std::map< uint16, EvilNumber >& attrMap)
{
    for (auto cur : mAttributes)
        attrMap.insert(std::pair<uint16, EvilNumber>(cur.first, cur.second));
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
