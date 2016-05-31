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
    Updates:        Allan
*/


#ifndef __STANDINGDB_H_INCL__
#define __STANDINGDB_H_INCL__

#include "ServiceDB.h"
#include "packets/Standing.h"

class PyRep;

class StandingDB
: public ServiceDB
{
public:
    PyRep* GetFactionStandings();
    PyRep* GetCorpStandings(uint32 corpID);
    PyRep* GetCharStandings(Client* pClient);
    PyRep* GetSystemSovInfo(uint32 systemID);
    PyRep* PrimeCharStandings(uint32 charID);
    PyRep* GetCharNPCStandings(uint32 charID);
    PyRep* GetStandingTransactions(uint32 fromID, uint32 toID, uint32 direction, uint32 eventID=0, uint32 eventType=0, uint64 eventDateTime=0);
    PyRep* GetStandingCompositions(uint32 toID, uint32 fromID);

    double GetStandingChanges(uint32 charID);
    double GetAgentStanding(uint32 toID, uint32 fromID);     // from agents to characters. changed by missions status'
    double GetAllianceStanding(uint32 toID, uint32 fromID);  // corporation<-->alliance, alliance<-->alliance - changed thru Corp window
    double GetCharStanding(uint32 toID, uint32 fromID);      // character<-->character, character<-->corporation - changed thru PnP window
    double GetCorpStanding(uint32 toID, uint32 fromID);      // corporation<-->character, corporation<-->corporation - changed thru Corp window
    double GetFactionStanding(uint32 toID, uint32 fromID);   // NPC Faction <--> NPC Faction - pre-set by game history
    double GetNPCCorpStanding(uint32 toID, uint32 fromID);   // NPC corps --> characters - changed by missions and faction kills

    void SetAgentStanding(uint32 toID, uint32 fromID, double standing);
    void SetAllianceStanding(uint32 toID, uint32 fromID, double standing);
    void SetCharStanding(uint32 toID, uint32 fromID, double standing);
    void SetCorpStanding(uint32 toID, uint32 fromID, double standing);
    void SetNPCCorpStanding(uint32 toID, uint32 fromID, double standing);
    void SaveStandingChanges(uint32 fromID, uint32 toID, uint32 eventType, double amount, std::string msg);

};

#endif
