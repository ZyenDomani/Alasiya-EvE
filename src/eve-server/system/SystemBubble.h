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
#include <eve-compat.h>

class SystemEntity;
class PyTuple;
class DoDestiny_SetState;
class Client;
class SystemManager;

class SystemBubble {
public:
    SystemBubble(SystemManager* pSystem, const GPoint& center, double radius);
    ~SystemBubble() { clear(); };


    const GPoint m_center;
    const double m_radius;
    const double m_radius_hysteresis;

	void BubblecastDestiny(std::vector<PyTuple*> &updates, std::vector<PyTuple*> &events, const char* desc) const;
	void BubblecastDestinyUpdate(std::vector<PyTuple*> &updates, const char* desc) const;
	void BubblecastDestinyEvent(std::vector<PyTuple*> &events, const char* desc) const;
	void BubblecastDestinyUpdate(PyTuple** payload, const char* desc) const;
	void BubblecastDestinyEvent(PyTuple** payload, const char* desc) const;
	void BubblecastDestinyUpdateExclusive(PyTuple** payload, const char* desc, SystemEntity* pEntity) const;

    bool ProcessWander(std::vector<SystemEntity*> &wanderers);

    void Add(SystemEntity* pEntity, bool isPostWarp=false);
    void Remove(SystemEntity* pEntity);
    void AddExclusive(SystemEntity* pEntity);
    void RemoveExclusive(SystemEntity* pEntity);
    void clear();
    bool IsEmpty() const { return (m_entities.empty()); }   // this is used by empty bubble checks for deletion.
    //use m_players for lower npc process usage.  use m_entities for constant npc updates.
    bool HasPlayers() const { return (m_players.empty() ? false : true); }
    SystemEntity* const GetEntity(uint32 entityID) const;
    void GetEntities(std::vector<SystemEntity*> &into) const;
    void GetPlayers(std::vector<Client*> &into) const;   /* for targeting purposes */
    uint32 GetID() { return m_bubbleID; }
    GPoint GetCenter() { return m_center; }

    void AppendBalls(SystemEntity* about_who) const;

    bool InBubble(const GPoint &pt) const;

    // for spawn system     -allan 15July15
    void SetBelt(uint32 beltID);
    void SetGate(uint32 gateID);
    bool IsBelt()                       { return m_belt; }
    bool IsGate()                       { return m_gate; }
    bool IsSpawned()                    { return m_spawned; }
    void SetSpawned(bool set=false)     { m_spawned = set; }
    uint32 GetSpawnID(uint16 bubbleID);

    uint32 Count()		                { return m_bubbleIncrementer; }
    uint32 CountDynamics()              { return m_dynamicEntities.size(); }
    uint32 CountPlayers()               { return m_players.size(); }
    uint32 CountNPCs();

    void PrintEntityList();

protected:
    void _SendAddBalls(SystemEntity* to_who);
    void _SendRemoveBalls(SystemEntity* to_who);
    void _BubblecastAddBall(SystemEntity* about_who);
    void _BubblecastAddBallExclusive(SystemEntity* about_who);
    void _BubblecastRemoveBall(SystemEntity* about_who);
    void _BubblecastRemoveBallExclusive(SystemEntity* about_who);

    SystemManager* m_system;
	uint32 m_systemID;
    uint32 m_bubbleID;
    static uint32 m_bubbleIncrementer;
    std::map<uint32, SystemEntity*> m_entities;    //we do not own these.
    std::vector<SystemEntity*> m_dynamicEntities;    //entities which may move. we do not own these.
    std::vector<Client*> m_players;                // testing with bubble client list (in std::vector)

    // for spawn system     -allan 15July15
    bool m_belt = false;
    bool m_gate = false;
    bool m_spawned = false;

};

#endif
