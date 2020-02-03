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
    Author:        Allan
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "inventory/Voucher.h"
#include "system/BookmarkDB.h"

PyCallable_Make_InnerDispatcher(VoucherService)

VoucherService::VoucherService(PyServiceMgr *mgr)
: PyService(mgr, "voucher"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(VoucherService, GetObject);
}

VoucherService::~VoucherService() {
    delete m_dispatch;
}

PyResult VoucherService::Handle_GetObject( PyCallArgs& call ) {
  /**
    voucher = self.GetVoucherSvc().GetObject(voucherID)
    if voucher is None:
        return
    self.data[voucherID] = voucher
    */

    //call.Dump(BOOKMARK__CALL_DUMP);
    // return none for now, to allow client to use default name of 'bookmark'
    //return PyStatic.NewNone();

    PyDict* dict = new PyDict();
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return dict;
    }
    InventoryItemRef iRef = sItemFactory.GetItem(arg.arg);
    if (iRef.get() == nullptr) {
        codelog(ITEM__ERROR, "%s: Failed to retrieve bookmark voucher for bmID %u", call.client->GetName(), arg.arg);
        return dict;
    }

    // this is how i return objects for method chaining
    //we just bind up a new voucher object for item requested and give it back to them.
    VoucherBound *vb = new VoucherBound(m_manager, iRef);
    PyRep *result = m_manager->BindObject(call.client, vb );
    return result;

    // this isnt working right...
    PyList* header = new PyList();
        header->AddItemString("itemID");
        header->AddItemString("typeID");
        header->AddItemString("ownerID");
        header->AddItemString("locationID");
        header->AddItemString("flagID");
        header->AddItemString("quantity");
        header->AddItemString("groupID");
        header->AddItemString("categoryID");
        header->AddItemString("customInfo");
    dict->SetItemString("header", header);
    PyDict* data = new PyDict();
        data->SetItemString( "itemID",       new PyInt(iRef->itemID()));
        data->SetItemString( "typeID",       new PyInt(iRef->type().id()));
        data->SetItemString( "ownerID",      new PyInt(iRef->ownerID()));
        data->SetItemString( "locationID",   new PyInt(iRef->locationID()));
        data->SetItemString( "flagID",       new PyInt(iRef->flag()));
        data->SetItemString( "quantity",     new PyInt(iRef->quantity()));
        data->SetItemString( "groupID",      new PyInt(iRef->type().groupID()));
        data->SetItemString( "categoryID",   new PyInt(iRef->type().categoryID()));
        data->SetItemString( "customInfo",   new PyString(iRef->customInfo()));
    dict->SetItemString("data", data );

    dict->SetItemString("description",   new PyString(BookmarkDB::GetBookmarkName(atoi(iRef->customInfo().c_str()))));

    return new PyObject("util.Row", dict);
    /*
     * /client/script/ui/shared/container.py(514) SortIconsBy
     * /client/script/ui/util/uix.py(283) GetItemName
     * /../carbon/common/script/sys/row.py(33) __getattr__
     *        self = <error printing value: KeyError('line',)>        name = 'GetDescription'
     * AttributeError: GetDescription
     */
}


PyCallable_Make_InnerDispatcher(VoucherBound)

VoucherBound::VoucherBound(PyServiceMgr* mgr, InventoryItemRef itemRef)
: PyBoundObject(mgr),
m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    m_strBoundObjectName = "VoucherBound";

    m_itemRef = itemRef;

    PyCallable_REG_CALL(VoucherBound, GetDescription);
}

VoucherBound::~VoucherBound()
{
    delete m_dispatch;
}

PyResult VoucherBound::Handle_GetDescription(PyCallArgs &call) {
    /*   name = voucher.GetDescription()
        name, _desc = sm.GetService('addressbook').UnzipMemo(voucher.GetDescription())
        */

    // get bookmark name (memo) as stored in db.  item.customInfo is bookmarkID this item is copied from
    return new PyString(BookmarkDB::GetBookmarkName(atoi(m_itemRef->customInfo().c_str())));

    // this gives error in client ui/control/editplaintext.py(221) InsertText
    //AttributeError: 'tuple' object has no attribute 'find'
    //return BookmarkDB::GetBookmarkDescription(atoi(m_itemRef->customInfo().c_str()));
}
