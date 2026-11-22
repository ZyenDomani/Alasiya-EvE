/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Rewrite:    Allan   (reorganized header 18Nov26)
*/

#pragma once

#include "ClientSession.h"

#include "character/Character.h"
#include "inventory/InventoryItem.h"
#include "inventory/ItemRef.h"
#include "packets/Crypto.h"
#include "packets/Destiny.h"
#include "packets/LSCPkts.h"
#include "ship/Ship.h"
#include "ship/modules/ModuleManager.h"
#include "system/SpaceEntity.h"

#include "../eve-common/EVE_Missions.h"
#include "../eve-common/EVE_Player.h"

class CryptoChallengePacket;
class EVENotificationStream;
class PySubStream;
class InventoryItem;
class SystemManager;
class PyServiceMgr;
class PyCallStream;
class PyTuple;
class LSCChannel;
class PyAddress;
class PyList;
class PyDict;
class PyPacket;
class PyRep;
class Scan;
class TradeSession;


// DO NOT INHERIT THIS OBJECT!
class Client final
: protected EVEClientSession,
protected EVEPacketDispatcher
{
public:
    Client(PyServiceMgr& services, EVETCPConnection** con);
    Client(const Client& oth) = delete;
    Client(Client&& oth) = delete;
    Client& operator= (const Client& oth) = delete;
    Client& operator= (Client&& oth) = delete;

    virtual ~Client();

    // ========================================================================
    // LIFECYCLE, CORE LOOPS & INITIALIZATION
    // ========================================================================
    bool                    ProcessNet();       // called from main() loop
    void                    ProcessClient();    // called by EntityMgr on 1Hz tic

    void                    InitSession(int32 characterID); // adds clone alpha if none found
    bool                    SelectCharacter(int32 char_id = 0);

    bool                    IsLoaded() const                { return m_loaded; }
    bool                    IsLogin() const                 { return m_login; }
    bool                    IsAFK() const                   { return m_afk; }
    void                    SetAFK(bool set = true)         { m_afk = set; }

    bool                    CanThrow() const                { return m_canThrow; }
    bool                    IsCharCreation() const          { return m_charCreation; }
    void                    CreateChar(bool set)            { m_charCreation = set; }
    void                    AddBindID(uint32 bindID)        { m_bindSet.emplace(bindID); }

    PyServiceMgr&           services() const                { return m_services; }
    SystemManager*          SystemMgr() const               { return m_system; } // should never be null

    // ========================================================================
    // SESSION, ROLES & FLEET ACCESSORS
    // ========================================================================
    bool                    IsValidSession() const          { return m_validSession; }
    ClientSession*          GetSession()                    { return pSession; }
    void                    SendSessionChange();
    void                    UpdateSessionInt(const char* varName, int value);
    void                    UpdateSessionLong(const char* varName, int64 value);

    bool                    IsSessionChange() const         { return m_sessionChangeActive; }
    uint32                  GetSessionChangeTime()          { return m_sessionTimer.GetRemainingTime() / 1000; }
    void                    SetSessionTimer()               { m_sessionChangeActive = true; m_sessionTimer.Start(Player::Timer::Session); }
    void                    SetSessionChange(bool set = false) { m_sessionChangeActive = set; }

    std::string             GetAddress() const              { return pSession->GetCurrentString("address"); }
    std::string             GetLanguageID() const           { return pSession->GetCurrentString("languageID"); }

    int32                   GetUserID() const               { return pSession->GetCurrentInt("userid"); }
    int32                   GetAccountType() const          { return pSession->GetCurrentInt("userType"); }
    int32                   GetGender() const               { return pSession->GetCurrentInt("genderID"); }
    int64                   GetAccountRole() const          { return pSession->GetCurrentLong("role"); }
    int64                   GetClientID() const             { return pSession->GetCurrentLong("user_clientid"); }

    // these below need Session initialized before use (not done during char creation)
    int32                   GetCharacterID() const          { return pSession->GetCurrentInt("charid"); }
    int32                   GetStationID() const            { return pSession->GetCurrentInt("stationid"); }
    int32                   GetStationID2() const           { return pSession->GetCurrentInt("stationid2"); }
    int32                   GetCloneStationID() const       { return pSession->GetCurrentInt("cloneStationID"); }

    // corporation & alliance session data
    int32                   GetCorporationID() const        { return pSession->GetCurrentInt("corpid"); }
    int32                   GetCorpHQ() const               { return pSession->GetCurrentInt("hqID"); }
    int32                   GetAllianceID() const           { return pSession->GetCurrentInt("allianceid"); }
    int32                   GetWarFactionID() const         { return pSession->GetCurrentInt("warfactionid"); }
    int32                   GetCorpAccountKey() const       { return pSession->GetCurrentInt("corpAccountKey"); }
    int64                   GetCorpRole() const             { return pSession->GetCurrentLong("corprole"); }
    int64                   GetRolesAtAll() const           { return pSession->GetCurrentLong("rolesAtAll"); }
    int64                   GetRolesAtBase() const          { return pSession->GetCurrentLong("rolesAtBase"); }
    int64                   GetRolesAtHQ() const            { return pSession->GetCurrentLong("rolesAtHQ"); }
    int64                   GetRolesAtOther() const         { return pSession->GetCurrentLong("rolesAtOther"); }
    void                    UpdateCorpSession(CorpData& data);

    // fleet session data
    int8                    GetFleetRole()                  { return pSession->GetCurrentInt("fleetrole"); }
    bool                    InFleet()                       { return IsFleetID(m_fleet); }
    bool                    IsFleetBoss()                   { return (IsFleetID(m_fleet) ? (GetFleetRole() == Fleet::Role::FleetLeader) : false); }
    bool                    IsFleetBooster()                { return (IsFleetID(m_fleet) ? (GetFleetRole() != Fleet::Booster::No) : false); }
    uint32                  GetFleetID() const              { return m_fleet; }
    int32                   GetWingID() const               { return m_wing; }
    int32                   GetSquadID() const              { return m_squad; }
    void                    UpdateFleetSession(CharFleetData& fleet, bool clear = false);

    // ========================================================================
    // CHARACTER, WALLET, SKILL TIMER & STATION HANGARS
    // ========================================================================
    const char*             GetName() const                 { return (m_char.get() != nullptr ? m_char->name() : "(null)"); }
    void                    SetChar(CharacterRef charRef)   { m_char = charRef; } // only used during char creation and ship state
    CharacterRef            GetChar() const                 { return m_char; }
    std::string             GetCharName()                   { return m_char->itemName(); }
    uint32                  GetCharID()                     { return (m_char.get() != nullptr ? m_char->itemID() : 0); }

    float                   GetCorpTaxRate()                { return (m_char.get() != nullptr ? m_char->corpTaxRate() : 0.0f); }
    float                   GetBounty() const               { return m_char->bounty(); }
    float                   GetSecurityRating() const       { return m_char->GetSecurityRating(); }
    uint32                  GetLoyaltyPoints(uint32 corpID);

    bool                    AddBalance(float amount, uint8 type = Account::CreditType::ISK)
    { return m_char->AlterBalance(amount, type); }
    float                   GetBalance(uint8 type = Account::CreditType::ISK)
    { return m_char->balance(type); }

    void                    SetTrainingEndTime(int64 endTime) { m_skillTimer = endTime; }
    bool                    RecPic() const                  { return m_portrait; }
    void                    SetPicRec(bool set = false)     { m_portrait = set; }

    // station & hangar management
    uint32                  GetLocationID() const           { return m_locationID; }
    uint32                  GetSystemID() const             { return m_systemData.systemID; }
    uint32                  GetConstellationID() const      { return m_systemData.constellationID; }
    uint32                  GetRegionID() const             { return m_systemData.regionID; }
    std::string             GetSystemName() const           { return m_systemData.name; }

    bool                    IsHangarLoaded(uint32 hangarID);
    void                    SetHangarLoaded(uint32 hangarID, bool set = false);
    void                    LoadStationHangar(uint32 stationID);
    void                    AddLoadedHangar(uint32 stationID);
    void                    RemoveStationHangar(uint32 stationID);
    void                    CharNowInStation();
    void                    CharNoLongerInStation();        // clears m_stationData and removes char from station guestList
    void                    MoveItem(uint32 itemID, uint32 location, EVEItemFlags flag);

    // ========================================================================
    // SHIP MANAGEMENT, NAVIGATION & STATE FLAGS
    // ========================================================================
    uint32                  GetShipID() const               { return m_shipId; }
    ShipItemRef             GetShip() const                 { return m_ship; }
    ShipSE*                 GetShipSE()                     { return pShipSE; }
    ShipItemRef             GetPod() const                  { return m_pod; }
    uint32                  GetPodID() const                { return m_pod->itemID(); }

    void                    SetPodItem();
    void                    CreateShipSE();
    void                    SetShip(ShipItemRef shipRef);
    void                    CreateNewPod();
    ShipItemRef             SpawnNewRookieShip(uint32 stationID);
    void                    PickAlternateShip();

    // docking, undocking, boarding & death transitions
    void                    UndockFromStation();
    void                    DockToStation();
    void                    ResetAfterPodded();
    void                    ResetAfterPopped(Vector3d& position); // delete killed ship, reset to pod, add pod to system
    void                    Eject();                        // only called in space
    void                    Board(ShipSE* newShipSE);       // only called when in space
    void                    BoardShip(ShipItemRef newShipRef); // only called when docked
    void                    SetDockStationID(uint32 stationID) { m_dockStationID = stationID; }
    uint32                  GetDockStationID() const        { return m_dockStationID; }
    void                    SetDockPoint(const Vector3d& pt){ m_dockPoint = pt; }
    const Vector3d&         GetDockPoint() const            { return m_dockPoint; }
    // system movement & jumping
    void                    EnterSystem(uint32 systemID);   // GM command only, and only if (bubble == null)
    void                    MoveToLocation(uint32 location, const Vector3d& pt); // accepts null coords -> random moon
    void                    MoveToPosition(const Vector3d& pt);
    void                    StargateJump(int32 fromGate, int32 toGate);
    void                    CynoJump(InventoryItemRef beacon);
    void                    WormholeJump(InventoryItemRef wormhole);
    void                    WarpIn();
    void                    WarpOut();
    void                    JumpInEffect();
    void                    JumpOutEffect(uint32 locationID);
    // destiny, ballpark & state timers
    void                    SetDestiny(const Vector3d& pt, bool update = false);
    void                    SetBallPark();
    void                    CheckBallparkTimer();
    void                    SetCloakTimer(uint32 time = Player::Timer::Default);    // time=0 to disable
    void                    SetInvulTimer(uint32 time = Player::Timer::Default);    // time=0 to disable
    void                    SetUncloakTimer(uint32 time = Player::Timer::Default);  // time=0 to disable
    void                    SetBallParkTimer(uint32 time = Player::Timer::Default); // time=0 to disable
    void                    SetStateTimer(int8 state, uint32 time = Player::Timer::Default); // time=0 to disable
    // ship & client status checks
    bool                    InPod() const                   { return (m_ship->groupID() == EVEDB::invGroups::Capsule); }
    bool                    IsInSpace() const               { return sDataMgr.IsSolarSystem(m_locationID); }
    bool                    IsDocked() const                { return sDataMgr.IsStation(m_locationID); }
    bool                    IsDock() const                  { return (m_clientState == Player::State::Dock); }
    bool                    IsIdle() const                  { return (m_clientState == Player::State::Idle); }
    bool                    IsGateJump() const              { return (m_clientState == Player::State::Jump); }
    bool                    IsDriveJump() const             { return (m_clientState == Player::State::DriveJump); }
    bool                    IsWormholeJump() const          { return (m_clientState == Player::State::WormholeJump); }
    bool                    IsJump() const                  { return ((m_clientState == Player::State::DriveJump) or (m_clientState == Player::State::Jump) or (m_clientState == Player::State::WormholeJump)); }
    bool                    IsBoard() const                 { return (m_clientState == Player::State::Board); }
    // this uses shipSE
    bool                    IsUndock() const                { return pShipSE->GetMoveData()->IsUndocking(); }
    bool                    IsInvul() const                 { return m_invul; }
    void                    SetInvul(bool invul = false);
    bool                    IsUncloak() const               { return m_uncloak; }
    void                    SetUncloak(bool uncloak = false){ m_uncloak = uncloak; }
    bool                    HasBeyonce() const              { return m_beyonce; }
    void                    SetBeyonce(bool beyonce = false){ m_beyonce = beyonce; }
    bool                    IsBubbleWait() const            { return m_bubbleWait; }
    void                    SetBubbleWait(bool wait = false){ m_bubbleWait = wait; }
    bool                    IsSetStateSent() const          { return m_setStateSent; }
    void                    SetStateSent(bool set = false)  { m_setStateSent = set; }
    bool                    IsAutoPilot();
    void                    SetAutoPilot(bool set = false);
    void                    SetShowAll(bool set = false)    { m_showall = set; }
    bool                    IsShowall() const               { return m_showall; }
    void                    SetAutoStop(bool set = false)   { m_autoStop = set; }
    bool                    AutoStop() const                { return m_autoStop; }
    void                    SetAutoAttack(bool set = false) { m_autoAttack = set; }
    bool                    AutoAttack() const              { return m_autoAttack; }
    // jetcan timer
    bool                    IsJetcanAvalible();
    uint32                  JetcanTime()                    { return (m_jetcanTimer.GetRemainingTime() / 1000); }
    void                    StartJetcanTimer()              { m_jetcanTimer.Start(Player::Timer::Jetcan); }
    // ========================================================================
    // DESTINY PACKET QUEUES & COMBAT STATE
    // ========================================================================
    PyRep*                  GetAggressors() const;
    void                    QueueDestinyEvent(PyTuple** multiEvent);
    void                    QueueDestinyUpdates(std::vector<PyTuple*>& updates);
    void                    QueueDestinyUpdate(PyTuple** update, bool DoPackage = false, bool IsSetState = false);
    void                    FlushQueue()                    { _SendQueuedUpdates(); }
    // ========================================================================
    // MESSAGING, LSC CHANNELS & NOTIFICATIONS
    // ========================================================================
    void                    SendErrorMsg(const char* fmt, ...);
    void                    SendErrorMsg(const char* fmt, va_list args);
    void                    SendNotifyMsg(const char* fmt, ...);
    void                    SendNotifyMsg(const char* fmt, va_list args);
    void                    SendInfoModalMsg(const char* fmt, ...);
    void                    SelfChatMessage(const char* fmt, ...);
    void                    SelfEveMail(const char* subject, const char* fmt, ...);
    void                    ChannelJoined(LSCChannel* chan);
    void                    ChannelLeft(LSCChannel* chan);
    void                    SendNotification(const PyAddress& dest, EVENotificationStream& noti, bool seq = true);
    void                    SendNotification(const char* notifyType, const char* idType, PyTuple* payload, bool seq = true);
    void                    SendNotification(const char* notifyType, const char* idType, PyTuple** payload, bool seq = true);
    // ========================================================================
    // SUBSYSTEMS (MISSIONS, SCANNING, TRADE) & SERVER ADMIN
    // ========================================================================
    void                    RemoveMissionItem(uint16 typeID, int32 qty);
    bool                    ContainsTypeQty(uint16 typeID, uint32 qty) const;
    bool                    IsMissionComplete(MissionOffer& data);
    Scan*                   scan()                          { return m_scan; }
    void                    SetScan(Scan* pScan)            { m_scan = pScan; }
    void                    SetScanTimer(uint16 time, bool useProbe = false) { m_scanTimer.Start(time); m_scanProbe = useProbe; }
    void                    SetTradeSession(TradeSession* ts) { m_TS = ts; }
    void                    ClearTradeSession()             { m_TS = nullptr; }
    TradeSession*           GetTradeSession()               { return m_TS; }
    void                    DisconnectClient();
    void                    BanClient();
protected:
    // ========================================================================
    // EVEClientSession INTERFACE
    // ========================================================================
    int16                   GetClientCount();
    int32                   _GetQueuePosition()             { /* hack */ return 1; }
    void                    UpdateSession();
    // ========================================================================
    // EVEClientLogin STATEMACHINE
    // ========================================================================
    bool                    _LoginFail(std::string fail_msg);
    bool                    _VerifyVersion(VersionExchangeClient& version);
    bool                    _VerifyCrypto(CryptoRequestPacket& cr);
    bool                    _VerifyLogin(CryptoChallengePacket& ccp);
    bool                    _VerifyVIPKey(const std::string& vipKey) { /* do nothing */ return true; }
    bool                    _VerifyFuncResult(CryptoHandshakeResult& result);
    // ========================================================================
    // EVEPacketDispatcher INTERFACE
    // ========================================================================
    void                    _SendPingRequest();
    void                    _SendException(const PyAddress& source, int64 callID, MACHONETMSG_TYPE in_response_to, MACHONETERR_TYPE exception_type, PyRep** payload);
    void                    _SendCallReturn(const PyAddress& source, int64 callID, PyResult& rsp);
    void                    _SendPingResponse(const PyAddress& source, int64 callID, int64 receivedTime = 0);
    bool                    Handle_CallReq(PyPacket* packet, PyCallStream* req);
    bool                    Handle_Notify(PyPacket* packet);
    bool                    Handle_PingReq(PyPacket* packet) { _SendPingResponse(packet->dest, packet->source.callID, packet->receivedTime); return true; }
    bool                    Handle_PingRsp(PyPacket* packet) { /* do nothing */ return true; }
private:
    // ========================================================================
    // INTERNAL HELPERS
    // ========================================================================
    void                    UpdateNewShip();                // calls destiny update methods (must be in system first)
    void                    CheckShipRef(ShipItemRef newShipRef); // can throw
    void                    ExecuteJump();
    void                    ExecuteDriveJump();
    void                    ExecuteWormholeJump();
    void                    DestroyShipSE();
    void                    _SendQueuedUpdates();
    std::string             GetStateName(int8 state);
    // ========================================================================
    // DATA MEMBERS (Ordered 8 -> 4 -> 1 byte for zero internal padding)
    // ========================================================================
    // --- 8-byte aligned group ---
    ServiceDB               m_sDB;
    StationData             m_stationData;
    SystemData              m_systemData;
    ShipItemRef             m_ship;
    ShipItemRef             m_pod;
    CharacterRef            m_char;
    PyServiceMgr&           m_services;
    Scan*                   m_scan;
    ShipSE*                 pShipSE;
    TradeSession*           m_TS;
    ClientSession*          pSession;
    SystemManager*          m_system;       // we do not own this
    PyPacket*               pPacket;
    PyList*                 m_EventQueue;   // we own these (OnMultiEvent)
    PyList*                 m_UpdateQueue;  // we own these (DoDestinyAction)
    Vector3d                m_movePoint;    // jump vector / undock heading
    Vector3d                m_dockPoint;    // dock location in space (absolute)
    std::set<LSCChannel*>   m_channels;     // we do not own these
    std::set<uint32>        m_bindSet;
    std::map<uint32, bool>  m_stationHangars; // hangarID/bool
    std::map<uint32, uint32> m_lpMap;       // corpID/points
    int64                   m_skillTimer;
    double                  m_profileStartTime;
    // --- 4-byte aligned group ---
    Timer                   m_stateTimer;       // state timer to consolidate timers
    Timer                   m_pingTimer;
    Timer                   m_scanTimer;        // delay scan results based on skills/items
    Timer                   m_cloakTimer;       // jump cloak timer
    Timer                   m_uncloakTimer;     // post-cloak action delay timer
    Timer                   m_invulTimer;
    Timer                   m_fleetTimer;       // apply fleet boost on undock/jump
    Timer                   m_clientTimer;      // process ticks for docked players
    Timer                   m_jetcanTimer;      // delay jetcan creation (3min default)
    Timer                   m_logoutTimer;      // hold client object until WarpOut finishes
    Timer                   m_sessionTimer;     // prevent rapid session changes
    Timer                   m_ballparkTimer;    // delay SetState data properly
    int32                   m_wing;
    int32                   m_squad;
    int32                   m_toGateID;
    uint32                  m_fleet;
    uint32                  m_shipId;
    uint32                  m_locationID;
    uint32                  m_moveSystemID;     // holder for jumping to systemID (timer based)
    uint32                  m_dockStationID;    // holder for docking to stationID (timer based)
    uint32                  m_nextNotifySequence;
    // --- 1-byte aligned group ---
    int8                    m_clientState;
    bool                    m_afk;              // for map info (pilots docked and active)
    bool                    m_invul;
    bool                    m_login;
    bool                    m_loaded;
    bool                    m_beyonce;
    bool                    m_uncloak;
    bool                    m_showall;          // show all dynamics on ship scanner (ROLE_GMH)
    bool                    m_autoStop;         // auto-stop modules when target attrib is full
    bool                    m_autoAttack;       // drones auto-attack on new target
    bool                    m_packaged;         // package updates into PackagedAction list
    bool                    m_portrait;         // verify new char pic received
    bool                    m_scanProbe;        // scanning with probes
    bool                    m_bubbleWait;       // expect another update for this bubble on this tic
    bool                    m_setStateSent;
    bool                    m_sessionChangeActive; // delay actions requiring destiny updates
    bool                    m_canThrow;
    bool                    m_validSession;
    bool                    m_charCreation;
};

/*
        if si.zoneid:
            sessValues['worldspaceid'] = si.zoneid
            sessValues['stationid2'] = si.stationID
            station = self.stationSvc.GetStation(si.stationID)
            sessValues['solarsystemid2'] = station.solarSystemID
        elif si.stationID:
            sessValues['stationid'] = si.stationID
            sessValues['stationid2'] = si.stationID
            sessValues['worldspaceid'] = si.stationID
            station = self.stationSvc.GetStation(si.stationID)
            sessValues['solarsystemid2'] = station.solarSystemID
        elif si.solarSystemID:
            sessValues['solarsystemid'] = si.solarSystemID
            sessValues['solarsystemid2'] = si.solarSystemID
        if 'solarsystemid2' in sessValues:
            if sessValues['solarsystemid2'] is not None:
                primeditems = sm.services['i2'].__primeditems__
                if sessValues['solarsystemid2'] in primeditems:
                    sessValues['constellationid'] = primeditems[sessValues['solarsystemid2']].locationID
                    sessValues['regionid'] = primeditems[sessValues['constellationid']].locationID
        return sessValues
    */
