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
#include "StaticDataMgr.h"
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
#include "cache/BulkDB.h"
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
#include "effects/EffectsDataMgr.h"
// dungeon services
#include "dungeon/DungeonExplorationMgrService.h"
#include "dungeon/DungeonService.h"
// entity service (player drones)
#include "npc/EntityService.h"
// exploration services
#include "exploration/ScanMgrService.h"
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
#include "market/MarketBotMgr.h"
// missions services
#include "missions/MissionMgrService.h"
// planet services
#include "planet/Planet.h"
#include "planet/PlanetDataMgr.h"
#include "planet/PlanetMgrBound.h"
#include "planet/PlanetORBBound.h"
// pos services
#include "pos/PosMgrService.h"
#include "pos/Structure.h"
// search services
#include "search/Search.h"
// ship services
#include "ship/BeyonceService.h"
#include "ship/ShipService.h"
// standing services
#include "standing/FactionWarMgrService.h"
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
#include "system/ScenarioService.h"
#include "system/SovereigntyMgrService.h"
#include "system/WormholeSvc.h"
// cosmic managers
#include "system/cosmicMgrs/DungeonMgr.h"
#include "system/cosmicMgrs/SpawnMgr.h"
#include "system/cosmicMgrs/WormholeMgr.h"
//console commands
#include "ConsoleCommands.h"


static const char* const SRV_CONFIG_FILE = EVEMU_ROOT "/etc/eve-server.xml";

static void SetupSignals();
static void CatchSignal( int sig_num );

static volatile bool m_run = true;

int main( int argc, char* argv[] )
{
    double profileStartTime = GetTimeMSeconds();

    /* Load server configuration */
    if (!sConfig.ParseFile(SRV_CONFIG_FILE)) {
        printf("ERROR: Loading server configuration '%s' failed.", SRV_CONFIG_FILE );
        std::cout << std::endl << "press any key to exit...";  std::cin.get();
        sConfig.~EVEServerConfig();
        return EXIT_FAILURE;
    }

    /* set current time for timer */
    Timer::SetCurrentTime();

    /* init logging */
    sLog.InitializeLogging(sConfig.files.logDir);
    sThread.Initialize();
    sLog.White( "        Threading", "Starting Main Loop thread with ID 0x%X", pthread_self() );
    //sThread.AddThread(pthread_self());
    sLog.White("       ServerInit", "Loading server");

    /* Load server log settings */
    if ( load_log_settings( sConfig.files.logSettings.c_str() ) )
        sLog.Green( "       ServerInit", "Log settings loaded from %s", sConfig.files.logSettings.c_str() );
    else
        sLog.Warning( "       ServerInit", "Unable to read %s (this file is optional)", sConfig.files.logSettings.c_str() );

    /* open up the log file if specified */
    if (!sConfig.files.logDir.empty()) {
        std::string logFile = sConfig.files.logDir + "eve-server.log";
        if( log_open_logfile( logFile.c_str() ) )
            sLog.Green( "       ServerInit", "Found log directory %s", sConfig.files.logDir.c_str() );
        else
            sLog.Warning( "       ServerInit", "Unable to find log directory '%s', only logging to the screen now.", sConfig.files.logDir.c_str() );
    }

    /* Start up the TCP server */
    EVETCPServer tcps;
    char errbuf[ TCPCONN_ERRBUF_SIZE ];
    if (tcps.Open(sConfig.net.port, errbuf)) {
        sLog.Green( "       ServerInit", "TCP Listener started on port %u.", sConfig.net.port );
    } else {
        sLog.Error( "       ServerInit", "Failed to start TCP listener on port %u: %s.", sConfig.net.port, errbuf );
        std::cout << std::endl << "press any key to exit...";  std::cin.get();
        return EXIT_FAILURE;
    }
    Sleep(250);

    sLog.White("", "");
    sLog.White(" Supported Client"," %s", EVEProjectVersion);
    sLog.White("   Client Version"," %.2f", EVEVersionNumber);
    sLog.White("     Client Build"," %d", EVEBuildVersion);
    sLog.White("         MachoNet"," %u", MachoNetVersion);
    sLog.White("     Server Build", " %.2f", EVE_Build );
    sLog.White("  Server Revision", " %s", EVEMU_REVISION );
    sLog.White("       Build Date", " %s", EVEMU_BUILD_DATE );
    sLog.White("MarketBot Version", " %.1f", Bot_Version );
    sLog.White("   Config Version", " %.1f", Config_Version );
    sLog.White("      Log Version", " %.1f", Log_Version );
    sLog.White("", "");

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

    /* create a single item factory */
    sLog.Green("       ServerInit", "Starting Item Factory");
    ItemFactory* item_factory = new ItemFactory();

    /* initialize EntityList singleton, clientID seed and start tic timer */
    sLog.Green("       ServerInit", "Starting Entity List");
    sEntityList.Initialize();

    /* create a service manager */
    sLog.Green("       ServerInit", "Starting Service Manager");
    PyServiceMgr pyServMgr( 888444, sEntityList, item_factory );

    /* create the WormholeMgr singleton */
    sLog.Green("       ServerInit", "Starting Wormhole Manager");
    sWHMgr.Initialize(&pyServMgr);

    /* create the BubbleManager singleton */
    sLog.Green("       ServerInit", "Starting Bubble Manager");
    sBubbleMgr.Initialize();

    /* create the MarketBot singleton */
    sLog.Green("       ServerInit", "Starting Market Bot Manager");
    sMktBotMgr.Initialize();

    /* create a command dispatcher */
    sLog.Green("       ServerInit", "Starting Command Dispatch Manager");
    CommandDispatcher command_dispatcher( pyServMgr );
    RegisterAllCommands( command_dispatcher );

    /* create console command interperter singleton */
    sLog.Green("       ServerInit", "Starting Console Manager");
    sConsole.Initialize(&command_dispatcher, item_factory);

    /* Service creation and registration. */
    sLog.Yellow("       ServerInit", "Creating pyServMgr."); // 85 currently known pyServMgr
    double startTime = GetTimeMSeconds();
    /* Please keep the pyServMgr list clean so it's easier to find things */
    /* service here are systems responding to client calls */
    // move this into a service Init() function?   will need more work to do...
    pyServMgr.RegisterService("account", new AccountService(&pyServMgr));
    pyServMgr.RegisterService("agentMgr", new AgentMgrService(&pyServMgr));
    pyServMgr.RegisterService("aggressionMgr", new AggressionMgrService(&pyServMgr));
    pyServMgr.RegisterService("alert", new AlertService(&pyServMgr));
    pyServMgr.RegisterService("allianceRegistry", new AllianceRegistry(&pyServMgr));
    pyServMgr.RegisterService("authentication", new AuthService(&pyServMgr));
    pyServMgr.RegisterService("billMgr", new BillMgrService(&pyServMgr));
    pyServMgr.RegisterService("beyonce", new BeyonceService(&pyServMgr));
    pyServMgr.RegisterService("bookmark", new BookmarkService(&pyServMgr));
    pyServMgr.RegisterService("browserLockdownSvc", new BrowserLockdownService(&pyServMgr));
    pyServMgr.RegisterService("bulkMgr", new BulkMgrService(&pyServMgr));
    pyServMgr.RegisterService("CalendarProxy", new CalendarProxy(&pyServMgr));
    pyServMgr.RegisterService("calendarMgr", new CalendarMgrService(&pyServMgr));
    pyServMgr.RegisterService("certificateMgr", new CertificateMgrService(&pyServMgr));
    pyServMgr.RegisterService("charFittingMgr", new CharFittingMgrService(&pyServMgr));
    pyServMgr.RegisterService("charUnboundMgr", new CharUnboundMgrService(&pyServMgr));
    pyServMgr.RegisterService("charMgr", new CharMgrService(&pyServMgr));
    pyServMgr.RegisterService("clientStatLogger", new ClientStatLogger(&pyServMgr));
    pyServMgr.RegisterService("clientStatsMgr", new ClientStatsMgr(&pyServMgr));
    pyServMgr.RegisterService("config", new ConfigService(&pyServMgr));
    pyServMgr.RegisterService("corpBookmarkMgr", new CorpBookmarkMgrService(&pyServMgr));
    pyServMgr.RegisterService("corpmgr", new CorpMgrService(&pyServMgr));
    pyServMgr.RegisterService("corporationSvc", new CorporationService(&pyServMgr));
    pyServMgr.RegisterService("corpRegistry", new CorpRegistryService(&pyServMgr));
    pyServMgr.RegisterService("corpStationMgr", new CorpStationMgrService(&pyServMgr));
    pyServMgr.RegisterService("contractMgr", new ContractMgrService(&pyServMgr));
    pyServMgr.RegisterService("contractProxy", new ContractProxyService(&pyServMgr));
    pyServMgr.RegisterService("devToolsProvider", new DevToolsProviderService(&pyServMgr));
    pyServMgr.RegisterService("dogmaIM", new DogmaIMService(&pyServMgr));
    pyServMgr.RegisterService("dogma", new DogmaService(&pyServMgr));
    pyServMgr.RegisterService("dungeonExplorationMgr", new DungeonExplorationMgrService(&pyServMgr));
    pyServMgr.RegisterService("dungeon", new DungeonService(&pyServMgr));
    pyServMgr.RegisterService("entity", new EntityService(&pyServMgr));
    pyServMgr.RegisterService("facWarMgr", new FactionWarMgrService(&pyServMgr));
    pyServMgr.RegisterService("factory", new FactoryService(&pyServMgr));
    pyServMgr.RegisterService("fleetMgr", new FleetManager(&pyServMgr));
    pyServMgr.RegisterService("fleetObjectHandler", new FleetObject(&pyServMgr));
    pyServMgr.RegisterService("fleetProxy", new FleetProxyService(&pyServMgr));
    pyServMgr.RegisterService("holoscreenMgr", new HoloscreenMgrService(&pyServMgr));
    pyServMgr.RegisterService("devIndexManager", new IndexManager(&pyServMgr));
    pyServMgr.RegisterService("infoGatheringMgr", new InfoGatheringMgr(&pyServMgr));
    pyServMgr.RegisterService("insuranceSvc", new InsuranceService(&pyServMgr));
    pyServMgr.RegisterService("invbroker", new InvBrokerService(&pyServMgr));
    pyServMgr.RegisterService("jumpCloneSvc", new JumpCloneService(&pyServMgr));
    pyServMgr.RegisterService("keeper", new KeeperService(&pyServMgr));
    pyServMgr.RegisterService("languageSvc", new LanguageService(&pyServMgr));
    pyServMgr.RegisterService("localizationServer", new LocalizationServerService(&pyServMgr));
    pyServMgr.RegisterService("lookupSvc", new LookupService(&pyServMgr));
    pyServMgr.RegisterService("LPSvc", new LPService(&pyServMgr));
    pyServMgr.RegisterService("storeServer", new LPStore(&pyServMgr));
    pyServMgr.RegisterService("LSC", (pyServMgr.lsc_service = new LSCService(&pyServMgr, &command_dispatcher)));
    pyServMgr.RegisterService("mailMgr", new MailMgrService(&pyServMgr));
    pyServMgr.RegisterService("mailingListsMgr", new MailingListMgrService(&pyServMgr));
    pyServMgr.RegisterService("map", new MapService(&pyServMgr));
    pyServMgr.RegisterService("marketProxy", new MarketProxyService(&pyServMgr));
    pyServMgr.RegisterService("missionMgr", new MissionMgrService(&pyServMgr));
    pyServMgr.RegisterService("machoNet", new NetService(&pyServMgr));
    pyServMgr.RegisterService("notificationMgr", new NotificationMgrService(&pyServMgr));
    pyServMgr.RegisterService("objectCaching", (pyServMgr.cache_service = new ObjCacheService(&pyServMgr, sConfig.files.cacheDir.c_str())));
    pyServMgr.RegisterService("onlineStatus", new OnlineStatusService(&pyServMgr));
    pyServMgr.RegisterService("paperDollServer", new PaperDollService(&pyServMgr));
    pyServMgr.RegisterService("petitioner", new PetitionerService(&pyServMgr));
    pyServMgr.RegisterService("photoUploadSvc", new PhotoUploadService(&pyServMgr));
    pyServMgr.RegisterService("planetMgr", new PlanetMgrService(&pyServMgr));
    pyServMgr.RegisterService("planetOrbitalRegistryBroker", new planetORB(&pyServMgr));
    pyServMgr.RegisterService("posMgr", new PosMgrService(&pyServMgr));
    pyServMgr.RegisterService("ramProxy", new RamProxyService(&pyServMgr));
    pyServMgr.RegisterService("repairSvc", new RepairService(&pyServMgr));
    pyServMgr.RegisterService("reprocessingSvc", new ReprocessingService(&pyServMgr));
    pyServMgr.RegisterService("search", new Search(&pyServMgr));
    pyServMgr.RegisterService("scanMgr", new ScanMgrService(&pyServMgr));
    pyServMgr.RegisterService("ship", new ShipService(&pyServMgr));
    pyServMgr.RegisterService("skillMgr", new SkillMgrService(&pyServMgr));
    pyServMgr.RegisterService("slash", new SlashService(&pyServMgr, &command_dispatcher));
    pyServMgr.RegisterService("sovMgr", new SovereigntyMgrService(&pyServMgr));
    pyServMgr.RegisterService("standing2", new Standing(&pyServMgr));
    pyServMgr.RegisterService("station", new StationService(&pyServMgr));
    pyServMgr.RegisterService("stationSvc", new StationSvcService(&pyServMgr));
    pyServMgr.RegisterService("trademgr", new TradeService(&pyServMgr));
    pyServMgr.RegisterService("tutorialSvc", new TutorialService(&pyServMgr));
    pyServMgr.RegisterService("userSvc", new UserService(&pyServMgr));
    pyServMgr.RegisterService("voiceMgr", new VoiceMgrService(&pyServMgr));
    pyServMgr.RegisterService("voucher", new VoucherService(&pyServMgr));
    pyServMgr.RegisterService("warRegistry", new WarRegistryService(&pyServMgr));
    pyServMgr.RegisterService("wormholeMgr", new WormHoleSvc(&pyServMgr));
    pyServMgr.Initalize(startTime);

    /** @note  this is NOT used correctly yet...
    sLog.Yellow("       ServerInit", "Priming cached objects.");
    pyServMgr.cache_service->PrimeCache();
    */

    // start up the image server
    sLog.Green("       ServerInit", "Starting Image Server");
    sImageServer.Run();
    //  this gives the imageserver's server time to load so the dynamic database msgs are in order
    Sleep(250);

    // Create In-Memory Database Objects for Critical and HighUse Systems:
    sLog.Yellow("       ServerInit", "Loading Static Database Table Objects...");
    sLog.Green("       ServerInit", "BulkData");
    if (sConfig.server.BulkDataOD)
        sLog.Yellow("      BulkDataMgr", "PreLoading Disabled. BulkData will load on first call.");
    else
        sBulkDB.Initialize();
    sLog.Green("       ServerInit", "Effect Data Sets");
    sFxDataMgr.Initialize();
    sLog.Green("       ServerInit", "Wreck Data");
    sDGM_Types_to_Wrecks_Table.Initialize();
    sLog.Green("       ServerInit", "Loot Data");
    sDGM_Loot_Groups_Table.Initialize();
    sLog.Green("       ServerInit", "Salvage Data");
    sDGM_Salvage_Table.Initialize();
    sLog.Green("       ServerInit", "Dungeon Data");
    sDunDataMgr.Initialize();
    sLog.Green("       ServerInit", "Spawn Data");
    sSpawnDataMgr.Initialize();
    sLog.Green("       ServerInit", "Planet Data");
    sPlanetDataMgr.Initialize();
    sLog.Green("       ServerInit", "PI Data");
    sPIDataMgr.Initialize();
    sLog.Green("       ServerInit", "Misc Data Sets");
    sDataMgr.Initialize();

    /* Custom config file options
     * current settings displayed on console at start-up
     *   -allan 7June2015
     */
    uint8 m_sleepTime = sConfig.server.ServerSleepTime; // delay 10 ms.
    if (sConfig.server.ServerSleepTime != 10) {
        m_sleepTime = sConfig.server.ServerSleepTime;
        sLog.Error("  Loop Sleep Time","**Be Careful With This Setting!**");
        sLog.Warning("  Loop Sleep Time","Changed from default 10ms to %ums.", m_sleepTime);
    } else
        sLog.Green("  Loop Sleep Time","Default at 10ms.");
    uint16 m_idle = sConfig.server.idleSleepTime;
    if (m_idle == 1000)
        sLog.Green("  Idle Sleep Time","Default at 1000ms.");
    else
        sLog.Yellow("  Idle Sleep Time","Changed from default 1000ms to %ums.", m_idle);
    if (sConfig.server.UseShipTracking)
        sLog.Warning("    Ship Tracking","Enabled.");
    else
        sLog.Warning("    Ship Tracking","Disabled.");
    if (sConfig.server.UseBeanCount)
        sLog.Green("     BeanCounting","Enabled.");
    else
        sLog.Warning("     BeanCounting","Disabled.");
    if (sConfig.server.UseProfiling) {
        sLog.Green(" Server Profiling","Enabled.");
        sProfile.Init();
    } else
        sLog.Warning(" Server Profiling","Disabled.");
    if (sConfig.cosmic.EnablePI)
        sLog.Green("        PI System","Enabled.");
    else
        sLog.Warning("        PI System","Disabled.");
    if (sConfig.npc.EnableDrones)
        sLog.Green("    Player Drones","Enabled.");
    else
        sLog.Warning("    Player Drones","Disabled.");
    if (sConfig.npc.StaticSpawns)
        sLog.Green("    Static Spawns","Enabled.  Checks every %u minutes", sConfig.npc.StaticTimer);
    else
        sLog.Warning("    Static Spawns","Disabled.");
    if (sConfig.npc.RoamingSpawns)
        sLog.Green("   Roaming Spawns","Enabled.  Checks every %u minutes", sConfig.npc.RoamingTimer);
    else
        sLog.Warning("   Roaming Spawns","Disabled.");
    if (sConfig.rates.secRate != 1.0)
        sLog.Yellow("        SecStatus","Modified at %.0f%%.", (sConfig.rates.secRate *100) );
    else
        sLog.Blue("        SecStatus","Normal.");
    if (sConfig.rates.npcBountyMultiply != 1.0)
        sLog.Yellow("          Bountys","Modified at %.0f%%.", (sConfig.rates.npcBountyMultiply *100) );
    else
        sLog.Blue("          Bountys","Normal.");
    if (sConfig.rates.damageRate != 1.0)
        sLog.Yellow("      All Damages","Modified at %.0f%%.", (sConfig.rates.damageRate *100) );
    else
        sLog.Blue("      All Damages","Normal.");
	if (sConfig.rates.missileRate != 1.0)
        sLog.Yellow("      Missile Dmg","Modified at %.0f%%.", (sConfig.rates.missileRate *100) );
	else
        sLog.Blue("      Missile Dmg","Normal.");
	if (sConfig.rates.missileTime != 1.0)
        sLog.Yellow("     Missile Time","Modified at %.0f%%.", (sConfig.rates.missileTime *100) );
	else
        sLog.Blue("     Missile Time","Normal.");
    if (sConfig.rates.turrentRate != 1.0)
        sLog.Yellow("      Turrent Dmg","Modified at %.0f%%.", (sConfig.rates.turrentRate *100) );
    else
        sLog.Blue("      Turrent Dmg","Normal.");
    sLog.Green("      Decay Timer","Runs every %u minutes", sConfig.rates.WorldDecay);
    sLog.White("","");

    //sLog.Warning("server init", "Adding NPC Market Orders.");
    //NPCMarket::CreateNPCMarketFromFile("/etc/npcMarket.xml");

    /* program events system */
    SetupSignals();

    ServiceDB m_sdb;
    m_sdb.SetServerOnlineStatus(true);

    uint32 start = 0;
    EVETCPConnection* tcpc(nullptr);

    sLog.Blue("       ServerInit", "Server Initialized in %.3f Seconds.", (GetTimeMSeconds() - profileStartTime) /1000);
    sLog.Green("       ServerInit", "Alasiya EvEmu Server is Online.");

    /////////////////////////////////////////////////////////////////////////////////////
    //     !!!  DO NOT PUT ANY INITIALIZATION CODE OR CALLS BELOW THIS LINE   !!!
    /////////////////////////////////////////////////////////////////////////////////////

    /*
     * THE MAIN LOOP
     * Everything except IO should happen in this loop, in this thread context.
     */
    while (m_run) {
        Timer::SetCurrentTime();
        start = GetTimeMSeconds();

        /* Freeze Detector Code */
        //++m_worldLoopCounter;

        if (tcpc = tcps.PopConnection())
            sEntityList.Add(new Client(pyServMgr, &tcpc));

        /** @todo test for adding OpenMP here, or at other Process() points in code */
        sEntityList.Process();

        /*  process console commands, if any, and check for 'exit' command */
        m_run = sConsole.Process();

        /* do the stuff for thread sleeping */
        if (sEntityList.GetClientCount()) {
            start = GetTimeMSeconds() - start;
            if (m_sleepTime > start)
                Sleep(start);
        } else /* if no clients, let server idle longer*/
            Sleep(m_idle);
    }

    /*
     * end of main loop
     *  at this point, server has been killed, and these are cleanup methods below here
     */

    sLog.Warning("   ServerShutdown", "Main loop stopped" );
    m_sdb.SetServerOnlineStatus(false);

    /* stop TCP listener */
    tcps.Close();
    sLog.Warning("   ServerShutdown", "TCP listener stopped." );
    /* stop Image Server */
    sImageServer.Stop();
    sLog.Warning("   ServerShutdown", "Image Server stopped." );
    /* Close the command dispatcher */
    command_dispatcher.Close();
    /* Stop Console Command Interperter */
    //sConsole.Stop();
    /* Close the entity list */
    sEntityList.Close();
	sLog.Warning("   ServerShutdown", "Saving Items." );
    /* Shut down the Item system */
    item_factory->SaveItems();
    sLog.Warning("   ServerShutdown", "Shutting down Item Factory." );
    SafeDelete(item_factory);
    /* Close the service manager */
    pyServMgr.Close();
    /* Close the bulk data manager */
    sLog.Warning("   ServerShutdown", "Closing the BulkData Manager." );
    sBulkDB.Close();
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
    //::signal( SIGSEGV, CatchSignal );

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
    sLog.White( "    Signal System", "Caught signal: %d", sig_num );
    EvE::traceStack();
    m_run = false;
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
