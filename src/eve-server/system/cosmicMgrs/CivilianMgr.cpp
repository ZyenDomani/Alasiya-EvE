
/**
 * @name CivilianMgr.cpp
 *     Civilian (non-combatant NPC) management system for Alasiya EvEmu
 *
 * @Author:        Allan
 * @date:          12 Feb 2017
 * @updated:   June 2026 (Refactored for Ambient FSM by Gemini)
 *
 */

/* Civilian Logging
 * CIV__ERROR
 * CIV__WARNING
 * CIV__INFO
 */


#include "eve-server.h"

#include "EVEServerConfig.h"
#include "PyServiceMgr.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/CivilianMgr.h"

/*  this class will be in charge of creating all non-combatant npcs ingame, hereinafter refered to as NC.
 * the purpose here is to simulate civilian actions/activity by having ships travel between locations.
 *
 *  this data is minimally-constructed and not tracked on server; all vfx are client-side
 *
 *  this is a singleton class, to keep things simple.
 */


CivilianMgr::CivilianMgr()
: m_services(nullptr),
m_initalized(false)
{
}

void CivilianMgr::Initialize(PyServiceMgr* svc) {
    m_services = svc;
    m_initalized = true;
    sLog.Blue(" Civilian Manager", "Civilian Manager Initialized.");
}

void CivilianMgr::SpawnCiv(SystemManager* sMgr) {
    if (!m_initalized)
        return;

    // Pick random point-to-point warp points inside the system manager landmarks
    /*
    SystemEntity* pOrig = sMgr->GetCivOrig();
    SystemEntity* pDest = sMgr->GetCivDest(pOrig); // verify separate destination

    if (pOrig->SysBubble()->IsEmpty() and pDest->SysBubble()->IsEmpty())
        return;

    // Use a vector to preserve our exact fill position order
    // Element [0] will ALWAYS be our leader, elements [1+] will be escorts
    std::vector<uint16_t> spawnSequence;
	uint8 formation = 0;
	Civilian* pLeader = nullptr;

    // Roll for class type: 60% Single, 40% Convoy  (config option?)
    uint32 roll = MakeRandomInt();
    if (roll < 61) {
        spawnSequence.push_back(GetCivilianShip()); // Solo ship (Index 0)
    } else {	// Convoy!
        spawnSequence.push_back(GetCivilianHauler());   // Convoy Leader (Index 0)
        formation  = sDataMgr.GetRandFormation(); // Choose formation footprint from available options

		// set up data for guards   **todo later - different guard faction?**
        uint8 escortCount = MakeRandomInt(2, 6); // Spawns 2 to 6 escorts
        for (uint8 i = 0; i < escortCount; ++i) {
			// once this is working, create guards in static data...can group by type, faction, size, etc.
            spawnSequence.push_back(CivGuards[MakeRandomInt(0, 3)]);  // Appended in fill order (Indices 1+)
        }
    }

	// spawn civ ships
	std::string name = "CivSpawn";
    for (size_t i = 0; i < spawnSequence.size(); ++i) {

		Civilian* pCiv = new Civilian(sItemFactory.GetNextTempID(), spawnSequence[i]);

        _log(CIV__INFO, "CivilianMgr::SpawnCiv - Spawning Civilian %s type %u (%u)", pCiv->name(), pCiv->typeID(), pCiv->itemID());

        // If this isn't first, it is an escort.
        if (i > 0) {
			pCiv->SetFormID(formation);
            pCiv->SetLeader(pLeader);
			pLeader->AddGuard(pCiv);
        } else {
			pLeader = pCiv;
        }

		// civilian created...drop in (player's) system
		pCiv->Init(pOrig, pDest);
	}
	*/
}

void CivilianMgr::Process() {
    // i think we'll let sysMgr handle the Process calls.
    // no point in extra maps to run it here
}


/*

    // there are 24 haulers(g297) and 4 guards(g298) in these groups
    static const uint32 CivHaulers[] = { 10043, 10044, 10045, 10114, 10115, 10116, 10823, 20719 };
    static const uint32 CivGuards[]  = { 10999, 11000, 11001, 11002 };





***********************  create
sysMgr()->AddCiv(Civilian* pCiv) 	{ m_civilians.push_back(pCiv); }
sysMgr()->RemoveCiv(Civilian* pCiv) { m_civilians.erase(pCiv); SafeDelete(pCiv); }

std::vector<Civilian*> m_civilians;




***********************  update sysMgr with data for civ ships
check for pirate spawn:  (for later)
if pirate=true, create an actual faction npc to warp to anom/sig (will have to disable auto-attack for this)
else send data to civMgr


in Init() or boot()

    // Scale True Security to ensure low-sec/null-sec don't wipe out numbers completely
    // High-Sec (1.0) -> Multiplier 1.0
    // Low-Sec (0.1)  -> Multiplier 0.55
    // Null-Sec (-1.0)-> Multiplier 0.0 (or forced minimum if you want smugglers!)
    float secMultiplier = (truSec + 1.0f) / 2.0f;
    if (secMultiplier < 0.1f)
		secMultiplier = 0.0f; // Silence traffic in deep Null

    m_civDensity = static_cast<uint32>(secMultiplier * playerCount * (minConvoys + (trand() % (maxConvoys - minConvoys + 1))));


in Process()

	for (auto& cur : m_civilians)
    	cur.Process();

	//on (x)m tic (config or system)
    if (m_players and m_civDensity) {
    	// create spawns to density limits
    	if (m_civilians.size() < m_civDensity)
        	sCivMgr.SpawnCiv(this);
    }





***********************  update marketbot
void TraderJoeManager::ExecuteCargoDrop(StationSpaceComponent* stationSE) {
    if (stationSE == nullptr)
        return;

    uint32 stationID = stationSE->GetID();
    uint32 regionID  = stationSE->GetRegionID();

    // 1. Roll to see if this specific cargo drop actually triggers a rare spawn (e.g., 25% chance)
    if ((trand() % 100) > 25) {
        _log(MARKET__INFO, "TraderJoe::ExecuteCargoDrop - Hauler docked at station %u but carried standard market supplies. No special injection.", stationID);
        return;
    }

    // 2. Select a random limited-item template from your configuration array
    uint32 targetTypeID   = sDataMgr.GetRandCivilianCargoItem();
    uint32 dropQuantity   = 1 + (trand() % 5); // Small, highly competitive quantities
    double competitivePrice = sMarketMgr.GetAverageMarketPrice(targetTypeID) * 0.90; // 10% discount!

    _log(MARKET__INFO, "TraderJoe::ExecuteCargoDrop - SPECIAL CARGO INJECTED! Station: %u, Item: %u, Qty: %u", stationID, targetTypeID, dropQuantity);

    // 3. Construct a standard sell order payload attributed to the TraderJoe bot NPC ID
    MarketOrderData order;
    order.orderID      = sEntityMgr.GenerateNewOrderID();
    order.charID       = TRADER_JOE_NPC_ID; // Use a dedicated, fixed corporate/NPC character ID
    order.stationID    = stationID;
    order.regionID     = regionID;
    order.typeID       = targetTypeID;
    order.price        = competitivePrice;
    order.volEntered   = dropQuantity;
    order.volRemaining = dropQuantity;
    order.minVolume    = 1;
    order.range        = Market::Range::Station; // Must be bought directly at this specific station
    order.duration     = 2; // Limited 2-day availability constraint
    order.isBid        = false; // This is a Sell Order

    // 4. Push directly into your market system's active memory index
    sMarketMgr.AddOrder(order);

    // 5. Broadcast a localized market update notification packet to all players currently inside the station
    stationSE->BroadcastMarketUpdate(targetTypeID);
}
*/