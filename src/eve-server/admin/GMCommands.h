//no macroguard on purpose


COMMAND( spawnn, ROLE_SPAWN,
         "(typeID) - Spawn an NPC with the specified type." )
COMMAND( spawn, ROLE_SPAWN,
         "(typeID) - Spawn an NPC with the specified type." )
COMMAND( search, ROLE_VIP,
         "(text) - Search for items matching the specified query" )
COMMAND( giveisk, ROLE_GMH,
         "(entityID) (amount) - Give the specified amount of cash to the specified character. 0=self." )
COMMAND( pop, ROLE_ADMIN,
         "(type) (key) (value) - Send an OnRemoteMessage" )
COMMAND( setbpattr, ROLE_CONTENT,
         "(itemID) (copy) (materialLevel) (productivityLevel) (licensedProductionRunsRemaining) - Change blueprint's attributes." )
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

/*
COMMAND( entity, ROLE_ADMIN,
        "(entityID) - unknown" )
COMMAND( chatban, ROLE_ADMIN,
        "(characterID) - bans character from channel" )
COMMAND( whois, ROLE_ADMIN,
        "(characterName) - returns information on character")
*/
