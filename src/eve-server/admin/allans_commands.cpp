

#include <stdio.h>
#include "eve-server.h"

#include "Client.h"
#include "ConsoleCommands.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "admin/AllCommands.h"
#include "admin/CommandDB.h"
#include "fleet/FleetService.h"
#include "inventory/AttributeEnum.h"
#include "inventory/InventoryDB.h"
#include "inventory/InventoryItem.h"
#include "manufacturing/Blueprint.h"
#include "npc/Drone.h"
#include "station/Station.h"
#include "system/Damage.h"
#include "system/DestinyManager.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"
#include "system/cosmicMgrs/AnomalyMgr.h"
#include "system/cosmicMgrs/BeltMgr.h"
#include "system/cosmicMgrs/DungeonMgr.h"
#include <planet/Moon.h>



PyResult Command_spawndungeon(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* this command is used to test dungeon spawn system - wip.   -allan 21Feb15
     *
     * upon execution, this command will spawn a random dungeon from db in callers solarSystem,
     *   then create a bookmark in their PnP/BM window
     */

    if (args.argCount() != 2) {
        throw PyException(MakeCustomError("Correct Usage: .spawndungeon <dungeonTemplateID>"));
    }

    if (!args.isNumber(1))
        throw PyException(MakeCustomError("Argument 1 must be a template ID."));

    /** @todo update this to new creation code */
    /*
        if (!who->SystemMgr()->GetDungMgr()->Create(atoi(args.arg(1).c_str())))
            who->SendErrorMsg("RoomID = 0 for templateID %u", atoi(args.arg(1).c_str()));
    */
    return nullptr;
}

PyResult Command_removedungeon(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* this command is used to test dungeon spawn system - wip.   -allan 21Feb15
     *
     * upon execution, this command will spawn a random dungeon from db in callers solarSystem,
     *   then create a bookmark in their PnP/BM window
     */
    return nullptr;
}

PyResult Command_siglist(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* this command is used to test dungeon spawn system - wip.   -allan 21Feb15
     *   will list currently active dungeons, by systemID.
     */

    std::vector<CosmicSignature> sig;
    who->SystemMgr()->GetAnomMgr()->GetSignatureList(sig);

    int count = sig.size();

    std::ostringstream str;
    str.clear();
    str << "There are currently %u active dungeons<br>"; //50
    str << "LocationID aID iID 'Name'<br>"; //30

    for (auto sigs : sig) {
        // sysSignatures (sigID,sigItemID,dungeonType,sigName,systemID,sigTypeID,sigGroupID,scanGroupID,scanAttributeID,x,y,z)
        str << sigs.systemID << " " << sigs.sigID.c_str() << " " << sigs.sigItemID << " '" << sigs.sigName.c_str() << "'<br>"; //100
    }

    int size = count * 100;
    size += 80;
    char reply[size];
    snprintf(reply, size, str.str().c_str(), count);

    who->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_heal(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (args.argCount()== 1)
        who->GetShip()->Heal();
    else if (args.argCount() == 2) {
        if (!args.isNumber(1))
            throw PyException(MakeCustomError("Argument 1 should be a character ID"));

        uint32 entity = atoi(args.arg(1).c_str());

        Client *target = sEntityList.FindClientByCharID(entity);
        if (target == NULL)
            throw PyException(MakeCustomError("Cannot find Character by the entityID %d", entity));

        target->GetShip()->Heal();
    }

    return(new PyString("Heal successful!"));
}

PyResult Command_status(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    //if (!who->IsInSpace())
    //    throw PyException(MakeCustomError("You're not in space."));
    if (!who->GetShipSE()->SysBubble())
        who->EnterSystem(who->GetSystemID());
    if (!who->GetShipSE()->DestinyMgr())
        who->SetDestiny(NULL_ORIGIN);

    ShipItem* pShip = who->GetShip().get();

    char reply[150];
    snprintf(reply, 150,
             "PG: %.2f(%.3f)<br>" //25
             "Cap: %.2f(%.3f)<br>" //28
             "CPU: %.2f(%.3f)<br>" //28
             "Hull: %.2f(%.3f)<br>" //32
             "Armor: %.2f(%.3f)<br>" //27
             "Shield: %.2f(%.3f)", //28
             pShip->GetShipPGLevel(), pShip->GetShipPGPercent().get_float(),
             pShip->GetShipCapacitorLevel(), pShip->GetShipCapacitorPercent().get_float(),
             pShip->GetShipCPULevel(), pShip->GetShipCPUPercent().get_float(),
             pShip->GetShipHullHP(), pShip->GetShipHullPercent().get_float(),
             pShip->GetShipArmorHP(), pShip->GetShipArmorPercent().get_float(),
             pShip->GetShipShieldHP(), pShip->GetShipShieldPercent().get_float()
    );

    who->SendInfoModalMsg(reply);

    return new PyString(reply);
}

PyResult Command_list(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* this command is used to debug bubble entities
     * wip.   -allan 25Apr15
     */

    if (!who->IsInSpace())
        return nullptr;

    if (!who->GetShipSE()->SysBubble())
        if (who->IsInSpace())
            who->EnterSystem(who->GetSystemID());
        else
            throw PyException(MakeCustomError("You must be in space to list bubble inventory."));

    SystemBubble *b = who->GetShipSE()->SysBubble();
    uint32 bubble = b->GetID();
    uint32 dynamics = b->CountDynamics();
    uint32 npcs = b->CountNPCs();
    uint32 players = b->CountPlayers();

    std::vector<SystemEntity*> into;
    b->GetEntities(into);

    std::ostringstream str;
    str.clear();
    str << "Bubble: %u<br>"; //22
    str << "Dynamics: %u<br>"; //19
    str << "NPCs: %u<br>"; //18
    str << "Players: %u<br>"; //23
    str << "<br>"; //5

    for (auto cur : into) {
        if (cur->DestinyMgr()) {
            std::string modeStr = "Rigid";
            if (cur->IsDynamicEntity()) {
                switch (cur->DestinyMgr()->GetState()) {
                    case 0: modeStr = "Goto"; break;
                    case 1: modeStr = "Follow"; break;
                    case 2: modeStr = "Stop"; break;
                    case 3: modeStr = "Warp"; break;
                    case 4: modeStr = "Orbit"; break;
                    case 5: modeStr = "Missile"; break;
                    case 6: modeStr = "Mushroom"; break;
                    case 7: modeStr = "Boid"; break;
                    case 8: modeStr = "Troll"; break;
                    case 9: modeStr = "Miniball"; break;
                    case 10: modeStr = "Field"; break;
                    case 11: modeStr = "Rigid"; break;
                    case 12: modeStr = "Formation"; break;
                }
            }
            str << cur->GetID() << ": " << modeStr.c_str() << " (csf: " << cur->DestinyMgr()->GetSpeedFraction() << ") speed: ";
            str << cur->DestinyMgr()->GetSpeed() << " [" << cur->GetName() << "]<br>"; // 13 + 27 + 40 for name (80)
        } else {
            str << cur->GetID() << ": None (csf: 0) speed: 0 [" << cur->GetName() << "]<br>"; // 13 + 27 + 40 for name (80)
        }
    }

    int count = into.size();
    int size = count * 80;
    size += 90;
    char reply[size];
    snprintf(reply, size, str.str().c_str(), bubble, dynamics, npcs, players);

    who->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_bubblelist(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* this command is used to debug bubble entities
     * wip.   -allan 2June16
     */

    if (!who->GetShipSE()->SysBubble())
        if (who->IsInSpace())
            who->EnterSystem(who->GetSystemID());
        else
            throw PyException(MakeCustomError("You must be in space to list space inventory."));

        SystemBubble *b = who->GetShipSE()->SysBubble();
    uint32 bubble = b->GetID();
    uint32 dynamics = b->CountDynamics();
    uint32 npcs = b->CountNPCs();
    uint32 players = b->CountPlayers();

    std::vector<SystemEntity*> into;
    b->GetEntities(into);

    std::ostringstream str;
    str.clear();
    str << "Bubble: %u<br>"; //22
    str << "Dynamics: %u<br>"; //19
    str << "NPCs: %u<br>"; //18
    str << "Players: %u<br>"; //23
    str << "<br>"; //5

    for (auto cur : into)
        str << cur->GetID() << ", " << cur->GetName() << "<br>"; // 13 + 27 for name (40)

        int count = into.size();
    int size = count * 40;
    size += 90;
    char reply[size];
    snprintf(reply, size, str.str().c_str(), bubble, dynamics, npcs, players);

    who->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_commandlist(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /*
     * this command will send the client a list of loaded game commands, role required, and description.  -allan 23May15
     */

    char reply[65];
    snprintf(reply, 65,
             "Working on making this list...check back later.<br>" //53
             " -Allan"); //9

    who->SendInfoModalMsg(reply);

    return new PyString(reply);
}


PyResult Command_secstatus(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /*
     * this command will send the client the security status of the current Character.  -allan 5July15
     */

    char reply[65];
    snprintf(reply, 65,
             "SecStatus: %f.", who->GetSecurityRating()); //53

    who->SendInfoModalMsg(reply);

    return new PyString(reply);
}

PyResult Command_destinyvars(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!who->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (!who->GetShipSE()->SysBubble())
        who->EnterSystem(who->GetSystemID());
    if (!who->GetShipSE()->DestinyMgr())
        who->SetDestiny(NULL_ORIGIN);

    DestinyManager* dm = who->GetShipSE()->DestinyMgr();

    char reply[250];
    snprintf(reply, 250,
             "ShipID: %u<br>"
             "IsCloaked: %u<br>" //28
             "IsWarping: %u<br>" //27
             "InPod: %u<br>" //27
             "IsInSpace: %u<br>" //27
             "IsDocked: %u<br>" //27
             "IsJump: %u<br>" //27
             "IsInvul: %u<br>" //27
             "IsLogin: %u<br>" //27
             "IsUndock: %u<br>" //27
             "HasBeyonce: %u<br>" //27
             "IsBubbleWait: %u<br>" //27
             "IsSetStateSent: %u<br>", //27
                who->GetShipID(), dm->IsCloaked(), dm->IsWarping(), who->InPod(), who->IsInSpace(), who->IsDocked(), who->IsJump(),
                who->IsInvul(), who->IsLogin(),  who->IsUndock(), who->HasBeyonce(), who->IsBubbleWait(), who->IsSetStateSent()
            );

    who->SendInfoModalMsg(reply);

    return new PyString(reply);
}

PyResult Command_shipvars(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!who->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (!who->GetShipSE()->SysBubble())
        who->EnterSystem(who->GetSystemID());
    if (!who->GetShipSE()->DestinyMgr())
        who->SetDestiny(NULL_ORIGIN);

    DestinyManager* dm = who->GetShipSE()->DestinyMgr();

    char reply[250];
    snprintf(reply, 250,
             "ShipID: %u<br>"
             "Mass: %.2f<br>" //28
             "AlignTime: %.2f<br>" //27
             "AccelTime: %.2f<br>"
             "MaxSpeed: %.2f<br>" //27
             "WarpSpeed: %.2f<br>" //27
             "WarpTime: %.2f<br>" //27
             "WarpDropSpeed: %.2f<br>" //27
             "Radius: %.2f<br>" //27
             "CapNeed: %.2f<br>" //27
             "Agility: %.3f<br>" //27
             "Inertia: %.3f<br>", //27
                who->GetShipID(), dm->GetMass(), dm->GetAlignTime(), dm->GetAccelTime(), dm->GetMaxVelocity(), (float)(dm->GetWarpSpeed() /10), dm->GetWarpTime(),
                dm->GetWarpDropSpeed(), dm->GetRadius(), dm->GetCapNeed(), dm->GetAgility(), dm->GetInertia()
            );

    who->SendInfoModalMsg(reply);

    return new PyString(reply);
}

PyResult Command_halt(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!who->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (!who->GetShipSE()->SysBubble())
        who->EnterSystem(who->GetSystemID());
    if (!who->GetShipSE()->DestinyMgr())
        who->SetDestiny(NULL_ORIGIN);

    who->GetShipSE()->DestinyMgr()->Halt();

    char reply[25];
    snprintf(reply, 25,
             "Ship Halted.");

    who->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_shutdown(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* ingame command to immediatly save loaded items and halt server.
     */
    sConsole.HaltServer();
    return new PyNone();
}

PyResult Command_beltlist(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* this command is used to debug asteroid creation/management
     * wip.   -allan 15April16
     */

    std::vector<AsteroidSE*> invMap;
    invMap.clear();
    uint32 beltID = sBubbleMgr.GetBeltID(who->GetShipSE()->SysBubble()->GetID());
    BeltMgr* belt = who->GetShipSE()->SystemMgr()->GetBeltMgr();
    belt->GetList(beltID, invMap);

    std::ostringstream str;
    str.clear();
    str << "BeltID %u has %u roids in it.<br><br>"; //40

    for (auto cur : invMap)
        str << cur->GetName() << ": " << cur->GetID() << "<br>"; // 20 + 40 for name (60)

    int count = invMap.size();
    int size = count * 60;
    size += 50;
    char reply[size];
    snprintf(reply, size, str.str().c_str(), beltID, count);

    who->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_inventory(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* this command is used to debug inventory
     * wip.   -allan 15Mar16
     */

    std::map<uint32, InventoryItemRef> invMap;
    invMap.clear();

    InventoryItem* item(nullptr);
    Inventory* inv(nullptr);
    uint32 inventoryID = 0;
    if (who->IsDocked()) {
        inventoryID = who->GetStationID();
        StationItemRef station = sEntityList.GetStationByID(inventoryID);
        if (station.get() == nullptr)
            throw PyException(MakeCustomError("Cannot find Station Reference for stationID %u", inventoryID));
        inv = station->GetMyInventory();
        if (inv == nullptr)
            throw PyException(MakeCustomError("Cannot find inventory for locationID %u", inventoryID));
        inv->GetInventoryList(invMap);
        item = station.get();
    } else {
        Command_list(who,db,services,args);
        inventoryID = who->GetSystemID();
        SolarSystemRef system = sItemFactory.GetSolarSystem(inventoryID);
        if (system.get() == nullptr)
            throw PyException(MakeCustomError("Cannot find Station Reference for systemID %u", inventoryID));
        inv = system->GetMyInventory();
        if (inv == nullptr)
            throw PyException(MakeCustomError("Cannot find inventory for locationID %u", inventoryID));
        inv->GetInventoryList(invMap);
        item = system.get();
    }

    std::ostringstream str;
    str.clear();
    str << "%s<br>";
    str << "InventoryID %u(%p) (Item %p) has %u items.<br><br>"; //70

    for (auto cur : invMap)
        str << cur.first << "(" << cur.second->flag() << "): " << cur.second->itemName() << "<br>"; // 20 + 70 for name (90)

    int count = invMap.size();
    int size = count * 90;
    size += 70;
    char reply[size];
    snprintf(reply, size, str.str().c_str(), item->itemName().c_str(), inventoryID, inv, item, count);

    who->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_shipinventory(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* this command is used to debug inventory
     * wip.   -allan 15Mar16
     */

    std::map<uint32, InventoryItemRef> invMap;
    invMap.clear();
    uint32 inventoryID = who->GetShipID();
    ShipItemRef ship = sItemFactory.GetShip(inventoryID);
    Inventory* inv = ship->GetMyInventory();
    inv->GetInventoryList(invMap);

    std::ostringstream str;
    str.clear();
    str << "%s<br>"; //40
    str << "InventoryID %u(%p) (Ship %p) has %u items.<br><br>"; //50

    for (auto cur : invMap)
        str << cur.first << "(" << cur.second->flag() << "): " << cur.second->itemName() << "<br>"; // 20 + 40 for name (60)

    int count = invMap.size();
    int size = count * 60;
    size += 90;
    char reply[size];
    snprintf(reply, size, str.str().c_str(), ship->itemName().c_str(), inventoryID, inv, ship.get(), count);

    who->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_skilllist(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* this command is used to debug char skills
     * wip.   -allan 15Mar16
     */

    std::map<uint32, InventoryItemRef> invMap;
    invMap.clear();
    uint32 inventoryID = who->GetCharacterID();
    Inventory* inv = who->GetChar()->GetMyInventory();
    inv->GetInventoryList(invMap);

    std::ostringstream str;
    str.clear();
    str << "InventoryID %u(%p) (%s) has %u skills.<br><br>"; //80

    for (auto cur : invMap) {
        str << cur.first << " - " << cur.second->itemName();    //45
        str  << " (" << cur.second->GetAttribute(AttrSkillLevel).get_int() << ") "; //3
        if (cur.second->GetAttribute(AttrSkillPoints).get_type() == evil_number_int)    //15
            str << "[i-" << cur.second->GetAttribute(AttrSkillPoints).get_int();
        else
            str << "[f-" << cur.second->GetAttribute(AttrSkillPoints).get_float();
        str << "]<br>"; // 45 + 3 + 15 + 5 (70)
    }

    int count = invMap.size();
    int size = count * 80;
    size += 80;
    char reply[size];
    snprintf(reply, size, str.str().c_str(), inventoryID, inv, who->GetChar()->itemName().c_str(), count);

    who->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_attrlist(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* this command is used to debug attributes
     * wip.   -allan 15Mar17
     */

    if (!args.isNumber(1))
        throw PyException(MakeCustomError("Argument 1 must be a valid itemID."));
    uint32 itemID = atol(args.arg(1).c_str());

    InventoryItemRef iRef = sItemFactory.GetItem(itemID);
    if (!iRef) {
        // make error msg here
        return new PyNone();
    }

    std::map<uint16, EvilNumber> attrMap;
    iRef->GetAttributeMap()->CopyAttributes(attrMap);

    std::ostringstream str;
    str.clear();
    str << "%u(%s) has %u attributes.<br><br>"; //70

    for (auto cur : attrMap) {
        str << cur.first << " ";  //15
        if (cur.second.get_type() == evil_number_int)    //15
            str << "i- " << cur.second.get_int();
        else
            str << "f- " << cur.second.get_float();
        str << "<br>"; // 3 + 15 + 15 (40)
    }

    int count = attrMap.size();
    int size = count * 40;
    size += 70;
    char reply[size];
    snprintf(reply, size, str.str().c_str(), itemID, iRef->itemName().c_str(), count);

    who->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_showsession(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {

    std::ostringstream str;
    str.clear();
    str << "Current Session Values.<br><br>"; //32

    str << "charid: %i <br>"; //14+10
    str << "charname: %s <br>"; //16+20
    str << "shipid: %i <br>"; //14+10
    str << "cloneStationID: %i <br>"; //21+10

    str << "clientid: %i <br>"; //16+10
    str << "userid: %i <br>"; //14+10
    str << "sessionID: %li <br>"; //18+20

    str << "locationid: %i <br>"; //18+10
    str << "stationid: %i <br>"; //17+10
    str << "stationid2: %i <br>"; //17+10
    str << "solarsystemid2: %i <br>"; //22+10
    str << "constellationid: %i <br>"; //23+10
    str << "regionid: %i <br>";

    str << "corpid: %i <br>"; //14+10
    str << "hqID: %i <br>"; //12+10
    str << "corpAccountKey: %i <br>"; //22+10
    str << "corpRole: %lu <br>"; //17+20
    str << "rolesAtAll: %lu <br>"; //19+20
    str << "rolesAtBase: %lu <br>"; //20+20
    str << "rolesAtHQ: %lu <br>"; //18+20
    str << "rolesAtOther: %lu <br>"; //21+20

    str << "fleetID: %i <br>"; //14+10
    str << "wingID: %i <br>"; //13+10
    str << "squadID: %i <br>"; //14+10
    str << "job: %s <br>"; //10+10
    str << "role: %s <br>"; //11+10
    str << "booster: %s <br>"; //14+10
    str << "joinTime: %lu <br>"; //16+20

    int size = 32;  // header
    size += 445;    // text
    size += 170;    // %i
    size += 140;    // %l*
    size += 50;     // %s
    char reply[size];
    snprintf(reply, size, str.str().c_str(),
             who->GetCharacterID(), who->GetName(), who->GetShipID(), who->GetCloneStationID(), who->GetClientID(), who->GetUserID(),
             who->GetSessionID(), who->GetLocationID(), who->GetStationID(), who->GetStationID2(), who->GetSystemID(), who->GetConstellationID(),
             who->GetRegionID(), who->GetCorporationID(), who->GetCorpHQ(), who->GetCorpAccountKey(), who->GetCorpRole(), who->GetRolesAtAll(),
             who->GetRolesAtBase(), who->GetRolesAtHQ(), who->GetRolesAtOther(), who->GetChar()->fleetID(), who->GetChar()->wingID(),
             who->GetChar()->squadID(), sFltSvc.GetJobName(who->GetChar()->fleetJob()).c_str(), sFltSvc.GetRoleName(who->GetChar()->fleetRole()).c_str(),
             sFltSvc.GetBoosterName(who->GetChar()->fleetBooster()).c_str(),who->GetChar()->fleetJoinTime());

    who->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_shipdna(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    char reply[200];
    snprintf(reply, 200, "%s", who->GetShip()->GetShipDNA().c_str());

    who->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_targlist(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!who->IsInSpace()) {
        who->SendInfoModalMsg("You are not in Space.");
        return nullptr;
    }

    uint16 length = 1, count = 0;
    std::string into = who->GetShipSE()->TargetMgr()->TargetList(length, count);

    std::ostringstream str;
    str.clear();
    str << "Target List for %s in shipID %u<br>"; //30+30
    str << "    %u entries in list<br>";   //30
    str << "%s"; //length

    int size = 60;  // header
    size += 30;    // text
    size += length;

    char reply[size];
    snprintf(reply, size, str.str().c_str(), who->GetName(), who->GetShipID(), count, into.c_str());

    who->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_track(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    bool tracking = sEntityList.GetTracking();
    std::string track = "enabled";
    if (tracking) {
        sEntityList.SetTracking(false);
        track = "disabled";
    } else
        sEntityList.SetTracking(true);

    char reply[30];
    snprintf(reply, 30, "Ship Tracking is %s.", track.c_str());

    who->SendNotifyMsg(reply);
    return new PyString(reply);
}

PyResult Command_bubbletrack(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    bool tracking = sConfig.debug.BubbleTrack;
    std::string track = "enabled";
    if (tracking) {
        sConfig.debug.BubbleTrack = false;
        track = "disabled";
        who->GetShipSE()->SysBubble()->RemoveMarkers();
    } else {
        sConfig.debug.BubbleTrack = true;
        who->GetShipSE()->SysBubble()->MarkCenter();
    }

    char reply[30];
    snprintf(reply, 30, "Bubble Tracking is %s.", track.c_str());

    who->SendNotifyMsg(reply);
    return new PyString(reply);
}

PyResult Command_warpto(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!who->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (!who->GetShipSE()->SysBubble())
        who->EnterSystem(who->GetSystemID());
    if (!who->GetShipSE()->DestinyMgr())
        who->SetDestiny(NULL_ORIGIN);

    /** @todo  finish this.... */
    who->GetShipSE()->DestinyMgr()->Halt();

    char reply[55];
    snprintf(reply, 55, "Command Unavalible.\nShip Halted.");

    who->SendInfoModalMsg(reply);
    return new PyString(reply);
}


PyResult Command_entityspawn(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!who->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (!who->GetShipSE()->SysBubble())
        who->EnterSystem(who->GetSystemID());
    if (!who->GetShipSE()->DestinyMgr())
        who->SetDestiny(NULL_ORIGIN);

//sm.RemoteSvc('slash').SlashCmd('/entityspawn {0} {1} {2} 0 {3}'.format(recipeID, typeID, x, y))
    /** @todo  finish this.... */
    who->GetShipSE()->DestinyMgr()->Halt();

    char reply[55];
    snprintf(reply, 55, "Command Unfinished.\nShip Halted.");

    who->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_fleetboost(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    uint32 fleetID = who->GetChar()->fleetID();

    if (fleetID == 0) {
        who->SendInfoModalMsg("You are not in a fleet");
        return nullptr;
    }

    uint16 length = 1;
    std::string into = sFltSvc.GetBoosterData(fleetID, length);

    std::ostringstream str;
    str.clear();
    str << "<color=aqua>FleetID %u Command and Boost Data Window.</color><br><br>";   //77
    str << "%s"; //length

    int size = 77;  // header
    size += length;

    char reply[size];
    snprintf(reply, size, str.str().c_str(), fleetID, into.c_str());

    who->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_fleetinvite(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!who->InFleet())
        throw PyException(MakeCustomError("You're not in a fleet."));
    if (!who->IsFleetBoss())
        throw PyException(MakeCustomError("You're not fleet boss."));

    if (args.isNumber(1))
        throw PyException(MakeCustomError("Argument 1 should be one of 'None', 'Corp', 'Alliance', 'Faction', or 'All'"));

    char reply[55];
    snprintf(reply, 55, "Command Unfinished.");

    who->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_getposition(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!who->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (!who->GetShipSE()->SysBubble())
        throw PyException(MakeCustomError("You're not in a bubble."));
    if (!who->GetShipSE()->DestinyMgr())
        throw PyException(MakeCustomError("You have no destiny manager."));

    GPoint sPos(who->GetShipSE()->GetPosition());
    GPoint mPos(who->SystemMgr()->GetClosestMoonSE(sPos)->GetPosition());
    GVector vec(sPos, mPos);

    float normProd = sPos.normalize() * mPos.normalize();
    float dotProd = sPos.dotProduct(mPos);
    float angle = std::acos( dotProd / normProd);

    float azimuth = std::atan2(vec.z, vec.x);
    float elevation = std::atan2(vec.y, std::sqrt(std::pow(vec.x,2) + std::pow(vec.z,2)));

    std::ostringstream str;
    str.clear();
    str << "Angle for current position is " << angle << "<br>";
    str << "Az: " << azimuth << " Ele: " << elevation;
    int size = 70;
    char reply[size];
    snprintf(reply, size, str.str().c_str());

    who->SendInfoModalMsg(reply);
    return new PyString(reply);
}


/* groove's new command.....
 *    /fit [me|itemID] [typeID] [flag=slot]
 * then sends ScatterEvent OnRefreshModuleBanks after successful call.
 */

/*defaultMacros = {'GMH: Unload All': '/unload me all',
 ' WM: Remove All Drones': '/unspawn range=500000* only=categoryDrone',
 'WM: Remove All Wrecks': '/unspawn range=500000 only=groupWreck',
 'WM: Remove CargoContainers': '/unspawn range=500000 only=groupCargoContainer',
 'WM: Remove SecureContainers': '/unspawn range=500000 only=groupSecureCargoContainer',
 'HEALSELF: Repair My Ship': '/heal',
 'HEALSELF: Repair My Modules': '/repairmodules',
 'GMH: Online My Modules': '/online me',
 'GML: Session Change 5sec': '/sessionchange 5'}
 */

/*@Allan new command to implement.  I may try but I don't know fuckall for destiny stuff
 * /reportdesync
 *
 * It wants a tuple of (dict, timestamp)
 * dict is:
 * key = ballID
 * val = pos (vector probably best to use util.KeyVal)(uses val.x, val.y, val.z)
 *
 * It compares the balls trying to find position and velocity inconsitancies showing them in a model window
 */
