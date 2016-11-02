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
    Author:        Zhur, Allan
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "account/TutorialService.h"

PyCallable_Make_InnerDispatcher(TutorialService)

TutorialService::TutorialService(PyServiceMgr *mgr)
: PyService(mgr, "tutorialSvc"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(TutorialService, GetTutorials);
    PyCallable_REG_CALL(TutorialService, GetTutorialInfo);
    PyCallable_REG_CALL(TutorialService, GetTutorialAgents);
    PyCallable_REG_CALL(TutorialService, GetCriterias);
    PyCallable_REG_CALL(TutorialService, GetCategories);
    PyCallable_REG_CALL(TutorialService, GetContextHelp);
    PyCallable_REG_CALL(TutorialService, GetCharacterTutorialState);
    PyCallable_REG_CALL(TutorialService, GetTutorialsAndConnections);
    PyCallable_REG_CALL(TutorialService, GetCareerAgents);
}

TutorialService::~TutorialService() {
    delete m_dispatch;
}

PyResult TutorialService::Handle_GetTutorials(PyCallArgs &call) {
  sLog.Log( "TutorialService::Handle_GetTutorials()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALL_DUMP);
    return(m_db.GetAllTutorials());
}

PyResult TutorialService::Handle_GetTutorialInfo(PyCallArgs &call) {
  sLog.Log( "TutorialService::Handle_GetTutorialInfo()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALL_DUMP);
    Call_GetTutorialInfo args;
    if(!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "Can't parse args.");
        return NULL;
    }

    Rsp_GetTutorialInfo rsp;

    rsp.pagecriterias = m_db.GetPageCriterias(args.tutorialID);
    if(rsp.pagecriterias == NULL) {
        codelog(SERVICE__ERROR, "An error occured while getting pagecriterias for tutorial %u.", args.tutorialID);
        return NULL;
    }

    rsp.pages = m_db.GetPages(args.tutorialID);
    if(rsp.pages == NULL) {
        codelog(SERVICE__ERROR, "An error occured while getting pages for tutorial %u.", args.tutorialID);
        return NULL;
    }

    rsp.tutorial = m_db.GetTutorial(args.tutorialID);
    if(rsp.tutorial == NULL) {
        codelog(SERVICE__ERROR, "An error occured while getting tutorial %u.", args.tutorialID);
        return NULL;
    }

    rsp.criterias = m_db.GetTutorialCriterias(args.tutorialID);
    if(rsp.criterias == NULL) {
        codelog(SERVICE__ERROR, "An error occured while getting criterias for tutorial %u.", args.tutorialID);
        return NULL;
    }

    return(rsp.Encode());
}

PyResult TutorialService::Handle_GetTutorialAgents(PyCallArgs &call) {
    /*  this should be cached
          [PyTuple 4 items]
            [PyInt 1]
            [PyString "GetTutorialAgents"]
            [PyTuple 1 items]
              [PyList 12 items]
                [PyInt 3018921]
                [PyInt 3019349]
                [PyInt 3019337]
                [PyInt 3018935]
                [PyInt 3019371]
                [PyInt 3019355]
                [PyInt 3018923]
                [PyInt 3019341]
                [PyInt 3019333]
                [PyInt 3018920]
                [PyInt 3019346]
                [PyInt 3019343]
            [PyDict 1 kvp]
              [PyString "machoVersion"]
              [PyInt 1]
              
      [PySubStream 377 bytes]
        [PyList 12 items]
          [PyPackedRow 28 bytes]
            ["agentID" => <3018920> [I4]]
            ["agentTypeID" => <8> [I4]]
            ["divisionID" => <22> [I4]]
            ["level" => <1> [UI1]]
            ["stationID" => <60015037> [I4]]
            ["bloodlineID" => <14> [UI1]]
            ["quality" => <0> [I4]]
            ["corporationID" => <0> [I4]]
            ["gender" => <0> [Bool]]
          [PyPackedRow 28 bytes]
            ["agentID" => <3018921> [I4]]
            ["agentTypeID" => <8> [I4]]
            ["divisionID" => <22> [I4]]
            ["level" => <1> [UI1]]
            ["stationID" => <60015021> [I4]]
            ["bloodlineID" => <6> [UI1]]
            ["quality" => <0> [I4]]
            ["corporationID" => <0> [I4]]
            ["gender" => <1> [Bool]]
          [PyPackedRow 28 bytes]
            ["agentID" => <3018923> [I4]]
            ["agentTypeID" => <8> [I4]]
            ["divisionID" => <22> [I4]]
            ["level" => <1> [UI1]]
            ["stationID" => <60015029> [I4]]
            ["bloodlineID" => <12> [UI1]]
            ["quality" => <0> [I4]]
            ["corporationID" => <0> [I4]]
            ["gender" => <1> [Bool]]
          [PyPackedRow 28 bytes]
            ["agentID" => <3018935> [I4]]
            ["agentTypeID" => <8> [I4]]
            ["divisionID" => <22> [I4]]
            ["level" => <1> [UI1]]
            ["stationID" => <60015027> [I4]]
            ["bloodlineID" => <11> [UI1]]
            ["quality" => <0> [I4]]
            ["corporationID" => <0> [I4]]
            ["gender" => <1> [Bool]]
          [PyPackedRow 28 bytes]
            ["agentID" => <3019333> [I4]]
            ["agentTypeID" => <8> [I4]]
            ["divisionID" => <22> [I4]]
            ["level" => <1> [UI1]]
            ["stationID" => <60015036> [I4]]
            ["bloodlineID" => <7> [UI1]]
            ["quality" => <0> [I4]]
            ["corporationID" => <0> [I4]]
            ["gender" => <1> [Bool]]
          [PyPackedRow 28 bytes]
            ["agentID" => <3019337> [I4]]
            ["agentTypeID" => <8> [I4]]
            ["divisionID" => <22> [I4]]
            ["level" => <1> [UI1]]
            ["stationID" => <60015010> [I4]]
            ["bloodlineID" => <5> [UI1]]
            ["quality" => <0> [I4]]
            ["corporationID" => <0> [I4]]
            ["gender" => <1> [Bool]]
          [PyPackedRow 28 bytes]
            ["agentID" => <3019341> [I4]]
            ["agentTypeID" => <8> [I4]]
            ["divisionID" => <22> [I4]]
            ["level" => <1> [UI1]]
            ["stationID" => <60015016> [I4]]
            ["bloodlineID" => <8> [UI1]]
            ["quality" => <0> [I4]]
            ["corporationID" => <0> [I4]]
            ["gender" => <1> [Bool]]
          [PyPackedRow 28 bytes]
            ["agentID" => <3019343> [I4]]
            ["agentTypeID" => <8> [I4]]
            ["divisionID" => <22> [I4]]
            ["level" => <1> [UI1]]
            ["stationID" => <60015041> [I4]]
            ["bloodlineID" => <3> [UI1]]
            ["quality" => <0> [I4]]
            ["corporationID" => <0> [I4]]
            ["gender" => <1> [Bool]]
          [PyPackedRow 28 bytes]
            ["agentID" => <3019346> [I4]]
            ["agentTypeID" => <8> [I4]]
            ["divisionID" => <22> [I4]]
            ["level" => <1> [UI1]]
            ["stationID" => <60015046> [I4]]
            ["bloodlineID" => <4> [UI1]]
            ["quality" => <0> [I4]]
            ["corporationID" => <0> [I4]]
            ["gender" => <1> [Bool]]
          [PyPackedRow 28 bytes]
            ["agentID" => <3019349> [I4]]
            ["agentTypeID" => <8> [I4]]
            ["divisionID" => <22> [I4]]
            ["level" => <1> [UI1]]
            ["stationID" => <60015020> [I4]]
            ["bloodlineID" => <13> [UI1]]
            ["quality" => <0> [I4]]
            ["corporationID" => <0> [I4]]
            ["gender" => <0> [Bool]]
          [PyPackedRow 28 bytes]
            ["agentID" => <3019355> [I4]]
            ["agentTypeID" => <8> [I4]]
            ["divisionID" => <22> [I4]]
            ["level" => <1> [UI1]]
            ["stationID" => <60015005> [I4]]
            ["bloodlineID" => <11> [UI1]]
            ["quality" => <0> [I4]]
            ["corporationID" => <0> [I4]]
            ["gender" => <0> [Bool]]
          [PyPackedRow 28 bytes]
            ["agentID" => <3019371> [I4]]
            ["agentTypeID" => <8> [I4]]
            ["divisionID" => <22> [I4]]
            ["level" => <1> [UI1]]
            ["stationID" => <60015001> [I4]]
            ["bloodlineID" => <1> [UI1]]
            ["quality" => <0> [I4]]
            ["corporationID" => <0> [I4]]
            ["gender" => <1> [Bool]]
                */
    sLog.Log( "TutorialService::Handle_GetTutorialAgents()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    return new PyInt( 0 );
}

PyResult TutorialService::Handle_GetCriterias(PyCallArgs &call) {
  sLog.Log( "TutorialService::Handle_GetCriterias()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALL_DUMP);
    return(m_db.GetAllCriterias());
}

PyResult TutorialService::Handle_GetCategories(PyCallArgs &call) {
  sLog.Log( "TutorialService::Handle_GetCategories()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALL_DUMP);
    return(m_db.GetCategories());
}

PyResult TutorialService::Handle_GetContextHelp( PyCallArgs& call )
{
  sLog.Log( "TutorialService::Handle_GetContextHelp()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
    /*
              [PyToken dbutil.CRowset]
            [PyDict 1 kvp]
              [PyString "header"]
              [PyObjectEx Normal]
                [PyTuple 2 items]
                  [PyToken blue.DBRowDescriptor]
                  [PyTuple 1 items]
                    [PyTuple 6 items]
                      [PyTuple 2 items]
                        [PyString "contextID"]
                        [PyInt 3]
                      [PyTuple 2 items]
                        [PyString "keywords"]
                        [PyInt 130]
                      [PyTuple 2 items]
                        [PyString "url"]
                        [PyInt 130]
                      [PyTuple 2 items]
                        [PyString "description"]
                        [PyInt 130]
                      [PyTuple 2 items]
                        [PyString "published"]
                        [PyInt 11]
                      [PyTuple 2 items]
                        [PyString "tutorialID"]
                        [PyInt 3]
          [PyPackedRow 9 bytes]
            ["contextID" => <53> [I4]]
            ["keywords" => <UI_SHARED_NOTEPAD> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/In_game_notepad> [WStr]]
            ["description" => <The notepad> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <67> [I4]]
            ["keywords" => <UI_SHARED_CHARACTERSHEET> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Skill_training> [WStr]]
            ["description" => <Skill training> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <159> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <68> [I4]]
            ["keywords" => <UI_SHARED_CHARACTERSHEET> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Standings_Mechanics> [WStr]]
            ["description" => <What are my standings and what do they do?> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <62> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <69> [I4]]
            ["keywords" => <UI_SHARED_CHARACTERSHEET> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Attributes_in_EVE> [WStr]]
            ["description" => <Attributes in EVE> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <70> [I4]]
            ["keywords" => <UI_SHARED_CHARACTERSHEET> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Races> [WStr]]
            ["description" => <Races and bloodlines in EVE> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <71> [I4]]
            ["keywords" => <UI_SHARED_CHARACTERSHEET> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Skill_training> [WStr]]
            ["description" => <Skill training queue> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <72> [I4]]
            ["keywords" => <UI_SHARED_CHARACTERSHEET> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Security_Status> [WStr]]
            ["description" => <Security status and travelling restrictions> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <73> [I4]]
            ["keywords" => <UI_SHARED_CHARACTERSHEET> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Jump_clones> [WStr]]
            ["description" => <Jump clones> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <74> [I4]]
            ["keywords" => <UI_SHARED_CHARACTERSHEET> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Neural_remapping> [WStr]]
            ["description" => <Neural remapping> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <77> [I4]]
            ["keywords" => <UI_MARKET_MARKET> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Market_guide> [WStr]]
            ["description" => <Market Guide> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <12> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <79> [I4]]
            ["keywords" => <UI_CORP_ASSETS> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Assets> [WStr]]
            ["description" => <Your assets> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <80> [I4]]
            ["keywords" => <UI_RMR_SCIENCEANDINDUSTRY> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Research_and_manufacturing> [WStr]]
            ["description" => <Production/research job time> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <146> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <81> [I4]]
            ["keywords" => <UI_RMR_SCIENCEANDINDUSTRY> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Researching_Blueprints> [WStr]]
            ["description" => <Researching blueprints> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <82> [I4]]
            ["keywords" => <UI_RMR_SCIENCEANDINDUSTRY> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Research_and_manufacturing> [WStr]]
            ["description" => <Researching and copying Tech Level II blueprints> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <83> [I4]]
            ["keywords" => <UI_RMR_SCIENCEANDINDUSTRY> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Tech_Level_II_production> [WStr]]
            ["description" => <Tech Level II production> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <84> [I4]]
            ["keywords" => <UI_RMR_SCIENCEANDINDUSTRY> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Ice_Harvesting> [WStr]]
            ["description" => <Ice Harvesting> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <86> [I4]]
            ["keywords" => <UI_CONTRACTS_CONTRACTS> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Contracts> [WStr]]
            ["description" => <Contracts> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <54> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <87> [I4]]
            ["keywords" => <UI_STATION_REPROCESSINGPLANT> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Ice_Harvesting> [WStr]]
            ["description" => <Ice Harvesting> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <9> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <88> [I4]]
            ["keywords" => <UI_STATION_REPROCESSINGPLANT> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Asteroids> [WStr]]
            ["description" => <Asteroids> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <89> [I4]]
            ["keywords" => <UI_STATION_REPROCESSINGPLANT> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Minerals> [WStr]]
            ["description" => <Minerals> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <90> [I4]]
            ["keywords" => <UI_STATION_FITTING> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Rigs> [WStr]]
            ["description" => <Rigs> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <13> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <91> [I4]]
            ["keywords" => <UI_STATION_REPAIRSHOP> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Repackage> [WStr]]
            ["description" => <Repackaging and repairing your ships and modules> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <46> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <92> [I4]]
            ["keywords" => <UI_LPSTORE_LPSTORE> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Loyalty_Points_and_Loyalty_Points_Store> [WStr]]
            ["description" => <Loyalty Points and LP Store> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <93> [I4]]
            ["keywords" => <UI_STATION_INSURANCE> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Insurance> [WStr]]
            ["description" => <Insurance> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <43> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <94> [I4]]
            ["keywords" => <UI_STATION_MEDICAL> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/What_happens_when_my_character_dies> [WStr]]
            ["description" => <What happens when my character dies?> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <27> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <95> [I4]]
            ["keywords" => <UI_SHARED_MAPWORLDCTRLPANEL> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Star_map> [WStr]]
            ["description" => <The world map> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <14> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <97> [I4]]
            ["keywords" => <UI_SHARED_WALLET> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Wallet_configuration> [WStr]]
            ["description" => <Wallet Configuration> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <99> [I4]]
            ["keywords" => <UI_SHARED_PEOPLEANDPLACES> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/People_and_places> [WStr]]
            ["description" => <I can't find a station/system> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <15> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <100> [I4]]
            ["keywords" => <UI_SHARED_PEOPLEANDPLACES> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Bookmarks> [WStr]]
            ["description" => <How to use bookmarks> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <101> [I4]]
            ["keywords" => <UI_SHARED_EVEMAIL> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/EVE_mail> [WStr]]
            ["description" => <The EVE mail system> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <102> [I4]]
            ["keywords" => <UI_SHARED_CHANNELS> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Cant_see_anything_on_a_chat_channel> [WStr]]
            ["description" => <Why can't I see anything on chat channels?> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <104> [I4]]
            ["keywords" => <UI_GENERIC_JOURNAL> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Mission_Journal> [WStr]]
            ["description" => <Mission Journal overview> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <105> [I4]]
            ["keywords" => <UI_GENERIC_JOURNAL> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Research_agent> [WStr]]
            ["description" => <Research points not updating> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <106> [I4]]
            ["keywords" => <UI_GENERIC_JOURNAL> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Loyalty_Points_and_Loyalty_Points_Store> [WStr]]
            ["description" => <Loyalty Points and LP Store> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <107> [I4]]
            ["keywords" => <UI_GENERIC_JOURNAL> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Escalating_encounters> [WStr]]
            ["description" => <Escalating encounters> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <108> [I4]]
            ["keywords" => <UI_GENERIC_JOURNAL> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Storyline_mission> [WStr]]
            ["description" => <Storyline missions> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <109> [I4]]
            ["keywords" => <UI_GENERIC_CORPORATION> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/CEO_replacement> [WStr]]
            ["description" => <CEO replacement> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <128> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <110> [I4]]
            ["keywords" => <UI_GENERIC_CORPORATION> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Why_are_my_corporate_management_skills_not_working> [WStr]]
            ["description" => <Why are my corporate management skills not working? > [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <129> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <111> [I4]]
            ["keywords" => <UI_GENERIC_CORPORATION> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Putting_votes_into_action> [WStr]]
            ["description" => <Putting votes into action> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <112> [I4]]
            ["keywords" => <UI_GENERIC_CORPORATION> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Renting_offices> [WStr]]
            ["description" => <Renting Corporation Offices> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <91> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <113> [I4]]
            ["keywords" => <UI_GENERIC_CORPORATION> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Votes> [WStr]]
            ["description" => <How are corporation votes used?> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <115> [I4]]
            ["keywords" => <UI_GENERIC_CORPORATION> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Alliance_System> [WStr]]
            ["description" => <The Alliance System> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <116> [I4]]
            ["keywords" => <UI_GENERIC_CORPORATION> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Corporation_management_guide#Benefits_of_Corporate_Membership> [WStr]]
            ["description" => <Benefits of belonging to a corporation?> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <117> [I4]]
            ["keywords" => <UI_GENERIC_CORPORATION> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Joining_a_corporation> [WStr]]
            ["description" => <Joining NPC corporations> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <118> [I4]]
            ["keywords" => <UI_GENERIC_CORPORATION> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Corporation_wars> [WStr]]
            ["description" => <Corporation wars> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <119> [I4]]
            ["keywords" => <UI_GENERIC_CORPORATION> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Shareholders> [WStr]]
            ["description" => <Shareholders> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <120> [I4]]
            ["keywords" => <UI_GENERIC_CORPORATION> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Creating_a_corporation> [WStr]]
            ["description" => <Creating a corporation> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <121> [I4]]
            ["keywords" => <UI_GENERIC_CORPORATION> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Corp_theft> [WStr]]
            ["description" => <Corp theft> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <122> [I4]]
            ["keywords" => <UI_GENERIC_CORPORATION> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Sovereignty_(Mechanics)> [WStr]]
            ["description" => <Sovereignty> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <123> [I4]]
            ["keywords" => <UI_STATION_INSURANCE> [WStr]]
            ["url" => <empty string> [WStr]]
            ["description" => <empty string> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <41> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <124> [I4]]
            ["keywords" => <UI_STATION_MEDICAL> [WStr]]
            ["url" => <empty string> [WStr]]
            ["description" => <empty string> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <42> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <125> [I4]]
            ["keywords" => <UI_STATION_MILITIAOFFICE> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Factional_Warfare> [WStr]]
            ["description" => <Factional Warfare> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <126> [I4]]
            ["keywords" => <UI_GANG_FLEET> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/EVE_voice> [WStr]]
            ["description" => <EVE Voice - How to use> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <127> [I4]]
            ["keywords" => <UI_SHARED_CERTPLANNER> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Certificates> [WStr]]
            ["description" => <Certificates> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <134> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <128> [I4]]
            ["keywords" => <UI_SHARED_CHARACTERSHEET> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Kill_Rights> [WStr]]
            ["description" => <Kill Rights> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <129> [I4]]
            ["keywords" => <UI_FLEET_FLEET> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Fleet> [WStr]]
            ["description" => <Fleets> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <130> [I4]]
            ["keywords" => <UI_SHARED_CHANNELS> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Channels> [WStr]]
            ["description" => <Channels> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <131> [I4]]
            ["keywords" => <SOVEREIGNTY_SOVEREIGNTYDASHBOARD> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Sovereignty_dashboard> [WStr]]
            ["description" => <Sovereignty Dashboard> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <132> [I4]]
            ["keywords" => <UI_FLEET_FLEET> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/EVE_voice> [WStr]]
            ["description" => <EVE Voice> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <133> [I4]]
            ["keywords" => <SOVEREIGNTY_SOVEREIGNTYDASHBOARD> [WStr]]
            ["url" => <http://wiki.eveonline.com/wiki/Sovereignty_(Mechanics)> [WStr]]
            ["description" => <Sovereignty> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
          [PyPackedRow 9 bytes]
            ["contextID" => <134> [I4]]
            ["keywords" => <UI_CAL_CALENDAR> [WStr]]
            ["url" => <http://wiki.eveonline.com/en/wiki/calendar> [WStr]]
            ["description" => <the calendar> [WStr]]
            ["published" => <0> [Bool]]
            ["tutorialID" => <0> [I4]]
    [PyNone]
*/
}

//00:25:53 L TutorialService::Handle_GetCharacterTutorialState(): size= 0
PyResult TutorialService::Handle_GetCharacterTutorialState( PyCallArgs& call ) {
  /*  Empty Call  */

    return new PyInt( 0 );
}

PyResult TutorialService::Handle_GetTutorialsAndConnections( PyCallArgs& call ) {
    /*  no logs */
  /*  This is used to link tutorials using connections to other tutorials  */
            /*
            t, tc = sm.RemoteSvc('tutorialSvc').GetTutorialsAndConnections()
            self.tutorials = t.Index('tutorialID')
            tc = tc.Filter('tutorialID')
            self.tutorialConnections = defaultdict(dict)
            for tutID, rows in tc.iteritems():
                for each in rows:
                    self.tutorialConnections[tutID][each.raceID] = each.nextTutorialID
    */

            /*  FIXME  this needs work.  not sure what's wrong, but i DO know our db is incomplete
    uint8 raceID = call.client->GetChar()->race();
    return (m_db.GetTutorialsAndConnections(raceID));
    */
    return new PyNone();
}

PyResult TutorialService::Handle_GetCareerAgents( PyCallArgs& call ) {
  /*  Empty Call  */
  /**
        agentMapping = sm.RemoteSvc('tutorialSvc').GetCareerAgents()
        for careerType in agentMapping:
            agentIDs = []
            if careerType not in self.careerAgents:
                self.careerAgents[careerType] = {}
                self.careerAgents[careerType]['agent'] = {}
                self.careerAgents[careerType]['station'] = {}
            agentIDs = agentMapping.get(careerType, [])
            agents = sm.RemoteSvc('tutorialSvc').GetTutorialAgents(agentIDs)
            for agent in agents:
                self.careerAgents[careerType]['agent'][agent.agentID] = agent
                self.careerAgents[careerType]['station'][agent.agentID] = sm.GetService('map').GetStation(agent.stationID)
*/

    return new PyNone();
}


/**
            sm.RemoteSvc('tutorialSvc').LogCompleted(tutorialID, pageNo, int(time))
        elif status == 'aborted':
            stat[sequenceID] = 'done'
            sm.RemoteSvc('tutorialSvc').LogAborted(tutorialID, pageNo, int(time))

                categories = sm.RemoteSvc('tutorialSvc').GetCategories()
                for category in categories:
                    self.categories[category.categoryID] = category
                    self.categories[category.categoryID].categoryName = localization.GetByMessageID(category.categoryNameID)
                    self.categories[category.categoryID].description = localization.GetByMessageID(category.descriptionID)

                criterias = sm.RemoteSvc('tutorialSvc').GetCriterias()
                for criteria in criterias:
                    self.criterias[criteria.criteriaID] = criteria
            actions = sm.RemoteSvc('tutorialSvc').GetActions()
            for action in actions:
                self.actions[action.actionID] = action

            tutData = sm.RemoteSvc('tutorialSvc').GetTutorialInfo(tutorialID)
                sm.RemoteSvc('tutorialSvc').LogAppClosed(tutorialID, pageNo, int(time))
                        sm.RemoteSvc('tutorialSvc').LogClosed(tutorialID, pageNo, int(time))
                    sm.RemoteSvc('tutorialSvc').LogStarted(tutorialID, pageNo, int(time))
            sm.RemoteSvc('tutorialSvc').LogCompleted(tutorialID, pageNo, int(time))
        return sm.RemoteSvc('tutorialLocationSvc').GiveTutorialGoodies(tutorialID, pageID, pageNo)
                tutData = sm.RemoteSvc('tutorialSvc').GetTutorialInfo(VID)

        rs = sm.RemoteSvc('tutorialSvc').GetCharacterTutorialState()
        if not rs or len(rs) == 0:
            return



            */