/*
 * SystemCommands.cpp
 *   this file is commands related to an item's creation/deletion and location/position
 *
 */

#include "eve-server.h"

#include "Client.h"
#include "ConsoleCommands.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "admin/AllCommands.h"
#include "admin/CommandDB.h"
#include "inventory/AttributeEnum.h"
#include "inventory/InventoryDB.h"
#include "inventory/InventoryItem.h"
#include "manufacturing/Blueprint.h"
#include "map/MapConnections.h"
#include "npc/Drone.h"
#include "system/Damage.h"
#include "system/DestinyManager.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"
#include "system/cosmicMgrs/BeltMgr.h"

PyResult Command_goto(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (args.argCount() != 4
        || !args.isNumber(1)
        || !args.isNumber(2)
        || !args.isNumber(3))
    {
        throw PyException(MakeCustomError("Correct Usage: /goto [x coord] [y coor] [z coord]"));
    }

    GPoint p(atoll(args.arg(1).c_str()),
             atoll(args.arg(2).c_str()),
             atoll(args.arg(3).c_str()));

    sLog.White("Command", "%s: Goto (%.13f, %.13f, %.13f)", who->GetName(), p.x, p.y, p.z);

    who->MoveToPosition(p);
    return new PyString("Goto successful.");
}

PyResult Command_translocate(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    return Command_tr(who,db,services,args);
}


PyResult Command_tr(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    std::string usageString =
    "Correct Usage:<br><br>\
    - General Notes:<br>\
    + tr is same as translocate command<br>\
    + object being teleported MUST be in space<br>\
    + destination object MUST be in space<br>\
    + 'entityID #1' MUST be a currently logged-in character<br>\
    + 'entityID #2' can be a character, ship, NPC, station, belt, stargate, or solar system<br>\
    + 'me' string is allowed for [character name] to indicate YOU being teleported<br><br>\
    .tr [entityID #1] - teleport YOU to 'entityID'<br>\
    .tr [character name] - teleport YOU to 'character name'<br>\
    .tr [locationID] - teleport YOU into 'locationID'<br>\
    .tr [solar system name] - teleport YOU into 'solar system name' system<br>\
    .tr [entityID #1|character name] [entityID #2|character name|locationID|solar system name] - teleport 'entityID #1' or 'character name' to 'entityID #2', 'character name' or solar system<br>\
    .tr x y z - teleport YOU to a specific (x,y,z) coordinate in the current solar system<br>\
    ";

    // Error if there are NO arguments past first string "tr" or "translocate":
    if (args.argCount() < 2) {
        throw PyException(MakeCustomError(usageString.c_str()));
    }

    // Argument Discovery
    Client* p_targetClient(nullptr);
    SystemEntity* destinationEntity(nullptr);
    int locationID = 0, trMode = 0;
    GPoint destinationPoint(NULL_ORIGIN);
    uint32 argsCount = args.argCount();
    std::string name1 = args.arg(1);
    std::string name2 = "";
    bool isFirstArgName = false, isSecondArgName = false;
    enum TR_MODE
    {
        TR_MODE_ME_TO_ENTITY = 1,
        TR_MODE_ME_TO_CHARACTER = 2,
        TR_MODE_ME_TO_SOLARSYSTEMID = 3,
        TR_MODE_ME_TO_SOLARSYSTEM = 4,
        TR_MODE_ENTITY_TO_ENTITY = 5,
        TR_MODE_ENTITY_TO_CHARACTER = 6,
        TR_MODE_ENTITY_TO_SOLARSYSTEMID = 7,
        TR_MODE_ENTITY_TO_SOLARSYSTEM = 8,
        TR_MODE_CHARACTER_TO_ENTITY = 9,
        TR_MODE_CHARACTER_TO_CHARACTER = 10,
        TR_MODE_CHARACTER_TO_SOLARSYSTEMID = 11,
        TR_MODE_CHARACTER_TO_SOLARSYSTEM = 12
    };

    isFirstArgName = args.isNumber(1) ? false : true;

    // First, determine nature of First argument
    if (isFirstArgName) {
        // First argument is a string, find out if it's a character or solar system:
        if ((name1 == "me") && (argsCount < 3))
            throw PyException(MakeCustomError(std::string(usageString+"<br><br>FIRST ARGUMENT WAS 'me' BUT MISSING SECOND ARGUMENT!").c_str()));

        if (name1 == "me")
            p_targetClient = who;
        else {
            // First argument is a string of a character or solar system:
            //TODO
        }
    } else {
        // First argument is a number, find out if it's a character, ship, NPC, station, belt, stargate, or solar system:
        //TODO
        p_targetClient = who;
        locationID = atoi(args.arg(1).c_str());
        SystemGPoint m_gp;
        if (IsSolarSystem(locationID))
            destinationPoint = m_gp.GetRandPointOnMoon(locationID);//GPoint(12457894200.0f, 17254864800.0f, 14851254800.0f);
            /*
        } else if (IsPlayerItem(locationID)) {
            destinationPoint = who->SystemMgr()->GetSE(locationID)->GetPosition();
            locationID = who->GetLocationID();
        }*/

        trMode = 1;
    }

    if (trMode == 0)
        throw PyException(MakeCustomError(std::string(usageString+"<br><br>UNABLE TO DETERMINE FORMAT OF ARGUMENTS 1 and 2!").c_str()));


    if (argsCount == 3) {
        // We are transporting either THIS client 'who' or some other entity or character to somewhere:
        name2 = args.arg(2);
        isSecondArgName = args.isNumber(2) ? false : true;

        // Determine nature of Second argument
        if (isSecondArgName) {
            // Second argument is a string, find out if it's a character or solar system:
            //TODO
            throw PyException(MakeCustomError(std::string(usageString+"<br><br>NOT SUPPORTED YET!").c_str()));
        } else {
            // Second argument is a number, find out if it's a character, ship, NPC, station, belt, stargate, or solar system:
            //TODO
            throw PyException(MakeCustomError(std::string(usageString+"<br><br>NOT SUPPORTED YET!").c_str()));
        }
    }

    if (argsCount == 4) {
        // SPECIAL CASE:  We are transporting ourselves to a specific (x,y,z) coordinate in the current solar system:
        p_targetClient = who;
        locationID = who->GetLocationID();
        if (!IsSolarSystem(locationID))
            throw PyException(MakeCustomError(std::string(usageString+"<br><br>YOU MUST BE IN SPACE!").c_str()));

        if (args.isNumber(1) && args.isNumber(2) && args.isNumber(3))
            destinationPoint = GPoint(atoll(args.arg(1).c_str()), atoll(args.arg(2).c_str()), atoll(args.arg(3).c_str()));
    }

    //  in case ap is set, unset it, as it will do odd things when undocking or loging in
    p_targetClient->SetAutoPilot(false);

    // We're still going, so we know now we have a target to translocate AND a destination solar system AND destination coordinates, so let's do the translocate:
    //    p_targetClient - target character in a ship to translocate (Client *)
    //    locationID - destination solar system ID
    //    destinationPoint - destination coordinates (GPoint)

    //if (!p_targetClient->GetShipSE())
    //    p_targetClient->CreateShipSE();
    if (IsSolarSystem(locationID) and p_targetClient->GetShipSE() and p_targetClient->GetShipSE()->DestinyMgr())
        p_targetClient->GetShipSE()->DestinyMgr()->SendJumpOutEffect("effects.JumpOut", locationID);

    p_targetClient->MoveToLocation(locationID, destinationPoint);
    //p_targetClient->SetClientTimer(ClientState::csJump, ClientTimers::JumpingTimer);
    if (p_targetClient->GetShipSE() and p_targetClient->GetShipSE()->DestinyMgr())
        p_targetClient->GetShipSE()->DestinyMgr()->SendJumpInEffect("effects.JumpIn");

    return new PyString("Translocation successful.");
}

static PyResult generic_createitem(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {

    int typeID = -1;
    if (args.isNumber(1)) {
        typeID = atoi(args.arg(1).c_str());
    } else {
        std::map<uint32_t, std::string> matches;
        if (!db->ItemSearch(args.arg(1).c_str(), matches)) {
            throw PyException(MakeCustomError("Item not found"));
        }

        if (matches.size() > 1) {
            auto c = matches.begin();
            auto e = matches.end();
            for (; c != e; c++) {
                _log(COMMAND__MESSAGE, "Got match: %s\n", c->second.c_str());

                // POSIX standard btw
                if (strcasecmp(c->second.c_str(), args.arg(1).c_str()) == 0) {
                    typeID = c->first;
                }
            }
            if (typeID == -1) {
                throw PyException(MakeCustomError("Item name is ambiguous.  Please use a full item name"));
            }
        } else if (matches.size() == 1) {
            auto cur = matches.begin();
            _log(COMMAND__MESSAGE,
                 "ItemSearch returned type: \"%s\" given \"%s\"\n", 
                 cur->second.c_str(), args.arg(1).c_str());
            typeID = cur->first;
        }
    }
    if (typeID == -1) {
        throw PyException(MakeCustomError("Unable to find valid type to create"));
    }
    

    int qty = 1;
    if (2 < args.argCount()) {
        if (args.isNumber(2))
            qty = atoi(args.arg(2).c_str());
    }

    _log(COMMAND__MESSAGE, "Create %s %u times", args.arg(1).c_str(), qty);

    //create into their cargo hold unless they are docked in a station,
    //then stick it in their hangar instead.
    uint32 locationID;
    EVEItemFlags flag;
    if (who->IsInSpace()) {
        locationID = who->GetShipID();
        flag = flagCargoHold;
    } else {
        locationID = who->GetStationID();
        flag = flagHangar;
    }

    ItemData idata(
        typeID,
        who->GetCharacterID(),
                   0, //temp location
                   flag,
                   qty
    );

    InventoryItemRef i = services->item_factory->SpawnItem(idata);
    if (i.get() == nullptr)
        throw PyException(MakeCustomError("Unable to create item of type %s.", args.arg(1).c_str()));

    //Move to location
    if (who->IsInSpace())
        who->GetShip()->AddItem(flag, i);
    else
        i->Move(locationID, flag, true);

    return new PyInt(i.get()->itemID());
}

PyResult Command_create(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    if (args.argCount() < 2) {
        throw PyException(MakeCustomError("Correct Usage: /create [typeID|\"Type Name\"] [qty] [where]"));
    }
    generic_createitem(who, db, services, args);
}

PyResult Command_createitem(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    if (args.argCount() < 4) {
        throw PyException(MakeCustomError("Correct Usage: /createitem [typeID|\"Type Name\"] [qty] [where]"));
    }
    generic_createitem(who, db, services, args);
}


PyResult Command_kill(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (args.argCount() == 2) {
        if (!args.isNumber(1)) {
            throw PyException(MakeCustomError("Argument 1 should be a character ID"));
        }
        int entity = atoi(args.arg(1).c_str());

        InventoryItemRef itemRef = services->item_factory->GetShip(entity);
        if (itemRef.get() == NULL)
            throw PyException(MakeCustomError("/kill NOT supported on non-ship types at this time"));

        SystemEntity* shipEntity = who->SystemMgr()->GetSE(entity);
        if (shipEntity == nullptr) {
            throw PyException(MakeCustomError("/kill cannot process this object"));
            sLog.Error("GMCommands - Command_kill()", "Cannot process this object, aborting kill: %s [%u]", itemRef->itemName().c_str(), itemRef->itemID());
        } else {
            who->SystemMgr()->RemoveEntity(shipEntity);
            if (shipEntity->IsNPCSE()) {
                NPC* npcEntity = shipEntity->GetNPCSE();
                Damage fatal_blow(who->GetShipSE(),true);
                npcEntity->Killed(fatal_blow);
                delete npcEntity;
            } else {
                Damage fatal_blow(who->GetShipSE(),true);
                shipEntity->Killed(fatal_blow);
                itemRef->Delete();
            }
        }
    } else
        throw PyException(MakeCustomError("Correct Usage: /kill <entityID>"));

    return nullptr;
}

PyResult Command_killallnpcs(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!who->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (args.argCount() != 1)
        throw PyException(MakeCustomError("Correct Usage: /killallnpcs"));
    if (who->GetShipSE() == nullptr)
        throw PyException(MakeCustomError("ShipSE invalid."));
    if (who->GetShipSE()->SysBubble() == nullptr)
        throw PyException(MakeCustomError("SysBubble invalid."));

    std::vector<SystemEntity *> entityVec;
    who->GetShipSE()->SysBubble()->GetEntities(entityVec);
    std::vector<SystemEntity *>::const_iterator cur = entityVec.begin();
    for (; cur != entityVec.end(); ++cur) {
        if (*cur == nullptr)
            continue;
        if ((*cur)->IsNPCSE()) {
            Damage fatal_blow(who->GetShipSE(),true);
            (*cur)->GetNPCSE()->Killed(fatal_blow);
        }
    }

    return nullptr;
}

PyResult Command_unspawn(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!who->IsInSpace())
        throw PyException(MakeCustomError("You must be in space to unspawn things."));

    if ((args.argCount() < 2) || (args.argCount() > 2))
        throw PyException(MakeCustomError("Correct Usage: /unspawn (itemID)"));

    if (!args.isNumber(1))
        throw PyException(MakeCustomError("Argument 1 should be itemID"));

    uint32 itemID = atoi(args.arg(1).c_str());

    // Search for the itemRef for itemID:
    InventoryItemRef itemRef = who->services().item_factory->GetItem(itemID);
    SystemEntity* pSE = who->SystemMgr()->GetSE(itemID);

    // Actually do the unspawn using SystemManager's RemoveEntity:
    if (pSE == nullptr) {
        throw PyException(MakeCustomError("Un-Spawn Failed: itemID %u not found.", itemID));
    } else {
        who->SystemMgr()->RemoveEntity(pSE);
        itemRef->Delete();
    }

    sLog.White("Command", "%s: Un-Spawned %u.", who->GetName(), itemID);

    return new PyString("Un-Spawn successful.");
}

PyResult Command_location(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!who->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (who->GetShipSE()->DestinyMgr() == nullptr)
        who->SetDestiny(NULL_ORIGIN);
    if (who->GetShipSE()->SysBubble() == nullptr)
        who->EnterSystem(who->GetSystemID());

    DestinyManager *dm = who->GetShipSE()->DestinyMgr();
    SystemBubble *pBubble = who->GetShipSE()->SysBubble();
    if (pBubble == nullptr) {
        sBubbleMgr.Add(who->GetShipSE());
        pBubble = who->GetShipSE()->SysBubble();
    }
    uint16 bubble = pBubble->GetID();

    const GPoint &loc = dm->GetPosition();
    const GVector &vel = dm->GetVelocity();

    char reply[140];
    snprintf(reply, 140,
             "SystemID: %u  BubbleID: %u<br>"
             "x: %.2f<br>"
             "y: %.2f<br>"
             "z: %.2f<br>"
             "speed: %.1f",
             who->GetSystemID(), bubble,
             loc.x, loc.y, loc.z,
             vel.length()
    );

    who->SendInfoModalMsg(reply);

    return new PyString(reply);
}

PyResult Command_syncloc(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!who->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (who->GetShipSE()->DestinyMgr() == nullptr)
        who->SetDestiny(NULL_ORIGIN);
    if (who->GetShipSE()->SysBubble() == nullptr)
        who->EnterSystem(who->GetSystemID());

    who->GetShipSE()->DestinyMgr()->SetPosition(who->GetShipSE()->GetPosition(), true);

    return new PyString("Position synchronized.");
}

PyResult Command_update(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
    if (!who->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (who->GetShipSE()->DestinyMgr() == nullptr)
        who->SetDestiny(NULL_ORIGIN);
    if (who->GetShipSE()->SysBubble() == nullptr)
        who->EnterSystem(who->GetSystemID());

    who->GetShipSE()->DestinyMgr()->SetPosition(who->GetShipSE()->GetPosition(), true);

    SystemBubble *pBubble = who->GetShipSE()->SysBubble();
    if (pBubble == nullptr) {
        sBubbleMgr.Add(who->GetShipSE());
        pBubble = who->GetShipSE()->SysBubble();
    }
    pBubble->SendAddBalls(who->GetShipSE());

    who->SetStateSent(false);
    who->GetShipSE()->DestinyMgr()->SendSetState();
    return new PyString("Update sent.");
}

PyResult Command_sendstate(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
    if (!who->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (who->GetShipSE()->DestinyMgr() == nullptr)
        who->SetDestiny(NULL_ORIGIN);
    if (who->GetShipSE()->SysBubble() == nullptr)
        who->EnterSystem(who->GetSystemID());

    who->SetStateSent(false);
    who->GetShipSE()->DestinyMgr()->SendSetState();
    return new PyString("Update sent.");
}

PyResult Command_addball(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
    if (!who->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (who->GetShipSE()->DestinyMgr() == nullptr)
        who->SetDestiny(NULL_ORIGIN);
    if (who->GetShipSE()->SysBubble() == nullptr)
        who->EnterSystem(who->GetSystemID());

    SystemBubble *pBubble = who->GetShipSE()->SysBubble();
    if (pBubble == nullptr) {
        sBubbleMgr.Add(who->GetShipSE());
        pBubble = who->GetShipSE()->SysBubble();
    }
    pBubble->SendAddBalls(who->GetShipSE());

    return new PyString("Update sent.");
}

PyResult Command_addball2(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
    if (!who->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (who->GetShipSE()->DestinyMgr() == nullptr)
        who->SetDestiny(NULL_ORIGIN);
    if (who->GetShipSE()->SysBubble() == nullptr)
        who->EnterSystem(who->GetSystemID());

    SystemBubble *pBubble = who->GetShipSE()->SysBubble();
    if (pBubble == nullptr) {
        sBubbleMgr.Add(who->GetShipSE());
        pBubble = who->GetShipSE()->SysBubble();
    }
    pBubble->SendAddBalls2(who->GetShipSE());

    return new PyString("Update sent.");
}

PyResult Command_cloak(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (args.argCount() == 1) {
        if (who->IsInSpace()) {
            if (who->GetShipSE()->DestinyMgr()->IsCloaked())
                who->GetShipSE()->DestinyMgr()->UnCloak();
            else
                who->GetShipSE()->DestinyMgr()->Cloak();
        } else
            throw PyException(MakeCustomError("ERROR!  You MUST be in space to cloak!"));
    } else
        throw PyException(MakeCustomError("Correct Usage: /cloak"));

    return nullptr;
}

PyResult Command_hop(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /*22:49:01 W GMCommands: Command_hop(): This command passes args.argCount() = 1.
     * sm.RemoteSvc('slash').SlashCmd('/hop %s' % distance)
     */
    return nullptr;
}

//13:54:11 W GMCommands: Command_sov(): This command passes args.argCount() = 3.
PyResult Command_sov(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    sLog.Warning("GMCommands: Command_sov()", "This command passes args.argCount() = %u.", args.argCount());
    /*
     *  ' /sov complete ' + str(itemID)
     */
    /*
     * 16:40:32 [CmdDump]   Call Arguments:
     * 16:40:32 [CmdDump]       Tuple: 1 elements
     * 16:40:32 [CmdDump]         [ 0] String: '/sov complete 140035963'
     */
    return nullptr;
}

//13:54:11 W GMCommands: Command_pos(): This command passes args.argCount() = 3.
PyResult Command_pos(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    sLog.Warning("GMCommands: Command_pos()", "This command passes args.argCount() = %u.", args.argCount());

    /*
     * ' /pos online ' + str(itemID)
     * ' /pos unanchor ' + str(itemID)
     * ' /pos anchor ' + str(itemID)
     * ' /pos offline ' + str(itemID)
     * ' /pos fuel [itemID]
     */

    /*
     * 16:39:26 [CmdDump]   Call Arguments:
     * 16:39:26 [CmdDump]       Tuple: 1 elements
     * 16:39:26 [CmdDump]         [ 0] String: '/pos offline 140035963'
     */
    return nullptr;
}
