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
    Author:     Zhur, Bloody.Rabbit
    Updates:    Allan
*/


#include "EVEServerConfig.h"

/*************************************************************************/
/* EVEServerConfig                                                       */
/*************************************************************************/
EVEServerConfig::EVEServerConfig()
{
    // register needed parsers
    AddMemberParser( "eve-server", &EVEServerConfig::ProcessEveServer );

    // Set sane defaults
    // items with a "N" behind them are NOT implemented
    // items with a "P" behind them are PARTIALLY implemented

    // items with /*x*/ behind them denote time idetifier, with x = (s=seconds, m=minutes, etc)

    // world
    world.testServer = true;
    world.chatLogs = false;//N
    world.globalChat = true;//N
    world.gridUnload = true;
    world.gridUnloadTime = 300 /*s*/; // 5 mins
    world.loginInfo = false;//N
    world.loginMsg = false;//N
    world.mailDelay = 5;//N
    world.maxPlayers = 500;//N
    world.idleSleepTime = 1000;

    // rates
    rates.npcBountyMultiply = 1.0;
    rates.secRate = 1.0;
    rates.damageRate = 1.0;
    rates.missileRate = 1.0;
    rates.missileTime = 1.0;
    rates.turrentRate = 1.0;
    rates.corpCost = 1599800;
    rates.WorldDecay = 120 /*m*/;//P   2 hours
    rates.NPCDecay = 90 /*m*/; //P 90 mins
    rates.RateDropItem = 1.0;//N
    rates.RateDropMoney = 1.0;//N
    rates.RepairCost = 1.0;//N
    rates.WebUpdate = 15 /*m*/;

    // account
    account.autoAccountRole = ROLE_STD;
    account.loginMessage = "";

    // character
    character.startBalance = 6666000000.0f;
    character.startAurBalance = 60000.0f;
    character.startStation = 0;
    character.startSecRating = 0.0;
    character.startCorporation = 0;
    character.terminationDelay = 180 /*s*/;

    // npc
    npc.ThreatRadius = 1.0;//N
    npc.RoamingSpawns = false;//P
    npc.StaticSpawns = false;//N
    npc.RoamingTimer = 15 /*m*/;//P
    npc.StaticTimer = 10 /*m*/;//P

    // database
    database.host = "localhost";
    database.port = 3306;
    database.username = "eve";
    database.password = "eve";
    database.db = "evemu";

    // files
    files.logDir = "../log/";
    files.logSettings = "../etc/log.ini";
    files.cacheDir = "../server_cache/";
    files.imageDir = "../image_cache/";

    // net
    net.port = 26000;
    net.imageServer = "localhost";
    net.imageServerPort = 26001;
    net.apiServer = "localhost";
    net.apiServerPort = 26002;

    // threads  -not implemented
    threads.APIThreads = 1;//N
    threads.ConsoleThreads = 1;//P
    threads.DatabaseThreads = 2;//N
    threads.ImageServerThreads = 1;//N
    threads.NetworkThreads = 2;//N
    threads.WorldThreads = 2;//N

    // misc
    misc.UseProfiling = false;
    misc.UseAPIServer = false;//N
    misc.UseShipTracking = false;
    misc.UseStackTrace = false;//N
    misc.ServerSleepTime = 10 /*ms*/;

    // crime
    crime.AggFlagTime = 900 /*s*/;//N
    crime.CrimFlagTime = 900 /*s*/;//N
    crime.CWSessionTime = 60 /*s*/;//N
    crime.KillRightTime = 900 /*s*/;//N
    crime.WeaponFlagTime = 60 /*s*/;//N
}

bool EVEServerConfig::ProcessEveServer( const TiXmlElement* ele )
{
    // entering element, extend allowed syntax
    AddMemberParser( "world",       &EVEServerConfig::ProcessWorld );
    AddMemberParser( "rates",       &EVEServerConfig::ProcessRates );
    AddMemberParser( "account",     &EVEServerConfig::ProcessAccount );
    AddMemberParser( "character",   &EVEServerConfig::ProcessCharacter );
    AddMemberParser( "npc",         &EVEServerConfig::ProcessNPC );
    AddMemberParser( "database",    &EVEServerConfig::ProcessDatabase );
    AddMemberParser( "files",       &EVEServerConfig::ProcessFiles );
    AddMemberParser( "net",         &EVEServerConfig::ProcessNet );
    AddMemberParser( "threads",     &EVEServerConfig::ProcessThreads );
    AddMemberParser( "misc",        &EVEServerConfig::ProcessMisc );
    AddMemberParser( "crime",       &EVEServerConfig::ProcessCrime );

    // parse the element
    const bool result = ParseElementChildren( ele );

    // leaving element, reduce allowed syntax
    RemoveParser( "world" );
    RemoveParser( "rates" );
    RemoveParser( "account" );
    RemoveParser( "character" );
    RemoveParser( "npc" );
    RemoveParser( "database" );
    RemoveParser( "files" );
    RemoveParser( "net" );
    RemoveParser( "threads" );
    RemoveParser( "misc" );
    RemoveParser( "crime" );

    // return status of parsing
    return result;
}

bool EVEServerConfig::ProcessWorld( const TiXmlElement* ele )
{
    AddValueParser( "testServer",       world.testServer );
    AddValueParser( "chatLogs",         world.chatLogs );
    AddValueParser( "globalChat",       world.globalChat );
    AddValueParser( "gridUnload",       world.gridUnload );
    AddValueParser( "gridUnloadTime",   world.gridUnloadTime );
    AddValueParser( "loginInfo",        world.loginInfo );
    AddValueParser( "loginMsg",         world.loginMsg );
    AddValueParser( "mailDelay",        world.mailDelay );
    AddValueParser( "maxPlayers",       world.maxPlayers );
    AddValueParser( "idleSleepTime",    world.idleSleepTime );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "testServer" );
    RemoveParser( "chatLogs" );
    RemoveParser( "globalChat" );
    RemoveParser( "gridUnload" );
    RemoveParser( "gridUnloadTime" );
    RemoveParser( "loginInfo" );
    RemoveParser( "loginMsg" );
    RemoveParser( "mailDelay" );
    RemoveParser( "maxPlayers" );
    RemoveParser( "idleSleepTime" );

    return result;
}

bool EVEServerConfig::ProcessRates( const TiXmlElement* ele )
{
    AddValueParser( "secRate",              rates.secRate );
    AddValueParser( "npcBountyMultiply",    rates.npcBountyMultiply );
    AddValueParser( "damageRate",           rates.damageRate );
    AddValueParser( "missileRate",          rates.missileRate );
    AddValueParser( "missileTime",          rates.missileTime );
    AddValueParser( "turrentRate",          rates.turrentRate );
    AddValueParser( "corpCost",             rates.corpCost );
    AddValueParser( "WorldDecay",           rates.WorldDecay );
    AddValueParser( "NPCDecay",             rates.NPCDecay );
    AddValueParser( "RateDropItem",         rates.RateDropItem );
    AddValueParser( "RateDropMoney",        rates.RateDropMoney );
    AddValueParser( "RepairCost",           rates.RepairCost );
    AddValueParser( "WebUpdate",            rates.WebUpdate );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "secRate" );
    RemoveParser( "npcBountyMultiply" );
    RemoveParser( "damageRate" );
    RemoveParser( "missileRate" );
    RemoveParser( "missileTime" );
    RemoveParser( "turrentRate" );
    RemoveParser( "corpCost" );
    RemoveParser( "WorldDecay" );
    RemoveParser( "NPCDecay" );
    RemoveParser( "RateDropItem" );
    RemoveParser( "RateDropMoney" );
    RemoveParser( "RepairCost" );
    RemoveParser( "WebUpdate" );

    return result;
}

bool EVEServerConfig::ProcessAccount( const TiXmlElement* ele )
{
    AddValueParser( "autoAccountRole", account.autoAccountRole );
    AddValueParser( "loginMessage",    account.loginMessage );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "autoAccountRole" );
    RemoveParser( "loginMessage" );

    return result;
}

bool EVEServerConfig::ProcessCharacter( const TiXmlElement* ele )
{
    AddValueParser( "startBalance",     character.startBalance );
    AddValueParser( "startAurBalance",  character.startAurBalance );  // added config entry and implemented  -allan 01/10/14
    AddValueParser( "startStation",     character.startStation );
    AddValueParser( "startSecRating",   character.startSecRating );
    AddValueParser( "startCorporation", character.startCorporation );
    AddValueParser( "terminationDelay", character.terminationDelay );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "startBalance" );
    RemoveParser( "startAurBalance" );
    RemoveParser( "startStation" );
    RemoveParser( "startSecRating" );
    RemoveParser( "startCorporation" );
    RemoveParser( "terminationDelay" );

    return result;
}

bool EVEServerConfig::ProcessNPC( const TiXmlElement* ele )
{
    AddValueParser( "ThreatRadius",  npc.ThreatRadius );
    AddValueParser( "RoamingSpawns", npc.RoamingSpawns );
    AddValueParser( "StaticSpawns",  npc.StaticSpawns );
    AddValueParser( "RoamingTimer",  npc.RoamingTimer );
    AddValueParser( "StaticTimer",   npc.StaticTimer );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "ThreatRadius" );
    RemoveParser( "RoamingSpawns" );
    RemoveParser( "StaticSpawns" );
    RemoveParser( "RoamingTimer" );
    RemoveParser( "StaticTimer" );

    return result;
}

bool EVEServerConfig::ProcessDatabase( const TiXmlElement* ele )
{
    AddValueParser( "host",     database.host );
    AddValueParser( "port",     database.port );
    AddValueParser( "username", database.username );
    AddValueParser( "password", database.password );
    AddValueParser( "db",       database.db );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "host" );
    RemoveParser( "port" );
    RemoveParser( "username" );
    RemoveParser( "password" );
    RemoveParser( "db" );

    return result;
}

bool EVEServerConfig::ProcessFiles( const TiXmlElement* ele )
{
    AddValueParser( "logDir",      files.logDir );
    AddValueParser( "logSettings", files.logSettings );
    AddValueParser( "cacheDir",    files.cacheDir );
    AddValueParser( "imageDir",    files.imageDir );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "logDir" );
    RemoveParser( "logSettings" );
    RemoveParser( "cacheDir" );
    RemoveParser( "imageDir" );

    return result;
}

bool EVEServerConfig::ProcessNet( const TiXmlElement* ele )
{
    AddValueParser( "port",             net.port );
    AddValueParser( "imageServerPort",  net.imageServerPort);
    AddValueParser( "imageServer",      net.imageServer);
    AddValueParser( "apiServerPort",    net.apiServerPort);
    AddValueParser( "apiServer",        net.apiServer);

    const bool result = ParseElementChildren( ele );

    RemoveParser( "port" );
    RemoveParser( "imageServerPort" );
    RemoveParser( "imageServer" );
    RemoveParser( "apiServerPort" );
    RemoveParser( "apiServer" );

    return result;
}

bool EVEServerConfig::ProcessThreads( const TiXmlElement* ele )
{
    AddValueParser( "APIThreads",           threads.APIThreads);
    AddValueParser( "ConsoleThreads",       threads.ConsoleThreads);
    AddValueParser( "DatabaseThreads",      threads.DatabaseThreads);
    AddValueParser( "ImageServerThreads",   threads.ImageServerThreads);
    AddValueParser( "NetworkThreads",       threads.NetworkThreads );
    AddValueParser( "WorldThreads",         threads.WorldThreads);

    const bool result = ParseElementChildren( ele );

    RemoveParser( "APIThreads" );
    RemoveParser( "ConsoleThreads" );
    RemoveParser( "DatabaseThreads" );
    RemoveParser( "ImageServerThreads" );
    RemoveParser( "NetworkThreads" );
    RemoveParser( "WorldThreads" );

    return result;
}

bool EVEServerConfig::ProcessMisc( const TiXmlElement* ele )
{
    AddValueParser( "UseProfiling",     misc.UseProfiling );
    AddValueParser( "UseAPIServer",     misc.UseAPIServer );
    AddValueParser( "UseShipTracking",  misc.UseShipTracking );
    AddValueParser( "UseStackTrace",  misc.UseStackTrace );
    AddValueParser( "ServerSleepTime",  misc.ServerSleepTime );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "UseProfiling" );
    RemoveParser( "UseAPIServer" );
    RemoveParser( "UseShipTracking" );
    RemoveParser( "UseStackTrace" );
    RemoveParser( "ServerSleepTime" );

    return result;
}

bool EVEServerConfig::ProcessCrime( const TiXmlElement* ele )
{
    AddValueParser( "AggFlagTime",     crime.AggFlagTime );
    AddValueParser( "CrimFlagTime",    crime.CrimFlagTime );
    AddValueParser( "CWSessionTime",   crime.CWSessionTime );
    AddValueParser( "KillRightTime",   crime.KillRightTime );
    AddValueParser( "WeaponFlagTime",  crime.WeaponFlagTime );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "AggFlagTime" );
    RemoveParser( "CrimFlagTime" );
    RemoveParser( "CWSessionTime" );
    RemoveParser( "KillRightTime" );
    RemoveParser( "WeaponFlagTime" );

    return result;
}
