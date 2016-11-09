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
    Author:     caytchen, Zhur
    Updates:        Allan
*/

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "PyServiceCD.h"
#include "cache/ObjCacheService.h"
#include "character/CharUnboundMgrService.h"
#include "imageserver/ImageServer.h"

PyCallable_Make_InnerDispatcher(CharUnboundMgrService)

CharUnboundMgrService::CharUnboundMgrService(PyServiceMgr* mgr)
: PyService(mgr, "charUnboundMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(CharUnboundMgrService, SelectCharacterID);
    PyCallable_REG_CALL(CharUnboundMgrService, GetCharacterToSelect);
    PyCallable_REG_CALL(CharUnboundMgrService, GetCharactersToSelect);
    PyCallable_REG_CALL(CharUnboundMgrService, GetCharacterInfo);
    PyCallable_REG_CALL(CharUnboundMgrService, IsUserReceivingCharacter);
    PyCallable_REG_CALL(CharUnboundMgrService, DeleteCharacter);
    PyCallable_REG_CALL(CharUnboundMgrService, PrepareCharacterForDelete);
    PyCallable_REG_CALL(CharUnboundMgrService, CancelCharacterDeletePrepare);
    PyCallable_REG_CALL(CharUnboundMgrService, ValidateNameEx);
    PyCallable_REG_CALL(CharUnboundMgrService, GetCharCreationInfo);
    PyCallable_REG_CALL(CharUnboundMgrService, GetCharNewExtraCreationInfo);
    PyCallable_REG_CALL(CharUnboundMgrService, CreateCharacterWithDoll);
}

CharUnboundMgrService::~CharUnboundMgrService() {
    delete m_dispatch;
}

void CharUnboundMgrService::GetCharacterData(uint32 characterID, std::map< std::string, uint64 >& characterDataMap)
{
    m_db.GetCharacterData(characterID, characterDataMap);
}

PyResult CharUnboundMgrService::Handle_IsUserReceivingCharacter(PyCallArgs &call) {
    _log(CLIENT__ERROR, "Called IsUserReceivingCharacter");
    /*  this is called when selecting the 3ed slot, when there are 2 chars on account already.
     * returning true will disable creating a 3ed character.
     * returning false will allow creating a 3ed character.
     */
    return new PyBool(false);
}

PyResult CharUnboundMgrService::Handle_ValidateNameEx(PyCallArgs &call)
{
    if (call.tuple->IsString()) {
        Call_SingleStringArg arg;
        if (!arg.Decode(&call.tuple)) {
            codelog(CLIENT__ERROR, "Failed to decode args for ValidateNameEx call");
            return new PyBool(false);
        }
        return m_db.ValidateCharName(arg.arg.c_str());
    } else if (call.tuple->IsWString()) {
        Call_SingleWStringArg arg;
        if (!arg.Decode(&call.tuple)) {
            codelog(CLIENT__ERROR, "Failed to decode args for ValidateNameEx call");
            return new PyBool(false);
        }
        return m_db.ValidateCharName(arg.arg.c_str());
    }
    return new PyBool(false);
}

PyResult CharUnboundMgrService::Handle_SelectCharacterID(PyCallArgs &call) {
  /*
        arg = {charID = 140002457, loadTutorialDungeon = false, secondChoiceID = 0x7ffff0d78180}
        __FUNCTION__ = "Handle_SelectCharacterID"
*/
    CallSelectCharacterID arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "Failed to decode args for SelectCharacterID call");
        return nullptr;
    }

    call.client->SelectCharacter(arg.charID);
    return nullptr;
}

PyResult CharUnboundMgrService::Handle_GetCharactersToSelect(PyCallArgs &call) {
    return m_db.GetCharacterList(call.client->GetUserID());
}

PyResult CharUnboundMgrService::Handle_GetCharacterToSelect(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if(!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "Invalid arguments");
        return nullptr;
    }

    PyRep *result = m_db.GetCharSelectInfo(args.arg);
    if(result == NULL) {
        _log(CLIENT__ERROR, "Failed to load character %d", args.arg);
        return nullptr;
    }

    return result;
}

PyResult CharUnboundMgrService::Handle_DeleteCharacter(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "Invalid arguments for DeleteCharacter call");
        return nullptr;
    }

    return m_db.DeleteCharacter(call.client->GetUserID(), args.arg);
}

PyResult CharUnboundMgrService::Handle_PrepareCharacterForDelete(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "Invalid arguments for PrepareCharacterForDelete call");
        return nullptr;
    }

    return new PyULong(m_db.PrepareCharacterForDelete(call.client->GetUserID(), args.arg));
}

PyResult CharUnboundMgrService::Handle_CancelCharacterDeletePrepare(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "Invalid arguments for CancelCharacterDeletePrepare call");
        return nullptr;
    }

    m_db.CancelCharacterDeletePrepare(call.client->GetUserID(), args.arg);

    // the client doesn't care what we return here
    return nullptr;
}

PyResult CharUnboundMgrService::Handle_GetCharacterInfo(PyCallArgs &call) {
    _log(CLIENT__ERROR, "Called GetCharacterInfo");
    return nullptr;
}

PyResult CharUnboundMgrService::Handle_GetCharCreationInfo(PyCallArgs &call) {
    PyDict *result = new PyDict();

    //send all the cache hints needed for char creation.
    m_manager->cache_service->InsertCacheHints(ObjCacheService::hCharCreateCachables, result);
    _log(CLIENT__MESSAGE, "Sending char creation info reply");

    return result;
}

PyResult CharUnboundMgrService::Handle_GetCharNewExtraCreationInfo(PyCallArgs &call) {
    PyDict *result = new PyDict();
    m_manager->cache_service->InsertCacheHints(ObjCacheService::hCharCreateNewExtraCachables, result);
    _log(CLIENT__MESSAGE, "Sending char new extra creation info reply");
    return result;
}

PyResult CharUnboundMgrService::Handle_CreateCharacterWithDoll(PyCallArgs &call) {
    Client* pClient = call.client;
    /*
        charID = sm.RemoteSvc('charUnboundMgr').CreateCharacterWithDoll(charactername, bloodlineID, genderID, ancestryID, charInfo, portraitInfo, schoolID)
        */
    CallCreateCharacterWithDoll arg;
    if (!arg.Decode(call.tuple)) {
        codelog(CLIENT__ERROR, "Failed to decode args for CreateCharacterWithDoll call");
        return nullptr;
    }

    if (!pClient->RecPic()) {
        pClient->SendInfoModalMsg("The Portrait for this character was not received.  Your character will still be created, but the server will not have their picture.");
    }
    _log(CLIENT__MESSAGE, "CreateCharacterWithDoll called for '%s' with schoolID: %u bloodlineID: %u genderID: %u ancestryID: %u", \
                        arg.name.c_str(), arg.schoolID, arg.bloodlineID, arg.genderID, arg.ancestryID);

    // obtain character type
    m_manager->item_factory->SetUsingClient( pClient );
    const CharacterType *char_type = m_manager->item_factory->GetCharacterTypeByBloodline(arg.bloodlineID);
    if (!char_type)
        return nullptr;

    // we need to fill these to successfully create character item
    CharacterData cdata;
        cdata.accountID = pClient->GetUserID();
        cdata.gender = arg.genderID;
        cdata.ancestryID = arg.ancestryID;
        cdata.bloodlineID = arg.bloodlineID;
        cdata.schoolID = arg.schoolID;
        cdata.description = "Character Created on ";
        cdata.description += currentDateTime();
        cdata.bounty = 0;
        cdata.balance = sConfig.character.startBalance;
        cdata.aurBalance = sConfig.character.startAurBalance; // Added aurBalance    -allan 01/07/14
        cdata.securityRating = sConfig.character.startSecRating;
        cdata.logonMinutes = 0;
        cdata.title = "No Title";
        cdata.createDateTime = Win32TimeNow();
        cdata.startDateTime = cdata.createDateTime;


    //Set the character's career and race based on the school they picked.
    if (m_db.GetCareerBySchool(cdata.schoolID, cdata.raceID, cdata.careerID)) {
        //  The Specialization has been taken out in Crucible.  set to same as Career (default)
        cdata.careerSpecialityID = cdata.careerID;
    } else {
        _log(CLIENT__MESSAGE, "Could not find default School ID %u. Using Caldari Military.", cdata.schoolID);
        cdata.raceID = 1;
        cdata.careerID = 11;
        cdata.careerSpecialityID = 11;
    }

    CorpData corpData;
        corpData.corpRole = 0;
        corpData.corpAccountKey = accountingKeyCash;
        corpData.rolesAtAll = 0;
        corpData.rolesAtBase = 0;
        corpData.rolesAtHQ = 0;
        corpData.rolesAtOther = 0;

    // Variables for storing attribute bonuses
    uint8 intelligence = char_type->intelligence();
    uint8 charisma = char_type->charisma();
    uint8 perception = char_type->perception();
    uint8 memory = char_type->memory();
    uint8 willpower = char_type->willpower();

    bool defCorp = true;
    if (sConfig.character.startCorporation) { // Skip if 0
        if( m_db.DoesCorporationExist( sConfig.character.startCorporation ) ) {
            cdata.corporationID = sConfig.character.startCorporation;
            defCorp = false;
        } else
            _log(CLIENT__MESSAGE, "Could not find default Corporation ID %u. Using Career Defaults instead.", sConfig.character.startCorporation);
    }
    if (defCorp) {
        if (!m_db.GetCorporationBySchool(cdata.schoolID, cdata.corporationID))
            _log(CLIENT__MESSAGE, "Could not place character in default corporation for school.");
    }

    // Setting character's default starting position, and getting the location...
    // this also sets schoolID, corporationID and allianceID based on career
    m_db.GetLocationCorporationByCareer(cdata);

    if (sConfig.character.startStation) { // Skip if 0
        uint32 stationID = sConfig.character.startStation;
        if (!m_db.GetLocationByStation(stationID, cdata))
            _log(CLIENT__MESSAGE, "Could not find data for stationID %u.  Using Corp Default.", stationID);
    }

    //now we have (almost) all the data we need, so spawn the char item
    ItemData idata;
        idata.typeID = char_type->id();
        idata.name = arg.name;
        idata.ownerID = 1; // EVE System
        idata.quantity = 1;
        idata.singleton = true;
        idata.locationID = cdata.stationID;
    //create char item
    CharacterRef char_item = m_manager->item_factory->SpawnCharacter(idata, cdata, corpData);
    if (!char_item) {
        //a return to the client of 0 seems to be the only means of marking failure
        _log(CLIENT__ERROR, "Failed to create character '%s'", idata.name.c_str());
        return nullptr;
    }

    //this builds appearance data from strdict
    CharacterAppearance capp;
        capp.Build(char_item->itemID(), arg.avatarInfo);

    // query attribute bonuses from ancestry
    if (!m_db.GetAttributesFromAncestry(cdata.ancestryID, intelligence, charisma, perception, memory, willpower)) {
        _log(CLIENT__ERROR, "Failed to load char create details. Bloodline %u, ancestry %u.", char_type->bloodlineID(), cdata.ancestryID);
        return nullptr;
    }
    // triple attributes and save
    uint8 multiplier = sConfig.character.statMultiplier;
    char_item->SetAttribute(AttrIntelligence, intelligence * multiplier, false);
    char_item->SetAttribute(AttrCharisma, charisma * multiplier, false);
    char_item->SetAttribute(AttrPerception, perception * multiplier, false);
    char_item->SetAttribute(AttrMemory, memory * multiplier, false);
    char_item->SetAttribute(AttrWillpower, willpower * multiplier, false);

    // register name
    m_db.add_name_validation_set(char_item->itemName().c_str(), char_item->itemID());

    //load skills
    std::map<uint32, uint32> startingSkills;
    startingSkills.clear();
	//  Base Skills
    if (!m_db.GetBaseSkills(startingSkills)) {
        _log(CLIENT__ERROR, "Failed to load char Base skills. Bloodline %u, Ancestry %u.",
            char_type->bloodlineID(), cdata.ancestryID);
        return nullptr;
    }
	//  Race Skills
    if (!m_db.GetSkillsByRace(char_type->race(), startingSkills)) {
        _log(CLIENT__ERROR, "Failed to load char Race skills. Bloodline %u, Ancestry %u.",
            char_type->bloodlineID(), cdata.ancestryID);
        return nullptr;
    }
	//  Career Skills
    if (!m_db.GetSkillsByCareer(cdata.careerID, startingSkills)) {
        _log(CLIENT__ERROR, "Failed to load char Career skills for %u.", cdata.careerSpecialityID);
        return nullptr;
    }

    //spawn all the skills
    uint8 skillLevel = 0;
    EvilNumber skillPoints = 0, totalPoints = 0;
    for (auto cur : startingSkills) {
        ItemData skillItem( cur.first, char_item->itemID(), char_item->itemID(), flagSkill );
        SkillRef skill = m_manager->item_factory->SpawnSkill( skillItem );
        if (!skill) {
            _log(CLIENT__ERROR, "Failed to add skill %u to char %s(%u) during create.",
                 cur.first, char_item->itemName().c_str(), char_item->itemID());
            continue;
        }

        skillLevel = cur.second;
        skill->SetAttribute(AttrSkillLevel, skillLevel, false);
        skillPoints = skill->GetSPForLevel( (EvilNumber)skillLevel );
        skill->SetAttribute(AttrSkillPoints, skillPoints, false);
        skill->SaveItem();
        totalPoints += skillPoints;
        char_item->SaveSkillHistory(skillEventCharCreation, // this shows as "Unknown" in PD>Skill>History
                                    Win32TimeNow(),
                                    char_item->itemID(),
                                    cur.first,
                                    skillLevel,
                                    skillPoints.get_float(),
                                    totalPoints.get_float());
    }

    //now set up some initial inventory:
    /** @todo update this to reflect char career */

    // add 1 unit of "Clone Grade Alpha"
    ItemData itemCloneAlpha( 164, char_item->itemID(), char_item->locationID(), flagClone, 1 );
    itemCloneAlpha.customInfo="active";
    InventoryItemRef initInvItem = m_manager->item_factory->SpawnItem( itemCloneAlpha );
    if (!initInvItem)
        codelog(CLIENT__ERROR, "%s: Failed to spawn a starting item", char_item->itemName().c_str());

    // give the player their pod and ship
    std::string ship_name = char_item->itemName() + "'s Noob Ship";
    std::string pod_name = char_item->itemName() + "'s Capsule";

    ItemData shipItem( char_type->shipTypeID(), char_item->itemID(), char_item->locationID(), flagHangar, ship_name.c_str() );
    ShipItemRef ship_item = m_manager->item_factory->SpawnShip( shipItem );
    ship_item->SaveItem();
    ItemData podItem( itemTypeCapsule, char_item->itemID(), char_item->locationID(), flagCapsule, pod_name.c_str() );
    ShipItemRef pod_item = m_manager->item_factory->SpawnShip( podItem );
    pod_item->SaveItem();

    pClient->SetChar(char_item);        // set new charRef in client to properly set and save ship in next call
    pClient->SetShip(ship_item);
    char_item->SetActivePod( pod_item->itemID() );  // we are now keeping pod until it's destroyed.
    char_item->SaveFullCharacter();

    // we need to report the charID to the ImageServer so it can correctly assign a previously received image
    sImageServer.ReportNewCharacter(pClient->GetUserID(), char_item->itemID());

    // Release the item factory now that the character is finished being accessed:
    m_manager->item_factory->UnsetUsingClient();

    //  add charID to staticOwners
    m_db.addOwnerCache(char_item->itemID(), char_item->itemName(), char_type->id() );

    _log( CLIENT__MESSAGE, "Created New Character  - Sending ID %u as reply", char_item->itemID() );
    return new PyInt(char_item->itemID());
}
