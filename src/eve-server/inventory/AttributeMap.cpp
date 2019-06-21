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
    /* First, we copy default attributes values from our itemType, loaded into memObj when type is loaded */
    mItem.type().CopyAttributes(mItem);

    /* Then we load saved attribs from the db, if any, to update the defaults with items current (saved) values*/
    DBQueryResult res;
    if (IsCharacter(mItem.itemID())) {
        if (!sDatabase.RunQuery(res, "SELECT  attributeID, valueInt, valueFloat FROM chrCharacterAttributes WHERE charID=%u", mItem.itemID())) {
            _log(DATABASE__ERROR, "AttributeMap", "Error in db load query: %s", res.error.c_str());
        }
    } else {
        if (!sDatabase.RunQuery(res, "SELECT  attributeID, valueInt, valueFloat FROM entity_attributes WHERE itemID=%u", mItem.itemID())) {
            _log(DATABASE__ERROR, "AttributeMap", "Error in db load query: %s", res.error.c_str());
        }
    }

    DBResultRow row;
    EvilNumber value = EvilZero;
    while (res.GetRow(row)) {
        if (row.IsNull(1)) {
            if (row.IsNull(2))
                value = EvilZero;
            else
                value = row.GetDouble(2);
        } else
            value = row.GetInt64(1);
        SetAttribute(row.GetUInt(0), value, false);
    }
    /* item now has it's own attribute map, and is deleted when item object is destroyed or reset */
    if (is_log_enabled(ITEM__DEBUG))
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
     *   level, sp and endtime attribs for skills
     *   all attribs for ISEs and CSEs, where applicable
     *   damage and online for modules
     *   damage for charges, where applicable (ship damage saved separately)
     */
    if (IsStaticItem(mItem.itemID()))
        return true;

    bool skill = false, damage = false, owner = false, module = false;
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
        case EVEDB::invCategories::Module:      // save damage and online for these
            module = true;                      // we're falling thru on purpose here
        case EVEDB::invCategories::Charge:      // remember, crystals and lenses are charges, too.
        case EVEDB::invCategories::Subsystem:
        case EVEDB::invCategories::Drone: {     // this may need more.  check once system is working
            damage = true;
        } break;
    }

    bool save = false;
    std::vector<AttrData> items;
    items.clear();
    AttrMapItr itr = mAttributes.begin(), end = mAttributes.end();
    for (; itr != end; ++itr) {
        save = false;
        if (skill)
            if ((itr->first == AttrSkillPoints) or (itr->first == AttrSkillLevel) or (itr->first == AttrExpiryTime))
                save = true;
        if (damage)
            if (itr->first == AttrDamage)
                save = true;
        if (module)
            if (itr->first == AttrOnline)
                save = true;
        if (owner)
            save = true;
        if (save) {
            AttrData data = AttrData();
            data.itemID = mItem.itemID();
            data.attrID = itr->first;
            if (itr->second.isInt())
                data.valueInt = itr->second.get_int();
            else
                data.valueFloat = itr->second.get_double();
            items.push_back(data);
        }
    }

    if (!items.empty())
        m_db.SaveAttributes(IsCharacter(mItem.itemID()), items);
    return true;
}


void AttributeMap::SetAttribute( uint16 attrID, EvilNumber& num, bool nofity /*true*/ )
{
    if (num.isNaN() or num.isInf()) {
        _log(ITEM__ERROR, "AttributeMap::SetAttribute() - Something sent NaN or Inf.");
        EvE::traceStack();
        return;
    }
    AttrMapItr itr = mAttributes.find(attrID);
    if (itr == mAttributes.end()) {
        mAttributes.emplace(attrID, num);
        if (nofity)
            Add(attrID, num);
        return;
    }

    if (itr->second == num)
        return;

    if (nofity)
        Change(attrID, itr->second, num);

    itr->second = num;
}

void AttributeMap::MultiplyAttribute(uint16 attrID, EvilNumber& num, bool nofity/*false*/)
{
    if (num.isNaN() or num.isInf())
        return;     // make error here for bad number?
    if (num == EvilZero)
        return;     // could this be on purpose?
    AttrMapItr itr = mAttributes.find(attrID);
    if (itr == mAttributes.end())
        return; // it doesnt exist...nothing to do.

    EvilNumber oldValue = itr->second;
    itr->second *= num;

    if (nofity)
        Change(attrID, oldValue, itr->second);
}


EvilNumber AttributeMap::GetAttribute( const uint16 attrID ) const
{
    AttrMapConstItr itr = mAttributes.find(attrID);
    if (itr != mAttributes.end())
        return itr->second;
    return EvilZero;
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
    value = EvilZero;
    return false;
}

bool AttributeMap::Change( uint16 attrID, EvilNumber& old_val, EvilNumber& new_val ) {
    if (old_val == new_val) return true;
    Notify_OnModuleAttributeChange modChange;
        modChange.ownerID = mItem.ownerID();
        modChange.itemKey = mItem.itemID();
        modChange.attributeID = attrID;
        modChange.time = GetFileTimeNow();
        modChange.newValue = new_val.GetPyObject();
        modChange.oldValue = old_val.GetPyObject();
	return SendChanges(modChange.Encode());
}

bool AttributeMap::Add( uint16 attrID, EvilNumber& num ) {
    Notify_OnModuleAttributeChange modChange;
        modChange.ownerID = mItem.ownerID();
        modChange.itemKey = mItem.itemID();
        modChange.attributeID = attrID;
        modChange.time = GetFileTimeNow();
        modChange.newValue = num.GetPyObject();
        modChange.oldValue = new PyNone();
    return SendChanges(modChange.Encode());
}

bool AttributeMap::SendChanges( PyTuple* attrChange ) {
    if (attrChange == nullptr)
        return true;
    if (IsCorp(mItem.ownerID()))
        return true;
    if ((mItem.ownerID() == 1)
    and (!IsCharacter(mItem.itemID())))
        return true;

    Client* pClient(nullptr);
    if (IsCharacter(mItem.itemID()))
        pClient = sEntityList.FindClientByCharID(mItem.itemID());
    else
        pClient = sEntityList.FindClientByCharID(mItem.ownerID());

    if (pClient == nullptr) {
        _log(PLAYER__WARNING, "AttributeMap::SendChanges() - ownerID for %u not found", mItem.itemID() );
        return false;
    }

    if (is_log_enabled(CLIENT__TRACE))
        attrChange->Dump(CLIENT__TRACE, "");
    pClient->QueueDestinyEvent(&attrChange);

    return true;
}

void AttributeMap::ResetAttribute(uint16 attrID, bool notify) {
    EvilNumber value = mItem.GetDefaultAttribute(attrID);
    SetAttribute(attrID, value, notify);
}

void AttributeMap::CopyAttributes(std::map< uint16, EvilNumber >& attrMap)
{
    for (auto cur : mAttributes)
        attrMap[cur.first] =  cur.second;
}

void AttributeMap::SaveShipState()
{
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "REPLACE INTO entity_attributes ";
    Inserts << " (itemID, attributeID, valueInt, valueFloat) VALUES";
    bool shield = false, armor = false, hull = false, hi = false, mid = false, lo = false;
    AttrMap::iterator cur = mAttributes.find(AttrShieldCharge);
    if (cur != mAttributes.end()) {
        shield = true;
        Inserts << "(" << mItem.itemID() << ", " << cur->first << ", ";
        if ( cur->second.get_type() == evil_number_int ) {
            Inserts << cur->second.get_int() << ", NULL)";
        } else {
            Inserts << " NULL, " << cur->second.get_double() << ")";
        }
    }
    cur = mAttributes.find(AttrArmorDamage);
    if (cur != mAttributes.end()) {
        armor = true;
        if (shield)
            Inserts << ",";
        Inserts << "(" << mItem.itemID() << ", " << cur->first << ", ";
        if ( cur->second.get_type() == evil_number_int ) {
            Inserts << cur->second.get_int() << ", NULL)";
        } else {
            Inserts << " NULL, " << cur->second.get_double() << ")";
        }
    }
    cur = mAttributes.find(AttrDamage);
    if (cur != mAttributes.end()) {
        hull = true;
        if (shield or armor)
            Inserts << ",";
        Inserts << "(" << mItem.itemID() << ", " << cur->first << ", ";
        if ( cur->second.get_type() == evil_number_int ) {
            Inserts << cur->second.get_int() << ", NULL)";
        } else {
            Inserts << " NULL, " << cur->second.get_double() << ")";
        }
    }
    cur = mAttributes.find(AttrHeatHi);
    if (cur != mAttributes.end()) {
        hi = true;
        if (shield or armor)
            Inserts << ",";
        Inserts << "(" << mItem.itemID() << ", " << cur->first << ", ";
        if ( cur->second.get_type() == evil_number_int ) {
            Inserts << cur->second.get_int() << ", NULL)";
        } else {
            Inserts << " NULL, " << cur->second.get_double() << ")";
        }
    }
    cur = mAttributes.find(AttrHeatMed);
    if (cur != mAttributes.end()) {
        mid = true;
        if (shield or armor)
            Inserts << ",";
        Inserts << "(" << mItem.itemID() << ", " << cur->first << ", ";
        if ( cur->second.get_type() == evil_number_int ) {
            Inserts << cur->second.get_int() << ", NULL)";
        } else {
            Inserts << " NULL, " << cur->second.get_double() << ")";
        }
    }
    cur = mAttributes.find(AttrHeatLow);
    if (cur != mAttributes.end()) {
        lo = true;
        if (shield or armor)
            Inserts << ",";
        Inserts << "(" << mItem.itemID() << ", " << cur->first << ", ";
        if ( cur->second.get_type() == evil_number_int ) {
            Inserts << cur->second.get_int() << ", NULL)";
        } else {
            Inserts << " NULL, " << cur->second.get_double() << ")";
        }
    }

    if (shield or armor or hull or hi or mid or lo) {
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
    if (itr != mAttributes.end())
        mAttributes.erase(itr);
    DBerror err;
    if (IsCharacter(mItem.itemID())) {
        if (!sDatabase.RunQuery(err, "DELETE FROM chrCharacterAttributes WHERE charID = %u AND attributeID = %u", mItem.itemID(), attrID)) {
            _log(DATABASE__ERROR, "DeleteAttribute - unable to delete attribute %u for %u - %s", attrID, mItem.itemID(), err.c_str());
        }
    } else {
        if (!sDatabase.RunQuery(err, "DELETE FROM entity_attributes WHERE itemID = %u AND attributeID = %u", mItem.itemID(), attrID)) {
            _log(DATABASE__ERROR, "DeleteAttribute - unable to delete attribute %u for %u - %s", attrID, mItem.itemID(), err.c_str());
        }
    }
}

AttrMapItr AttributeMap::begin() {
    return mAttributes.begin();
}

AttrMapItr AttributeMap::end() {
    return mAttributes.end();
}
