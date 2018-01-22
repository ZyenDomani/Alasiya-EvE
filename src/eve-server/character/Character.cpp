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
    Author:     Zhur, Bloody.Rabbit
    Updates:        Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "ConsoleCommands.h"
#include "EntityList.h"
#include "StaticDataMgr.h"
#include "account/AccountService.h"
#include "character/Character.h"
#include "effects/EffectsProcessor.h"
#include "fleet/FleetService.h"
#include "inventory/AttributeEnum.h"
#include "ship/Ship.h"

/*
 * CharacterTypeData
 */
CharacterTypeData::CharacterTypeData(
    const char* _bloodlineName,
    EVERace _race,
    const char *_desc,
    const char *_maleDesc,
    const char *_femaleDesc,
    uint32 _shipTypeID,
    uint32 _corporationID,
    uint8 _perception,
    uint8 _willpower,
    uint8 _charisma,
    uint8 _memory,
    uint8 _intelligence,
    const char* _shortDesc,
    const char* _shortMaleDesc,
    const char* _shortFemaleDesc)
: bloodlineName(_bloodlineName),
  race(_race),
  description(_desc),
  maleDescription(_maleDesc),
  femaleDescription(_femaleDesc),
  shipTypeID(_shipTypeID),
  corporationID(_corporationID),
  perception(_perception),
  willpower(_willpower),
  charisma(_charisma),
  memory(_memory),
  intelligence(_intelligence),
  shortDescription(_shortDesc),
  shortMaleDescription(_shortMaleDesc),
  shortFemaleDescription(_shortFemaleDesc)
{
}

/*
 * CharacterType
 */
CharacterType::CharacterType(
    uint32 _id,
    uint8 _bloodlineID,
    // ItemType stuff:
    const ItemGroup &_group,
    const TypeData &_data,
    // CharacterType stuff:
    const ItemType &_shipType,
    const CharacterTypeData &_charData)
: ItemType(_id, _group, _data),
  m_bloodlineID(_bloodlineID),
  m_bloodlineName(_charData.bloodlineName),
  m_description(_charData.description),
  m_maleDescription(_charData.maleDescription),
  m_femaleDescription(_charData.femaleDescription),
  m_shipType(_shipType),
  m_corporationID(_charData.corporationID),
  m_perception(_charData.perception),
  m_willpower(_charData.willpower),
  m_charisma(_charData.charisma),
  m_memory(_charData.memory),
  m_intelligence(_charData.intelligence),
  m_shortDescription(_charData.shortDescription),
  m_shortMaleDescription(_charData.shortMaleDescription),
  m_shortFemaleDescription(_charData.shortFemaleDescription)
{
    // check for consistency
    assert(_data.race == _charData.race);
    assert(_charData.shipTypeID == _shipType.id());
}

CharacterType *CharacterType::Load( uint32 characterTypeID)
{
    return ItemType::Load<CharacterType>(characterTypeID );
}

/*
 * CharacterAppearance
 */

void CharacterAppearance::Build(uint32 ownerID, PyDict* data)
{
	PyList* colors = new PyList();
        colors = data->GetItemString("colors")->AsList();
	PyList::const_iterator color_cur = colors->begin();
	for (; color_cur != colors->end(); color_cur++) {
		if ((*color_cur)->IsObjectEx()) {
			PyObjectEx_Type2* color_obj = (PyObjectEx_Type2*)(*color_cur)->AsObjectEx();
			PyTuple* color_tuple = color_obj->GetArgs()->AsTuple();

			//color tuple data structure
			//[0] PyToken
			//[1] colorID
			//[2] colorNameA
			//[3] colorNameBC
			//[4] weight
			//[5] gloss

			m_db.SetAvatarColors(ownerID,
								color_tuple->GetItem(1)->AsInt()->value(),
								color_tuple->GetItem(2)->AsInt()->value(),
								color_tuple->GetItem(3)->AsInt()->value(),
								color_tuple->GetItem(4)->AsFloat()->value(),
								color_tuple->GetItem(5)->AsFloat()->value());

		}
	}

    PyObjectEx* appearance;
        appearance = data->GetItemString("appearance")->AsObjectEx();
	PyObjectEx_Type2* app_obj = (PyObjectEx_Type2*)appearance;
	PyTuple* app_tuple = app_obj->GetArgs()->AsTuple();

	m_db.SetAvatar(ownerID, app_tuple->GetItem(1));

    PyList* modifiers = new PyList();
        modifiers = data->GetItemString("modifiers")->AsList();
	PyList::const_iterator modif_cur = modifiers->begin();
	for (; modif_cur != modifiers->end(); modif_cur++) {
		if ((*modif_cur)->IsObjectEx()) {
			PyObjectEx_Type2* modif_obj = (PyObjectEx_Type2*)(*modif_cur)->AsObjectEx();
			PyTuple* modif_tuple = modif_obj->GetArgs()->AsTuple();

			//color tuple data structure
			//[0] PyToken
			//[1] modifierLocationID
			//[2] paperdollResourceID
			//[3] paperdollResourceVariation
			m_db.SetAvatarModifiers(ownerID,
										modif_tuple->GetItem(1),
										modif_tuple->GetItem(2),
										modif_tuple->GetItem(3));
		}
	}

    PyList* sculpts = new PyList();
        sculpts = data->GetItemString("sculpts")->AsList();
	PyList::const_iterator sculpt_cur = sculpts->begin();
	for (; sculpt_cur != sculpts->end(); sculpt_cur++) {
		if ((*sculpt_cur)->IsObjectEx()) {
			PyObjectEx_Type2* sculpt_obj = (PyObjectEx_Type2*)(*sculpt_cur)->AsObjectEx();
			PyTuple* sculpt_tuple = sculpt_obj->GetArgs()->AsTuple();

			//sculpts tuple data structure
			//[0] PyToken
			//[1] sculptLocationID
			//[2] weightUpDown
			//[3] weightLeftRight
			//[4] weightForwardBack

			m_db.SetAvatarSculpts(ownerID,
									sculpt_tuple->GetItem(1),
									sculpt_tuple->GetItem(2),
									sculpt_tuple->GetItem(3),
									sculpt_tuple->GetItem(4));

		}
	}
}


/*
 * Character
 */
Character::Character(
    uint32 _characterID,
    // InventoryItem stuff:
    const CharacterType &_charType,
    const ItemData &_data,
    // Character stuff:
    const CharacterData &_charData,
    const CorpData &_corpData)
: InventoryItem(_characterID, _charType, _data),
  m_charData(_charData),
  m_corpData(_corpData),
  m_pClient(nullptr)
{
    // allow characters to be only singletons
    assert(singleton());

    m_loaded = false;

    if (!IsAgent(m_itemID)) {
        m_fleetData.fleetID = 0;
        m_fleetData.wingID = 0;
        m_fleetData.squadID = 0;
        m_fleetData.job = 0;
        m_fleetData.role = 0;
        m_fleetData.booster = 0;
        m_fleetData.joinTime = 0;
        m_freePoints = 0;
        m_loginTime = sEntityList.GetStamp();
        pInventory = new Inventory(InventoryItemRef(this));
    }
}

Character::~Character()
{
    SaveBookMarks();
    SaveFullCharacter();
    SaveCertificates();
    SafeDelete(pInventory);
}

CharacterRef Character::Load( uint32 characterID) {
    return InventoryItem::Load<Character>( characterID );
}

bool Character::_Load() {
    if (m_loaded)
        return true;
    if (IsAgent(m_itemID))
        return true;

    if (!pInventory->LoadContents()) {
        sLog.Warning("Character::_Load","LoadContents returned false for char %u", m_itemID);
        return (m_loaded = false);
    }
    if (!m_db.LoadSkillQueue(m_itemID, m_skillQueue)) {
        sLog.Warning("Character::_Load","LoadSkillQueue returned false for char %u", m_itemID);
        return (m_loaded = false);
    }

    m_loaded = InventoryItem::_Load();

    // Update Skill Queue and Total Skill Points Trained:
    if (m_loaded) {
        if (GetSkillInTraining())
            UpdateSkillQueue();
        VerifySP();
        m_certificates.clear();
        if (!m_cdb.LoadCertificates(m_itemID, m_certificates)) {
            sLog.Warning("Character::_Load","LoadCertificates returned false for char %u", m_itemID);
            return (m_loaded = false);
        }
    }

    // get all skills, implants, and boosters, then process each, and store processed effect in m_modifiers.
    // m_modifiers will be looped and applied when ship is boarded.
    // no reason to delete m_modifiers as they are generic, and nothing changes in char when appling to ship.
    //   ....yes, they do.  skills apply to other skills, ships, modules, etc.
    //  it all gets reset when docking, and undock applies everything, so char will need reset also.
    //ProcessEffects();

    // load char personal bookmarks and folders ... corp shit will be done ???
    LoadBookmarks();

    m_charData.loginTime = GetFileTimeNow();

    return m_loaded;
}

void Character::VerifySP()
{
    std::vector<InventoryItemRef> skillList;
    GetSkillsList(skillList);
    for (auto cur : skillList) {
        SkillRef::StaticCast(cur)->VerifySP();
    }
}

CharacterRef Character::Spawn( CharacterData& charData, CorpData& corpData) {
    // make sure it's a character
    const CharacterType *ct = sItemFactory.GetCharacterType(charData.typeID);
    if (ct == nullptr)
        return CharacterRef(nullptr);

    uint32 characterID = CharacterDB::NewCharacter(charData, corpData);
    if (characterID == 0) {
        _log(CHARACTER__ERROR, "Failed to get ItemID for new character.");
        return CharacterRef(nullptr);
    }

    return Character::Load(characterID );
}

void Character::LogOut()
{
    SaveCharacter();
    m_db.SetLogOffTime(m_itemID);
    if (!sConsole.IsShutdown())
        if (IsFleet(m_fleetData.fleetID))
            sFltSvc.LeaveFleet(m_pClient);

    sItemFactory.RemoveItem(m_itemID);
    // remove char from station inventory, if docked
    /*
    Inventory* inv(nullptr);
    if (IsStation(m_locationID)) {
        InventoryItemRef station = sEntityList.GetStationByID(m_locationID);
        inv = station->GetMyInventory();
    }
    if (inv != nullptr)
        inv->RemoveItem(inv->GetByID(m_itemID));
    */
}

void Character::Delete() {
    // delete contents
    pInventory->DeleteContents();
    // delete character record
    m_db.DeleteCharacter(m_itemID);
    // let the parent care about the rest
    InventoryItem::Delete();
}

double Character::balance(uint8 type)
{
    if (type == Account::CreditType::ISK)
        return m_charData.balance;
    else if (type == Account::CreditType::AURUM)
        return m_charData.aurBalance;
    else
        _log(ACCOUNT__ERROR, "Character::balance() - invalid type %u", type);
    return 0;
}

bool Character::AlterBalance(double amount, uint8 type) {
    if (amount == 0)
        return true;

    // amount can be negative.  check for funds to remove, if applicable
    if ((balance(type) + amount) < 0) {
        std::map<std::string, PyRep *> args;
        args["amount"] = new PyFloat(amount);
        args["balance"] = new PyFloat(balance(type));
        throw(PyException(MakeUserError("NotEnoughMoney", args)));
        return false;
    }

    //adjust balance and send notification of change
    OnAccountChange ac;
    ac.ownerid = m_itemID;
    if (type == Account::CreditType::ISK) {
        m_charData.balance += amount;
        ac.balance = m_charData.balance;
        ac.accountKey = "cash";
    } else if (type == Account::CreditType::AURUM) {
        m_charData.aurBalance += amount;
        ac.balance = m_charData.aurBalance;
        ac.accountKey = "AURUM";
    }

    PyTuple *answer = ac.Encode();
    m_pClient->SendNotification("OnAccountChange", "cash", &answer, false);

    return true;
}

void Character::SetLocation(uint32 stationID, uint32 solarSystemID, uint32 constellationID, uint32 regionID) {
    m_charData.locationID = (stationID == 0 ? solarSystemID : stationID);
    m_charData.stationID = stationID;
    m_charData.solarSystemID = solarSystemID;
    m_charData.constellationID = constellationID;
    m_charData.regionID = regionID;
    SaveCharacter();
}

void Character::SetDescription(const char *newDescription) {
    m_charData.description = newDescription;
    SaveCharacter();
}

void Character::JoinCorporation(const CorpData &data) {
    m_corpData = data;
    // Add new employment history record    -allan  25Mar14   update 20Jan15
    CharacterDB::AddEmployment(m_itemID, m_corpData.corporationID);
    m_pClient->UpdateCorpSession(m_corpData);
    /** @todo remove this in favor of updating data when changed */
    m_db.SaveCorpData(m_itemID, m_corpData);
}

void Character::SetAccountKey(int32 accountKey)
{
    m_corpData.corpAccountKey = accountKey;
    /** @todo remove this in favor of updating data when changed */
    m_db.SaveCorpData(m_itemID, m_corpData);
    m_pClient->UpdateCorpSession(m_corpData);
}

void Character::SetBaseID(uint32 baseID)
{
    m_corpData.baseID = baseID;
    /** @todo remove this in favor of updating data when changed */
    m_db.SaveCorpData(m_itemID, m_corpData);
    m_pClient->UpdateCorpSession(m_corpData);
}

void Character::UpdateCorpData(CorpData data)
{
    m_corpData = data;
    m_pClient->UpdateCorpSession(m_corpData);
}

uint32 Character::PickAlternateShip(uint32 locationID)
{
    return m_db.PickAlternateShip(m_itemID, locationID);
}

void Character::SetFleetData(CharFleetData& fleet)
{
    if (fleet.fleetID == 0) {
        m_fleetData.fleetID = 0;
        m_fleetData.wingID = 0;
        m_fleetData.squadID = 0;
        m_fleetData.job = 0;
        m_fleetData.role = 0;
        m_fleetData.booster = 0;
        m_fleetData.joinTime = 0;
    } else {
        m_fleetData = fleet;
        //if ((fleet.joinTime) and (m_fleetJoinTime != fleet.joinTime))
          //  m_fleetJoinTime = fleet.joinTime;
    }
    m_pClient->UpdateFleetSession(m_fleetData);
}


bool Character::HasSkill(uint32 skillTypeID) const {
    return GetSkill(skillTypeID);
}

bool Character::HasSkillTrainedToLevel(uint32 skillTypeID, uint32 skillLevel) const {
    SkillRef requiredSkill = GetSkill( skillTypeID );
    // First, check for existence of skill trained or in training:
    if (!requiredSkill) return false;
    // Second, check for required minimum level of skill, note it must already be trained to this level:
    if (requiredSkill->GetAttribute(AttrSkillLevel) < skillLevel) return false;
    return true;
}

SkillRef Character::GetSkill(uint32 skillTypeID) const
{
    InventoryItemRef skill = pInventory->GetByTypeFlag( skillTypeID, flagSkill );
    if (skill.get() == nullptr)
        skill = pInventory->GetByTypeFlag( skillTypeID, flagSkillInTraining );

    return SkillRef::StaticCast( skill );
}

int8 Character::GetSkillLevel(uint32 skillTypeID, bool zeroForNotInjected /*true*/) const {
    SkillRef requiredSkill = GetSkill( skillTypeID );
    // First, check for existence of skill trained or in training:
    if (requiredSkill.get() == nullptr)
        return (zeroForNotInjected ? 0 : -1);
    return (int8)requiredSkill->GetAttribute(AttrSkillLevel).get_int() ;
}

PyRep* Character::GetRAMSkills()
{
    /*  this queries RAM skills and is used to display blueprints tab (S&I -> Blueprints)
     *      called by RamProxy::GetRelevantCharSkills()
     *
     *            skillLevels, attributeValues = sm.GetService('manufacturing').GetRelevantCharSkills()
     *            maxManufacturingJobCount = int(attributeValues[const.attributeManufactureSlotLimit])    -AttrManufactureSlotLimit = 196,
     *            maxResearchJobCount = int(attributeValues[const.attributeMaxLaborotorySlots])           -AttrMaxLaborotorySlots = 467,
     *
     *            skillLevels <<  this is a dict. of max remote ram jobs
     *            attributeValues  << this is a dict. of max ram jobs
     */

    PyDict* skillLevels = new PyDict();
    PyDict* attributeValues = new PyDict();

    skillLevels->SetItem(new PyInt(EVEDB::invTypes::typeScientificNetworking), new PyInt(GetSkillLevel(skillScientificNetworking)));
    skillLevels->SetItem(new PyInt(EVEDB::invTypes::typeSupplyChainManagement), new PyInt(GetSkillLevel(skillSupplyChainManagement)));

    uint8 mLab = 1 + GetSkillLevel(skillLaboratoryOperation) + GetSkillLevel(skillAdvancedLaboratoryOperation);
    attributeValues->SetItem(new PyInt(AttrMaxLaborotorySlots), new PyInt(mLab));

    uint8 mSlot = 1 + GetSkillLevel(skillMassProduction) + GetSkillLevel(skillAdvancedMassProduction);
    attributeValues->SetItem(new PyInt(AttrManufactureSlotLimit), new PyInt(mSlot));

    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, skillLevels);
        tuple->SetItem(1, attributeValues);
    return tuple;
}

void Character::ResetModifiers()
{
    ClearModifiers();
    std::vector<InventoryItemRef> allSkills;
    GetSkillsList(allSkills);
    for (auto curSkill : allSkills)
        curSkill->ClearModifiers();
}

void Character::ProcessEffects()
{
    //  427 total skills.  this should be fairly fast...it is.
    std::vector<InventoryItemRef> allSkills;
    GetSkillsList(allSkills);

    _log(EFFECTS__TRACE, "Character::ParseExpression():  Beginning Character Effects Processing.");
    Effect curEffect;
    fxData data;
    data.action = Effects::Action::dgmActInvalid;
    std::vector<TypeEffects> typeFx;
    for (auto curSkill : allSkills) {
        typeFx.clear();
        sFxDataMgr.GetTypeEffect(curSkill->typeID(), typeFx);
        for (auto curFx : typeFx) {
            curEffect = sFxDataMgr.GetEffect(curFx.effectID);
            data.srcRef = curSkill;
            data.math = data.targLoc = data.fxSrc = data.targAttr = data.srcAttr = data.grpID = data.typeID = 0;
            sFxProc.ParseExpression(this, sFxDataMgr.GetExpression(curEffect.preExpression), data);
        }
    }
}

SkillRef Character::GetSkillInTraining() const {
    InventoryItemRef item;
    pInventory->FindSingleByFlag(flagSkillInTraining, item);
    return SkillRef::StaticCast( item );
}

void Character::GetSkillsList(std::vector<InventoryItemRef> &skills) const
{
    pInventory->FindByFlag( flagSkillInTraining, skills );
    pInventory->FindByFlag( flagSkill, skills );
}

EvilNumber Character::GetSPPerMin(Skill* skill)
{
	return EvEMath::Skill::PointsPerMinute(GetAttribute(skill->GetAttribute(AttrPrimaryAttribute).get_int()), GetAttribute(skill->GetAttribute(AttrSecondaryAttribute).get_int()));
}

double Character::GetEndOfTraining() const {
    InventoryItemRef item;
    if (pInventory->FindSingleByFlag(flagSkillInTraining, item))
        return item->GetAttribute(AttrExpiryTime).get_double();
    return 0;
}

bool Character::InjectSkillIntoBrain ( SkillRef skill, uint8 level )
{
    return false;
}

bool Character::InjectSkillIntoBrain(SkillRef skill) {
    if (m_pClient == nullptr)
        return false;

    SkillRef oldSkill = GetSkill( skill->typeID() );
    if (oldSkill.get() != nullptr) {
        /** @todo: build and send proper UserError for CharacterAlreadyKnowsSkill. */
        m_pClient->SendNotifyMsg( "You already know this skill." );
        return false;
    }

    /** @todo config option (later) to check if another character on this account is training a skill.
     *  If so, send error, cancel inject and return. (flagID=61).
     */

    if ( !skill->SkillPrereqsComplete( *this ) ) {
        /** @todo need to send back a response to the client.  need packet specs. */
        _log( CHARACTER__MESSAGE, "%s (%u): Requested to train skill %u item %u but prereq not complete.", itemName().c_str(), m_itemID, skill->typeID(), skill->itemID() );
        m_pClient->SendNotifyMsg( "Injection failed!  Skill prerequisites incomplete." );
        return false;
    }

    // are we injecting from a stack of skills?
    if ( skill->quantity() > 1 ) {
        // split the stack to obtain single item
        InventoryItemRef single_skill = skill->Split( 1 );
        if (single_skill.get() == nullptr ) {
            _log( ITEM__ERROR, "%s (%u): Unable to split stack of %s (%u).", itemName().c_str(), m_itemID, skill->itemName().c_str(), skill->itemID() );
            return false;
        }
        // use single_skill ...
        single_skill->SetAttribute(AttrSkillPoints, (int8)0);
        single_skill->SetAttribute(AttrSkillLevel, (int8)0);
        single_skill->ChangeSingleton(true);
        single_skill->Move(m_itemID, flagSkill, true);
    } else {  // use original skill
        skill->SetAttribute(AttrSkillPoints, (int8)0);
        skill->SetAttribute(AttrSkillLevel, (int8)0);
        skill->ChangeSingleton(true);
        skill->Move(m_itemID, flagSkill, true);
    }
    // 'skillEventSkillInjected' shows as "Unknown" in PD>Skill>History
    SaveSkillHistory(skillEventSkillInjected, GetFileTimeNow(), m_itemID, skill->typeID(), 0, 0);
    _log(CHARACTER__SKILL_TRACE, "%s(%u) Skill Injected: %u", itemName().c_str(), m_itemID, skill->itemID());

    m_pClient->SendNotifyMsg( "Injection of skill complete." );
    return true;
}

void Character::AddToSkillQueue(uint32 typeID, uint8 level) {
    QueuedSkill qs;
		qs.typeID = typeID;
		qs.level = level;
    m_skillQueue.push_back( qs );
    _log(CHARACTER__SKILL_TRACE, "%s(%u) added Skill %u Level %u to queue", itemName().c_str(), m_itemID, typeID, level);
}

void Character::SendSkillComplete(Skill* pSkill, uint8 oldLevel, uint8 newLevel, EvilNumber EN_Points, int64 newPoints, bool stopped) {
    /** @todo this is no longer needed, as attribmgr is handling attrib updates.  -allan 28Apr16 */
}

void Character::ClearSkillQueue() {
    _log(CHARACTER__SKILL_TRACE, "%s(%u) Skill Queue Cleared", itemName().c_str(), m_itemID);
    m_skillQueue.clear();
}

void Character::PauseSkillQueue() {
    _log(CHARACTER__SKILL_TRACE, "%s(%u) Skill Queue Paused", itemName().c_str(), m_itemID);
    m_db.SavePausedSkillQueue(m_itemID, m_skillQueue);
}

void Character::LoadPausedSkillQueue() {
    _log(CHARACTER__SKILL_TRACE, "%s(%u) Paused Skill Queue Loaded", itemName().c_str(), m_itemID);
    m_db.LoadPausedSkillQueue(m_itemID, m_skillQueue);
}

void Character::UpdateSkillQueue() {
    /* cleaned up code and reworked logic  -allan 28Apr16   -- revisited 23Mar17  --updated code, logic and timers 16Nov17  -again 9jan18*/
    if (m_pClient == nullptr)
        return;
    EvilNumber nextLevelSP = 0;
    Skill* currentTraining = GetSkillInTraining().get();
    if (currentTraining != nullptr) {
        currentTraining->VerifySP();
        if (m_skillQueue.empty() or (currentTraining->typeID() != m_skillQueue.front().typeID)) {
            uint8 oldLevel = currentTraining->GetAttribute(AttrSkillLevel).get_int();
            if (oldLevel < 0)
                oldLevel = 0;
            //EvilNumber oldPoints = currentTraining->GetAttribute(AttrSkillPoints);
            nextLevelSP = currentTraining->GetSPForLevel(oldLevel + 1);
            EvilNumber skillPointsTrained = (nextLevelSP - (((currentTraining->GetAttribute(AttrExpiryTime) - GetFileTimeNow()) / Win32Time_Minute) * GetSPPerMin(currentTraining)));

            SaveSkillHistory(skillEventTrainingCancelled, GetFileTimeNow(), m_itemID, currentTraining->typeID(), oldLevel +1, skillPointsTrained.get_double());
            _log(CHARACTER__SKILL_TRACE, "%s(%u) SkillTraining cancelled - skill: %u, level: %u, completionTime: %.0f, timeNow: %.0f", \
            itemName().c_str(), m_itemID, currentTraining->typeID(), oldLevel, currentTraining->GetAttribute(AttrExpiryTime).get_float(), GetFileTimeNow());

            OnSkillTrainingStopped osst;
                osst.itemID = currentTraining->itemID();
                osst.silent = true;    // slient means 'disable neocom blink event'
            PyTuple* tmp = osst.Encode();
            m_pClient->QueueDestinyEvent(&tmp);

            currentTraining->SetAttribute(AttrSkillPoints, skillPointsTrained.get_int());
            currentTraining->SetAttribute(AttrExpiryTime, 0);
            currentTraining->SetFlag(flagSkill, true);
            currentTraining->SaveItem();
            currentTraining = nullptr;
        }
    }

    uint32 skillID = 0;
    EvilNumber CurrentSP = 0, trainingEndTime = 0;
    while (!m_skillQueue.empty()) {
        if (currentTraining == nullptr) {
            skillID = m_skillQueue.front().typeID;
            currentTraining = GetSkill( skillID ).get();
            if (currentTraining == nullptr) {
                _log( CHARACTER__WARNING, "%s(%u): Skill %u to train was not found.", itemName().c_str(), m_itemID, skillID );
                m_skillQueue.erase( m_skillQueue.begin() );
                continue;
            }

            currentTraining->VerifySP();
            EvilNumber level = (currentTraining->GetAttribute(AttrSkillLevel) + 1);
            if (level > 5)
                level = 5;
            nextLevelSP = currentTraining->GetSPForLevel(level);
            CurrentSP = currentTraining->GetAttribute(AttrSkillPoints);
            if (CurrentSP >= nextLevelSP) {
                SaveSkillHistory(skillEventTrainingComplete, currentTraining->GetAttribute(AttrExpiryTime).get_double(), m_itemID, currentTraining->typeID(), (uint8)level.get_int(), \
                            nextLevelSP.get_double());
                _log(CHARACTER__SKILL_TRACE, "%s(%u) SkillTraining completed - skill: %u, level: %u, completionTime: %.0f, timeNow: %.0f", \
                                itemName().c_str(), m_itemID, currentTraining->typeID(), level, currentTraining->GetAttribute(AttrExpiryTime).get_float(), GetFileTimeNow());
                if (level == 5)
                    currentTraining->DeleteAttribute(AttrExpiryTime);
                else
                    currentTraining->SetAttribute(AttrExpiryTime, 0);
                currentTraining->SetFlag(flagSkill, true);
                currentTraining->SaveItem();
                currentTraining = nullptr;
                m_skillQueue.erase( m_skillQueue.begin() );
                continue;
            }
            trainingEndTime = EvEMath::Skill::EndTime(CurrentSP, nextLevelSP, GetSPPerMin(currentTraining), GetFileTimeNow());

            SaveSkillHistory(skillEventTrainingStarted, GetFileTimeNow(), m_itemID, skillID, (uint8)level.get_int(), CurrentSP.get_double());
            _log(CHARACTER__SKILL_TRACE, "%s(%u) SkillTraining started - skill: %u, level: %u, completionTime: %.0f, timeNow: %.0f", \
                            itemName().c_str(), m_itemID, skillID, level.get_int(), trainingEndTime.get_float(), GetFileTimeNow());

            // need to set end time for persistance
            currentTraining->SetAttribute(AttrExpiryTime, trainingEndTime);

            if (trainingEndTime > GetFileTimeNow()) {
                currentTraining->SetFlag(flagSkillInTraining, true);

                OnSkillStartTraining osst;
                    osst.itemID = currentTraining->itemID();
                    osst.endOfTraining = trainingEndTime.get_double();
                PyTuple* tmp = osst.Encode();
                m_pClient->QueueDestinyEvent(&tmp);
                currentTraining->SaveItem();
            }
        }

        if ( trainingEndTime < GetFileTimeNow() ) {
            // training has been finished
            uint8 level = currentTraining->GetAttribute(AttrSkillLevel).get_int() +1;
            if (level > 5)
                level = 5;
            EvilNumber newPoints = currentTraining->GetSPForLevel( (EvilNumber)level );
            EvilNumber completeTime = currentTraining->GetAttribute(AttrExpiryTime);

            OnSkillTrained ost;
                ost.itemID = currentTraining->itemID();
            PyTuple* tmp = ost.Encode();
            m_pClient->QueueDestinyEvent(&tmp);

            if (m_pClient->IsInSpace() and (!m_pClient->IsLogin())) {
                switch (currentTraining->groupID()) {
                    case EVEDB::invGroups::Trade:
                    case EVEDB::invGroups::Social:
                    case EVEDB::invGroups::Planet_Management:
                    case EVEDB::invGroups::Corporation_Management: {
                        ;   // do nothing for these.
                    } break;
                    default: {
                        m_pClient->SendInfoModalMsg("Your ship will update to your new skill level when you dock.");
                    } break;
                }
            }
            currentTraining->SetAttribute(AttrSkillLevel, (int8)level );
            currentTraining->SetAttribute(AttrSkillPoints, newPoints.get_int());
            SaveSkillHistory(skillEventQueueTrainingCompleted, completeTime.get_double(), m_itemID, currentTraining->typeID(), level, newPoints.get_float());
            _log(CHARACTER__SKILL_TRACE, "%s(%u) Queued SkillTraining completed - skill: %u, level: %u, completionTime: %.0f, timeNow: %.0f", \
                        itemName().c_str(), m_itemID, currentTraining->typeID(), level, completeTime.get_float(), GetFileTimeNow());
            if (level == 5)
                currentTraining->DeleteAttribute(AttrExpiryTime);
            else
                currentTraining->SetAttribute(AttrExpiryTime, 0);
            currentTraining->SetFlag(flagSkill, true);
            currentTraining->SaveItem();
            currentTraining = nullptr;
            m_skillQueue.erase( m_skillQueue.begin() );

            //  start training the next skill in queue when previous skill finished.....hackish persistance  -allan 7Apr14
            //  first, check for skills in queue...
            if (m_skillQueue.empty())
                break;

            skillID = m_skillQueue.front().typeID;
            currentTraining = GetSkill(skillID).get();
            if (currentTraining == nullptr) {
                _log( CHARACTER__WARNING, "%s(%u): Skill %u to train was not found.", itemName().c_str(), m_itemID, skillID );
                m_skillQueue.erase( m_skillQueue.begin() );
                continue;
            }

            currentTraining->VerifySP();
            level = (currentTraining->GetAttribute(AttrSkillLevel).get_int() + 1);
            if (level > 5)
                level = 5;
            nextLevelSP = currentTraining->GetSPForLevel(level);
            CurrentSP = currentTraining->GetAttribute(AttrSkillPoints);
            if (CurrentSP >= nextLevelSP) {
                SaveSkillHistory(skillEventTrainingComplete, completeTime.get_double(), m_itemID, currentTraining->typeID(), level, \
                            nextLevelSP.get_double());
                _log(CHARACTER__SKILL_TRACE, "%s(%u) Persistant Training completed - skill: %u, level: %u, completionTime: %.0f, timeNow: %.0f", \
                            itemName().c_str(), m_itemID, currentTraining->typeID(), level, completeTime.get_float(), GetFileTimeNow());
                if (level == 5)
                    currentTraining->DeleteAttribute(AttrExpiryTime);
                else
                    currentTraining->SetAttribute(AttrExpiryTime, 0);
                currentTraining->SetFlag(flagSkill, true);
                currentTraining->SaveItem();
                currentTraining = nullptr;
                m_skillQueue.erase( m_skillQueue.begin() );
                continue;
            }
            trainingEndTime = EvEMath::Skill::EndTime(CurrentSP, nextLevelSP, GetSPPerMin(currentTraining), completeTime.get_int());

            SaveSkillHistory(skillEventTrainingStarted, trainingEndTime.get_int(), m_itemID, skillID, level, CurrentSP.get_double());
            _log(CHARACTER__SKILL_TRACE, "%s(%u) Persistant Training started - skill: %u, level: %u, completionTime: %.0f, timeNow: %.0f", \
                        itemName().c_str(), m_itemID, skillID, level, trainingEndTime.get_float(), GetFileTimeNow());

            // need to set end time for persistance
            currentTraining->SetAttribute(AttrExpiryTime, trainingEndTime);

            if (trainingEndTime > GetFileTimeNow()) {
                currentTraining->SetFlag(flagSkillInTraining, true);

                OnSkillStartTraining osst;
                    osst.itemID = currentTraining->itemID();
                    osst.endOfTraining = trainingEndTime.get_double();
                PyTuple *tmp2 = osst.Encode();
                m_pClient->QueueDestinyEvent(&tmp2);
                currentTraining->SaveItem();
            }
        } else
            break;  // this skill is still in training.  break out of while() loop
    }

    if (!m_skillQueue.empty())
        SaveSkillQueue();
    else
        ClearSkillQueue();

    GetTotalSP();
    UpdateSkillQueueEndTime(m_skillQueue);
    m_pClient->UpdateSkillTraining();
    GetSkillQueue();
}

void Character::UpdateSkillQueueEndTime(const SkillQueue &queue) {
    std::map<uint16, uint8> flatSkillQueue;
    std::map<uint16, uint8>::iterator itr = flatSkillQueue.end();
    for (auto cur : queue) {
        QueuedSkill qs = cur;
        itr = flatSkillQueue.find(qs.typeID);
        if (itr != flatSkillQueue.end()) {
            if (itr->second < qs.level)
                itr->second = qs.level;
        } else
            flatSkillQueue.emplace(qs.typeID, qs.level);
    }

    Skill* skill(nullptr);
    EvilNumber chrMinRemaining = 0;
    for (auto cur : flatSkillQueue) {
        skill = GetSkill(cur.first).get();
        if (skill == nullptr)
            continue;
        chrMinRemaining += (skill->GetSPForLevel(cur.second) - skill->GetAttribute( AttrSkillPoints )) / GetSPPerMin(skill);
    }
    chrMinRemaining = (chrMinRemaining * Win32Time_Minute) + GetFileTimeNow();

    m_db.UpdateSkillQueueEndTime(chrMinRemaining.get_int(), m_itemID);
}

PyDict *Character::GetCharInfo() {
    // this is char, skills, implants, boosters.
    if (!pInventory->ContentsLoaded()) {
        if (!pInventory->LoadContents()) {
            codelog(CHARACTER__ERROR, "%s (%u): Failed to load contents for GetCharInfo", m_itemName.c_str(), m_itemID);
            return nullptr;
        }
    }

    Rsp_CommonGetInfo_Entry entry1;
    if (!Populate(entry1))
        return nullptr;

    PyDict *result = new PyDict();
    result->SetItem(new PyInt(m_itemID), new PyObject("util.KeyVal", entry1.Encode()));

    //now encode skills...
    std::vector<InventoryItemRef> skills;
    skills.clear();
    //find all the skills contained within ourself.
    pInventory->FindByFlag( flagSkill, skills );
    pInventory->FindByFlag( flagSkillInTraining, skills );

    /** @todo  get implants and boosters here once implemented */

    //encode an entry for each one.
    std::vector<InventoryItemRef>::iterator itr = skills.begin();
    for (; itr != skills.end(); ++itr) {
        Rsp_CommonGetInfo_Entry entry;
        if ((*itr)->Populate(entry))
            result->SetItem(new PyInt((*itr)->itemID()), new PyObject("util.KeyVal", entry.Encode()));
        else
            codelog(CHARACTER__ERROR, "%s (%u): Failed to load character item %u for GetCharInfo", m_itemName.c_str(), m_itemID, (*itr)->itemID());
    }

    /** @todo i dont know how boosters and implants work yet, so may have to set item different for them.  */

    return result;
}

PyObject *Character::GetDescription() const {
    util_Row row;
        row.header.push_back("description");
        row.line = new PyList();
        row.line->AddItemString( description().c_str() );
    return row.Encode();
}

PyTuple *Character::GetSkillQueue() {
    PyList *list = new PyList();
    for (SkillQueue::iterator itr = m_skillQueue.begin(); itr != m_skillQueue.end(); ++itr) {
        SkillQueue_Element el;
            el.typeID = itr->typeID;
            el.level = itr->level;
        list->AddItem( el.Encode() );
    }

    // now encapsulate it in a tuple with the free points
    PyTuple *tuple = new PyTuple(2);
        tuple->SetItem(0, list);
        tuple->SetItem(1, new PyInt(m_freePoints));
    return tuple;
}

void Character::AddItem(InventoryItemRef item) {
    pInventory->AddItem( item );

    if ((item->flag() == flagSkill) || (item->flag() == flagSkillInTraining)) {
        SkillRef skill = SkillRef::StaticCast( item );

        if( !skill->singleton() ) {
            skill->ChangeSingleton( true, true );
            skill->SetAttribute(AttrSkillLevel, (int8)0);
            skill->SetAttribute(AttrSkillPoints, (int8)0);
            if( skill->flag() != flagSkillInTraining )
                skill->SetAttribute(AttrExpiryTime, 0);
        }
    }

    _log( CHARACTER__INFO, "%s(%u) has been added with flag %d.", itemName().c_str(), m_itemID, (int)item->flag() );
}

void Character::SetActiveShip(uint32 shipID)
{
    m_charData.shipID = shipID;
    m_db.SetCurrentShip(m_itemID, shipID);
}

void Character::SetActivePod(uint32 podID)
{
    m_charData.capsuleID = podID;
    m_db.SetCurrentPod(m_itemID, podID);
}

void Character::ResetClone()
{
    m_db.ChangeCloneType(m_itemID, 164);       // typeID = 164 is for Clone Grade Alpha
}

void Character::SaveCharacter() {
    _log( CHARACTER__INFO, "Saving character info for %u.", m_itemID );

    SetLogonMinutes();
    m_db.SaveCharacter(m_itemID, m_charData);
}

void Character::SaveFullCharacter() {
    _log( CHARACTER__INFO, "Saving full character info for %u.", m_itemID );
    GetTotalSP();
	SaveCharacter();
    m_db.SaveCorpData(m_itemID, m_corpData);
    SaveAttributes();
    if (GetSkillInTraining())
        GetSkillInTraining()->SaveItem();
    SaveSkillQueue();
}

void Character::SaveSkillQueue() {
    _log( CHARACTER__SKILL_TRACE, "Saving skill queue of character %u.", m_itemID );
    m_db.SaveSkillQueue( m_itemID, m_skillQueue );
}

EvilNumber Character::GetTotalSP() {
    // Loop through all skills trained and calculate total SP this character has trained so far
    EvilNumber totalSP = 0;
    std::vector<InventoryItemRef> skills;
    GetSkillsList( skills );
    for (auto cur : skills)
        totalSP += cur->GetAttribute( AttrSkillPoints );    // much cleaner and more accurate    -allan

    return (m_charData.skillPoints = totalSP.get_double());
}

void Character::SaveSkillHistory(uint16 eventID, double logDate, uint32 characterID, uint32 skillTypeID, uint8 skillLevel, double absolutePoints)
{
    if (absolutePoints < 0)
        return;
    if (skillLevel < 0)
        return;
    if (logDate < 0)
        return;
    if (!sDataMgr.IsSkillTypeID(skillTypeID))
        return;
    m_db.SaveSkillHistory(eventID, (double)logDate, characterID, skillTypeID, skillLevel, absolutePoints);
}

PyRep* Character::GetSkillHistory()
{
    return m_db.GetSkillHistory(m_itemID);
}

void Character::PayBounty(CharacterRef cRef)
{
    std::string reason = "Bounty paid for the killing of ";
    reason += cRef->itemName();
    AccountService::TranserFunds(ownerCONCORD, m_itemID, cRef->bounty(), reason, Journal::EntryType::Bounty);
}

void Character::SetLoginTime()
{
    m_loginTime = sEntityList.GetStamp();
    m_db.SetLogInTime(m_itemID);
}

uint16 Character::OnlineTime()
{
    double onlineTime = m_charData.loginTime - GetFileTimeNow();
    onlineTime /= 10000000;
    onlineTime -= 11644473600;
    onlineTime /= 60;
    return (uint16)onlineTime;
}

// called on 10m timer from client
void Character::SetLogonMinutes() {
    //  get login time and set _logonMinutes       -allan
    uint16 loginMinutes = (sEntityList.GetStamp() - m_loginTime) /60;

    // some checks are done < 1m, so if this check has no minutes, keep original time and exit
    if (loginMinutes > 0) {
        m_charData.logonMinutes += loginMinutes;
        m_loginTime = sEntityList.GetStamp();
    }
}

// certificate system
bool Character::HasCertificate( uint32 certificateID ) const {
    for (auto cur : m_certificates)
        if (cur.certificateID == certificateID)
            return true;

    return false;
}

void Character::GetCertificates( CertVector &crt ) {
    crt = m_certificates;
}

void Character::GrantCertificate( uint32 certificateID )
{
    CharCerts cert;
        cert.certificateID = certificateID;
        cert.grantDate = GetFileTimeNow();
        cert.visibilityFlags = 0;
    m_certificates.push_back(cert);
    m_cdb.AddCertificate(m_itemID, cert);
}

void Character::UpdateCertificate( uint32 certificateID, bool pub ) {
    m_cdb.UpdateCertificate(m_itemID, certificateID, pub);
}

void Character::SaveCertificates() {
    _log( CHARACTER__INFO, "Saving Certificates of character %u", m_itemID );
    m_cdb.SaveCertificates( m_itemID, m_certificates );
}

// functions and methods for bookmark system (char mem maps)
/** @todo this will need more thought/work */
void Character::LoadBookmarks()
{

}

void Character::SaveBookMarks()
{

}


// functions and methods for standings system
double Character::GetStanding(uint32 toID, uint32 fromID) {
	/*
	 *    double res = s_db.GetCorpStanding(toID, fromID);
	 *
	 *    if (res < 0)
	 *        res += ((10+res) * 0.04 * GetSkillLevel(skillDiplomacy));
	 *    else
	 *        res += ((10-res) * 0.04 * GetSkillLevel(skillConnections));
	 */
	return s_db.GetStanding(toID, fromID);
}

double Character::GetNPCCorpStanding(uint32 toID, uint32 fromID) {
	/*
	 *    double res = s_db.GetNPCCorpStanding(toID, fromID);
	 *
	 *    if (res < 0)
	 *        res += ((10+res) * 0.04 * GetSkillLevel(skillDiplomacy));
	 *    else
	 *        res += ((10-res) * 0.04 * GetSkillLevel(skillConnections));
	 */
	return s_db.GetStanding(toID, fromID);
}

double Character::GetStandingChanges() {
	return s_db.GetStandingChanges(m_itemID);
}

void Character::SetStanding(uint32 fromID, uint32 toID, double standing) {
	s_db.SetStanding(fromID, toID, standing);
}

void Character::SaveStandingChanges(uint32 fromID, uint32 toID, uint32 eventType, double amount, std::string msg) {
	s_db.SaveStandingChanges(fromID, toID, eventType, amount, msg);
}

// functions and methods for map system
void Character::VisitSystem(uint32 solarSystemID) {
	m_db.VisitSystem(solarSystemID, m_itemID);
}

void Character::chkDynamicSystemID(uint32 solarSystemID) {
	/**  this ensures mapDynamicData.solarSystemID for `solarSystemID` is in the DB for later calls. -allan 16Mar14 */

	// seen some weird shit lately...not sure wtf is going on.  check to ensure solarSystemID REALLY IS a solarSystem...
	if (IsSolarSystem(solarSystemID))
		m_db.chkDynamicSystemID(solarSystemID);
	else
		sLog.Error("Character::chkDynamicSystemID","%s(%u): IsSolarSystem returned false for system %u",
				   itemName().c_str(), m_itemID, solarSystemID);
}

/** the following functions rely on solarSystemID being in the mapDynamicData table.
  * the check is called before these are used, and solarSystemID is then verified for existance and added if needed.
  *   the function is as follows and is defined above...
  *         void SystemDB::chkDynamicSystemID(uint32 solarSystemID)
  *
  *  NOTE: these will have to be reset each server start.
  *        really should trunicate table on restart after everything is working.
  */

void Character::AddJumpToDynamicData(uint32 solarSystemID) {  /**jumpsHour, jumps24Hours */
	m_db.AddJumpToDynamicData(solarSystemID);
}

void Character::AddPilotToDynamicData(uint32 solarSystemID, bool isAdd, bool isDocked, bool isLogin) {  /**pilotsDocked, pilotsInSpace */
	m_db.AddPilotToDynamicData(solarSystemID, isAdd, isDocked, isLogin);
}

void Character::AddKillToDynamicData(uint32 solarSystemID) {  /**killsHour, kills24Hours */
	m_db.AddKillToDynamicData(solarSystemID);
}

void Character::AddPodKillToDynamicData(uint32 solarSystemID) {   /**podKillsHour, podKills24Hour */
	m_db.AddPodKillToDynamicData(solarSystemID);
}

void Character::AddFactionKillToDynamicData(uint32 solarSystemID) {     /**factionKills*/
	m_db.AddFactionKillToDynamicData(solarSystemID);
}
