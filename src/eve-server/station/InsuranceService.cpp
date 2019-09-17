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

#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "Client.h"
#include "account/AccountService.h"
#include "chat/LSCService.h"
#include "station/InsuranceService.h"
#include "system/SystemEntity.h"
#include "system/SystemManager.h"

/* TODO:
* - set ship->basePrice to use history market value from marketProxy!
* - send eveMail detailing coverage on InsureShip
*/

class InsuranceBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(InsuranceBound)

    InsuranceBound(PyServiceMgr *mgr, ShipDB* db)
    : PyBoundObject(mgr),
      m_db(db),
      m_dispatch(new Dispatcher(this)) {
        _SetCallDispatcher(m_dispatch);

        PyCallable_REG_CALL(InsuranceBound, InsureShip);
        PyCallable_REG_CALL(InsuranceBound, UnInsureShip);
        PyCallable_REG_CALL(InsuranceBound, GetContracts);
        PyCallable_REG_CALL(InsuranceBound, GetInsurancePrice);

        m_strBoundObjectName = "InsuranceBound";
    }

    virtual ~InsuranceBound() { delete m_dispatch; }
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(InsureShip);
    PyCallable_DECL_CALL(UnInsureShip);
    PyCallable_DECL_CALL(GetContracts);
    PyCallable_DECL_CALL(GetInsurancePrice);

protected:
    ShipDB* m_db;
    Dispatcher *const m_dispatch;
};

PyCallable_Make_InnerDispatcher(InsuranceService)

InsuranceService::InsuranceService(PyServiceMgr *mgr)
: PyService(mgr, "insuranceSvc"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(InsuranceService, GetContractForShip);
    PyCallable_REG_CALL(InsuranceService, GetInsurancePrice);
	//SetSessionCheck?
}

InsuranceService::~InsuranceService() {
    delete m_dispatch;
}

PyBoundObject* InsuranceService::_CreateBoundObject( Client* c, const PyRep* bind_args ) {
    return new InsuranceBound( m_manager, &m_db );
}

PyResult InsuranceService::Handle_GetInsurancePrice( PyCallArgs& call ) {
    /* called in space */
    const ItemType *type = sItemFactory.GetType(PyRep::IntegerValue(call.tuple->GetItem(0)));
    if (type != nullptr)
        return new PyFloat(type->basePrice() /15);

    return PyStatic.NewNone();
}

PyResult InsuranceBound::Handle_GetInsurancePrice( PyCallArgs& call ) {
    /* called when docked */
    const ItemType *type = sItemFactory.GetType(PyRep::IntegerValue(call.tuple->GetItem(0)));
    if (type != nullptr)
        return new PyFloat(type->basePrice() /15);

    return PyStatic.NewNone();
}

PyResult InsuranceBound::Handle_GetContracts( PyCallArgs& call ) {
    if (call.tuple->size() > 1) {
        Call_IntBoolArg args;
        if (!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
            return nullptr;
        }

        return m_db->GetInsuranceByOwnerID(call.client->GetCorporationID());
    }

    return m_db->GetInsuranceByOwnerID(call.client->GetCharacterID());
}

PyResult InsuranceService::Handle_GetContractForShip( PyCallArgs& call ) {
    return m_db.GetInsuranceByShipID(call.tuple->GetItem(0)->AsInt()->value());
}

PyResult InsuranceBound::Handle_InsureShip( PyCallArgs& call ) {
	Call_InsureShip args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    InventoryItemRef shipRef = sItemFactory.GetItem(args.shipID);
    if (shipRef.get() == nullptr)
        return nullptr;

    /* added check for groupID 237 (rookie ship - items 588, 596, 601, 606) as they cannot be insured.
     *   may not need this, as client will not try to insure rookie ships
     */
    if (shipRef->groupID() == EVEDB::invGroups::Rookieship) {
        call.client->SendInfoModalMsg("You cannot insure Rookie ships.");
        return PyStatic.NewNone();
    } // end rookie ship check

    /* INSURANCE FRACTION TABLE:
     *    Label    Fraction  Payment
     *   --------  --------  ------
     *   Platinum   1.0       0.3
     *   Gold       0.9       0.25
     *   Silver     0.8       0.2
     *   Bronze     0.7       0.15
     *   Standard   0.6       0.1
     *   Basic      0.5       0.05
     */
    // calculate the fraction value
    double paymentFraction = (args.amount / (shipRef->type().basePrice() /15));
    if (paymentFraction < 0.05) {
            // catchall for fuckedup prices.
        call.client->SendErrorMsg("Your payment of %.2f is below the minimum payment of %.2f required for coverage.", \
                    args.amount, (shipRef->type().basePrice() /15) * 0.05);
        return PyStatic.NewNone();
    }

    float fraction = 0.0f;  // with no insurance, SCC pays 40%
    if (paymentFraction == 0.05)
        fraction = 0.5f;
    else if (paymentFraction == 0.1)
        fraction = 0.6f;
    else if (paymentFraction == 0.15)
        fraction = 0.7f;
    else if (paymentFraction == 0.2)
        fraction = 0.8f;
    else if (paymentFraction == 0.25)
        fraction = 0.9f;
    else if (paymentFraction == 0.3)
        fraction = 1.0f;

    if (fraction < 0.05) {
        call.client->SendErrorMsg("There was a problem with your insurance premium calculation.  Ref: ServerError 75520.");
        throw PyException(MakeUserError("InsureShipFailed"));
    } else if (fraction == 0.3)
        call.client->SendErrorMsg("Your insurance is at minimum coverage due to incorrect base prices.  Ref: ServerError 75521.");

    if (m_db->IsShipInsured(args.shipID)) {     //this hits db...can you insure unloaded ship? (if no, make this a ship memobj)
        if (call.byname.find("voidOld") != call.byname.end()) {
            if (call.byname.find("voidOld")->second->AsBool()->value())
                m_db->DeleteInsuranceByShipID(args.shipID);
        } else  // this will send voidOld=true after asking player to cancel old insurance
            throw PyException(MakeUserError("InsureShipFailedSingleContract"));
    }

    uint8 numWeeks = 12;

    if (m_db->InsertInsuranceByShipID(args.shipID, shipRef->itemName().c_str(), call.client->GetCharacterID(), fraction, shipRef->type().basePrice(), args.isCorp, numWeeks)) {
        //  it sucessfully added, now, have the player pay for the insurance
        std::string reason = "Insurance Premium on ";
        reason += call.client->GetShip()->itemName();
        reason += ".  Reference ID : xxxxx";     // put contractID here
        AccountService::TranserFunds(call.client->GetCharacterID(), ownerSCC, args.amount, reason, Journal::EntryType::Insurance);
	} else {
        //call.client->SendErrorMsg("Failed to install new insurance contract.");
        throw PyException(MakeUserError("InsureShipFailed"));
    }

    // TODO:  send mail detailing insurance coverage and length of coverage

    const std::string subject = "New Ship Insurance"; //std::string("New application from ") + call.client->GetName(),
    std::string body = "Dear valued customer,<br>";
            body += "Congratulations on the insurance on your ship. A very wise choice indeed.<br>";
            body += "This letter is to confirm that we have issued an insurance contract for your ship, ";
            body += shipRef->itemName();
            body += ", at level of ";
            body += itoa(fraction *100);
            body += "%.<br>This contract will expire after 12 weeks.<br><br>";   //*insert endDate Here*,
            body += "Best,<br>";
            body += "The Secure Commerce Commission,<br><br>";
            body += "Reference ID: xxxxx <br>"; // put contractID here
            body += "jav";

    m_manager->lsc_service->SendMail(ownerSCC, call.client->GetCharacterID(), subject, body);

    return m_db->GetInsuranceByShipID(args.shipID);
}

PyResult InsuranceBound::Handle_UnInsureShip( PyCallArgs& call ) {
    m_db->DeleteInsuranceByShipID(call.tuple->GetItem(0)->AsInt()->value());
    return PyStatic.NewNone();
}