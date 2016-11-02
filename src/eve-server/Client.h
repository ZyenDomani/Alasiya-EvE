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

//#include "eve-server.h"
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
    typedef enum {
        msIdle,
        msJump,
        msUndock
    } _MoveState;

    Client(PyServiceMgr &services, EVETCPConnection** con);
    virtual ~Client();

    bool                    ProcessNet();
    void                    ProcessClient();

    PyServiceMgr&           services() const            { return m_services; }
    SystemManager*          SystemMgr() const           { return m_system; }
    bool                    IsClient() const            { return true; }
    const char*             GetName() const             { return (m_char ? m_char->itemName().c_str() : "(null)"); }


    /********************************************************************/
    /* Session values                                                   */
    /********************************************************************/
    std::string GetAddress() const                      { return mSession.GetCurrentString( "address" ); }
    std::string GetLanguageID() const                   { return mSession.GetCurrentString( "languageID" ); }

    uint32 GetAccountType() const                       { return mSession.GetCurrentInt( "userType" ); }
    uint64 GetAccountRole() const                       { return mSession.GetCurrentULong( "role" ); }
    uint32 GetClientID() const                          { return mSession.GetCurrentInt( "clientid" ); }
    uint32 GetUserID() const                            { return mSession.GetCurrentInt( "userid" ); }
    int64 GetSessionID()                                { return mSession.GetCurrentLong( "sessionID" ); }

    uint32 GetCharacterID() const                       { return mSession.GetCurrentInt( "charid" ); }
    std::string GetCharacterName() const                { return mSession.GetCurrentString( "charname" ); }
    uint32 GetShipID() const                            { return m_shipId; }
    uint32 GetCorporationID() const                     { return mSession.GetCurrentInt( "corpid" ); }
    uint32 GetLocationID() const                        { return m_locationID; }
    uint32 GetStationID() const                         { return mSession.GetCurrentInt( "stationid" ); }
    uint32 GetStationID2() const                        { return mSession.GetCurrentInt( "stationid2" ); }
    uint32 GetSystemID() const                          { return m_SystemData.systemID; }
    uint32 GetConstellationID() const                   { return m_SystemData.constellationID; }
    uint32 GetRegionID() const                          { return m_SystemData.regionID; }
    uint32 GetCloneStationID() const                    { return mSession.GetCurrentInt( "cloneStationID" ); }

    uint32 GetCorpHQ() const                            { return mSession.GetCurrentInt( "hqID" ); }
    int32 GetCorpAccountKey() const                     { return mSession.GetCurrentInt( "corpAccountKey" ); }
    uint64 GetCorpRole() const                          { return mSession.GetCurrentULong( "corpRole" ); }
    uint64 GetRolesAtAll() const                        { return mSession.GetCurrentULong( "rolesAtAll" ); }
    uint64 GetRolesAtBase() const                       { return mSession.GetCurrentULong( "rolesAtBase" ); }
    uint64 GetRolesAtHQ() const                         { return mSession.GetCurrentULong( "rolesAtHQ" ); }
    uint64 GetRolesAtOther() const                      { return mSession.GetCurrentULong( "rolesAtOther" ); }

    uint32 GetGangRole() const                          { return mSession.GetCurrentInt( "gangrole" ); }
    uint8 GetFleetRole() const                          { return mSession.GetCurrentInt( "fleetrole" ); }
    /** @todo need to add gang and fleet session data here */

    //  public functions to update client session when char's roles are changed
    void UpdateCorpSession(Character* pChar);
    void UpdateFleetSession(Character* pChar);

    // character data
    void SetChar(CharacterRef charRef)                  { m_char = charRef; }   // only used in char creation
    CharacterRef GetChar() const                        { return m_char; }
    ShipItemRef GetShip() const                         { return m_ship; }
    SystemEntity* GetShipSE()                           { return pShipSE; }
    ShipItemRef GetPod() const                          { return m_pod; }
    uint32 GetPodID() const                             { return m_char->capsuleID(); }
    uint32 GetAllianceID() const                        { return m_char->allianceID(); }
    uint32 GetWarFactionID() const                      { return m_char->warFactionID(); }
    double GetBounty() const                            { return m_char->bounty(); }
    double GetSecurityRating() const                    { return m_char->GetSecurityRating(); }
    double GetBalance() const                           { return m_char->balance(); }
    double GetAurBalance() const                        { return m_char->aurBalance(); }

    std::string GetSystemName() const                   { return m_SystemData.name; }

    void SetPodItem();
    void CreateShipSE();

    // misc char functions
    void WarpIn();
    void WarpOut();
    void SetJumpTimers();
    void SetShip(ShipItemRef shipRef);
    void CreateNewPod();
    void PickAlternateShip();
    void ResetAfterPodded();
    void BoardShip(ShipItemRef newShipRef);
    void UndockFromStation();
    void DockToStation();
    void EnterSystem(uint32 systemID);     // only called by gm command, and only if (bubble == null)
    void MoveToLocation(uint32 location, const GPoint &pt);
    void MoveToPosition(const GPoint &pt);
    void MoveItem(uint32 itemID, uint32 location, EVEItemFlags flag);
    void SetDestiny(bool count=false);
    void JoinCorporationUpdate(uint32 corp_id=0);
    void SavePosition();
    void SaveAllToDatabase();
    void UpdateSkillTraining();
	void SpawnNewRookieShip();
    void LoadStationHangar(uint32 stationID);
    void AddStationHangar(uint32 stationID);
    void RemoveStationHangar(uint32 stationID);

    bool AddBalance(double amount);
    bool SelectCharacter( uint32 char_id=0);
    bool IsHangarLoaded(uint32 stationID);

    PyRep* GetInfoWindowDataForChar(Client *pClient);

    double GetPropulsionStrength() const;

    bool LaunchDrone(InventoryItemRef drone);

    //destiny stuff...
    void SetDockStationID(uint32 stationID)             { m_dockStationID = stationID; };
    void SetDockPoint(GPoint &pt)                       { m_dockPoint = pt; }
    void StartDockTimer()                               { m_dockTimer.Start(sConfig.server.StationDockDelay); } // default @ 3sec
    uint32 GetDockStationID()                           { return m_dockStationID; };
    GPoint GetDockPoint()                               { return m_dockPoint; }
    bool InPod()                                        { return (m_ship->groupID() == EVEDB::invGroups::Capsule ? true : false); }
    bool IsInSpace()                                    { return (IsSolarSystem(m_locationID) ? true : false); }
    bool IsDocked()                                     { return (IsStation(m_locationID) ? true : false); }
    bool IsJump()                                       { return (m_moveState == msJump ? true : false); }
    bool IsInvul()                                      { return m_invul; }
    bool IsLogin()                                      { return m_login; }
    bool IsUndock()                                     { return m_undock; }
    bool HasBeyonce()                                   { return m_beyonce; }
    bool IsBubbleWait()                                 { return m_bubbleWait; }
    bool IsSetStateSent()                               { return m_setStateSent; }
    bool IsSessionChange()                              { return m_sessionChangeActive; }

    void SetJump(_MoveState state = msIdle)             { m_moveState = state; }
    void SetInvul(bool invul=false)                     { m_invul = invul; }
    void SetUndock(bool undock=false)                   { m_undock = undock; }
    void SetBeyonce(bool beyonce=false)                 { m_beyonce = beyonce; }
    void SetBubbleWait(bool wait=false)                 { m_bubbleWait = wait; }
    void SetStateSent(bool set=false)                   { m_setStateSent = set; }
    void SetSessionTimer(uint32 time=10000)             { SetSessionChange(true); m_sessionTimer.Start(time); }
    void SetSessionChange(bool set=false)               { m_sessionChangeActive = set; }
    void SetBallPark();
    void SetAutoPilot(bool=false);
    void StargateJump(uint32 fromGate, uint32 toGate);
    void StartKilledTimer();

    //jetcan timer
    bool IsJetcanAvalible();
    uint32 JetcanTime()                                 { return m_jetcanTimer.GetRemainingTime(); }
    void StartJetcanTimer()                             { m_jetcanTimer.Start(180000); }

    //messages and LSC
    void SendErrorMsg(const char *fmt, ...);
    void SendErrorMsg(const char *fmt, va_list args);
    void SendNotifyMsg(const char *fmt, ...);
    void SendNotifyMsg(const char *fmt, va_list args);
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

    //  scan
    Scan* scan()                                        { return m_scan; }
    void SetScan(Scan* pScan)                           { m_scan = pScan; }
    // set scan timer in ms  this is used in scan.cpp after time calc's are done
    void SetScanTimer(uint32 time)                      { m_scanTimer.Start(time); }

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
    SystemEntity* pShipSE;
    TradeSession* m_TS;
    ClientSession mSession;
    SystemManager* m_system;    //we do not own this

    //void _AwardBounty(SystemEntity *who);
    void _DropLoot(uint32 groupID, uint32 owner, uint32 locationID);
    void InitSession( uint32 characterID  );
    void _ExecuteJump();
    void DestroyShipSE();

    bool m_invul;
    bool m_login;
    bool m_undock;
    bool m_beyonce;
    bool m_packaged;        // used to correctly package updates into a PackagedAction list
    bool m_portrait;        // used to verify new char pic received
    bool m_autoPilot;       // set true for using autopilot.
    bool m_bubbleWait;
    bool m_setStateSent;
    bool m_sessionChangeActive; // used to delay actions requiring destiny updates

    uint32 m_shipId;
    uint32 m_toGate;
    uint32 m_locationID;
    uint32 m_moveSystemID;  // holder for jumping to 'systemID'.    timer based.
    uint32 m_dockStationID; // holder for docking to 'stationID'.  timer based.

    Timer m_dockTimer;      // Timer to delay docking (as on live)
    Timer m_jumpTimer;
    Timer m_moveTimer;
    Timer m_pingTimer;
    Timer m_scanTimer;       // used to delay scan results based on skills, items, and other shit
    Timer m_cloakTimer;
    Timer m_invulTimer;
    Timer m_clientTimer;     // used to give process ticks to docked players (for skill updates...tick cycle consumption negligible)
    Timer m_jetcanTimer;     // used to delay jetcan creation.  3min default
    Timer m_killedTimer;     // used to reset destiny set state after killed or otherwise changing ships
    Timer m_logoutTimer;     // used to hold client object until WarpOut finishes
    Timer m_sessionTimer;    // used to prevent multiple session changes from occuring too fast

    GPoint m_movePoint;
    GPoint m_dockPoint;

    std::set<LSCChannel*>   m_channels;    //we do not own these.
    std::map<uint32, bool>  m_hangarLoaded;

    EvilNumber              m_timeEndTrain;

    void                    _postMove(_MoveState type, uint32 wait_ms=500);
    _MoveState              m_moveState;

    /********************************************************************/
    /* EVEClientSession interface                                       */
    /********************************************************************/
    void _GetVersion( VersionExchangeServer& version );
    uint32 _GetUserCount();
    uint32 _GetQueuePosition()                          { /* hack */ return 1; }

    /********************************************************************/
    /* EVEClientLogin statemachine                                      */
    /********************************************************************/
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
    void SendNotification(const char *notifyType, const char *idType, PyTuple **payload, bool seq=true);

    // this is to check/enable Python Throw keyword, to avoid throws/segfault when not applicable
    bool CanThrow()                                     { return m_canThrow; }
private:
    bool m_canThrow = false;

protected:
    void _SendPingRequest();
    void _UpdateSession( const CharacterConstRef& character );
    void _SendException( const PyAddress& source, uint64 callID, MACHONETMSG_TYPE in_response_to, MACHONETERR_TYPE exception_type, PyRep** payload );
    void _SendCallReturn( const PyAddress& source, uint64 callID, uint32 clientID, PyRep** return_value, const char* channel = NULL );
    void _SendPingResponse( const PyAddress& source, uint64 callID );

    bool Handle_CallReq( PyPacket* packet, PyCallStream& req );
    bool Handle_Notify( PyPacket* packet );
    bool Handle_PingReq( PyPacket* packet )             { _SendPingResponse( packet->dest, packet->source.callID ); return true; }
    bool Handle_PingRsp( PyPacket* packet )             { /* do nothing */ return true; }

private:
    //queues for destiny updates:
    PyList* m_destinyEventQueue;    //we own these. These are events as used in OnMultiEvent
    PyList* m_destinyUpdateQueue;    //we own these. They are the `update` which go into DoDestinyAction
    void _SendQueuedUpdates();

    uint32 m_nextNotifySequence;

    /************************************************************************/
    /* new system for MultiEvents      (fix for docked pilot reporting)     */
    /************************************************************************/
    bool ScatterEvent(const char* event_name, PyRep* packet);

    bool DoDestinyUpdate();
    std::list<PyTuple*> mDogmaMessages;
};

#endif
