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

#include "Client.h"
#include "LiveUpdateDB.h"
#include "PyBoundObject.h"
#include "chat/LSCService.h"
#include "character/CharUnboundMgrService.h"
#include "corporation/CorporationDB.h"
#include "imageserver/ImageServer.h"
#include "npc/NPC.h"
#include "ship/DestinyManager.h"
#include "ship/ShipOperatorInterface.h"
#include "ship/Drone.h"
#include "system/SystemGPoint.h"
#include "system/SystemManager.h"
#include "scanning/Scan.h"
#include "station/Station.h"
#include "station/TradeService.h"

static const uint32 PING_INTERVAL_US = 60000;

Client::Client(PyServiceMgr &services, EVETCPConnection** con)
: DynamicSystemEntity(nullptr),
  EVEClientSession(con),
  m_services(services),
  m_pingTimer(PING_INTERVAL_US),
  m_system(nullptr),
  m_scan(nullptr),
  m_TS(nullptr),
  m_movePoint(0, 0, 0),
  m_moveState(msIdle),
  m_jumpTimer(2000),
  m_moveTimer(1000),
  m_scanTimer(10000),
  m_cloakTimer(10000),
  m_invulTimer(10000),
  m_killedTimer(3000),
  m_undockTimer(6000),
  m_clientTimer(1000),
  m_logoutTimer(1000),
  m_jetcanTimer(180000),
  m_sessionTimer(10000),
  m_moveSystemID(0),
  m_timeEndTrain(0),
  m_destinyEventQueue(new PyList),
  m_destinyUpdateQueue(new PyList),
  m_nextNotifySequence(1)
{
    m_pingTimer.Start();
    m_jumpTimer.Disable();
    m_moveTimer.Disable();
    m_scanTimer.Disable();
    m_cloakTimer.Disable();
    m_invulTimer.Disable();
    m_killedTimer.Disable();
    m_undockTimer.Disable();
    m_clientTimer.Disable();
    m_logoutTimer.Disable();
    m_jetcanTimer.Disable();
    m_sessionTimer.Disable();

    m_login = true;
    m_invul = true;
    m_undock = false;
    m_beyonce = false;
    m_packaged = false;
    m_bubbleWait = true;
    m_needToDock = false;
    m_setStateSent = false;
    m_sessionChangeActive = false;

    m_dockStationID = 0;

	m_systemName = "";

    // Start handshake
    Reset();
}

Client::~Client() {
    if (GetChar()) {   // we have valid character
        // remove targets before anything else, to avoid crash with npcai targeting.
        TargMgr.DoDestruction();

        // LSC logout
        m_services.lsc_service->CharacterLogout(GetCharacterID(), LSCChannel::_MakeSenderInfo(this));
        m_services.serviceDB().SetAccountOnlineStatus(GetUserID(), false);
        m_services.serviceDB().SetCharacterOnlineStatus(GetCharacterID(), false);
        m_services.ClearBoundObjects(this);

	//TODO  - for warping to random point when client logs out in space...
	//		1)  check client IsInSpace(?)
	//		2)  set timer to delay removing bubble/sysmgr/destiny...or check based on destiny->isstopped() or timer on destiny->ismoving()
	//		3)  set current position (DB::character_.logoutPosition?)  initial code in place for warp-in on login
	//		4)  generate random point to warp to ** use m_SGP.GetRandPointInSystem(systemID, distance)
	//		5)  _warp to random point, but DONT make/update new bubble with entering ship
	//		6)  remove client from sysmgr/destiny/server

        char ci[1];
        snprintf(ci, sizeof(ci), "");
        GetShip()->SetCustomInfo(ci);
        GetShip()->OfflineAll();
        SaveAllToDatabase();

        if (IsDocked()) {
            if (GetTradeSession()) {
                TradeService* mts = (TradeService*)(m_services.LookupService("trademgr"));
                mts->CancelTrade(this);
            }
            OnCharNoLongerInStation();
        } else
            WarpOut();

        m_system->RemoveClient(this, IsDocked(), true);

        m_TS = nullptr;
        m_system = nullptr;
        SafeDelete(m_scan);
        SafeDelete(m_destiny);
        PyDecRef(m_destinyEventQueue);
        PyDecRef(m_destinyUpdateQueue);
    }
}

bool Client::ProcessNet()
{
    if (GetState() != TCPConnection::STATE_CONNECTED)
        return false;

    if (m_pingTimer.Check())    //60s
        _SendPingRequest();

    PyPacket *p;
    while (p = PopPacket()) {
        _log(CLIENT__IN_ALL, "Received packet:");
        PyLogDumpVisitor dumper(CLIENT__IN_ALL, CLIENT__IN_ALL);
        p->Dump(CLIENT__IN_ALL, dumper);

        try {
            if (!DispatchPacket(p))
                sLog.Error("Client", "%s: Failed to dispatch packet of type %s (%d).", GetName(), MACHONETMSG_TYPE_NAMES[ p->type ], (int)p->type);
        }
        catch(PyException& e) {
            _SendException(p->dest, p->source.callID, p->type, WRAPPEDEXCEPTION, &e.ssException);
        }

        SafeDelete(p);
    }

    if (m_clientTimer.Check() && !IsInSpace())
        Process();

    // send queued updates
    _SendQueuedUpdates();

    return true;
}

void Client::Process() {
    double profileStartTime = 0.0;
    if (sConfig.misc.UseProfiling)
        profileStartTime = GetTimeUSeconds();

    if ((m_timeEndTrain != 0) && (m_timeEndTrain < EvilTimeNow()))
        GetChar()->UpdateSkillQueue();

    if (m_sessionTimer.Check(false)) {
        _log(CLIENT__TRACE, "Client::Process():  SetSessionChange to false for %s(%u)", GetName(), GetID());
        m_sessionTimer.Disable();
        // return to false
        SetSessionChange();
    }

    if (!IsInSpace()) {
        if (m_killedTimer.Enabled())
            m_killedTimer.Disable();
        if (sConfig.misc.UseProfiling)
            sProfile.AddTime(_clientProfile, GetTimeUSeconds() - profileStartTime);
        return;
    }

    if (m_killedTimer.Check(false)) {
        _log(CLIENT__TRACE, "Client::Process():  setKilled to false for %s(%u)", GetName(), GetID());
        m_killedTimer.Disable();
        if (!Bubble())
            UpdateLocation();
        m_destiny->SendSetState(Bubble(), GetShipID());
    }

    if (m_destiny->IsCloaked() && m_cloakTimer.Check(false)) {
        _log(CLIENT__TRACE, "Client::Process():  SetCloak to false for %s(%u)", GetName(), GetID());
        m_cloakTimer.Disable();
        m_destiny->UnCloak();
    }

    if (IsUndock() && m_undockTimer.Check(false)) {
        _log(CLIENT__TRACE, "Client::Process():  SetUndock to false for %s(%u)", GetName(), GetID());
        m_undockTimer.Disable();
        SetUndock(false);
        if (!IsSetStateSent())
            HasUndocked();
    }

    if (IsInvul() && m_invulTimer.Check(false)) {
        _log(CLIENT__TRACE, "Client::Process():  SetInvul to false for %s(%u)", GetName(), GetID());
        m_invulTimer.Disable();
        SetInvul(false);
    }

    if (m_moveTimer.Check(false)) {
        m_moveTimer.Disable();
        switch (m_moveState) {
            case msIdle: {
                sLog.Error("Client","%s: Move timer expired when no move is pending.", GetName());
            } break;
            //used to delay stargate animation
            case msJump: {
                _log(CLIENT__TRACE, "Client::Process():  case: msJump");
                _ExecuteJump();
            } break;
        }
    }

    if (IsJump() && m_jumpTimer.Check(false)) {
        m_jumpTimer.Disable();
        m_destiny->SendSetState(Bubble(), GetShipID());
        m_moveState = msIdle;
        SetStateSent(true);
        SetBubbleWait(false);
    }

    if (m_scanTimer.Check(false)) {
        m_scanTimer.Disable();
        m_scan->ScanResult();
    }

    // Check Character Save Timer Expiry:
    if (GetChar()->CheckSaveTimer()) {
        _log(CLIENT__TRACE, "Client::Process():  SaveTimer for %s(%u)", GetName(), GetID());
        GetChar()->SaveCharacter();
		GetShip()->SaveShip();
    }

    if (sConfig.misc.UseProfiling)
        sProfile.AddTime(_clientProfile, GetTimeUSeconds() - profileStartTime);

    if (!IsUndock()) {
        SystemEntity::Process();
        GetShip()->Process();
    }
}

void Client::ResetDestiny(bool count) {
    SafeDelete(m_destiny);
    SetDestiny(count);
}

void Client::SetDestiny(bool count) {
    if (!m_destiny)
        m_destiny = new DestinyManager(this, m_system);
    //need to set players position before adding to bubble (if in space)
    if (IsInSpace()) {
        if (GetShip()->position().isZero())
            MoveToPosition(m_SGP.GetRandPointOnMoon(m_system->GetID()));
        else
            m_destiny->SetPosition(GetShip()->position());
    }
    m_destiny->SetShipCapabilities(GetShip());
    m_system->AddClient(this, IsDocked(), count);
    SetStateSent(false);
}

void Client::WarpIn() {
    // We are just logging in, so we need to warp to our last position from our WarpOut spot.
    GPoint warpToPoint(GetShip()->position());
    GPoint warpFromPoint(GetShip()->position());
    warpFromPoint.MakeRandomPointOnSphere(0.5*ONE_AU_IN_METERS);
    m_destiny->SetPosition(warpFromPoint, false, true);
    m_destiny->WarpTo(warpToPoint);        // Warp ship from the random login point to the position saved on last disconnect
}

void Client::WarpOut() {
    sLog.Blue("Client::WarpOut()", "Client Destructor called WarpOut().  Finish code here.");
    return;
    // We are logging out, so we need to warp to a random spot 1Mm away:
    GPoint warpToPoint(GetShip()->position());
    warpToPoint.MakeRandomPointOnSphere(0.5*ONE_AU_IN_METERS);
    m_destiny->WarpTo(warpToPoint);        // Warp ship from the random login point to the position saved on last disconnect
}

void Client::LoginToSystem(uint32 systemID) {
    // called upon initial login
    m_systemName = m_system->GetName();
    sLog.Warning("Client::LoginToSystem()", "Client %s(%u) Login %s in %s", \
        GetName(), GetCharacterID(), (IsInSpace() ? "InSpace" : "InStation"), m_systemName.c_str());
    GetChar()->chkDynamicSystemID(systemID);

    SetDestiny(true);
    if (IsInSpace()) {
        OnCharNoLongerInStation();
        //WarpIn();
        GetChar()->AddPilotToDynamicData(systemID, false, true);
        m_invulTimer.Start(20000);
    } else {
        uint32 stationID = GetLocationID();
        GPoint dockPosition(NULL_ORIGIN);
        m_sDB.GetStationInfo(stationID, nullptr, nullptr, nullptr, nullptr, &dockPosition, nullptr);
		SetDockPoint(dockPosition);
        OnCharNowInStation();
        SetBubbleWait(true);
        m_clientTimer.Start(1000);
        GetChar()->AddPilotToDynamicData(systemID, true, true);
    }
    return;
}

bool Client::EnterSystem(uint32 systemID) {
    // called when entering new system or verifying correct/current system.  sets session data for current system

    /*  TODO this will need more work once i figure out what the damn client wants
    if (m_autoPilot) {
        sLog.Warning("Client::EnterSystem", "m_autoPilot = 1");
        if (IsStation(GetLocationID())
            m_services.serviceDB().GetStationInfo(GetLocationID(), &systemID, nullptr, nullptr, nullptr, nullptr, nullptr);
        else
            systemID = GetLocationID();
    } */

    if (m_system && (m_system->GetID() != systemID)) {
        sLog.Warning("Client::EnterSystem()", "m_system = %p, m_system->GetID(%u) != GetSystemID(%u)", m_system, m_system->GetID(), systemID);
        // remove targets before anything else, to avoid crash with npcai targeting.
        TargMgr.DoDestruction();
        //we have different m_system
        m_system->RemoveClient(this, IsDocked(), true);
        m_system = nullptr;
    }

    if (!m_system) {
        sLog.Warning("Client::EnterSystem()", "m_system == NULL, GetSystemID() = %u, mLocation = %u", systemID, GetLocationID());
        //m_system is NULL, so find our new system's manager and register ourself with it.
        m_services.item_factory->SetUsingClient(this);
        m_system = sEntityList.FindOrBootSystem(systemID);
        m_services.item_factory->UnsetUsingClient();
        if (!m_system) {
            sLog.Error("Client", "Failed to boot system %u for char %s (%u)", systemID, GetName(), GetCharacterID());
            SendErrorMsg("Unable to boot system");
            return false;
        }
        m_systemName = m_system->GetName();
        ResetDestiny(true);

        GetChar()->chkDynamicSystemID(systemID);
        GetChar()->AddPilotToDynamicData(systemID, IsInSpace());
    }

    return true;
}

void Client::UpdateLocation(uint32 locationID) {
    // called when docking or other movement in same system
    if (IsStation(locationID)) {
        sLog.Success("Client::UpdateLocation()", "Character %s (%u) Docked.", GetName(), GetCharacterID());
        m_system->bubbles.Remove(this);
        m_clientTimer.Start(1000);
        OnCharNowInStation();
    } else if (IsSolarSystem(locationID)) {
        sLog.Success("Client::UpdateLocation()", "Character %s (%u) InSpace.", GetName(), GetCharacterID());
        if (!m_system)
            EnterSystem(locationID);
        m_system->bubbles.Add(this);
	}
}

void Client::UndockFromStation(uint32 stationID, uint32 systemID, uint32 constellationID, uint32 regionID, GPoint dockPosition, GPoint direction)
{
    if (GetTradeSession()) {
        TradeService* mts = (TradeService*)(m_services.LookupService("trademgr"));
        mts->CancelTrade(this);
    }
    sLog.Log("Client::UndockFromStation()", "Character %s(%u) undocking from stationID() %u", \
               GetName(), GetCharacterID(), GetChar()->stationID());

    SetInvul(true);
    SetUndock(true);  // bool for movement and invul and SendState checks (maybe more later)
    SetStateSent(false);
    SetBubbleWait(true);

    //  OnCharNoLongerInStation -> GetCriminalTimeStamps -> Undock -> OnItemsChanged (Undocking:xxxxxxxx) ->
    //set position of docking ramp for later position checks/setting (to remove ship at origin)
    SetDockPoint(dockPosition);
    //update client and set session change
    OnCharNoLongerInStation();
    // tell ship its' undocking
    ShipRef shipRef = GetShip();
	//remove ship from station inv, add to system inv, and inform client.
    shipRef->Move(systemID, flagAutoFit);
    //set character location to current system, and save.
    GetChar()->SetLocation(0, systemID, constellationID, regionID);
    //need mlocation set to current system before updating client with new position.
    _UpdateSession(GetChar());
    DestinyUndock(direction);
    // set modules online and recharge shields and cap.
    shipRef->Undock();
    //update char session with the new values
    _SendSessionChange();

    if (HasBeyonce())
        m_undockTimer.Start(2000);
    else
        m_undockTimer.Start(6000);

    m_invulTimer.Start(10000);
    SetSessionTimer();
}

void Client::DestinyUndock(GPoint direction) {
    if (!m_destiny)
        SetDestiny();
    m_destiny->Undock(direction);
}

void Client::HasUndocked() {
    m_system->bubbles.Add(this);
    if (!Bubble()) {
        if (m_destiny)
            m_destiny->AttemptDockOperation();
        else
            DockToStation(GetDockStationID());
        SendErrorMsg("Error on Undock. You have been returned to station.  You may have to relog.");
        return;
    }

    m_destiny->SendSetState(Bubble(), GetShipID());
    //m_destiny->SendBallInfoOnUndock();
    m_destiny->SetUndockSpeed();

    SetLogin(false);
    SetStateSent(true);
    SetBubbleWait(false);

    char ci[1];
    snprintf(ci, sizeof(ci), "");
    GetShip()->SetCustomInfo(ci);
}

void Client::DockToStation(uint32 stationID) {
    if (!IsHangarLoaded(stationID))
        LoadStationHangar(stationID);
    //clear all targets.
	if (TargMgr.GetTotalTargets())
		TargMgr.ClearTargets();
    //clear AutoPilot
    SetAutoPilot(false);
    SetBubbleWait(false);

    ShipRef shipRef = GetShip();
    char ci[25];
    snprintf(ci, sizeof(ci), "Docked:%u", stationID);
    shipRef->SetCustomInfo(ci);
    //tell ship it's docking.  this deactivates modules.
    shipRef->Dock();

    uint32 solarsystemID = 0,constellationID = 0, regionID = 0;
    m_services.serviceDB().GetStationInfo(
        stationID,
        &solarsystemID, &constellationID, &regionID,
        nullptr, nullptr, nullptr
   );
    //update char session values
    GetChar()->SetLocation(stationID, solarsystemID, constellationID, regionID);
    //update session with new values
    _UpdateSession(GetChar());
    UpdateLocation(stationID);
    shipRef->Move(stationID, flagHangar);
    // When docking, Set ship's InventoryItem to system origin so that when changing ships in stations, they don't appear outside
    shipRef->Relocate(NULL_ORIGIN);

    //Check if player is in pod, in which case they get a rookie ship for free
    //  on live, SCC sends mail about the loss of the players ship, and offers a new, fully-fitted ship as replacement.  we dont....yet
    //  TODO  maybe we should check for recent ship loss to determine if player should get new ship.  ???
    if (shipRef->typeID() == itemTypeCapsule)
        SpawnNewRookieShip();

    _SendSessionChange();
    SetSessionTimer();
}

void Client::MoveToLocation(uint32 location, const GPoint& pt)
{
    if (GetLocationID() == location) {
        sLog.Warning("Client::MoveToLocation()", "GetLocationID() == location");
        // This is a simple movement
        MoveToPosition(pt);
        return;
    }

    ShipRef shipRef = GetShip();
    uint32 stationID = 0, solarSystemID = 0, constellationID = 0, regionID = 0;
    if (IsStation(location)) {
        sLog.Warning("Client::MoveToLocation()", "IsStation()");
        // Entering station
        stationID = location;

        m_services.serviceDB().GetStationInfo(
            stationID,
            &solarSystemID, &constellationID, &regionID,
            nullptr, nullptr, nullptr
        );

        shipRef->Move(stationID, flagHangar);
    } else if (IsSolarSystem(location)) {
        sLog.Warning("Client::MoveToLocation()", "IsSolarSystem()");
        // Entering a solarsystem   origin is GetLocation()    destination is location
        stationID = 0;
        solarSystemID = location;

        m_services.serviceDB().GetSystemInfo(
            solarSystemID,
            &constellationID, &regionID,
            nullptr, nullptr, nullptr
        );

        shipRef->Move(solarSystemID, flagAutoFit);
    } else {
        SendErrorMsg("Move requested to unsupported location %u", location);
        return;
    }

    shipRef->Relocate(pt);

    GetChar()->SetLocation(stationID, solarSystemID, constellationID, regionID);
    //update session with new values
    _UpdateSession(GetChar());
    EnterSystem(solarSystemID);
    _SendSessionChange();
}

void Client::MoveToPosition(const GPoint &pt) {
    if (!m_destiny)
        return;
    m_destiny->SetPosition(pt, true);
    m_destiny->Halt();
    GetShip()->Relocate(pt);
}

void Client::MoveItem(uint32 itemID, uint32 location, EVEItemFlags flag)
{
    m_services.item_factory->SetUsingClient(this);
    InventoryItemRef item = m_services.item_factory->GetItem(itemID);
    if (!item) {
        sLog.Error("Client","%s: Unable to load item %u", GetName(), itemID);
        return;
    }

    bool was_module = (item->flag() >= flagSlotFirst && item->flag() <= flagSlotLast);

    //do the move. This will update the DB and send the notification.
    item->Move(location, flag);
    //For now - this check is hacked, as i want to make sure holds do get updated and this method works.
    GetShip()->_UpdateCargoHoldsUsedVolume();

    if (was_module || (item->flag() >= flagSlotFirst && item->flag() <= flagSlotLast)) {
        //it was equipped, or is now. so ModuleMgr needs to know.
        GetShip()->UpdateModules();
    }

    // Release the item factory now that the ItemFactory is finished being used:
    m_services.item_factory->UnsetUsingClient();
}

void Client::BoardShip(ShipRef newShipRef) {
    if (!newShipRef) {
        sLog.Error("Client::BoardShip()", "%s: newShipRef == NULL.", GetName());
        SendErrorMsg("Internal Server Error.  Ref: ServerError 12321.");
        return;
    } else if (!newShipRef->singleton()) {
        sLog.Error("Client","%s: tried to board ship %u, which is not assembled.", GetName(), newShipRef->itemID());
        SendErrorMsg("You cannot board a ship which is not assembled!");
        return;
    }

    _SetSelf(newShipRef);
    // live puts char into ship inventory as flagPilot.
    //  we are missing something, as char will not load.  hangs on "entering game as ....."
    //m_char->MoveInto(*newShipRef, flagPilot, true);

    newShipRef->GetOperator()->SetOperatorObject(this);
    if (newShipRef->groupID() == EVEDB::invGroups::Capsule)
        newShipRef->InitPod();
    else
        newShipRef->Init();

    m_shipId = newShipRef->itemID();
    m_char->SetActiveShip(m_shipId);    // this also saves shipID for char in db. (error fix)

    if (!m_destiny)
        ResetDestiny();

    m_destiny->SetShipCapabilities(newShipRef);

    if (IsInSpace()) {
        mSession.SetInt("shipid", m_shipId);
        m_destiny->UpdateNewShip(newShipRef);
    }

    GetShip()->UpdateModules();
    GetShip()->SaveShip();

    SetSessionTimer();
}

void Client::UpdateCorpSession(const CharacterConstRef& character)
{
    if (!character) return;

    mSession.SetInt("corpid", character->corporationID());
    mSession.SetInt("hqID", character->corporationHQ());
    mSession.SetInt("corpAccountKey", character->corpAccountKey());
    mSession.SetULong("corpRole", character->corpRole());
    mSession.SetULong("rolesAtAll", character->rolesAtAll());
    mSession.SetULong("rolesAtBase", character->rolesAtBase());
    mSession.SetULong("rolesAtHQ", character->rolesAtHQ());
    mSession.SetULong("rolesAtOther", character->rolesAtOther());

    _SendSessionChange();
}

void Client::UpdateFleetSession(const CharacterConstRef& character)
{
    if (!character) return;

    mSession.SetLong("fleetid", character->fleetID());
    mSession.SetInt("fleetrole", character->fleetRole());
    mSession.SetInt("fleetbooster", character->fleetBooster());
    mSession.SetInt("wingid", character->wingID());
    mSession.SetInt("squadid", character->squadID());

    _SendSessionChange();
}

void Client::_UpdateSession(const CharacterConstRef& character)
{
    if (!character) return;

    mSession.SetInt("charid", character->itemID());
    mSession.SetString("charname", character->itemName().c_str());
    mSession.SetInt("corpid", character->corporationID());
    if (character->stationID()) {
        sLog.Warning("Client::_UpdateSession()", "Character %s(%u) IsDocked at %u.",
                      GetCharacterName().c_str(), GetCharacterID(), character->stationID());
        mSession.Clear("solarsystemid");  //must be 0 when in station
        mSession.Clear("shipid");

        mSession.SetInt("stationid", character->stationID());
        mSession.SetInt("stationid2", character->stationID());
        mSession.SetInt("worldspaceid", character->stationID());
        mSession.SetInt("locationid", character->stationID());
    } else {
        sLog.Warning("Client::_UpdateSession()", "Character %s(%u) InSpace at %u",
                      GetCharacterName().c_str(), GetCharacterID(), character->solarSystemID());
        mSession.Clear("stationid");
        mSession.Clear("stationid2");
        mSession.Clear("worldspaceid");

        //if (!m_autoPilot)   TODO  will have to look into AP shit more to understand what it uses to work.  ssid is only part of it.
        mSession.SetInt("solarsystemid", character->solarSystemID()); //  used to tell client they are in space
        mSession.SetInt("locationid", character->solarSystemID());
        mSession.SetInt("shipid", GetShipID());
    }
    // solarsystemid2 is used by client to determine current system.  NOTE:  *MUST* be set to current system.
	mSession.SetInt("solarsystemid2", character->solarSystemID());
    mSession.SetInt("constellationid", character->constellationID());
    mSession.SetInt("regionid", character->regionID());
}

void Client::_UpdateSession2(uint32 characterID)
{
    if (!characterID) {
        sLog.Error("Client::_UpdateSession2()", "characterID == 0");
        return;
    }

    std::map<std::string, uint64> characterDataMap;
    ((CharUnboundMgrService *)(m_services.LookupService("charUnboundMgr")))->GetCharacterData(characterID, characterDataMap);
    if (!characterDataMap.size()) {
        sLog.Error("Client::_UpdateSession2()", "characterDataMap.size() returned zero.");
        return;
    }

    uint32 stationID = static_cast<uint32>(characterDataMap["stationID"]);
    uint32 solarSystemID = static_cast<uint32>(characterDataMap["solarSystemID"]);

    mSession.SetInt("genderID", static_cast<uint32>(characterDataMap["gender"]));
    mSession.SetInt("charid", characterID);
    mSession.SetInt("corpid", static_cast<uint32>(characterDataMap["corporationID"]));
    m_shipId = static_cast<uint32>(characterDataMap["shipID"]);

    m_char->SetActiveShip(m_shipId);

    if (stationID) {
        sLog.Warning("Client::_UpdateSession2()", "Character %s(%u) IsDocked at %u.",
                      m_char->itemName().c_str(), characterID, stationID);
        mSession.Clear("solarsystemid");
        mSession.Clear("shipid");

        mSession.SetInt("stationid", stationID);
        mSession.SetInt("stationid2", stationID);
        mSession.SetInt("locationid", stationID); // used to be locationID, I don't know if this change will screw up using medical clones and such -- Aknor Jaden
    } else {
        sLog.Warning("Client::_UpdateSession2()", "Character %s(%u) InSpace at %u",
                      m_char->itemName().c_str(), characterID, solarSystemID);
        mSession.Clear("stationid");
        mSession.Clear("stationid2");
        mSession.Clear("worldspaceid");

	//  used to tell client they are in space.
	//also used as current system in following menus:  JumpPortalBridgeMenu, GetHybridBeaconJumpMenu, GetHybridBridgeMenu,
        mSession.SetInt("solarsystemid", solarSystemID);
        mSession.SetInt("locationid", solarSystemID);
        mSession.SetInt("shipid", m_shipId);
    }

    mSession.SetInt("cloneStationID", static_cast<uint32>(characterDataMap["cloneStationID"]));

    // solarsystemid2 is used by client to determine current system.
	//NOTE:  *MUST* be set to current system.
    mSession.SetInt("solarsystemid2", solarSystemID);
    mSession.SetInt("constellationid", static_cast<uint32>(characterDataMap["constellationID"]));
    mSession.SetInt("regionid", static_cast<uint32>(characterDataMap["regionID"]));

    mSession.SetInt("hqID", static_cast<uint32>(characterDataMap["corporationHQ"]));
    //mSession.SetInt("baseID", 0); //TODO  added this, means a corp alternate station, outpost/pos maybe?    -allan  28Jan15
    mSession.SetInt("corpAccountKey", static_cast<int32>(characterDataMap["corpAccountKey"]));
    mSession.SetULong("corpRole",     characterDataMap["corpRole"]);
    mSession.SetULong("rolesAtAll",   characterDataMap["rolesAtAll"]);
    mSession.SetULong("rolesAtBase",  characterDataMap["rolesAtBase"]);
    mSession.SetULong("rolesAtHQ",    characterDataMap["rolesAtHQ"]);
    mSession.SetULong("rolesAtOther", characterDataMap["rolesAtOther"]);
}

void Client::_SendCallReturn(const PyAddress& source, uint64 callID, uint32 clientID, PyRep** return_value, const char* channel)
{
    //build the packet:
    PyPacket* p = new PyPacket;
    p->type_string = "macho.CallRsp";
    p->type = CALL_RSP;

    p->source = source;

    p->dest.type = PyAddress::Client;
    p->dest.objectID = clientID;
    p->dest.callID = callID;

    p->userid = GetUserID();

    p->payload = new PyTuple(1);
    p->payload->SetItem(0, new PySubStream(*return_value));
    *return_value = nullptr;   //consumed

    if (channel) {
        p->named_payload = new PyDict();
        p->named_payload->SetItemString("channel", new PyString(channel));
    }

    if (!p) return;     // in the case of empty return packets (segfault)

    FastQueuePacket(&p);
}

void Client::_SendException(const PyAddress& source, uint64 callID, MACHONETMSG_TYPE in_response_to, MACHONETERR_TYPE exception_type, PyRep** payload)
{
    //build the packet:
    PyPacket* p = new PyPacket;
    p->type_string = "macho.ErrorResponse";
    p->type = ERRORRESPONSE;

    p->source = source;

    p->dest.type = PyAddress::Client;
    p->dest.objectID = GetClientID();
    p->dest.callID = callID;

    p->userid = GetUserID();

    macho_MachoException e;
    e.in_response_to = in_response_to;
    e.exception_type = exception_type;
    e.payload = *payload;
    *payload = nullptr;    //consumed

    p->payload = e.Encode();
    FastQueuePacket(&p);
}

void Client::_SendSessionChange()
{
    if (!mSession.isDirty())
        return;

    if (GetCharacterID() && (!GetLocationID())) {
        codelog(CLIENT__ERROR, "Session::LocationID == NULL for %s(%u)", GetCharacterName().c_str(), GetCharacterID());
        uint32 location = GetSystemID();
        if (IsDocked())
            location = GetStationID();
        UpdateSession("locationid", location);
    }

    SessionChangeNotification scn;
    scn.changes = new PyDict;

    mSession.EncodeChanges(scn.changes);
    if (scn.changes->empty())
        return;

    sLog.Debug("Client::_SendSessionChange()"," Session updated, sending session change");
    scn.changes->Dump(CLIENT__SESSION, "  Changes: ");

    //this is probably not necessary...
    scn.nodesOfInterest.push_back(services().GetNodeID());

    //build the packet:
    PyPacket* p = new PyPacket;
    p->type_string = "macho.SessionChangeNotification";
    p->type = SESSIONCHANGENOTIFICATION;

    p->source.type = PyAddress::Node;
    p->source.objectID = services().GetNodeID();
    p->source.callID = 0;

    p->dest.type = PyAddress::Client;
    p->dest.objectID = GetClientID();
    p->dest.callID = 0;

    p->userid = GetUserID();

    p->payload = scn.Encode();

    p->named_payload = nullptr;
    //p->named_payload = new PyDict();
    //p->named_payload->SetItemString("channel", new PyString("sessionchange"));

    _log(CLIENT__IN_ALL, "Sending Session packet:");
    //PyLogDumpVisitor dumper(CLIENT__OUT_ALL, CLIENT__OUT_ALL);
    //p->Dump(CLIENT__OUT_ALL, dumper);

    FastQueuePacket(&p);
}

void Client::_SendPingRequest()
{
    PyPacket *ping_req = new PyPacket();

    ping_req->type = PING_REQ;
    ping_req->type_string = "macho.PingReq";

    ping_req->source.type = PyAddress::Node;
    ping_req->source.objectID = services().GetNodeID();
    ping_req->source.service = "ping";
    ping_req->source.callID = 0;

    ping_req->dest.type = PyAddress::Client;
    ping_req->dest.objectID = GetClientID();
    ping_req->dest.callID = 0;

    ping_req->userid = GetUserID();

    ping_req->payload = new_tuple(new PyList()); //times
    ping_req->named_payload = new PyDict();

    FastQueuePacket(&ping_req);
}

void Client::_SendPingResponse(const PyAddress& source, uint64 callID)
{
    PyPacket* ret = new PyPacket;
    ret->type = PING_RSP;
    ret->type_string = "macho.PingRsp";

    ret->source = source;

    ret->dest.type = PyAddress::Client;
    ret->dest.objectID = GetClientID();
    ret->dest.callID = callID;

    ret->userid = GetUserID();

    /*  Here the hacking begins, the ping packet handles the timestamps of various packet handling steps.
        To really simulate/emulate that we need the various packet handlers which in fact we don't have (:P).
        So the next piece of code "fake's" it, with a slight delay on the received packet time.
    */
    PyList* pingList = new PyList;
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
    ret->payload = new PyTuple(1);
    ret->payload->SetItem(0, pingList);

    // Don't clone so it eats the ret object upon sending.
    FastQueuePacket(&ret);
}

//these are specialized Queue functions when our caller can
//easily provide us with our own copy of the data.
void Client::QueueDestinyUpdate(PyTuple **update /*, bool addballs, bool removeballs*/) {
    /*
    bool addball = false, removeball = false;
    //  this needs more reasearch into the logic of how to make it right.  may not need.
    if (((balls) && m_packaged) || m_packaged) {
        // this will package all current updates (and those comming in before next flush) into
        //   a single PackagedAction packet, which is then inserted into the DoDestinyAction packet.
        size_t i = m_destinyUpdateQueue->size();
        PyList* paList = new PyList(i);
        for (auto n : m_destinyUpdateQueue) {
            paList->AddItem(--i, n);
        }
        PackagedAction pa;
            pa.substream = new PySubStream(paList);
    }  */
    DoDestinyAction act;
        act.stamp = sEntityList.GetStamp();
        act.update = *update;
    *update = nullptr;

    m_destinyUpdateQueue->AddItem(act.Encode());
}

void Client::QueueDestinyEvent(PyTuple** multiEvent) {
    m_destinyEventQueue->AddItem(*multiEvent);
    *multiEvent = nullptr;
}

void Client::FlushQueue() {
    if ((!m_destinyUpdateQueue->empty())
        || (!m_destinyEventQueue->empty()))
        _SendQueuedUpdates();
}

void Client::_SendQueuedUpdates() {
    if (!m_destinyUpdateQueue->empty()) {
        DoDestinyUpdateMain dum;

        //first insert the destiny updates.
        dum.updates = m_destinyUpdateQueue;
        PyIncRef(m_destinyUpdateQueue);

        //encode any multi-events which go along with it.
        dum.events = m_destinyEventQueue;
        PyIncRef(m_destinyEventQueue);

        // attempted fix for trying to update when (bubble == NULL)
        //  seems to work correctly  -allan 18Apr15
        if (Bubble())
            dum.waitForBubble = IsBubbleWait();
        else
            dum.waitForBubble = false;

        //now send it
        PyTuple* t = dum.Encode();
        t->Dump(DESTINY__UPDATES, "");
        SendNotification("DoDestinyUpdate", "clientID", &t);
    } else if (!m_destinyEventQueue->empty()) {
        Notify_OnMultiEvent nom;

        //insert updates, clear our queue
        nom.events = m_destinyEventQueue;
        PyIncRef(m_destinyEventQueue);

        //send it
        PyTuple* t = nom.Encode();   //this is consumed below
        t->Dump(DESTINY__UPDATES, "");
        SendNotification("OnMultiEvent", "charid", &t);
    } //else nothing to be sent ...

    // clear the queues now, after the packets have been sent
    m_destinyEventQueue->clear();
    m_destinyUpdateQueue->clear();
}

void Client::SendNotification(const char *notifyType, const char *idType, PyTuple **payload, bool seq) {
    //build a little notification out of it.
    EVENotificationStream notify;
        notify.remoteObject = 1;
        notify.args = *payload;    //consumed
    *payload = nullptr;

    PyAddress dest;
        dest.type = PyAddress::Broadcast;
        dest.service = notifyType;
        dest.bcast_idtype = idType;

    //now send it to the client
    SendNotification(dest, notify, seq);
}

void Client::SendNotification(const PyAddress &dest, EVENotificationStream &noti, bool seq) {
    //build the packet:
    PyPacket *p = new PyPacket();
    p->type_string = "macho.Notification";
    p->type = NOTIFICATION;

    p->source.type = PyAddress::Node;
    p->source.objectID = m_services.GetNodeID();

    p->dest = dest;

    p->userid = GetUserID();

    p->payload = noti.Encode();

    if (seq) {
        p->named_payload = new PyDict();
        p->named_payload->SetItemString("sn", new PyInt(m_nextNotifySequence++));
    }

    _log(CLIENT__NOTIFY_REP, "Sending notify of type %s with ID type %s", dest.service.c_str(), dest.bcast_idtype.c_str());
    if (is_log_enabled(CLIENT__NOTIFY_REP)) {
        PyLogDumpVisitor dumper(CLIENT__NOTIFY_REP, CLIENT__NOTIFY_REP, "", true, true);
        p->Dump(CLIENT__NOTIFY_REP, dumper);
    }

    FastQueuePacket(&p);
}

PyDict* Client::MakeSlimItem() const {
    //  this is actually the slimItem for the clients current ship.  easier to get data here.

    _log(COMMON__WARNING, "MakeSlimItem for ShipID %u via Client %u", GetShip()->itemID(), GetCharacterID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",           new PyLong(GetShip()->itemID()));
        slim->SetItemString("typeID",           new PyInt(GetShip()->typeID()));
        slim->SetItemString("categoryID",       new PyInt(GetShip()->categoryID()));
        slim->SetItemString("groupID",          new PyInt(GetShip()->groupID()));
        slim->SetItemString("name",             new PyString(GetShip()->itemName()));
        slim->SetItemString("ownerID",          new PyInt(GetCharacterID()));
        slim->SetItemString("charID",           new PyInt(GetCharacterID()));
        slim->SetItemString("corpID",           new PyInt(GetCorporationID()));
        slim->SetItemString("allianceID",       new PyInt(GetAllianceID()));
        slim->SetItemString("warFactionID",     new PyInt(GetWarFactionID()));
        slim->SetItemString("bounty",           new PyFloat(GetBounty()));
        slim->SetItemString("securityStatus",   new PyFloat(GetSecurityRating()));

    //encode the modules list, if we have any visible modules
    std::vector<InventoryItemRef> items;
    GetShip()->FindByFlagRange(flagLowSlot0, flagHiSlot7, items);
    if (!items.empty())
    {
        PyList *l = new PyList();

        std::vector<InventoryItemRef>::iterator cur;
        cur = items.begin();
        for (; cur != items.end(); cur++) {

            PyTuple* t = new_tuple((*cur)->itemID(), (*cur)->typeID());
            l->AddItem(t);
        }

        slim->SetItemString("modules", l);
        PySafeDecRef(l);
    }

    return(slim);
}

void Client::EncodeDestiny(Buffer& into) const
{
    const GPoint& position = GetPosition();

    uint8 mode = Destiny::DSTBALL_STOP;
    if (Destiny()->IsWarping())
        mode = Destiny::DSTBALL_WARP;
    else if (Destiny()->IsFollowing())
        mode = Destiny::DSTBALL_FOLLOW;
    else if (Destiny()->IsOrbiting())
        mode = Destiny::DSTBALL_ORBIT;
    else if (Destiny()->IsMoving())
        mode = Destiny::DSTBALL_GOTO;

    Destiny::BallHeader head;
        head.entityID = GetID();
        head.mode = mode;
        head.radius = GetRadius();
        head.x = position.x;
        head.y = position.y;
        head.z = position.z;
        head.flags = Destiny::IsFree | Destiny::IsMassive;
    into.Append(head);

    Destiny::MassSector mass;
        mass.mass = GetMass();
        mass.cloak = m_destiny->IsCloaked();
        mass.Harmonic = 1.0f;
        mass.corporationID = GetCorporationID();
        mass.allianceID = GetAllianceID();
    into.Append(mass);

    Destiny::ShipSector ship;
        ship.maxVelocity = GetMaxVelocity();
        ship.velocity_x = GetVelocity().x;
        ship.velocity_y = GetVelocity().y;
        ship.velocity_z = GetVelocity().z;
        ship.agility = GetAgility();
        ship.speedfraction = m_destiny->GetSpeedFraction();
    into.Append(ship);

    if (mode == Destiny::DSTBALL_WARP) {
        GPoint target = m_destiny->GetTargetPoint();
        Destiny::DSTBALL_WARP_Struct warp;
            warp.effectStamp = -1;   //unknown value  seen many -1, few other random 4-5 digits
            warp.unknown_x = target.x;
            warp.unknown_y = target.y;
            warp.unknown_z = target.z;
            warp.ownerID = m_destiny->GetWarpSpeed();       //ship warp speed x10  (dont ask...this is what it is...more dumb ccp shit)
            warp.unk_1 = 0;      //unknown 64bit number.  seen 4666723172467343360 once....others are 0
            warp.unk_2 = 0;         //unknown 64bit number
        into.Append(warp);
    } else if (mode == Destiny::DSTBALL_FOLLOW) {
        Destiny::DSTBALL_FOLLOW_Struct follow;
            follow.followID = m_destiny->GetTargetID();
            follow.followRange = m_destiny->GetFollowDistance();
            follow.formationID = 0xFF;
        into.Append(follow);
    } else if (mode == Destiny::DSTBALL_ORBIT) {
        Destiny::DSTBALL_ORBIT_Struct orbit;
            orbit.followID = m_destiny->GetTargetID();
            orbit.followRange = m_destiny->GetFollowDistance();
            orbit.formationID = 0xFF;
        into.Append(orbit);
    } else if (mode == Destiny::DSTBALL_GOTO) {
        GPoint target = m_destiny->GetTargetPoint();
        Destiny::DSTBALL_GOTO_Struct go;
            go.x = target.x;
            go.y = target.y;
            go.z = target.z;
        into.Append(go);
    } else {
        Destiny::DSTBALL_STOP_Struct main;
            main.formationID = 0xFF;
        into.Append(main);
    }
    _log(COMMON__WARNING, "Client::EncodeDestiny() - %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void Client::StartKilledTimer() {
    m_killedTimer.Start(1000);
}

bool Client::IsJetcanAvalible() {
    if (m_jetcanTimer.Enabled())
        return (m_jetcanTimer.Check(false));
    else
        return true;
}

PyRep *Client::GetAggressors() const {
    PyDict *dict = nullptr;
    /*  items are set here as
     *               [PyInt 90971469]     <- entity
     *               [PyDict 1 kvp]       <- dictionary
     *                  [PyInt 1000127]   <- agression TO corpID
     *                  [PyIntegerVar 129550906897224125] <- timestamp (dont know if this is begin time or end time)
     */
    /*
     *    while (entity has agressions)
     *        dict->SetItem(PyInt(agressionToEntityID, Win32TimeNow()));
     */
    return dict;
}

void Client::SetAutoPilot(bool autoPilot) {
    //FIXME  this needs more info and lots more work
    if (autoPilot) {
        if (m_autoPilot)
            mSession.SetInt("solarsystemid", GetSystemID());   //this is currrent system.
        else
            mSession.SetInt("solarsystemid", GetSystemID());   //this is currrent system.
    } else
        mSession.SetInt("solarsystemid", GetSystemID());   //this is currrent system.
    m_autoPilot = autoPilot;

}

void Client::StargateJump(uint32 fromGate, uint32 toGate) {
    if ((m_moveState != msIdle) || m_moveTimer.Enabled()) {
        sLog.Error("Client","%s: StargateJump called when a jump is already pending. Ignoring.", GetName());
        return;
    }

    uint32 solarSystemID = 0;
    GPoint position(NULL_ORIGIN);
    if (!m_services.serviceDB().GetStaticItemInfo(
        toGate, &solarSystemID, nullptr, nullptr, &position
   )) {
        sLog.Error("Client","%s: Failed to query information for stargate %u", GetName(), toGate);
        return;
    }

    GetShip()->DeactivateAllModules();

    m_moveSystemID = solarSystemID;
    m_movePoint = position;
    m_movePoint.MakeRandomPointOnSphereLayer(7500, 9500);   // Make Jump-In point a random spot on a 10km radius sphere about the stargate

    char ci[25];
    snprintf(ci, sizeof(ci), "Jumping:%u", toGate);
    GetShip()->SetCustomInfo(ci);

    // add jump to mapDynamicData for showing in StarMap (F10)    -allan 06Mar14
    // this is the code for removing pilot from previous system
    uint32 fromSystem = 0;
    if (!m_services.serviceDB().GetStaticItemInfo(
        fromGate,
        &fromSystem, nullptr, nullptr, nullptr )) {
        sLog.Error("Client","%s: Failed to query information for stargate %u", GetName(), fromGate);
        return;
    }
    //add jump in previous system
    GetChar()->chkDynamicSystemID(fromSystem);
    GetChar()->AddJumpToDynamicData(fromSystem);
    //add jump in this system
    GetChar()->chkDynamicSystemID(solarSystemID);
    GetChar()->AddJumpToDynamicData(solarSystemID);
    // used for showing Visited Systems in StarMap(F10)  -allan 30Jan14
    GetChar()->VisitSystem(solarSystemID);

    // call Stop() per packet sniff
    m_destiny->Stop();
    //  show gate animation in both to and from gate.   -working -allan 15Nov15
    m_destiny->SendJumpOut(fromGate);
    m_destiny->SendJumpOut(toGate);

    //delay the move so they can see the JumpOut animation
    _postMove(msJump, 5000);
}

void Client::_postMove(_MoveState type, uint32 wait_ms) {
    m_moveState = type;
    m_moveTimer.Start(wait_ms);
}

void Client::_ExecuteJump() {
    if (!m_destiny)
        SetDestiny();

    char ci[1];
    snprintf(ci, sizeof(ci), "");
    GetShip()->SetCustomInfo(ci);

    SetInvul(true);
    SetBubbleWait(true);
    SetBeyonce(false);
    SetStateSent(false);
    MoveToLocation(m_moveSystemID, m_movePoint);
    m_destiny->Cloak();
}

void Client::IsJumping()
{
    m_cloakTimer.Start(10000);
    m_invulTimer.Start(/*InvulTimer::*/JumpingInvul);
    m_jumpTimer.Start(500);
}

bool Client::AddBalance(double amount) {
    if (!GetChar()->AlterBalance(amount))
        return false;

    //send notification of change
    OnAccountChange ac;
        ac.accountKey = "cash";
        ac.ownerid = GetCharacterID();
        ac.balance = GetBalance();
    PyTuple *answer = ac.Encode();
    SendNotification("OnAccountChange", "cash", &answer, false);

    return true;
}

bool Client::SelectCharacter(uint32 char_id) {
    m_services.item_factory->SetUsingClient(this);
    m_char = m_services.item_factory->GetCharacter(char_id);
    if (!GetChar()) {
          sLog.Error("Client::SelectCharacter()", "GetChar for %u = nullptr", char_id);
        // Release the item factory now that the ItemFactory is finished being used:
        m_services.item_factory->UnsetUsingClient();
        return false;
    }

    _UpdateSession2(char_id);

    uint32 systemID =GetSystemID();
    m_system = sEntityList.FindOrBootSystem(systemID);

    if (!m_system) {
        sLog.Error("Client::LoginToSystem()", "Failed to boot system %u for char %s (%u)", systemID, GetName(), GetCharacterID());
        SendErrorMsg("Unable to boot system %u", systemID);
        return false;
    }

    if (!m_char->capsuleID())
        CreateNewPod();

    ShipRef ship = m_services.item_factory->GetShip(GetShipID());
    if (!ship) {
        sLog.Error("Client::SelectCharacter()", "ship for %u = nullptr.  Picking new ship...", char_id);
        PickAlternateShip();    // incase shipID wasnt set correctly in db (seen on 'bad' Damage::Killed())
        ship = m_services.item_factory->GetShip(GetShipID());
        if (!ship) {
            sLog.Error("Client::SelectCharacter()", "ship for %u = nullptr Again.  Loading Pod.", char_id);
            // Release the item factory now that the ItemFactory is finished being used:
            ship = m_services.item_factory->GetShip(GetPodID());
            m_services.item_factory->UnsetUsingClient();
        }
    }

    _SetSelf(ship);
    ship->GetOperator()->SetOperatorObject(this);
    ship->Init();
    m_shipId = ship->itemID();
    m_char->SetActiveShip(m_shipId);    // this also saves shipID for char in db. (error fix)
    LoginToSystem(systemID);
    UpdateSkillTraining();
    _SendSessionChange();

    //johnsus - characterOnline mod
    m_services.serviceDB().SetCharacterOnlineStatus(GetCharacterID(), true);
    // Release the item factory now that we're finished with it.
    m_services.item_factory->UnsetUsingClient();
    m_char->SetLoginTime();
    sLog.Success("Client::SelectCharacter()", "SelectCharacter for %u completed", char_id);
    return true;
}

void Client::PickAlternateShip() {
    if (GetChar())
        m_shipId = GetChar()->PickAlternateShip(GetLocationID());
}

void Client::CreateNewPod() {
    EVEItemFlags flag = flagCapsule;
    //if (IsDocked()) flag = flagHangar;
    std::string pod_name = m_char->itemName() + "'s Capsule";   // use m_char because GetCharacterName() may not be pouplated (i.e. on login)
    ItemData podItem( itemTypeCapsule, GetCharacterID(), GetLocationID(), flag, pod_name.c_str() );
    ShipRef podItemRef = m_services.item_factory->SpawnShip( podItem );
    ShipEntity* pPodEntity = new ShipEntity(podItemRef, System(), m_services, NULL_ORIGIN);
    System()->AddEntity(pPodEntity);
    m_char->SetActivePod(podItemRef->itemID());
}

void Client::SpawnNewRookieShip() {
    //create rookie ship of appropriate type
    uint32 typeID = amarrRookie;
    EVERace race = GetChar()->race();
    if (race == raceAmarr)  typeID = amarrRookie;
    else if (race == raceCaldari)  typeID = caldariRookie;
    else if (race == raceGallente)  typeID = gallenteRookie;
    else if (race == raceMinmatar)  typeID = minmatarRookie;

    std::string name = GetCharacterName() + "'s Noob Ship";

    //create data for new rookie ship
    ItemData idata(
        typeID,
        GetCharacterID(),
        0, //temp location
        flagHangar,
        name.c_str(),
        NULL_ORIGIN
    );
    //spawn rookie ship
    ShipRef i = services().item_factory->SpawnShip(idata);

    if (!i)
        throw PyException(MakeCustomError("Unable to generate rookie ship"));

    //move the new rookie ship into the players hanger in station
    i->Move(GetStationID(), flagHangar, true);
}

void Client::ResetAfterPodded()
{ //  NOTE TODO   ****  this method has not been tested yet.  ****
    //TODO: destroy all implants
    //TODO  check skillpoints vs. clone grade and adjust accordingly.

    //clear all targets.
    if (TargMgr.GetTotalTargets())
        TargMgr.ClearTargets();
    //clear AutoPilot
    SetAutoPilot(false);
    SetBubbleWait(false);

    uint32 solarsystemID = 0,constellationID = 0, regionID = 0, stationID = GetCloneStationID();
    m_services.serviceDB().GetStationInfo(
        stationID,
        &solarsystemID, &constellationID, &regionID,
        nullptr, nullptr, nullptr
    );
    GetChar()->SetLocation(stationID, solarsystemID, constellationID, regionID);

    EnterSystem(solarsystemID);
    SpawnNewRookieShip();
    CreateNewPod();
    ShipRef podRef = System()->GetShipFromInventory(m_char->capsuleID());
    _SetSelf(podRef);
    podRef->GetOperator()->SetOperatorObject(this);

    m_shipId = podRef->itemID();
    m_char->SetActiveShip(m_shipId);    // this also saves shipID for char in db. (error fix)

    if (m_destiny)
        m_destiny->SetShipCapabilities(podRef);

    GetShip()->UpdateModules();
    GetShip()->SaveShip();
    GetChar()->ResetClone();
    GetChar()->SaveCharacter();
    //update session with new values
    _UpdateSession(GetChar());
    _SendSessionChange();
}

void Client::UpdateSkillTraining() {
    if (GetChar())
        m_timeEndTrain = GetChar()->GetEndOfTraining();
    else
        m_timeEndTrain = 0;
}

void Client::AddStationHangar(uint32 stationID)
{
    m_hangarLoaded.insert(std::make_pair(stationID, true));
}

bool Client::IsHangarLoaded(uint32 stationID)
{
    std::map<uint32, bool>::const_iterator itr = m_hangarLoaded.find(stationID);
    if (itr != m_hangarLoaded.end())
        return itr->second;
    return false;
}

void Client::LoadStationHangar(uint32 stationID)
{
    sLog.Warning("Client::LoadStationHangar()", "Loading Hangar for %s(%u) in stationID %u", GetName(), GetID(), stationID);
    StationRef sRef = System()->GetStationFromInventory(stationID);
    m_system->itemFactory()->SetUsingClient(this);
    sRef->LoadContents(m_system->itemFactory());
    m_system->itemFactory()->UnsetUsingClient();
}

void Client::TargetAdded(SystemEntity* who)
{
    PyTuple* up = nullptr;

    DoDestiny_OnDamageStateChange odsc;
    odsc.entityID = who->GetID();
    odsc.state = who->MakeDamageState();
    up = odsc.Encode();
    QueueDestinyUpdate(&up);

    Notify_OnTarget te;
    te.mode = "add";
    te.targetID = who->GetID();

    up = te.Encode();
    QueueDestinyEvent(&up);
    PySafeDecRef(up);
}

void Client::TargetLost(SystemEntity *who)
{
    //OnMultiEvent: OnTarget lost
    Notify_OnTarget te;
    te.mode = "lost";
    te.targetID = who->GetID();

    Notify_OnMultiEvent multi;
    multi.events = new PyList;
    multi.events->AddItem(te.Encode());

    PyTuple* tmp = multi.Encode();   //this is consumed below
    SendNotification("OnMultiEvent", "clientID", &tmp);
}

void Client::TargetedAdd(SystemEntity *who) {
    //OnMultiEvent: OnTarget otheradd
    Notify_OnTarget te;
    te.mode = "otheradd";
    te.targetID = who->GetID();

    Notify_OnMultiEvent multi;
    multi.events = new PyList;
    multi.events->AddItem(te.Encode());

    PyTuple* tmp = multi.Encode();   //this is consumed below
    SendNotification("OnMultiEvent", "clientID", &tmp);
}

void Client::TargetedLost(SystemEntity *who)
{
    //OnMultiEvent: OnTarget otherlost
    Notify_OnTarget te;
    te.mode = "otherlost";
    te.targetID = who->GetID();

    Notify_OnMultiEvent multi;
    multi.events = new PyList;
    multi.events->AddItem(te.Encode());

    PyTuple* tmp = multi.Encode();   //this is consumed below
    SendNotification("OnMultiEvent", "clientID", &tmp);
}

void Client::TargetsCleared()
{
    //OnMultiEvent: OnTarget clear
    Notify_OnTarget te;
    te.mode = "clear";
    te.targetID = 0;

    Notify_OnMultiEvent multi;
    multi.events = new PyList;
    multi.events->AddItem(te.Encode());

    PyTuple* tmp = multi.Encode();   //this is consumed below
    SendNotification("OnMultiEvent", "clientID", &tmp);
}

void Client::SavePosition() {
    if (!GetShip().get() || !m_destiny) {
        sLog.Debug("Client","%s: Unable to save position. We are probably not in space.", GetName());
        return;
    }
    GetShip()->Relocate(m_destiny->GetPosition());
}

void Client::SaveAllToDatabase() {
    SavePosition();
    if (GetShip()) GetShip()->SaveShip();  // Save Ship and Modules' attributes and info to DB

    GetChar()->SaveFullCharacter();         // Save Character info to DB
}

PyRep *Client::GetInfoWindowDataForChar(Client *pClient) {
    CharacterDB c_db;
    return c_db.GetInfoWindowDataForChar(pClient->GetCharacterID());
}

bool Client::LaunchDrone(InventoryItemRef drone) {
    if (!IsSolarSystem(GetLocationID())) {
        sLog.Log("Client::LaunchDrone()","%s: Trying to launch drone when not in space!", GetName());
        return false;
    }

    sLog.Log("Client::LaunchDrone()","%s: Launching drone %u", GetName(), drone->itemID());

    drone->Move(GetSystemID(), flagAutoFit);
    drone->ChangeOwner(GetClientID());

    GPoint position(GetPosition());
    position.MakeRandomPointOnSphere(500.0);

    //now we create an entity to represent it.
    Drone* pDrone = new Drone(drone, m_system, m_services, position);
    pDrone->SetOwner(this);
    // add drone entity to system, set speed, begin orbit around launching ship
    m_system->AddEntity(pDrone);
    pDrone->Destiny()->Orbit(pDrone->GetOwner(), 800);  //FIXME
    pDrone->Destiny()->SetMaxVelocity(500);             //FIXME
    pDrone->Destiny()->SetSpeedFraction(0.5f);          //FIXME

    DoDestiny_OnDroneStateChange du;
        du.droneID = drone->itemID();
        du.ownerID = GetCharacterID();
        du.droneTypeID = drone->typeID();
        du.controllerID = GetShipID();
        du.controllerOwnerID = GetCharacterID();
        du.activityState = 0;   // DroneState::Idle
        du.targetID = GetShipID();  // use ship as initial target for launch and orbit command
    PyTuple* up = du.Encode();
    Destiny()->SendSingleDestinyUpdate(&up);

    return true;
}

//assumes that the backend DB stuff was already done.
void Client::JoinCorporationUpdate(uint32 corp_id) {
    //GetChar()->JoinCorporation(corp_id);
    _UpdateSession(GetChar());
    //logs indicate that we need to push this update out asap.
    _SendSessionChange();
}

/************************************************************************/
/* character notification messages wrapper                              */
/************************************************************************/
void Client::OnCharNoLongerInStation()
{
    NotifyOnCharNoLongerInStation n;
        n.charID = GetCharacterID();
        n.corpID = GetCorporationID();
        n.allianceID = GetAllianceID();
        n.factionID = GetWarFactionID();
    PyTuple* tmp = n.Encode();
    sEntityList.Broadcast("OnCharNoLongerInStation", "stationid", &tmp);
}

void Client::OnCharNowInStation()
{
    NotifyOnCharNowInStation n;
        n.charID = GetCharacterID();
        n.corpID = GetCorporationID();
        n.allianceID = GetAllianceID();
        n.warFactionID = GetWarFactionID();
    PyTuple* tmp = n.Encode();
    sEntityList.Broadcast("OnCharNowInStation", "stationid", &tmp);
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
    services().serviceDB().SetAccountBanStatus(GetUserID(), true);
}

/************************************************************************/
/* EVEClientSession interface                                           */
/************************************************************************/
void Client::_GetVersion(VersionExchangeServer& version)
{
    version.birthday = EVEBirthday;
    version.macho_version = MachoNetVersion;
    version.user_count = _GetUserCount();
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
    sLog.Log("Client","%s: Received Low Level Version Exchange:", GetAddress().c_str());
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
    std::string account_hash;
    std::string transport_closed_msg = "LoginAuthFailed";
    int32 isOnline;

    AccountInfo account_info;
    CryptoServerHandshake server_shake;

    /* send passwordVersion required: 1=plain, 2=hashed */
    // TODO  look into using plain pass to facilitate forgotten-password retrieval from web.
    PyRep* rsp = new PyInt(2);

    //sLog.Debug("Client","%s: Received Client Challenge.", GetAddress().c_str());
    //sLog.Debug("Client","Login with %s:", ccp.user_name.c_str());

    if (!services().serviceDB().GetAccountInformation(
				ccp.user_name.c_str(),
				ccp.user_password_hash.c_str(),
				account_info))
	{
        goto error_login_auth_failed;
    }

    /* check wether the account has been banned and if so send the semi correct message */
    if (account_info.banned) {
        transport_closed_msg = "Your account is banned. Contact the Game Master for further support";
        goto error_login_auth_failed;
    }

    /* if we have stored a password we need to create a hash from the username and pass and remove the pass */
    if (account_info.password.empty())
        account_hash = account_info.hash;
    else
    {
        /* here we generate the password hash ourselves */
        std::string password_hash;
        if (!PasswordModule::GeneratePassHash(
                ccp.user_name,
                account_info.password,
                password_hash))
        {
            sLog.Error("Client", "unable to generate password hash, sending LoginAuthFailed");
            goto error_login_auth_failed;
        }

        if (!services().serviceDB().UpdateAccountHash(
                ccp.user_name.c_str(),
                password_hash))
        {
            sLog.Error("Client", "unable to update account hash, sending LoginAuthFailed");
            goto error_login_auth_failed;
        }

        account_hash = password_hash;
    }

    /* here we check if the user successfully entered his password or if he failed
     * If the name check runs out correctly, we go to online check (DB call + respective variable definition)
     * This is not the prettiest way to do this, but i didn't wanted to do any new constructs just for temporary online check.
     * TODO: figure out why the heck online status does not get updated in account_info. */
    if (account_hash != ccp.user_password_hash) {
    	transport_closed_msg = "Your login/password was entered incorrectly.";
        goto error_login_auth_failed;
    }else{
        // I am NOT happy with this,but i'll have to put this hack here, as AcountInfo do not get updated properly, so it keeps
        // online status = False at all times. So, to make sure the check's being performed properly, i copy the query runner from
        // ServiceDB.cpp and modifying those parts to only return us the status. This will make sure that we get a valid data every time the function is called
    //	############################################################################################################################
        DBQueryResult online_indicator;
        if ( !sDatabase.RunQuery( online_indicator, "SELECT online FROM account WHERE accountName = '%s'", ccp.user_name.c_str() ) )
            {
                sLog.Error( "ServiceDB", "Error in query: %s.", online_indicator.error.c_str() );
                return false;
            }

        DBResultRow row;
        if(online_indicator.GetRow(row)){
        	isOnline = row.GetInt(0);
        }

    //	############################################################################################################################
    }

    /* Check if we already have a client online and if we do disconnect it
     * @note we should send GPSTransportClosed with reason "The user's connection has been usurped on the proxy"
     */

    if (isOnline == 1) {
        Client* client = sEntityList.FindClientByAccount(account_info.id);
        if (client){
        	transport_closed_msg = "This account is being used right now. Try logging in again later.";
        	// If user logs-out while on the login screen, the online status will stay True until the server gets restarted.
        	// So disconnecting the parent client is a neccessary measure to make sure user can log in after that.
        	//client->DisconnectClient();
        	goto error_login_auth_failed;
        }
    }

    mNet->QueueRep(rsp);
    PyDecRef(rsp);

    /* update account information, increase login count, last login timestamp and mark account as online */
    m_services.serviceDB().UpdateAccountInformation(account_info.name.c_str(), true);

    /* marshaled Python string "None" */
    static const uint8 handshakeFunc[] = { 0x74, 0x04, 0x00, 0x00, 0x00, 0x4E, 0x6F, 0x6E, 0x65 };

    /* send our handshake */

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

    rsp = server_shake.Encode();
    mNet->QueueRep(rsp);
    PyDecRef(rsp);

    // Setup session, but don't send the change yet.
    mSession.SetString("address", EVEClientSession::GetAddress().c_str());
    mSession.SetString("languageID", ccp.user_languageid.c_str());

    //user type 1 is normal user, type 23 is a trial account user.
    mSession.SetInt("userType", 1);
    mSession.SetInt("userid", account_info.id);
    mSession.SetInt("clientID", account_info.clientID);
    mSession.SetULong("role", account_info.role);
    //mSession.SetInt("inDetention", 0);

    sLog.Success("  Client::Login()","Account \"%s\" logged in from IP %s", account_info.name.c_str() ,EVEClientSession::GetAddress().c_str());

    return true;

error_login_auth_failed:

    GPSTransportClosed* except = new GPSTransportClosed(transport_closed_msg);
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
        ack.userid = GetUserID();
        ack.maxSessionTime = new PyNone;
        ack.userType = 1;
        ack.role = GetAccountRole();
        ack.address = GetAddress();
        ack.inDetention = new PyNone;
    // no client update available
        ack.client_hash = new PyNone;
        ack.user_clientid = GetClientID();
        ack.live_updates = sLiveUpdateDB.GetUpdates();
    PyRep* r = ack.Encode();
    mNet->QueueRep(r);
    PyDecRef(r);

    // Send out the session change
    _SendSessionChange();

    return true;
}

/************************************************************************/
/* EVEPacketDispatcher interface                                        */
/************************************************************************/
bool Client::Handle_CallReq(PyPacket* packet, PyCallStream& req)
{
    PyCallable* dest;
    if (packet->dest.service == "")
    {
        //bound object
        uint32 nodeID, bindID;
        if (sscanf(req.remoteObjectStr.c_str(), "N=%u:%u", &nodeID, &bindID) != 2)
        {
            sLog.Error("Client","Failed to parse bind string '%s'.", req.remoteObjectStr.c_str());
            return false;
        }

        if (nodeID != m_services.GetNodeID())
        {
            sLog.Error("Client","Unknown nodeID %u received (expected %u).", nodeID, m_services.GetNodeID());
            return false;
        }

        dest = services().FindBoundObject(bindID);
        if (dest == nullptr)
        {
            sLog.Error("Client", "Failed to find bound object %u.", bindID);
            return false;
        }
    }
    else
    {
        //service
        dest = services().LookupService(packet->dest.service);
        if (dest == nullptr)
        {
            sLog.Error("Client","Unable to find service to handle call to: %s", packet->dest.service.c_str());
            packet->dest.Dump(CLIENT__ERROR, "    ");

//#pragma message("TODO: throw proper exception to client (exceptions.ServiceNotFound).")
            throw PyException(new PyNone);
        }
    }

    //Debug code
    if (req.method == "BeanCount")
        sLog.Warning("Client::BeanCount","(%s/%s) BeanCount error reporting and handling is not implemented yet.", \
                     req.method.c_str(),packet->dest.service.c_str());
    else
        sLog.Debug("Server", "%s call made to %s",req.method.c_str(),packet->dest.service.c_str());

    //build arguments
    PyCallArgs args(this, req.arg_tuple, req.arg_dict);

    //parts of call may be consumed here
    PyResult result = dest->Call(req.method, args);

    _SendSessionChange();  //send out the session change before the return.
    result.ssResult->Dump(SERVICE__WARNING, "    ");
    _SendCallReturn(packet->dest, packet->source.callID, GetClientID(), &result.ssResult);

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
        for (; cur != notify.elements->end(); cur++) {
            if (!element.Decode(*cur)) {
                sLog.Error("Client","Notification '%s' from %s: Failed to decode element. Skipping.", notify.method.c_str(), GetName());
                continue;
            }

            uint32 nodeID, bindID;
            if (sscanf(element.boundID.c_str(), "N=%u:%u", &nodeID, &bindID) != 2) {
                sLog.Error("Client","Notification '%s' from %s: Failed to parse bind string '%s'. Skipping.",
                    notify.method.c_str(), GetName(), element.boundID.c_str());
                continue;
            }

            if (nodeID != m_services.GetNodeID()) {
                sLog.Error("Client","Notification '%s' from %s: Unknown nodeID %u received (expected %u). Skipping.",
                    notify.method.c_str(), GetName(), nodeID, m_services.GetNodeID());
                continue;
            }

            m_services.ClearBoundObject(bindID);
        }
    } else {
        sLog.Error("Client","Unhandled notification from %s: unknown method '%s'", GetName(), notify.method.c_str());
        return false;
    }

    _SendSessionChange();  //just for good measure...
    return true;
}

void Client::UpdateSession(const char *sessionType, int value)
{
    mSession.SetInt(sessionType, value);
}


//this displays a modal error dialog on the client side.
void Client::SendErrorMsg(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char* str = nullptr;
    vasprintf(&str, fmt, args);
    assert(str);

    sLog.Error("Client","Sending Error Message to %s:", GetName());
    log_messageVA(CLIENT__ERROR, fmt, args);
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

    sLog.Error("Client","Sending Error Message to %s:", GetName());
    log_messageVA(CLIENT__ERROR, fmt, args);

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

    sLog.Log("Client","Info Modal to %s:", GetName());
    log_messageVA(CLIENT__MESSAGE, fmt, args);
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

    sLog.Log("Client","Notify to %s:", GetName());
    log_messageVA(CLIENT__MESSAGE, fmt, args);
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

    sLog.Log("Client","Notify to %s:", GetName());
    log_messageVA(CLIENT__MESSAGE, fmt, args);

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

    if (m_channels.empty())
    {
        sLog.Error("Client", "%s: Tried to send self chat, but we are not joined to any channels: %s", GetName(), str);
        free(str);
        return;
    }

    sLog.Log("Client","%s: Self message on all channels: %s", GetName(), str);

    //this is such a pile of crap, but im not sure whats better.
    //maybe a private message...
    std::set<LSCChannel*>::iterator cur;
    cur = m_channels.begin();
    for (; cur != m_channels.end(); ++cur)
        (*cur)->SendMessage(this, str, true);

    //m_channels[

    //just send it to the first channel we are in..
    /*LSCChannel *chan = *(m_channels.begin());
     *    char self_id[24];   //such crap..
     *    snprintf(self_id, sizeof(self_id), "%u", GetCharacterID());
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
