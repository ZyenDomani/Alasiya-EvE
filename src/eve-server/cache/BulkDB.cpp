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
    cacheDogmaAttributes = 800004,              -1788
    cacheDogmaEffects = 800005,                 -3537
    cacheDogmaExpressionCategories = 800001,    -27
    cacheDogmaExpressions = 800003,             -17757
    cacheDogmaOperands = 800002,                -74
    cacheDogmaTypeAttributes = 800006,          -372904
    cacheDogmaTypeEffects = 800007,             -36340

    will have to split these up in chunks, hard-code chunk numbers, and keep track of the data and its relevant number
    not sure how im gonna do this yet.

    */