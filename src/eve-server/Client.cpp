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
    Author:     Zhur
    Updates:    Allan, AlTahir(DaVinci)
*/

#include "eve-server.h"
#include "../eve-common/EVEVersion.h"

#include "Client.h"
#include "ConsoleCommands.h"
#include "EVEServerConfig.h"
#include "LiveUpdateDB.h"
#include "PyBoundObject.h"
#include "StaticDataMgr.h"
#include "chat/LSCService.h"
#include "character/CharUnboundMgrService.h"
#include "corporation/CorporationDB.h"
#include "fleet/FleetService.h"
#include "imageserver/ImageServer.h"
#include "missions/MissionDataMgr.h"
#include "npc/NPC.h"
#include "npc/Drone.h"
#include "npc/DroneAI.h"
#include "station/StationDataMgr.h"
#include "station/StationOffice.h"
#include "system/DestinyManager.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"
#include "exploration/Scan.h"
#include "station/Station.h"
#include "station/TradeService.h"

static const uint32 PING_INTERVAL_MS = 600000; //10m

Client::Client(PyServiceMgr &services, EVETCPConnection** con)
: EVEClientSession(con),
  m_TS(nullptr),
  m_scan(nullptr),
  pShipSE(nullptr),
  m_system(nullptr),
  m_services(services),
  m_movePoint(NULL_ORIGIN),
  m_clientState(ClientState::csIdle),
  m_stateTimer(0),
  m_jumpTimer(0),
  m_pingTimer(PING_INTERVAL_MS),
  m_scanTimer(0),
  m_cloakTimer(0),
  m_fleetTimer(0),
  m_invulTimer(0),
  m_clientTimer(0),
  m_logoutTimer(0),
  m_jetcanTimer(0),
  m_sessionTimer(0),
  m_destinyEventQueue(new PyList()),
  m_destinyUpdateQueue(new PyList()),
  m_nextNotifySequence(0)
{
    m_pod = ShipItemRef(nullptr);
    m_ship = ShipItemRef(nullptr);

    m_afk = false;
    m_login = true;
    m_invul = true;
    m_wing = false;
    m_fleet = false;
    m_squad = false;
    m_loaded = false;
    m_undock = false;
    m_showall = false;
    m_beyonce = false;
    m_canThrow = false;
    m_packaged = false;
    m_portrait = false;
    m_autoPilot = false;
    m_bubbleWait = false;     // allow client processing of subsquent destiny msgs
    m_setStateSent = false;
    m_sessionChangeActive = false;

    m_toGate = 0;
    m_locationID = 0;
    m_moveSystemID = 0;
    m_timeEndTrain = 0;
    m_dockStationID = 0;

    m_lpMap.clear();
    m_channels.clear();
    m_hangarLoaded.clear();
    mDogmaMessages.clear();

    // Start handshake
    Reset();
}

Client::~Client() {
    if (!m_loaded)
        return;
    if (m_char.get() != nullptr) {   // we have valid character
        /** @todo  - for warping to random point when client logs out in space...
         *      1)  check client IsInSpace(?)
         *      2)  set timer to delay removing bubble/sysmgr/destiny...or check based on destiny->isstopped() or timer on destiny->ismoving()
         *      3)  set current position (DB::chrCharacters.logoutPosition?)  initial code in place for warp-in on login
         *      4)  generate random point to warp to ** use m_SGP.GetRandPointInSystem(systemID, distance)
         *      5)  _warp to random point, but DONT make/update new bubble with entering ship
         *      6)  remove client from sysmgr/destiny/server
         */
        if (IsDocked()) {
            StationItemRef sRef = sEntityList.GetStationByID(m_locationID);
            sRef->RemoveGuest(this);
            if (GetTradeSession()) {
                TradeService* mts = (TradeService*)(m_services.LookupService("trademgr"));
                mts->CancelTrade(this);
            }
            if (!sConsole.IsShutdown())
                OnCharNoLongerInStation();
        }

        if (pShipSE != nullptr)
            WarpOut();

        if (!sConsole.IsDbError()) {
            ServiceDB::SetAccountOnlineStatus(GetUserID(), false);
            ServiceDB::SetCharacterOnlineStatus(m_char->itemID(), false);
        }

        //if (!sConsole.IsShutdown()) {
        // LSC logout
            for (auto cur : m_channels)
                cur->LeaveChannel(this);
            // remove ship and char memory objects from running server
            m_system->RemoveClient(this, IsDocked(), true);
            // char logout removed fleet data, if any
            m_char->LogOut();
            // ship logout also offlines modules.  this resets ship effects data for error fix on char relog
            m_ship->LogOut();
       // }
    }

    m_services.ClearBoundObjects(this);

    m_TS = nullptr;
    m_system = nullptr;

    //sEntityList.RemoveSID(GetSessionID());

    SafeDelete(m_scan);
    SafeDelete(pShipSE);
    PyDecRef(m_destinyEventQueue);
    PyDecRef(m_destinyUpdateQueue);
}

bool Client::ProcessNet()
{
    if (GetState() != TCPConnection::STATE_CONNECTED)
        return false;

    PyPacket *p(nullptr);
    while (p = PopPacket()) {
        if (is_log_enabled(CLIENT__IN_ALL)) {
            _log(CLIENT__IN_ALL, "Received packet:");
            PyLogDumpVisitor dumper(CLIENT__IN_ALL, CLIENT__IN_ALL);
            p->Dump(CLIENT__IN_ALL, dumper);
        }
        try {
            if (!DispatchPacket(p))
                sLog.Error("Client", "%s: Failed to dispatch packet of type %s (%d).", m_char->itemName().c_str(), MACHONETMSG_TYPE_NAMES[ p->type ], (int)p->type);
        }
        catch(PyException& e) {
            _SendException(p->dest, p->source.callID, p->type, WRAPPEDEXCEPTION, &e.ssException);
        }

        SafeDelete(p);
    }

    // send queue
    _SendQueuedUpdates();

    return true;
}

bool Client::SelectCharacter(uint32 char_id) {
    InitSession(char_id);

    sItemFactory.SetUsingClient(this);
    m_system = sEntityList.FindOrBootSystem(m_SystemData.systemID);

    if (m_system == nullptr) {
        sLog.Error("Client::LoginToSystem()", "Failed to boot system %u for char %u.", m_SystemData.systemID, char_id);
        SendErrorMsg("Unable to boot system %u", m_SystemData.systemID);
        return false;
    }

    m_char = sItemFactory.GetCharacter(char_id);
    if (m_char.get() == nullptr) {
        sLog.Error("Client::SelectCharacter()", "GetChar for %u = nullptr", char_id);
        sItemFactory.UnsetUsingClient();
        return false;
    }

    m_char->SetClient(this);
    m_char->VerifySP();
    m_char->UpdateSkillQueue();
    /*
    // this will eventually check for d/c timer and rejoin existing fleet if applicable
    CharFleetData fleet;
        fleet.wingID = 0;
        fleet.fleetID = 0;
        fleet.squadID = 0;
        fleet.fleetJob = 0;
        fleet.joinTime = 0;
        fleet.fleetRole = 0;
        fleet.fleetBooster = 0;
    m_char->SetFleetData(fleet);
    */

    SetPodItem();

    m_ship = sItemFactory.GetShip(m_shipId);
    if (m_ship.get() == nullptr) {
        sLog.Error("Client::SelectCharacter()", "shipID %u invalid for %u.  Picking new ship...", m_shipId, char_id);
        PickAlternateShip();    // incase shipID wasnt set correctly in db (seen on 'bad' Damage::Killed())
        m_ship = sItemFactory.GetShip(m_shipId);
        if (m_ship.get() == nullptr) {
            sLog.Error("Client::SelectCharacter()", "shipID %u for %u also invalid.  Loading Pod.", m_shipId, char_id);
            m_ship = m_pod;
        }
        m_shipId = m_ship->itemID();
    }

    // register new pilot in system data
    m_system->AddClient(this, IsStation(m_locationID), m_login);
    MapDB::AddPilotToDynamicData(m_SystemData.systemID, true, IsStation(m_locationID), m_login);

    m_char->Move(m_shipId, flagPilot);
    m_ship->SetPlayer(this);

    GPoint pos(NULL_ORIGIN);
    if (IsSolarSystem(m_locationID))
        pos = m_ship->position();
    MoveToLocation(m_locationID, pos);

    if (IsSolarSystem(m_locationID)) {
        m_invulTimer.Start(ClientTimers::WarpInInvul);
        WarpIn();
    } else {
        //Check if player is in pod and have no ships in hangar, in which case they get a rookie ship for free
        // on live, SCC sends mail about the loss of the players ship, and offers a new, fully-fitted ship as replacement.  we dont....yet
        //  NOTE:   this also creates rookie ship for new char
        if (m_ship->typeID() == itemTypeCapsule) {
            if (sConfig.server.NoobShipCheck) {
                Inventory* inv = m_system->GetStationFromInventory(m_locationID)->GetMyInventory();
                if (!inv->HasShip())
                    SpawnNewRookieShip();
            } else
                SpawnNewRookieShip();
        }
    }

    //create corp and ally chat channels (if not already created)
    m_services.lsc_service->CharacterLogin(this);

    // load char LPs
    //m_lpMap

    //johnsus - characterOnline mod
    ServiceDB::SetCharacterOnlineStatus(m_char->itemID(), true);
    ServiceDB::SetAccountOnlineStatus(GetUserID(), true);
    sItemFactory.UnsetUsingClient();
    m_char->SetLoginTime();
    UpdateSkillTraining();

    SetClientTimer(ClientState::csLogin, (IsSolarSystem(m_locationID) ? ClientTimers::LoginTimer *2 : ClientTimers::LoginTimer));
    return (m_loaded = true);
}

void Client::ProcessClient() {
    double profileStartTime = GetTimeUSeconds();

    // wtf is this for?
    if (m_pingTimer.Check()) {
        _SendPingRequest();  //10m
        m_char->SetLogonMinutes();
    }

    if (m_timeEndTrain > 0)
        if (m_timeEndTrain < GetFileTimeNow())
            m_char->UpdateSkillQueue();

    if (m_sessionTimer.Check(false)) {
        _log(CLIENT__TIMER, "Client::ProcessClient():  SetSessionChange to false for %s(%u)", m_char->itemName().c_str(), m_char->itemID());
        m_sessionTimer.Disable();
        SetSessionChange();
    }

    /* Check Character Save Timer Expiry:  (not currently used  -allan 17May16)
    if (m_char->CheckSaveTimer()) {
        _log(CLIENT__TIMER, "Client::ProcessClient():  SaveTimer for %s(%u)", m_char->itemName().c_str(), m_char->itemID());
        m_char->SaveCharacter();
        m_ship->SaveShip();
    }
    */

    if (IsStation(m_locationID)) {
        if (m_stateTimer.Enabled())
            if (m_stateTimer.Check(false)) {
                m_stateTimer.Disable();
                switch (m_clientState) {
                    case ClientState::csLogin: {
                        _log(CLIENT__TIMER, "Client::ProcessClient()::IsDocked()::CheckState():  case: csLogin");
                        m_login = false;
                    } break;
                    case ClientState::csIdle: {
                        _log(CLIENT__TIMER, "Client::ProcessClient()::IsDocked()::CheckState():  case: csIdle");
                    } break;
                    case ClientState::csLogout: {
                        _log(CLIENT__TIMER, "Client::ProcessClient()::IsDocked()::CheckState():  case: csLogout");
                    } break;
                    case ClientState::csKilled: {
                        _log(CLIENT__TIMER, "Client::ProcessClient()::CheckState():  case: csKilled");
                    } break;
                }
                m_clientState = ClientState::csIdle;
                _log(AUTOPILOT__TRACE, "ProcessClient() - Docked - m_clientState set to Idle");
            }
        if (sConfig.debug.UseProfiling)
            sProfile.AddTime(_clientProfile, GetTimeUSeconds() - profileStartTime);
        return;
    }

    if (pShipSE == nullptr) {
        sLog.Error("Client","%s: InSpace with no shipSE.", m_char->itemName().c_str());
        return;
    }

    if (m_invul)
        if (m_invulTimer.Check(false)) {
            _log(CLIENT__TIMER, "Client::ProcessClient():  SetInvul to false for %s(%u)", m_char->itemName().c_str(), m_char->itemID());
            m_invulTimer.Disable();
            SetInvul(false);
            SetUndock(false);
        }

    if (m_scanTimer.Enabled())
        if (m_scanTimer.Check(false)) {
            _log(CLIENT__TIMER, "Client::ProcessClient():  Scan Timer hit for %s(%u).", m_char->itemName().c_str(), m_char->itemID());
            m_scanTimer.Disable();
            m_scan->ProcessScan(m_scanProbe);
        }

    if (m_jumpTimer.Enabled())
        if (m_jumpTimer.Check(false)) {
            _log(CLIENT__TIMER, "Client::ProcessClient():  Jump Timer hit for %s(%u).", m_char->itemName().c_str(), m_char->itemID());
            m_jumpTimer.Disable();
            SetBallPark();
            pShipSE->DestinyMgr()->SendGateActivity(m_toGate);
            m_toGate = 0;
            SetJumpTimers();
            SetClientTimer(ClientState::csJump, ClientTimers::JumpInvul);  // infant AP system needs this to stay as "Jumping" for now...timer is 5s
        }

    if (pShipSE->DestinyMgr()->IsCloaked())
        if (m_cloakTimer.Check(false)) {
            _log(CLIENT__TIMER, "Client::ProcessClient():  SetCloak to false for %s(%u)", m_char->itemName().c_str(), m_char->itemID());
            m_cloakTimer.Disable();
            pShipSE->DestinyMgr()->UnCloak();
            m_clientState = ClientState::csIdle;
            _log(AUTOPILOT__TRACE, "ProcessClient() - Cloaked - m_clientState set to Idle");
        }

    if (m_stateTimer.Enabled())
        if (m_stateTimer.Check(false)) {
            _log(CLIENT__TIMER, "Client::ProcessClient() - timer check: timenow %u", m_stateTimer.GetCurrentTime());
            m_stateTimer.Disable();
            switch (m_clientState) {
                case ClientState::csDock: {
                    _log(CLIENT__TIMER, "Client::ProcessClient()::CheckState():  case: csDock");
                    DockToStation();
                } break;
                case ClientState::csUndock: {
                    _log(CLIENT__TIMER, "Client::ProcessClient()::CheckState():  case: csUndock");
                    SetBallPark();
                } break;
                case ClientState::csKilled: {
                    _log(CLIENT__TIMER, "Client::ProcessClient()::CheckState():  case: csKilled");
                    // live does NOT resend destiny state when killed.  see csBoard notes.
                    SendSessionChange();
                    m_clientState = ClientState::csIdle;
                } break;
                case ClientState::csBoard: {
                    _log(CLIENT__TIMER, "Client::ProcessClient()::CheckState():  case: csBoard");
                    // calling OnSessionChanged() in client with shipid in change will update ego with new ship.
                    // NOTE: all items must be in same bubble.
                    SendSessionChange();
                    m_clientState = ClientState::csIdle;
                } break;
                case ClientState::csLogin: {
                    _log(CLIENT__TIMER, "Client::ProcessClient()::CheckState():  case: csLogin");
                    SetBallPark();
                } break;
                case ClientState::csJump: {
                    _log(CLIENT__TIMER, "Client::ProcessClient()::CheckState():  case: csJump");
                    ExecuteJump();
                } break;
                case ClientState::csIdle: {
                    _log(CLIENT__TIMER, "Client::ProcessClient()::CheckState():  case: csIdle");
                } break;
                case ClientState::csLogout: {
                    _log(CLIENT__TIMER, "Client::ProcessClient()::CheckState():  case: csLogout");
                } break;
                default: {
                    sLog.Error("Client","%s: State timer expired with invalid state: %s.", m_char->itemName().c_str(), GetStateName(m_clientState).c_str());
                    //SendErrorMsg("Server Error - Move not initalized properly.  You may need to relog.  Ref: ServerError 10928");
                } break;
            }
        }

    // only set for location change
    if (m_fleetTimer.Enabled())
        if (m_fleetTimer.Check(false)) {
            m_fleetTimer.Disable();
            BoostData data;
            data.armored = 0;
            data.info = 0;
            data.leader = 0;
            data.mining = 0;
            data.siege = 0;
            data.skirmish = 0;
            if (IsSquad(m_squad)) {
                SquadData sData;
                sFltSvc.GetSquadData(m_squad, sData);
                if ((sData.leader != nullptr) and (sData.booster != nullptr))
                    if ((sData.leader->IsInSpace()) and (sData.booster->IsInSpace()))
                        data = sData.boost;
            } else if (IsWing(m_wing)) {
                WingData wData;
                sFltSvc.GetWingData(m_wing, wData);
                if ((wData.leader != nullptr) and (wData.booster != nullptr))
                    if ((wData.leader->IsInSpace()) and (wData.booster->IsInSpace()))
                        data = wData.boost;
            } else if (IsFleet(m_fleet)) {
                FleetData fData;
                sFltSvc.GetFleetData(m_fleet, fData);
                if ((fData.leader != nullptr) and (fData.booster != nullptr))
                    if ((fData.leader->IsInSpace()) and (fData.booster->IsInSpace())) {
                        data.leader    = m_char->GetSkillLevel(skillLeadership);
                        data.armored   = fData.booster->GetChar()->GetSkillLevel(skillArmoredWarfare);
                        data.info      = fData.booster->GetChar()->GetSkillLevel(skillInformationWarfare);
                        data.mining    = fData.booster->GetChar()->GetSkillLevel(skillMiningForeman);
                        data.siege     = fData.booster->GetChar()->GetSkillLevel(skillSiegeWarfare);
                        data.skirmish  = fData.booster->GetChar()->GetSkillLevel(skillSkirmishWarfare);
                    }
            }
            pShipSE->ApplyBoost(data);
        }

    if (sConfig.debug.UseProfiling)
        sProfile.AddTime(_clientProfile, GetTimeUSeconds() - profileStartTime);
}

void Client::SetAutoPilot(bool set/*false*/)
{
    // itemID=10644  flag=*module*  not published - not in client data
    m_autoPilot = set;
    _log(AUTOPILOT__MESSAGE, "%s called SetAutoPilot to %s", GetName(), (set ? "true" : "false"));
}


void Client::SetDestiny(const GPoint& pt, bool count/*false*/) {
    if ((pShipSE == nullptr) or (pShipSE->DestinyMgr() == nullptr)) {
        CreateShipSE();
        pShipSE->SetPilot(this);
        m_system->AddEntity(pShipSE);
        UpdateNewShip();
    }

    if (IsSolarSystem(m_locationID)) {
        m_bubbleWait = false;        // allow client processing of subsquent destiny msgs
        if (pt.isZero()) {
            if (pShipSE->GetPosition().isZero())
                pShipSE->DestinyMgr()->SetPosition(m_SGP.GetRandPointOnMoon(m_system->GetID()), false);
            else
                pShipSE->DestinyMgr()->SetPosition(pShipSE->GetPosition(), false);
        } else
            pShipSE->DestinyMgr()->SetPosition(pt, false);
        if (count and !m_login)
            pShipSE->ResetShipSystemMgr(m_system);
        m_setStateSent = false;
        if (m_beyonce)
            return;
        if ((!m_login) and (!IsJump()) and (!m_undock))
            SetBallPark();
    } else
        _log(CLIENT__ERROR, "%s(%u) - Calling SetDestiny() when not in space.", GetName(), m_char->itemID());
}

void Client::SetBallPark() {
    m_login = false;
    m_bubbleWait = false;   // allow client processing of subsquent destiny msgs
    if (pShipSE->SysBubble() == nullptr)
        m_system->AddEntity(pShipSE);
    if (m_clientState == ClientState::csUndock) {
        m_ship->Undock();
        pShipSE->DestinyMgr()->Undock(m_movePoint);
    }
    if (m_clientState == ClientState::csJump)
        pShipSE->DestinyMgr()->Jump();
    if (!m_setStateSent)
        pShipSE->DestinyMgr()->SendSetState();
    m_clientState = ClientState::csIdle;
    _log(AUTOPILOT__TRACE, "SetBallPark() - m_clientState set to Idle");
}

void Client::WarpIn() {
	sLog.Blue("Client::WarpIn()", "%s(%u) called WarpIn().  Finish code here.", GetName(), m_char->itemID());
    char ci[1];
    snprintf(ci, sizeof(ci), "");
    m_ship->SetCustomInfo(ci);
    if (!InPod())
        m_ship->SetFlag(flagAutoFit);
    m_invulTimer.Start(ClientTimers::WarpInInvul);
    return;
    // We are just logging in, so we need to warp to our last position from our WarpOut spot.
    /** @todo  when implemented, make sure we move the ship item, if needed....check this  */
    GPoint warpToPoint(m_ship->position());
    GPoint warpFromPoint(m_ship->position());
    warpFromPoint.MakeRandomPointOnSphere(0.5*ONE_AU_IN_METERS);
    pShipSE->DestinyMgr()->SetPosition(warpFromPoint);
    pShipSE->DestinyMgr()->WarpTo(warpToPoint);        // Warp ship from the random login point to the position saved on last disconnect
}

void Client::WarpOut() {
	sLog.Blue("Client::WarpOut()", "Client Destructor for %s(%u) called WarpOut().  Finish code here.", GetName(), m_char->itemID());
    char ci[35];
    snprintf(ci, sizeof(ci), "Logout: %s", GetName());
    m_ship->SetCustomInfo(ci);
    if (!InPod())
        m_ship->SetFlag(flagShipOffline);
    pShipSE->SetPosition(m_ship->position());
    DestroyShipSE();
    return;
    m_invulTimer.Start(ClientTimers::WarpOutInvul);
    // We are logging out, so we need to warp to a random spot 1Mm away:
    GPoint warpToPoint(m_ship->position());
    warpToPoint.MakeRandomPointOnSphere(0.5*ONE_AU_IN_METERS);
    if (sConsole.IsShutdown())      // if server is being shutdown, set ship to WarpOut point, as if they warped there.
        pShipSE->SetPosition(warpToPoint);
    else
        pShipSE->DestinyMgr()->WarpTo(warpToPoint);
}

void Client::EnterSystem(uint32 systemID)
{
    MoveToLocation(systemID, m_ship->position());
}

void Client::MoveToLocation(uint32 locationID, const GPoint& pt) {
    // process ALL location changes here.
    if (!IsStation(locationID) and !IsSolarSystem(locationID)) {
        SendErrorMsg("Move requested to unsupported location %u", locationID);
        return;
    }

    _log(AUTOPILOT__TRACE, "MoveToLocation() - m_autoPilot = %s", (m_autoPilot ? "true" : "false"));

    if (!m_login and (m_locationID == locationID)) {
        _log(PLAYER__WARNING, "MoveToLocation() - m_locationID == location");
        if (IsStation(locationID))
            return;
        // This is a simple movement
        SetDestiny(pt);
        return;
    }

    bool count = m_login;
    m_locationID = locationID;
    // get data for new system.  this checks for stationID sent as locationID, so is safe here.
    sDataMgr.GetSystemInfo(locationID, m_SystemData);
    uint32 stationID = 0;
    if (IsStation(locationID))
        stationID = locationID;

    m_bubbleWait = false;           // allow client processing of subsquent destiny msgs

    // location changed...verify current system and set session data for current system.
    if (IsJump() or ((m_system != nullptr) and (m_system->GetID() != m_SystemData.systemID))) {
        //we have different m_system
        _log(PLAYER__WARNING, "MoveToLocation() - m_system = %p, m_system->GetID(%u) != locationID(%u)", m_system, m_system->GetID(), m_locationID);
        // remove from 'current' system before resetting system vars
        if (pShipSE != nullptr) {
            pShipSE->DestinyMgr()->Halt();
            m_system->RemoveEntity(pShipSE);
        }
        MapDB::AddPilotToDynamicData(m_SystemData.systemID);
        m_system->RemoveClient(this, false, (count = true));
        m_system = nullptr;
    }

    if (m_system == nullptr) {
        _log(PLAYER__WARNING, "MoveToLocation() - m_system == NULL, m_locationID = %u", m_locationID);
        // find our new system's manager
        sItemFactory.SetUsingClient(this);
        m_system = sEntityList.FindOrBootSystem(m_SystemData.systemID);
        sItemFactory.UnsetUsingClient();
        if (m_system == nullptr) {
            sLog.Error("Client", "Failed to boot system %u for char %s (%u)", m_SystemData.systemID, m_char->itemName().c_str(), m_char->itemID());
            SendErrorMsg("Unable to boot system.  Relog and try again.");
            return;
        }

        m_beyonce = false;
        m_setStateSent = false;

        MapDB::AddPilotToDynamicData(m_SystemData.systemID, true, IsStation(locationID), count);
        // register ourself with new system manager.
        m_system->AddClient(this, IsStation(locationID), count);
    }

    m_char->SetLocation(stationID, m_SystemData.systemID, m_SystemData.constellationID, m_SystemData.regionID);   // stationID MUST be 0 when InSpace.

    char ci[25];
    if (stationID > 0) {
        _log(PLAYER__WARNING, "MoveToLocation() - Character %s (%u) Docked in %u.", m_char->itemName().c_str(), m_char->itemID(), m_locationID);
        stDataMgr.GetStationData(locationID, m_StationData);
        snprintf(ci, sizeof(ci), "Docked:%u", locationID);
        StationItemRef sRef = sEntityList.GetStationByID(locationID);
        sRef->LoadStationOffice(GetCorporationID());
        sRef->AddGuest(this);
        m_char->Move(locationID, flagAutoFit, true);
        m_ship->Move(locationID, flagHangar, true);
        m_ship->Relocate(pt);
        m_ship->Dock();

        //m_bubbleWait = true;     // deny client processing of subsquent destiny msgs

        if (IsFleet(m_fleet)) {
            m_fleetTimer.Disable();
            if (IsFleetBooster()) {
                std::list<int32> wing, squad;
                wing.clear();
                squad.clear();
                if (IsSquad(m_squad))
                    squad.emplace(squad.end(), m_squad);
                else if (IsWing(m_wing))
                    wing.emplace(wing.end(), m_wing);
                sFltSvc.UpdateBoost(m_fleet, IsFleetBoss(), wing, squad);
            }
        }

        if (!IsHangarLoaded(locationID))
            LoadStationHangar(locationID);
        OnCharNowInStation();
        DestroyShipSE();
    } else {
        _log(PLAYER__WARNING, "MoveToLocation() - Character %s(%u) InSpace in %u. (setState %s, beyonce %s)", \
                m_char->itemName().c_str(), m_char->itemID(), m_locationID, m_setStateSent ? "true" : "false", m_beyonce ? "true" : "false");
        snprintf(ci, sizeof(ci), "InSpace:%u", locationID);

        if (IsFleet(m_fleet)) {
            m_fleetTimer.Start(ClientTimers::FleetTimer);
            if (IsFleetBooster()) {
                std::list<int32> wing, squad;
                wing.clear();
                squad.clear();
                if (IsSquad(m_squad))
                    squad.emplace(squad.end(), m_squad);
                else if (IsWing(m_wing))
                    wing.emplace(wing.end(), m_wing);
                sFltSvc.UpdateBoost(m_fleet, IsFleetBoss(), wing, squad);
            }
        }

        if (InPod()) {
            m_ship->Move(locationID, flagCapsule, true);
        } else {
            m_pod->Move(locationID, flagCapsule, false);
            m_ship->Move(locationID, flagAutoFit, true);
        }

        if (m_char->flag() != flagPilot)
            m_char->Move(m_shipId, flagPilot, true);

        SetDestiny(pt, !m_undock);

        if (IsJump() and !m_autoPilot)
            pShipSE->DestinyMgr()->Stop();
    }

    m_ship->SetCustomInfo(ci);
    m_ship->SaveShip();

    _UpdateSession();
    SendSessionChange();
}

void Client::MoveToPosition(const GPoint &pt) {
    if ((pShipSE == nullptr) or (pShipSE->DestinyMgr() == nullptr)) {
        CreateShipSE();
        pShipSE->SetPilot(this);
        m_system->AddEntity(pShipSE);
        UpdateNewShip();
    }
    pShipSE->DestinyMgr()->SetPosition(pt, true);
    if (m_undock)
        return;
    if (pShipSE->DestinyMgr()->IsMoving())
        pShipSE->DestinyMgr()->Halt();
}

void Client::UndockFromStation() {
    if (m_TS != nullptr) {
        TradeService* mts = (TradeService*)(m_services.LookupService("trademgr"));
        mts->CancelTrade(this);
    }

    //ShipIllegalTypeUndock

    m_invul = true;
    m_undock = true;
    //set position and direction of docking ramp for later use
    m_dockPoint = m_StationData.dockPosition;
    m_movePoint = m_StationData.dockOrientation;
    m_ship->SetUndocking(true);

    /** @todo  this needs a bit of work to match live....
     *  Undock Request -> GetCriminalTimeStamps -> Undock -> OnItemsChanged (Undocking:xxxxxxxx) -> OnCharNoLongerInStation ->
     *  GetAllInfo -> GetNPCStandings -> GetFormations -> AddBalls2 (slim, not ball, wait=true)
     * -> GotoDirection(etc, etc) -> SetState (dmg, ego, ball, slim)
     *  ***** 9sec from hitting undock to space view on live. *****
     */
    sEntityList.GetStationByID(m_StationData.stationID)->RemoveGuest(this);
    OnCharNoLongerInStation();
    SetClientTimer(ClientState::csUndock, ClientTimers::UndockTimer +800);
    MoveToLocation(m_SystemData.systemID, m_StationData.dockPosition);
    m_invulTimer.Start(ClientTimers::UndockInvul);
    SetSessionTimer();
}

void Client::CreateShipSE() {
    FactionData data;
        data.allianceID = GetAllianceID();
        data.corporationID = GetCorporationID();
        data.factionID = GetWarFactionID();
        data.ownerID = GetCharacterID();
    pShipSE = new Ship(m_ship, *(m_system->GetServiceMgr()), m_system, data);
    _log(PLAYER__MESSAGE, "CreateShipSE() - pShipSE %p created for %s(%u)", pShipSE, m_char->itemName().c_str(), m_char->itemID());
}

void Client::DestroyShipSE() {
    if (pShipSE != nullptr) {
        _log(PLAYER__MESSAGE, "DestroyShipSE() - pShipSE %p (%s) destroyed for %s(%u)", pShipSE, m_ship->itemName().c_str(), m_char->itemName().c_str(), m_char->itemID());
        m_system->RemoveEntity(pShipSE);
        SafeDelete(pShipSE);
    } else
        _log(PLAYER__WARNING, "DestroyShipSE() - pShipSE = null for %s(%u)", m_char->itemName().c_str(), m_char->itemID());
    pShipSE = nullptr;
}

void Client::UpdateNewShip()
{
    pShipSE->SetPilot(this);
    pShipSE->DestinyMgr()->SetShipCapabilities(m_ship);
    pShipSE->DestinyMgr()->UpdateNewShip(m_ship);
    pShipSE->DestinyMgr()->SendBallInteractive(m_ship, true);

    char ci[25];
    snprintf(ci, sizeof(ci), "InSpace:%u", m_locationID);
    m_ship->SetCustomInfo(ci);
    SetSessionTimer();
}

void Client::SetPodItem() {
    if (!IsPlayerItem(m_char->capsuleID()))
        CreateNewPod();
    else
        m_pod = sItemFactory.GetShip(m_char->capsuleID());
    if (m_pod.get() == nullptr)
        CreateNewPod();
}

void Client::DockToStation() {
    // ap cleared on client side when docking.
    m_autoPilot = false;
    m_setStateSent = false;
    m_clientState = ClientState::csIdle;
    _log(AUTOPILOT__TRACE, "DockToStation()() - m_clientState set to Idle");
    pShipSE->DestinyMgr()->Dock();
    MoveToLocation(m_dockStationID, NULL_ORIGIN);

    m_bubbleWait = true;     // deny client processing of subsquent destiny msgs

    //Check if player is in pod and have no ships in hangar, in which case they get a rookie ship for free
    //  on live, SCC sends mail about the loss of the players ship, and offers a new, fully-fitted ship as replacement.  we dont....yet
    if (m_ship->typeID() == itemTypeCapsule) {
        if (sConfig.server.NoobShipCheck) {
            Inventory* inv = m_system->GetStationFromInventory(m_locationID)->GetMyInventory();
            if (!inv->HasShip())
                SpawnNewRookieShip();
        } else
            SpawnNewRookieShip();
    }

    SetSessionTimer();
}

void Client::CheckShipRef(ShipItemRef newShipRef)
{
    if (newShipRef.get() == nullptr) {
        _log(PLAYER__ERROR, "BoardShip() - %s: newShipRef == NULL.", m_char->itemName().c_str());
        throw PyException(MakeCustomError("Could not find ship's ItemRef.  Cannot Board.   Ref: ServerError 12321."));
    } else if (!newShipRef->singleton()) {
        _log(PLAYER__MESSAGE, "%s tried to board ship %u, which is not assembled.", m_char->itemName().c_str(), newShipRef->itemID());
        throw PyException(MakeCustomError("You cannot board a ship which is not assembled!"));
    } else if ((m_ship == newShipRef) and !m_login) {
        // if char is loging in, this will hit.  unknown about any other time.
        _log(PLAYER__MESSAGE, "%s tried to board active ship %u.", m_char->itemName().c_str(), newShipRef->itemID());
        throw PyException(MakeCustomError("You are already aboard this ship."));
    }
}

void Client::BoardShip(ShipItemRef newShipRef) {
    CheckShipRef(newShipRef);

    /* check for and delete pod entity if boarding new ship */
    if (m_login) {
        _log(PLAYER__MESSAGE, "%s boarding active ship %u on login.", m_char->itemName().c_str(), newShipRef->itemID());
    } else if (m_ship->typeID() == itemTypeCapsule) {
        m_ship->Relocate(NULL_ORIGIN);
        m_ship->Move(m_system->GetID(), flagCapsule, true);
    } else {
        m_ship->GetModuleManager()->CharacterLeavingShip();
        m_ship->SaveShip();
    }

    SetShip(newShipRef, true);

    char ci[25];
    snprintf(ci, sizeof(ci), "Docked:%u", m_locationID);
    m_ship->SetCustomInfo(ci);
    SetSessionTimer();
}

void Client::Board(ShipItemRef newShipRef)
{
    CheckShipRef(newShipRef);

    if (m_ship->typeID() == itemTypeCapsule) {
        m_ship->Relocate(NULL_ORIGIN);
        m_ship->Move(m_system->GetID(), flagCapsule, true);
    } else {
        m_ship->GetModuleManager()->CharacterLeavingShip();
        m_ship->SaveShip();
    }

    SetShip(newShipRef, true);
    UpdateNewShip();

    SetClientTimer(ClientState::csBoard, ClientTimers::BoardTimer);
}

void Client::Eject()
{
    if (m_pod.get() == nullptr)
        CreateNewPod();

    if (m_pod.get() == nullptr) {
        _log(SHIP__ERROR, "Handle_Eject() - Failed to get podItem for %s.", GetName());
        if (m_canThrow)
            throw PyException(MakeCustomError("Something bad happened as you prepared to eject.  Ref: ServerError 25107."));
        else
            return;
    }

    pShipSE->Abandon();
    m_ship->SetFlag(flagShipOffline);

    char ci[40];
    snprintf(ci, sizeof(ci), "Abandoned by %s", GetName());
    m_ship->SetCustomInfo(ci);

    pShipSE->DestinyMgr()->UpdateOldShip(m_ship);
    pShipSE->DestinyMgr()->SendBallInteractive(m_ship);
    // send OnItemsChanged notifications here for abandonded ship

    uint32 oldShipID = m_shipId;
    GPoint capsulePosition(pShipSE->GetPosition());
    capsulePosition.MakeRandomPointOnSphere(m_ship->radius() + m_pod->radius() + MakeRandomFloat(300, 400));
    m_pod->Relocate(capsulePosition);
    SetShip(m_pod);

    m_ship->ChangeOwner(m_char->itemID());
    m_ship->Move(m_locationID, flagCapsule, true);
    Ship* oldShipSE = pShipSE;  // copy pShipSE

    //  set shipSE to null.  this allows sending AddBalls when pod added to system
    pShipSE = nullptr;
    FactionData data;
        data.allianceID = GetAllianceID();
        data.corporationID = GetCorporationID();
        data.factionID = GetWarFactionID();
        data.ownerID = GetCharacterID();
    Ship* newShipSE = new Ship(m_ship, *(m_system->GetServiceMgr()), m_system, data);

    if (newShipSE == nullptr) {
        _log(PLAYER__ERROR, "%s Eject() - pShipSE = NULL for shipID %u.", m_char->itemName().c_str(), m_pod->itemID());
        // we should probably send char to their clone station if this happens....
        MoveToLocation(GetCloneStationID(), NULL_ORIGIN);
        throw PyException(MakeCustomError("There was a problem creating your pod in space.<br>You have been transfered to your home station.<br>Ref: ServerError 15107."));
    }

    newShipSE->SetLauncherID(oldShipSE->GetID());
    m_system->AddEntity(newShipSE);

    pShipSE = newShipSE;
    m_char->Move(m_shipId, flagPilot, true);

    UpdateNewShip();

    SetClientTimer(ClientState::csBoard, ClientTimers::BoardTimer);
}

void Client::ResetAfterPopped(GPoint& position)
{
    m_autoPilot = false;
    m_bubbleWait = false;    // allow client processing of subsquent destiny msgs

    pShipSE->DestinyMgr()->Stop();

    if (m_pod.get() == nullptr)
        CreateNewPod();

    Ship* oldShipSE = pShipSE;  // copy pShipSE

    m_pod->Relocate(position);
    SetShip(m_pod);

    //  set shipSE to null.  this allows sending AddBalls when pod added to system
    pShipSE = nullptr;
    FactionData data;
        data.allianceID = GetAllianceID();
        data.corporationID = GetCorporationID();
        data.factionID = GetWarFactionID();
        data.ownerID = GetCharacterID();
    Ship* newShipSE = new Ship(m_ship, *(m_system->GetServiceMgr()), m_system, data);

    if (newShipSE == nullptr) {
        _log(PLAYER__ERROR, "%s ResetAfterPopped() - pShipSE = NULL for shipID %u.", m_char->itemName().c_str(), m_pod->itemID());
        // we should probably send char to their clone station if this happens....
        MoveToLocation(GetCloneStationID(), NULL_ORIGIN);
        SpawnNewRookieShip();
        throw PyException(MakeCustomError("There was a problem creating your pod in space.<br>You have been transfered to your home station.<br>Ref: ServerError 15107."));
    }

    newShipSE->SetLauncherID(oldShipSE->GetID());
    m_system->AddEntity(newShipSE);

    pShipSE = newShipSE;

    UpdateNewShip();

    // update shipID in session
    UpdateSessionInt("shipid", m_shipId);
    SetClientTimer(ClientState::csKilled, ClientTimers::KilledTimer);
    //SendSessionChange();
}

void Client::ResetAfterPodded() {
    /** @todo
     * destroy all implants
     * check skillpoints vs. clone grade and adjust accordingly.
     * reset skill effects if clone != current SP and skills lost
     */

    m_autoPilot = false;

    MoveToLocation(GetCloneStationID(), NULL_ORIGIN);
    SpawnNewRookieShip();
    CreateNewPod();
    SetShip(m_system->GetShipFromInventory(m_char->capsuleID()), true);

    m_ship->SaveShip();
    m_char->ResetClone();
    m_char->SaveCharacter();
    //update session with new values
    _UpdateSession();
    SendSessionChange();
}

void Client::SetShip(ShipItemRef shipRef, bool setPlayer/*false*/) {
    pShipSE = nullptr;
    m_ship->SetPlayer(nullptr); // nullify ship pilot pointer (just in case)
    // make sure shipRef is singleton
    if (!shipRef->singleton())
        shipRef->ChangeSingleton(true, true);
    
    m_ship = shipRef;
    m_shipId = shipRef->itemID();
    m_char->SetActiveShip(m_shipId);
    if (IsSolarSystem(m_locationID)) {
        m_char->Move(m_shipId, flagPilot, true);
        UpdateSessionInt("shipid", m_shipId);   // update shipID in session
    }
    if (setPlayer)
        m_ship->SetPlayer(this);
}

void Client::PickAlternateShip() {
    if (m_char.get() != nullptr)
        m_shipId = m_char->PickAlternateShip(m_locationID);
}

void Client::CreateNewPod() {
    std::string pod_name = m_char->itemName() + "'s Capsule";
    ItemData podItem( itemTypeCapsule, m_char->itemID(), m_locationID, flagCapsule, pod_name.c_str() );
    m_pod = sItemFactory.SpawnShip( podItem );
    m_char->SetActivePod(m_pod->itemID());  // is this used?
}

ShipItemRef Client::SpawnNewRookieShip() {
    /** @todo  create/send mail from scc about lost ship */
    //create rookie ship of appropriate type
    uint32 shipID = amarrRookie, gunID = amarrWeapon;
    EVERace race = m_char->race();
    if (race == raceCaldari) {
        gunID = caldariWeapon;
        shipID = caldariRookie;
    } else if (race == raceGallente) {
        gunID = gallenteWeapon;
        shipID = gallenteRookie;
    } else if (race == raceMinmatar) {
        gunID = minmatarWeapon;
        shipID = minmatarRookie;
    }

    std::string name =  m_char->itemName() + "'s Noob Ship";
    //create data for new rookie ship
    ItemData sData(shipID, m_char->itemID(), 0, flagAutoFit, name.c_str());
    ItemData mData(itemCivilianMiner, m_char->itemID(), 0, flagAutoFit);
    ItemData wData(gunID, m_char->itemID(), 0, flagAutoFit);
    ItemData cData(itemTypeTrit, m_char->itemID(), 0, flagAutoFit, 100);
    //spawn rookie ship
    ShipItemRef sRef = sItemFactory.SpawnShip(sData);
    InventoryItemRef mRef = sItemFactory.SpawnItem(mData);
    InventoryItemRef wRef = sItemFactory.SpawnItem(wData);
    InventoryItemRef cRef = sItemFactory.SpawnItem(cData);
    // create and fit noob items in ship
    if (sRef.get() != nullptr) {
        sRef->ChangeSingleton(true);
        sRef->Move(m_char->stationID(), flagHangar);
    }
    if (mRef.get() != nullptr)
        mRef->Move(sRef->itemID(), flagHiSlot0);
    if (wRef.get() != nullptr)
        wRef->Move(sRef->itemID(), flagHiSlot1);
    if (cRef.get() != nullptr)
        cRef->Move(sRef->itemID(), flagCargoHold);
    // in case caller needs ref to new noob ship
    return sRef;
}

bool Client::IsJetcanAvalible() {
    if (m_jetcanTimer.Enabled())
        return (m_jetcanTimer.Check(false));
    else
        return true;
}

PyRep *Client::GetAggressors() const {
    PyDict* dict(nullptr);
    /*
     *            for aggressorID, aggressor in aggressors.iteritems():
     *                for aggresseeID, lastAggression in aggressor.iteritems():
     *                    lastAggression = int(lastAggression / SEC) * SEC
     *                    when = lastAggression + const.aggressionTime * MIN
     *                    self.clearAggressions[aggressorID, aggresseeID, solarsystemID] = when
     *
     *                    aggressionTime = 15
     */
    /*  items are set here as
     *               [PyInt 90971469]                    <- entity (aggressorID)
     *               [PyDict 1 kvp]                      <- dictionary
     *                  [PyInt 1000127]                     <- entity (aggresseeID)
     *                  [PyIntegerVar 129550906897224125]   <- beginning timestamp (lastAggression)
     */

    return dict;
}

void Client::StargateJump(uint32 fromGate, uint32 toGate) {
    if ((m_clientState != csIdle) or m_stateTimer.Enabled()) {
        sLog.Error("Client","%s: StargateJump called when a move is already pending. Ignoring.", m_char->itemName().c_str());
        return;
    }

    m_toGate = toGate;
    StaticData toData;
    if (!sDataMgr.GetStaticInfo(m_toGate, toData)) {
        sLog.Error("Client","%s: Failed to query information for stargate %u", m_char->itemName().c_str(), toGate);
        return;
    }
    m_moveSystemID = toData.systemID;
    m_movePoint = toData.position;
    m_movePoint.MakeRandomPointOnSphereLayer(7500, 9500);   // Make Jump-In point a random spot on ~10km radius sphere about the stargate

    char ci[25];
    snprintf(ci, sizeof(ci), "Jumping:%u", toGate);
    m_ship->SetCustomInfo(ci);

    // add jump to mapDynamicData for showing in StarMap (F10)    -allan 06Mar14
    // this is the code for removing pilot from previous system
    StaticData fromData;
    if (!sDataMgr.GetStaticInfo(fromGate, fromData)) {
            sLog.Error("Client","%s: Failed to query information for stargate %u", m_char->itemName().c_str(), fromGate);
            return;
        }
    //add jump in previous system
    MapDB::AddJumpToDynamicData(fromData.systemID);
    //add jump in this system
    MapDB::AddJumpToDynamicData(toData.systemID);
    // used for showing Visited Systems in StarMap(F10)  -allan 30Jan14
    m_char->VisitSystem(toData.systemID);

    // call Stop() per packet sniff - shuts off AP.  Halt() does also.  try not calling any movement updates
    //pShipSE->DestinyMgr()->Halt();  // Stop() disables ap.  try Halt() to reset ship movement to null
    pShipSE->DestinyMgr()->SendJumpOut(fromGate);
    //  show gate animation in from gate.   -working -allan 15Nov15
    pShipSE->DestinyMgr()->SendGateActivity(fromGate);

    //delay the move 5sec so they can see the JumpOut animation
    SetClientTimer(ClientState::csJump, ClientTimers::JumpingTimer);
}

void Client::ExecuteJump() {
    if (m_movePoint == NULL_ORIGIN) {   // this is part of infant AP hack
        m_clientState = ClientState::csIdle;
        _log(AUTOPILOT__TRACE, "ExecuteJump() - m_clientState set to Idle");
        return;
    }

    m_ship->Jump();
    m_invul = true;
    m_beyonce = m_setStateSent = false;

    MoveToLocation(m_moveSystemID, m_movePoint);

    m_jumpTimer.Start(ClientTimers::JumpTimer);

    m_movePoint = NULL_ORIGIN;
    /*
(67187, `{[location]system.name} Traffic Control: your jump-in clearance has expired.`)
(67191, `{[location]system.name} Traffic Control: you have been cleared for jump-in within {[timeinterval]expiration.writtenForm, from=second, to=second}.`)
(67210, `{[location]system.name} Traffic Control: you are at position {[numeric]pos} in queue for jump-in.`)
(258673, `{system} is currently loading. Bizzarre, seeing that you're there already... Please try again in a minute or two.`)
(258674, `The stargates in {system} are currently experiencing minor technical difficulties. Please try again in a moment.`)
(258675, `Jump Prohibited`)
(258676, `Officials have closed the stargates in {system} due to heavy congestion. Travelers are advised to enjoy the local scenery.`)
(258677, `{system} Traffic Control is currently experiencing heavy load and is unable to process your request. Please try again in a moment.`)
(258678, `{system} Traffic Control is currently offline and unable to process your jump request. Please try again in a moment.`)
(258679, `{system} Traffic Control is currently experiencing heavy load and is unable to process your request. You are #{[numeric]position} in queue for jump-in.`)
(258680, `Jump Prohibited`)
(258681, `Officials have closed the stargates in {system} due to heavy congestion. Travelers are advised to seek alternate routes.`)
(258683, `Your character is located within {system}, which is currently loading. Please try again in a moment.`)
(258685, `Your character is located within {system}, which is currently experiencing heavy load. Please try again in a moment.`)
(258687, `Your character is located within {system}, which has reached maximum capacity. You are #{[numeric]position} in queue for entrance. Please try again in a moment.`)
(258689, `Your character is located within {system}, which is currently stuck. Please select a different character or try later.`)
(258691, `The character is located within {system}, which is currently loading. Please try again in a moment.`)
(258693, `The character is located within {system}, which is currently experiencing heavy load. Please try again in a moment.`)
(258695, `The character is located within {system}, which is currently stuck. Please try again later.`)
(258696, `Undock Delayed`)
(258697, `{system} Traffic Control is currently offline and unable to process your undocking request. Please try again in a moment.`)
(258699, `The item <b>{example}</b> was not found in the station <b>{[location]station.name}</b>. Please make sure all items are in the correct hangar before finalizing this contract.`)
(258700, `Undock Delayed`)
(258701, `{system} Traffic Control is currently experiencing heavy load and was unable to process your request. Please try again in a moment.`)
(258702, `Undock Prohibited`)
(258703, `Officials have closed the undocking ramps in {system} due to heavy congestion. Please try again later.`)
(258704, `You cannot leave {system} yet because of instability in the space-time continuum. Please try again in a moment.`)

*/
}

void Client::SetJumpTimers() {
    m_cloakTimer.Start(ClientTimers::JumpCloak);
    m_invulTimer.Start(ClientTimers::JumpInvul);
}

void Client::SetClientTimer(ClientState state, uint32 time)
{
    _log(CLIENT__TIMER, "%s: ClientTimer set from %s to %s at %ums.  timenow %u", m_char->itemName().c_str(), GetStateName(m_clientState).c_str(), GetStateName(state).c_str(), time, m_stateTimer.GetCurrentTime());
    m_clientState = state;
    m_stateTimer.Start(time);
}

std::string Client::GetStateName(ClientState state)
{
    switch (state) {
        case csIdle:    return "Idle";
        case csJump:    return "Jump";
        case csDock:    return "Dock";
        case csUndock:  return "Undock";
        case csKilled:  return "Killed";
        case csLogout:  return "Logout";
        case csBoard:   return "Board";
        case csLogin:   return "Login";
    }
}

void Client::UpdateSkillTraining() {
    if (m_char.get() != nullptr)
        m_timeEndTrain = m_char->GetEndOfTraining();
    else
        m_timeEndTrain = 0;
}

void Client::AddStationHangar(uint32 stationID) {
    m_hangarLoaded.insert(std::make_pair(stationID, true));
}

void Client::RemoveStationHangar(uint32 hangarID) {
    m_hangarLoaded.erase(hangarID);
}

bool Client::IsHangarLoaded(uint32 hangarID) {
    std::map<uint32, bool>::const_iterator itr = m_hangarLoaded.find(hangarID);
    if (itr != m_hangarLoaded.end())
        return itr->second;
    return false;
}

void Client::LoadStationHangar(uint32 stationID) {
    _log(PLAYER__INFO, "Client::LoadStationHangar() is loading personal hangar for %s(%u) in stationID %u",  m_char->itemName().c_str(), m_char->itemID(), stationID);
    sItemFactory.SetUsingClient(this);
    m_system->GetStationFromInventory(stationID)->GetMyInventory()->LoadContents();
    sItemFactory.UnsetUsingClient();
}

void Client::MoveItem(uint32 itemID, uint32 location, EVEItemFlags flag)
{
    sItemFactory.SetUsingClient(this);
    InventoryItemRef item = sItemFactory.GetItem(itemID);
    if (item.get() == nullptr) {
        _log(INV__ERROR, "Client::MoveItem() - %s Unable to load item %u", m_char->itemName().c_str(), itemID);
        return;
    }

    item->Move(location, flag, true);

    if (IsPlayerItem(location)) {
        if (IsModuleSlot(item->flag())) {
            m_ship->UpdateModules(item->flag());
        } else if (IsCargoHoldFlag(item->flag())) {
            // do nothing here.  this is to avoid throwing error msg below
        } else {
            _log(INV__WARNING, "Client::MoveItem() - %s Unhandled PlayerItem %u", m_char->itemName().c_str(), itemID);
        }
    } else {
        _log(INV__WARNING, "Client::MoveItem() - %s Unhandled NonPlayerItem %u", m_char->itemName().c_str(), itemID);
    }
    sItemFactory.UnsetUsingClient();
}

PyRep *Client::GetInfoWindowDataForChar(Client *pClient) {
    CharacterDB c_db;
    return c_db.GetInfoWindowDataForChar(pClient->GetCharacterID());
}

bool Client::LaunchDrone(InventoryItemRef drone) {
    if (!sConfig.npc.EnableDrones) {
        SendNotifyMsg("Drones are disabled.");
        return false;
    }
    if (!IsSolarSystem(m_locationID)) {
        sLog.White("Client::LaunchDrone()","%s: Trying to launch drone when not in space!",  m_char->itemName().c_str());
        return false;
    }

    sLog.White("Client::LaunchDrone()","%s: Launching drone %u",  m_char->itemName().c_str(), drone->itemID());

    drone->Move(m_locationID, flagAutoFit, true);

    GPoint position(pShipSE->GetPosition());
    position.MakeRandomPointOnSphere(500.0);

    //now we create an entity to represent it.
    FactionData data;
        data.allianceID = m_char->allianceID();
        data.corporationID = m_char->corporationID();
        data.factionID = m_char->warFactionID();
        data.ownerID = m_char->itemID();

    Drone* pDrone = new Drone(drone, m_services, m_system, position, data);
    // add drone entity to system, set speed, begin orbit around launching ship
    m_system->AddEntity(pDrone);
     OnDroneStateChange du;
        du.droneID = drone->itemID();
        du.ownerID = m_char->itemID();
        du.droneTypeID = drone->typeID();
        du.controllerID = m_shipId;
        du.controllerOwnerID = m_char->itemID();
        du.activityState = droneIdle;
        du.targetID = 0;
    PyTuple* up = du.Encode();
    pShipSE->DestinyMgr()->SendSingleDestinyUpdate(&up);
    PyDecRef(up);

    pDrone->DestinyMgr()->Orbit(pShipSE, 800);  //FIXME
    pDrone->DestinyMgr()->SetMaxVelocity(500);      //FIXME
    pDrone->DestinyMgr()->SetSpeedFraction(0.5f);   //FIXME

    return true;
}

uint32 Client::GetLoyaltyPoints(uint32 corpID) {
    std::map<uint32, uint32>::iterator itr = m_lpMap.find(corpID);
    if (itr != m_lpMap.end())
        return itr->second;
    return 0;
}

void Client::RemoveMissionItem(uint16 typeID, uint32 qty)
{
    uint16 count = qty;
    InventoryItemRef iRef = InventoryItemRef(nullptr);
    if (IsStation(m_locationID)) {
        iRef = sItemFactory.GetStation(m_locationID)->GetMyInventory()->GetByTypeFlag(typeID, flagHangar);
        if (iRef.get() != nullptr) {
            if (count < iRef->quantity()) {
                iRef->AlterQuantity(count, true);
                count = 0;
            } else {
                count -= iRef->quantity();
                iRef->Delete();
            }
        }
    }

    if (count) {
        iRef = GetShip()->GetMyInventory()->GetByTypeFlag(typeID, flagCargoHold);
        if (iRef.get() != nullptr){
            if (count < iRef->quantity()) {
                iRef->AlterQuantity(count, true);
                count = 0;
            } else {
                count -= iRef->quantity();
                iRef->Delete();
            }
        }
    }
    if (count)
        ;  // make error here for not enough?
}

bool Client::ContainsTypeQty(uint16 typeID, uint32 qty) const
{
    uint16 count = 0;
    InventoryItemRef iRef = InventoryItemRef(nullptr);
    // this is for missions....we will have to determine if we have the TOTAL qty desired, in both cargo and hangar
    if (IsStation(m_locationID)) {
        iRef = sItemFactory.GetStation(m_locationID)->GetMyInventory()->GetByTypeFlag(typeID, flagHangar);
        if (iRef.get() != nullptr)
            count = iRef->quantity();
    }

    iRef = GetShip()->GetMyInventory()->GetByTypeFlag(typeID, flagCargoHold);
    if (iRef.get() != nullptr)
        count += iRef->quantity();

    if (count >= qty)
        return true;
    return false;
}

bool Client::IsMissionComplete(MissionOffer& data)
{
    // dont know all the stipulations of "completion" yet, but skeleton code for starters...
    switch (data.typeID) {
        case Mission::Type::Tutorial: {
        } break;
        case Mission::Type::Encounter: {
        } break;
        case Mission::Type::Courier: {
            if (m_locationID == data.destinationID)
                if (ContainsTypeQty(data.courierTypeID, data.courierAmount))
                    return true;
        } break;
        case Mission::Type::Trade: {
        } break;
        case Mission::Type::Mining: {
        } break;
        case Mission::Type::Research: {
        } break;
        case Mission::Type::Data: {
        } break;
        case Mission::Type::Storyline: {
        } break;
        case Mission::Type::Cosmos: {
        } break;
        case Mission::Type::Arc: {
        } break;
        case Mission::Type::Anomic: {
        } break;
    }

    return false;
}


/************************************************************************/
/* character notification messages wrapper                              */
/************************************************************************/
void Client::OnCharNoLongerInStation() {
    NotifyOnCharNoLongerInStation n;
        n.charID = m_char->itemID();
        n.corpID = GetCorporationID();
        n.allianceID = GetAllianceID();
        n.factionID = GetWarFactionID();
    PyTuple* tmp = n.Encode();
    std::vector<Client*> clients;
    clients.clear();
    sEntityList.FindClientByStationID(m_locationID, clients);
    for (auto cur : clients) {
        PySafeIncRef(tmp);
        cur->SendNotification("OnCharNoLongerInStation", "stationid", &tmp); //consumed
    }
    PySafeDecRef(tmp);
}

void Client::OnCharNowInStation() {
    NotifyOnCharNowInStation n;
        n.charID = m_char->itemID();
        n.corpID = GetCorporationID();
        n.allianceID = GetAllianceID();
        n.warFactionID = GetWarFactionID();
    PyTuple* tmp = n.Encode();
    std::vector<Client*> clients;
    clients.clear();
    sEntityList.FindClientByStationID(m_locationID, clients);
    for (auto cur : clients) {
        PySafeIncRef(tmp);
        cur->SendNotification("OnCharNowInStation", "stationid", &tmp);
    }
    PySafeDecRef(tmp);
}

/**********************************************************************
 *  session shit and other non-player-related things
 * ****************************************************************/
void Client::UpdateSessionInt(const char *sessionType, int value)
{
    mSession.SetInt(sessionType, value);
}

void Client::UpdateCorpSession(CorpData& data)
{
    // session.Set* methods only updates on change
    mSession.SetInt("corpid", data.corporationID);
    mSession.SetInt("baseID", data.baseID);
    mSession.SetInt("hqID", data.corpHQ);
    mSession.SetInt("allianceid", data.allianceID);
    mSession.SetInt("warfactionid", data.warFactionID);
    mSession.SetInt("corpAccountKey", data.corpAccountKey);
    mSession.SetLong("corprole", data.corpRole);
    mSession.SetLong("rolesAtAll", data.rolesAtAll);
    mSession.SetLong("rolesAtBase", data.rolesAtBase);
    mSession.SetLong("rolesAtHQ", data.rolesAtHQ);
    mSession.SetLong("rolesAtOther", data.rolesAtOther);
    SendSessionChange();
}

void Client::UpdateFleetSession(CharFleetData& fleet)
{
    m_fleet = fleet.fleetID;
    m_wing = fleet.wingID;
    m_squad = fleet.squadID;

    mSession.SetInt("fleetjob", fleet.job);
    mSession.SetInt("fleetrole", fleet.role);
    mSession.SetInt("fleetbooster", fleet.booster);
    mSession.SetInt("fleetid", m_fleet);
    mSession.SetInt("wingid", m_wing);
    mSession.SetInt("squadid", m_squad);
    SendSessionChange();
}

void Client::_UpdateSession()
{
    if (m_char.get() == nullptr)
        return;
    uint32 stationID = m_char->stationID();
    uint32 solarsystemID = m_char->solarSystemID();
    if (stationID) {
        mSession.Clear("solarsystemid");    //must be 0 in station
        mSession.Clear("shipid");    //must be 0 in station

        mSession.SetInt("stationid", stationID);
        mSession.SetInt("stationid2", stationID);   // client uses this for continer location checks
        //mSession.SetInt("worldspaceid", stationID);
        mSession.SetInt("locationid", stationID);
    } else {
        mSession.Clear("stationid");
        mSession.Clear("stationid2");
        mSession.Clear("worldspaceid");
        mSession.SetInt("solarsystemid", solarsystemID); //  used to tell client they are in space
        mSession.SetInt("locationid", solarsystemID);
        mSession.SetInt("shipid", m_shipId);
    }

    mSession.SetInt("charid", m_char->itemID());
    mSession.SetString("charname", m_char->itemName().c_str());
    mSession.SetInt("corpid", m_char->corporationID());
    // solarsystemid2 is used by client to determine current system.  NOTE:  *MUST* be set to current system.
    mSession.SetInt("solarsystemid2", solarsystemID);
    mSession.SetInt("constellationid", m_char->constellationID());
    mSession.SetInt("regionid", m_char->regionID());
}

void Client::InitSession(uint32 characterID)
{
    if (characterID < 0) {
        sLog.Error("Client::InitSession()", "characterID == 0");
        return;
    }

    std::map<std::string, int64> characterDataMap;
    ((CharUnboundMgrService *)(m_services.LookupService("charUnboundMgr")))->GetCharacterData(characterID, characterDataMap);
    if (characterDataMap.size() < 1) {
        sLog.Error("Client::InitSession()", "characterDataMap.size() returned zero.");
        return;
    }

    int32 stationID         = (int32)(characterDataMap["stationID"]);
    int32 solarSystemID     = (int32)(characterDataMap["solarSystemID"]);
    m_shipId                = (int32)(characterDataMap["shipID"]);

    mSession.SetInt("genderID",         (int32)(characterDataMap["gender"]));
    mSession.SetInt("bloodlineID",      (int32)(characterDataMap["bloodlineID"]));
    mSession.SetInt("raceID",           (int32)(characterDataMap["raceID"]));
    mSession.SetInt("charid",           characterID);
    mSession.SetInt("corpid",           (int32)(characterDataMap["corporationID"]));

    mSession.SetInt("cloneStationID",   (int32)(characterDataMap["cloneStationID"]));
    mSession.SetInt("solarsystemid2",   solarSystemID);
    mSession.SetInt("constellationid",  (int32)(characterDataMap["constellationID"]));
    mSession.SetInt("regionid",         (int32)(characterDataMap["regionID"]));

    mSession.SetInt("hqID",             (int32)(characterDataMap["corporationHQ"]));
    mSession.SetInt("baseID",           characterDataMap["baseID"]);
    mSession.SetInt("corpAccountKey",   characterDataMap["corpAccountKey"]);
    mSession.SetLong("corprole",        characterDataMap["corpRole"]);
    mSession.SetLong("rolesAtAll",      characterDataMap["rolesAtAll"]);
    mSession.SetLong("rolesAtBase",     characterDataMap["rolesAtBase"]);
    mSession.SetLong("rolesAtHQ",       characterDataMap["rolesAtHQ"]);
    mSession.SetLong("rolesAtOther",    characterDataMap["rolesAtOther"]);
    mSession.SetInt("allianceid",       characterDataMap["allianceID"]);
    mSession.SetInt("warfactionid",     characterDataMap["warFactionID"]);

    /*  solarSystemID != 0  -character in space
     *   also used as current system in following menus:
     *  JumpPortalBridgeMenu, GetHybridBeaconJumpMenu, GetHybridBridgeMenu
     */
    if (stationID) {
        m_locationID = stationID;
        mSession.SetInt("stationid", stationID);
        mSession.SetInt("stationid2", stationID);
        mSession.SetInt("locationid", stationID);
        //mSession.SetInt("worldspaceid", stationID);
    } else {
        m_locationID = solarSystemID;
        mSession.SetInt("shipid", m_shipId);
        mSession.SetInt("solarsystemid", solarSystemID);
        mSession.SetInt("locationid", solarSystemID);
    }

    sDataMgr.GetSystemInfo(m_locationID, m_SystemData);
}

void Client::SendSessionChange()
{
    if (!mSession.isDirty())
        return;
    if ((GetCharacterID() > 0) and (m_locationID == 0)) {
        // this should never happen now.  -allan 3Aug16
        codelog(CLIENT__ERROR, "Session::LocationID == 0 for %s(%u)", GetCharacterName().c_str(), GetCharacterID());
        m_locationID = GetSystemID();
        if (IsDocked())
            m_locationID = GetStationID();
        /* a session.locationid change will trigger a ballpark update (add/delete bp) */
        UpdateSessionInt("locationid", m_locationID);
    }

    SessionChangeNotification scn;
    scn.changes = new PyDict();

    mSession.EncodeChanges(scn.changes);
    if (scn.changes->empty())
        return;

    if (is_log_enabled(CLIENT__SESSION)) {
        _log(CLIENT__SESSION, "Session updated.  Sending session change");
        scn.changes->Dump(CLIENT__SESSION, "   Changes: ");
    }

    //scn.sessionID = GetSessionID();       /* this isnt right....client creates sessionID.  i dont know how to retrieve it yet. */
    scn.clueless = 0;
    scn.nodesOfInterest.push_back(-1);  /* this means 'all nodes' */
    scn.nodesOfInterest.push_back(m_services.GetNodeID());  /* add current node to list */
    /* if other nodes are created, add those that are 'live' for this client here */
    //scn.nodesOfInterest.push_back(m_services.GetNodeID());

    //build the packet:
    PyPacket* packet = new PyPacket();
    packet->type_string = "macho.SessionChangeNotification";
    packet->type = SESSIONCHANGENOTIFICATION;

    packet->source.type = PyAddress::Node;
    packet->source.objectID = m_services.GetNodeID();
    packet->source.callID = 0;

    packet->dest.type = PyAddress::Client;
    packet->dest.objectID = GetClientID();
    packet->dest.callID = 0;

    packet->userid = GetUserID();

    packet->payload = scn.Encode();

    packet->named_payload = nullptr;
    //p->named_payload = new PyDict();
    //p->named_payload->SetItemString("channel", new PyString("sessionchange"));

    if (is_log_enabled(CLIENT__OUT_ALL)) {
        _log(CLIENT__OUT_ALL, "Sending Session packet:");
        PyLogDumpVisitor dumper(CLIENT__OUT_ALL, CLIENT__OUT_ALL);
        packet->Dump(CLIENT__OUT_ALL, dumper);
    }

    FastQueuePacket(packet);
}

void Client::FlushQueue() {
    if ((!m_destinyUpdateQueue->empty())
        or (!m_destinyEventQueue->empty()))
        _SendQueuedUpdates();
}

void Client::QueueDestinyEvent(PyTuple** event) {
    if ((event == nullptr) or ((*event) == nullptr))
        return;
    m_destinyEventQueue->AddItem(*event);
    //PyDecRef(*event);
}

void Client::QueueDestinyUpdate(PyTuple **update, bool DoPackage /*false*/, bool IsSetState /*false*/) {
    if ((update == nullptr) or ((*update) == nullptr))
        return;
    if (IsStation(m_locationID))
        return;
    DoDestinyAction act;
        act.stamp = sEntityList.GetStamp();
    if (DoPackage/* or m_packaged*/) {
        if (IsSetState) {
            // send the setstate buffer alone
            act.update = *update;
        } else {
            // this will package all current updates (and those comming in before next flush) into
            //   a single PackagedAction packet, which is then inserted into the DoDestinyAction packet.
            PyList* paList = new PyList();
                paList->AddItem(*update);
            if (!m_destinyUpdateQueue->empty())
                paList->AddItem(m_destinyUpdateQueue);
            PackagedAction pa;
                pa.substream = new PySubStream(paList);
            act.update = pa.Encode();
            m_packaged = false;
        }
        DoDestinyUpdateMain_2 dum;
            dum.updates = new PyList();
            dum.updates->AddItem(act.Encode());
            dum.waitForBubble = m_bubbleWait;
        PyTuple* t = dum.Encode();
        if (is_log_enabled(CLIENT__QUEUE_DUMP))
            t->Dump(CLIENT__QUEUE_DUMP, "");
        SendNotification("DoDestinyUpdate", "clientID", &t, false);
        PyDecRef(t);
    } else {
        act.update = *update;
        m_packaged = true;
        m_destinyUpdateQueue->AddItem(act.Encode());
    }
    //PyDecRef(*update);
}

void Client::_SendQueuedUpdates() {
    if (!m_destinyUpdateQueue->empty()) {
        if (m_destinyEventQueue->empty()) {
            DoDestinyUpdateMain_2 dum;
                dum.updates = m_destinyUpdateQueue;
                dum.waitForBubble = m_bubbleWait;
            PyTuple* t = dum.Encode();
            if (is_log_enabled(CLIENT__QUEUE_DUMP))
                t->Dump(CLIENT__QUEUE_DUMP, "");
            SendNotification("DoDestinyUpdate", "clientID", &t);
            PyDecRef(t);
        } else {
            DoDestinyUpdateMain dum;
                dum.updates = m_destinyUpdateQueue;
                dum.events = m_destinyEventQueue;
                dum.waitForBubble = m_bubbleWait;
            PyTuple* t = dum.Encode();
            if (is_log_enabled(CLIENT__QUEUE_DUMP))
                t->Dump(CLIENT__QUEUE_DUMP, "");
            SendNotification("DoDestinyUpdate", "clientID", &t);
            PyDecRef(t);
        }
    } else if (!m_destinyEventQueue->empty()) {
        Notify_OnMultiEvent nom;
            nom.events = m_destinyEventQueue;
        PyTuple* t = nom.Encode();
        if (is_log_enabled(CLIENT__QUEUE_DUMP))
            t->Dump(CLIENT__QUEUE_DUMP, "");
        SendNotification("OnMultiEvent", "charid", &t);
        PyDecRef(t);
    } //else nothing to be sent ...

    // clear the queues now, after the packets have been sent
    m_destinyUpdateQueue->clear();
    m_destinyEventQueue->clear();
    m_packaged = false;
}

void Client::SendNotification(const char *notifyType, const char *idType, PyTuple *payload, bool seq /*true*/) {
    //build a little notification out of it.
    EVENotificationStream notify;
    notify.notifyType = notifyType;
    notify.remoteObject = 1;
    notify.args = payload;

    PyAddress dest;
    dest.type = PyAddress::Broadcast;
    dest.service = notifyType;
    dest.bcast_idtype = idType;
    /*
    if (dest.bcast_idtype.compare("clientID") == 0)
        dest.objectID = GetClientID();*/

    //now send it to the client
    SendNotification(dest, notify, seq);
    //PyDecRef(*payload);
}

void Client::SendNotification(const char *notifyType, const char *idType, PyTuple **payload, bool seq /*true*/) {
    if ((*payload) == nullptr)
        return;
    //build a little notification out of it.
    EVENotificationStream notify;
        notify.notifyType = notifyType;
        notify.remoteObject = 1;
        notify.args = (*payload);

    PyAddress dest;
        dest.type = PyAddress::Broadcast;
        dest.service = notifyType;
        dest.bcast_idtype = idType;
        dest.objectID = GetClientID();

    //now send it to the client
    SendNotification(dest, notify, seq);
    //PyDecRef(*payload);
}

void Client::SendNotification(const PyAddress &dest, EVENotificationStream &noti, bool seq/*true*/) {
    //build the packet:
    PyPacket *packet = new PyPacket();
    packet->type_string = "macho.Notification";
    packet->type = NOTIFICATION;

    packet->source.type = PyAddress::Node;
    packet->source.objectID = m_services.GetNodeID();

    packet->dest = dest;

    packet->userid = GetUserID();

    packet->payload = noti.Encode();

    if (seq) {
        packet->named_payload = new PyDict();
        packet->named_payload->SetItemString("sn", new PyInt(++m_nextNotifySequence));
    }

    _log(CLIENT__NOTIFY_REP, "Sending notify of type %s with ID type %s to %s", dest.service.c_str(), dest.bcast_idtype.c_str(), GetName());
    if (is_log_enabled(CLIENT__NOTIFY_DUMP)) {
        PyLogDumpVisitor dumper(CLIENT__NOTIFY_DUMP, CLIENT__NOTIFY_REP, "", true, true);
        packet->Dump(CLIENT__NOTIFY_DUMP, dumper);
    }

    FastQueuePacket(packet);
}

/************************************************************************/
/* EVEAdministration Interface                                          */
/************************************************************************/
void Client::DisconnectClient()
{
    //initiate closing the client TCP Connection
    CloseClientConnection();
}
void Client::BanClient()
{
    //send message to client
    SendNotifyMsg("You have been banned from this server and will be disconnected shortly.  You will no longer be able to log in");

    //ban the client
    ServiceDB::SetAccountBanStatus(GetUserID(), true);
}

/************************************************************************/
/* EVEClientSession interface                                           */
/************************************************************************/
void Client::_GetVersion(VersionExchangeServer& version)
{
    version.birthday = EVEBirthday;
    version.macho_version = MachoNetVersion;
    version.user_count = sEntityList.GetClientCount();
    version.version_number = EVEVersionNumber;
    version.build_version = EVEBuildVersion;
    version.project_version = EVEProjectVersion;
}

uint32 Client::_GetUserCount()
{
    return sEntityList.GetClientCount();
}

bool Client::_VerifyVersion(VersionExchangeClient& version)
{
    version.Dump(NET__PRES_REP, "    ");
    if (version.birthday != EVEBirthday)
        sLog.Error("Client","%s: Client's birthday does not match ours!", GetAddress().c_str());
    if (version.macho_version != MachoNetVersion)
        sLog.Error("Client","%s: Client's macho_version not match ours!", GetAddress().c_str());
    if (version.version_number != EVEVersionNumber)
        sLog.Error("Client","%s: Client's version_number not match ours!", GetAddress().c_str());
    if (version.build_version != EVEBuildVersion)
        sLog.Error("Client","%s: Client's build_version not match ours!", GetAddress().c_str());
    if (version.project_version != EVEProjectVersion)
        sLog.Error("Client","%s: Client's project_version not match ours!", GetAddress().c_str());
    return true;
}

bool Client::_VerifyCrypto(CryptoRequestPacket& cr)
{
    if (cr.keyVersion != "placebo")
    {
        //I'm sure cr.keyVersion can specify either CryptoAPI or PyCrypto, but its all binary so im not sure how.
        CryptoAPIRequestParams car;
        if (!car.Decode(cr.keyParams))
        {
            sLog.Error("Client","%s: Received invalid CryptoAPI request!", GetAddress().c_str());
        }
        else
        {
            sLog.Error("Client","%s: Unhandled CryptoAPI request: hashmethod=%s sessionkeylength=%d provider=%s sessionkeymethod=%s", GetAddress().c_str(), car.hashmethod.c_str(), car.sessionkeylength, car.provider.c_str(), car.sessionkeymethod.c_str());
            sLog.Error("Client","%s: You must change your client to use Placebo crypto in common.ini to talk to this server!\n", GetAddress().c_str());
        }

        return false;
    }
    else
    {
        sLog.Debug("Client","%s: Received Placebo crypto request, accepting.", GetAddress().c_str());

        //send out accept response
        PyRep* rsp = new PyString("OK CC");
        mNet->QueueRep(rsp);
        PyDecRef(rsp);

        return true;
    }
}

bool Client::_VerifyLogin(CryptoChallengePacket& ccp)
{
    /* send passwordVersion required: 1=plain, 2=hashed */
    // TODO  look into using plain pass to facilitate forgotten-password retrieval from web.
    mNet->QueueRep(new PyInt(2));

    std::string fail_msg = "LoginAuthFailed";
    AccountData account_info {};
    if (!ServiceDB::GetAccountInformation(ccp.user_name.c_str(), ccp.user_password_hash.c_str(), account_info))
        return _LoginFail(fail_msg);

    /* check wether the account has been banned and if so send the semi correct message */
    if (account_info.banned) {
        fail_msg = "Your account is banned. Contact the Game Master for further support";
        return _LoginFail(fail_msg);
    }

    if (account_info.online) {
        fail_msg = "This account is being used right now. Try logging in again later.";
        return _LoginFail(fail_msg);
    }

    std::string account_hash = "";
    /* if we have stored a password we need to create a hash from the username and pass and remove the pass */
    if (account_info.password.empty())
        account_hash = account_info.hash;
    else {
        /* here we generate the password hash ourselves */
        std::string password_hash;
        if (!PasswordModule::GeneratePassHash(ccp.user_name, account_info.password, password_hash)) {
            sLog.Error("Client", "unable to generate password hash, sending LoginAuthFailed");
            return _LoginFail(fail_msg);
        }

        if (!ServiceDB::UpdateAccountHash(ccp.user_name.c_str(), password_hash)) {
            sLog.Error("Client", "unable to update account hash, sending LoginAuthFailed");
            return _LoginFail(fail_msg);
        }

        account_hash = password_hash;
    }

    /* here we check if the user successfully entered his password or if he failed  */
    if (account_hash != ccp.user_password_hash) {
        fail_msg = "Your password was entered incorrectly.";
        return _LoginFail(fail_msg);
    }

    // check this character/account for newbie status and revoke as needed before account update.
    /* update account information, increase login count, last login timestamp */
    ServiceDB::IncrementLoginCount(account_info.id);

    /* marshaled Python string "None" */
    static const uint8 handshakeFunc[] = { 0x74, 0x04, 0x00, 0x00, 0x00, 0x4E, 0x6F, 0x6E, 0x65 };

    /* send our handshake */
    CryptoServerHandshake server_shake;
    //server_shake.context = ??
    server_shake.serverChallenge = "";
    server_shake.func_marshaled_code = new PyBuffer(handshakeFunc, handshakeFunc + sizeof(handshakeFunc));
    server_shake.verification = new PyBool(false);
    server_shake.cluster_usercount = _GetUserCount();
    server_shake.proxy_nodeid = 0xFFAA;
    server_shake.user_logonqueueposition = _GetQueuePosition();
    // binascii.crc_hqx of marshaled single-element tuple containing 64 zero-bytes string
    server_shake.challenge_responsehash = "55087";

    // the image server used by the client to download images
    server_shake.imageserverurl = sImageServer.url();

    server_shake.macho_version = MachoNetVersion;
    server_shake.boot_version = EVEVersionNumber;
    server_shake.boot_build = EVEBuildVersion;
    server_shake.boot_codename = EVEProjectCodename;
    server_shake.boot_region = EVEProjectRegion;
    mNet->QueueRep(server_shake.Encode());

    // Setup session, but don't send the change yet.
    mSession.SetString("address", EVEClientSession::GetAddress().c_str());
    mSession.SetString("languageID", ccp.user_languageid.c_str());

    //user type 30 is normal user, type 23 is a trial account user.
    mSession.SetInt("userType", userTypeMammon);
    mSession.SetInt("userid", account_info.id);
    mSession.SetLong("clientID", 0/*10000000000L * account_info.clientID + 888444*/);   /* this should be sent in rsp packet for "clientid".  not sure how yet.   */
    mSession.SetLong("role", account_info.role);
    //mSession.SetLong("sessionID", mSession.CreateSessionID());

    sLog.Green("  Client::Login()","Account \"%s\" logging in from IP %s", account_info.name.c_str() ,EVEClientSession::GetAddress().c_str());

    return true;
}

bool Client::_LoginFail(std::string fail_msg)
{
    GPSTransportClosed* except = new GPSTransportClosed(fail_msg);
    mNet->QueueRep(except);
    PyDecRef(except);
    return false;
}

bool Client::_VerifyFuncResult(CryptoHandshakeResult& result)
{
    _log(NET__PRES_DEBUG, "%s: Handshake result received.", GetAddress().c_str());

    //send this before session change
    CryptoHandshakeAck ack;
        ack.jit = GetLanguageID();
        ack.userid = GetUserID();   //5654387 accountID?
        ack.maxSessionTime = new PyNone();
        ack.userType = 1;
        ack.role = Acct::Role::PLAYER | Acct::Role::NEWBIE; /*  live returns these */
        ack.address = GetAddress();
        ack.inDetention = new PyNone();
    // no client update available
        ack.client_hash = new PyNone();
        ack.user_clientid = GetClientID();  //241241000001103
        ack.live_updates = sLiveUpdateDB.GetUpdates();
        /* the client creates and sends sessionID in the initial packet.  unknown how to get it yet. */
        //ack.sessionID = GetSessionID();   //398773966249980114
    PyRep* r = ack.Encode();
    if (is_log_enabled(CLIENT__CALL_DUMP))
        r->Dump(CLIENT__CALL_DUMP, "    ");
    mNet->QueueRep(r, false);
    PyDecRef(r);

    // Send out the session change
    SendSessionChange();

    return true;
}

void Client::_SendCallReturn(const PyAddress& source, int64 callID, int64 clientID, PyRep** return_value, const char* channel)
{
    //build the packet:
    PyPacket* packet = new PyPacket();
    packet->type_string = "macho.CallRsp";
    packet->type = CALL_RSP;

    packet->source = source;     /* address should be 'ship' for warpto response */

    packet->dest.type = PyAddress::Client;
    packet->dest.objectID = clientID;
    packet->dest.callID = callID;

    packet->userid = GetUserID();

    packet->payload = new PyTuple(1);
    packet->payload->SetItem(0, new PySubStream(*return_value));
    return_value = nullptr;   //consumed

    if (channel != nullptr) {
        packet->named_payload = new PyDict();
        packet->named_payload->SetItemString("channel", new PyString(channel));
    }

    if (packet == nullptr)
        return;     // in the case of empty return packets (segfault)

    FastQueuePacket(packet);
}

void Client::_SendException(const PyAddress& source, int64 callID, MACHONETMSG_TYPE msgType, MACHONETERR_TYPE errCode, PyRep** payload)
{
    //build the packet:
    PyPacket* packet = new PyPacket();
    packet->type_string = "macho.ErrorResponse";
    packet->type = ERRORRESPONSE;

    packet->source = source;

    packet->dest.type = PyAddress::Client;
    packet->dest.objectID = GetClientID();
    packet->dest.callID = callID;

    packet->userid = GetUserID();

    ErrorResponse e;
    e.MsgType = msgType;
    e.ErrorCode = errCode;
    e.payload = *payload;   //consumed
    payload = nullptr;

    packet->payload = e.Encode();
    FastQueuePacket(packet);
}

void Client::_SendPingRequest()
{
    PyPacket *packet = new PyPacket();

    packet->type = PING_REQ;
    packet->type_string = "macho.PingReq";

    packet->source.type = PyAddress::Node;
    packet->source.objectID = m_services.GetNodeID();
    packet->source.service = "ping";
    packet->source.callID = 0;

    packet->dest.type = PyAddress::Client;
    packet->dest.objectID = GetClientID();
    packet->dest.callID = 0;

    packet->userid = GetUserID();

    packet->payload = new_tuple(new PyList()); //times
    packet->named_payload = new PyDict();

    FastQueuePacket(packet);
}

void Client::_SendPingResponse(const PyAddress& source, int64 callID)
{
    PyPacket* packet = new PyPacket();
    packet->type = PING_RSP;
    packet->type_string = "macho.PingRsp";

    packet->source = source;

    packet->dest.type = PyAddress::Client;
    packet->dest.objectID = GetClientID();
    packet->dest.callID = callID;

    packet->userid = GetUserID();

    /*  Here the hacking begins, the ping packet handles the timestamps of various packet handling steps.
     *        To really simulate/emulate that we need the various packet handlers which in fact we don't have (:P).
     *        So the next piece of code "fake's" it, with a slight delay on the received packet time.
     */
    PyList* pingList = new PyList();
    PyTuple* pingTuple;

    pingTuple = new PyTuple(3);
    pingTuple->SetItem(0, new PyLong(Win32TimeNow() - 20));        // this should be the time the packet was received (we cheat here a bit)
    pingTuple->SetItem(1, new PyLong(Win32TimeNow()));             // this is the time the packet is (handled/writen) by the (proxy/server) so we're cheating a bit again.
    pingTuple->SetItem(2, new PyString("proxy::handle_message"));
    pingList->AddItem(pingTuple);

    pingTuple = new PyTuple(3);
    pingTuple->SetItem(0, new PyLong(Win32TimeNow() - 20));
    pingTuple->SetItem(1, new PyLong(Win32TimeNow()));
    pingTuple->SetItem(2, new PyString("proxy::writing"));
    pingList->AddItem(pingTuple);

    pingTuple = new PyTuple(3);
    pingTuple->SetItem(0, new PyLong(Win32TimeNow() - 20));
    pingTuple->SetItem(1, new PyLong(Win32TimeNow()));
    pingTuple->SetItem(2, new PyString("server::handle_message"));
    pingList->AddItem(pingTuple);

    pingTuple = new PyTuple(3);
    pingTuple->SetItem(0, new PyLong(Win32TimeNow() - 20));
    pingTuple->SetItem(1, new PyLong(Win32TimeNow()));
    pingTuple->SetItem(2, new PyString("server::turnaround"));
    pingList->AddItem(pingTuple);

    pingTuple = new PyTuple(3);
    pingTuple->SetItem(0, new PyLong(Win32TimeNow() - 20));
    pingTuple->SetItem(1, new PyLong(Win32TimeNow()));
    pingTuple->SetItem(2, new PyString("proxy::handle_message"));
    pingList->AddItem(pingTuple);

    pingTuple = new PyTuple(3);
    pingTuple->SetItem(0, new PyLong(Win32TimeNow() - 20));
    pingTuple->SetItem(1, new PyLong(Win32TimeNow()));
    pingTuple->SetItem(2, new PyString("proxy::writing"));
    pingList->AddItem(pingTuple);

    // Set payload
    packet->payload = new PyTuple(1);
    packet->payload->SetItem(0, pingList);

    // Don't clone so it eats the ret object upon sending.
    FastQueuePacket(packet);
}

/************************************************************************/
/* EVEPacketDispatcher interface                                        */
/************************************************************************/
bool Client::Handle_CallReq(PyPacket* packet, PyCallStream& req)
{
    PyCallable* dest(nullptr);
    if (packet->dest.service == "") {
        //bound object
        uint32 nodeID = 0, bindID = 0;
        if (sscanf(req.remoteObjectStr.c_str(), "N=%u:%u", &nodeID, &bindID) != 2) {
            sLog.Error("Client","Failed to parse bind string '%s'.", req.remoteObjectStr.c_str());
            return false;
        }

        if (nodeID != m_services.GetNodeID()) {
            sLog.Error("Client","Unknown nodeID - received %u but expected %u.", nodeID, m_services.GetNodeID());
            return false;
        }

        dest = m_services.FindBoundObject(bindID);
        if (dest == nullptr) {
            sLog.Error("Client", "Failed to find bound object %u.", bindID);
            return false;
        }
    } else {
        //service
        dest = m_services.LookupService(packet->dest.service);
        if (dest == nullptr) {
            sLog.Error("Client","Unable to find service to handle call to: %s", packet->dest.service.c_str());
            packet->dest.Dump(CLIENT__CALL_DUMP, "    ");
            throw PyException(MakeUserError("ServiceNotFound"));
        }
    }

    //Debug code
    //if (req.method != "BeanCount")
        //_log(CLIENT__CALL_REP, "%s call made to %s",req.method.c_str(),packet->dest.service.c_str());
        //sLog.Warning("Client::BeanCount","(%s/%s) BeanCount error reporting and handling is not implemented yet.", \
                     req.method.c_str(),packet->dest.service.c_str());

    //build arguments
    PyCallArgs args(this, req.arg_tuple, req.arg_dict);

    //parts of call may be consumed here
    m_canThrow = true;      // test for throwable.  -allan 29Jul16      should we use try/catch here?
    PyResult result = dest->Call(req.method, args);
    m_canThrow = false;

    SendSessionChange();  //send out the session change before the return.
    if (is_log_enabled(CLIENT__OUT_ALL))
        if (result.ssResult != nullptr)
            result.ssResult->Dump(CLIENT__OUT_ALL, "    ");
    _SendCallReturn(packet->dest, packet->source.callID, GetClientID(), &result.ssResult);  //ssResult is consumed here

    //PySafeDecRef(result.ssResult);
    return true;
}

bool Client::Handle_Notify(PyPacket* packet)
{
    //turn this thing into a notify stream:
    ServerNotification notify;
    if (!notify.Decode(packet->payload)) {
        sLog.Error("Client","Failed to convert rep into a notify stream");
        return false;
    }

    if (notify.method == "ClientHasReleasedTheseObjects") {
        _log(SERVICE__MESSAGE, "Client Has Released These Objects:");
        ServerNotification_ReleaseObj element;

        PyList::const_iterator cur = notify.elements->begin();
        for (; cur != notify.elements->end(); ++cur) {
            if (!element.Decode(*cur)) {
                sLog.Error("Client","Notification '%s' from %s: Failed to decode element. Skipping.", notify.method.c_str(),  m_char->itemName().c_str());
                continue;
            }

            uint32 nodeID = 0, bindID = 0;
            if (sscanf(element.boundID.c_str(), "N=%u:%u", &nodeID, &bindID) != 2) {
                sLog.Error("Client","Notification '%s' from %s: Failed to parse bind string '%s'. Skipping.", \
                           notify.method.c_str(), m_char->itemName().c_str(), element.boundID.c_str());
                continue;
            }

            if (nodeID != m_services.GetNodeID()) {
                sLog.Error("Client","Notification '%s' from %s: Unknown nodeID %u received (expected %u). Skipping.", \
                           notify.method.c_str(), m_char->itemName().c_str(), nodeID, m_services.GetNodeID());
                continue;
            }

            m_services.ClearBoundObject(bindID);
        }
    } else {
        sLog.Error("Client","Unhandled notification from %s: unknown method '%s'", m_char->itemName().c_str(), notify.method.c_str());
        return false;
    }

    //SendSessionChange();  //just for good measure...
    return true;
}

//this displays a modal error dialog on the client side.
void Client::SendErrorMsg(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char* str = nullptr;
    vasprintf(&str, fmt, args);
    assert(str);
    va_end(args);

    //want to send some sort of notify with a "ServerMessage" message ID maybe?
    //else maybe a "ChatTxt"??
    Notify_OnRemoteMessage n;
    n.msgType = "CustomError";
    n.args[ "error" ] = new PyString(str);

    PyTuple* tmp = n.Encode();
    SendNotification("OnRemoteMessage", "charid", &tmp);

    SafeFree(str);
}

void Client::SendErrorMsg(const char* fmt, va_list args)
{
    char* str = nullptr;
    vasprintf(&str, fmt, args);
    assert(str);

    //want to send some sort of notify with a "ServerMessage" message ID maybe?
    //else maybe a "ChatTxt"??
    Notify_OnRemoteMessage n;
    n.msgType = "CustomError";
    n.args[ "error" ] = new PyString(str);

    PyTuple* tmp = n.Encode();
    SendNotification("OnRemoteMessage", "charid", &tmp);

    SafeFree(str);

}

//this displays a modal info dialog on the client side.
void Client::SendInfoModalMsg(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char* str = nullptr;
    vasprintf(&str, fmt, args);
    assert(str);
    va_end(args);

    //want to send some sort of notify with a "ServerMessage" message ID maybe?
    //else maybe a "ChatTxt"??
    Notify_OnRemoteMessage n;
    n.msgType = "ServerMessage";
    n.args[ "msg" ] = new PyString(str);

    PyTuple* tmp = n.Encode();
    SendNotification("OnRemoteMessage", "charid", &tmp);

    SafeFree(str);
}

//this displays a little notice (like combat messages)
void Client::SendNotifyMsg(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char* str = nullptr;
    vasprintf(&str, fmt, args);
    assert(str);
    va_end(args);

    //want to send some sort of notify with a "ServerMessage" message ID maybe?
    //else maybe a "ChatTxt"??
    Notify_OnRemoteMessage n;
    n.msgType = "CustomNotify";
    n.args[ "notify" ] = new PyString(str);

    PyTuple* tmp = n.Encode();
    SendNotification("OnRemoteMessage", "charid", &tmp);

    SafeFree(str);
}

void Client::SendNotifyMsg(const char* fmt, va_list args)
{
    char* str = nullptr;
    vasprintf(&str, fmt, args);
    assert(str);

    //want to send some sort of notify with a "ServerMessage" message ID maybe?
    //else maybe a "ChatTxt"??
    Notify_OnRemoteMessage n;
    n.msgType = "CustomNotify";
    n.args[ "notify" ] = new PyString(str);

    PyTuple* tmp = n.Encode();
    SendNotification("OnRemoteMessage", "charid", &tmp);

    SafeFree(str);
}

//there may be a less hackish way to do this.
void Client::SelfChatMessage(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char* str = nullptr;
    vasprintf(&str, fmt, args);
    assert(str);
    va_end(args);

    if (m_channels.empty()) {
        if (m_char.get() != nullptr)
            sLog.Error("Client", "%s: Tried to send self chat, but we are not joined to any channels: %s", m_char->itemName().c_str(), str);
        free(str);
        return;
    }

    if (m_char.get() != nullptr)
        sLog.White("Client","%s: Self message on all channels: %s", m_char->itemName().c_str(), str);

    //this is such a pile of crap, but im not sure whats better.
    //maybe a private message...
    std::set<LSCChannel*>::iterator cur = m_channels.begin();
    for (; cur != m_channels.end(); ++cur)
        (*cur)->SendMessage(this, str, true);

    //m_channels[

    //just send it to the first channel we are in..
    /*LSCChannel *chan = *(m_channels.begin());
     *    char self_id[24];   //such crap..
     *    snprintf(self_id, sizeof(self_id), "%u", m_char->itemID());
     *    if (chan->GetName() == self_id) {
     *        if (m_channels.size() > 1) {
     *            chan = *(++m_channels.begin());
}
}*/

    SafeFree(str);
}

void Client::ChannelJoined(LSCChannel *chan) {
    m_channels.insert(chan);
}

void Client::ChannelLeft(LSCChannel *chan) {
    m_channels.erase(chan);
}

