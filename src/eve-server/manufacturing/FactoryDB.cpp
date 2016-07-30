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
*/

#include "eve-server.h"

#include "manufacturing/FactoryDB.h"

PyRep *FactoryDB::GetMaterialsForTypeWithActivity(const uint32 blueprintTypeID) const {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
                "SELECT requiredTypeID, quantity, damagePerJob, activityID AS activity"
				" FROM ramTypeRequirements"
                " WHERE typeID IN (%u, (SELECT productTypeID FROM bpTypes WHERE blueprintTypeID = %u))",
                blueprintTypeID, blueprintTypeID))
    {
        _log(DATABASE__ERROR, "Could not retrieve materials for type %u : %s", blueprintTypeID, res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);

    // should be indexed rowset, but may have to build the packet here, as we are missing a lot of info in this return.
    //return DBResultToIndexRowset(res, "activity");
    /*
     *      bomByActivity = sm.RemoteSvc('factory').GetMaterialsForTypeWithActivity(typeID)
     *      for activity in activities:
                indexedExtras = copy.deepcopy(bomByActivity[activity].extras).Index('requiredTypeID')
                for skill in bomByActivity[activity].skills:
                    propertyInfo = cfg.invtypes.Get(skill.requiredTypeID)
                    propertyName = propertyInfo.typeName
                    propertyValue = localization.GetByLabel('UI/InfoWindow/SkillAndLevel', skill=skill.requiredTypeID, skillLevel=skill.quantity)
                    skills.append((propertyName,
                     propertyValue,
                     skill.requiredTypeID,
                     skill.quantity))

                for material in bomByActivity[activity].rawMaterials:
                    if material.quantity <= 0:
                        continue
                    propertyInfo = cfg.invtypes.Get(material.requiredTypeID)
                    propertyName = propertyInfo.typeName
                    amountRequired = amountRequiredByPlayer = material.quantity
                    blueprintWaste = characterWaste = 0.0
                    extraAmount = 0
                    if activity in (const.activityManufacturing, const.activityDuplicating):
                        if material.requiredTypeID in indexedExtras and indexedExtras[material.requiredTypeID].quantity > 0:
                            extraAmount = indexedExtras[material.requiredTypeID].quantity
                            indexedExtras[material.requiredTypeID].quantity = 0
                            */
}

PyRep *FactoryDB::GetMaterialCompositionOfItemType(const uint32 typeID) const {
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
        return NULL;
    }

    return DBResultToRowset(res);
}

