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
: public DynamicSystemEntity,
  protected EVEClientSession,
  protected EVEPacketDispatcher
{
public:
    Client(PyServiceMgr &services, EVETCPConnection** con);
    virtual ~Client();

    bool ProcessNet();
    void Process();

    PyServiceMgr& services() const                  { return m_services; }

    /********************************************************************/
    /* Session values                                                   */
    /********************************************************************/
    std::string GetAddress() const                  { return mSession.GetCurrentString( "address" ); }
    std::string GetLanguageID() const               { return mSession.GetCurrentString( "languageID" ); }

    uint32 GetAccountType() const                   { return mSession.GetCurrentInt( "userType" ); }
    uint64 GetAccountRole() const                   { return mSession.GetCurrentULong( "role" ); }
    uint32 GetClientID() const                      { return mSession.GetCurrentInt( "clientid" ); }
    uint32 GetUserID() const                        { return mSession.GetCurrentInt( "userid" ); }
    uint32 GetSessionID() const                     { return 0; }

    uint32 GetCharacterID() const                   { return mSession.GetCurrentInt( "charid" ); }
    std::string GetCharacterName() const            { return mSession.GetCurrentString( "charname" ); }
    uint32 GetCorporationID() const                 { return mSession.GetCurrentInt( "corpid" ); }
    uint32 GetLocationID() const                    { return mSession.GetCurrentInt( "locationid" ); }
    uint32 GetStationID() const                     { return mSession.GetCurrentInt( "stationid" ); }
    uint32 GetSystemID() const                      { return mSession.GetCurrentInt( "solarsystemid2" ); }
    uint32 GetConstellationID() const               { return mSession.GetCurrentInt( "constellationid" ); }
    uint32 GetRegionID() const                      { return mSession.GetCurrentInt( "regionid" ); }
    uint32 GetCloneStationID() const                { return mSession.GetCurrentInt( "cloneStationID" ); }

    uint32 GetCorpHQ() const                        { return mSession.GetCurrentInt( "hqID" ); }
    int32 GetCorpAccountKey() const                 { return mSession.GetCurrentInt( "corpAccountKey" ); }
    uint64 GetCorpRole() const                      { return mSession.GetCurrentULong( "corpRole" ); }
    uint64 GetRolesAtAll() const                    { return mSession.GetCurrentULong( "rolesAtAll" ); }
    uint64 GetRolesAtBase() const                   { return mSession.GetCurrentULong( "rolesAtBase" ); }
    uint64 GetRolesAtHQ() const                     { return mSession.GetCurrentULong( "rolesAtHQ" ); }
    uint64 GetRolesAtOther() const                  { return mSession.GetCurrentULong( "rolesAtOther" ); }

    uint32 GetGangRole() const                      { return mSession.GetCurrentInt( "gangrole" ); }
    uint8 GetFleetRole() const                      { return mSession.GetCurrentInt( "fleetrole" ); }


    //  public functions to update client session when char's roles are changed
    void UpdateCorpSession( const CharacterConstRef& character );
    void UpdateFleetSession( const CharacterConstRef& character );

    // character data
    CharacterRef GetChar() const                    { return m_char; }
    ShipRef GetShip() const                         { return ShipRef::StaticCast( Item() ); }
    uint32 GetShipID() const                        { return m_shipId; }
    uint32 GetPodID() const                         { return m_char->capsuleID(); }

    double x() const                                { return GetPosition().x; }    //this is terribly inefficient.
    double y() const                                { return GetPosition().y; }    //this is terribly inefficient.
    double z() const                                { return GetPosition().z; }    //this is terribly inefficient.

    uint32 GetAllianceID() const                    { return GetChar()->allianceID(); }
    uint32 GetWarFactionID() const                  { return GetChar()->warFactionID(); }
    double GetBounty() const                        { return GetChar()->bounty(); }
    double GetSecurityRating() const                { return GetChar()->GetSecurityRating(); }
    double GetBalance() const                       { return GetChar()->balance(); }
    double GetAurBalance() const                    { return GetChar()->aurBalance(); }

    std::string GetSystemName() const               { return (m_systemName); }

    bool AddBalance(double amount);

    // misc char functions
    void SetShip(ShipRef shipRef);
    void CreateNewPod();
    void PickAlternateShip();
    void ResetAfterPodded();
    void BoardShip(ShipRef newShipRef);
    void UndockFromStation(uint32 stationID, uint32 systemID, uint32 constellationID, uint32 regionID, GPoint dockPosition, GPoint direction);
    void DockToStation(uint32 stationID);
    void MoveToLocation(uint32 location, const GPoint &pt);
    void MoveToPosition(const GPoint &pt);
    void MoveItem(uint32 itemID, uint32 location, EVEItemFlags flag);
    void SetDestiny(bool count=false);
    void ResetDestiny(bool count=false);
    void DestinyUndock(GPoint direction);
    void HasUndocked();
    void WarpIn();
    void WarpOut();
    void IsJumping();
    bool EnterSystem(uint32 systemID=0);
    void LoginToSystem(uint32 systemID=0);
    void UpdateLocation(uint32 locationID=0);
    bool SelectCharacter( uint32 char_id=0);
    void JoinCorporationUpdate(uint32 corp_id=0);
    void SavePosition();
    void SaveAllToDatabase();
    void UpdateSkillTraining();
	void SpawnNewRookieShip();
    bool IsHangarLoaded(uint32 stationID);
    void LoadStationHangar(uint32 stationID);
    void AddStationHangar(uint32 stationID);

    PyRep* GetInfoWindowDataForChar(Client *pClient);

    double GetPropulsionStrength() const;

    bool LaunchDrone(InventoryItemRef drone);

    void SendNotification(const PyAddress &dest, EVENotificationStream &noti, bool seq=true);
    void SendNotification(const char *notifyType, const char *idType, PyTuple **payload, bool seq=true);

    //destiny stuff...
    void SetDockStationID(uint32 stationID)         { m_dockStationID = stationID; };
    void SetDockPoint(GPoint &pt)                   { m_dockPoint = pt; }
    uint32 GetDockStationID()                       { return m_dockStationID; };
    GPoint GetDockPoint()                           { return m_dockPoint; }
    bool GetPendingDockOperation()                  { return m_needToDock; };
    void SetPendingDockOperation(bool needToDock)   { m_needToDock = needToDock; }
    bool InPod()                                    { return (GetShip()->groupID() == EVEDB::invGroups::Capsule ? true : false); }
    bool IsInSpace()                                { return (GetStationID() ? false : true); }
    bool IsDocked()                                 { return (GetStationID() ? true : false); }
    bool IsJump()                                   { return (m_moveState == msJump ? true : false); }
    bool IsInvul()                                  { return m_invul; }
    bool IsLogin()                                  { return m_login; }
    bool IsUndock()                                 { return m_undock; }
    bool HasBeyonce()                               { return m_beyonce; }
    bool IsBubbleWait()                             { return m_bubbleWait; }
    bool IsSetStateSent()                           { return m_setStateSent; }
    bool IsSessionChange()                          { return m_sessionChangeActive; }
    //bool SetMoveState(Client::_MoveState state)   { m_moveState = state; }
    void SetLogin(bool login=false)                 { m_login = login; }
    void SetInvul(bool invul=false)                 { m_invul = invul; }
    void SetUndock(bool undock=false)               { m_undock = undock; }
    void SetBeyonce(bool beyonce=false)             { m_beyonce = beyonce; }
    void SetBubbleWait(bool wait=false)             { m_bubbleWait = wait; }
    void SetStateSent(bool set=false)               { m_setStateSent = set; }
    void SetSessionTimer(uint32 time=10000)         { SetSessionChange(true); m_sessionTimer.Start(time); }
    void SetSessionChange(bool set=false)           { m_sessionChangeActive = set; }
    void SetAutoPilot(bool=false);
    void StargateJump(uint32 fromGate, uint32 toGate);
    void StartKilledTimer();

    //jetcan timer
    bool IsJetcanAvalible();
    uint32 JetcanTime()                             { return (m_jetcanTimer.GetRemainingTime()); }
    void StartJetcanTimer()                         { m_jetcanTimer.Start(180000); }

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

    // character notification messages
    void OnCharNowInStation();
    void OnCharNoLongerInStation();

    /********************************************************************/
    /* DynamicSystemEntity interface                                    */
    /********************************************************************/
    SystemManager* System() const                   { return m_system; }    //may yeild NULL  we DO NOT own this. (entityList does)
    EntityClass GetClass() const                    { return ecClient; }
    bool IsClient() const                           { return true; }
    Client *CastToClient()                          { return this; }
    const Client *CastToClient() const              { return this; }
    const char *GetName() const                     { return (GetChar() ? GetChar()->itemName().c_str() : "(null)"); }
    PyDict *MakeSlimItem() const;
    void EncodeDestiny( Buffer& into ) const;
    PyRep *GetAggressors() const;
    void QueueDestinyUpdate(PyTuple** update);
    void QueueDestinyEvent(PyTuple** multiEvent);
    void FlushQueue();

    void TargetAdded(SystemEntity *who);
    void TargetLost(SystemEntity *who);
    void TargetedAdd(SystemEntity *who);
    void TargetedLost(SystemEntity *who);
    void TargetsCleared();

    bool ApplyDamage(Damage &d);
    void Killed(Damage &fatal_blow);

    /********************************************************************/
    /* Server Administration Interface                                  */
    /********************************************************************/
    void DisconnectClient();
    void BanClient();

    Scan* scan()                    { return m_scan; }
    void SetScan(Scan* pScan)   { m_scan = pScan; }
    // set scan timer in ms  this is used in scan.cpp after time calc's are done
    void SetScanTimer(uint32 time)   { m_scanTimer.Start(time); }

    //  trade
    void SetTradeSession(TradeSession* ts)  { m_TS = ts; }
    void ClearTradeSession()                { m_TS = nullptr; }
    TradeSession* GetTradeSession()         { return m_TS; }

protected:
    //void _AwardBounty(SystemEntity *who);
    void _DropLoot(uint32 groupID, uint32 owner, uint32 locationID);

    void _UpdateSession( const CharacterConstRef& character );
    void InitSession( uint32 characterID  );

    // Packet stuff
    void _SendCallReturn( const PyAddress& source, uint64 callID, uint32 clientID, PyRep** return_value, const char* channel = NULL );
    void _SendException( const PyAddress& source, uint64 callID, MACHONETMSG_TYPE in_response_to, MACHONETERR_TYPE exception_type, PyRep** payload );
    void SendSessionChange();
    void _SendPingRequest();
    void _SendPingResponse( const PyAddress& source, uint64 callID );

    PyServiceMgr& m_services;
    Timer m_pingTimer;
    ClientSession mSession;

    SystemManager* m_system;    //we do not own this
    CharacterRef m_char;
    ServiceDB m_sDB;
    Scan* m_scan;
    TradeSession* m_TS;

    ShipRef m_ship;
    uint32 m_shipId;

	SystemGPoint m_SGP;     // interface to my variable 3-d point generating system  (which isnt finished yet... -allan)

    std::set<LSCChannel*> m_channels;    //we do not own these.

    //this whole move system is a piece of crap:
    typedef enum {
        msIdle,
        msJump
    } _MoveState;
    void _postMove(_MoveState type, uint32 wait_ms=500);
    _MoveState m_moveState;

    Timer m_moveTimer;
    Timer m_undockTimer;    // used to check for multiple calls to serices that should be ignored during this time
    Timer m_clientTimer;    // used to give process ticks to docked players (for skill updates...tick cycle consumption negligible)
    Timer m_cloakTimer;
    Timer m_invulTimer;
    Timer m_jumpTimer;
    Timer m_killedTimer;     // used to reset destiny set state after killed or otherwise changing ships
    Timer m_jetcanTimer;     // used to delay jetcan creation.  3min default
    Timer m_scanTimer;       // used to delay scan results based on skills, items, and other shit
    Timer m_sessionTimer;    // used to prevent multiple session changes from occuring too fast
    Timer m_logoutTimer;     // used to hold client object until WarpOut finishes
    uint32 m_moveSystemID;
    GPoint m_movePoint;
    GPoint m_dockPoint;
    uint32 m_dockStationID;
    void _ExecuteJump();
    bool m_needToDock;
    bool m_login;
    bool m_undock;
    bool m_bubbleWait;
    bool m_invul;
    bool m_beyonce;
    bool m_setStateSent;
    bool m_sessionChangeActive; // used to delay actions requiring destiny updates

    bool m_packaged;        // used to correctly package updates into a PackagedAction list

    std::map<uint32, bool> m_hangarLoaded;

    // set true for using autopilot.
    bool m_autoPilot = false;

    EvilNumber m_timeEndTrain;

    /********************************************************************/
    /* EVEClientSession interface                                       */
    /********************************************************************/
    void _GetVersion( VersionExchangeServer& version );
    uint32 _GetQueuePosition() { /* hack */ return 1; }

    /********************************************************************/
    /* EVEClientLogin statemachine                                      */
    /********************************************************************/
    bool _VerifyVersion( VersionExchangeClient& version );
    bool _VerifyCrypto( CryptoRequestPacket& cr );
    bool _VerifyLogin( CryptoChallengePacket& ccp );
    bool _VerifyVIPKey( const std::string& vipKey ) { /* do nothing */ return true; }
    bool _VerifyFuncResult( CryptoHandshakeResult& result );

    /********************************************************************/
    /* EVEPacketDispatcher interface                                    */
    /********************************************************************/
    bool Handle_CallReq( PyPacket* packet, PyCallStream& req );
    bool Handle_Notify( PyPacket* packet );
    bool Handle_PingReq( PyPacket* packet ) { _SendPingResponse( packet->dest, packet->source.callID ); return true; }
    bool Handle_PingRsp( PyPacket* packet ) { /* do nothing */ return true; }

private:
    //queues for destiny updates:
    PyList* m_destinyEventQueue;    //we own these. These are events as used in OnMultiEvent
    PyList* m_destinyUpdateQueue;    //we own these. They are the `update` which go into DoDestinyAction
    void _SendQueuedUpdates();

    uint32 m_nextNotifySequence;

    /************************************************************************/
    /* new system for MultiEvents                                           */
    /************************************************************************/
    bool ScatterEvent(const char* event_name, PyRep* packet);

    bool DoDestinyUpdate();
    std::list<PyTuple*> mDogmaMessages;
    std::string m_systemName;

};

#endif
