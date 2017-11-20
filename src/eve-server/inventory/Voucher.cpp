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
    return nullptr;
    // this isnt working right....return doesnt make a "voucher" object in client, so subsquent call to "voucher.GetDescription" returns error.
    /*
     * /client/script/ui/util/uix.py(283) GetItemName
     *        invItem = <DBRow object [140006619L, 51, 140000000, 140005905, 5, 1, 24, 5, '2', 1, 0]>
     *        data = None
     *        voucher = <util.IndexRowset instance at 0x4B035E40>
     *        name = u'Bookmark'
     * AttributeError: IndexRowset instance has no attribute 'GetDescription'
     */

    _log(COMMON__INFO,  "VoucherService::Handle_GetObject", "size= %u", call.tuple->size() );
    call.Dump(COMMON__INFO);

    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }
    InventoryItemRef iRef = m_manager->item_factory->GetItem(arg.arg);
    if (iRef.get() == nullptr) {
        codelog(ITEM__ERROR, "%s: Failed to spawn bookmark voucher for bmID %u", call.client->GetName(), arg.arg);
        return nullptr;
    }

    return iRef->ItemGetInfo();
}
