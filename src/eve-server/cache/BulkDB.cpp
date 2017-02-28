/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2011 The EVEmu Team
 *    For the latest information visit http://evemu.org
 *    ------------------------------------------------------------------------------------
 *    This program is free software; you can redistribute it and/or modify it under
 *    the terms of the GNU Lesser General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option) any later
 *    version.
 *
 *    This program is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License along with
 *    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 *    http://www.gnu.org/copyleft/lesser.txt.
 *    ------------------------------------------------------------------------------------
 *    Author:   Allan
 */


#include "cache/BulkDB.h"


BulkDB::BulkDB()
{

}

/* updated files to send to client in bulkdata
 *
    dgmOperands = 800002,                -74
    dgmExpressions = 800003,             -17757
    dgmAttributes = 800004,              -1788
    dgmEffects = 800005,                 -3537
    dgmTypeAttributes = 800006,          -372904
    dgmTypeEffects = 800007,             -36340

    will have to split these up in chunks, hard-code chunk numbers, and keep track of the data and its relevant number
    not sure how im gonna do this yet.

    */

PyRep* BulkDB::GetOperands()
{
    DBQueryResult res;
    if( !sDatabase.RunQuery(res,
        " SELECT"
        " operandID,"
        " operandKey,"
        " description,"
        " format,"
        " arg1categoryID,"
        " arg2categoryID,"
        " resultCategoryID,"
        " pythonFormat"
        " FROM dgmOperands"))
    {
        codelog(DATABASE__ERROR, "Error in GetOperands: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToCRowset(res);
}

PyRep* BulkDB::GetDogmaAttribs()
{
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT attributeID, attributeName, attributeCategory, description, maxAttributeID, attributeIdx, chargeRechargeTimeID, defaultValue, "
        " published, displayName, unitID, stackable, highIsGood, categoryID, iconID, displayNameID, dataID "
        "FROM dgmAttributeTypes"))
    {
        _log(DATABASE__ERROR, "Error in GetDogmaAttribs: %s",res.error.c_str());
        return nullptr;
    }
    return DBResultToCRowset(res);
}

PyRep* BulkDB::GetDogmaEffects()
{
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT effectID, effectName, effectCategory, preExpression, postExpression, description, guid, iconID, "
        "isOffensive, isAssistance, durationAttributeID, trackingSpeedAttributeID, dischargeAttributeID, rangeAttributeID, "
        "falloffAttributeID, published, displayName, isWarpSafe, rangeChance, electronicChance, propulsionChance, distribution, "
        "sfxName, npcUsageChanceAttributeID, npcActivationChanceAttributeID, 0 as graphicID, fittingUsageChanceAttributeID, 0 AS dataID "
        "FROM dgmEffects"))
    {
        _log(DATABASE__ERROR, "Error in GetDogmaEffects: %s",res.error.c_str());
        return nullptr;
    }
    return DBResultToCRowset(res);
}
uint8 BulkDB::GetNumChunks(uint8 setID /*0*/)
{
    switch (setID) {
        case 0: {
            return m_chunks;
        }
        case 1: {   // dgmExpressions
            return 2;
        }
        case 2: {   // dgmTypeAttributes
            return 37;
        }
        case 3: {   // dgmTypeEffects
            return 4;
        }
    }
}
int32 BulkDB::GetFileIDfromChunk(uint8 setID, uint8 chunkID)
{
    switch (setID) {
        case 0: {
            switch (chunkID) {
                case 0:
                case 1: {
                    return 800003;  //dgmExpressions
                } break;
                case 2:
                case 3:
                case 4:
                case 5: {
                    return 800007;  //dgmTypeEffects
                } break;
                default: {
                    return 800006;  //dgmTypeAttributes
                } break;
            }
        } break;
        case 1: {
            return 800003;  //dgmExpressions
        } break;
        case 2: {
            return 800007;  //dgmTypeEffects
        } break;
        case 3: {
            return 800006;  //dgmTypeAttributes
        } break;
    }
}

PyRep* BulkDB::GetBulkDataChunks(uint8 setID, uint8 chunkID)
{
    switch (setID) {
        case 0: {
            switch (chunkID) {
                case 0:
                case 1: {
                    return GetExpressions(chunkID);
                } break;
                case 2:
                case 3:
                case 4:
                case 5: {
                    uint8 chunk = chunkID - 2;  // 0 to 4
                    return GetDogmaTypeEffects(chunk);
                } break;
                default: {
                    uint8 chunk = chunkID - 6;  // 0 to 37
                    if (chunk > 37)
                        ;   // make error here for wrong chunkID
                    return GetDogmaTypeAttribs(chunk);
                } break;
            }
        } break;
        case 1: {
            return GetExpressions(chunkID);
        } break;
        case 2: {
            return GetDogmaTypeEffects(chunkID);
        } break;
        case 3: {
            return GetDogmaTypeAttribs(chunkID);
        } break;
    }
}

PyRep* BulkDB::GetExpressions(uint8 chunkID)    // 2 chunks
{   //17757
    DBQueryResult res;
    std::ostringstream q;
    q << " SELECT expressionID, operandID, arg1, arg2, expressionValue, description, expressionName, expressionTypeID, expressionGroupID, expressionAttributeID";
    q << " FROM dgmExpressions";
    switch (chunkID) {
        case 0: {
            q << " LIMIT 0, 9000";
        } break;
        case 1: {
            q << " LIMIT 9000, 18000";
        } break;
    };

    if( !sDatabase.RunQuery(res,q.str().c_str()))  // start at row 0 and get next 8k rows
    {
        codelog(DATABASE__ERROR, "Error in GetExpressions: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToCRowset(res);
}

PyRep* BulkDB::GetDogmaTypeEffects(uint8 chunkID)   // 4 chunks
{   //36340
    DBQueryResult res;
    std::ostringstream q;
    switch (chunkID) {
        case 0: {
            q << " LIMIT 0, 10000";
        } break;
        case 1: {
            q << " LIMIT 10000, 10000";
        } break;
        case 2: {
            q << " LIMIT 20000, 10000";
        } break;
        case 3: {
            q << " LIMIT 30000, 10000";
        } break;
    };

    if(!sDatabase.RunQuery(res, "SELECT typeID,effectID,isDefault FROM dgmTypeEffects"))
    {
        _log(DATABASE__ERROR, "Error in GetDogmaTypeEffects: %s",res.error.c_str());
        return nullptr;
    }
    return DBResultToCRowset(res);
}

PyRep* BulkDB::GetDogmaTypeAttribs(uint8 chunkID)   // 37 chunks
{   //372904
    DBQueryResult res;
    std::ostringstream q;
    switch (chunkID) {
        case 0: {
            q << " LIMIT 0, 10000";
        } break;
        case 1: {
            q << " LIMIT 10000, 10000";
        } break;
        case 2: {
            q << " LIMIT 20000, 10000";
        } break;
        case 3: {
            q << " LIMIT 30000, 10000";
        } break;
        case 4: {
            q << " LIMIT 40000, 10000";
        } break;
        case 5: {
            q << " LIMIT 50000, 10000";
        } break;
        case 6: {
            q << " LIMIT 60000, 10000";
        } break;
        case 7: {
            q << " LIMIT 70000, 10000";
        } break;
        case 8: {
            q << " LIMIT 80000, 10000";
        } break;
        case 9: {
            q << " LIMIT 90000, 10000";
        } break;
        case 10: {
            q << " LIMIT 100000, 10000";
        } break;
        case 11: {
            q << " LIMIT 110000, 10000";
        } break;
        case 12: {
            q << " LIMIT 120000, 10000";
        } break;
        case 13: {
            q << " LIMIT 130000, 10000";
        } break;
        case 14: {
            q << " LIMIT 140000, 10000";
        } break;
        case 15: {
            q << " LIMIT 150000, 10000";
        } break;
        case 16: {
            q << " LIMIT 160000, 10000";
        } break;
        case 17: {
            q << " LIMIT 170000, 10000";
        } break;
        case 18: {
            q << " LIMIT 180000, 10000";
        } break;
        case 19: {
            q << " LIMIT 190000, 10000";
        } break;
        case 20: {
            q << " LIMIT 200000, 10000";
        } break;
        case 21: {
            q << " LIMIT 210000, 10000";
        } break;
        case 22: {
            q << " LIMIT 220000, 10000";
        } break;
        case 23: {
            q << " LIMIT 230000, 10000";
        } break;
        case 24: {
            q << " LIMIT 240000, 10000";
        } break;
        case 25: {
            q << " LIMIT 250000, 10000";
        } break;
        case 26: {
            q << " LIMIT 260000, 10000";
        } break;
        case 27: {
            q << " LIMIT 270000, 10000";
        } break;
        case 28: {
            q << " LIMIT 280000, 10000";
        } break;
        case 29: {
            q << " LIMIT 290000, 10000";
        } break;
        case 30: {
            q << " LIMIT 300000, 10000";
        } break;
        case 31: {
            q << " LIMIT 310000, 10000";
        } break;
        case 32: {
            q << " LIMIT 320000, 10000";
        } break;
        case 33: {
            q << " LIMIT 330000, 10000";
        } break;
        case 34: {
            q << " LIMIT 340000, 10000";
        } break;
        case 35: {
            q << " LIMIT 350000, 10000";
        } break;
        case 36: {
            q << " LIMIT 360000, 10000";
        } break;
        case 37: {
            q << " LIMIT 370000, 10000";
        } break;
    };

    if(!sDatabase.RunQuery(res, "SELECT typeID, attributeID, IF(valueInt IS NULL, valueFloat, valueInt) AS value FROM dgmTypeAttributes"))
    {
        _log(DATABASE__ERROR, "Error in GetDogmaTypeAttribs: %s",res.error.c_str());
        return nullptr;
    }
    return DBResultToCRowset(res);
}
