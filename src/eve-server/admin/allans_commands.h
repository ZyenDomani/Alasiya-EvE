

 COMMAND( spawndungeon, ROLE_CONTENT,
          " - spawns dungeon <dungeonID> in your system, with BM to location")
 COMMAND( removedungeon, ROLE_CONTENT,
          " - removes identified dungeon")
 COMMAND( siglist, ROLE_CONTENT,
          " - lists all active signatures, with location, name, and type." )
 COMMAND( heal, ROLE_HEALSELF,
         "(entityID) - heal the character with the entityID" )
 COMMAND( status, ROLE_PLAYER,
          " - note giving you detailed ship status information")
 COMMAND( secstatus, ROLE_PLAYER,
          " - note giving you this character's current security status")
 COMMAND( list, ROLE_PLAYER,
          " - gives a list of all dynamic entities and players and their destinyState in this bubble")
 COMMAND( commandlist, ROLE_PLAYER,
          " - gives a list of all game commands, required role, and a description")
 COMMAND( destinyvars, ROLE_GMH,
          " - shows some current destiny variables")
 COMMAND( shipvars, ROLE_GMH,
          " - shows other current destiny variables")
 COMMAND( fixconnections, ROLE_WORLDMOD,
          " - updates current (incorrect) db table for mapConnections")
 COMMAND( shutdown, ROLE_GMH,
          " - save all items, kick all connections, and halt server. immediate command." )
 COMMAND( beltlist, ROLE_PROGRAMMER,
          " - list all roids in current belt's inventory." )
 COMMAND( inventory, ROLE_PROGRAMMER,
          " - list all items in current location's inventory (either station or solsystem)." )
 COMMAND( shipinventory, ROLE_PROGRAMMER,
          " - list all items in current ship's inventory." )
 COMMAND( showsession, ROLE_PROGRAMMER,
          " - list current session values." )
 COMMAND( skilllist, ROLE_PROGRAMMER,
          " - list all skills loaded for character." )
 COMMAND( shipdna, ROLE_PROGRAMMER,
          " - show current ship DNA." )
 COMMAND( targlist, ROLE_PROGRAMMER,
          " - show current ship target list." )
 COMMAND( bubblelist, ROLE_PROGRAMMER,
          " - show current objects in bubble, their destiny state, and movement speed." )
 COMMAND( track, ROLE_PROGRAMMER,
          " - toggles current object movement tracking state." )
 COMMAND( warpto, ROLE_GMH,
          " - warp to an object using its itemID." )
