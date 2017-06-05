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
    Version:    7.7
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

    // server
    server.UseBeanCount = false;
    server.UseMarketBot = false;
    server.IsTestServer = true;
    server.maxPlayers = 500;//N
    server.UseProfiling = false;
    server.PositionHack = false;
    server.UseShipTracking = false;
    server.DeleteTrackingCans = true;
    server.UseStackTrace = false;//N
    server.BulkDataOD = false;
    server.ServerSleepTime = 10 /*ms*/;
    server.idleSleepTime = 1000;
    server.DisableIGB = true;
    server.MaxThreadReport = 20;

    // world
    world.chatLogs = false;//N
    world.globalChat = true;//N
    world.gridUnload = true;
    world.gridUnloadTime = 300 /*s*/; // 5 mins
    world.loginInfo = false;//N
    world.loginMsg = false;//N
    world.mailDelay = 5;//N
    world.StationDockDelay = 4 /*s*/;

    // rates
    rates.npcBountyMultiply = 1.0;
    rates.secRate = 1.0;
    rates.damageRate = 1.0;
    rates.missileRate = 1.0;
    rates.missileTime = 1.0;
    rates.turretRate = 1.0;
    rates.corpCost = 1599800;
    rates.WorldDecay = 120 /*m*/;//P   2 hours
    rates.NPCDecay = 90 /*m*/; //P 90 mins
    rates.RateDropItem = 1.0;//N
    rates.RateDropMoney = 1.0;//N
    rates.RepairCost = 1.0;//N
    rates.WebUpdate = 15 /*m*/;

    // bpTimes
    bpTimes.ProdTime = 1.0;
    bpTimes.MatMod = 1.0;
    bpTimes.ProdMod = 1.0;
    bpTimes.WasteMod = 1.0;
    bpTimes.ResCopy = 1.0;
    bpTimes.ResME = 1.0;
    bpTimes.ResPE = 1.0;
    bpTimes.ResRE = 1.0;

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
    character.statMultiplier = 1;

    // npc
    npc.IdleWander = false;//P
    npc.ThreatRadius = 1.0;//N
    npc.RoamingSpawns = false;//P
    npc.StaticSpawns = false;//N
    npc.RoamingTimer = 15 /*m*/;
    npc.StaticTimer = 10 /*m*/;//P
    npc.RatFaction = 0;
    npc.SpawnTest = false;
    npc.EnableDrones = false;

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

    // threads  -not implemented
    threads.ConsoleThreads = 1;//P
    threads.DatabaseThreads = 2;//N
    threads.ImageServerThreads = 1;//N
    threads.NetworkThreads = 2;//N
    threads.WorldThreads = 2;//N

    // cosmic
    cosmic.EnablePI = false;
    cosmic.AnomalyEnabled = false;
    cosmic.DungeonEnabled = false;
    cosmic.BeltEnabled = false;
    cosmic.BeltRespawn = 8 /*h*/;
    cosmic.BeltGrowth = 6 /*h*/;
    cosmic.roidRadiusMultiplier = 1.0;
    cosmic.WormHoleEnabled = false;
    cosmic.CiviliansEnabled = false;
    cosmic.BumpEnabled = false;

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
    AddMemberParser( "server",      &EVEServerConfig::ProcessServer );
    AddMemberParser( "world",       &EVEServerConfig::ProcessWorld );
    AddMemberParser( "rates",       &EVEServerConfig::ProcessRates );
    AddMemberParser( "bpTimes",     &EVEServerConfig::ProcessBPTimes );
    AddMemberParser( "account",     &EVEServerConfig::ProcessAccount );
    AddMemberParser( "character",   &EVEServerConfig::ProcessCharacter );
    AddMemberParser( "npc",         &EVEServerConfig::ProcessNPC );
    AddMemberParser( "database",    &EVEServerConfig::ProcessDatabase );
    AddMemberParser( "files",       &EVEServerConfig::ProcessFiles );
    AddMemberParser( "net",         &EVEServerConfig::ProcessNet );
    AddMemberParser( "threads",     &EVEServerConfig::ProcessThreads );
    AddMemberParser( "cosmic",      &EVEServerConfig::ProcessCosmic );
    AddMemberParser( "crime",       &EVEServerConfig::ProcessCrime );

    // parse the element
    const bool result = ParseElementChildren( ele );

    // leaving element, reduce allowed syntax
    RemoveParser( "server" );
    RemoveParser( "world" );
    RemoveParser( "rates" );
    RemoveParser( "bpTimes" );
    RemoveParser( "account" );
    RemoveParser( "character" );
    RemoveParser( "npc" );
    RemoveParser( "database" );
    RemoveParser( "files" );
    RemoveParser( "net" );
    RemoveParser( "threads" );
    RemoveParser( "cosmic" );
    RemoveParser( "crime" );

    // return status of parsing
    return result;
}

bool EVEServerConfig::ProcessServer( const TiXmlElement* ele )
{
    AddValueParser( "DisableIGB",           server.DisableIGB );
    AddValueParser( "IsTestServer",         server.IsTestServer );
    AddValueParser( "UseBeanCount",         server.UseBeanCount );
    AddValueParser( "UseMarketBot",         server.UseMarketBot );
    AddValueParser( "maxPlayers",           server.maxPlayers );
    AddValueParser( "UseProfiling",         server.UseProfiling );
    AddValueParser( "UseShipTracking",      server.UseShipTracking );
    AddValueParser( "PositionHack",         server.PositionHack );
    AddValueParser( "DeleteTrackingCans",   server.DeleteTrackingCans );
    AddValueParser( "UseStackTrace",        server.UseStackTrace );
    AddValueParser( "BulkDataOD",           server.BulkDataOD );
    AddValueParser( "ServerSleepTime",      server.ServerSleepTime );
    AddValueParser( "idleSleepTime",        server.idleSleepTime );
    AddValueParser( "MaxThreadReport",      server.MaxThreadReport );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "DisableIGB" );
    RemoveParser( "IsTestServer" );
    RemoveParser( "UseBeanCount" );
    RemoveParser( "maxPlayers" );
    RemoveParser( "UseProfiling" );
    RemoveParser( "UseShipTracking" );
    RemoveParser( "PositionHack" );
    RemoveParser( "DeleteTrackingCans" );
    RemoveParser( "UseStackTrace" );
    RemoveParser( "BulkDataOD" );
    RemoveParser( "ServerSleepTime" );
    RemoveParser( "idleSleepTime" );
    RemoveParser( "MaxThreadReport" );

    return result;
}

bool EVEServerConfig::ProcessWorld( const TiXmlElement* ele )
{
    AddValueParser( "chatLogs",         world.chatLogs );
    AddValueParser( "globalChat",       world.globalChat );
    AddValueParser( "gridUnload",       world.gridUnload );
    AddValueParser( "gridUnloadTime",   world.gridUnloadTime );
    AddValueParser( "loginInfo",        world.loginInfo );
    AddValueParser( "loginMsg",         world.loginMsg );
    AddValueParser( "mailDelay",        world.mailDelay );
    AddValueParser( "StationDockDelay", world.StationDockDelay );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "chatLogs" );
    RemoveParser( "globalChat" );
    RemoveParser( "gridUnload" );
    RemoveParser( "gridUnloadTime" );
    RemoveParser( "loginInfo" );
    RemoveParser( "loginMsg" );
    RemoveParser( "mailDelay" );
    RemoveParser( "StationDockDelay" );

    return result;
}

bool EVEServerConfig::ProcessRates( const TiXmlElement* ele )
{
    AddValueParser( "secRate",          rates.secRate );
    AddValueParser( "npcBountyMultiply",rates.npcBountyMultiply );
    AddValueParser( "damageRate",       rates.damageRate );
    AddValueParser( "missileRate",      rates.missileRate );
    AddValueParser( "missileTime",      rates.missileTime );
    AddValueParser( "turretRate",       rates.turretRate );
    AddValueParser( "corpCost",         rates.corpCost );
    AddValueParser( "WorldDecay",       rates.WorldDecay );
    AddValueParser( "NPCDecay",         rates.NPCDecay );
    AddValueParser( "RateDropItem",     rates.RateDropItem );
    AddValueParser( "RateDropMoney",    rates.RateDropMoney );
    AddValueParser( "RepairCost",       rates.RepairCost );
    AddValueParser( "WebUpdate",        rates.WebUpdate );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "secRate" );
    RemoveParser( "npcBountyMultiply" );
    RemoveParser( "damageRate" );
    RemoveParser( "missileRate" );
    RemoveParser( "missileTime" );
    RemoveParser( "turretRate" );
    RemoveParser( "corpCost" );
    RemoveParser( "WorldDecay" );
    RemoveParser( "NPCDecay" );
    RemoveParser( "RateDropItem" );
    RemoveParser( "RateDropMoney" );
    RemoveParser( "RepairCost" );
    RemoveParser( "WebUpdate" );

    return result;
}

bool EVEServerConfig::ProcessBPTimes(const TiXmlElement* ele)
{
    AddValueParser( "ProdTime",         bpTimes.ProdTime);
    AddValueParser( "ResCopy",          bpTimes.ResCopy);
    AddValueParser( "ProdMod",          bpTimes.ProdMod);
    AddValueParser( "WasteMod",         bpTimes.WasteMod);
    AddValueParser( "MatMod",           bpTimes.MatMod);
    AddValueParser( "ResME",            bpTimes.ResME);
    AddValueParser( "ResPE",            bpTimes.ResPE);
    AddValueParser( "ResRE",            bpTimes.ResRE);

    const bool result = ParseElementChildren( ele );

    RemoveParser( "ProdTime" );
    RemoveParser( "ResCopy" );
    RemoveParser( "ProdMod" );
    RemoveParser( "WasteMod" );
    RemoveParser( "MatMod" );
    RemoveParser( "ResME" );
    RemoveParser( "ResPE" );
    RemoveParser( "ResRE" );

    return result;
}

bool EVEServerConfig::ProcessAccount( const TiXmlElement* ele )
{
    AddValueParser( "autoAccountRole",  account.autoAccountRole );
    AddValueParser( "loginMessage",     account.loginMessage );

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
    AddValueParser( "statMultiplier",   character.statMultiplier );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "startBalance" );
    RemoveParser( "startAurBalance" );
    RemoveParser( "startStation" );
    RemoveParser( "startSecRating" );
    RemoveParser( "startCorporation" );
    RemoveParser( "terminationDelay" );
    RemoveParser( "statMultiplier" );

    return result;
}

bool EVEServerConfig::ProcessNPC( const TiXmlElement* ele )
{
    AddValueParser( "IdleWander",       npc.IdleWander );
    AddValueParser( "ThreatRadius",     npc.ThreatRadius );
    AddValueParser( "RoamingSpawns",    npc.RoamingSpawns );
    AddValueParser( "StaticSpawns",     npc.StaticSpawns );
    AddValueParser( "RoamingTimer",     npc.RoamingTimer );
    AddValueParser( "StaticTimer",      npc.StaticTimer );
    AddValueParser( "RatFaction",       npc.RatFaction );
    AddValueParser( "SpawnTest",        npc.SpawnTest );
    AddValueParser( "EnableDrones",     npc.EnableDrones );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "IdleWander" );
    RemoveParser( "ThreatRadius" );
    RemoveParser( "RoamingSpawns" );
    RemoveParser( "StaticSpawns" );
    RemoveParser( "RoamingTimer" );
    RemoveParser( "StaticTimer" );
    RemoveParser( "RatFaction" );
    RemoveParser( "SpawnTest" );
    RemoveParser( "EnableDrones" );

    return result;
}

bool EVEServerConfig::ProcessDatabase( const TiXmlElement* ele )
{
    AddValueParser( "host",             database.host );
    AddValueParser( "port",             database.port );
    AddValueParser( "username",         database.username );
    AddValueParser( "password",         database.password );
    AddValueParser( "db",               database.db );

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
    AddValueParser( "logDir",           files.logDir );
    AddValueParser( "logSettings",      files.logSettings );
    AddValueParser( "cacheDir",         files.cacheDir );
    AddValueParser( "imageDir",         files.imageDir );

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

    const bool result = ParseElementChildren( ele );

    RemoveParser( "port" );
    RemoveParser( "imageServerPort" );
    RemoveParser( "imageServer" );

    return result;
}

bool EVEServerConfig::ProcessThreads( const TiXmlElement* ele )
{
    AddValueParser( "ConsoleThreads",       threads.ConsoleThreads);
    AddValueParser( "DatabaseThreads",      threads.DatabaseThreads);
    AddValueParser( "ImageServerThreads",   threads.ImageServerThreads);
    AddValueParser( "NetworkThreads",       threads.NetworkThreads );
    AddValueParser( "WorldThreads",         threads.WorldThreads);

    const bool result = ParseElementChildren( ele );

    RemoveParser( "ConsoleThreads" );
    RemoveParser( "DatabaseThreads" );
    RemoveParser( "ImageServerThreads" );
    RemoveParser( "NetworkThreads" );
    RemoveParser( "WorldThreads" );

    return result;
}

bool EVEServerConfig::ProcessCosmic( const TiXmlElement* ele )
{
    AddValueParser( "EnablePI",             cosmic.EnablePI );
    AddValueParser( "AnomalyEnabled",       cosmic.AnomalyEnabled );
    AddValueParser( "DungeonEnabled",       cosmic.DungeonEnabled );
    AddValueParser( "BeltEnabled",          cosmic.BeltEnabled );
    AddValueParser( "BeltRespawn",          cosmic.BeltRespawn );
    AddValueParser( "BeltGrowth",           cosmic.BeltGrowth );
    AddValueParser( "roidRadiusMultiplier", cosmic.roidRadiusMultiplier );
    AddValueParser( "WormHoleEnabled",      cosmic.WormHoleEnabled );
    AddValueParser( "CiviliansEnabled",     cosmic.CiviliansEnabled);

    const bool result = ParseElementChildren( ele );

    RemoveParser( "EnablePI" );
    RemoveParser( "AnomalyEnabled" );
    RemoveParser( "DungeonEnabled" );
    RemoveParser( "BeltEnabled" );
    RemoveParser( "BeltRespawn" );
    RemoveParser( "BeltGrowth" );
    RemoveParser( "roidRadiusMultiplier" );
    RemoveParser( "WormHoleEnabled" );
    RemoveParser( "CiviliansEnabled" );

    return result;
}

bool EVEServerConfig::ProcessCrime( const TiXmlElement* ele )
{
    AddValueParser( "AggFlagTime",      crime.AggFlagTime );
    AddValueParser( "CrimFlagTime",     crime.CrimFlagTime );
    AddValueParser( "CWSessionTime",    crime.CWSessionTime );
    AddValueParser( "KillRightTime",    crime.KillRightTime );
    AddValueParser( "WeaponFlagTime",   crime.WeaponFlagTime );

    const bool result = ParseElementChildren( ele );

    RemoveParser( "AggFlagTime" );
    RemoveParser( "CrimFlagTime" );
    RemoveParser( "CWSessionTime" );
    RemoveParser( "KillRightTime" );
    RemoveParser( "WeaponFlagTime" );

    return result;
}
