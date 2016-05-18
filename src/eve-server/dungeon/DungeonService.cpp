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
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "dungeon/DungeonService.h"
//#include "dungeon/DungeonMgr.h"

/*
class DungeonBound
: public PyBoundObject {
public:

    PyCallable_Make_Dispatcher(DungeonBound)

    DungeonBound(PyServiceMgr *mgr, DungeonDB *db)
    : PyBoundObject(mgr, "DungeonBound"),
      m_db(db),
      m_dispatch(new Dispatcher(this))
    {
        _SetCallDispatcher(m_dispatch);

        PyCallable_REG_CALL(DungeonBound, )
        PyCallable_REG_CALL(DungeonBound, )
    }
    virtual ~DungeonBound() { delete m_dispatch; }
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL()
    PyCallable_DECL_CALL()

protected:
    DungeonDB *const m_db;
    Dispatcher *const m_dispatch;   //we own this
};
*/

PyCallable_Make_InnerDispatcher(DungeonService)

DungeonService::DungeonService(PyServiceMgr *mgr)
: PyService(mgr, "dungeon"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(DungeonService, DEGetFactions);
    PyCallable_REG_CALL(DungeonService, DEGetDungeons);
    PyCallable_REG_CALL(DungeonService, DEGetRooms);
}

DungeonService::~DungeonService() {
    delete m_dispatch;
}


/*
PyBoundObject *DungeonService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
    _log(CLIENT__MESSAGE, "DungeonService bind request for:");
    bind_args->Dump(CLIENT__MESSAGE, "    ");

    return(new DungeonBound(m_manager, &m_db));
}*/


PyResult DungeonService::Handle_DEGetFactions( PyCallArgs& call )
{
    //PyRep *result = NULL;

    sLog.Debug( "DungeonService", "Called DEGetFactions stub." );

    return NULL;
}


PyResult DungeonService::Handle_DEGetDungeons( PyCallArgs& call )
{
    //PyRep *result = NULL;
    //dict args:
    // factionID
    // or dungeonVID

    // rows: status (1=RELEASE,2=TESTING,else Working Copy),
    //       dungeonVName
    //       dungeonVID

    sLog.Debug( "DungeonService", "Called DEGetDungeons stub." );

    return NULL;
}


PyResult DungeonService::Handle_DEGetRooms( PyCallArgs& call )
{
    //dict arg: dungeonVID
    //PyRep *result = NULL;

    //rows: roomName

    sLog.Debug( "DungeonService", "Called DEGetRooms stub." );

    return NULL;
}
/**
        archetypes = sm.RemoteSvc('dungeon').GetArchetypes()
        archetypeOptions = [ (archetype.archetypeName, archetype.archetypeID) for archetype in archetypes ]
        roomObjectGroups = sm.RemoteSvc('dungeon').DEGetRoomObjectPaletteData()
        objectIDs = sm.RemoteSvc('dungeon').AddTemplateObjects(roomID, self.sr.node.id, (posInRoom.x, posInRoom.y, posInRoom.z))
            sm.RemoteSvc('dungeon').TemplateRemove(self.sr.node.id)
        dungeonSvc = sm.RemoteSvc('dungeon')
        templateID = dungeonSvc.TemplateAdd(templateName, templateDescription)
        dungeonSvc.TemplateObjectAddDungeonList(templateID, objectIDList)
        */
/*
        self.templateRows = sm.RemoteSvc('dungeon').DEGetTemplates()
        for row in self.templateRows:
            data = {'label': row.templateName,
             'hint': row.description != row.templateName and row.description or '',
             'id': row.templateID,
             'form': self}
             */
/**
    return sm.RemoteSvc('dungeon').IsObjectLocked(objectID)
        sm.RemoteSvc('dungeon').EditObjectXYZ(objectID=objectID, x=x, y=y, z=z)
        sm.RemoteSvc('dungeon').EditObjectYawPitchRoll(objectID=objectID, yaw=yaw, pitch=pitch, roll=roll)
        sm.RemoteSvc('dungeon').EditObjectRadius(objectID=objectID, radius=radius)
        newObjectID = sm.RemoteSvc('dungeon').CopyObject(objectID, roomID, offsetX, offsetY, offsetZ)
        newObjectID, revisionID = sm.RemoteSvc('dungeon').AddObject(roomID, typeID, x, y, z, yaw, pitch, roll, radius)
        sm.RemoteSvc('dungeon').RemoveObject(objectID)
    */

/*
dunArchetypeAgentMissionDungeon = 20
dunArchetypeFacwarDefensive = 32
dunArchetypeFacwarOffensive = 35
dunArchetypeFacwarDungeons = (dunArchetypeFacwarDefensive, dunArchetypeFacwarOffensive)
dunArchetypeWormhole = 38
dunArchetypeZTest = 19
dunEventMessageEnvironment = 3
dunEventMessageImminentDanger = 1
dunEventMessageMissionInstruction = 7
dunEventMessageMissionObjective = 6
dunEventMessageMood = 4
dunEventMessageNPC = 2
dunEventMessageStory = 5
dunEventMessageWarning = 8
dunExpirationDelay = 48
dungeonGateUnlockPeriod = 66
dunTriggerArchaeologyFailure = 16
dunTriggerArchaeologySuccess = 15
dunTriggerArmorConditionLevel = 5
dunTriggerAttacked = 1
dunTriggerCounterEQ = 34
dunTriggerCounterGE = 36
dunTriggerCounterGT = 35
dunTriggerCounterLE = 38
dunTriggerCounterLT = 37
dunTriggerEffectActivated = 27
dunTriggerExploding = 3
dunTriggerFWShipEnteredProximity = 21
dunTriggerFWShipLeftProximity = 30
dunTriggerHackingFailure = 12
dunTriggerHackingSuccess = 11
dunTriggerItemInCargo = 33
dunTriggerItemPlacedInMissionContainer = 23
dunTriggerItemRemovedFromSpawnContainer = 32
dunTriggerMined = 7
dunTriggerPlayerKilled = 26
dunTriggerRoomCapturedAlliance = 19
dunTriggerRoomCapturedFacWar = 20
dunTriggerRoomCapturedCorp = 18
dunTriggerRoomEntered = 8
dunTriggerRoomMined = 10
dunTriggerRoomMinedOut = 9
dunTriggerRoomWipedOut = 31
dunTriggerSalvagingFailure = 14
dunTriggerSalvagingSuccess = 13
dunTriggerShieldConditionLevel = 4
dunTriggerShipEnteredProximity = 2
dunTriggerShipLeftProximity = 29
dunTriggerShipsEnteredRoom = 17
dunTriggerShipsLeftRoom = 28
dunTriggerStructureConditionLevel = 6
dunTriggerEventActivateGate = 1
dunTriggerEventAdjustSystemInfluence = 39
dunTriggerEventAgentMessage = 23
dunTriggerEventAgentTalkTo = 22
dunTriggerEventCounterAdd = 32
dunTriggerEventCounterDivide = 35
dunTriggerEventCounterMultiply = 34
dunTriggerEventCounterSet = 36
dunTriggerEventCounterSubtract = 33
dunTriggerEventDropLoot = 24
dunTriggerEventDungeonCompletion = 11
dunTriggerEventEffectBeaconActivate = 13
dunTriggerEventEffectBeaconDeactivate = 14
dunTriggerEventEntityDespawn = 18
dunTriggerEventEntityExplode = 19
dunTriggerEventGrantGroupReward = 37
dunTriggerEventGrantGroupRewardLimitedRestrictions = 45
dunTriggerEventGrantDelayedGroupReward = 38
dunTriggerFacWarLoyaltyPointsGranted = 48
dunTriggerFacWarVictoryPointsGranted = 20
dunTriggerEventMessage = 10
dunTriggerEventMissionCompletion = 9
dunTriggerEventMissionFailure = 31
dunTriggerEventObjectDespawn = 15
dunTriggerEventObjectExplode = 16
dunTriggerEventOpenTutorial = 46
dunTriggerEventRangedNPCDamageEM = 26
dunTriggerEventRangedNPCDamageExplosive = 27
dunTriggerEventRangedNPCDamageKinetic = 28
dunTriggerEventRangedNPCDamageThermal = 29
dunTriggerEventRangedNPCHealing = 4
dunTriggerEventRangedPlayerDamageEM = 5
dunTriggerEventRangedPlayerDamageExplosive = 6
dunTriggerEventRangedPlayerDamageKinetic = 7
dunTriggerEventRangedPlayerDamageThermal = 8
dunTriggerEventRangedPlayerHealing = 25
dunTriggerEventSpawnGuardObject = 3
dunTriggerEventSpawnGuards = 2
dunTriggerEventSpawnItemInCargo = 30
dunTriggerEventSpawnShip = 47
dunTriggerEventSupressAllRespawn = 42
dunTriggerEventWarpShipAwayAndComeBack = 41
dunTriggerEventWarpShipAwayDespawn = 40
DUNGEON_EVENT_TYPE_AFFECTS_ENTITY = [dunTriggerEventEntityExplode,
 dunTriggerEventEntityDespawn,
 dunTriggerEventSpawnGuards,
 dunTriggerEventWarpShipAwayDespawn,
 dunTriggerEventWarpShipAwayAndComeBack]
DUNGEON_EVENT_TYPE_AFFECTS_OBJECT = [dunTriggerEventSpawnGuardObject,
 dunTriggerEventEffectBeaconActivate,
 dunTriggerEventEffectBeaconDeactivate,
 dunTriggerEventObjectExplode,
 dunTriggerEventObjectDespawn,
 dunTriggerEventActivateGate]
DUNGEON_ORIGIN_UNDEFINED = None
DUNGEON_ORIGIN_STATIC = 1
DUNGEON_ORIGIN_AGENT = 2
DUNGEON_ORIGIN_PLAYTEST = 3
DUNGEON_ORIGIN_EDIT = 4
DUNGEON_ORIGIN_DISTRIBUTION = 5
DUNGEON_ORIGIN_PATH = 6
DUNGEON_ORIGIN_TUTORIAL = 7
dungeonSpawnBelts = 0
dungeonSpawnGate = 1
dungeonSpawnNear = 2
dungeonSpawnDeep = 3
dungeonSpawnReinforcments = 4
dungeonSpawnStations = 5
dungeonSpawnFaction = 6
dungeonSpawnConcord = 7
*/