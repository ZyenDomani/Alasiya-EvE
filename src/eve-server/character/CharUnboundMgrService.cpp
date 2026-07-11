/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
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

#include "../eve-server.h"

#include "EntityMgr.h"
#include "EVEServerConfig.h"
#include "PyServiceCD.h"
#include "account/AccountService.h"
#include "cache/ObjCacheService.h"
#include "character/CharUnboundMgrService.h"
#include "imageserver/ImageServer.h"
#include "station/StationDataMgr.h"

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

void CharUnboundMgrService::GetCharacterData(uint32 characterID, CharacterData &characterData) {
    CharacterDB::GetCharacterData(characterID, characterData);
}

PyResult CharUnboundMgrService::Handle_ValidateNameEx(PyCallArgs &call) {
    return CharacterDB::ValidateCharNameRep(PyRep::StringContent(call.tuple->GetItem(0)));
}

PyResult CharUnboundMgrService::Handle_GetCharacterToSelect(PyCallArgs &call) {
    return CharacterDB::GetCharSelectInfo(PyRep::IntegerValue(call.tuple->GetItem(0)));
}

PyResult CharUnboundMgrService::Handle_GetCharactersToSelect(PyCallArgs &call) {
    return CharacterDB::GetCharacterList(call.client->GetUserID());
}

PyResult CharUnboundMgrService::Handle_DeleteCharacter(PyCallArgs &call) {
    CharacterDB::DeleteCharacter(PyRep::IntegerValue(call.tuple->GetItem(0)));
    return nullptr;
}

PyResult CharUnboundMgrService::Handle_PrepareCharacterForDelete(PyCallArgs &call) {
    return new PyLong(CharacterDB::PrepareCharacterForDelete(call.client->GetUserID(), PyRep::IntegerValue(call.tuple->GetItem(0))));
}

PyResult CharUnboundMgrService::Handle_CancelCharacterDeletePrepare(PyCallArgs &call) {
    CharacterDB::CancelCharacterDeletePrepare(call.client->GetUserID(), PyRep::IntegerValue(call.tuple->GetItem(0)));
    return nullptr;
}

PyResult CharUnboundMgrService::Handle_IsUserReceivingCharacter(PyCallArgs &call) {
    /*  this is called when selecting the 3ed slot when there are 2 chars on account already.
     * returning true will disable creating a 3ed character.
     * returning false will allow creating a 3ed character.
     */
    if (sConfig.character.allow3edChar)
        return PyStatic.NewFalse();
    return PyStatic.NewTrue();
}

//  called from petitioner (but only when session.characterID is None)
PyResult CharUnboundMgrService::Handle_GetCharacterInfo(PyCallArgs &call) {
    // chars = sm.RemoteSvc('charUnboundMgr').GetCharacterInfo()
    _log(CLIENT__ERROR, "Called GetCharacterInfo");
    //   characterID, characterName

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

PyResult CharUnboundMgrService::Handle_SelectCharacterID(PyCallArgs &call) {
    CallSelectCharacterID arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    if (!IsCharacterID(arg.charID)) {
        sLog.Error("Client::SelectCharacter()", "CharacterID %u invalid.", arg.charID);
        call.client->SendErrorMsg("Character ID %u invalid.", arg.charID);
        return nullptr;
    }

    call.client->SelectCharacter(arg.charID);
    return nullptr;
}

PyResult CharUnboundMgrService::Handle_CreateCharacterWithDoll(PyCallArgs &call) {
    // charID = sm.RemoteSvc('charUnboundMgr').CreateCharacterWithDoll(charactername, bloodlineID, genderID, ancestryID, charInfo, portraitInfo, schoolID)
    CallCreateCharacterWithDoll arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewZero();
    }

    // check name and throw on failure before we get too far in this
    CharacterDB::ValidateCharName(PyRep::StringContent(arg.charactername));

    Client* pClient(call.client);
    pClient->CreateChar(true);

    if (!pClient->RecPic())
        pClient->SendInfoModalMsg("The Portrait for this character was not received.  Your character will still be created, but the server will not have their picture.");

    _log(CLIENT__MESSAGE, "Calling CreateCharacterWithDoll with schoolID: %i bloodlineID: %i gender: %s ancestryID: %i", \
                        arg.schoolID, arg.bloodlineID, arg.genderID == 1 ? "male" : "female", arg.ancestryID);

    // obtain character type
    const ItemType *char_type = sItemFactory.GetCharacterTypeByBloodline(arg.bloodlineID);
    if (char_type == nullptr)
        return PyStatic.NewZero();

    // we need to fill these to successfully create character item
    CharacterData cdata = CharacterData();
        cdata.accountID = pClient->GetUserID();
        cdata.gender = arg.genderID;
        cdata.ancestryID = arg.ancestryID;
        cdata.bloodlineID = arg.bloodlineID;
        cdata.schoolID = arg.schoolID;
        cdata.description = "Character Created on ";
        cdata.description += currentDateTime();
        cdata.securityRating = sConfig.character.startSecRating;
        cdata.title = "No Title";
        cdata.createDateTime = GetFileTimeNow();

    //Set the character's career and race based on the school they picked.
    if (CharacterDB::GetCareerBySchool(cdata.schoolID, cdata.raceID, cdata.careerID)) {
        //  The Specialization has been taken out in Crucible.  set to same as Career (default)
        cdata.careerSpecialityID = cdata.careerID;
    } else {
        _log(CLIENT__MESSAGE, "Could not find default SchoolID %u. Using Caldari Military.", cdata.schoolID);
        cdata.raceID = 1;
        cdata.careerID = 11;
        cdata.careerSpecialityID = 11;
    }

    CorpData corpData = CorpData();
        corpData.startDateTime = cdata.createDateTime;
        corpData.corpRole = Corp::Role::Member;
        corpData.corpAccountKey = Account::KeyType::Cash;
        corpData.rolesAtAll = Corp::Role::Member;
        corpData.rolesAtBase = Corp::Role::Member;
        corpData.rolesAtHQ = Corp::Role::Member;
        corpData.rolesAtOther = Corp::Role::Member;
        corpData.grantableRoles = Corp::Role::Member;
        corpData.grantableRolesAtBase = Corp::Role::Member;
        corpData.grantableRolesAtHQ = Corp::Role::Member;
        corpData.grantableRolesAtOther = Corp::Role::Member;

    bool defCorp(true);
    if (sConfig.character.startCorporation) { // Skip if 0
        if ( CorporationDB::DoesCorporationExist( sConfig.character.startCorporation ) ) {
            corpData.corporationID = sConfig.character.startCorporation;
            defCorp = false;
        } else {
            _log(CLIENT__MESSAGE, "Could not find default Corporation ID %u. Using Career Defaults instead.", sConfig.character.startCorporation);
        }
    }
    if (defCorp) {
        if (!CorporationDB::GetCorporationBySchool(cdata.schoolID, corpData.corporationID))
            _log(CLIENT__MESSAGE, "Could not place character in default corporation for school.");
    }

    // Setting character's default starting position, and getting the location...
    // this also sets schoolID and corporationID based on career
    CorporationDB::GetLocationCorporationByCareer(cdata, corpData.corporationID);

    if (sDataMgr.IsStation(sConfig.character.startStation)) { // Skip if 0
        cdata.stationID = sConfig.character.startStation;
        StationData sData = StationData();
        stDataMgr.GetStationData(cdata.stationID, sData);
        cdata.solarSystemID = sData.systemID;
        cdata.constellationID = sData.constellationID;
        cdata.regionID = sData.regionID;
    }

    corpData.baseID = cdata.stationID;

    cdata.typeID = char_type->id();
    cdata.name = PyRep::StringContent(arg.charactername);
    cdata.locationID = cdata.stationID;
    cdata.logonMinutes = 2;

    sItemFactory.SetUsingClient( pClient );

    // we have enough data to spawn a new character, so lets do it.
    CharacterRef charRef = sItemFactory.SpawnCharacter(cdata, corpData);
    if (charRef.get() == nullptr) {
        //a return to the client of 0 seems to be the only means of marking failure
        _log(CLIENT__ERROR, "Failed to create character '%s'", cdata.name.c_str());
        sItemFactory.UnsetUsingClient();
        return PyStatic.NewZero();
    }

    charRef->SetClient(pClient);      // set client in char

    // add call to JoinCorp here, and remove corp shit from charDB

    //this builds and saves appearance data from charInfo dict
    CharacterAppearance capp;
        capp.Build(charRef->itemID(), arg.charInfo);

    //this builds and saves portrait data from portraitInfo dict
    CharacterPortrait cpor;
        cpor.Build(charRef->itemID(), arg.portraitInfo);

    // set attribute bonuses from ancestry
    Char::AttrData ancestryData = Char::AttrData();
    sDataMgr.GetAncestryBonuses(cdata.ancestryID, ancestryData);
    // query attribute bonuses from bloodline
    Char::AttrData bloodlineData = Char::AttrData();
    sDataMgr.GetBloodlineBonuses(cdata.bloodlineID, bloodlineData);

    // set attrib bonuses on char ...  update this to get base data from db, incase i change it again
    charRef->SetAttribute(AttrIntelligence, 25 + ancestryData.intelligence + bloodlineData.intelligence, false);
    charRef->SetAttribute(AttrCharisma, 25 + ancestryData.charisma + bloodlineData.charisma, false);
    charRef->SetAttribute(AttrPerception, 25 + ancestryData.perception + bloodlineData.perception, false);
    charRef->SetAttribute(AttrMemory, 25 + ancestryData.memory + bloodlineData.memory, false);
    charRef->SetAttribute(AttrWillpower, 25 + ancestryData.willpower + bloodlineData.willpower, false);
    charRef->SetAttribute(AttrIntelligenceBonus, ancestryData.intelligence + bloodlineData.intelligence, false);
    charRef->SetAttribute(AttrCharismaBonus, ancestryData.charisma + bloodlineData.charisma, false);
    charRef->SetAttribute(AttrPerceptionBonus, ancestryData.perception + bloodlineData.perception, false);
    charRef->SetAttribute(AttrMemoryBonus, ancestryData.memory + bloodlineData.memory, false);
    charRef->SetAttribute(AttrWillpowerBonus, ancestryData.willpower + bloodlineData.willpower, false);
    charRef->SaveAttributes();

    //load skills
    std::map<uint32, uint8> startingSkills;
	//  Base Skills
    if (!CharacterDB::GetBaseSkills(startingSkills)) {
        _log(CLIENT__ERROR, "Failed to load char Base skills. Bloodline %u, Ancestry %u.",
             cdata.bloodlineID, cdata.ancestryID);
        // dont really care if this fails.  not enough to deny creation ...maybe make error?
    }
	//  Race Skills
    if (!CharacterDB::GetSkillsByRace(char_type->race(), startingSkills)) {
        _log(CLIENT__ERROR, "Failed to load char Race skills. Bloodline %u, Ancestry %u.",
             cdata.bloodlineID, cdata.ancestryID);
        // dont really care if this fails.  not enough to deny creation ...maybe make error?
    }
	//  Career Skills
    if (!CharacterDB::GetSkillsByCareer(cdata.careerID, startingSkills)) {
        _log(CLIENT__ERROR, "Failed to load char Career skills for %u.", cdata.careerSpecialityID);
        // dont really care if this fails.  not enough to deny creation ...maybe make error?
    }

    //spawn all the skills
    uint32 skillPoints = 0;
    for (auto &cur : startingSkills) {
        ItemData skillItem( cur.first, charRef->itemID(), charRef->itemID(), flagSkill );
        SkillRef skill = sItemFactory.SpawnSkill( skillItem );
        if (skill.get() == nullptr) {
            _log(CLIENT__ERROR, "Failed to add skill %u to char %s(%u) during create.",
                 cur.first, charRef->name(), charRef->itemID());
            // missed a skill...whatever.
            continue;
        }

        skill->SetAttribute(AttrSkillLevel, cur.second, false);
        skillPoints = skill->GetSPForLevel(cur.second);
        skill->SetAttribute(AttrSkillPoints, skillPoints, false);
        skill->SaveItem();
        cdata.skillPoints += skillPoints;
        charRef->SaveSkillHistory(EvESkill::Event::CharCreation,
                                  GetFileTimeNow(),
                                    charRef->itemID(),
                                    cur.first,
                                    cur.second,
                                    skillPoints);
    }

    //now set up some initial inventory:
    /** @todo update this to reflect char career */

    // add client to EntityMgr for subsequent calls that need Client* (AttributeMgr changes)
    pClient->SetChar(charRef);        // AddPlayer() needs charRef
    sEntityMgr.AddPlayer(pClient);

    // need system loaded for proper ship creation/loading and subsquent character login
    sEntityMgr.FindOrBootSystem(cdata.solarSystemID);

    // give the player their pod and set in system (NOT hangar)
    pClient->CreateNewPod();
    pClient->GetPod()->Move(cdata.solarSystemID, flagCapsule, false);

    ShipItemRef sRef = pClient->SpawnNewRookieShip(cdata.locationID);
    // set shipID in client and char objects and save (shipID error fix)
    pClient->SetShip(sRef);
    charRef->SaveFullCharacter();

    // Release the item factory now that the character is finished being accessed:
    sItemFactory.UnsetUsingClient();

    // we need to report the charID to the ImageServer so it can correctly assign a previously received image
    sImageServer.ReportNewCharacter(pClient->GetUserID(), charRef->itemID());

    //  add charID to staticOwners
    CharacterDB::AddOwnerCache(charRef->itemID(), charRef->itemName(), char_type->id());

    std::string reason = "DESC: Inheritance Payment to ";
    reason += charRef->itemName();
    AccountService::TransferFunds(corpSCC, charRef->itemID(), sConfig.character.startBalance, reason, Journal::EntryType::Inheritance);
    AccountService::TransferFunds(corpSCC, charRef->itemID(), sConfig.character.startAurBalance, std::move(reason), \
                            Journal::EntryType::Inheritance, 0, Account::KeyType::AUR, Account::KeyType::AUR);

    charRef->LogOut();
    sRef->LogOut();

    sEntityMgr.RemovePlayer(pClient);

    pClient->CreateChar(false);

    _log( CLIENT__MESSAGE, "Created New Character - Sending charID %u as reply", charRef->itemID() );

    return new PyInt(charRef->itemID());
}

