// Squad.cpp - A lightweight, transient coordinator for grouping ship fleets
//  allan - 4nov26

#include "system/Squad.h"



Squad::Squad(uint32 squadID)
: m_breakTimer(0),
m_tier(0),
m_squadID(squadID),
m_formationID(0),
m_spacing(0),
m_leader(nullptr),
m_target(nullptr)
{

}

Squad::~Squad() {
    m_members.clear();
}

void Squad::RegisterMember(NPC* pNPC) {
    if (pNPC == nullptr)
        return;

    m_members.push_back(pNPC);
    pNPC->SetSquad(this);

    // Dynamic Promotion Engine
    uint8 newRank = pNPC->GetRank();
    if (m_leader == nullptr) {
        // First ship on grid is leader by default
        m_leader = pNPC;
    } else if (newRank > m_leader->GetRank()) {
        m_leader = pNPC;

        _log(NPC__AI_MESSAGE, "Squad ID %u Hierarchy Shift: %s(%u) is assuming fleet command.",
             m_squadID, pNPC->GetName(), pNPC->GetID());
    }
}

void Squad::UnregisterMember(NPC* pNPC) {
    if (pNPC == nullptr)
        return;

    auto it = std::find(m_members.begin(), m_members.end(), pNPC);
    if (it != m_members.end())
        m_members.erase(it);

    if (m_leader == pNPC) {
        m_leader = nullptr;
        if (!m_members.empty()) {
            m_leader = m_members.front();
            _log(NPC__AI_MESSAGE, "Squad Leader was killed! NPC %u is stepping up to take tactical command.",
                 m_leader->GetID());
        }
    }

    if (m_members.empty()) {
        _log(NPC__AI_MESSAGE, "Squad ID %u has been completely wiped out from grid partition.", m_squadID);
        m_target = nullptr;
        m_leader = nullptr;
    }
}

void Squad::OnAllMembersArrived() {
    // this should be a switch
    if (m_tier == 1) {
        _log(NPC__AI_MESSAGE, "Squad ID %u fully landed. Assembling into formation for 5 seconds.", m_squadID);

        // Broadcast the formation ID to the client: watch them glide into position!
        //BroadcastFormationToGrid(EVEDB::Formations::Wedge);

        // Start a 5-second combat delay countdown timer
        m_breakTimer.Start(5000);
    }
}

void Squad::AssignLeader(NPC* pNPC) {
	// verify npc is part of this squad?   what if they're not?
    m_leader = pNPC;
}


const char* Squad::GetTierName() {
    return GetTierName(m_tier);
}

const char* Squad::GetTierName(uint8 tier/*0*/) {
    switch (tier) {
        case 1: return "Rookie"; break;
        case 2: return "Soldier"; break;
        case 3: return "Veteran"; break;
        case 4: return "Elite"; break;
        case 5: return "Apex"; break;
        default:
        case 0: return "Invalid"; break;
    }
}

