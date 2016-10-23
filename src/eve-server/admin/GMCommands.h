//no macroguard on purpose


COMMAND( create, ROLE_VIP,
         "(itemID) [count] - Create count or 1 of the specified item." )
COMMAND( createitem, ROLE_ADMIN,
         "(itemID) [count] - Create count or 1 of the specified item.(from Insider)" )
COMMAND( search, ROLE_VIP,
         "(text) - Search for items matching the specified query" )
COMMAND( translocate, ROLE_VIP,
         "(entityID) - Translocate to the specified entity" )
COMMAND( tr, ROLE_VIP,
         "(who) (entityID) - Translocate the specified person to the specified entity" )
COMMAND( giveisk, ROLE_GMH,
         "(entityID) (amount) - Give the specified amount of cash to the specified character. 0=self." )
COMMAND( pop, ROLE_ADMIN,
         "(type) (key) (value) - Send an OnRemoteMessage" )
COMMAND( goto, ROLE_VIP,
         "(x) (y) (z) - Jump to the specified position in space. Stopped." )
COMMAND( spawnn, ROLE_SPAWN,
         "(typeID) - Spawn an NPC with the specified type." )
COMMAND( spawn, ROLE_SPAWN,
         "(typeID) - Spawn an NPC with the specified type." )
COMMAND( halt, ROLE_VIP,
         "- Immediatly stops ship, setting Destiny::State = dstball_halt.")
COMMAND( location, ROLE_PLAYER,
         "- Gives you back your current location in space." )
COMMAND( syncloc, ROLE_PLAYER,
         "- Synchonizes your location in client with location on server." )
COMMAND( setbpattr, ROLE_CONTENT,
         "(itemID) (copy) (materialLevel) (productivityLevel) (licensedProductionRunsRemaining) - Change blueprint's attributes." )
COMMAND( update, ROLE_CONTENT,
         "- Sets Current Position according to Server's DestinyManager, then Sends Bubble AddBalls and Destiny SetState. (resets spaceview with current server data)" )
COMMAND( sendstate, ROLE_CONTENT,
         "- Sends DoDestinyUpdate SetState." )
COMMAND( addball, ROLE_PLAYER,
         "- Sends BubbleManager AddBalls.")
COMMAND( addball2, ROLE_PLAYER,
         "- Sends BubbleManager AddBalls2.")
COMMAND( getattr, ROLE_VIP,
         "(itemID) (attributeID) - Retrieves attribute value." )
COMMAND( setattr, ROLE_CONTENT,
         "(itemID) (attributeID) (value) - Sets attributeID of itemID to value." )
COMMAND( fit, ROLE_VIP,
        "(itemID) - Fits selected item to active ship." )
COMMAND( giveallskills, ROLE_ADMIN,
        "['me'|<characterID>] - gives ALL skills to designated character or self" )
COMMAND( giveskill, ROLE_VIP,
         "(skillID) (level) - gives skillID to specified level." )
COMMAND( online, ROLE_VIP,
        "(entityID) - online all modules on the ship of the entityID. entityID=me=>online my modules" )
COMMAND( unload, ROLE_VIP,
        "(entityID) (itemID) - unload module itemID from entityID (itemID=all=>unload all) (entityID=me=>my modules)" )
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
COMMAND( kill, ROLE_VIP,
        "(entityID) - insta-pops a destroyable ship, drone, structure, if applicable")
COMMAND( killallnpcs, ROLE_ADMIN,
        " - insta-pops all NPC ships in the current bubble")
COMMAND( cloak, ROLE_VIP,
         " - instantly and unconditionally toggles cloak state of your vessel")
COMMAND( sov, ROLE_CONTENT,
         " - unknown at this time")
COMMAND( pos, ROLE_PROGRAMMER,
         " - unknown at this time")
COMMAND( hop, ROLE_CONTENT,
         " - unknown at this time")

/*
COMMAND( entity, ROLE_ADMIN,
        "(entityID) - unknown" )
COMMAND( chatban, ROLE_ADMIN,
        "(characterID) - bans character from channel" )
COMMAND( whois, ROLE_ADMIN,
        "(characterName) - returns information on character")
*/
