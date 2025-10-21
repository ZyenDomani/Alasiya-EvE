/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
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
    Rewrite:    Allan
*/
#ifndef _EVE_SERVER_SYSTEM_BUBBLE_H_
#define _EVE_SERVER_SYSTEM_BUBBLE_H_

#include <map>
#include <vector>

#include "../eve-server.h"

namespace Bubble {
    namespace Type {
        enum {
            Normal              = 0,    // bubble not specified below
            Ice                 = 1,
            Belt                = 2,
            Gate                = 3,
            Anomaly             = 4,
            Mission             = 5,
            Incursion           = 6,
            Escalation          = 7
            // should we classify pos here also?  what about poco?  station?
        };
    }
    namespace Error {
        enum {
            None                 = 0,
            BubbleNull           = 1,
            BeltDisabled         = 2,
            RoamingDisabled      = 3,
            StaticDisabled       = 4,
            Spawned              = 5,
            PrepFail             = 6,
            NotAllowed           = 7
        };
    }
}

class Client;
class SetState;
class PyTuple;
class ContainerSE;
class SystemEntity;
class SystemManager;
class Timer;
class TowerSE;
class TCUSE;
class SBUSE;
class IHubSE;
class NPC;
class DroneSE;
class PyObject;
class ActiveModule;

class SystemBubble {
public:
    SystemBubble(SystemManager* pSystem, const GPoint& center, double radius);
    ~SystemBubble() noexcept;

    SystemEntity* const GetEntity(uint32 entityID) const;
    SystemManager* const GetSystem() const              { return m_system; }

    /* for spawn system     -allan 15July15 */
    void Process();
    void SetBelt(InventoryItemRef itemRef);
    void SetGate(uint32 gateID);
    void ResetBubbleRatSpawn();

    bool IsIce()                                        { return (m_type == Bubble::Type::Ice); }
    bool IsBelt()                                       { return (m_type == Bubble::Type::Belt); }
    bool IsGate()                                       { return (m_type == Bubble::Type::Gate); }
    bool IsNormal()                                     { return (m_type == Bubble::Type::Normal); }
    bool IsAnomaly()                                    { return (m_type == Bubble::Type::Anomaly); }
    bool IsMission()                                    { return (m_type == Bubble::Type::Mission); }
    bool IsIncursion()                                  { return (m_type == Bubble::Type::Incursion); }
    bool IsSpawned()                                    { return m_spawned; }

    // these are set in spawnMgr.
    void SetAnomaly()                                   { m_type = Bubble::Type::Anomaly; }
    void SetMission()                                   { m_type = Bubble::Type::Mission; }
    void SetIncursion()                                 { m_type = Bubble::Type::Incursion; }
    void SetSpawned(bool set=true)                      { m_spawned = set; }
    void SetSpawnTimer();

    /* various count queries */
    uint32 CountNPCs()                                  { return m_npcs.size(); }
    uint32 CountDrones()                                { return m_drones.size(); }
    uint32 CountPlayers()                               { return m_players.size(); }
    uint32 CountDynamics()                              { return m_dynamicEntities.size(); }

    /* used for bubble management */
    bool IsEmpty() const                                { return (m_entities.empty() and m_dynamicEntities.empty()); }
    bool HasPlayers() const                             { return !m_players.empty(); }
    bool HasStatics() const                             { return !m_entities.empty(); }
    // this includes items like containers, wrecks, pos, etc.
    bool HasDynamics() const                            { return !m_dynamicEntities.empty(); }
    double x() const                                    { return m_center.x; }
    double y() const                                    { return m_center.y; }
    double z() const                                    { return m_center.z; }
    uint16 GetID()                                      { return m_bubbleID; }
    uint32 GetSystemID();
    GPoint GetCenter()                                  { return m_center; }
    ContainerSE* GetCenterMarker()                      { return m_centerSE; }

    void clear();
    void PrintEntityList();

    void Add(SystemEntity* pSE);
    void Remove(SystemEntity* pSE);
    void ProcessWander(std::vector< SystemEntity* >& wanderers);

    void SendAddBalls(SystemEntity* to_who);
    void SendAddBalls2(SystemEntity* to_who);
    void RemoveExclusive(SystemEntity* pSE);
    void AddBallExclusive(SystemEntity* about_who);

    //send a set of destiny updates to every client in the bubble.
    void BubblecastDestinyUpdate(std::vector<PyTuple*> &updates, const char* desc) const;
    //send a set of destiny events to every client in the bubble.
    void BubblecastDestinyEvent(std::vector<PyTuple*> &events, const char* desc) const;
    //send a destiny update to every client in the bubble.
    void BubblecastDestinyUpdate(PyTuple** payload, const char* desc) const;
    //send a destiny event to every client in the bubble.
    void BubblecastDestinyEvent(PyTuple** payload, const char* desc) const;
    void BubblecastSendNotification(const char *notifyType, const char *idType, PyTuple **payload, bool seq=true);
    //send a destiny update to every client in the bubble EXCLUDING the given SystemEntity 'pSE'
    void BubblecastDestinyUpdateExclusive(PyTuple** payload, const char* desc, SystemEntity* pSE) const;

    bool InBubble(const GPoint &pt, bool inWarp=false) const;
    bool IsOverlap(const GPoint &pt) const;
    void MarkCenter();
    void RemoveMarkers();

    /* for warp bubble checks */
    bool HasWarpBubble()                                { return m_hasBubble; }
    void SetWarpBubble(bool set=false)                  { m_hasBubble = set; }
    /* for SetState */
    void GetEntities(std::map< uint32, SystemEntity* >& into) const;    // this one only sends visible entities
    /* for ??? */
    void GetAllEntities(std::map< uint32, SystemEntity* >& into) const; // this one gets all entities regardless of visibility
    /* for targeting purposes */
    void GetPlayers(std::vector<Client*> &into) const;
    /* for scanning */
    void GetEntityVec(std::vector<SystemEntity*> &into) const;
    SystemEntity* GetRandomEntity();

    /* for towers/ship abandoning */
    bool HasTower()                                     { return (m_towerSE != nullptr); }
    TowerSE* GetTowerSE()                               { return m_towerSE; }
    void SetTowerSE(TowerSE* pTower=nullptr)            { m_towerSE = pTower; }

    /* for setting TCU in bubble */
    bool HasTCU()                                       { return (m_tcuSE != nullptr); }
    TCUSE* GetTCUSE()                                   { return m_tcuSE; }
    void SetTCUSE(TCUSE* pTCU=nullptr)                  { m_tcuSE = pTCU; }

    bool HasSBU()                                       { return (m_sbuSE != nullptr); }
    SBUSE* GetSBUSE()                                   { return m_sbuSE; }
    void SetSBUSE(SBUSE* pSBU=nullptr)                  { m_sbuSE = pSBU; }

    bool HasIHub()                                      { return (m_ihubSE != nullptr); }
    IHubSE* GetIHubSE()                                 { return m_ihubSE; }
    void SetIHubSE(IHubSE* pIHub=nullptr)               { m_ihubSE = pIHub; }

    /* for system setstate */
    PyObject* GetDroneState() const;

    /* methods for updating new ships with currently active GFX */
    void AddNPC(NPC* pNPC);
    void RemoveNPC(NPC* pNPC);
    void AddActiveModule(ActiveModule* pMod);
    void RemoveActiveModule(ActiveModule* pMod);

    /* for command .syncloc - updates all players in bubble with positions of all dSE */
    void SyncPos();
    /* for command dropLoot - commands all npcs in bubble to jettison loot */
    void CmdDropLoot();


protected:
    const GPoint m_center;
    const double m_radius;

    // remove all balls in bubble for this SE
    void RemoveBall(SystemEntity* pSE);
    void RemoveBalls(SystemEntity* pSE);
    // remove this ball from bubble.  update all clients in bubble this SE has left.
    void RemoveBallExclusive(SystemEntity* pSE);

    void MarkBubble(const GPoint& position, std::string& name, std::string& desc, bool center=false);

private:
    TCUSE* m_tcuSE;
    SBUSE* m_sbuSE;
    IHubSE* m_ihubSE;
    TowerSE* m_towerSE;
    SystemManager* m_system;
    ContainerSE* m_centerSE;

    bool m_hasMarkers :1;
    bool m_hasBubble :1;       // for warp disruption bubbles (placeholder for later)

    // for spawn system     -allan 15July15
    Timer m_spawnTimer;
    bool m_spawned :1;

    uint8 m_type;                                       //Bubble::Type

    uint16 m_bubbleID;

    std::map<uint32, NPC*>                              m_npcs;             //we do not own these.
    std::map<uint32, Client*>                           m_players;          // testing with bubble player list (in std::map)
    std::map<uint32, DroneSE*>                          m_drones;           //we do not own these.
    std::map<uint32, SystemEntity*>                     m_markers;          // bubble marker cans.  we do own these.
    std::map<uint32, SystemEntity*>                     m_entities;         //we do not own these.
    std::map<uint32, ActiveModule*>                     m_activeModules;    // for sending gfx to new ships in bubble
    std::map<uint32, SystemEntity*>                     m_dynamicEntities;  //entities which may/may not move. we do not own these.
};

#endif  // _EVE_SERVER_SYSTEM_BUBBLE_H_
