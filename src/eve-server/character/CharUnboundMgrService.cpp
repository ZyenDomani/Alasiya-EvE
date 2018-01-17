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
#include "account/AccountService.h"
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

void CharUnboundMgrService::GetCharacterData(uint32 characterID, std::map< std::string, int64 >& characterDataMap)
{
    m_db.GetCharacterData(characterID, characterDataMap);
}

PyResult CharUnboundMgrService::Handle_IsUserReceivingCharacter(PyCallArgs &call) {
    _log(CLIENT__ERROR, "Called IsUserReceivingCharacter");
    /*  this is called when selecting the 3ed slot, when there are 2 chars on account already.
     * returning true will disable creating a 3ed character.
     * returning false will allow creating a 3ed character.
     */
    if (sConfig.character.allow3edChar)
        return new PyBool(false);
    return new PyBool(true);
}

PyResult CharUnboundMgrService::Handle_ValidateNameEx(PyCallArgs &call)
{
    if (call.tuple->GetItem(0)->IsString()) {
        Call_SingleStringArg arg;
        if (!arg.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
            return new PyInt(-1);
        }
        return m_db.ValidateCharName(arg.arg.c_str());
    } else if (call.tuple->GetItem(0)->IsWString()) {
        Call_SingleWStringArg arg;
        if (!arg.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
            return new PyInt(-1);
        }
        return m_db.ValidateCharName(arg.arg.c_str());
    } else
        _log(CLIENT__ERROR, "ValidateName() called with unhandled type %s", call.tuple->GetItem(0)->TypeString());

    return new PyInt(-1);
}

PyResult CharUnboundMgrService::Handle_SelectCharacterID(PyCallArgs &call)
{
    CallSelectCharacterID arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    call.client->SelectCharacter(arg.charID);
    return nullptr;
}

PyResult CharUnboundMgrService::Handle_GetCharactersToSelect(PyCallArgs &call)
{
    return m_db.GetCharacterList(call.client->GetUserID());
}

PyResult CharUnboundMgrService::Handle_GetCharacterToSelect(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    return m_db.GetCharSelectInfo(args.arg);
}

PyResult CharUnboundMgrService::Handle_DeleteCharacter(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    m_db.DeleteCharacter(args.arg);
    return nullptr;
}

PyResult CharUnboundMgrService::Handle_PrepareCharacterForDelete(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    return new PyLong(m_db.PrepareCharacterForDelete(call.client->GetUserID(), args.arg));
}

PyResult CharUnboundMgrService::Handle_CancelCharacterDeletePrepare(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
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
    // charID = sm.RemoteSvc('charUnboundMgr').CreateCharacterWithDoll(charactername, bloodlineID, genderID, ancestryID, charInfo, portraitInfo, schoolID)
    CallCreateCharacterWithDoll arg;
    if (!arg.Decode(call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    Client* pClient = call.client;
    if (!pClient->RecPic()) {
        pClient->SendInfoModalMsg("The Portrait for this character was not received.  Your character will still be created, but the server will not have their picture.");
    }
    _log(CLIENT__MESSAGE, "CreateCharacterWithDoll called with schoolID: %i bloodlineID: %i genderID: %i ancestryID: %i", \
                        arg.schoolID, arg.bloodlineID, arg.genderID, arg.ancestryID);

    // obtain character type
    sItemFactory.SetUsingClient( pClient );
    const CharacterType *char_type = sItemFactory.GetCharacterTypeByBloodline(arg.bloodlineID);
    if (char_type == nullptr)
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
        cdata.balance = /*sConfig.character.startBalance*/0;    // updated to use TranserFunds and record journal entry
        cdata.aurBalance = /*sConfig.character.startAurBalance*/0; // Added aurBalance    -allan 01/07/14
        cdata.securityRating = sConfig.character.startSecRating;
        cdata.logonMinutes = 0;
        cdata.title = "No Title";
        cdata.createDateTime = (int64)GetFileTimeNow();
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

    if (IsStation(sConfig.character.startStation)) { // Skip if 0
        cdata.stationID = sConfig.character.startStation;
        if (!m_db.GetLocationByStation(cdata.stationID, cdata))
            _log(CLIENT__MESSAGE, "Could not find data for stationID %u.  Using Corp Default.", sConfig.character.startStation);
    }

    std::string name = "";
    if (arg.name->IsWString())
        name = arg.name->AsWString()->content();
    else if (arg.name->IsString())
        name = arg.name->AsString()->content();

    cdata.typeID = char_type->id();
    cdata.name = name;
    cdata.locationID = cdata.stationID;
    cdata.logonMinutes = 2;

    CorpData corpData;
        corpData.corporationID = cdata.corporationID;
        corpData.baseID = cdata.stationID;
        corpData.corpRole = Corp::Role::Member;
        corpData.corpAccountKey = Account::KeyType::Cash;
        corpData.rolesAtAll = Corp::Role::None;
        corpData.rolesAtBase = Corp::Role::None;
        corpData.rolesAtHQ = Corp::Role::None;
        corpData.rolesAtOther = Corp::Role::None;
        corpData.grantableRoles = Corp::Role::None;
        corpData.grantableRolesAtBase = Corp::Role::None;
        corpData.grantableRolesAtHQ = Corp::Role::None;
        corpData.grantableRolesAtOther = Corp::Role::None;
        // these arent needed yet, but set to 0 to avoid trash data
        corpData.taxRate = 0;
        corpData.corpHQ = 0;
        corpData.allianceID = 0;
        corpData.warFactionID = 0;

    CharacterRef charRef = sItemFactory.SpawnCharacter(cdata, corpData);
    if (charRef.get() == nullptr) {
        //a return to the client of 0 seems to be the only means of marking failure
        _log(CLIENT__ERROR, "Failed to create character '%s'", cdata.name.c_str());
        return nullptr;
    }

    // add call to JoinCorp here, and remove corp shit from charDB

    //this builds appearance data from strdict
    CharacterAppearance capp;
        capp.Build(charRef->itemID(), arg.avatarInfo);

    // query attribute bonuses from ancestry
    if (!m_db.GetAttributesFromAncestry(cdata.ancestryID, intelligence, charisma, perception, memory, willpower)) {
        _log(CLIENT__ERROR, "Failed to load char create details. Bloodline %u, ancestry %u.", char_type->bloodlineID(), cdata.ancestryID);
        return nullptr;
    }
    // triple attributes and save
    uint8 multiplier = sConfig.character.statMultiplier;
    charRef->SetAttribute(AttrIntelligence, intelligence * multiplier, false);
    charRef->SetAttribute(AttrCharisma, charisma * multiplier, false);
    charRef->SetAttribute(AttrPerception, perception * multiplier, false);
    charRef->SetAttribute(AttrMemory, memory * multiplier, false);
    charRef->SetAttribute(AttrWillpower, willpower * multiplier, false);

    // register name
    m_db.add_name_validation_set(charRef->itemName().c_str(), charRef->itemID());

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
        ItemData skillItem( cur.first, charRef->itemID(), charRef->itemID(), flagSkill );
        SkillRef skill = sItemFactory.SpawnSkill( skillItem );
        if (skill.get() == nullptr) {
            _log(CLIENT__ERROR, "Failed to add skill %u to char %s(%u) during create.",
                 cur.first, charRef->itemName().c_str(), charRef->itemID());
            continue;
        }

        skillLevel = cur.second;
        skill->SetAttribute(AttrSkillLevel, skillLevel, false);
        skillPoints = skill->GetSPForLevel( (EvilNumber)skillLevel );
        skill->SetAttribute(AttrSkillPoints, skillPoints, false);
        skill->SaveItem();
        totalPoints += skillPoints;
        charRef->SaveSkillHistory(skillEventSkillPointsApplied/*skillEventCharCreation*/, // #33 shows as "Unknown" in PD>Skill>History
                                  GetFileTimeNow(),
                                    charRef->itemID(),
                                    cur.first,
                                    skillLevel,
                                    skillPoints.get_double());
    }

    //now set up some initial inventory:
    /** @todo update this to reflect char career */

    // add 1 unit of "Clone Grade Alpha"
    ItemData itemCloneAlpha( 164, charRef->itemID(), cdata.stationID, flagClone, 1 );
    itemCloneAlpha.customInfo="active";
    InventoryItemRef initInvItem = sItemFactory.SpawnItem( itemCloneAlpha );
    if (initInvItem.get() == nullptr)
        codelog(CLIENT__ERROR, "%s: Failed to spawn a starting item", charRef->itemName().c_str());

    // give the player their pod
    std::string pod_name = charRef->itemName() + "'s Capsule";
    ItemData podItem( itemTypeCapsule, charRef->itemID(), cdata.solarSystemID, flagCapsule, pod_name.c_str() );
    ShipItemRef pod_item = sItemFactory.SpawnShip( podItem );
    if (pod_item.get() != nullptr) {
        pod_item->SaveItem();
        charRef->SetActivePod( pod_item->itemID() );  // we are now keeping pod until it's destroyed.
    }
    pClient->SetChar(charRef);        // set new charRef in client
    pClient->SetShip(pClient->SpawnNewRookieShip());

    CharacterDB::AddEmployment(charRef->itemID(), cdata.corporationID);
    charRef->SetFlag(flagAutoFit);
    charRef->SaveFullCharacter();

    // we need to report the charID to the ImageServer so it can correctly assign a previously received image
    sImageServer.ReportNewCharacter(pClient->GetUserID(), charRef->itemID());

    // Release the item factory now that the character is finished being accessed:
    sItemFactory.UnsetUsingClient();

    //  add charID to staticOwners
    m_db.addOwnerCache(charRef->itemID(), charRef->itemName(), char_type->id() );

    std::string reason = "DESC: Inheritance Payment to ";
    reason += charRef->itemName().c_str();
    AccountService::TranserFunds(ownerSCC, charRef->itemID(), sConfig.character.startBalance, reason, Journal::EntryType::Inheritance);
    AccountService::TranserFunds(ownerSCC, charRef->itemID(), sConfig.character.startAurBalance, reason, \
                                    Journal::EntryType::Inheritance, Account::KeyType::AUR, Account::KeyType::AUR);

    _log( CLIENT__MESSAGE, "Created New Character  - Sending ID %u as reply", charRef->itemID() );
    return new PyInt(charRef->itemID());
}
