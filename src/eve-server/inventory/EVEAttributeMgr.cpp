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
    Updates:    Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "EntityList.h"
#include "inventory/EVEAttributeMgr.h"
#include "inventory/InventoryDB.h"
#include "inventory/InventoryItem.h"

/*
 * EVEAttributeMgr
 */
bool EVEAttributeMgr::m_persistentLoaded = false;
bool EVEAttributeMgr::m_persistent[EVEAttributeMgr::Invalid_Attr];

PyRep *EVEAttributeMgr::PyGet(Attr attr) const {
    return _PyGet(GetReal(attr));
}

void EVEAttributeMgr::EncodeAttributes(std::map<int32, PyRep *> &into) const {
    // integers first
    for (auto cur : m_ints) {
        if (into.find(cur.first) != into.end())
            PyDecRef( into[cur.first] );
        into[cur.first] = _PyGet(cur.second);
    }
    // then reals
    for (auto cur : m_reals) {
        if (into.find(cur.first) != into.end())
            PyDecRef( into[cur.first] );
        into[cur.first] = _PyGet(cur.second);
    }
}

PyRep *EVEAttributeMgr::_PyGet(const real_t &v)
{
    if (_IsInt(v))
        return new PyInt(static_cast<int32>(v));

    return new PyFloat(v);
}

void EVEAttributeMgr::_LoadPersistent() {
    if (!m_persistentLoaded) {
        memset(m_persistent, false, sizeof(m_persistent));

        #define ATTR(ID, name, default_value, persistent) \
            m_persistent[Attr_##name] = persistent;
        #include "inventory/EVEAttributes.h"

        m_persistentLoaded = true;
    }
}

/*
 * EVEAdvancedAttributeMgr
 */
void EVEAdvancedAttributeMgr::EncodeAttributes(std::map<int32, PyRep *> &into) const {
    // EVEAttributeMgr::EncodeAttributes(into);
    // integers first
    for (auto cur : m_ints) {
        if (into.find(cur.first) != into.end())
            PyDecRef( into[cur.first] );
        into[cur.first] = _PyGet(cur.second);
    }
    // then reals
    for (auto cur : m_reals) {
        if (into.find(cur.first) != into.end())
            PyDecRef( into[cur.first] );
        into[cur.first] = _PyGet(cur.second);
    }
}

/*
 * TypeAttributeMgr
 */
bool TypeAttributeMgr::Load(InventoryDB &db) {
    // load new contents from DB
    return db.LoadTypeAttributes(type().id(), *this);
}

/*
 * ItemAttributeMgr (no longer used)
 */
#ifdef (0)
ItemAttributeMgr::ItemAttributeMgr( ItemFactory &factory, const InventoryItem &item, bool save, bool notify) :
    m_factory(factory), m_item(item), m_save(save), m_notify(notify)
    {
        sLog.Blue("ItemAttributeMgr", "Calling Constructor.");
    }

ItemAttributeMgr::real_t ItemAttributeMgr::GetReal(Attr attr) const {
    real_t v;
    if (!_Get(attr, v))
        if (!m_item.type().attributes._Get(attr, v)) // try the type attributes
            v = GetDefault(attr);

    _CalcTauCap(attr, v);

    return v;
}

void ItemAttributeMgr::SetIntEx(Attr attr, const int_t &v, bool persist) {
    PyRep *oldValue = NULL;
    if (m_notify && !IsRechargable(attr)) {
        // get old value
        oldValue = PyGet(attr);
    }
    // set the attribute value
    EVEAdvancedAttributeMgr::SetInt(attr, v);
    // check if we shall save to DB
    if (GetSave() && (persist || IsPersistent(attr))) {
        // save to DB
        m_factory.db().UpdateAttribute_int(m_item.itemID(), attr, v);
    }
    if (m_notify) {
        std::map<Attr, TauCap>::const_iterator i = m_tauCap.find(attr);
        if (i != m_tauCap.end()) {
            // build the special list for rechargables
            PyList *l = new PyList;

            l->AddItemInt( v );
            l->AddItemLong( Win32TimeNow() );
            l->AddItem( _PyGet( GetReal( i->second.tau ) / 5.0 ) );
            l->AddItem( PyGet( i->second.cap ) );

            oldValue = l;
        }
        // send change
        _SendAttributeChange(attr, oldValue, new PyFloat(v));
    }
}

void ItemAttributeMgr::SetRealEx(Attr attr, const real_t &v, bool persist) {
    // first check if it can be stored as integer
    if (_IsInt(v)) {
        // store as integer
        SetIntEx(attr, static_cast<int32>(v), persist);
    } else {
        // store as real
        PyRep *oldValue = NULL;
        if (m_notify && !IsRechargable(attr)) {
            // get old value
            oldValue = PyGet(attr);
        }
        // set the attribute value
        EVEAdvancedAttributeMgr::SetReal(attr, v);
        // check if we shall save to DB
        if (GetSave() && (persist || IsPersistent(attr))) {
            // save to DB
            m_factory.db().UpdateAttribute_double(m_item.itemID(), attr, v);
        }
        if (m_notify) {
            std::map<Attr, TauCap>::const_iterator i = m_tauCap.find(attr);
            if (i != m_tauCap.end()) {
                // build the special list for rechargables
                PyList *l = new PyList;

                l->AddItemReal( v );
                l->AddItemLong( Win32TimeNow() );
                l->AddItem( _PyGet( GetReal( i->second.tau ) / 5.0 ) );
                l->AddItem( PyGet( i->second.cap ) );

                oldValue = l;
            }
            // send change
            _SendAttributeChange(attr, oldValue, new PyFloat(v));
        }
    }
}

void ItemAttributeMgr::Clear(Attr attr) {
    PyRep *oldValue = NULL;
    if (m_notify && !IsRechargable(attr)) {
        // get old value
        oldValue = PyGet(attr);
    }
    // clear the attribute
    EVEAdvancedAttributeMgr::Clear(attr);
    // delete the attribute from DB (no matter if it really is there)
    if (GetSave()) {
        m_factory.db().EraseAttribute(m_item.itemID(), attr);
    }
    if (m_notify) {
        std::map<Attr, TauCap>::const_iterator i = m_tauCap.find(attr);
        if (i != m_tauCap.end()) {
            // build the special list for rechargables
            PyList *l = new PyList;

            l->AddItem( PyGet( attr ) );
            l->AddItemLong( Win32TimeNow() );
            l->AddItem( _PyGet( GetReal( i->second.tau ) / 5.0 ) );
            l->AddItem( PyGet( i->second.cap ) );

            oldValue = l;
        }
        // send change
        _SendAttributeChange(attr, oldValue, new PyFloat(GetReal(attr)));
    }
}

void ItemAttributeMgr::DeleteEx(bool notify) {
    bool old_notify = m_notify;
    SetNotify(notify);
    EVEAdvancedAttributeMgr::Delete();
    SetNotify(old_notify);
}

bool ItemAttributeMgr::Load(bool notify) {
    bool old_notify = m_notify;
    SetNotify(notify);
    bool old_save = GetSave();
    SetSave(false);

    EVEAdvancedAttributeMgr::Delete();
    bool res = m_factory.db().LoadItemAttributes(item().itemID(), *this);

    SetSave(old_save);
    SetNotify(old_notify);
    return res;
}

void ItemAttributeMgr::Save() const {    /* check if we have something to save, if not return*/
    if (m_ints.empty() && m_reals.empty())
        return;
    _log(ITEM__TRACE, "Saving %lu attributes of item %u.", m_ints.size()+m_reals.size(), m_item.itemID());

    real_t v = 0;
    // integers first
    for (auto cur : m_ints) {
        v = GetReal(cur.first);
        if (_IsInt(v))
            m_factory.db().UpdateAttribute_int(m_item.itemID(), cur.first, static_cast<int32>(v));
        else
            m_factory.db().UpdateAttribute_double(m_item.itemID(), cur.first, v);
    }
    // then reals
    for (auto cur : m_reals) {
        v = GetReal(cur.first);
        if (_IsInt(v))
            m_factory.db().UpdateAttribute_int(m_item.itemID(), cur.first, static_cast<int32>(v));
        else
            m_factory.db().UpdateAttribute_double(m_item.itemID(), cur.first, v);
    }
}

void ItemAttributeMgr::EncodeAttributes(std::map<int32, PyRep *> &into) const {
    // first insert type attributes
    m_item.type().attributes.EncodeAttributes(into);
    // now insert (or overwrite) with our values
    EVEAdvancedAttributeMgr::EncodeAttributes(into);
}

void ItemAttributeMgr::_SendAttributeChange(Attr attr, PyRep *oldValue, PyRep *newValue) {
    if (!m_notify) return;

    Client *c = sEntityList.FindClientByCharID( item().ownerID() );
    if (c) {
        Notify_OnModuleAttributeChange omac;
            omac.ownerID = m_item.ownerID();
            omac.itemKey = m_item.itemID();
            omac.attributeID = attr;
            omac.time = Win32TimeNow();
            omac.oldValue = oldValue;
            omac.newValue = newValue;
        PyTuple* tmp = omac.Encode();
        c->QueueDestinyEvent(&tmp);
        _log(ITEM__WARNING, "ItemAttributeMgr::_SendAttributeChange() - old shit called.");
    } else {
        // delete the reps
        PyDecRef( oldValue );
        PyDecRef( newValue );
    }
}
#endif // 0

/************************************************************************/
/* Start of new attribute system                                        */
/************************************************************************/
AttributeMap::AttributeMap( InventoryItem & item ) : mItem(item), mChanged(true), mDefault(false)
{
}

AttributeMap::AttributeMap( InventoryItem & item, bool bDefaultMap ) : mItem(item), mChanged(true), mDefault(bDefaultMap)
{
}

bool AttributeMap::SetAttribute( uint32 attributeId, EvilNumber &num, bool notify /*= true*/ )
{
    AttrMapItr itr = mAttributes.find(attributeId);

    /* most attribute have default values which are related to the item type */
    if (itr == mAttributes.end()) {
        mAttributes.insert(std::make_pair(attributeId, num));
		mChanged = true;	// Mark the map as having been modified by a new attribute being added
        if (notify)
            Add(attributeId, num);
        return mChanged;
    }

    if (itr->second == num)
        return true;

    mChanged = true;

    // notify dogma of attribute change
    if (notify)
        Change(attributeId, itr->second, num);

    itr->second = num;
	// Mark the map as having been modified
    return mChanged;
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
        attrChange->Dump(CLIENT__TRACE, "");
        client->QueueDestinyEvent(&attrChange);
    } else {
        _log(PLAYER__WARNING, "AttributeMap::SendAttributeChanges() - ownerID for %u not found", mItem.itemID() );
        return false;
    }
    return true;
}

bool AttributeMap::ResetAttribute(uint32 attrID, bool notify) {
    EvilNumber value = mItem.GetDefaultAttribute(attrID);
    return SetAttribute(attrID, value, notify);
}

bool AttributeMap::Load() {
    /* First, we load default attributes values using existing attribute system */
    DgmTypeAttributeSet *attr_set = sDgmTypeAttrMgr.GetDmgTypeAttributeSet( mItem.typeID() );
    if (attr_set) {
        DgmTypeAttributeSet::AttrSetItr itr = attr_set->attributeset.begin();
        for (; itr != attr_set->attributeset.end(); itr++)
            SetAttribute((*itr)->attributeID, (*itr)->number, false);
    }
    /* Then we load the saved attributes from the db, if there are any yet, and overwrite the defaults */
    DBQueryResult res;
    if (mDefault) {
        if (!sDatabase.RunQuery(res, "SELECT * FROM entity_default_attributes WHERE itemID='%u'", mItem.itemID())) {
            _log(DATABASE__ERROR, "AttributeMap (DEFAULT)", "Error in db load query: %s", res.error.c_str());
            return false;
        }
    } else {
        if (!sDatabase.RunQuery(res, "SELECT * FROM entity_attributes WHERE itemID='%u'", mItem.itemID())) {
            _log(DATABASE__ERROR, "AttributeMap", "Error in db load query: %s", res.error.c_str());
            return false;
        }
    }
    DBResultRow row;
    EvilNumber attr_value = 0;
    uint32 attributeID = 0;
    int amount = res.GetRowCount();
    for (int i = 0; i < amount; i++) {
        res.GetRow(row);
        attributeID = row.GetUInt(1);
        if (row.IsNull(2))
            attr_value = row.GetDouble(3);
        else
            attr_value = row.GetInt64(2);
        SetAttribute(attributeID, attr_value, false);
    }
    return true;
}

bool AttributeMap::SaveIntAttribute(uint32 attributeID, int64 value)
{
    // SAVE INTEGER ATTRIBUTE
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "REPLACE INTO ";

    if (mDefault)
        Inserts << "entity_default_attributes ";
    else
        Inserts << "entity_attributes ";

    Inserts << "(itemID, attributeID, valueInt, valueFloat) VALUES (";
    Inserts << mItem.itemID() << ", " << attributeID << ", ";
    Inserts << value << ", NULL " << ")";

    DBerror err;
    if (!sDatabase.RunQuery(err, Inserts.str().c_str())) {
        _log(DATABASE__ERROR, "AttributeMap - unable to save float attributes");
        return false;
    }

    return true;
}

bool AttributeMap::SaveFloatAttribute(uint32 attributeID, double value)
{
    // SAVE FLOAT ATTRIBUTE
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "REPLACE INTO ";

	if (mDefault)
        Inserts << "entity_default_attributes ";
	else
        Inserts << "entity_attributes ";

    Inserts << "(itemID, attributeID, valueInt, valueFloat) VALUES (";
    Inserts << mItem.itemID() << ", " << attributeID;
    Inserts << ", NULL, " << value << ")";

    DBerror err;
    if (!sDatabase.RunQuery(err, Inserts.str().c_str())) {
        _log(DATABASE__ERROR, "AttributeMap - unable to save float attributes");
        return false;
    }

    return true;
}

/* hmmm only save 'state' related attributes... and calculate the rest on the fly....*/
/* we should save skills */
bool AttributeMap::Save() {
	bool success = false;

    /* if nothing changed... it means this action has been successful we return true... */
    if (!mChanged)
        return true;

    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO ";
    // set the appropriate table name.
    if (mDefault)
        Inserts << "entity_default_attributes";
    else
        Inserts << "entity_attributes";
    Inserts << " (itemID, attributeID, valueInt, valueFloat) ";
    bool first = true;
    AttrMapItr itr = mAttributes.begin();
    for (; itr != mAttributes.end(); itr++) {
        // if this is the first row specify the VALUES keyword
        if (first) {
            Inserts << "VALUES";
            first = false;
        }
        // otherwise comma separate the values.
        else
            Inserts << ", ";
        // itemID and attributeID keys.
        Inserts << "(" << mItem.itemID() << ", " << itr->first << ", ";
        // the value to set.
        if ( itr->second.get_type() == evil_number_int ) {
            Inserts << itr->second.get_int() << ", NULL)";
        } else {
            Inserts << " NULL, " << itr->second.get_float() << ")";
        }
    }
    // did we get at least 1 insert?
    if (!first) {
        // finish creating the command.
        Inserts << "ON DUPLICATE KEY UPDATE ";
        Inserts << "valueInt=VALUES(valueInt), ";
        Inserts << "valueFloat=VALUES(valueFloat)";
        // execute the command.
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str())) {
            _log(DATABASE__ERROR, "AttributeMap - unable to save attributes");
            return false;
        }
    }

    mChanged = false;
    return true;
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
            Inserts << " NULL, " << cur->second.get_float() << ")";
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
            Inserts << " NULL, " << cur->second.get_float() << ")";
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
            Inserts << " NULL, " << cur->second.get_float() << ")";
        }
    }

    if (shield or armor or hull) {
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str())) {
            _log(DATABASE__ERROR, "SaveShipState - unable to save attributes");
        }
    }
}

bool AttributeMap::Delete() {
    // Remove all attributes from the entity_default_attributes table or entity_attributes table for this item:
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "DELETE FROM ";
    // set the appropriate table name.
    if (mDefault)
        Inserts << "entity_default_attributes";
    else
        Inserts << "entity_attributes";
    Inserts << " WHERE itemID = " << mItem.itemID();

    // execute the command.
    DBerror err;
    if (!sDatabase.RunQuery(err, Inserts.str().c_str())) {
        _log(DATABASE__ERROR, "AttributeMap - unable to delete attributes");
        return false;
    }

	mAttributes.clear();
	mChanged = false; // just synced with database, no need to save
    return true;
}

bool AttributeMap::DeleteAttribute(uint32 attributeID) {
    DBerror err;
    if (!sDatabase.RunQuery(err, "DELETE FROM entity_default_attributes WHERE itemID = %u AND attributeID = %u", mItem.itemID(), attributeID)) {
        _log(DATABASE__ERROR, "AttributeMap - unable to delete attributeID %u for itemID %u", attributeID, mItem.itemID());
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