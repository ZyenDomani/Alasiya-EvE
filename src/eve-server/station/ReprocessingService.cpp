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
    Updates:    Allan (rewrite)
*/

/**  @todo  update this entire file...  */
/*
 * # Manufacturing Logging:
 * MANUF__ERROR
 * MANUF__WARNING
 * MANUF__MESSAGE
 * MANUF__INFO
 * MANUF__DEBUG
 * MANUF__TRACE
 * MANUF__DUMP
 */


#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "packets/Manufacturing.h"
#include "manufacturing/RamMethods.h"
#include "station/ReprocessingService.h"
#include "Station.h"
#include "system/SystemManager.h"

class ReprocessingServiceBound
: public PyBoundObject
{
public:
    ReprocessingServiceBound(PyServiceMgr *mgr, ReprocessingDB& db, uint32 stationID);
    virtual ~ReprocessingServiceBound();

    PyCallable_DECL_CALL(GetOptionsForItemTypes);
    PyCallable_DECL_CALL(GetReprocessingInfo);
    PyCallable_DECL_CALL(GetQuote);
    PyCallable_DECL_CALL(GetQuotes);
    PyCallable_DECL_CALL(Reprocess);

    virtual void Release();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    ReprocessingDB& m_db;

    StationItemRef m_station;
	uint32 m_stationCorpID; //corp that owns station. Used for standing
    float m_staEfficiency;
    float m_tax;

    float CalcReprocessingEfficiency(const Client *pClient, InventoryItemRef item = InventoryItemRef(nullptr)) const;
    float CalcTax(float standing) const;
    PyRep* GetQuote(uint32 itemID, Client* pClient);

    float GetStanding(const Client* pClient) const; // gets the higher of char/corp standings with station owner
};

PyCallable_Make_InnerDispatcher(ReprocessingService)
PyCallable_Make_InnerDispatcher(ReprocessingServiceBound)

ReprocessingService::ReprocessingService(PyServiceMgr *mgr)
: PyService(mgr, "reprocessingSvc"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);
}

ReprocessingService::~ReprocessingService() {
    delete m_dispatch;
}

PyBoundObject *ReprocessingService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
    if (!bind_args->IsInt()) {
        codelog(CLIENT__ERROR, "%s: Non-integer bind argument '%s'", c->GetName(), bind_args->TypeString());
        return nullptr;
    }

    uint32 stationID = bind_args->AsInt()->value();
    if (!IsStation(stationID)) {
        codelog(CLIENT__ERROR, "%s: Expected stationID, but got %u.", c->GetName(), stationID);
        return nullptr;
    }

    return new ReprocessingServiceBound(m_manager, m_db, stationID);
}


ReprocessingServiceBound::ReprocessingServiceBound(PyServiceMgr *mgr, ReprocessingDB& db, uint32 stationID)
: PyBoundObject(mgr),
  m_dispatch(new Dispatcher(this)),
  m_db(db)
{
    _SetCallDispatcher(m_dispatch);

    m_strBoundObjectName = "ReprocessingServiceBound";

    PyCallable_REG_CALL(ReprocessingServiceBound, GetOptionsForItemTypes);
    PyCallable_REG_CALL(ReprocessingServiceBound, GetReprocessingInfo);
    PyCallable_REG_CALL(ReprocessingServiceBound, GetQuote);
    PyCallable_REG_CALL(ReprocessingServiceBound, GetQuotes);
    PyCallable_REG_CALL(ReprocessingServiceBound, Reprocess);

    m_station = sItemFactory.GetStation(stationID);
    m_station->GetRefineData(m_stationCorpID, m_staEfficiency, m_tax);
}

ReprocessingServiceBound::~ReprocessingServiceBound() {
    delete m_dispatch;
}

void ReprocessingServiceBound::Release() {
    //I hate this statement
    delete this;
}

PyResult ReprocessingServiceBound::Handle_GetOptionsForItemTypes(PyCallArgs &call) {
    Call_GetOptionsForItemTypes args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    Rsp_GetOptionsForItemTypes      rsp;
    Rsp_GetOptionsForItemTypes_Arg  arg;

    // this data may be in static...nope.  and not sure i can do this better....
    for (auto cur : args.typeIDs) {
        arg.isRecyclable = m_db.IsRecyclable(cur.first);
        arg.isRefinable = m_db.IsRefinable(cur.first);
        rsp.typeIDs[cur.first] = arg.Encode();
    }

    return rsp.Encode();
}

PyResult ReprocessingServiceBound::Handle_GetReprocessingInfo(PyCallArgs &call) {
    Client *pClient = call.client;
    Rsp_GetReprocessingInfo rsp;
        rsp.standing = GetStanding(pClient);
        rsp.tax = CalcTax( rsp.standing );
        rsp.yield = m_staEfficiency;
        rsp.combinedyield = CalcReprocessingEfficiency(pClient);
    return rsp.Encode();
}

PyResult ReprocessingServiceBound::Handle_GetQuote(PyCallArgs &call) {
    Call_SingleIntegerArg arg;    // itemID
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    return GetQuote(arg.arg, call.client);
}

PyResult ReprocessingServiceBound::Handle_GetQuotes(PyCallArgs &call) {
    // why shipID here?  processing in cap indy ships?
     Call_GetQuotes args;
     if (!args.Decode(&call.tuple)) {
         codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
         return nullptr;
     }

    Rsp_GetQuotes rsp;
    for (auto cur : args.itemIDs) {
        PyRep* quote = GetQuote(cur, call.client);
        if (quote != nullptr)
            rsp.quotes[cur] = quote;
    }

    return rsp.Encode();
}

PyResult ReprocessingServiceBound::Handle_Reprocess(PyCallArgs &call) {
    if (!IsStation(call.client->GetLocationID())) {
        _log(SERVICE__MESSAGE, "Character %s tried to reprocess, but isn't is station.", call.client->GetName());
        return nullptr;
    }

    Call_Reprocess args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    if (args.ownerID == 0)
        args.ownerID = call.client->GetCharacterID();

    if (args.flag == flagAutoFit)
        args.flag = flagHangar;

    if (args.ownerID == call.client->GetCorporationID()) {
        int64 roles = call.client->GetCorpRole();
        if (roles & Corp::Role::FactoryManager != Corp::Role::FactoryManager) {
            _log(MANUF__WARNING, "%s(%u) doesnt have FactoryManager role to access materials for reprocessing.", \
                        call.client->GetName(), call.client->GetCharacterID());
            call.client->SendErrorMsg("You do not have the role \'Factory Manager\' which is required to access factory services on behalf of a corporation.");
            //throw error here...dunno the format yet.
            return nullptr;
        }

        if ((args.flag == flagHangar and (roles & Corp::Role::HangarCanTake1) != Corp::Role::HangarCanTake1)
        or  (args.flag == flagCorpHangar2 and (roles & Corp::Role::HangarCanTake2) != Corp::Role::HangarCanTake2)
        or  (args.flag == flagCorpHangar3 and (roles & Corp::Role::HangarCanTake3) != Corp::Role::HangarCanTake3)
        or  (args.flag == flagCorpHangar4 and (roles & Corp::Role::HangarCanTake4) != Corp::Role::HangarCanTake4)
        or  (args.flag == flagCorpHangar5 and (roles & Corp::Role::HangarCanTake5) != Corp::Role::HangarCanTake5)
        or  (args.flag == flagCorpHangar6 and (roles & Corp::Role::HangarCanTake6) != Corp::Role::HangarCanTake6)
        or  (args.flag == flagCorpHangar7 and (roles & Corp::Role::HangarCanTake7) != Corp::Role::HangarCanTake7))
            _log(MANUF__WARNING, "%s(%u) tried to reprocess items they are not allowed to access.", call.client->GetName(), call.client->GetCharacterID());
            call.client->SendErrorMsg("You do not have the role required to access the materials in this hangar.");
            //throw error here...dunno the format yet.
            return nullptr;
    }

    double tax = CalcTax(GetStanding(call.client));
    InventoryItemRef iRef = InventoryItemRef(nullptr);
    for (auto cur : args.items)  {
        iRef = sItemFactory.GetItem(cur);
        if (iRef.get() == nullptr)
            continue;

        // this should never happen, but for sure ...
        if (iRef->type().portionSize() > iRef->quantity()) {
            std::map<std::string, PyRep *> args;
            args["typename"] = new PyString(iRef->itemName().c_str());
            args["portion"] = new PyInt(iRef->type().portionSize());
            throw(PyException(MakeUserError("QuantityLessThanMinimumPortion", args)));
        }

        double efficiency = CalcReprocessingEfficiency( call.client, iRef );

        // dont hit db for this shit...we kinda have to....dont have this data in static shit.
        std::vector<Recoverable> recoverables;
        if ( !m_db.GetRecoverables( iRef->typeID(), recoverables ) )
            continue;

        std::vector<Recoverable>::iterator cur_rec = recoverables.begin();
        for (; cur_rec != recoverables.end(); cur_rec++) {
			uint32 full = cur_rec->amountPerBatch * iRef->quantity() / iRef->type().portionSize();
            uint32 quantity = uint32(full * efficiency * (1.0 - tax) );
			if (quantity == 0)
                continue;

            ItemData idata(cur_rec->typeID, args.ownerID, 0, flagAutoFit, quantity);
            InventoryItemRef iRef2 = sItemFactory.SpawnItem( idata );
            if (iRef2.get() == nullptr)
                continue;

            // update this for corp usage
            iRef2->Move(m_station->GetID(), (EVEItemFlags)args.flag, true);
        }

        uint32 qtyLeft = iRef->quantity() % iRef->type().portionSize();
        if (qtyLeft)
            iRef->SetQuantity(qtyLeft, true);
        else {
            iRef->Move(iRef->locationID(), flagJunkyardReprocessed, true);
            m_station->RemoveItem(iRef);
            iRef->Delete();
        }
    }

    return nullptr;
}

float ReprocessingServiceBound::CalcReprocessingEfficiency(const Client* pClient, InventoryItemRef item) const {
    /* formula is:
        reprocessingEfficiency = 0.375
        *(1 + 0.02 * RefiningSkill)
        *(1 + 0.04 * RefineryEfficiencySkill)
        *(1 + 0.05 * OreProcessingSkill)
    */
    /** @todo  check for implants here ... once they're working  */
    CharacterRef cRef = pClient->GetChar();
    double efficiency =  (0.375
                        * (1 + (0.02 * cRef->GetSkillLevel(skillRefining)))
                        * (1 + (0.04 * cRef->GetSkillLevel(skillRefineryEfficiency))));

    if (item.get() != nullptr) {
        uint32 specificSkill = item->GetAttribute(AttrReprocessingSkillType).get_int();
        if (specificSkill)
            efficiency *= (1 + 0.05 * cRef->GetSkillLevel(specificSkill));
        else
            efficiency *= (1 + 0.05 * cRef->GetSkillLevel(skillScrapmetalProcessing));    // use Scrapmetal Processing as default
    }

    efficiency += m_staEfficiency;

    if (efficiency > 1)
        efficiency = 1.05;  // should be 1.0 max

    return efficiency;
}

PyRep *ReprocessingServiceBound::GetQuote(uint32 itemID, Client* pClient) {
    InventoryItemRef iRef = sItemFactory.GetItem( itemID );
    if (iRef.get() == nullptr)
        return nullptr;    // No action as GetQuote is also called for reprocessed items (probably for check)

    // update this for corp items
    if (iRef->ownerID() == pClient->GetCorporationID()) {
        /** @todo update this for item location - need to verify corp roles are being set correctly  */
        int64 roles = pClient->GetRolesAtAll();
        //roles = pClient->GetRolesAtBase() | pClient->GetRolesAtAll();
        //roles = pClient->GetRolesAtHQ() | pClient->GetRolesAtAll();
        //roles = pClient->GetRolesAtOther() | pClient->GetRolesAtAll();
        if (pClient->GetCorpRole() & Corp::Role::FactoryManager != Corp::Role::FactoryManager) {
            _log(MANUF__WARNING, "%s(%u) doesnt have FactoryManager role to access materials for reprocessing.", \
                    pClient->GetName(), pClient->GetCharacterID());
            pClient->SendErrorMsg("You do not have the role \'Factory Manager\' which is required to access factory services on behalf of a corporation.");
            //throw error here...dunno the format yet.
            return nullptr;
        }

        if ((iRef->flag() == flagHangar and (roles & Corp::Role::HangarCanTake1) != Corp::Role::HangarCanTake1)
        or  (iRef->flag() == flagCorpHangar2 and (roles & Corp::Role::HangarCanTake2) != Corp::Role::HangarCanTake2)
        or  (iRef->flag() == flagCorpHangar3 and (roles & Corp::Role::HangarCanTake3) != Corp::Role::HangarCanTake3)
        or  (iRef->flag() == flagCorpHangar4 and (roles & Corp::Role::HangarCanTake4) != Corp::Role::HangarCanTake4)
        or  (iRef->flag() == flagCorpHangar5 and (roles & Corp::Role::HangarCanTake5) != Corp::Role::HangarCanTake5)
        or  (iRef->flag() == flagCorpHangar6 and (roles & Corp::Role::HangarCanTake6) != Corp::Role::HangarCanTake6)
        or  (iRef->flag() == flagCorpHangar7 and (roles & Corp::Role::HangarCanTake7) != Corp::Role::HangarCanTake7))
            _log(MANUF__WARNING, "%s(%u) tried to reprocess items they are not allowed to access.", \
                    pClient->GetName(), pClient->GetCharacterID());
            pClient->SendErrorMsg("You do not have the role required to access the materials in this hangar.");
            //throw error here...dunno the format yet.
            return nullptr;
    } else if (iRef->ownerID() != pClient->GetCharacterID()) {
        _log(SERVICE__ERROR, "Character %u tried to reprocess item %u of character %u.", pClient->GetCharacterID(), iRef->itemID(), iRef->ownerID());
        pClient->SendErrorMsg("The requested item is not yours.");
        return nullptr;
    }

    Rsp_GetQuote quote;
    quote.lines = new PyList();
    quote.leftOvers = iRef->quantity() % iRef->type().portionSize();
    quote.quantityToProcess = iRef->quantity() - quote.leftOvers;
    quote.playerStanding = GetStanding(pClient);

    double tax = CalcTax( quote.playerStanding );

    if (iRef->quantity() >= iRef->type().portionSize()) {
        std::vector<Recoverable> recoverables;
        if (!m_db.GetRecoverables( iRef->typeID(), recoverables))
            return nullptr;

        double efficiency = CalcReprocessingEfficiency(pClient, iRef);

        for (auto cur :recoverables) {
            uint32 ratio = cur.amountPerBatch * quote.quantityToProcess / iRef->type().portionSize();
            Rsp_GetQuote_Recoverables_Line line;
                line.typeID			= cur.typeID;
                line.client			= uint32(efficiency * (1.0 - tax)   * ratio);
                line.station		= uint32(efficiency * tax           * ratio);
                line.unrecoverable	= ratio - line.client - line.station;
            quote.lines->AddItem( line.Encode() );
        }
    } else {
        std::map<std::string, PyRep *> args;
        args["typename"] = new PyString(iRef->itemName().c_str());
        args["portion"] = new PyInt(iRef->type().portionSize());
        throw(PyException(MakeUserError("QuantityLessThanMinimumPortion", args)));
    }

    return quote.Encode();
}

float ReprocessingServiceBound::GetStanding(const Client* pClient) const
{
    float standing = pClient->GetChar()->GetStanding(m_stationCorpID, pClient->GetCharacterID());
    if (standing < 0)
        standing += ((10 +standing) * 0.04 * pClient->GetChar()->GetSkillLevel(skillDiplomacy));
    else
        standing += ((10 -standing) * 0.04 * pClient->GetChar()->GetSkillLevel(skillConnections));

    return EvE::max(standing, pClient->GetChar()->GetStanding(m_stationCorpID, pClient->GetCorporationID()));
}

float ReprocessingServiceBound::CalcTax(float standing) const {
    //EvEMath::Refine::StationTaxesForReprocessing(standing);
    double tax = m_tax - 0.75/100 * standing;
    if (tax < 0)
        tax = 0;
    return tax;
}