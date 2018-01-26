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
    Author:        Zhur
    Updates:    Allan
*/

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "manufacturing/FactoryDB.h"


PyRep* FactoryDB::GetMaterialCompositionOfItemType(const uint32 typeID) const {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
                "SELECT requiredTypeID AS typeID, quantity"
                " FROM ramTypeRequirements"
                " WHERE typeID = (SELECT blueprintTypeID FROM invBlueprintTypes WHERE productTypeID = %u)"
                " AND activityID = 1"
                " AND damagePerJob = 1",
                typeID))
    {
        _log(DATABASE__ERROR, "Could not retrieve material composition for type %u : %s", typeID, res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

bool FactoryDB::SaveBlueprintData(uint32 blueprintID, BlueprintData& data) {
    DBerror err;
    if(!sDatabase.RunQuery(err,
        "INSERT INTO invBlueprints"
        "  (itemID, copy, materialLevel, productivityLevel, licensedProductionRunsRemaining)"
        " VALUES"
        "  (%u, %u, %i, %i, %i)"
        "ON DUPLICATE KEY UPDATE "
        "materialLevel=VALUES(materialLevel), "
        "productivityLevel=VALUES(productivityLevel), "
        "licensedProductionRunsRemaining=VALUES(licensedProductionRunsRemaining) ",
                           blueprintID, (data.copy ? 1 : 0), data.mLevel, data.pLevel, data.runs))
    {
        codelog(DATABASE__ERROR, "Error in SaveBlueprint query: %s.", err.c_str());
        return false;
    }

    return true;
}

bool FactoryDB::DeleteBlueprint(uint32 blueprintID) {
    DBerror err;

    if(!sDatabase.RunQuery(err,
        "DELETE FROM invBlueprints"
        " WHERE blueprintID=%u",
        blueprintID))
    {
        codelog(DATABASE__ERROR, "Failed to delete blueprint %u: %s.", blueprintID, err.c_str());
        return false;
    }
    return true;
}

bool FactoryDB::GetBlueprint(uint32 blueprintID, BlueprintData &into) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  copy,"
        "  materialLevel,"
        "  productivityLevel,"
        "  licensedProductionRunsRemaining"
        " FROM invBlueprints"
        " WHERE itemID=%u",
        blueprintID))
    {
        codelog(DATABASE__ERROR, "Error in GetBlueprint query: %s.", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "Blueprint %u not found.", blueprintID);
        return false;
    }

    into.copy = (row.GetInt(0) ? true : false);
    into.mLevel = row.GetInt(1);
    into.pLevel = row.GetInt(2);
    into.runs = row.GetInt(3);

    return true;
}
