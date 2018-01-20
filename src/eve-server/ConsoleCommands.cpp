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
    Author:			Allan
    Thanks to:		avianrr  for the idea
*/

#include <ios>
#include <iostream>
#include <fstream>

#include "ConsoleCommands.h"
#include "threading/Threading.h"

#include "effects/EffectsDataMgr.h"


ConsoleCommand::ConsoleCommand() :
m_updateTimer(sConfig.rates.WebUpdate * 60000)	//15 mins
{
    m_updateTimer.Disable();
}

void ConsoleCommand::Initialize(CommandDispatcher* cd)
{
    pCommand = cd;
	m_updateTimer.Start(sConfig.rates.WebUpdate * 60000);	// change minutes to ms for timer
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	UpdateStatus();	//initial status setting
    m_haltServer = false;
    sLog.Blue( "   ConsoleCommand", "Console Commands Initialized." );
    sLog.Yellow( "   ConsoleCommand", "Enter 'h' for current list of supported commands." );
}

bool ConsoleCommand::Process() {
    if (m_haltServer) {
        sEntityList.Shutdown();
        return false;
    }
	if (m_updateTimer.Check())
        UpdateStatus();
    /* reset timeouts because select() reset them */
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
	select(1, &fds, nullptr, nullptr, &tv);
	if (!FD_ISSET(0, &fds)) {
	    return true;
	} else {
		buf = (char*) malloc (BUFLEN);
	    if (fgets(buf, BUFLEN, stdin)) {
			if (strncmp(buf, "x", 1) == 0) {
                sLog.Warning("  Alasiya's EvEMu", "EXIT called.  Breaking out of Main Loop.");
                m_haltServer = true;
				free (buf);
				return false;
			} else if (strncmp(buf, "h", 1) == 0) {
                sLog.Green("  Alasiya's EvEMu", "Current Console Commands and Descriptions: ");
				sLog.Warning("","");
				sLog.Warning("           (h)elp", " Displays this dialog.");
				sLog.Warning("          h(e)llo", " Displays the 'Hello World' message.");
                sLog.Warning("           e(x)it", " Exits the server, saving all loaded items and loging out all connected clients.");
                sLog.Warning("        (c)lients", " Displays connected clients, showing account, character, and ip.");
                sLog.Warning("         (s)tatus", " Displays Server's System Status, showing threads, memory, and cpu time.");
                sLog.Warning("        (v)ersion", " Displays server version.");
                sLog.Warning("    (i)nformation", " Displays server information: version, memory, uptime, clients, items, systems, bubbles.");
                sLog.Warning("           s(a)ve", " Immediatly saves all loaded items.  *Broken*");
                sLog.Warning("      (b)roadcast", " Broadcasts a message to all clients thru the LocalChat window.  *Not Implemented*");
                sLog.Warning("           (n)ote", " Broadcasts a message to all clients thru a notification window.");
                sLog.Warning("        (m)essage", " Broadcasts a message to all clients thru a message window.");
                sLog.Warning("        (p)rofile", " Prints a profile of current server runtimes.  *Incomplete*");
				sLog.Warning("          (r)oles", " Prints a list of common roles and their values.");
				sLog.Warning("       c(o)mmands", " Prints a list of currently loaded Commands and their required role. (long list)");
                sLog.Warning("           (t)est", " Prints the current test object *varies*");
                sLog.Warning("        threa(d)s", " Prints a list of current threads.");
                sLog.Warning("    reload (l)ogs", " Reloads log.ini to change values without restarting server.");
			} else if (strncmp(buf, "e", 1) == 0) {
				sLog.Green("  Alasiya's EvEMu", "Server Hello:");
				sLog.Magenta("      Server Says", " Hello World!" );
			} else if (strncmp(buf, "c", 1) == 0) {
                sLog.Green("  Alasiya's EvEMu", "Client Connection Information");
				uint8 clients = sEntityList.GetClientCount();
                sLog.Warning("      Connections", " %u Current Clients Online", clients);
                sLog.Warning("      Connections", " %u Clients Connected since startup.", sEntityList.GetConnections() );
				if (clients) {
					std::vector<Client*> list;
					sEntityList.GetClients(list);
					for (auto cur : list) {
                        if (cur->GetChar().get() == nullptr) {
                            sLog.Magenta("      Connections", " Account %u Connected, but no character selected yet.", cur->GetUserID() );
                            continue;
                        } else
                            sLog.Warning("      Connections", " [(%u)%u] %s in %s(%u).  %u Minutes Online.", \
										 cur->GetUserID(), cur->GetCharacterID(), cur->GetName(), \
										 cur->GetSystemName().c_str(), cur->GetLocationID(), cur->GetChar()->OnlineTime() );
					}
				}
			} else if (strncmp(buf, "s", 1) == 0) {
			    std::string state = "";
                int64 threads = 0;
                uint8 aThreads = std::thread::hardware_concurrency();
				float vm = 0.0f, rss = 0.0f, user = 0.0f, kernel = 0.0f;
				Status(&state, &threads, &vm, &rss, &user, &kernel);
                sLog.Warning("    Server Status", "  S: %s | T: %d(%u) | RSS: %.3fMb | VM: %.3fMb | U: %.2f | K: %.2f", \
                        state.data(), threads, aThreads, rss, vm, user, kernel );
                sLog.Warning("      Connections", " %u Current Clients Online.", sEntityList.GetClientCount());
                sLog.Warning("      Connections", " %u Clients Connected since startup.", sEntityList.GetConnections() );
                if (sConfig.debug.UseProfiling)
                    sLog.Green(" Server Profiling","Enabled.");
                else
					sLog.Warning(" Server Profiling","Disabled.");
                if (sConfig.npc.StaticSpawns)
                    sLog.Green("    Static Spawns","Enabled.");
                else
					sLog.Warning("    Static Spawns","Disabled.");
                if (sConfig.npc.RoamingSpawns)
                    sLog.Green("   Roaming Spawns","Enabled.");
                else
					sLog.Warning("   Roaming Spawns","Disabled.");
                if (sConfig.rates.secRate != 1.0)
                    sLog.Green("        SecStatus","Enabled at %.0f%%.", (sConfig.rates.secRate *100) );
                else
					sLog.Warning("        SecStatus","Normal.");
                if (sConfig.rates.npcBountyMultiply != 1.0)
                    sLog.Green("          Bountys","Enabled at %.0f%%.", (sConfig.rates.npcBountyMultiply *100) );
                else
					sLog.Warning("          Bountys","Normal.");
                if (sConfig.rates.damageRate != 1.0)
                    sLog.Green("      All Damages","Enabled at %.0f%%.", (sConfig.rates.damageRate *100) );
                else
					sLog.Warning("      All Damages","Normal.");
                if (sConfig.rates.missileRoF != 1.0)
                    sLog.Green("      Missile Dmg","Enabled at %.0f%%.", (sConfig.rates.missileRoF *100) );
                else
					sLog.Warning("      Missile Dmg","Normal.");
                if (sConfig.rates.turretRoF != 1.0)
                    sLog.Green("      Turret Dmg","Enabled at %.0f%%.", (sConfig.rates.turretRoF *100) );
                else
                    sLog.Warning("      Turret Dmg","Normal.");
			} else if (strncmp(buf, "v", 1) == 0) {
                sLog.Green("  Alasiya's EvEMu", "Server Version:");
                sLog.Warning("     Server Build", " %.2f", EVE_Build );
                sLog.Warning("  Server Revision", " %s", EVEMU_REVISION );
                sLog.Warning("       Build Date", " %s", EVEMU_BUILD_DATE );
			} else if (strncmp(buf, "i", 1) == 0) {
                sLog.Green("  Alasiya's EvEMu", "Server Information:");
                sLog.Warning("     Server Build", " %.2f", EVE_Build );
                sLog.Warning("  Server Revision", " %s", EVEMU_REVISION );
                sLog.Warning("       Build Date", " %s", EVEMU_BUILD_DATE );
                //  memory
                std::string state = "";
                int64 threads = 0;
                uint8 aThreads = std::thread::hardware_concurrency();
                float vm = 0.0f, rss = 0.0f, user = 0.0f, kernel = 0.0f;
                Status(&state, &threads, &vm, &rss, &user, &kernel);
                sLog.Warning("     Memory Usage", " RSS: %.3fMb  VM: %.3fMb", rss, vm );
                sLog.Warning("    Server Status", "  S: %s | T: %d(%u) | U: %.2f | K: %.2f", \
                         state.data(), threads, aThreads, user, kernel );
                uint8 w = 0, d = 0, h = 0, m = 0, s = 0;
				GetUpTime(&w, &d, &h, &m, &s);
				if (w)
                    sLog.Warning("    Server UpTime", " %u W, %u D, %u H, %u M, %u S.", w, d, h, m, s );
				else if (d)
                    sLog.Warning("    Server UpTime", " %u D, %u H, %u M, %u S.", d, h, m, s );
				else if (h)
                    sLog.Warning("    Server UpTime", " %u H, %u M, %u S.", h, m, s );
				else if (m)
                    sLog.Warning("    Server UpTime", " %u M, %u S.", m, s );
				else
                    sLog.Warning("    Server UpTime", " %u S.", s );
                //  loaded items
                sLog.Warning("     Loaded Items", " %u", sItemFactory.Count());
                //  loaded NPCs
                sLog.Warning("      Loaded NPCs", " %u", sEntityList.GetNPCCount());
                //  loaded systems
                sLog.Warning("   Active Systems", " %u", sEntityList.GetSystemCount());
                //  loaded stations
                sLog.Warning("  Active Stations", " %u", sEntityList.GetStationCount());
                //  loaded bubbles
                sLog.Warning("   Active Bubbles", " %u", sBubbleMgr.Count());
                //  current clients
                sLog.Warning("      Connections", " %u Current Clients Online.", sEntityList.GetClientCount());
                sLog.Warning("      Connections", " %u Clients Connected since startup.", sEntityList.GetConnections() );
			} else if (strncmp(buf, "a", 1) == 0) {
                sLog.Green("  Alasiya's EvEMu", "Server SaveAll:");
                //sLog.Error("      Server Save", " Not Avalible Yet." );
				sItemFactory.SaveItems();
			} else if (strncmp(buf, "b", 1) == 0) {
                sLog.Green("  Alasiya's EvEMu", "Server Broadcast:");
                sLog.Error(" Server Broadcast", " Not Avalible Yet." );
                //const char* buff = buf +2;
				//SendMessage(buff);
			} else if (strncmp(buf, "n", 1) == 0) {
                sLog.Green("  Alasiya's EvEMu", "Server Notify:");
                const char* buff = buf +2;
				std::vector<Client*> list;
				sEntityList.GetClients(list);
				for (auto cur : list) {
                    cur->SendNotifyMsg( buff );
				}
				sLog.Warning("  Console Command", " Notification sent to all online clients." );
            } else if (strncmp(buf, "m", 1) == 0) {
                sLog.Green("  Alasiya's EvEMu", "Server Modal Message:");
                const char* buff = buf +2;
                std::vector<Client*> list;
                sEntityList.GetClients(list);
                for (auto cur : list) {
                    cur->SendInfoModalMsg( buff );
                }
                sLog.Warning("  Console Command", " Modal Message sent to all online clients." );
            } else if (strncmp(buf, "p", 1) == 0) {
                sLog.Green("  Alasiya's EvEMu", "Server Profile:");
                if (!sConfig.debug.UseProfiling) {
                    sLog.Error("   Server Profile", "Profiling is turned off.");
                    return true;
                }
                sLog.Warning("   Server Profile", "Items prefixed with an asterisk are disabled or not implemented yet.");
                uint8 w = 0, d = 0, h = 0, m = 0, s = 0;
                GetUpTime(&w, &d, &h, &m, &s);
                if (w)
                    sLog.Warning("    Server UpTime", " %u W, %u D, %u H, %u M, %u S.", w, d, h, m, s );
                else if (d)
                    sLog.Warning("    Server UpTime", " %u D, %u H, %u M, %u S.", d, h, m, s );
                else if (h)
                    sLog.Warning("    Server UpTime", " %u H, %u M, %u S.", h, m, s );
                else if (m)
                    sLog.Warning("    Server UpTime", " %u M, %u S.", m, s );
                else
                    sLog.Warning("    Server UpTime", " %u S.", s );
                sLog.Warning("      Connections", " %u Current Clients Online.", sEntityList.GetClientCount());
                sLog.Warning("      Connections", " %u Clients Connected since startup.", sEntityList.GetConnections() );
                sProfile.PrintProfile();
            } else if (strncmp(buf, "r", 1) == 0) {
                sLog.Green("  Alasiya's EvEMu", "Common Account Roles:");
                sLog.Warning("         ROLE_DEV", " %" PRIi64 "(%p)", ROLE_DEV, ROLE_DEV);
                sLog.Warning("         ROLE_STD", " %" PRIi64 "(%p)", ROLE_STD, ROLE_STD);
                sLog.Warning("         ROLE_VIP", " %" PRIi64 "(%p)", ROLE_VIP, ROLE_VIP);
                sLog.Warning("        ROLE_VIP+", " %" PRIi64 "(%p)", ROLE_ELEVATEDPLAYER, ROLE_ELEVATEDPLAYER);
                sLog.Warning("        ROLE_VIEW", " %" PRIi64 "(%p)", ROLE_VIEW, ROLE_VIEW);
                sLog.Warning("        ROLE_BOSS", " %" PRIi64 "(%p)", ROLE_BOSS, ROLE_BOSS);
                sLog.Warning("       ROLE_SLASH", " %" PRIi64 "(%p)", ROLE_SLASH, ROLE_SLASH);
                sLog.Warning("     ROLE_CREATOR", " %" PRIi64 "(%p)", ROLE_CREATOR, ROLE_CREATOR);
                sLog.White("", "");
                sLog.Green("  Alasiya's EvEMu", "Common Corp Roles:");
                sLog.Warning("         Role_All", " %" PRIi64 "(%p)", Corp::Role::All, Corp::Role::All);
                sLog.Warning("        Role_Cont", " %" PRIi64 "(%p)", Corp::Role::AllContainer, Corp::Role::AllContainer);
                sLog.Warning("       Role_Admin", " %" PRIi64 "(%p)", Corp::Role::Admin, Corp::Role::Admin);
                sLog.Warning("      Role_Hangar", " %" PRIi64 "(%p)", Corp::Role::AllHangar, Corp::Role::AllHangar);
                sLog.Warning("     Role_Account", " %" PRIi64 "(%p)", Corp::Role::AllAccount, Corp::Role::AllAccount);
                sLog.Warning("    Role_Starbase", " %" PRIi64 "(%p)", Corp::Role::AllStarbase, Corp::Role::AllStarbase);
            } else if (strncmp(buf, "o", 1) == 0) {
                pCommand->ListCommands();
            } else if (strncmp(buf, "t", 1) == 0) {
                Test();
            } else if (strncmp(buf, "d", 1) == 0) {
                uint8 maxCount = sConfig.server.MaxThreadReport;
                uint16 count = sThread.Count();
                sLog.Blue("   Active Threads", "There are %u active threads running in the server.", count);
                if (count > maxCount)
                    sLog.Warning("   Active Threads", "Individual thread IDs are not displayed for more than %u active threads.", maxCount);
                else
                    sThread.ListThreads();
            } else if (strncmp(buf, "l", 1) == 0) {
                /*
                sLog.~NewLog();
                sLog.InitializeLogging(sConfig.files.logDir);
                */
                if (load_log_settings(sConfig.files.logSettings.c_str())) {
                    sLog.Green("  Alasiya's EvEMu", "Log settings reloaded from %s", sConfig.files.logSettings.c_str() );
                    // reset config switches based on log settings
                    sConfig.server.StackTrace = is_log_enabled(SERVER__STACKTRACE);
                    sConfig.server.UseBeanCount = is_log_enabled(SERVER__BEANCOUNT);
                    sConfig.debug.IsTestServer = is_log_enabled(SERVER__TESTSERVER);
                } else
                    sLog.Warning("  Alasiya's EvEMu", "Unable to reload settings from %s", sConfig.files.logSettings.c_str() );
			} else {
				sLog.Error("  Alasiya's EvEMu", "Command not recognized: %s", buf);
			}
		}
		free (buf);
		return true;
	}
}

void ConsoleCommand::GetUpTime(uint8* w, uint8* d, uint8* h, uint8* m, uint8* s) {
    uint32 seconds = sEntityList.GetStamp() - 1000;
    float minutes = seconds/60;
    float hours = minutes/60;
	float days = hours/24;
	float weeks = days/7;

    *s = fmod(seconds,60);
    *m = fmod(minutes,60);
    *h = fmod(hours,24);
	*d = fmod(days,7);
	*w = fmod(weeks,4);
}

void ConsoleCommand::SendMessage(const char* msg) {
	// LSCChannel::SendMessage(Client * c, const char * message, bool self)
    //  this isnt how it works....need more info
}

void ConsoleCommand::Status(std::string* state, int64* threads, float* vm_usage, float* resident_set, float* user, float* kernel)
{
    // the fields we want
    std::string ignore = "", run_state = "";
    int64 num_threads = 0;  //this is saved from OS as long decimal....*sigh*  gotta allocate long int for it or weird shit happens.
    int64 vsize = 0;      //in bytes
    int64 rss = 0;			//in pages
	float utime = 0.0f, stime = 0.0f;

    // stat seems to give the most reliable results
    std::ifstream ifs ("/proc/self/stat", std::ios_base::in);
    ifs >> ignore >> ignore >> run_state >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
        >> ignore >> ignore >> ignore >> utime >> stime >> ignore >> ignore >> ignore >> ignore >> num_threads
        >> ignore >> ignore >> vsize >> rss;
	ifs.close();

    if (sConfig.debug.IsTestServer)
        _log(SERVER__INFO, "ConsoleCommand::Status() proc/self/stat returns RSS: %i, VM: %u", rss, vsize);

	*state = run_state;
	/*	state = One character from the string "RSDZTW" where
			  R is running,
			  S is sleeping in an interruptible wait,
			  D is waiting in uninterruptible disk sleep,
			  Z is zombie,
			  T is traced or stopped (on a signal),
			  W is paging
        */
	*threads = num_threads;

	*user = utime/sysconf(_SC_CLK_TCK)/100.0;
	*kernel = stime/sysconf(_SC_CLK_TCK)/100.0;

    *vm_usage     = ((vsize / sysconf(_SC_PAGE_SIZE)) /1024.0 /6);
    //rss (in pages) * page_size(in bytes, converted to k), then convert to Mb.
    *resident_set = (rss * (sysconf(_SC_PAGE_SIZE) /1024.0) /1024.0);

}

void ConsoleCommand::MemStatus(float* vm_usage, float* resident_set)
{
    std::string ignore = "";
    // the fields we want
    int64 vsize = 0;      //in bytes
    int64 rss = 0;			//in pages

    // stat seems to give the most reliable results
    std::ifstream ifs ("/proc/self/stat", std::ios_base::in);
    ifs >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
        >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
        >> ignore >> ignore >> vsize >> rss;
	ifs.close();

    *vm_usage     = ((vsize / sysconf(_SC_PAGE_SIZE)) /1024.0 /6);
	//rss (in pages) * page_size(in bytes, converted to k), then convert to Mb.
    *resident_set = (rss * (sysconf(_SC_PAGE_SIZE) /1024.0) /1024.0);
}

void ConsoleCommand::Test()
{
    sLog.Green("  Alasiya's EvEMu", "Server Test:");
    sLog.Error("     Allan\'s Test", "Nothing Avalible at this time.");
}

void ConsoleCommand::UpdateStatus() {
	std::string state = "";
	int64 threads = 0;
	float vm = 0.0f, rss = 0.0f, user = 0.0f, kernel = 0.0f;
	Status(&state, &threads, &vm, &rss, &user, &kernel);
    if (sConfig.debug.IsTestServer)
        _log(SERVER__INFO, "Current Mem usage - RSS: %f, VM: %f", rss, vm);
    ServiceDB::SaveServerStats(threads + sThread.Count(), rss, vm, user, kernel, sItemFactory.Count(), sBubbleMgr.Count());
}

