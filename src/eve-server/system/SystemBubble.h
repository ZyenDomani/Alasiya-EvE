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
#ifndef __SYSTEMBUBBLE_H_INCL__
#define __SYSTEMBUBBLE_H_INCL__

#include <map>
#include <vector>

#include "utils/gpoint.h"

class Client;
class DoDestiny_SetState;
class PyTuple;
class SystemEntity;
class SystemManager;
class Timer;

class SystemBubble {
public:
    SystemBubble(SystemManager* pSystem, const GPoint& center, double radius);
    ~SystemBubble()                                     { clear(); }

    SystemEntity* const GetEntity(uint32 entityID) const;

    /* for spawn system     -allan 15July15 */
    void Process();
    void SetBelt(uint32 beltID);
    void SetGate(uint32 gateID);
    void ResetBubbleSpawn();
    void SetSpawned(bool set)                           { m_spawned = set; }
    bool IsBelt()                                       { return m_belt; }
    bool IsGate()                                       { return m_gate; }
    bool IsSpawned()                                    { return m_spawned; }
    void SetSpawnTimer(bool isBelt = false);

    /* various count queries */
    uint32 CountNPCs();
    uint32 CountPlayers()                               { return m_players.size(); }
    uint32 CountDynamics()                              { return m_dynamicEntities.size(); }

    /* used for bubble management */
    bool IsEmpty() const                                { return m_entities.empty(); }
    bool HasStatics() const                             { return (m_entities.empty() ? false : true); }
    bool HasDynamics() const                            { return (m_dynamicEntities.empty() ? false : true); }
    bool HasPlayers() const                             { return (m_players.empty() ? false : true); }
    double x() const                                    { return m_center.x; }
    double y() const                                    { return m_center.y; }
    double z() const                                    { return m_center.z; }
    uint32 GetID()                                      { return m_bubbleID; }
    GPoint GetCenter()                                  { return m_center; }

    void clear();
    void PrintEntityList();

    void Add(SystemEntity* pEntity);
    void Remove(SystemEntity* pEntity);
    void ProcessWander(std::vector< SystemEntity* >& wanderers);

    void SendAddBalls(SystemEntity* to_who);
    void SendAddBalls2(SystemEntity* to_who);
    void RemoveExclusive(SystemEntity* pEntity);
    void AddBallExclusive(SystemEntity* about_who);

	void BubblecastDestiny(std::vector<PyTuple*> &updates, std::vector<PyTuple*> &events, const char* desc) const;
	void BubblecastDestinyUpdate(std::vector<PyTuple*> &updates, const char* desc) const;
	void BubblecastDestinyEvent(std::vector<PyTuple*> &events, const char* desc) const;
	void BubblecastDestinyUpdate(PyTuple** payload, const char* desc) const;
	void BubblecastDestinyEvent(PyTuple** payload, const char* desc) const;
    void BubblecastSendNotification(const char *notifyType, const char *idType, PyTuple **payload, bool seq=true);
    void BubblecastDestinyUpdateExclusive(PyTuple** payload, const char* desc, SystemEntity* pEntity) const;

    bool InBubble(const GPoint &pt) const;

    /* for targeting purposes */
    void GetEntities(std::vector<SystemEntity*> &into) const;
    void GetPlayers(std::vector<Client*> &into) const;

protected:
    const GPoint m_center;
    const double m_radius;
    const double m_radius_hysteresis;

    void RemoveBall(SystemEntity* about_who);
    void RemoveBalls(SystemEntity* to_who);
    void RemoveBallExclusive(SystemEntity* about_who);

private:
    SystemManager* m_system = nullptr;

	uint32 m_systemID = 0;
    uint32 m_bubbleID = 0;

    static uint32 m_bubbleIncrementer;

    std::vector<Client*> m_players;                  // testing with bubble player list (in std::vector)
    std::vector<SystemEntity*> m_dynamicEntities;    //entities which may/may not move. we do not own these.
    std::map<uint32, SystemEntity*> m_entities;      //we do not own these.

    // for spawn system     -allan 15July15
    Timer m_spawnTimer;
    bool m_belt = false;
    bool m_gate = false;
    bool m_spawned = false;
};

#endif
