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
#include "admin/TranslocateHelper.h"
#include "admin/CommandHelper.h"
#include "tables/invGroups.h"
#include "tables/invCategories.h"

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


PyResult Command_tr(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    codelog(COMMAND__ERROR, "/tr issued:");
    for (int i = 0; i < args.argCount(); i++) {
        codelog(COMMAND__ERROR, "  %s", args.arg(i).c_str());
    }
    TRData d = {};
    d.who = who;
    d.db = db;
    d.services = services;

    uint32 victim = 0;
    uint32 dest = 0;
    LocationTag tag = LocationTag_Invalid;

    if (args.argCount() == 2) {
        if (args.isNumber(1)) {
            dest = atoi(args.arg(1).c_str());
            tag = translocate_resolve_id(&d, atoi(args.arg(1).c_str()));
        } else {
            codelog(COMMAND__ERROR, "argument 1: %s", args.arg(1).c_str());
            tag = translocate_resolve_location_name(&d, args.arg(1).c_str(), &dest);
        }
        if (translocate_to(&d, who->GetCharacterID(), dest, tag)) {
            return new PyBool(true);
        }
        codelog(COMMAND__ERROR, "translocate_to failed");
        return new PyBool(false);
    }
    if (args.argCount() == 3) {
        if (args.isNumber(1)) {
            victim = atoi(args.arg(1).c_str());
            tag = translocate_resolve_id(&d, victim);
        } else {
            tag = translocate_resolve_location_name(&d, args.arg(1).c_str(),
                                                    &victim);
        }
        if (args.isNumber(2)) {
            dest = atoi(args.arg(2).c_str());
            tag = translocate_resolve_id(&d, dest);
        } else {
            tag = translocate_resolve_location_name(&d, args.arg(2).c_str(),
                                                    &dest);
        }

        if (translocate_to(&d, victim, dest, tag)) {
            return new PyBool(true);
        }
        codelog(COMMAND__ERROR, "translocate_to failed");
        return new PyBool(false);
    }

    return new PyBool(false);
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

    InventoryItemRef i = sItemFactory.SpawnItem(idata);
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
    return generic_createitem(who, db, services, args);
}

PyResult Command_createitem(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    if (args.argCount() < 2) {
        throw PyException(MakeCustomError("Correct Usage: /createitem [typeID|\"Type Name\"] [qty] [where]"));
    }
    return generic_createitem(who, db, services, args);
}


PyResult Command_kill(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (args.argCount() == 2) {
        if (!args.isNumber(1)) {
            throw PyException(MakeCustomError("Argument 1 should be a character ID"));
        }
        int entity = atoi(args.arg(1).c_str());

        InventoryItemRef itemRef = sItemFactory.GetShip(entity);
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
#define DEFAULT_RANGE 500000
    if (!who->IsInSpace()) {
        throw PyException(MakeCustomError("You must be in space to unspawn things."));
    }

    if (who->GetShipSE() == nullptr) {
            throw PyException(MakeCustomError("/unspawn failed. You don't appear to have a ship?"));
    }
    int target_index = cmd_find_nth_noneq(args, 1);
    uint32 target = 0;
    if (target_index > 0) {
        if (!args.isNumber(target_index)) {
            throw PyException(MakeCustomError("/unspawn called with non number"));
        }
        target = atoi(args.arg(target_index).c_str());
    }
    
    std::string range_str = cmd_parse_eq_arg(args, "range=");
    std::string only_str = cmd_parse_eq_arg(args, "only=");

    codelog(COMMAND__ERROR, "unspawn got: %s %s %u", 
            range_str.c_str(), only_str.c_str(), target);

    uint32 range = DEFAULT_RANGE;

    if (range_str.size() > 0) {
        if (!IsNumber(range_str)) {
            throw PyException(MakeCustomError("/unspawn with range=x must be a number"));
        }
        range = atoi(range_str.c_str());
    }

    if ((range != DEFAULT_RANGE or
            only_str.size() > 0) and
            target != 0) {
            throw PyException(MakeCustomError("/unspawn cannot be called with an explcit target and either range= or only="));
    }

    if (target != 0) {
        InventoryItemRef item_ref = sItemFactory.GetItem(target);
        SystemEntity *sys_entity = who->SystemMgr()->GetSE(target);
        if (sys_entity == nullptr) {
            throw PyException(MakeCustomError("/unspawn failed.  Item %u not found.", target));
        }

        who->SystemMgr()->RemoveEntity(sys_entity);
        item_ref->Delete();
        codelog(COMMAND__MESSAGE, "/unspawn called with single target successful");
        return new PyBool(true);
    }

    if (only_str.size() == 0) {
        throw PyException(MakeCustomError("/unspawn usage:<br>  /unspawn [itemID]<br>/unspawn only=category|group<br>If using only the default range is 10k.  You can set this by adding range=x in meters"));
    }

    bool is_category_match = false;
    bool is_group_match = false;
    uint16 match_id = 0;

    if (strcmp(only_str.c_str(), "categoryDrone") == 0) {
        match_id = EVEDB::invCategories::Drone;
        is_category_match = true;
    } else if (strcmp(only_str.c_str(), "groupWreck") == 0) {
        match_id = EVEDB::invGroups::Wreck;
        is_group_match = true;
    } else {
        throw PyException(MakeCustomError("only='%s' not a supported group or category", only_str.c_str()));
    }

    SystemBubble *bubble = who->GetShipSE()->SysBubble();
    if (bubble == nullptr) {
        throw PyException(MakeCustomError("/unspawn failed.  You don't appear to be in a bubble.  Try /update"));
    }

    GPoint player_pos = who->GetShipSE()->GetPosition();
    Inventory *sys_inv = who->SystemMgr()->GetSystemInv();

    std::vector<SystemEntity *> entities;
    bubble->GetEntities(entities);
    for (int i = 0; i < entities.size(); i++) {
        SystemEntity *e = entities[i];
        if (is_group_match == true and match_id != e->GetGroupID()) {
            codelog(COMMAND__ERROR, "m: g%d c%d skipping match_id %u groupID %u", 
                    is_group_match, is_category_match, match_id, e->GetGroupID());
            continue;
        }
        if (is_category_match == true and match_id != e->GetCategoryID()) {

            codelog(COMMAND__ERROR, "m: g%d c%d skipping match_id %u categoryID %u", 
                    is_group_match, is_category_match, match_id, e->GetGroupID());
            continue;
        }

        uint32_t itemID = e->GetID();
        codelog(COMMAND__ERROR, "Grid item: %u passed initial checks", itemID);
        GPoint pos = e->GetPosition();
        float d = player_pos.distance(pos);
        if (d > (float)range) {
            continue;
        }


        who->SystemMgr()->RemoveEntity(e);
        InventoryItemRef item = sItemFactory.GetItem(itemID);
        item->Delete();
    }

#undef DEFAULT_RANGE

    return new PyBool(true);
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
