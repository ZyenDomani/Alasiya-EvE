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
    TRData d = {};
    d.who = who;
    d.db = db;
    d.services = services;

    uint32 victim = 0;
    uint32 dest = 0;
    LocationTag tag = LocationTag_Invalid;

    if (args.argCount() == 2) {
        if (args.isNumber(1)) {
            tag = translocate_resolve_id(&d, atoi(args.arg(1).c_str()));
        } else {
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
            tag = translocate_resolve_id(&d, atoi(args.arg(1).c_str()));
        } else {
            tag = translocate_resolve_location_name(&d, args.arg(1).c_str(),
                                                    &victim);
        }
        if (args.isNumber(2)) {
            tag = translocate_resolve_id(&d, atoi(args.arg(2).c_str()));
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
