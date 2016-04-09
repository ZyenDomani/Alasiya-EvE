/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2008 The EVEmu Team
    For the latest information visit http://evemu.mmoforge.org
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
    Author:     Zhur, Bloody.Rabit
    Updates:    Allan
*/

#ifndef __EVE_SERVER_CONFIG__H__INCL__
#define __EVE_SERVER_CONFIG__H__INCL__

#include "eve-server.h"


/**
 * @brief Class which parses and stores eve-server configuration.
 *
 * @author Zhur, Bloody.Rabbit
 */
class EVEServerConfig
: public XMLParserEx,
  public Singleton< EVEServerConfig >
{
public:
    /**
     * @brief Primary constructor; initializes the object with default values.
     */
    EVEServerConfig();
    ~EVEServerConfig()          { /* do nothing here */}

    // From <world/>
    struct
    {
        bool testServer;    // to distuinguish between live production server or experimental testing server
        bool chatLogs;
        bool globalChat;
        bool gridUnload;
        uint gridUnloadTime;
        bool loginInfo;
        bool loginMsg;
        uint8 mailDelay;
        uint16 maxPlayers;
        uint16 idleSleepTime;
    } world;

    // From <rates/>
    struct
    {
        /// Modifier for security rating changes. Changes how fast it goes up/down based on actions
        double secRate;
        /// Modifier for npc bounties automatically awarded for shooting down npc enemies.
        float npcBountyMultiply;
        /// Modifier for damage from NPCs
        float damageRate;
        /// Modifier for damage from missiles
        float missileRate;
        /// Modifier for missile flightTime
        float missileTime;
        /// Modifier for damage from PC turrents
        float turrentRate;
        /// Startup Cost to create a corporation.
        double corpCost;
        // Decay timer for item deletion (garbage collection)
        uint8 WorldDecay;
        // Decay timer for wreck deletion (garbage collection)
        float NPCDecay;

        float RateDropItem;
        float RateDropMoney;
        float RepairCost;

        uint8 WebUpdate;
    } rates;

    // From <account/>
    struct
    {
        /// Role to assign to auto created account; set to 0 to disable auto account creation.
        uint64 autoAccountRole;
        /// A message shown to every client on login (if enabled in <World><LoginMsg>).
        std::string loginMessage;
    } account;

    // From <character/>
    struct
    {
        /// Money balance of new created characters.
        double startBalance;
        /// Aura balance of new created characters.   -allan 01/10/14
        double startAurBalance;
        /// Starting station ID for new characters
        uint32 startStation;
        /// Starting security rating for new characters.
        double startSecRating;
        /// Starting corp ID for new characters
        uint32 startCorporation;
        /// Delay for terminating a character in seconds
        uint32 terminationDelay;
    } character;

    // From <NPC/>
    struct
    {
        float ThreatRadius;
        bool RoamingSpawns;
        bool StaticSpawns;
        uint8 RoamingTimer;
        uint8 StaticTimer;
    } npc;

    // From <database/>
    struct
    {
        /// Hostname of database server.
        std::string host;
        /// A port at which the database server listens.
        uint16 port;
        /// Name of database account to use.
        std::string username;
        /// Password for the database account.
        std::string password;
        /// A database to be used by server.
        std::string db;
    } database;

    // From <files/>
    struct
    {
        /// A directory in which the log files are stored
        std::string logDir;
        /// A log configuration file.
        std::string logSettings;
        /// A directory at which the cache files should be stored.
        std::string cacheDir;
        // used as the base directory for the image server
        std::string imageDir;
    } files;

    // From <net/>
    struct
    {
        /// Port at which the server should listen.
        uint16 port;
        /// Port at which the imageServer should listen.
        uint16 imageServerPort;
        /// the imageServer for char images. should be the evemu server external ip/host
        std::string imageServer;
        /// Port at which the apiServer should listen.
        uint16 apiServerPort;
        /// the apiServer for API functions. should be the evemu server external ip/host
        std::string apiServer;
    } net;

    // From <thread/>
    struct
    {
        uint8 NetworkThreads;
        uint8 DatabaseThreads;
        uint8 WorldThreads;
        uint8 APIThreads;
        uint8 ImageServerThreads;
        uint8 ConsoleThreads;
    } threads;

    // From <misc/>
    struct
    {
        bool UseProfiling;
        bool UseAPIServer;
        bool UseShipTracking;
        bool UseStackTrace;
        uint8 ServerSleepTime;
    } misc;

    // From <crime/>
    struct
    {
        uint8 CWSessionTime;
        uint8 WeaponFlagTime;
        uint16 KillRightTime;
        uint16 AggFlagTime;
        uint16 CrimFlagTime;
    } crime;

protected:
    bool ProcessEveServer( const TiXmlElement* ele );
    bool ProcessWorld( const TiXmlElement* ele );
    bool ProcessRates( const TiXmlElement* ele );
    bool ProcessAccount( const TiXmlElement* ele );
    bool ProcessCharacter( const TiXmlElement* ele );
    bool ProcessNPC( const TiXmlElement* ele );
    bool ProcessDatabase( const TiXmlElement* ele );
    bool ProcessFiles( const TiXmlElement* ele );
    bool ProcessNet( const TiXmlElement* ele );
    bool ProcessThreads( const TiXmlElement* ele );
    bool ProcessMisc( const TiXmlElement* ele );
    bool ProcessCrime( const TiXmlElement* ele );
};

/// A macro for easier access to the singleton.
#define sConfig \
    ( EVEServerConfig::get() )

#endif /* !__EVE_SERVER_CONFIG__H__INCL__ */
