 

COMMAND( create, ROLE_VIP,
         "(itemID) [count] - Create count or 1 of the specified item." )
COMMAND( createitem, ROLE_ADMIN,
         "(itemID) [count] - Create count or 1 of the specified item.(from Insider)" )
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
COMMAND( update, ROLE_CONTENT,
         "- Sets Current Position according to Server's DestinyManager, then Sends Bubble AddBalls and Destiny SetState. (resets spaceview with current server data)" )
COMMAND( sendstate, ROLE_CONTENT,
         "- Sends DoDestinyUpdate SetState." )
COMMAND( addball, ROLE_PLAYER,
         "- Sends BubbleManager AddBalls.")
COMMAND( addball2, ROLE_PLAYER,
         "- Sends BubbleManager AddBalls2.")
COMMAND( unspawn, ROLE_SPAWN,
         "(itemID) - remove and delete itemID" )
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