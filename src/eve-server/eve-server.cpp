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
    Author:     Zhur, mmcs
    Updates:    Allan
*/

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "NetService.h"
// account services
#include "account/AccountService.h"
#include "account/AuthService.h"
#include "account/BrowserLockdownSvc.h"
#include "account/ClientStatMgrService.h"
#include "account/InfoGatheringMgr.h"
#include "account/TutorialService.h"
#include "account/UserService.h"
// admin services
#include "admin/AlertService.h"
#include "admin/AllCommands.h"
#include "admin/ClientStatLogger.h"
#include "admin/CommandDispatcher.h"
#include "admin/DevToolsProviderService.h"
#include "admin/PetitionerService.h"
#include "admin/SlashService.h"
// agent services
#include "agents/Agent.h"
#include "agents/AgentMgrService.h"
// calendar services
#include "system/Calendar.h"
#include "system/CalendarMgrService.h"
#include "system/CalendarProxy.h"
// cache services
#include "cache/BulkMgrService.h"
#include "cache/ObjCacheService.h"
// character services
#include "character/AggressionMgrService.h"
#include "character/CertificateMgrService.h"
#include "character/CharFittingMgrService.h"
#include "character/CharMgrService.h"
#include "character/CharUnboundMgrService.h"
#include "character/PaperDollService.h"
#include "character/PhotoUploadService.h"
#include "character/SkillMgrService.h"
// chat services
#include "chat/LookupService.h"
#include "chat/LSCService.h"
#include "chat/OnlineStatusService.h"
#include "chat/VoiceMgrService.h"
// config services
#include "config/ConfigService.h"
#include "config/LanguageService.h"
#include "config/LocalizationServerService.h"
// corporation services
#include "corporation/AllianceRegistry.h"
#include "corporation/CorpBookmarkMgrService.h"
#include "corporation/CorpMgrService.h"
#include "corporation/CorporationService.h"
#include "corporation/CorpRegistryService.h"
#include "corporation/CorpStationMgrService.h"
#include "corporation/LPService.h"
#include "corporation/LPStore.h"
// dogmaim services
#include "dogmaim/DogmaIMService.h"
#include "dogmaim/DogmaService.h"
// dungeon services
#include "dungeon/DungeonExplorationMgrService.h"
#include "dungeon/DungeonService.h"
// fleet services
#include "fleet/FleetObject.h"
#include "fleet/FleetProxy.h"
// imageserver services
#include "imageserver/ImageServer.h"
// development index service
#include "system/IndexManager.h"
// inventory services
#include "inventory/InvBrokerService.h"
#include "inventory/Voucher.h"
// mail services
#include "mail/MailMgrService.h"
#include "mail/MailingListMgrService.h"
#include "mail/NotificationMgrService.h"
// manufacturing services
#include "manufacturing/FactoryService.h"
#include "manufacturing/RamProxyService.h"
// map services
#include "map/MapService.h"
// market services
#include "market/BillMgrService.h"
#include "market/ContractMgrService.h"
#include "market/ContractProxy.h"
#include "market/MarketProxyService.h"
// missions services
#include "missions/MissionMgrService.h"
// planet services
#include "planet/Planet.h"
#include "planet/PlanetMgrBound.h"
#include "planet/PlanetORBBound.h"
// pos services
#include "pos/PosMgrService.h"
#include "pos/Structure.h"
// scanning services
#include "scanning/ScanMgrService.h"
// search services
#include "search/Search.h"
// ship services
#include "ship/BeyonceService.h"
#include "ship/ShipService.h"
#include "ship/modules/ModuleEffects.h"
// standing services
#include "standing/FactionWarMgrService.h"
#include "standing/SovereigntyMgrService.h"
#include "standing/Standing.h"
#include "standing/WarRegistryService.h"
// station services
#include "station/HoloscreenMgrService.h"
#include "station/InsuranceService.h"
#include "station/JumpCloneService.h"
#include "station/RepairService.h"
#include "station/ReprocessingService.h"
#include "station/StationService.h"
#include "station/StationSvcService.h"
#include "station/TradeService.h"
// system services
#include "system/BookmarkService.h"
#include "system/BubbleManager.h"
#include "system/KeeperService.h"
#include "system/LootSystem.h"
#include "system/Modifiers.h"
#include "system/ScenarioService.h"
#include "system/WormholeSvc.h"
// cosmic managers
#include "system/cosmicMgrs/DungeonMgr.h"
#include "system/cosmicMgrs/SpawnMgr.h"
#include "system/cosmicMgrs/WormholeMgr.h"
//console commands
#include "ConsoleCommands.h"

static void SetupSignals();
static void CatchSignal( int sig_num );
static const char* const CONFIG_FILE = EVEMU_ROOT "/etc/eve-server.xml";
uint8 MAIN_LOOP_DELAY = sConfig.server.ServerSleepTime; // delay 10 ms.

static volatile bool RunLoops = true;

int main( int argc, char* argv[] )
{
    double profileStartTime = GetTimeMSeconds();

    /* Load server configuration */
    if (!sConfig.ParseFile(CONFIG_FILE)) {
        printf("ERROR: Loading server configuration '%s' failed.", CONFIG_FILE );
        std::cout << std::endl << "press any key to exit...";  std::cin.get();
        sConfig.~EVEServerConfig();
        return EXIT_FAILURE;
    }

    /* set current time for timer */
    Timer::SetCurrentTime();

    /* init logging */
    sLog.InitializeLogging(sConfig.files.logDir);
    sThread.Init();
    sLog.Log( "        Threading", "Starting Main Loop thread with ID 0x%X", pthread_self() );
    //sThread.AddThread(pthread_self());
    sLog.Log("       ServerInit", "Loading server");

    /* Load server log settings */
    if ( load_log_settings( sConfig.files.logSettings.c_str() ) )
        sLog.Success( "       ServerInit", "Log settings loaded from %s", sConfig.files.logSettings.c_str() );
    else
        sLog.Warning( "       ServerInit", "Unable to read %s (this file is optional)", sConfig.files.logSettings.c_str() );
    /* open up the log file if specified */
    if (!sConfig.files.logDir.empty()) {
        std::string logFile = sConfig.files.logDir + "eve-server.log";
        if( log_open_logfile( logFile.c_str() ) )
            sLog.Success( "       ServerInit", "Found log directory %s", sConfig.files.logDir.c_str() );
        else
            sLog.Warning( "       ServerInit", "Unable to find log directory '%s', only logging to the screen now.", sConfig.files.logDir.c_str() );
    }

    sLog.Log("", "");
    sLog.Log(" Supported Client"," %s", EVEProjectVersion);
    sLog.Log("   Client Version"," %.2f", EVEVersionNumber);
    sLog.Log("     Client Build"," %d", EVEBuildVersion);
    sLog.Log("         MachoNet"," %u", MachoNetVersion);
    sLog.Log("     Server Build", " %.2f", EVE_Build );
    sLog.Log("  Server Revision", " %s", EVEMU_REVISION );
    sLog.Log("       Build Date", " %s", EVEMU_BUILD_DATE );
    sLog.Log("   Config Version", " %.1f", Config_Version );
    sLog.Log("      Log Version", " %.1f", Log_Version );
    sLog.Log("", "");

    /* connect to the database */
    DBerror err;
    if ( !sDatabase.Open( err,
        sConfig.database.host.c_str(),
        sConfig.database.username.c_str(),
        sConfig.database.password.c_str(),
        sConfig.database.db.c_str(),
        sConfig.database.port ) )
    {
        sLog.Error( "       ServerInit", "Unable to connect to the database: %s", err.c_str() );
        std::cout << std::endl << "press any key to exit...";  std::cin.get();
        return EXIT_FAILURE;
    }

    /* start dogma type attrib mgr singleton */
    sLog.Success("       ServerInit", "Initializing Dogma Attribute Cache");
    sDgmTypeAttrMgr.Init();

    /* Start up the TCP server */
    EVETCPServer tcps;
    char errbuf[ TCPCONN_ERRBUF_SIZE ];
    if (tcps.Open(sConfig.net.port, errbuf)) {
        sLog.Success( "       ServerInit", "TCP Listener started on port %u.", sConfig.net.port );
    } else {
        sLog.Error( "       ServerInit", "Failed to start TCP listener on port %u: %s.", sConfig.net.port, errbuf );
        std::cout << std::endl << "press any key to exit...";  std::cin.get();
        return EXIT_FAILURE;
    }
    Sleep(250);

    /* create a single item factory and the entity list singleton */
    sLog.Success("       ServerInit", "Starting Item Factory and Entity List");
    ItemFactory* item_factory = new ItemFactory( sEntityList );

    /* create a service manager */
    sLog.Success("       ServerInit", "Starting Service Manager");
    PyServiceMgr services( 888444, sEntityList, item_factory );

    /* initialize clientID seed and start tic timer */
    sEntityList.Init(&services);

    /* create the WormholeMgr singleton */
    sLog.Success("       ServerInit", "Starting Wormhole Manager");
    sWHMgr.Init(&services);

    /* create the BubbleManager singleton */
    sLog.Success("       ServerInit", "Starting Bubble Manager");
    sBubbleMgr.Init();

    /* create a command dispatcher */
    sLog.Success("       ServerInit", "Starting Command Dispatch Manager");
    CommandDispatcher command_dispatcher( services );
    RegisterAllCommands( command_dispatcher );

    /* create console command interperter singleton */
    sLog.Success("       ServerInit", "Starting Console Manager");
    sConsole.Init(&command_dispatcher, item_factory);

    /* Service creation and registration. */
    sLog.Warning("       ServerInit", "Creating services.");

    /* Please keep the services list clean so it's easier to find things */
    /* service here are systems responding to client calls */
    /** @todo  update these to smart pointers.  -allan */
    services.RegisterService(new AccountService(&services));
    services.RegisterService(new AgentMgrService(&services));
    services.RegisterService(new AggressionMgrService(&services));
    services.RegisterService(new AlertService(&services));
    services.RegisterService(new AllianceRegistry(&services));
    services.RegisterService(new AuthService(&services));
    services.RegisterService(new BillMgrService(&services));
    services.RegisterService(new BeyonceService(&services));
    services.RegisterService(new BookmarkService(&services));
    services.RegisterService(new BrowserLockdownService(&services));
    services.RegisterService(new BulkMgrService(&services));
    services.RegisterService(new CalendarProxy(&services));
    services.RegisterService(new CalendarMgrService(&services));
    services.RegisterService(new CertificateMgrService(&services));
    services.RegisterService(new CharFittingMgrService(&services));
    services.RegisterService(new CharUnboundMgrService(&services));
    services.RegisterService(new CharMgrService(&services));
    services.RegisterService(new ClientStatLogger(&services));
    services.RegisterService(new ClientStatsMgr(&services));
    services.RegisterService(new ConfigService(&services));
    services.RegisterService(new CorpBookmarkMgrService(&services));
    services.RegisterService(new CorpMgrService(&services));
    services.RegisterService(new CorporationService(&services));
    services.RegisterService(new CorpRegistryService(&services));
    services.RegisterService(new CorpStationMgrService(&services));
    services.RegisterService(new ContractMgrService(&services));
    services.RegisterService(new ContractProxyService(&services));
    services.RegisterService(new DevToolsProviderService(&services));
    services.RegisterService(new DogmaIMService(&services));
    services.RegisterService(new DogmaService(&services));
    services.RegisterService(new DungeonExplorationMgrService(&services));
    services.RegisterService(new DungeonService(&services));
    services.RegisterService(new FactionWarMgrService(&services));
    services.RegisterService(new FactoryService(&services));
    services.RegisterService(new FleetManager(&services));
    services.RegisterService(new FleetObject(&services));
    services.RegisterService(new FleetProxyService(&services));
    services.RegisterService(new HoloscreenMgrService(&services));
    services.RegisterService(new IndexManager(&services));
    services.RegisterService(new InfoGatheringMgr(&services));
    services.RegisterService(new InsuranceService(&services));
    services.RegisterService(new InvBrokerService(&services));
    services.RegisterService(new JumpCloneService(&services));
    services.RegisterService(new KeeperService(&services));
    services.RegisterService(new LanguageService(&services));
    services.RegisterService(new LocalizationServerService(&services));
    services.RegisterService(new LookupService(&services));
    services.RegisterService(new LPService(&services));
    services.RegisterService(new LPStore(&services));
    services.RegisterService(services.lsc_service = new LSCService(&services, &command_dispatcher));
    services.RegisterService(new MailMgrService(&services));
    services.RegisterService(new MailingListMgrService(&services));
    services.RegisterService(new MapService(&services));
    services.RegisterService(new MarketProxyService(&services));
    services.RegisterService(new MissionMgrService(&services));
    services.RegisterService(new NetService(&services));
    services.RegisterService(new NotificationMgrService(&services));
    services.RegisterService(services.cache_service = new ObjCacheService(&services, sConfig.files.cacheDir.c_str()));
    services.RegisterService(new OnlineStatusService(&services));
    services.RegisterService(new PaperDollService(&services));
    services.RegisterService(new PetitionerService(&services));
    services.RegisterService(new PhotoUploadService(&services));
    services.RegisterService(new PlanetMgrService(&services));
    services.RegisterService(new planetORB(&services));
    services.RegisterService(new PosMgrService(&services));
    services.RegisterService(new RamProxyService(&services));
    services.RegisterService(new RepairService(&services));
    services.RegisterService(new ReprocessingService(&services));
    services.RegisterService(new Search(&services));
    services.RegisterService(new ScanMgrService(&services));
    services.RegisterService(new ShipService(&services));
    services.RegisterService(new SkillMgrService(&services));
    services.RegisterService(new SlashService(&services, &command_dispatcher));
    services.RegisterService(new SovereigntyMgrService(&services));
    services.RegisterService(new Standing(&services));
    services.RegisterService(new StationService(&services));
    services.RegisterService(new StationSvcService(&services));
    services.RegisterService(new TradeService(&services));
    services.RegisterService(new TutorialService(&services));
    services.RegisterService(new UserService(&services));
    services.RegisterService(new VoiceMgrService(&services));
    services.RegisterService(new VoucherService(&services));
    services.RegisterService(new WarRegistryService(&services));
    services.RegisterService(new WormHoleSvc(&services));

    sLog.Success("       ServerInit", "Priming cached objects.");
    services.cache_service->PrimeCache();

    // start up the image server
    sLog.Success("       ServerInit", "Starting Image Server");
    sImageServer.Run();
    //  this gives the imageserver server time to load so the dynamic database msgs are in order
    Sleep(250);

    sLog.Success("       ServerInit", "Loading Static Database Table Objects...");

    // Create In-Memory Database Objects for Critical and HighUse Systems:
    sLog.Log("       ServerInit", "Module Effects Table");
    sDGM_Effects_Table.Initialize();
    sLog.Log("       ServerInit", "Skill Modifiers");
    sDGM_Skill_Bonus_Modifiers_Table.Initialize();
    sLog.Log("       ServerInit", "Ship Modifiers");
    sDGM_Ship_Bonus_Modifiers_Table.Initialize();
    sLog.Log("       ServerInit", "Implant Modifiers");
    sDGM_Implant_Modifiers_Table.Initialize();
    sLog.Log("       ServerInit", "Wrecks Table");
    sDGM_Types_to_Wrecks_Table.Initialize();
    sLog.Log("       ServerInit", "Loot Table");
    sDGM_Loot_Groups_Table.Initialize();
    sLog.Log("       ServerInit", "Salvage Table");
    sDGM_Salvage_Table.Initialize();
    sLog.Log("       ServerInit", "Dungeon Data");
    sDunDataMgr.Initialize();
    sLog.Log("       ServerInit", "Asteroid Data");
    sMgrData.Initialize();
    sLog.Log("       ServerInit", "Spawn Data");
    sSpawnDataMgr.Initialize();
    sLog.Log("       ServerInit", "Planet Data");
    sPlanetDataMgr.Initialize();

    /* Custom config file options
     * current settings displayed on console at start-up
     *   -allan 7June2015
     */
    if (sConfig.server.ServerSleepTime != 10) {
        MAIN_LOOP_DELAY = sConfig.server.ServerSleepTime;
        sLog.Error("  Loop Sleep Time","**Be Careful With This Setting!**");
        sLog.Warning("  Loop Sleep Time","Changed from default 10ms to %ums.", MAIN_LOOP_DELAY);
    } else
        sLog.Success("  Loop Sleep Time","Default at 10ms.");
    int idle = sConfig.server.idleSleepTime;
    if (idle == 1000)
        sLog.Success("  Idle Sleep Time","Default at 1000ms.");
    else
        sLog.Warning("  Idle Sleep Time","Changed from default 1000ms to %ums.", idle);
    if (sConfig.server.UseShipTracking)
        sLog.Warning("    Ship Tracking","Enabled.");
    else
        sLog.Magenta("    Ship Tracking","Disabled.");
    if (sConfig.server.UseBeanCount)
        sLog.Success("     BeanCounting","Enabled.");
    else
        sLog.Magenta("     BeanCounting","Disabled.");
    if (sConfig.server.UseProfiling) {
        sLog.Success(" Server Profiling","Enabled.");
        sProfile.Init();
    } else
        sLog.Magenta(" Server Profiling","Disabled.");
    if (sConfig.npc.StaticSpawns)
        sLog.Success("    Static Spawns","Enabled.  Checks every %u minutes", sConfig.npc.StaticTimer);
    else
        sLog.Warning("    Static Spawns","Disabled.");
    if (sConfig.npc.RoamingSpawns)
        sLog.Success("   Roaming Spawns","Enabled.  Checks every %u minutes", sConfig.npc.RoamingTimer);
    else
        sLog.Warning("   Roaming Spawns","Disabled.");
    if (sConfig.rates.secRate != 1.0)
        sLog.Warning("        SecStatus","Modified at %.0f%%.", (sConfig.rates.secRate *100) );
    else
        sLog.Blue("        SecStatus","Normal.");
    if (sConfig.rates.npcBountyMultiply != 1.0)
        sLog.Warning("          Bountys","Modified at %.0f%%.", (sConfig.rates.npcBountyMultiply *100) );
    else
        sLog.Blue("          Bountys","Normal.");
    if (sConfig.rates.damageRate != 1.0)
        sLog.Warning("      All Damages","Modified at %.0f%%.", (sConfig.rates.damageRate *100) );
    else
        sLog.Blue("      All Damages","Normal.");
	if (sConfig.rates.missileRate != 1.0)
        sLog.Warning("      Missile Dmg","Modified at %.0f%%.", (sConfig.rates.missileRate *100) );
	else
        sLog.Blue("      Missile Dmg","Normal.");
	if (sConfig.rates.missileTime != 1.0)
        sLog.Warning("     Missile Time","Modified at %.0f%%.", (sConfig.rates.missileTime *100) );
	else
        sLog.Blue("     Missile Time","Normal.");
    if (sConfig.rates.turrentRate != 1.0)
        sLog.Warning("      Turrent Dmg","Modified at %.0f%%.", (sConfig.rates.turrentRate *100) );
    else
        sLog.Blue("      Turrent Dmg","Normal.");
    sLog.Success("      Decay Timer","Runs every %u minutes", sConfig.rates.WorldDecay);
    sLog.Log("","");

    //sLog.Warning("server init", "Adding NPC Market Orders.");
    //NPCMarket::CreateNPCMarketFromFile("/etc/npcMarket.xml");

    srand(Win32TimeNow());

    /* program events system */
    SetupSignals();

    services.serviceDB().SetServerOnlineStatus(true);
    sLog.Success("       ServerInit", "Alasiya EvEmu Server is Online.");

    sLog.Blue("       ServerInit", "Server Initialized in %.3f Seconds.", (GetTimeMSeconds() - profileStartTime));

    uint32 start = 0;
    EVETCPConnection* tcpc(nullptr);
    /////////////////////////////////////////////////////////////////////////////////////
    //     !!!  DO NOT PUT ANY INITIALIZATION CODE OR CALLS BELOW THIS LINE   !!!
    /////////////////////////////////////////////////////////////////////////////////////

    /*
     * THE MAIN LOOP
     * Everything except IO should happen in this loop, in this thread context.
     */

    while (RunLoops) {
        Timer::SetCurrentTime();
        start = GetTimeMSeconds();

        /* Freeze Detector Code */
        //++m_worldLoopCounter;

        if (tcpc = tcps.PopConnection())
            sEntityList.Add(new Client(services, &tcpc));

        /** @todo test for adding OpenMP here, or at other Process() points in code */
        sEntityList.Process();

        /*  process console commands, if any, and check for 'exit' command */
        RunLoops = sConsole.Process();

        /* do the stuff for thread sleeping */
        if (sEntityList.GetClientCount()) {
            start = GetTimeMSeconds() - start;
            if (MAIN_LOOP_DELAY > start)
                Sleep(start);
        } else /* if no clients, let server idle longer*/
            Sleep(idle);

    }

    sLog.Warning("   ServerShutdown", "Main loop stopped" );
    services.serviceDB().SetServerOnlineStatus(false);

    /* stop TCP listener */
    tcps.Close();
    sLog.Warning("   ServerShutdown", "TCP listener stopped." );
    /* stop Image Server */
    sImageServer.Stop();
    sLog.Warning("   ServerShutdown", "Image Server stopped." );
    /* Stop Console Command Interperter */
    //sConsole.Stop();
    /* delete the dogma attrib object */
    sLog.Warning("   ServerShutdown", "Deleting Dogma Attribute Cache" );
    sDgmTypeAttrMgr.Close();
	/* Shut down the Item system */
	sLog.Warning("   ServerShutdown", "Saving Items and Shutting down Item Factory." );
    SafeDelete(item_factory);
    /* Close the entity list */
    sEntityList.Close();
    /* Close the service manager */
    services.Close();
    /* Close the command dispatcher */
    command_dispatcher.Close();
    /* close the db handler */
    sDatabase.Close();
    /** @todo  the thread system is only implemented for tcp connections at this time. */
    /* join open threads */
    sThread.EndThreads();
    sLog.Warning("   ServerShutdown", "Alasiya EvEmu is Offline.");
    /* close logfile */
    log_close_logfile();
    exit(EXIT_SUCCESS);
}

static void SetupSignals()
{
    /* setup sigaction to prevent zombies */
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = SA_NOCLDWAIT;
    if (sigemptyset(&sa.sa_mask) == -1 ) {  /* MT safe */
        perror("SigEmptySet Failure");
        exit(EXIT_FAILURE);     /* NOT MT safe */
    }
    if (sigaction(SIGCHLD, &sa, nullptr) == -1) {  /* MT safe */
        perror("SigAction Failure");
        exit(EXIT_FAILURE);     /* NOT MT safe */
    }

    //::signal( SIGCHLD, SIG_IGN );
    ::signal( SIGINT, CatchSignal );
    ::signal( SIGTERM, CatchSignal );
    ::signal( SIGABRT, CatchSignal );

    #ifdef SIGABRT_COMPAT
    ::signal( SIGABRT_COMPAT, CatchSignal );
    #endif /* SIGABRT_COMPAT */

    #ifdef SIGBREAK
    ::signal( SIGBREAK, CatchSignal );
    #endif /* SIGBREAK */

    #ifdef SIGHUP
    ::signal( SIGHUP, CatchSignal );
    #endif /* SIGHUP */
}

static void CatchSignal( int sig_num )
{
    sLog.Log( "    Signal System", "Caught signal: %d", sig_num );

    RunLoops = false;
}

/*      Freeze Detector Code taken from TrinityCore.  figure out how to implement here (based on seeing occational freezes on main)  -allan 29Dec15
void FreezeDetectorHandler(const boost::system::error_code& error)
{
    if (!error)
    {
        uint32 curtime = getMSTime();

        uint32 worldLoopCounter = World::m_worldLoopCounter;
        if (_worldLoopCounter != worldLoopCounter)
        {
            _lastChangeMsTime = curtime;
            _worldLoopCounter = worldLoopCounter;
        }
        // possible freeze
        else if (getMSTimeDiff(_lastChangeMsTime, curtime) > _maxCoreStuckTimeInMs)
        {
            TC_LOG_ERROR("server.worldserver", "World Thread hangs, kicking out server!");
            ASSERT(false);
        }

        _freezeCheckTimer.expires_from_now(boost::posix_time::seconds(1));
        _freezeCheckTimer.async_wait(FreezeDetectorHandler);
    }
}  */
