/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    Copyright 2016 - 2026 Alasiya-EvE by Allan
    For the latest implementation status visit http://eve.alasiya.net/?p=op_status
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
    Author:     "everyone" who ever changed this file.
*/

#ifndef __EVE_VERSION_H
#define __EVE_VERSION_H

// Client version info
// supported client is Crucible v1.6.5 build 360229
// this is ALL CPP information needed by the client
static const double EVEVersionNumber = 7.31;
static const uint16 MachoNetVersion = 320;
static const int32 EVEBuildVersion = 360229;
static const char* const EVEProjectRegion = "ccp";
static const char* const EVEProjectVersion = "EVE-EVE-TRANQUILITY@ccp";
static const char* const EVEProjectCodename = "EVE-EVE-TRANQUILITY";

static const int32 EVEBirthday = 170472;

/*  Version Definitions */
//static std::string GIT_SHORT_HASH = std::string(GIT_COMMIT_HASH).erase(7, std::string::npos);
//static std::string REVISION_STRING = std::string("0.72.75-") + std::string(GIT_BRANCH) + std::string("-") + GIT_SHORT_HASH;

static const char* const EVEMU_REVISION = "0.91.17";
static const char* const EVEMU_BUILD_DATE = __DATE__;

/*  Allan's Static Definitions */
//static const char* const EVEMU_REVISION = "0.72.75";
//static const char* const EVEMU_BUILD_DATE = "24 March 2023";
/* match versions here with stated files for full support */
static const float Config_Version = 12.10f; /* eve-server.xml and EveServerConfig.cpp */
static const float Log_Version = 11.8f;    /* logtypes.h and log.ini */
/* AI versions for shitz-n-giggles */
static const float Joe_Version = 0.25f;   /* MarketBot.xml and MarketBotConf.cpp */
/* these  dont have separate config files ...yet */
static const float NPC_AI_Version = 1.75f;
static const float Swarm_AI_Version = 1.57f;
static const float Drone_AI_Version = 1.19f;
static const float Mission_Version = 0.77f;
static const float Agent_Version = 0.75f;
static const float Civilian_AI_Version = 0.73f;
static const float Sentry_AI_Version = 0.10f;
static const float POS_AI_Version = 0.01f;
static const float Scan_Version = 0.43f;

#endif
