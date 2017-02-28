/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2017 The EVEmu Team
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
    Author: Allan
*/

#ifndef _EVE_CACHE_BULKDB_H_
#define _EVE_CACHE_BULKDB_H_


#include "../eve-server.h"


class BulkDB
{
public:
    BulkDB();

    uint8 GetNumChunks(uint8 setID=0);
    int32 GetFileIDfromChunk(uint8 setID, uint8 chunkID);

    /* updated dogma files to send to client in bulkData */
    PyRep* GetOperands();
    PyRep* GetDogmaAttribs();
    PyRep* GetDogmaEffects();

    /* these need to be split into ~8k-row chunks for easier handling */
    PyRep* GetExpressions(uint8 chunkID);
    PyRep* GetDogmaTypeEffects(uint8 chunkID);
    PyRep* GetDogmaTypeAttribs(uint8 chunkID);
    /* this is used to determine what to get */
    PyRep* GetBulkDataChunks(uint8 setID, uint8 chunkID);

private:
    uint8 m_chunks;

};

#endif  // _EVE_CACHE_BULKDB_H_

/*  notes to keep track of chunkID and the data it refers to
 *
 *
 *
 *
 *
 */