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
Author: Zhur, Allan
*/

#include "../eve-server.h"

#include "PyBoundObject.h"
#include "ship/ShipDB.h"


PyRep *ShipDB::GetInsuranceByShipID(uint32 shipID) {
    DBQueryResult res;
    DBResultRow row;
    sDatabase.RunQuery(res,
        "SELECT startDate, shipName, shipID, endDate, ownerID, fraction"
		" FROM shipInsurance"
        " WHERE shipID = %u", shipID );

    if (res.GetRow(row))
        return DBRowToRow(row);

    return PyStatic.NewZero();
}

PyRep *ShipDB::GetInsuranceByOwnerID(uint32 ownerID) {
    DBQueryResult res;
    sDatabase.RunQuery(res,
        "SELECT startDate, shipName, shipID, endDate, ownerID, fraction"
		" FROM shipInsurance"
        " WHERE ownerID = %u", ownerID );

    return DBResultToRowset(res);
}

bool ShipDB::InsertInsuranceByShipID(uint32 shipID, std::string name, uint32 ownerID, float fraction, double payOut, bool isCorpItem, uint8 numWeeks) {
    DBerror err;
    return sDatabase.RunQuery(err, "INSERT INTO shipInsurance"
            " (shipID, shipName, ownerID, isCorpItem, startDate, endDate, fraction, payOutAmount)"
            " VALUES (%u, '%s', %u, %u, %lli, %lli, %.2f, %f)",
            shipID, name.c_str(), ownerID, isCorpItem, GetFileTimeNow(),
            (GetFileTimeNow() + (EvE::Time::Week * numWeeks)), fraction, payOut );
}

void ShipDB::DeleteInsuranceByShipID(uint32 shipID) {
    DBerror err;
	sDatabase.RunQuery(err, "DELETE FROM shipInsurance WHERE shipID=%u", shipID);
}

float ShipDB::GetShipInsurancePayout(uint32 shipID) {
    DBQueryResult res;
    DBResultRow row;
    sDatabase.RunQuery(res, "SELECT payOutAmount FROM shipInsurance WHERE shipID = %u", shipID);
    if (res.GetRow(row))
        return row.GetFloat(0);

    /** @todo  send mail to owner about no insurance, so limited payout. from SCC  */
    return 15000;  //default to flat 15K for no insurance.
}

bool ShipDB::IsShipInsured(uint32 shipID)
{
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT ownerID FROM shipInsurance WHERE shipID = %u", shipID );
    DBResultRow row;
    if (res.GetRow(row))
        return true;

    return false;
}

void ShipDB::ClearWeaponGroups(uint32 shipID)
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM shipWeaponGroups WHERE shipID = %u", shipID );
}

void ShipDB::LoadWeaponGroups(uint32 shipID, DBQueryResult& res)
{
    sDatabase.RunQuery(res, "SELECT masterID, slaveID FROM shipWeaponGroups WHERE shipID = %u", shipID );
}

void ShipDB::SaveWeaponGroups(uint32 shipID, std::multimap< uint32, uint32 >& data)
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM shipWeaponGroups WHERE shipID = %u", shipID );
    std::ostringstream Inserts;
    // start the insert data.
    Inserts << "INSERT INTO shipWeaponGroups";
    Inserts << " (shipID, masterID, slaveID)";
    Inserts << " VALUES ";
    bool save(false);
    for (auto &cur : data) {
        if (save) {
            Inserts << ", ";
        } else {
            save = true;
        }
        Inserts << "(" << shipID << ", " << cur.first << ", " << cur.second << ")";
    }

    if (save)
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "SaveLinkedWeapons - unable to save data - %s", err.c_str());
}
