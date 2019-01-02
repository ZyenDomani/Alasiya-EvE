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
    Author:        Zhur
    Updates:    Allan
*/


#ifndef EVE_CLIENT_H
#define EVE_CLIENT_H


#include "ClientSession.h"

#include "character/Character.h"
#include "inventory/InventoryItem.h"
#include "inventory/ItemRef.h"
#include "packets/Crypto.h"
#include "packets/Destiny.h"
#include "packets/LSCPkts.h"
#include "ship/Ship.h"
#include "ship/modules/ModuleManager.h"
#include "system/SystemEntity.h"
#include "system/SystemGPoint.h"

#include "../eve-common/EVE_Missions.h"

//  -updated 18Dec16
enum ClientTimers {
    DefaultTimer     = 1000,
    BoardTimer       = 600,
    JumpTimer        = 300,
    UndockTimer      = 500,     // used to delay sending Destiny::State (client error fix)
    DockingTimer     = 1000,    // Timer to delay docking (as on live)
    JumpingTimer     = 4000,    // Timer to delay jumping
    MovingTimer      = 1000,
    ScanningTimer    = 10000,   // used to delay scan results based on skills, items, and other shit
    KilledTimer      = 800,    // used to reset ego after killed or otherwise changing ships
    ProcTimer        = 1000,    // used to give process ticks to docked players (for skill updates...tick cycle consumption negligible)
    JetcanTimer      = 180000,  // used to delay jetcan creation.  3min default
    LogoutTimer      = 10000,    // used to hold client object until WarpOut finishes
    LoginTimer       = 2500,
    SessionTimer     = 10000,   // used to prevent multiple session changes from occuring too fast
    DockInvul        = 3000,
    FleetTimer       = 1500,
    JumpInvul        = 5000,
    WarpOutInvul     = 5000,
    WarpInInvul      = 18000,   // increased from 10s
    UndockInvul      = 20000,
    RestoringInvul   = 60000,
    JumpCloak        = 30000,
    LoginCloak       = 20000
};

enum ClientState {
    csIdle = 1,
    csJump = 2,
    csDock = 3,
    csUndock = 4,
    csKilled = 5,
    csLogout = 6,
    csBoard  = 7,
    csLogin  = 8
};

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

//DO NOT INHERIT THIS OBJECT!
class Client
: protected EVEClientSession,
  protected EVEPacketDispatcher
{
public:
    Client(PyServiceMgr &services, EVETCPConnection** con);
    virtual ~Client();

    bool                    ProcessNet();
    void                    ProcessClient();

    bool                    IsLoaded()                  { return m_loaded; }

    PyServiceMgr&           services() const            { return m_services; }
    SystemManager*          SystemMgr() const           { return m_system; }
    bool                    IsClient() const            { return true; }
    const char*             GetName() const             { return (m_char ? m_char->itemName().c_str() : "(null)"); }

    bool                    IsAFK()                     { return m_afk; }
    void                    SetAFK(bool set=true)       { m_afk = set; }


    /********************************************************************/
    /* Session values                                                   */
    /********************************************************************/
    std::string GetAddress() const                      { return mSession.GetCurrentString( "address" ); }
    std::string GetLanguageID() const                   { return mSession.GetCurrentString( "languageID" ); }

    uint32 GetAccountType() const                       { return mSession.GetCurrentInt( "userType" ); }
    int64 GetAccountRole() const                        { return mSession.GetCurrentLong( "role" ); }
    int64 GetClientID() const                           { return mSession.GetCurrentLong( "clientID" ); }
    uint32 GetUserID() const                            { return mSession.GetCurrentInt( "userid" ); }
    int64 GetSessionID()                                { return mSession.GetCurrentLong( "sessionID" ); }

    uint32 GetCharacterID() const                       { return mSession.GetCurrentInt( "charid" ); }
    std::string GetCharacterName() const                { return mSession.GetCurrentString( "charname" ); }
    uint32 GetStationID() const                         { return mSession.GetCurrentInt( "stationid" ); }
    uint32 GetStationID2() const                        { return mSession.GetCurrentInt( "stationid2" ); }
    uint32 GetCloneStationID() const                    { return mSession.GetCurrentInt( "cloneStationID" ); }

    double GetCorpTaxRate()                             { return m_char->corpTaxRate(); }
    uint32 GetCorporationID() const                     { return mSession.GetCurrentInt( "corpid" ); }
    uint32 GetCorpHQ() const                            { return mSession.GetCurrentInt( "hqID" ); }
    int32 GetAllianceID() const                         { return mSession.GetCurrentInt( "allianceid" ); }
    int32 GetWarFactionID() const                       { return mSession.GetCurrentInt( "warfactionid" ); }
    int32 GetCorpAccountKey() const                     { return mSession.GetCurrentInt( "corpAccountKey" ); }
    // corporation management-type roles (manager, officer, trader)  also has container roles
    int64 GetCorpRole() const                           { return mSession.GetCurrentLong( "corprole" ); }
    // access roles everywhere.  is joined with other access roles
    int64 GetRolesAtAll() const                         { return mSession.GetCurrentLong( "rolesAtAll" ); }
    // access roles at base defined for this char. overrides hq if same location
    int64 GetRolesAtBase() const                        { return mSession.GetCurrentLong( "rolesAtBase" ); }
    // access roles at corp HQ.
    int64 GetRolesAtHQ() const                          { return mSession.GetCurrentLong( "rolesAtHQ" ); }
    // access roles for non-station containers with corp hangars
    int64 GetRolesAtOther() const                       { return mSession.GetCurrentLong( "rolesAtOther" ); }

    // fleet data
    bool InFleet()                                      { return IsFleet(m_fleet); }
    bool IsFleetBoss()                                  { return (IsFleet(m_fleet) ? ((GetFleetRole() == Fleet::Role::FleetLeader) ? true : false) : false); }
    bool IsFleetBooster()                               { return (IsFleet(m_fleet) ? ((GetFleetRole() == Fleet::Booster::No) ? false : true) : false); }

    int32 GetFleetID() const                            { return m_fleet; }
    int32 GetWingID() const                             { return m_wing; }
    int32 GetSquadID() const                            { return m_squad; }
    int8 GetFleetRole()                                 { return mSession.GetCurrentInt("fleetrole"); }

    uint32 GetShipID() const                            { return m_shipId; }
    uint32 GetLocationID() const                        { return m_locationID; }
    uint32 GetSystemID() const                          { return m_SystemData.systemID; }
    uint32 GetConstellationID() const                   { return m_SystemData.constellationID; }
    uint32 GetRegionID() const                          { return m_SystemData.regionID; }

    //  public functions to update client session when char's roles are changed
    void UpdateCorpSession(CorpData& data);
    void UpdateFleetSession(CharFleetData& fleet);

    // character data
    void SetChar(CharacterRef charRef)                  { m_char = charRef; }   // only used in char creation
    CharacterRef GetChar() const                        { return m_char; }
    ShipItemRef GetShip() const                         { return m_ship; }
    Ship* GetShipSE()                                   { return pShipSE; }
    ShipItemRef GetPod() const                          { return m_pod; }
    uint32 GetPodID() const                             { return m_pod->itemID(); }
    double GetBounty() const                            { return m_char->bounty(); }
    double GetSecurityRating() const                    { return m_char->GetSecurityRating(); }
    //check all these and update to AccountService::TransferFunds() where applicable
    bool AddBalance(double amount, uint8 type=Account::CreditType::ISK) { return m_char->AlterBalance(amount, type); }
    double GetBalance(uint8 type=Account::CreditType::ISK) { return m_char->balance(type); }

    std::string GetSystemName() const                   { return m_SystemData.name; }

    uint32 GetLoyaltyPoints(uint32 corpID);

    // ship functions
    void SetPodItem();
    void CreateShipSE();
    void SetShip(ShipItemRef shipRef);
    void CreateNewPod();
    void UndockFromStation();
    void DockToStation();
    void PickAlternateShip();
    void ResetAfterPodded();
    void ResetAfterPopped(GPoint& position);  //  delete killed ship, reset player to pod, add pod to system
    void Eject();       // only called in space
    void Board(Ship* newShipSE); // only called when in space
    void BoardShip(ShipItemRef newShipRef); // only called when docked
private:
    void UpdateNewShip();     //  calls destiny update methods
    void CheckShipRef(ShipItemRef newShipRef);  // called by Board methods

public:
    // misc char functions
    void WarpIn();
    void WarpOut();
    void EnterSystem(uint32 systemID);     // only called by gm command, and only if (bubble == null)
    void MoveToLocation(uint32 location, const GPoint &pt);
    void MoveToPosition(const GPoint &pt);
    void MoveItem(uint32 itemID, uint32 location, EVEItemFlags flag);
    void SetInvulTimer(uint32 time=ClientTimers::DefaultTimer);
    void SetClientTimer(ClientState state, uint32 time=ClientTimers::DefaultTimer);
    void SetDestiny(const GPoint& pt, bool count = false);
    void UpdateSkillTraining();
    ShipItemRef SpawnNewRookieShip();
    void LoadStationHangar(uint32 stationID);
    void AddStationHangar(uint32 stationID);
    void RemoveStationHangar(uint32 stationID);

    bool SelectCharacter( uint32 char_id=0);
    bool IsHangarLoaded(uint32 stationID);

    PyRep* GetInfoWindowDataForChar(Client *pClient);

    double GetPropulsionStrength() const;

    bool LaunchDrone(InventoryItemRef drone);

    //destiny stuff...
    void SetDockStationID(uint32 stationID)             { m_dockStationID = stationID; };
    void SetDockPoint(GPoint &pt)                       { m_dockPoint = pt; }
    uint32 GetDockStationID()                           { return m_dockStationID; };
    GPoint GetDockPoint()                               { return m_dockPoint; }
    bool InPod()                                        { return (m_ship->groupID() == EVEDB::invGroups::Capsule); }
    bool IsInSpace()                                    { return IsSolarSystem(m_locationID); }
    bool IsDocked()                                     { return IsStation(m_locationID); }
    bool IsDock()                                       { return (m_clientState == ClientState::csDock); }
    bool IsJump()                                       { return (m_clientState == ClientState::csJump); }
    bool IsBoard()                                      { return (m_clientState == ClientState::csBoard); }
    bool IsInvul()                                      { return m_invul; }
    bool IsLogin()                                      { return m_login; }
    bool IsUndock()                                     { return m_undock; }
    bool HasBeyonce()                                   { return m_beyonce; }
    bool IsBubbleWait()                                 { return m_bubbleWait; }
    bool IsSetStateSent()                               { return m_setStateSent; }
    bool IsSessionChange()                              { return m_sessionChangeActive; }

    void SetInvul(bool invul=false)                     { m_invul = invul; }
    void SetUndock(bool undock=false)                   { m_undock = undock; }
    void SetBeyonce(bool beyonce=false)                 { m_beyonce = beyonce; }
    void SetBubbleWait(bool wait=false)                 { m_bubbleWait = wait; }
    void SetStateSent(bool set=false)                   { m_setStateSent = set; }
    void SetSessionTimer()                              { SetSessionChange(true); m_sessionTimer.Start(ClientTimers::SessionTimer); }
    void SetSessionChange(bool set=false)               { m_sessionChangeActive = set; }
    void SetBallPark();
    void SetJumpTimers();
    void StargateJump(uint32 fromGate, uint32 toGate);

    bool IsAutoPilot()                                  { return m_autoPilot; }
    void SetAutoPilot(bool set=false);

    //jetcan timer
    bool IsJetcanAvalible();
    // return time remaining in seconds
    uint32 JetcanTime()                                 { return (m_jetcanTimer.GetRemainingTime() /1000); }
    void StartJetcanTimer()                             { m_jetcanTimer.Start(ClientTimers::JetcanTimer); }

    void SetShowAll(bool set=false)                     { m_showall = set; }
    bool IsShowall()                                    { return m_showall; }

    //messages and LSC
    // error requires dismissal (click 'ok')
    void SendErrorMsg(const char *fmt, ...);
    void SendErrorMsg(const char *fmt, va_list args);
    // notify self-removes after delay
    void SendNotifyMsg(const char *fmt, ...);
    void SendNotifyMsg(const char *fmt, va_list args);
    // info requires dismissal (click 'ok')
    void SendInfoModalMsg(const char *fmt, ...);
    void SelfChatMessage(const char *fmt, ...);
    void SelfEveMail(const char *subject, const char *fmt, ...);
    void ChannelJoined(LSCChannel *chan);
    void ChannelLeft(LSCChannel *chan);
    void UpdateSessionInt( const char *sessionType, int value );

    PyRep *GetAggressors() const;
    void QueueDestinyUpdate(PyTuple** update, bool DoPackage=false, bool IsSetState=false);
    void QueueDestinyEvent(PyTuple** multiEvent);
    void FlushQueue();

    bool ApplyDamage(Damage &d);
    void Killed(Damage &fatal_blow);

    //  mission
    void RemoveMissionItem(uint16 typeID, uint32 qty);
    bool ContainsTypeQty(uint16 typeID, uint32 qty) const;
    bool IsMissionComplete(MissionOffer& data);

    //  scan
    Scan* scan()                                        { return m_scan; }
    void SetScan(Scan* pScan)                           { m_scan = pScan; }
    // set scan timer in ms  this is used in scan.cpp after time calc's are done
    void SetScanTimer(uint16 time, bool useProbe=false) { m_scanTimer.Start(time);  m_scanProbe = useProbe; }

    //  trade
    void SetTradeSession(TradeSession* ts)              { m_TS = ts; }
    void ClearTradeSession()                            { m_TS = nullptr; }
    TradeSession* GetTradeSession()                     { return m_TS; }

    // character notification messages
    void OnCharNowInStation();
    void OnCharNoLongerInStation();

    // portrait stuff....
    bool RecPic()                                       { return m_portrait; }
    void SetPicRec(bool set=false)                      { m_portrait = set; }

    /********************************************************************/
    /* Server Administration Interface                                  */
    /********************************************************************/
    void DisconnectClient();
    void BanClient();

protected:
    Scan* m_scan;
    ServiceDB m_sDB;
    SystemData m_SystemData;
    ShipItemRef m_ship;
    ShipItemRef m_pod;
    StationData m_StationData;
    CharacterRef m_char;
    PyServiceMgr& m_services;
    SystemGPoint m_SGP;     // interface to my variable 3-d point generating system  (which isnt finished yet... -allan)
    Ship* pShipSE;
    TradeSession* m_TS;
    ClientSession mSession;
    SystemManager* m_system;    //we do not own this

    //void _AwardBounty(SystemEntity *who);
    /** @todo finish this... */
    void _DropLoot(uint32 groupID, uint32 owner, uint32 locationID);
    void InitSession( uint32 characterID  );
    void ExecuteJump();
    void DestroyShipSE();

    bool m_afk;             // for map info (pilots docked and active)
    bool m_invul;
    bool m_login;
    bool m_undock;
    bool m_loaded;
    bool m_beyonce;
    bool m_showall;         // boolean for showing all dynamics in system on ships scanner (ROLE_GMH)
    bool m_packaged;        // used to correctly package updates into a PackagedAction list
    bool m_portrait;        // used to verify new char pic received
    bool m_autoPilot;       // set true for using autopilot.
    bool m_scanProbe;       // scanning with probes
    bool m_bubbleWait;
    bool m_setStateSent;
    bool m_sessionChangeActive; // used to delay actions requiring destiny updates

    int32 m_wing;
    int32 m_squad;

    uint32 m_fleet;
    uint32 m_shipId;
    uint32 m_toGate;
    uint32 m_locationID;
    uint32 m_moveSystemID;  // holder for jumping to 'systemID'.    timer based.
    uint32 m_dockStationID; // holder for docking to 'stationID'.  timer based.

    Timer m_stateTimer;      // state timer to consolidate timers
    Timer m_jumpTimer;       // this is to properly send SetState data after a delay (cant do it correctly otherwise)
    Timer m_pingTimer;
    Timer m_scanTimer;       // used to delay scan results based on skills, items, and other shit
    Timer m_cloakTimer;
    Timer m_invulTimer;
    Timer m_fleetTimer;      // used to apply fleet boost on undock and jump when applicable
    Timer m_clientTimer;     // used to give process ticks to docked players (for skill updates...tick cycle consumption negligible)
    Timer m_jetcanTimer;     // used to delay jetcan creation.  3min default
    Timer m_logoutTimer;     // used to hold client object until WarpOut finishes
    Timer m_sessionTimer;    // used to prevent multiple session changes from occuring too fast

    GPoint m_movePoint;
    GPoint m_dockPoint;

    std::set<LSCChannel*>   m_channels;    //we do not own these.
    std::map<uint32, bool>  m_hangarLoaded;

    double                  m_timeEndTrain;

    ClientState             m_clientState;

    /********************************************************************/
    /* EVEClientSession interface                                       */
    /********************************************************************/
    void _GetVersion( VersionExchangeServer& version );
    uint32 _GetUserCount();
    uint32 _GetQueuePosition()                          { /* hack */ return 1; }

    /********************************************************************/
    /* EVEClientLogin statemachine                                      */
    /********************************************************************/
    bool _LoginFail(std::string fail_msg);
    bool _VerifyVersion( VersionExchangeClient& version );
    bool _VerifyCrypto( CryptoRequestPacket& cr );
    bool _VerifyLogin( CryptoChallengePacket& ccp );
    bool _VerifyVIPKey( const std::string& vipKey )     { /* do nothing */ return true; }
    bool _VerifyFuncResult( CryptoHandshakeResult& result );

    /********************************************************************/
    /* EVEPacketDispatcher interface                                    */
    /********************************************************************/
public:
    void SendSessionChange();
    void SendNotification(const PyAddress &dest, EVENotificationStream &noti, bool seq=true);
    void SendNotification(const char *notifyType, const char *idType, PyTuple *payload, bool seq=true);
    void SendNotification(const char *notifyType, const char *idType, PyTuple **payload, bool seq=true);

    // this is to check/enable Python Throw keyword, to avoid throws/segfault when not applicable
    bool CanThrow()                                     { return m_canThrow; }

private:
    bool m_canThrow;

protected:
    void _SendPingRequest();
    void _UpdateSession();
    void _SendException( const PyAddress& source, int64 callID, MACHONETMSG_TYPE in_response_to, MACHONETERR_TYPE exception_type, PyRep** payload );
    void _SendCallReturn( const PyAddress& source, int64 callID, int64 clientID, PyRep** return_value, const char* channel = 0 );
    void _SendPingResponse( const PyAddress& source, int64 callID );

    bool Handle_CallReq( PyPacket* packet, PyCallStream& req );
    bool Handle_Notify( PyPacket* packet );
    bool Handle_PingReq( PyPacket* packet )             { _SendPingResponse( packet->dest, packet->source.callID ); return true; }
    bool Handle_PingRsp( PyPacket* packet )             { /* do nothing */ return true; }

private:
    //queues for destiny updates:
    PyList* m_destinyEventQueue;    //we own these. These are events as used in OnMultiEvent
    PyList* m_destinyUpdateQueue;    //we own these. They are the `update` which go into DoDestinyAction
    void _SendQueuedUpdates();

    uint32 m_nextNotifySequence = 0;

    std::map<uint32, uint32>    m_lpMap;    // corpID/points

    /************************************************************************/
    /* new system for MultiEvents      (fix for docked pilot reporting)     */
    /************************************************************************/
    bool ScatterEvent(const char* event_name, PyRep* packet);

    bool DoDestinyUpdate();
    std::list<PyTuple*> mDogmaMessages;

    std::string GetStateName(ClientState state);
};

#endif
