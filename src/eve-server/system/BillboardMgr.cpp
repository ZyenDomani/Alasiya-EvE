// SystemBillboardMgr.h
#pragma once
#include <map>
#include <vector>
#include <string>

struct BillboardTerminal {
    uint32_t ballID;         // The physical 3D entity ID in the Destiny universe
    uint32_t parentEntityID; // The station or stargate this screen is physically welded to
    uint32_t currentGraphicID; // The active text/image/video stream graphic template ID
    bool isHijacked;         // Track if the mesh network has overridden this screen
};

class SystemBillboardMgr {
private:
    uint32_t m_systemID;

    // Catalog of all screen assets currently active across all grids in this system
    std::map<uint32_t, BillboardTerminal> m_activeTerminals;

public:
    SystemBillboardMgr(uint32_t systemID) : m_systemID(systemID) {}
    ~SystemBillboardMgr() {}

    // Core Management Interfaces
    void RegisterScreen(uint32_t ballID, uint32_t parentID, uint32_t defaultGfxID);
    void UpdateScreenGraphic(uint32_t ballID, uint32_t newGfxID);

    // The Player Sync Hook (Called when a player warps onto a grid or docks)
    void SyncGridScreensToClient(Character* pPlayer, uint32_t gridID);

    // The Hive Mind Interface (Where your Borg code hooks in later)
    void ForceSwarmTelemetryBleed(uint32_t droneSitesCount);
};



void SystemBillboardMgr::UpdateScreenGraphic(uint32_t ballID, uint32_t newGfxID) {
    auto it = m_activeTerminals.find(ballID);
    if (it == m_activeTerminals.end()) return;

    BillboardTerminal& screen = it->second;
    screen.currentGraphicID = newGfxID;

    // Fetch the live grid containing the parent object to broadcast the dynamic change live
    SystemGrid* pGrid = m_pSystemMgr->GetGridByEntityID(screen.parentEntityID);
    if (pGrid) {
        Packet broadcastPacket(pyoOnObjectAdvertiserChange);
        broadcastPacket.WriteInt32(screen.ballID);
        broadcastPacket.WriteInt32(screen.currentGraphicID);

        // Broadcast the update instantly to everyone currently rendering the space scene
        pGrid->BroadcastToActiveClients(broadcastPacket);
    }
}
void SystemBillboardMgr::ForceSwarmTelemetryBleed(uint32_t droneSitesCount) {
    // Standard EVE corporate advertisement IDs
    // 1042 = Scope News, 1043 = Kaalakiota, 1044 = Quafe
    uint32_t corruptedDroneScreenID = 88402; // Static raw hex animation template

    for (auto& [ballID, screen] : m_activeTerminals) {
        // Roll a clean percentage check based on system infection levels
        if (droneSitesCount >= 4 && (rand() % 100 < 60)) {
            screen.currentGraphicID = corruptedDroneScreenID;
            screen.isHijacked = true;

            // Push the update live onto the active screen
            UpdateScreenGraphic(screen.ballID, corruptedDroneScreenID);
        }
    }
}
// Within your Hangar/Station Session Manager
std::string HangarManager::GenerateStationInteriorLayout(Character* pPlayer, uint32_t stationTypeID) {
    // 1. Fetch the default client-side graphic ID string for this hangar's screen loop
    // (e.g., "res:/dx9/scene/station/interior/advertisements/scope_news_01.red")
    std::string activeScreenAssetPath = GetDefaultStationAdPath(stationTypeID);

    // 2. Query the System Master Hive Mind to see if an infestation is active
    SystemManager* pSystemMgr = pPlayer->GetSystemManager();
    uint32_t activeDroneHives = pSystemMgr->GetAnomalyManager()->CountActiveSignaturesByFaction(factionUnknown);

    if (activeDroneHives >= 3) {
        // Severe infestation! Roll a chance to corrupt the initial runtime cache load
        if (rand() % 100 < 50) {
            // OVERWRITE the standard path with the client's built-in corrupted static asset file!
            activeScreenAssetPath = "res:/dx9/scene/station/interior/advertisements/corrupted_mesh_telemetry_01.red";

            // Trigger a direct cockpit HUD flash so the player notices the network corruption live
            pPlayer->SendCriticalHUDAlert("[CRITICAL: STATION_DATA_BUS_BLEED // CORE_TELEMETRY_OVERFLOW]");
        }
    }

    // 3. Construct the finalized binary layout definition packet block
    std::stringstream layoutBuffer;
    layoutBuffer << "{ 'hangarType': " << stationTypeID
                 << ", 'screenAsset': '" << activeScreenAssetPath << "'"
                 << ", 'cacheMode': 'runtime' }"; // Injects straight into their runtime cache block

    return layoutBuffer.str();
}

// Inside your Emulator's Billboard/Bounty data serializing manager
std::string GetBountyAmountTextOverride(uint32_t solarSystemID, double actualBountyIsk) {
    uint32_t hiveCount = sAnomalyMgr.CountActiveSignaturesByFaction(solarSystemID, factionUnknown);

    if (hiveCount >= 3) {
        // OVERRIDE: Instead of showing standard ISK numbers, output a severe network failure message
        // This will be baked straight into bounty_caption.dds natively by the client!
        return "[9,999,999,999.00 ISK // WARNING: NODE_ALIGNMENT_CRITICAL // TARGET_RECLAIM_SEQ_ACTIVE]";
    }

    // Baseline: Return normal formatted currency string
    return FormatIskCurrencyValue(actualBountyIsk);
}

// Within your C++ Emulator's Core System Loop
void SystemGrid::ProcessEnvironmentalWorldSync(uint64_t currentFrame) {
    // Only pay the execution cost to scan web directories once every 30 seconds (900 frames)
    if (currentFrame % 900 != 0) return;

    SystemManager* pSystemMgr = this->GetSystemManager();
    uint32_t currentSystemID = pSystemMgr->GetSystemID();

    // Query your live anomaly manager to verify if a collective infection is active
    uint32_t activeHiveNodes = pSystemMgr->GetAnomalyManager()->CountActiveSignaturesByFaction(factionUnknown);

    if (activeHiveNodes >= 3) {
        // Severe Infestation: Force copy the pre-rendered Borg asset textures into your
        // Apache active cache directories so that any incoming client HTTP requests harvest the virus look!
        std::system("cp /var/www/html/assets/templates/borg_main.dds /var/www/html/live_cache/advert_active.dds");
        std::system("cp /var/www/html/assets/templates/borg_face.jpg /var/www/html/live_cache/portrait_active.jpg");

        // Update the SQL tracking matrix so your PHP headline tickers synchronize instantly
        pSystemMgr->ExecuteDatabaseQuery("UPDATE system_infestation_matrix SET activeHiveCount = %u WHERE solarSystemID = %u", activeHiveNodes, currentSystemID);
    } else {
        // Stable State: Restore clean corporate advertising assets into the active directory path
        std::system("cp /var/www/html/assets/templates/quafe_main.dds /var/www/html/live_cache/advert_active.dds");
        std::system("cp /var/www/html/assets/templates/default_face.jpg /var/www/html/live_cache/portrait_active.jpg");
    }
}









eve\client\script\environment\spaceObject\billboard.py
TQ_NEWS_HEADLINES_URL = 'http://www.eveonline.com/mb/news-headlines.asp'
SERENITY_NEWS_HEADLINES_URL = 'http://eve.gtgame.com.cn/gamenews/indexhl.htm'

eve\client\script\ui\services\holoscreenSvc.py
 RSS_FEEDS = ['http://www.eveonline.com/feed/rdfnews.asp?tid=7', 'http://www.eveonline.com/feed/rdfnews.asp?tid=4']

