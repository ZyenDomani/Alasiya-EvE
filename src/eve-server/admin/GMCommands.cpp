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
    Updates:        Allan
*/

/** @todo this file needs to be updated... */


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
#include "ship/DestinyManager.h"
#include "ship/Drone.h"
#include "system/Damage.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"
#include "system/cosmicMgrs/BeltMgr.h"

PyResult Command_create(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    if (args.argCount() < 2) {
        throw PyException(MakeCustomError("Correct Usage: /create [typeID]"));
    }

    if (!args.isNumber(1))
        throw PyException(MakeCustomError("Argument 1 must be type ID."));
    const uint32 typeID = atoi(args.arg(1).c_str());

    uint32 qty = 1;
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
    if (!i)
        throw PyException(MakeCustomError("Unable to create item of type %s.", args.arg(1).c_str()));

    //Move to location
    if (who->IsInSpace())
        who->GetShip()->AddItem(flag, i);
    else
        i->Move(locationID, flag, true);

    return new PyString("Creation successful.");
}

PyResult Command_createitem(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    if (args.argCount() < 2) {
        throw PyException(MakeCustomError("Correct Usage: /create [typeID]"));
    }

    //basically, a copy/paste from Command_create. The client seems to call this multiple times,
    //each time it creates an item
    if (!args.isNumber(1))
        throw PyException(MakeCustomError("Argument 1 must be type ID."));
    const uint32 typeID = atoi(args.arg(1).c_str());

    uint32 qty = 1;
    if (2 < args.argCount()) {
        if (args.isNumber(2))
            qty = atoi(args.arg(2).c_str());
    }

    sLog.Log("command message", "Create %s %u times", args.arg(1).c_str(), qty);

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
    if (!i)
        throw PyException(MakeCustomError("Unable to create item of type %s.", args.arg(1).c_str()));

    //Move to location
    i->Move(locationID, flag, true);

    return new PyString("Creation successful.");
}


PyResult Command_search(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    if (args.argCount() < 2) {
        throw PyException(MakeCustomError("Correct Usage: /search [text]"));
    }

    const std::string& query = args.arg(1);

    //an empty query is a bad idea.
    if (query.length() == 0)
        throw PyException(MakeCustomError("Usage: /search [text]"));

    std::map<uint32, std::string> matches;
    if (!db->ItemSearch(query.c_str(), matches))
        throw PyException(MakeCustomError("Failed to query DB."));

    std::string result(itoa(matches.size()));
    result += " matches found.<br>";

    std::map<uint32, std::string>::iterator cur, end;
    cur = matches.begin();
    end = matches.end();
    for(; cur != end; cur++) {
        result += itoa(cur->first);
        result += ": ";
        result += cur->second;
        result += "<br>";
    }

    if (10 < matches.size()) {
        //send the results in an evemail.
        std::string subject("Search results for ");
        subject += query;

        who->SelfEveMail(subject.c_str(), result.c_str());

        return new PyString("Results sent via evemail.");
    } else
        return new PyString(result);
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
        .tr [solarSystemID] - teleport YOU into 'solarSystemID' system<br>\
        .tr [solar system name] - teleport YOU into 'solar system name' system<br>\
        .tr [entityID #1|character name] [entityID #2|character name|solarSystemID|solar system name] - teleport 'entityID #1' or 'character name' to 'entityID #2', 'character name' or solar system<br>\
        .tr x y z - teleport YOU to a specific (x,y,z) coordinate in the current solar system<br>\
        ";

    // Error if there are NO arguments past first string "tr" or "translocate":
    if (args.argCount() < 2) {
        throw PyException(MakeCustomError(usageString.c_str()));
    }

    // Argument Discovery
    Client * p_targetClient = NULL;
    SystemEntity * destinationEntity = NULL;
    uint32 solarSystemID = 0;
    GPoint destinationPoint(0,0,0);
    uint32 argsCount = args.argCount();
    std::string name1 = args.arg(1);
    std::string name2 = "";
    bool isFirstArgName = false;
    bool isSecondArgName = false;
    uint32 trMode = 0;
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
    if (isFirstArgName)
    {
        // First argument is a string, find out if it's a character or solar system:
        if ((name1 == "me") && (argsCount < 3))
            throw PyException(MakeCustomError(std::string(usageString+"<br><br>FIRST ARGUMENT WAS 'me' BUT MISSING SECOND ARGUMENT!").c_str()));

        if (name1 == "me")
            p_targetClient = who;
        else
        {
            // First argument is a string of a character or solar system:
            //TODO
        }
    }
    else
    {
        // First argument is a number, find out if it's a character, ship, NPC, station, belt, stargate, or solar system:
        //TODO
        p_targetClient = who;
        solarSystemID = atoi(args.arg(1).c_str());
        destinationPoint = GPoint(12457894200.0f, 17254864800.0f, 14851254800.0f);
        trMode = 1;
    }

    if (trMode == 0)
        throw PyException(MakeCustomError(std::string(usageString+"<br><br>UNABLE TO DETERMINE FORMAT OF ARGUMENTS 1 and 2!").c_str()));


    if (argsCount == 3)
    {
        // We are transporting either THIS client 'who' or some other entity or character to somewhere:
        name2 = args.arg(2);
        isSecondArgName = args.isNumber(2) ? false : true;

        // Determine nature of Second argument
        if (isSecondArgName)
        {
            // Second argument is a string, find out if it's a character or solar system:
            //TODO
            throw PyException(MakeCustomError(std::string(usageString+"<br><br>NOT SUPPORTED YET!").c_str()));
        }
        else
        {
            // Second argument is a number, find out if it's a character, ship, NPC, station, belt, stargate, or solar system:
            //TODO
            throw PyException(MakeCustomError(std::string(usageString+"<br><br>NOT SUPPORTED YET!").c_str()));
        }
    }

    if (argsCount == 4)
    {
        // SPECIAL CASE:  We are transporting ourselves to a specific (x,y,z) coordinate in the current solar system:
        p_targetClient = who;
        solarSystemID = who->GetLocationID();
        if (!IsSolarSystem(solarSystemID))
            throw PyException(MakeCustomError(std::string(usageString+"<br><br>YOU MUST BE IN SPACE!").c_str()));

        if (args.isNumber(2) && args.isNumber(3) && args.isNumber(4))
            destinationPoint = GPoint(atoi(args.arg(2).c_str()), atoi(args.arg(3).c_str()), atoi(args.arg(4).c_str()));
    }

    //  in case ap is set, unset it, as it will do odd things when undocking or loging in
    p_targetClient->SetAutoPilot(false);

    // We're still going, so we know now we have a target to translocate AND a destination solar system AND destination coordinates, so let's do the translocate:
    //    p_targetClient - target character in a ship to translocate (Client *)
    //    solarSystemID - destination solar system ID
    //    destinationPoint - destination coordinates (GPoint)

    if (!p_targetClient->GetShipSE())
        p_targetClient->CreateShipSE();
    if (p_targetClient->GetShipSE()->DestinyMgr())
        p_targetClient->GetShipSE()->DestinyMgr()->SendJumpOutEffect("effects.JumpOut", solarSystemID);

    p_targetClient->MoveToLocation(solarSystemID, destinationPoint);
    if (p_targetClient->GetShipSE()->DestinyMgr())
        p_targetClient->GetShipSE()->DestinyMgr()->SendJumpInEffect("effects.JumpIn");

    return new PyString("Translocation successful.");
}

PyResult Command_giveisk(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{

    if (args.argCount() < 3) {
        throw PyException(MakeCustomError("Correct Usage: /giveisk [entityID ('me'=self)] [amount]"));
    }

    // Check for target (arg #1) for either a number or the string "me":
    std::string target = "";
    if (!args.isNumber(1))
    {
        target = args.arg(1);
        if (target != "me")
            throw PyException(MakeCustomError("Argument 1 should be an entity ID ('me'=self)"));
    }

    // If target (arg #1) is not the string "me" then decode number from argument string, otherwise get this character's ID:
    uint32 entity;
    if (target == "")
        entity = atoi(args.arg(1).c_str());
    else
        entity = who->GetCharacterID();

    if (!args.isNumber(2))
        throw PyException(MakeCustomError("Argument 2 should be an amount of ISK"));
    double amount = strtod(args.arg(2).c_str(), NULL);

    Client* tgt;
    if (entity >= EVEMU_MINIMUM_ID)
    {
        tgt = sEntityList.FindClientByCharID(entity);
        if (!tgt)
            throw PyException(MakeCustomError("Unable to find character %u", entity));
    }
    else
        throw PyException(MakeCustomError("Invalid entityID for characters %u", entity));

    tgt->AddBalance(amount);
    return new PyString("Operation successful.");
}

PyResult Command_pop(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (4 != args.argCount())
        throw PyException(MakeCustomError("Correct Usage: /pop [message type] [key] [text]"));

    //CustomNotify: notify
    //ServerMessage: msg
    //CustomError: error

    const std::string& msgType = args.arg(1);
    const std::string& key = args.arg(2);
    const std::string& text = args.arg(3);

    Notify_OnRemoteMessage n;
    n.msgType = msgType;
    n.args[ key ] = new PyString(text);

    PyTuple* t = n.Encode();
    who->SendNotification("OnRemoteMessage", "charid", &t);
    PySafeDecRef(t);

    return new PyString("Message sent.");
}

PyResult Command_goto(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (args.argCount() != 4
        || !args.isNumber(1)
        || !args.isNumber(2)
        || !args.isNumber(3))
    {
        throw PyException(MakeCustomError("Correct Usage: /goto [x coord] [y coor] [z coord]"));
    }

    GPoint p(atof(args.arg(1).c_str()),
              atof(args.arg(2).c_str()),
              atof(args.arg(3).c_str()));

    sLog.Log("Command", "%s: Goto (%.13f, %.13f, %.13f)", who->GetName(), p.x, p.y, p.z);

    who->MoveToPosition(p);
    return new PyString("Goto successful.");
}

PyResult Command_spawnn(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    uint32 typeID = 0;
    uint32 actualTypeID = 0;
    std::string actualTypeName = "";
    uint32 actualGroupID = 0;
    uint32 actualCategoryID = 0;
    double actualRadius = 0.0;
    InventoryItemRef item;
    ShipItemRef ship;

    // "/spawnn" arguments:
    // #1 = quantity ?
    // #2 = some double value ?
    // #3 = typeID

    if ((args.argCount() < 4) || (args.argCount() > 4))
    {
        throw PyException(MakeCustomError("LOL, we don't know the correct usage of /spawnn, sorry you're S.O.L., BUT it should have 4 arguments."));
    }

    // Since we don't know what args 1 and 2 are, we don't care about them right now...

    if (!args.isNumber(3))
        throw PyException(MakeCustomError("Argument 3 should be an item type ID"));

    typeID = atoi(args.arg(3).c_str());

    if (!who->IsInSpace())
        throw PyException(MakeCustomError("You must be in space to spawn things."));

    // Search for item type using typeID:
    if (!db->ItemSearch(typeID, actualTypeID, actualTypeName, actualGroupID, actualCategoryID, actualRadius)) {
        return new PyString("Unknown typeID or typeName returned no matches.");
    }

    GPoint loc(who->GetShipSE()->GetPosition());
    // Calculate a random coordinate on the sphere centered on the player's position with
    // a radius equal to the radius of the ship/celestial being spawned times 10 for really good measure of separation:
    double radius = (actualRadius * 5.0) * (double)(MakeRandomInt(1, 3));     // Scale the distance from player that the object will spawn to between 10x and 15x the object's radius
    loc.MakeRandomPointOnSphere(radius);

    // Spawn the item:
    ItemData idata(
        actualTypeID,
        1, // owner is EVE System
        who->GetLocationID(),
        flagAutoFit,
        actualTypeName.c_str(),
        loc
   );

    item = services->item_factory->SpawnItem(idata);
    if (!item)
        throw PyException(MakeCustomError("Unable to spawn item of type %u.", typeID));

    DBSystemDynamicEntity entity;
        entity.allianceID = 0;
        entity.categoryID = actualCategoryID;
        entity.corporationID = 0;
        entity.flag = 0;
        entity.groupID = actualGroupID;
        entity.itemID = item->itemID();
        entity.itemName = actualTypeName;
        entity.locationID = who->GetLocationID();
        entity.ownerID = 1;
        entity.typeID = actualTypeID;
        entity.x = loc.x;
        entity.y = loc.y;
        entity.z = loc.z;

    // Actually do the spawn using SystemManager's BuildEntity:
    if (!who->SystemMgr()->BuildDynamicEntity(entity))
        return new PyString("Spawn Failed: typeID or typeName not supported.");

    sLog.Log("Command", "%s: Spawned %u.", who->GetName(), typeID);

    return new PyString("Spawn successful.");
}

PyResult Command_spawn(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    uint32 typeID = 0;
    uint32 spawnCount = 1;
    uint32 spawnIndex = 0;
    uint32 maximumSpawnCountAllowed = 100;
    uint32 actualTypeID = 0;
    std::string actualTypeName = "";
    uint32 actualGroupID = 0;
    uint32 actualCategoryID = 0;
    double actualRadius = 0.0;
    InventoryItemRef item;
    ShipItemRef ship;
    double radius;
    bool offsetLocationSet = false;
    std::string usage = "Correct Usage: <br><br> /spawn [typeID(int)/typeName(string)] <br><br>With optional spawn count: <br> /spawn [typeID(int)/typeName(string)] [count] <br><br>With optional count and (X,Y,Z) coordinate: <br> /spawn [typeID(int/typeName(string)] [count] [x(float)] [y(float)] [z(float)]";

    if (!who->IsInSpace())
        throw PyException(MakeCustomError("You must be in space to spawn things."));

    if (args.argCount() < 2) {
        throw PyException(MakeCustomError(usage.c_str()));
    }

    if (!args.isNumber(1))
        throw PyException(MakeCustomError("Argument 1 should be an item type ID"));

    typeID = atoi(args.arg(1).c_str());

    // Search for item type using typeID:
    if (!(db->ItemSearch(typeID, actualTypeID, actualTypeName, actualGroupID, actualCategoryID, actualRadius)))
    {
        return new PyString("Unknown typeID or typeName returned no matches.");
    }

    if (args.argCount() > 2)
    {
        if (!(args.isNumber(2)))
            throw PyException(MakeCustomError("Argument 3 should be the number of spawns of this type you want to create"));

        spawnCount = atoi(args.arg(2).c_str());
        if (spawnCount > maximumSpawnCountAllowed)
            throw PyException(MakeCustomError("Argument 3, spawn count, is allowed to be no more than 100"));
    }

    // Check to see if the X Y Z optional coordinates were supplied with the command:
    GPoint offsetLocation;
    if (args.argCount() > 3)
    {
        if (!(args.isNumber(3)))
            throw PyException(MakeCustomError("Argument 4 should be the X distance from your ship in meters you want the item spawned"));

        if (args.argCount() > 4)
        {
            if (!(args.isNumber(4)))
                throw PyException(MakeCustomError("Argument 5 should be the Y distance from your ship in meters you want the item spawned"));
        }
        else
            throw PyException(MakeCustomError("TOO FEW PARAMETERS: %s", usage.c_str()));

        if (args.argCount() > 5)
        {
            if (!(args.isNumber(5)))
                throw PyException(MakeCustomError("Argument 6 should be the Z distance from your ship in meters you want the item spawned"));
        }
        else
            throw PyException(MakeCustomError("TOO FEW PARAMETERS: %s", usage.c_str()));

        offsetLocation.x = atoi(args.arg(3).c_str());
        offsetLocation.y = atoi(args.arg(4).c_str());
        offsetLocation.z = atoi(args.arg(5).c_str());
        offsetLocationSet = true;
    }

    GPoint loc;

    for(spawnIndex=0; spawnIndex < spawnCount; spawnIndex++)
    {
        loc = who->GetShipSE()->GetPosition();

        if (offsetLocationSet)
        {
            // An X, Y, Z coordinate offset was specified along with the command, so use this to calculate
            // the final cooridnate of the newly spawned item:
            loc.x += offsetLocation.x;
            loc.y += offsetLocation.y;
            loc.z += offsetLocation.z;
        }
        else
        {
            // Calculate a random coordinate on the sphere centered on the player's position with
            // a radius equal to the radius of the ship/celestial being spawned times 10 for really good measure of separation:
            radius = (actualRadius * 5.0) * (double)(MakeRandomInt(1, 3));     // Scale the distance from player that the object will spawn to between 10x and 15x the object's radius
            loc.MakeRandomPointOnSphere(radius);
        }

        // Spawn the item:
        ItemData idata(
            actualTypeID,
            1, // owner is EVE System
            who->GetLocationID(),
            flagAutoFit,
            actualTypeName.c_str(),
            loc
       );

        item = services->item_factory->SpawnItem(idata);
        if (!item)
            throw PyException(MakeCustomError("Unable to spawn item of type %u.", typeID));

        DBSystemDynamicEntity entity;
        entity.allianceID = 0;
        entity.categoryID = actualCategoryID;
        entity.corporationID = 0;
        entity.flag = 0;
        entity.groupID = actualGroupID;
        entity.itemID = item->itemID();
        entity.itemName = actualTypeName;
        entity.locationID = who->GetLocationID();
        entity.ownerID = 1;
        entity.typeID = actualTypeID;
        entity.x = loc.x;
        entity.y = loc.y;
        entity.z = loc.z;

        // Actually do the spawn using SystemManager's BuildEntity:
        if (!who->SystemMgr()->BuildDynamicEntity(entity))
            return new PyString("Spawn Failed: typeID or typeName not supported.");
    }

    sLog.Log("Command_spawn", "%s: Spawned %u in space, %u times", who->GetName(), typeID, spawnCount);

    return new PyString("Spawn successful.");
}

PyResult Command_location(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (!who->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (!who->GetShipSE()->SysBubble())
        who->EnterSystem(who->GetSystemID());
    if (!who->GetShipSE()->DestinyMgr())
        who->ResetDestiny();

    DestinyManager *dm = who->GetShipSE()->DestinyMgr();
    SystemBubble *b = who->GetShipSE()->SysBubble();
    uint16 bubble = b->GetID();

    const GPoint &loc = dm->GetPosition();
    const GVector &vel = dm->GetVelocity();

    char reply[135];
    snprintf(reply, 135,
        "SystemID: %li (%li)<br>"
        "x: %lf<br>"
        "y: %lf<br>"
        "z: %lf<br>"
        "speed: %lf",
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
    if (!who->GetShipSE()->DestinyMgr())
        who->ResetDestiny();
    if (!who->GetShipSE()->SysBubble())
        who->EnterSystem(who->GetSystemID());

    who->GetShipSE()->DestinyMgr()->SetPosition(who->GetShipSE()->GetPosition(), true);

    return new PyString("Position synchronized.");
}

// command to modify blueprint's attributes, we have to give it blueprint's itemID ...
// isn't much comfortable, but I don't know about better solution ...
PyResult Command_setbpattr(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{

    if (args.argCount() < 6) {
        throw PyException(MakeCustomError("Correct Usage: /setbpattr [blueprintID] [0 (not copy) or 1 (copy)] [material level] [productivity level] [remaining runs]"));
    }

    if (!args.isNumber(1))
        throw PyException(MakeCustomError("Argument 1 must be blueprint ID. (got %s)", args.arg(1).c_str()));
    uint32 blueprintID = atoi(args.arg(1).c_str());

    if ("0" != args.arg(2) && "1" != args.arg(2))
        throw PyException(MakeCustomError("Argument 2 must be 0 (original) or 1 (copy). (got %s)", args.arg(2).c_str()));
    bool copy = (atoi(args.arg(2).c_str()) ? true : false);

    if (!args.isNumber(3))
        throw PyException(MakeCustomError("Argument 3 must be material level. (got %s)", args.arg(3).c_str()));
    uint32 materialLevel = atoi(args.arg(3).c_str());

    if (!args.isNumber(4))
        throw PyException(MakeCustomError("Argument 4 must be productivity level. (got %s)", args.arg(4).c_str()));
    uint32 productivityLevel = atoi(args.arg(4).c_str());

    if (!args.isNumber(5))
        throw PyException(MakeCustomError("Argument 5 must be remaining licensed production runs. (got %s)", args.arg(5).c_str()));
    uint32 licensedProductionRunsRemaining = atoi(args.arg(5).c_str());

    BlueprintRef bp = services->item_factory->GetBlueprint(blueprintID);
    if (!bp)
        throw PyException(MakeCustomError("Failed to load blueprint %u.", blueprintID));

    // these need to check current settings to see if anything changed
    bp->SetCopy(copy);
    bp->SetMaterialLevel(materialLevel);
    bp->SetProductivityLevel(productivityLevel);
    bp->SetLicensedProductionRunsRemaining(licensedProductionRunsRemaining);

    return new PyString("Properties modified.");
}

PyResult Command_state(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
    if (!who->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (!who->GetShipSE()->SysBubble())
        who->EnterSystem(who->GetSystemID());
    if (!who->GetShipSE()->DestinyMgr())
        who->ResetDestiny();

    who->GetShipSE()->DestinyMgr()->SendSetState(who->GetShipSE()->SysBubble(), who->GetShipID());
    return(new PyString("Update sent."));
}

PyResult Command_update(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
    if (!who->IsInSpace())
        throw PyException(MakeCustomError("You're not in space."));
    if (!who->GetShipSE()->SysBubble())
        who->EnterSystem(who->GetSystemID());
    if (!who->GetShipSE()->DestinyMgr())
        who->ResetDestiny();

    who->GetShipSE()->DestinyMgr()->SendSetState(who->GetShipSE()->SysBubble(), who->GetShipID());
    /*
     *    SystemEntity *pCharRef = who->SystemMgr()->get(who->GetCharacterID());
     *    SystemBubble *m_bubble = who->Bubble();
     *    m_bubble->_SendAddBalls(pCharRef);
     */
    return(new PyString("Update sent."));
}

PyResult Command_getattr(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (args.argCount() < 3) {
        throw PyException(MakeCustomError("Correct Usage: /getattr [itemID] [attributeID]"));
    }
    if (!args.isNumber(1))
        throw PyException(MakeCustomError("1st argument must be itemID (got %s).", args.arg(1).c_str()));
    const uint32 itemID = atoi(args.arg(1).c_str());

    if (!args.isNumber(2))
        throw PyException(MakeCustomError("2nd argument must be attributeID (got %s).", args.arg(2).c_str()));
    const ItemAttributeMgr::Attr attribute = (ItemAttributeMgr::Attr)atoi(args.arg(2).c_str());

    InventoryItemRef item = services->item_factory->GetItem(itemID);
    if (!item)
        throw PyException(MakeCustomError("Failed to load item %u.", itemID));

    //return item->attributes.PyGet(attribute);
    return item->GetAttribute(attribute).GetPyObject();
}

PyResult Command_setattr(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (args.argCount() < 4) {
        throw PyException(MakeCustomError("Correct Usage: /setattr [itemID] [attributeID] [value]"));
    }

    // Check for target (arg #1) for either a number or the string "myship":
    uint32 itemID = 0;
    std::string target = "";
    if (!args.isNumber(1))
    {
        target = args.arg(1);
        if (target != "myship")
            throw PyException(MakeCustomError("1st argument should be an entity ID ('myship'=current ship) (got %s).", args.arg(1).c_str()));

        itemID = who->GetShipID();
    }
    else
    {
        // target (arg #1) is a number, so decode it and move on:
        itemID = atoi(args.arg(1).c_str());
    }

    if (!args.isNumber(2))
        throw PyException(MakeCustomError("2nd argument must be attributeID (got %s).", args.arg(2).c_str()));
    const ItemAttributeMgr::Attr attribute = (ItemAttributeMgr::Attr)atoi(args.arg(2).c_str());

    if (!args.isNumber(3))
        throw PyException(MakeCustomError("3rd argument must be value (got %s).", args.arg(3).c_str()));
    const double value = atof(args.arg(3).c_str());

    if (itemID < EVEMU_MINIMUM_ID)
        throw PyException(MakeCustomError("1st argument must be a valid 'entity' table itemID (got %s) that MUST be larger >= 140000000.", args.arg(1).c_str()));

    InventoryItemRef item = services->item_factory->GetItem(itemID);
    if (!item)
        throw PyException(MakeCustomError("Failed to load item %u.", itemID));

    //item->attributes.SetReal(attribute, value);
    sLog.Warning("GMCommands: Command_setattr()", "This command will modify attribute and send change to client, but change does not take effect in client for some reason.");
    item->SetAttribute(attribute, (float)value);

    return new PyString("Operation successful.");
}

PyResult Command_fit(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{

    if (args.argCount() < 2) {
        throw PyException(MakeCustomError("Correct Usage: /fit [typeID] "));
    }

    uint32 typeID = 0;

    if (args.argCount() == 3)
    {
        if (!args.isNumber(2))
            throw PyException(MakeCustomError("Argument 1 must be type ID."));
        typeID = atoi(args.arg(2).c_str());
    }
    else if (args.argCount() == 2)
    {
        if (!args.isNumber(1))
            throw PyException(MakeCustomError("Argument 1 must be type ID."));
        typeID = atoi(args.arg(1).c_str());
    }

    uint32 qty = 1;

    _log(COMMAND__MESSAGE, "Create %s %u times", typeID, qty);

    EVEItemFlags flag;
    uint32 powerSlot;
    uint32 useableSlot;
    std::string affectName = "online";

    if (typeID == 0)
        throw PyException(MakeCustomError("Unable to create item of type %u.", typeID));
    else
    {
        //Get Range of slots for item
        InventoryDB::GetModulePowerSlotByTypeID(typeID, powerSlot);

        //Get open slots available on ship
        InventoryDB::GetOpenPowerSlots(powerSlot, who->GetShip(), useableSlot);

        ItemData idata(
            typeID,
            who->GetCharacterID(),
            0, //temp location
            flag = (EVEItemFlags)useableSlot,
            qty
       );

        InventoryItemRef i = services->item_factory->SpawnItem(idata);
        if (!i)
            throw PyException(MakeCustomError("Unable to create item of type %u.", typeID));

        who->MoveItem(i->itemID(), who->GetShipID(), flag);

        return new PyString("Creation successful.");
    }
}
PyResult Command_giveallskills(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    uint8 level = 5;            // Ensure that ALL skills are trained to level 5
    CharacterRef character;
    uint32 ownerID = 0;
    Client * pTarget = NULL;

    if (args.argCount() >= 2) {
        if (args.isNumber(1)) {
            ownerID = atoi(args.arg(1).c_str());
            pTarget = sEntityList.FindClientByCharID(ownerID);
            if (!pTarget)
                throw PyException(MakeCustomError("ERROR: Cannot find character #%d", ownerID));
            else
                character = pTarget->GetChar();
        } else if (args.arg(1) == "me") {
            ownerID = who->GetCharacterID();
            character = who->GetChar();
            pTarget = who;
        } else if (!args.isNumber(1)) {
            throw PyException(MakeCustomError("The use of string based Character names for this command is not yet supported!  Use 'me' instead or the entityID of the character to which you wish to give skills."));
            /*
             *            const char *name = args.arg(1).c_str();
             *            Client *target = sEntityList.FindCharacter(name);
             *           if (target == NULL)
             *                throw PyException(MakeCustomError("Cannot find Character by the name of %s", name));
             *            ownerID = target->GetCharacterID();
             *            character = target->GetChar();
             */
        } else
            throw PyException(MakeCustomError("Argument 1 must be Character ID or Character Name "));
    } else
        throw PyException(MakeCustomError("Correct Usage: /giveallskills [Character Name or ID]"));

    // Make sure character reference is not NULL before trying to use it:
    if (character.get()) {
        // Query Database to get list of ALL skills, then LOOP through each one, checking character for skill, setting level to 5:
        std::vector<uint32> skillList;
        db->FullSkillList(skillList);

        SkillRef skill;
        uint8 oldLevel = 0;
        uint32 skillID = 0, oldPoints = 0, newPoints = 0;

        std::vector<uint32>::const_iterator cur = skillList.begin();
        for (; cur != skillList.end(); cur++) {
            skillID = *cur;
            if (character->HasSkillTrainedToLevel(skillID, level))
                return new PyNone();
            else if (character->HasSkill(skillID)) {
                skill = character->GetSkill(skillID);
                oldLevel = skill->GetAttribute(AttrSkillLevel).get_int();
                oldPoints = skill->GetAttribute(AttrSkillPoints).get_int();
                skill->SetAttribute(AttrSkillLevel, level);
                skill->SetAttribute(AttrSkillPoints, skill->GetSPForLevel(level));
                if (skill->flag() == flagSkillInTraining) {
                    skill->SetFlag(flagSkill, false);
                    skill->SetAttribute(AttrExpiryTime, 0, false);
                }
            } else {    // Character DOES NOT have this skill
                ItemData idata(skillID, ownerID, ownerID, flagSkill, 1);
                InventoryItemRef item = services->item_factory->SpawnItem(idata);

                if (!item)
                    throw PyException(MakeCustomError("Unable to create item of type %s.", item->typeID()));
                else {
                    skill = SkillRef::StaticCast(item);
                    skill->SetAttribute(AttrSkillLevel, level);
                    skill->SetAttribute(AttrSkillPoints, skill->GetSPForLevel(level));
                }
            }

            //  save gm skill gift in history  -allan
            //  maybe not for this....WAAAAYYY  to much DB traffic for this.
            //character->SaveSkillHistory(skillEventGMGive, EvilTimeNow().get_float(), ownerID, skillID.get_int(), level, \
            skill->GetAttribute(AttrSkillPoints).get_float(), \
            character->GetTotalSP().get_float());
        }
        // END LOOP
        pTarget->SendErrorMsg("You need to relog for skills to get saved and show in character sheet.");
        return new PyString ("Gifting skills complete");
    }

    return new PyString ("Skill Gifting Failure");
}

PyResult Command_giveskills(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    //pass to command_giveskill
    Command_giveskill(who, db, services, args);
    return NULL;
}

PyResult Command_giveskill(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    uint8 level = 0, oldLevel = 0;
    uint32 ownerID = 0, skillID = 0;
    EvilNumber oldPoints = 0;
    int64 newPoints = 0;
    CharacterRef character;
    Client *pTarget = nullptr;

    if (args.argCount() == 4) {
        if (args.isNumber(1)) {
            ownerID = atoi(args.arg(1).c_str());
            character = sEntityList.FindClientByCharID(ownerID)->GetChar();
        } else if (args.arg(1) == "me") {
            ownerID = who->GetCharacterID();
            character = who->GetChar();
            pTarget = who;
        } else if (!args.isNumber(1)) {
            throw PyException(MakeCustomError("The use of string based Character names for this command is not yet supported!  Use 'me' instead or the entityID of the character to which you wish to give skills."));
            const char *name = args.arg(1).c_str();
            pTarget = sEntityList.FindClientByName(name);
            if (!pTarget)
                throw PyException(MakeCustomError("Cannot find Character by the name of %s", name));
            ownerID = pTarget->GetCharacterID();
            character = pTarget->GetChar();
        } else
            throw PyException(MakeCustomError("Argument 1 must be Character ID or Character Name "));

        if (!args.isNumber(2))
            throw PyException(MakeCustomError("Argument 2 must be type ID."));
        skillID = atoi(args.arg(2).c_str());

        if (!args.isNumber(3))
            throw PyException(MakeCustomError("Argument 3 must be level"));
        level = atoi(args.arg(3).c_str());

        if (level > 5) level = 5;    //levels don't go higher than 5

    } else
        throw PyException(MakeCustomError("Correct Usage: /giveskill [CharacterID] [skillID] [desired level]"));

    if (pTarget && character.get()) {       // Make sure references are not NULL before trying to use them:
        SkillRef skill;
        InventoryItemRef item;

        if (character->HasSkillTrainedToLevel(skillID, level))
            return new PyNone();
        else if (character->HasSkill(skillID)) {
            skill = character->GetSkill(skillID);
            oldLevel = skill->GetAttribute(AttrSkillLevel).get_int();
            oldPoints = skill->GetAttribute(AttrSkillPoints);
            EvilNumber tmp = EVIL_SKILL_BASE_POINTS * skill->GetAttribute(AttrSkillTimeConstant) * EvilNumber::pow(2, (2.5*(level -1)));
            newPoints = tmp.get_int();
            skill->SetAttribute(AttrSkillLevel, level);
            skill->SetAttribute(AttrSkillPoints, tmp);

            if (skill->flag() == flagSkillInTraining) {
                skill->SetFlag(flagSkill, false);
                skill->SetAttribute(AttrExpiryTime, 0, false);
            }

            item = services->item_factory->GetItem(skill.get()->itemID());
        } else {    // Character DOES NOT have this skill
            ItemData idata(skillID, ownerID, ownerID, flagSkill, 1);
            item = services->item_factory->SpawnItem(idata);

            if (!item) {
                throw PyException(MakeCustomError("Unable to create item for skillID %u.", skillID));
                return new PyString ("Skill Gifting Failure - Unable to create item for skillID %u.", skillID);
            } else {
                skill = SkillRef::StaticCast(item);
                EvilNumber tmp = EVIL_SKILL_BASE_POINTS * skill->GetAttribute(AttrSkillTimeConstant) * EvilNumber::pow(2, (2.5*(level - 1)));
                newPoints = tmp.get_int();
                skill->SetAttribute(AttrSkillLevel, level);
                skill->SetAttribute(AttrSkillPoints, tmp);
            }
        }

        item->SaveAttributes();

        // Either way, this character now has this skill trained to the specified level, so inform client:
        character->SendSkillComplete(skill.get(), oldLevel, level, oldPoints, newPoints);

        //  save gm skill gift in history  -allan
        character->SaveSkillHistory(skillEventGMGive, EvilTimeNow().get_float(), ownerID, skillID, level,
                                    skill->GetAttribute(AttrSkillPoints).get_float(), character->GetTotalSP().get_float());

        sLog.Log("Command::GiveSkill", "skill %u upped to level %u.", skillID, level);

        return new PyString ("Skill Gifting Complete");
    } else
        throw PyException(MakeCustomError("ERROR: Unable to validate character object, it was found to be NULL!"));

    return new PyNone;
}

PyResult Command_online(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {

    if (args.argCount() == 2)
    {
        if (strcmp("me", args.arg(1).c_str())!=0)
            if (!args.isNumber(1))
                throw PyException(MakeCustomError("Argument 1 should be an entity ID or me (me=self)"));
            uint32 entity = atoi(args.arg(1).c_str());

        Client* tgt;
        if (strcmp("me", args.arg(1).c_str())==0)
            tgt = who;
        else
        {
            tgt = sEntityList.FindClientByCharID(entity);
            if (!tgt)
                throw PyException(MakeCustomError("Unable to find character %u", entity));
        }

        if (!tgt->InPod())
            tgt->GetShip()->OnlineAll();
        else
            throw PyException(MakeCustomError("Command failed: You can't activate modules while in a pod"));

        return(new PyString("All modules have been put Online"));
    }
    else
        throw PyException(MakeCustomError("Command failed: You got the arguments all wrong!"));
}

PyResult Command_unload(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {

    if (args.argCount() >= 2 && args.argCount() <= 3)
    {
        uint32 item=0,entity=0;

        if (strcmp("me", args.arg(1).c_str())!=0)
            if (!args.isNumber(1))
            {
                throw PyException(MakeCustomError("Argument 1 should be an entity ID or me (me=self)"));
            }
            entity = atoi(args.arg(1).c_str());

        if (args.argCount() ==3)
        {
            if (strcmp("all", args.arg(2).c_str())!=0)
                if (!args.isNumber(2))
                    throw PyException(MakeCustomError("Argument 2 should be an item ID or all"));
                item = atoi(args.arg(2).c_str());
        }

        //select character
        Client* tgt;
        if (strcmp("me", args.arg(1).c_str())==0)
            tgt = who;
        else
        {
            tgt = sEntityList.FindClientByCharID(entity);

            if (!tgt)
                throw PyException(MakeCustomError("Unable to find character %u", entity));
        }

        if (tgt->IsInSpace())
            throw PyException(MakeCustomError("Character needs to be docked!"));

        if (args.argCount() == 3 && strcmp("all", args.arg(2).c_str())!=0)
            tgt->GetShip()->UnloadModule(item);

        if (args.argCount() == 3 && strcmp("all", args.arg(2).c_str())==0)
            tgt->GetShip()->UnloadAllModules();

        return(new PyString("All mModulesMgr have been unloaded"));
    }
    else
        throw PyException(MakeCustomError("Command failed: You got the arguments all wrong!"));
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

PyResult Command_repairmodules(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{

   if (args.argCount()==1)
    {
        who->GetShip()->RepairModules();
    }
   if (args.argCount()==2)
    {
        if (!args.isNumber(1))
            {
                throw PyException(MakeCustomError("Argument 1 should be a character ID"));
            }
        uint32 charID = atoi(args.arg(1).c_str());

        Client *target = sEntityList.FindClientByCharID(charID);
       if (target == NULL)
            throw PyException(MakeCustomError("Cannot find Character by the entity %d", charID));
        target->GetShip()->RepairModules();
    }

    return(new PyString("Modules repaired successful!"));
}

PyResult Command_unspawn(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    uint32 entityID = 0;
    uint32 itemID = 0;

    if ((args.argCount() < 3) || (args.argCount() > 3))
        throw PyException(MakeCustomError("Correct Usage: /unspawn (entityID) (itemID), and for now (entityID) is unused, so just type 0, and use the itemID from the entity table for (itemID)"));

    if (!args.isNumber(1))
        throw PyException(MakeCustomError("Argument 1 should be an item entity ID"));

    if (!args.isNumber(2))
        throw PyException(MakeCustomError("Argument 2 should be an item item ID"));

    entityID = atoi(args.arg(1).c_str());
    itemID = atoi(args.arg(2).c_str());

    if (!who->IsInSpace())
        throw PyException(MakeCustomError("You must be in space to unspawn things."));

    // Search for the itemRef for itemID:
    InventoryItemRef itemRef = who->services().item_factory->GetItem(itemID);
    SystemEntity* entityRef = who->SystemMgr()->get(itemID);

    // Actually do the unspawn using SystemManager's RemoveEntity:
    if (!entityRef) {
        return new PyString("Un-Spawn Failed: itemID not found.");
    } else {
        who->SystemMgr()->RemoveEntity(entityRef);
        itemRef->Delete();
    }

    sLog.Log("Command", "%s: Un-Spawned %u.", who->GetName(), itemID);

    return new PyString("Un-Spawn successful.");
}

PyResult Command_dogma(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    //"dogma" "140019878" "agility" "=" "0.2"

    if (!(args.argCount() == 5)) {
        throw PyException(MakeCustomError("Correct Usage: /dogma [itemID] [attributeName] = [value]"));
    }

    if (!args.isNumber(1)) {
        throw PyException(MakeCustomError("Invalid itemID. \n Correct Usage: /dogma [itemID] [attributeName] = [value]"));
    }
    uint32 itemID = atoi(args.arg(1).c_str());

    if (args.isNumber(2)) {
        throw PyException(MakeCustomError("Invalid attributeName. \n Correct Usage: /dogma [itemID] [attributeName] = [value]"));
    }
    const char *attributeName = args.arg(2).c_str();

    if (!args.isNumber(4)) {
        throw PyException(MakeCustomError("Invalid attribute value. \n Correct Usage: /dogma [itemID] [attributeName] = [value]"));
    }
    double attributeValue = atof(args.arg(4).c_str());

    //get item
    InventoryItemRef item = services->item_factory->GetItem(itemID);

    //get attributeID
    uint32 attributeID = db->GetAttributeID(attributeName);

    sLog.Warning("GMCommands: Command_dogma()", "This command will modify attribute and send change to client, but change does not take effect in client for some reason.");
    item->SetAttribute(attributeID, attributeValue);

    return NULL;
}

PyResult Command_kick(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    Client *target;

    if (args.argCount() == 2)
    {

        if (args.isNumber(1))
        {
            int id = atoi(args.arg(1).c_str());
            target = sEntityList.FindClientByCharID(id);
        }
        else
        {
            const char *name = args.arg(1).c_str();
            target = sEntityList.FindClientByName(name);
        }
    }
    //support for characters with first and last names
    else if (args.argCount() == 3)
    {
        if (args.isHexNumber(1))
            throw PyException(MakeCustomError("Unknown arguments"));

        std::string name = args.arg(1) + " " + args.arg(2);
        target = sEntityList.FindClientByName(name.c_str()) ;
    }
    else
        throw PyException(MakeCustomError("Correct Usage: /kick [Character Name]"));

   if (target == NULL)
        throw PyException(MakeCustomError("Cannot find Character"));
    else
        target->DisconnectClient();

    return NULL;
}

PyResult Command_ban(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    Client *target;

    if (args.argCount() == 2)
    {

        if (!args.isNumber(1))
        {
            const char *name = args.arg(1).c_str();
            target = sEntityList.FindClientByName(name);
        }
        else
            throw PyException(MakeCustomError("Correct Usage: /ban [Character Name]"));
    }
    //support for characters with first and last names
    else if (args.argCount() == 3)
    {
        if (args.isHexNumber(1))
            throw PyException(MakeCustomError("Unknown arguments"));

        std::string name = args.arg(1) + " " + args.arg(2);
        target = sEntityList.FindClientByName(name.c_str()) ;
    }
    else
        throw PyException(MakeCustomError("Correct Usage: /ban [Character Name]"));

    //ban client
    target->BanClient();

    //disconnect client
    target->DisconnectClient();

    return NULL;
}

PyResult Command_unban(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (args.argCount() == 2)
    {

        if (!args.isNumber(1))
        {
            const char *name = args.arg(1).c_str();
            services->serviceDB().SetAccountBanStatus(db->GetAccountID(name),false);
        }
        else
            throw PyException(MakeCustomError("Correct Usage: /ban [Character Name]"));
    }
    //support for characters with first and last names
    else if (args.argCount() == 3)
    {
        if (args.isHexNumber(1))
            throw PyException(MakeCustomError("Unknown arguments"));

        std::string name = args.arg(1) + " " + args.arg(2);
        services->serviceDB().SetAccountBanStatus(db->GetAccountID(name),false);
    }
    else
        throw PyException(MakeCustomError("Correct Usage: /unban [Character Name / Character ID]"));

    return NULL;
}

PyResult Command_kill(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (args.argCount() == 2) {
        if (!args.isNumber(1)) {
            throw PyException(MakeCustomError("Argument 1 should be a character ID"));
        }
        uint32 entity = atoi(args.arg(1).c_str());

        InventoryItemRef itemRef = services->item_factory->GetShip(entity);
        if (itemRef.get() == NULL)
            throw PyException(MakeCustomError("/kill NOT supported on non-ship types at this time"));

        SystemEntity* shipEntity = who->SystemMgr()->get(entity);
        if (!shipEntity) {
            throw PyException(MakeCustomError("/kill cannot process this object"));
            sLog.Error("GMCommands - Command_kill()", "Cannot process this object, aborting kill: %s [%u]", itemRef->itemName().c_str(), itemRef->itemID());
        } else {
            who->SystemMgr()->RemoveEntity(shipEntity);
            if (shipEntity->IsNPCSE()) {
                NPC * npcEntity = shipEntity->GetNPCSE();
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

    return NULL;
}

PyResult Command_killallnpcs(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    if (args.argCount() == 1)
    {
        if (!who->GetShipSE()->SysBubble())
            who->EnterSystem(who->GetSystemID());

        std::vector<SystemEntity *> whosBubbleEntityList;
        who->GetShipSE()->SysBubble()->GetEntities(whosBubbleEntityList);
        std::vector<SystemEntity *>::const_iterator cur = whosBubbleEntityList.begin();
        for(; cur != whosBubbleEntityList.end(); cur++) {
            if ((*cur)->IsNPCSE()) {
                NPC * npcEntity = (*cur)->GetNPCSE();
                Damage fatal_blow(who->GetShipSE(),true);
                npcEntity->Killed(fatal_blow);
                delete npcEntity;
            }
        }
    } else
        throw PyException(MakeCustomError("Correct Usage: /killallnpcs"));

    return NULL;
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

    return NULL;
}

PyResult Command_hop(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /*22:49:01 W GMCommands: Command_hop(): This command passes args.argCount() = 1.
     * sm.RemoteSvc('slash').SlashCmd('/hop %s' % distance)
     */
    return NULL;
}

PyResult Command_spawndungeon(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* this command is used to test dungeon spawn system - wip.   -allan 21Feb15
     *
     * upon execution, this command will spawn a random dungeon from db in callers solarSystem,
     *   then create a bookmark in their PnP/BM window
     */
    return NULL;
}

PyResult Command_status(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args)
{
    //if (!who->IsInSpace())
    //    throw PyException(MakeCustomError("You're not in space."));
    if (!who->GetShipSE()->SysBubble())
        who->EnterSystem(who->GetSystemID());
    if (!who->GetShipSE()->DestinyMgr())
        who->ResetDestiny();

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
        who->ResetDestiny();

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
              who->IsInvul(), who->IsLogin(),  who->IsUndock(), who->HasBeyonce(), who->IsBubbleWait(), who->IsSetStateSent());

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
        who->ResetDestiny();

    who->GetShipSE()->DestinyMgr()->Halt();

    char reply[25];
    snprintf(reply, 25,
             "Ship Halted.");

    who->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_fixconnections(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /*
     * this command will reset the conectionType field of the database mapConnections table to the correct settings.  -allan 5July15
     * MapConnections.cpp removed from this build.  -allan 7July15
     */
    //MapCon mc;
    //mc.PopulateConnections();
    return new PyNone;
}

PyResult Command_shutdown(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* ingame command to immediatly save loaded items and halt server.
     */
    sConsole.HaltServer();
    return new PyNone;
}

//13:54:11 W GMCommands: Command_sov(): This command passes args.argCount() = 3.
PyResult Command_sov(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    sLog.Warning("GMCommands: Command_sov()", "This command passes args.argCount() = %u.", args.argCount());
    /*
     *  ' /sov complete ' + str(itemID)
     */
    return NULL;
}

//13:54:11 W GMCommands: Command_pos(): This command passes args.argCount() = 3.
PyResult Command_pos(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    sLog.Warning("GMCommands: Command_pos()", "This command passes args.argCount() = %u.", args.argCount());

    /*
     * ' /pos online ' + str(itemID)
     * ' /pos unanchor ' + str(itemID)
     * ' /pos anchor ' + str(itemID)
     * ' /pos offline ' + str(itemID)
     */
    return NULL;
}

PyResult Command_beltlist(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {
    /* this command is used to debug asteroid creation/management
     * wip.   -allan 15April16
     */

    std::vector<AsteroidSE*> invMap;
    invMap.clear();
    uint32 beltID = who->GetShipSE()->SysBubble()->GetSpawnID(who->GetShipSE()->SysBubble()->GetID());
    BeltMgr* belt = who->GetShipSE()->SystemMgr()->GetBeltMgr();
    belt->GetList(beltID, invMap);

    std::ostringstream str;
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
    uint32 inventoryID = who->GetStationID();

    InventoryItem* item(nullptr);
    Inventory* inv(nullptr);
    if (inventoryID) {
        InventoryItemRef station = sEntityList.GetStationByID(inventoryID);
        if (!station) throw PyException(MakeCustomError("Cannot find Station Reference for stationID %u", inventoryID));
        inv = station->GetInventory();
        inv->GetInventoryList(invMap);
        item = station.get();
    } else {
        Command_list(who,db,services,args);
        inventoryID = who->GetSystemID();
        SolarSystemRef system = services->item_factory->GetSolarSystem(inventoryID);
        inv = system->GetInventory();
        inv->GetInventoryList(invMap);
        item = system.get();
    }

    std::ostringstream str;
    str << "InventoryID %u(%p) (Item %p) has %u items.<br><br>"; //70

    for (auto cur : invMap)
        str << cur.first << "(" << cur.second->flag() << "): " << cur.second->itemName() << "<br>"; // 20 + 70 for name (90)

        int count = invMap.size();
    int size = count * 90;
    size += 70;
    char reply[size];
    snprintf(reply, size, str.str().c_str(), inventoryID, inv, item, count);

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
    ShipItemRef ship = services->item_factory->GetShip(inventoryID);
    Inventory* inv = ship->GetInventory();
    inv->GetInventoryList(invMap);

    std::ostringstream str;
    str << "InventoryID %u(%p) (Ship %p) has %u items.<br><br>"; //50

    for (auto cur : invMap)
        str << cur.first << "(" << cur.second->flag() << "): " << cur.second->itemName() << "<br>"; // 20 + 40 for name (60)

        int count = invMap.size();
    int size = count * 60;
    size += 50;
    char reply[size];
    snprintf(reply, size, str.str().c_str(), inventoryID, inv, ship.get(), count);

    who->SendInfoModalMsg(reply);
    return new PyString(reply);
}

PyResult Command_showsession(Client* who, CommandDB* db, PyServiceMgr* services, const Seperator& args) {

    std::ostringstream str;
    str << "Current Session Values.<br><br>"; //32

    str << "charid: %i <br>"; //14+10
    str << "charname: %s <br>"; //16+10
    str << "shipid: %i <br>"; //14+10
    str << "cloneStationID: %i <br>"; //21+10

    str << "clientid: %i <br>"; //16+10
    str << "userid: %i <br>"; //14+10
    str << "sessionID: %li <br>"; //18+20

    str << "locationid: %i <br>"; //18+10
    str << "stationid: %i <br>"; //17+10
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

    str << "gangrole: %i <br>"; //16+10
    str << "fleetrole: %i <br>"; //17+10

    int size = 32;  // header
    size += 370;    // text
    size += 150;    // %i
    size += 120;    // %lu
    char reply[size];
    snprintf(reply, size, str.str().c_str(),
             who->GetCharacterID(), who->GetName(), who->GetShipID(), who->GetCloneStationID(), who->GetClientID(), who->GetUserID(),
             who->GetSessionID(), who->GetLocationID(), who->GetStationID(), who->GetSystemID(), who->GetConstellationID(), who->GetRegionID(),
             who->GetCorporationID(), who->GetCorpHQ(), who->GetCorpAccountKey(), who->GetCorpRole(), who->GetRolesAtAll(), who->GetRolesAtBase(),
             who->GetRolesAtHQ(), who->GetRolesAtOther(), who->GetGangRole(),who->GetFleetRole() );

    who->SendInfoModalMsg(reply);
    return new PyString(reply);
}