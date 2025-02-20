/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
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
    Author:        Zhur
*/

#ifndef __MARKETPROXY_SERVICE_H_INCL__
#define __MARKETPROXY_SERVICE_H_INCL__

#include "market/MarketDB.h"
#include "PyService.h"

class MarketProxyService
: public PyService
{
public:
    MarketProxyService(PyServiceMgr *mgr);
    virtual ~MarketProxyService();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    MarketDB m_db;

    PyCallable_DECL_CALL(GetStationAsks);
    PyCallable_DECL_CALL(GetSystemAsks);
    PyCallable_DECL_CALL(GetRegionBest);
    PyCallable_DECL_CALL(GetMarketGroups);
    PyCallable_DECL_CALL(GetOrders);
    PyCallable_DECL_CALL(GetOldPriceHistory);
    PyCallable_DECL_CALL(GetNewPriceHistory);
    PyCallable_DECL_CALL(PlaceCharOrder);
    PyCallable_DECL_CALL(GetCharOrders);
    PyCallable_DECL_CALL(ModifyCharOrder);
    PyCallable_DECL_CALL(CancelCharOrder);
    PyCallable_DECL_CALL(CharGetNewTransactions);
    PyCallable_DECL_CALL(CorpGetNewTransactions);
    PyCallable_DECL_CALL(StartupCheck);
    PyCallable_DECL_CALL(GetCorporationOrders);
};

#endif
/*
 *    def GetSkillLimits(self):
 *        limits = {}
 *        currentOpen = 0
 *        myskills = sm.GetService('skills').MySkillLevelsByID()
 *        retailLevel = myskills.get(const.typeRetail, 0)
 *        tradeLevel = myskills.get(const.typeTrade, 0)
 *        wholeSaleLevel = myskills.get(const.typeWholesale, 0)
 *        accountingLevel = myskills.get(const.typeAccounting, 0)
 *        brokerLevel = myskills.get(const.typeBrokerRelations, 0)
 *        tycoonLevel = myskills.get(const.typeTycoon, 0)
 *        marginTradingLevel = myskills.get(const.typeMarginTrading, 0)
 *        marketingLevel = myskills.get(const.typeMarketing, 0)
 *        procurementLevel = myskills.get(const.typeProcurement, 0)
 *        visibilityLevel = myskills.get(const.typeVisibility, 0)
 *        daytradingLevel = myskills.get(const.typeDaytrading, 0)
 *        maxOrderCount = 5 + tradeLevel * 4 + retailLevel * 8 + wholeSaleLevel * 16 + tycoonLevel * 32
 *        limits['cnt'] = maxOrderCount
 *        commissionPercentage = const.marketCommissionPercentage / 100.0
 *        commissionPercentage *= 1 - brokerLevel * 0.05
 *        transactionTax = const.mktTransactionTax / 100.0
 *        transactionTax *= 1 - accountingLevel * 0.1
 *        limits['fee'] = commissionPercentage
 *        limits['acc'] = transactionTax
 *        limits['ask'] = jumpsPerSkillLevel[marketingLevel]
 *        limits['bid'] = jumpsPerSkillLevel[procurementLevel]
 *        limits['vis'] = jumpsPerSkillLevel[visibilityLevel]
 *        limits['mod'] = jumpsPerSkillLevel[daytradingLevel]
 *        limits['esc'] = 0.75 ** marginTradingLevel
 *        return limits
 */