/**
 * Squad.h
 *      this class is a lightweight, transient group coordinator.
 *  by having a common data container, this makes formations, groups,
 *  tiers and waves easier to track/query/manipulate
 *
 * NOTE:  not for use with swarm
 *
 * @Author:     Allan
 * @Version:    0.25
 * @Date:       4nov26
 */

#pragma once

#include "npc/NPC.h"


class Squad {
public:

    struct Tier {
        enum {
            Invalid	= 0,
            Rookie      = 1,
            Soldier     = 2,
            Veteran     = 3,
            Elite       = 4,
            Apex        = 5
        };
    };

    // default c'tor
    Squad() =delete;
    // main c'tor
    Squad(uint32 squadID);
    // copy c'tor
    Squad(const Squad* oth) =delete;
    // move c'tor
    Squad(Squad&& oth) noexcept =default;
    // copy assignment
    Squad& operator=(Squad& oth) =delete;
    // move assignment
    Squad& operator=(Squad&& oth) =default;
    // d'tor
    ~Squad();

    void                RegisterMember(NPC* pNPC);
    void                UnregisterMember(NPC* pNPC);
    void                OnAllMembersArrived();

    //
    SpaceEntity*       GetTarget()                     { return m_target; }
    void                SetTarget(SpaceEntity* pTarget)
                                                        { m_target = pTarget; }

    uint16              GetID()                         { return m_squadID; }
    uint8               GetFormID()                     { return m_formationID; }
    float               GetSpacing()                    { return m_spacing; }
    uint8               GetTier()                       { return m_tier; }
	const char* 		GetTierName();
	const char* 		GetTierName(uint8 tier=0);

    //
    void                AssignLeader(NPC* pNPC);
    NPC*              GetLeader()                   { return m_leader; }
    std::vector<NPC*>&  GetMemberVec()                { return m_members; }

private:
    std::vector<NPC*> m_members;     // members of this squad, can be in multiple bubbles
    NPC*              m_leader;
    SpaceEntity*       m_target;

    uint32              m_squadID;
    float               m_spacing;
    Timer               m_breakTimer;
    uint8               m_tier;
    uint8               m_formationID;
};
