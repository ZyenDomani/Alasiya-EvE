//no macroguard on purpose


COMMAND( create, ROLE_GML,
         "(itemID) [count] - Create count or 1 of the specified item." )
COMMAND( createitem, ROLE_ADMIN,
         "(itemID) [count] - Create count or 1 of the specified item.(from Insider)" )
COMMAND( search, ROLE_GML,
         "(text) - Search for items matching the specified query" )
COMMAND( translocate, ROLE_GML,
         "(entityID) - Translocate to the specified entity" )
COMMAND( tr, ROLE_GML,
         "(who) (entityID) - Translocate the specified person to the specified entity" )
COMMAND( giveisk, ROLE_GMH,
         "(entityID) (amount) - Give the specified amount of cash to the specified character. 0=self." )
COMMAND( pop, ROLE_ADMIN,
         "(type) (key) (value) - Send an OnRemoteMessage" )
COMMAND( goto, ROLE_GML,
         "(x) (y) (z) - Jump to the specified position in space. Stopped." )
COMMAND( spawnn, ROLE_SPAWN,
         "(typeID) - Spawn an NPC with the specified type." )
COMMAND( spawn, ROLE_SPAWN,
         "(typeID) - Spawn an NPC with the specified type." )
COMMAND( halt, ROLE_GML,
         "- Immediatly stops ship, setting Destiny::State = dstball_halt.")
COMMAND( location, ROLE_PLAYER,
         "- Gives you back your current location in space." )
COMMAND( syncloc, ROLE_PLAYER,
         "- Synchonizes your location in client with location on server." )
COMMAND( setbpattr, ROLE_CONTENT,
         "(itemID) (copy) (materialLevel) (productivityLevel) (licensedProductionRunsRemaining) - Change blueprint's attributes." )
COMMAND( state, ROLE_GML,
         "- Sends DoDestinyUpdate SetState." )
COMMAND( update, ROLE_PLAYER,
         "- Sends DoDestinyUpdate SetState, and BubbleManager _AddBalls.")
COMMAND( getattr, ROLE_GML,
         "(itemID) (attributeID) - Retrieves attribute value." )
COMMAND( setattr, ROLE_CONTENT,
         "(itemID) (attributeID) (value) - Sets attributeID of itemID to value." )
COMMAND( fit, ROLE_GML,
        "(itemID) - Fits selected item to active ship." )
COMMAND( giveallskills, ROLE_ADMIN,
        "['me'|<characterID>] - gives ALL skills to designated character or self" )
COMMAND( giveskill, ROLE_GML,
         "(skillID) (level) - gives skillID to specified level." )
COMMAND( online, ROLE_GML,
        "(entityID) - online all modules on the ship of the entityID. entityID=me=>online my modules" )
COMMAND( unload, ROLE_GML,
        "(entityID) (itemID) - unload module itemID from entityID (itemID=all=>unload all) (entityID=me=>my modules)" )
COMMAND( heal, ROLE_HEALSELF,
        "(entityID) - heal the character with the entityID" )
COMMAND( repairmodules, ROLE_HEALSELF,
        "(entityID) (itemID) - repair the modules of the character with the entityID" )
COMMAND( unspawn, ROLE_SPAWN,
        "(entityID) (itemID) - unload module itemID from entityID (itemID=all=>unload all) (entityID=me=>my modules)" )
COMMAND( giveskills, ROLE_ADMIN,
        "(itemID) - gives skills to character." )
COMMAND( dogma, ROLE_ADMIN,
        "(attribute) - change item attribute value" )
COMMAND( kick, ROLE_ADMIN,
        "(charName) - kicks [charName] from the server")
COMMAND( ban, ROLE_ADMIN,
        "(charName) - bans player's account from the server")
COMMAND( unban, ROLE_ADMIN,
        "(charName) - removes ban on player's account")
COMMAND( kill, ROLE_GML,
        "(entityID) - insta-pops a destroyable ship, drone, structure, if applicable")
COMMAND( killallnpcs, ROLE_ADMIN,
        " - insta-pops all NPC ships in the current bubble")
COMMAND( cloak, ROLE_GML,
         " - instantly and unconditionally toggles cloak state of your vessel")
COMMAND( spawndungeon, ROLE_CONTENT,
         " - spawns dungeon in your system, with BM to location")
COMMAND( status, ROLE_PLAYER,
         " - note giving you detailed ship status information")
COMMAND( secstatus, ROLE_PLAYER,
         " - note giving you this character's current security status")
COMMAND( list, ROLE_PLAYER,
         " - gives a list of all dynamic entities and players in this bubble")
COMMAND( commandlist, ROLE_PLAYER,
         " - gives a list of all game commands, required role, and a description")
COMMAND( sov, ROLE_CONTENT,
         " - unknown at this time")
COMMAND( pos, ROLE_PROGRAMMER,
         " - unknown at this time")
COMMAND( hop, ROLE_GML,
         " - unknown at this time")
COMMAND( destinyvars, ROLE_GMH,
         " - shows current destiny variables")
COMMAND( fixconnections, ROLE_WORLDMOD,
         " - updates current (incorrect) db table for mapConnections")
COMMAND( shutdown, ROLE_GMH,
         " - save all items, kick all connections, and halt server. immediate command." )
COMMAND( beltlist, ROLE_PROGRAMMER,
         " - list all roids in current belt's inventory." )
/*  need new inventory code to enable these
COMMAND( inventory, ROLE_PROGRAMMER,
         " - list all items in current location's inventory (either station or solsystem)." )
COMMAND( shipinventory, ROLE_PROGRAMMER,
         " - list all items in current ship's inventory." )
*/
COMMAND( showsession, ROLE_PROGRAMMER,
         " - list current session values." )

/*COMMAND( entity, ROLE_ADMIN,
        "(entityID) - unknown" )
COMMAND( chatban, ROLE_ADMIN,
        "(characterID) - bans character from channel" )
COMMAND( whois, ROLE_ADMIN,
        "(characterName) - returns information on character")
*/
