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
#include "EntityList.h"
#include "character/Character.h"
#include "effects/EffectsProcessor.h"
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

CharacterType *CharacterType::Load(ItemFactory &factory, uint32 characterTypeID)
{
    return ItemType::Load<CharacterType>( factory, characterTypeID );
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
 * CharacterData
 */
CharacterData::CharacterData(   //uses v6
    uint32 _accountID,
    const char *_title,
    const char *_desc,
    bool _gender,
    double _bounty,
    double _balance,
    double _aurBalance,
    double _securityRating,
    uint32 _logonMinutes,
    double _skillPoints,
    uint32 _corporationID,
    uint32 _allianceID,
    uint32 _warFactionID,
    uint32 _stationID,
    uint32 _solarSystemID,
    uint32 _constellationID,
    uint32 _regionID,
    uint32 _ancestryID,
    uint8 _bloodlineID,
    uint8 _raceID,
    uint32 _careerID,
    uint32 _schoolID,
    uint32 _careerSpecialityID,
    uint64 _startDateTime,
    uint64 _createDateTime,
    uint32 _shipID,
    uint32 _capsuleID)
: accountID(_accountID),
  title(_title),
  description(_desc),
  gender(_gender),
  bounty(_bounty),
  balance(_balance),
  aurBalance(_aurBalance),
  securityRating(_securityRating),
  logonMinutes(_logonMinutes),
  skillPoints(_skillPoints),
  corporationID(_corporationID),
  allianceID(_allianceID),
  warFactionID(_warFactionID),
  stationID(_stationID),
  solarSystemID(_solarSystemID),
  constellationID(_constellationID),
  regionID(_regionID),
  ancestryID(_ancestryID),
  bloodlineID(_bloodlineID),
  raceID(_raceID),
  careerID(_careerID),
  schoolID(_schoolID),
  careerSpecialityID(_careerSpecialityID),
  startDateTime(_startDateTime),
  createDateTime(_createDateTime),
  shipID(_shipID),
  capsuleID(_capsuleID)
{
}

/*
 * * CorpMemberInfo
 */
CorpData::CorpData(
    uint32 _corpHQ,
    int32 _corpAccountKey,
    uint64 _corpRole,
    uint64 _rolesAtAll,
    uint64 _rolesAtBase,
    uint64 _rolesAtHQ,
    uint64 _rolesAtOther)
: corpHQ(_corpHQ),
  corpAccountKey(_corpAccountKey),
  corpRole(_corpRole),
  rolesAtAll(_rolesAtAll),
  rolesAtBase(_rolesAtBase),
  rolesAtHQ(_rolesAtHQ),
  rolesAtOther(_rolesAtOther)
{
}

/*
 * Character
 */
Character::Character(
    ItemFactory &_factory,
    uint32 _characterID,
    // InventoryItem stuff:
    const CharacterType &_charType,
    const ItemData &_data,
    // Character stuff:
    const CharacterData &_charData,
    const CorpData &_corpData)
: InventoryItem(_factory, _characterID, _charType, _data),
  m_accountID(_charData.accountID),
  m_title(_charData.title),
  m_description(_charData.description),
  m_gender(_charData.gender),
  m_bounty(_charData.bounty),
  m_balance(_charData.balance),
  m_aurBalance(_charData.aurBalance),
  m_securityStatus(_charData.securityRating),
  m_logonMinutes(_charData.logonMinutes),
  m_totalSPtrained(((double)(_charData.skillPoints))),
  m_corporationID(_charData.corporationID),
  m_corpHQ(_corpData.corpHQ),
  m_allianceID(_charData.allianceID),
  m_warFactionID(_charData.warFactionID),
  m_corpAccountKey(_corpData.corpAccountKey),
  m_corpRole(_corpData.corpRole),
  m_rolesAtAll(_corpData.rolesAtAll),
  m_rolesAtBase(_corpData.rolesAtBase),
  m_rolesAtHQ(_corpData.rolesAtHQ),
  m_rolesAtOther(_corpData.rolesAtOther),
  m_stationID(_charData.stationID),
  m_solarSystemID(_charData.solarSystemID),
  m_constellationID(_charData.constellationID),
  m_regionID(_charData.regionID),
  m_ancestryID(_charData.ancestryID),
  m_bloodlineID(_charData.bloodlineID),
  m_raceID(_charData.raceID),
  m_careerID(_charData.careerID),
  m_schoolID(_charData.schoolID),
  m_careerSpecialityID(_charData.careerSpecialityID),
  m_startDateTime(_charData.startDateTime),
  m_createDateTime(_charData.createDateTime),
  m_shipID(_charData.shipID),
  m_capsuleID(_charData.capsuleID),
  m_pClient(nullptr)
{
    m_loaded = false;
    m_freePoints = 0;
    // allow characters to be only singletons
    assert(singleton());

    if (!IsAgent(m_itemID)) {
        m_loginTime = sEntityList.GetStamp();
        m_inventory = new Inventory(InventoryItemRef(this));
    }
}

Character::~Character()
{
    SaveFullCharacter();
    SaveCertificates();
    SafeDelete(m_inventory);
}

CharacterRef Character::Load(ItemFactory &factory, uint32 characterID) {
    return InventoryItem::Load<Character>( factory, characterID );
}

bool Character::_Load() {
    if (m_loaded) return true;
    if (IsAgent(m_itemID)) return true;

    if (!m_inventory->LoadContents(&m_factory)) {
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
        m_certificates.clear();
        if (!m_db.LoadCertificates(m_itemID, m_certificates)) {
            sLog.Warning("Character::_Load","LoadCertificates returned false for char %u", m_itemID);
            return (m_loaded = false);
        }
    }

    // get all skills, implants, and boosters, then process each, and store processed effect in m_modifiers.
    // m_modifiers will be looped and applied when ship is boarded.
    // no reason to delete m_modifiers as they are generic, and nothing changes in char when appling to ship.
    //   ....yes, they do.  skills apply to other skills, ships, modules, etc.
    //  it all gets reset when docking, and undock applies everything, so char will need reset also.
    ProcessEffects();

    return m_loaded;
}

template<class _Ty>
RefPtr<_Ty> Character::CreateCharacter(ItemFactory &factory, uint32 characterID,
                                       const CharacterType &charType, const ItemData &data,
                                       const CharacterData &charData, const CorpData &corpData)
{
    // construct the character item
    return CharacterRef( new Character( factory, characterID, charType, data, charData, corpData ) );
}

CharacterRef Character::Spawn(ItemFactory &factory, ItemData &data, CharacterData &charData, CorpData &corpData) {
    // make sure it's a character
    const CharacterType *ct = factory.GetCharacterType(data.typeID);
    if (!ct) return CharacterRef();

    // make sure it's a singleton with qty 1
    if (!data.singleton || data.quantity != 1) {
        _log(CHARACTER__ERROR, "Tried to create non-singleton character %s.", data.name.c_str());
        return CharacterRef();
    }

    uint32 characterID = InventoryItem::CreateItemID(factory, data);
    if (characterID == 0) {
        _log(CHARACTER__ERROR, "Failed to get ItemID for new character.");
        return CharacterRef();
    }

    // then character
    if (!factory.db().NewCharacter(characterID, charData, corpData)) {
        // delete the item
        factory.db().DeleteItem(characterID);
        return CharacterRef();
    }

    CharacterRef charRef = Character::Load( factory, characterID );
    charRef->SetAttribute(AttrMass, 0);

    return charRef;
}

void Character::Delete() {
    // delete contents
    m_inventory->DeleteContents();
    // delete character record
    m_factory.db().DeleteCharacter(m_itemID);
    // let the parent care about the rest
    InventoryItem::Delete();
}

bool Character::AlterBalance(double balanceChange) {
    if(balanceChange == 0)
        return true;
    double result = m_balance + balanceChange;
    //remember, this can take a negative amount...
    if (result < 0)
        return false;
    m_balance = result;

    /** @todo (allan) save some info to journal. */
    //SaveCharacter();

    return true;
}

void Character::SetLocation(uint32 stationID, uint32 solarSystemID, uint32 constellationID, uint32 regionID) {
    m_stationID = stationID;
    m_solarSystemID = solarSystemID;
    m_constellationID = constellationID;
    m_regionID = regionID;
    SaveCharacter();
}

void Character::JoinCorporation(uint32 corporationID, const CorpData &roles) {
	m_corporationID = corporationID;
	m_corpRole = roles.corpRole;
    m_corpAccountKey = roles.corpAccountKey;
    m_rolesAtAll = roles.rolesAtAll;
    m_rolesAtBase = roles.rolesAtBase;
    m_rolesAtHQ = roles.rolesAtHQ;
	m_rolesAtOther = roles.rolesAtOther;
    // Add new employment history record    -allan  25Mar14   update 20Jan15
    m_db.UpdateCharCorpRecords(m_itemID, corporationID);
    m_pClient->UpdateCorpSession(this);
    SaveCharacter();
}

void Character::SetDescription(const char *newDescription) {
    m_description = newDescription;
    SaveCharacter();
}

void Character::SetAccountKey(int32 accountKey)
{
    m_corpAccountKey = accountKey;
    m_pClient->UpdateCorpSession(this);

    SaveCharacter();
}

uint32 Character::PickAlternateShip(uint32 locationID)
{
    return m_db.PickAlternateShip(m_itemID, locationID);
}

void Character::SetFleetData(FleetData &fleet)
{
    m_fleetID = fleet.fleetID;
    m_wingID = fleet.wingID;
    m_squadID = fleet.squadID;
    m_fleetRole = fleet.fleetRole;
    m_fleetBooster = fleet.fleetBooster;
    m_fleetJob = fleet.fleetJob;
    m_pClient->UpdateFleetSession(this);
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

bool Character::GrantCertificate( uint32 certificateID )
{
    cCertificates cert;
        cert.certificateID = certificateID;
        cert.grantDate = Win32TimeNow();
        cert.visibilityFlags = true;
    m_certificates.push_back(cert);

    m_db.AddCertificate(m_itemID, cert);

    return true;
}

void Character::UpdateCertificate( uint32 certificateID, bool pub ) {
    m_db.UpdateCertificate(m_itemID, certificateID, pub);
}

void Character::GetCertificates( Certificates &crt ) {
    crt = m_certificates;
}

bool Character::HasCertificate( uint32 certificateID ) const {
    for (uint32 i = 0; i < m_certificates.size(); i++) {
        if (m_certificates.at( i ).certificateID == certificateID)
            return true;
    }
    return false;
}

SkillRef Character::GetSkill(uint32 skillTypeID) const
{
    InventoryItemRef skill = m_inventory->GetByTypeFlag( skillTypeID, flagSkill );
    if (!skill)
        skill = m_inventory->GetByTypeFlag( skillTypeID, flagSkillInTraining );

    return SkillRef::StaticCast( skill );
}

int8 Character::GetSkillLevel(uint32 skillTypeID, bool zeroForNotInjected /*true*/) const {
    SkillRef requiredSkill = GetSkill( skillTypeID );
    // First, check for existence of skill trained or in training:
    if (!requiredSkill) return (zeroForNotInjected ? 0 : -1);
    return (int8)requiredSkill->GetAttribute(AttrSkillLevel).get_int() ;
}

float Character::GetAgilitySkills(bool cap) {
    /* Spaceship Command: 2% agility for all ships per level
     * Evasive Maneuvering: 5% agility bonus for all ships per level
     * Advanced Spaceship Command: 5% agility bonus per level on ships requiring this skill
     * Capital Ships: 5% agility bonus per level on ships requiring this skill
     * **  these 2 will have to be set in fleet, once that system is operational  **
     * Skirmish Warfare: 2% agility to fleet per skill level
     * Skirmish warfare Mindlink (implant): 15% agility to fleet, replaces Skirmish warfare skill
     */
    float modifier = 1.0;
    modifier *= (1 - (0.02 * GetSkillLevel(skillSpaceshipCommand, true)));  //2%
    modifier *= (1 - (0.05 * GetSkillLevel(skillEvasiveManeuvering, true)));  //5%
    if (cap) {
        modifier *= (1 - (0.05 * GetSkillLevel(skillAdvancedSpaceshipCommand, true)));  //5%
        modifier *= (1 - (0.05 * GetSkillLevel(skillCapitalShips, true)));    //5%
    }
    return modifier;
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

void Character::ProcessEffects()
{
    //  427 total skills.  this should be fairly fast...it is.
    /** @note  ALL module, missile, and drone code will have to be rewritten because of this new effects code */
    std::vector<InventoryItemRef> allSkills;
    GetSkillsList(allSkills);

    _log(EFFECTS__TRACE, "Character::ParseExpression():  Beginning Character Effects Processing.");
    Effect curEffect;
    std::vector<TypeEffects> typeFx;
    for (auto curSkill : allSkills) {
        typeFx.clear();
        sFxDataMgr.GetTypeEffect(curSkill->typeID(), typeFx);
        for (auto curFx : typeFx) {
            curEffect = sFxDataMgr.GetEffect(curFx.effectID);
            fxData data;
            data.result = false;
            data.srcRef = curSkill;
            data.math = data.targLoc = data.fxSrc = data.targAttr = data.srcAttr = data.grpID = data.typeID = 0;
            sFxProc.ParseExpression(this, sFxDataMgr.GetExpression(curEffect.preExpression), data);
        }
    }
}

SkillRef Character::GetSkillInTraining() const {
    InventoryItemRef item;
    m_inventory->FindSingleByFlag(flagSkillInTraining, item);
    return SkillRef::StaticCast( item );
}

void Character::GetSkillsList(std::vector<InventoryItemRef> &skills) const
{
    m_inventory->FindByFlag( flagSkillInTraining, skills );
    m_inventory->FindByFlag( flagSkill, skills );
}

EvilNumber Character::GetSPPerMin(SkillRef skill)
{
	return EvEMath::SkillPointsPerMinute(GetAttribute(skill->GetAttribute(AttrPrimaryAttribute).get_int()), GetAttribute(skill->GetAttribute(AttrSecondaryAttribute).get_int()));
}

int64 Character::GetEndOfTraining() const {
    InventoryItemRef item;
    if (m_inventory->FindSingleByFlag(flagSkillInTraining, item))
        return item->GetAttribute(AttrExpiryTime).get_int();
    return 0;
}

bool Character::InjectSkillIntoBrain ( SkillRef skill, uint8 level )
{
    return false;
}

bool Character::InjectSkillIntoBrain(SkillRef skill) {
    if (!m_pClient) return false;

    SkillRef oldSkill = GetSkill( skill->typeID() );
    if ( oldSkill ) {
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
        if( !single_skill ) {
            _log( ITEM__ERROR, "%s (%u): Unable to split stack of %s (%u).", itemName().c_str(), m_itemID, skill->itemName().c_str(), skill->itemID() );
            return false;
        }
        // use single_skill ...
        single_skill->SetAttribute(AttrSkillPoints, 0, false);
        single_skill->SetAttribute(AttrSkillLevel, 0, false);
        single_skill->ChangeSingleton(true);
        single_skill->Move(m_itemID, flagSkill);
    } else {  // use original skill
        skill->SetAttribute(AttrSkillPoints, 0, false);
        skill->SetAttribute(AttrSkillLevel, 0, false);
        skill->ChangeSingleton(true);
        skill->Move(m_itemID, flagSkill);
    }
    // 'skillEventSkillInjected' shows as "Unknown" in PD>Skill>History
    SaveSkillHistory(skillEventSkillInjected, Win32TimeNow(), m_itemID, skill->typeID(), 0, 0, GetTotalSP().get_double() );
    _log(CHARACTER__SKILL_TRACE, "%s(%u) Skill Injected: %u", itemName().c_str(), m_itemID, skill->itemID());

    m_pClient->SendNotifyMsg( "Injection of skill complete." );
    return true;
}

void Character::AddToSkillQueue(uint32 typeID, uint8 level) {
    QueuedSkill qs;
		qs.typeID = typeID;
		qs.level = level;
    m_skillQueue.push_back( qs );
    _log(CHARACTER__SKILL_TRACE, "%s(%u) Skill %u training to level %u added to queue", itemName().c_str(), m_itemID, typeID, level);
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
    /* cleaned up code and reworked logic  -allan 28Apr16   -- revisited 23Mar17*/
    if (!m_pClient) return;
    SkillRef currentTraining = GetSkillInTraining();
    if (currentTraining) {
        if (m_skillQueue.empty() or (currentTraining->typeID() != m_skillQueue.front().typeID)) {
            uint8 oldLevel = currentTraining->GetAttribute(AttrSkillLevel).get_int();
            EvilNumber oldPoints = currentTraining->GetAttribute(AttrSkillPoints);
            EvilNumber nextLevelSP = currentTraining->GetSPForLevel(oldLevel + 1);
            EvilNumber skillPointsTrained = (nextLevelSP - (((currentTraining->GetAttribute(AttrExpiryTime) - EvilTimeNow()) / EvilTime_Minute) * GetSPPerMin(currentTraining)));

            OnSkillTrainingStopped osst;
                osst.itemID = currentTraining->itemID();
                osst.silent = 0;    //look into this...why would it be silent?
                PyTuple* tmp = osst.Encode();
            m_pClient->QueueDestinyEvent(&tmp); // consumed

            SaveSkillHistory(skillEventTrainingCancelled, EvilTimeNow(), m_itemID, currentTraining->typeID(), oldLevel, skillPointsTrained.get_double(), GetTotalSP().get_double() );
            _log(CHARACTER__SKILL_TRACE, "%s(%u) SkillTraining cancelled - skill: %u, level: %u, completionTime: %.1f", \
                        itemName().c_str(), m_itemID, currentTraining->typeID(), oldLevel, currentTraining->GetAttribute(AttrExpiryTime).get_float());

            currentTraining->SetAttribute(AttrSkillPoints, skillPointsTrained);
            currentTraining->SetAttribute(AttrExpiryTime, 0, false);
            currentTraining->SetFlag(flagSkill);
            currentTraining = SkillRef();
        }
    }

    while (!m_skillQueue.empty()) {                        // skills in queue to be trained
        if (!currentTraining) {                            //nothing being trained
            uint32 skillID = m_skillQueue.front().typeID;   //....get first skill in list
            currentTraining = GetSkill( skillID );
            if (!currentTraining) {
                _log( CHARACTER__WARNING, "%s (%u): Skill %u to train was not found.", itemName().c_str(), m_itemID, skillID );
                m_skillQueue.erase( m_skillQueue.begin() );
                break;
            }

            EvilNumber level = (currentTraining->GetAttribute(AttrSkillLevel) + 1);
            if (level > 5)
                level = 5;
            EvilNumber NextLevelSP = currentTraining->GetSPForLevel(level);
            EvilNumber CurrentSP = currentTraining->GetAttribute(AttrSkillPoints);
            if (CurrentSP >= NextLevelSP) {
                currentTraining->SetAttribute(AttrExpiryTime, 0);
                currentTraining->SetFlag(flagSkill);
                currentTraining = SkillRef();
                m_skillQueue.erase( m_skillQueue.begin() );
                break;
            }
            EvilNumber trainingEndTime = EvEMath::SkillEndingTime(CurrentSP, NextLevelSP, GetSPPerMin(currentTraining), EvilTimeNow());

            SaveSkillHistory(skillEventTrainingStarted, EvilTimeNow(), m_itemID, skillID, (uint8)level.get_int(), CurrentSP.get_double(), GetTotalSP().get_double() );
            _log(CHARACTER__SKILL_TRACE, "%s(%u) SkillTraining started - skill: %u, level: %u, completionTime: %.1f", \
                            itemName().c_str(), m_itemID, skillID, level.get_int(), trainingEndTime.get_float());

            currentTraining->SetAttribute(AttrExpiryTime, trainingEndTime);
            currentTraining->SetFlag(flagSkillInTraining);

            OnSkillStartTraining osst;
				osst.itemID = currentTraining->itemID();
                osst.endOfTraining = trainingEndTime.get_double();
            PyTuple* tmp = osst.Encode();
            m_pClient->QueueDestinyEvent(&tmp); // consumed
            break;
        }

        if ( currentTraining->GetAttribute(AttrExpiryTime) < EvilTimeNow() ) {
            // training has been finished
            uint8 level = currentTraining->GetAttribute(AttrSkillLevel).get_int() +1;
            if (level > 5)
                level = 5;
            EvilNumber newPoints = currentTraining->GetSPForLevel( (EvilNumber)level );
            EvilNumber completeTime = currentTraining->GetAttribute(AttrExpiryTime);

            SaveSkillHistory(skillEventTrainingComplete, completeTime, m_itemID, currentTraining->typeID(), level, currentTraining->GetAttribute(AttrSkillPoints).get_double(), GetTotalSP().get_double() );
            _log(CHARACTER__SKILL_TRACE, "%s(%u) SkillTraining completed - skill: %u, level: %u, completionTime: %.1f", \
                        itemName().c_str(), m_itemID, currentTraining->typeID(), level, completeTime.get_float());

            OnSkillTrained ost;
                ost.itemID = currentTraining->itemID();
            PyTuple* tmp = ost.Encode();
            m_pClient->QueueDestinyEvent(&tmp); // consumed

            currentTraining->SetAttribute(AttrSkillLevel, level );
            currentTraining->SetAttribute(AttrSkillPoints, newPoints);
            currentTraining->SetAttribute(AttrExpiryTime, 0, false);
            currentTraining->SetFlag(flagSkill);
            m_skillQueue.erase( m_skillQueue.begin() );

            //  start training the next skill in queue when previous skill finished.....hackish persistance  -allan 7Apr14
            //  first, check for skills in queue...
            if (m_skillQueue.empty())
                break;

            uint32 skillID = m_skillQueue.front().typeID;
            currentTraining = GetSkill( skillID );
            if (!currentTraining)
                break;

            level = (currentTraining->GetAttribute(AttrSkillLevel).get_int() + 1);
            if (level > 5) level = 5;
            EvilNumber NextLevelSP = currentTraining->GetSPForLevel(level);
            EvilNumber CurrentSP = currentTraining->GetAttribute(AttrSkillPoints);
            if (CurrentSP >= NextLevelSP) {
                currentTraining->SetAttribute(AttrExpiryTime, 0);
                currentTraining->SetFlag(flagSkill);
                currentTraining = SkillRef();
                m_skillQueue.erase( m_skillQueue.begin() );
                break;
            }
            EvilNumber trainingEndTime = EvEMath::SkillEndingTime(CurrentSP, NextLevelSP, GetSPPerMin(currentTraining), EvilTimeNow());

            SaveSkillHistory(skillEventTrainingStarted, trainingEndTime.get_int(), m_itemID, skillID, level, CurrentSP.get_double(), GetTotalSP().get_double() );
            _log(CHARACTER__SKILL_TRACE, "%s(%u) Persistant Training started - skill: %u, level: %u, completionTime: %.1f", \
                        itemName().c_str(), m_itemID, skillID, level, trainingEndTime.get_float());

            currentTraining->SetAttribute(AttrExpiryTime, trainingEndTime);
            currentTraining->SetFlag(flagSkillInTraining);

            OnSkillStartTraining osst;
                osst.itemID = currentTraining->itemID();
                osst.endOfTraining = trainingEndTime.get_double();
            PyTuple *tmp2 = osst.Encode();
            m_pClient->QueueDestinyEvent(&tmp2); // consumed
        } else
            break;
    }

    if ( !m_skillQueue.empty() && currentTraining ) {
        GetTotalSP();                           // Re-Calculate total SP trained and store in internal variable:
        SaveSkillQueue();                       // Save character skill data
        UpdateSkillQueueEndTime(m_skillQueue);  // and Queue end time:
    } else
        ClearSkillQueue();

    m_pClient->UpdateSkillTraining();           // update skill queue end time
    GetSkillQueue();                            //update skill queue on client
}

/** @todo update this to use highest level of skills when multiple levels in queue */
void Character::UpdateSkillQueueEndTime(const SkillQueue &queue) {
    /**   this code is start for looping skillqueue for multiple levels of same skill.
    std::multimap<uint16, uint8> flatSkillQueue;
    std::multimap<uint16, uint8>::iterator itr;
    for (auto cur : queue) {
        const QueuedSkill qs = cur;
        itr = flatSkillQueue.find(qs.typeID);
        if (itr != flatSkillQueue.end()) {
            if (cur->second < qs.level)
                cur->second = qs.level;
        } else
            flatSkillQueue.insert(std::make_pair(qs.typeID, qs.level));
    }   */

    EvilNumber chrMinRemaining = 0;
    for (uint8 i = 0; i < queue.size(); i++) {    // loop thru skills currently in queue
        const QueuedSkill& qs = queue[ i ];     // get skill id from queue
        SkillRef skill = Character::GetSkill( qs.typeID );   //make ref for current skill
        if (!skill) continue;
        chrMinRemaining += (skill->GetSPForLevel(qs.level) - skill->GetAttribute( AttrSkillPoints )) / GetSPPerMin(skill);
    }
    chrMinRemaining = (chrMinRemaining * EvilTime_Minute) + EvilTimeNow();

    m_db.UpdateSkillQueueEndTime(chrMinRemaining.get_int(), m_itemID);
}

PyDict *Character::GetCharInfo() {
    // this is char, skills, implants, boosters.
    if (!m_inventory->ContentsLoaded()) {
        if (!m_inventory->LoadContents(&m_factory)) {
            codelog(CHARACTER__ERROR, "%s (%u): Failed to load contents for GetCharInfo", m_itemName.c_str(), m_itemID);
            return nullptr;
        }
    }

    PyDict *result = new PyDict;
    Rsp_CommonGetInfo_Entry entry1;

    if (!Populate(entry1))
        return nullptr;
    result->SetItem(new PyInt(m_itemID), new PyObject("util.KeyVal", entry1.Encode()));

    //now encode skills...
    std::vector<InventoryItemRef> skills;
    skills.clear();
    //find all the skills contained within ourself.
    m_inventory->FindByFlag( flagSkill, skills );
    m_inventory->FindByFlag( flagSkillInTraining, skills );

    /** @todo  get implants and boosters here once implemented */

    //encode an entry for each one.
    Rsp_CommonGetInfo_Entry entry;
    std::vector<InventoryItemRef>::iterator cur = skills.begin();
    for (; cur != skills.end(); cur++) {
        if(!(*cur)->Populate(entry)) {
            codelog(CHARACTER__ERROR, "%s (%u): Failed to load character item %u for GetCharInfo", m_itemName.c_str(), m_itemID, (*cur)->itemID());
        } else {
            result->SetItem(new PyInt((*cur)->itemID()), new PyObject("util.KeyVal", entry.Encode()));
        }
    }

    /** @todo i dont know how boosters and implants work yet, so may have to set item different for them.  */

    return result;
}

PyObject *Character::GetDescription() const {
    util_Row row;
        row.header.push_back("description");
        row.line = new PyList;
        row.line->AddItemString( description().c_str() );
    return row.Encode();
}

PyTuple *Character::GetSkillQueue() {
    PyList *list = new PyList;

    SkillQueue::iterator cur = m_skillQueue.begin();
    for(; cur != m_skillQueue.end(); cur++) {
        SkillQueue_Element el;
            el.typeID = cur->typeID;
            el.level = cur->level;
        list->AddItem( el.Encode() );
    }

    // now encapsulate it in a tuple with the free points
    PyTuple *tuple = new PyTuple(2);
        tuple->SetItem(0, list);
        tuple->SetItem(1, new PyInt(m_freePoints));
    return tuple;
}

void Character::AddItem(InventoryItemRef item) {
    m_inventory->AddItem( item );

    if ((item->flag() == flagSkill) || (item->flag() == flagSkillInTraining)) {
        SkillRef skill = SkillRef::StaticCast( item );

        if( !skill->singleton() ) {
            skill->ChangeSingleton( true );
            skill->SetAttribute(AttrSkillLevel, 0);
            skill->SetAttribute(AttrSkillPoints, 0);
            if( skill->flag() != flagSkillInTraining )
                skill->SetAttribute(AttrExpiryTime, 0);
        }
    }

    _log( CHARACTER__INFO, "%s(%u) has been added with flag %d.", itemName().c_str(), m_itemID, (int)item->flag() );
}

void Character::SetActiveShip(uint32 shipID)
{
    m_shipID = shipID;
    m_db.SetCurrentShip(m_itemID, shipID);
}

void Character::SetActivePod(uint32 podID)
{
    m_capsuleID = podID;
    m_db.SetCurrentPod(m_itemID, podID);

}
void Character::ResetClone()
{
    m_db.ChangeCloneType(m_itemID, 164);       // typeID = 164 is for Clone Grade Alpha
}

void Character::SaveCharacter() {
    _log( CHARACTER__INFO, "Saving character info for %u.", m_itemID );

    // Set current m_logonMinutes
    SetLogonMinutes();

    // character data
    /** @todo rework this to save only updated data */
    m_factory.db().SaveCharacter(
        m_itemID,
        CharacterData(
            m_accountID,
            m_title.c_str(),
            m_description.c_str(),
            m_gender,
            m_bounty,
            m_balance,
            m_aurBalance,
            m_securityStatus,
            m_logonMinutes,
            GetTotalSP().get_double(),
            m_corporationID,
            m_allianceID,
            m_warFactionID,
            m_stationID,
            m_solarSystemID,
            m_constellationID,
            m_regionID,
            m_ancestryID,
            m_bloodlineID,
            m_raceID,
            m_careerID,
            m_schoolID,
            m_careerSpecialityID,
            m_startDateTime,
            m_createDateTime,
            m_shipID,
            m_capsuleID
        )
    );

    // corporation data
    /** @todo remove this in favor of updating data when changed */
    m_factory.db().SaveCorpData(
        m_itemID,
        CorpData(
            m_corpHQ,
            m_corpAccountKey,
            m_corpRole,
            m_rolesAtAll,
            m_rolesAtBase,
            m_rolesAtHQ,
            m_rolesAtOther
        )
    );
}

void Character::SaveFullCharacter() {
    _log( CHARACTER__INFO, "Saving full character info for %u.", m_itemID );
	// First save basic character info:
	SaveCharacter();
    // Save this character's attributes:
    SaveAttributes();
    // Save currently training skill:
    if (GetSkillInTraining())
        GetSkillInTraining()->SaveItem();
    // Save skill queue:
    SaveSkillQueue();
}

void Character::SaveSkillQueue() {
    _log( CHARACTER__SKILL_TRACE, "Saving skill queue of character %u.", m_itemID );

    // skill queue
    m_db.SaveSkillQueue( m_itemID, m_skillQueue );
}

void Character::SaveCertificates() {
    _log( CHARACTER__INFO, "Saving Certificates of character %u", m_itemID );
    m_db.SaveCertificates( m_itemID, m_certificates );
}

EvilNumber Character::GetTotalSP() {
    // Loop through all skills trained and calculate total SP this character has trained so far
    EvilNumber totalSP = 0.0f;
    std::vector<InventoryItemRef> skills;
    GetSkillsList( skills );
    for (auto cur : skills) {
        totalSP += cur->GetAttribute( AttrSkillPoints );    // much cleaner and more accurate    -allan
    }

    return (m_totalSPtrained = totalSP);
}

void Character::SaveSkillHistory(uint8 eventID, EvilNumber logDate, uint32 characterID, uint32 skillTypeID, uint8 skillLevel, double relativePoints, double absolutePoints) {
    m_db.SaveSkillHistory(eventID, logDate.get_double(), characterID, skillTypeID, skillLevel, relativePoints, absolutePoints);
}

PyRep* Character::GetSkillHistory() {
    return m_db.GetSkillHistory(m_itemID);
}

void Character::PayBounty(CharacterRef cRef) {
    AlterBalance(m_db.PayBounty(cRef));
}

void Character::SetLoginTime() {
    m_loginTime = sEntityList.GetStamp();
}

// called on 10m timer from client
void Character::SetLogonMinutes() {
    //  get login time and set _logonMinutes       -allan
    uint16 loginMinutes = (sEntityList.GetStamp() - m_loginTime) /60;

    // some checks are done < 1m, so if this check has no minutes, keep original time and exit
    if (loginMinutes > 0) {
        m_logonMinutes += loginMinutes;
        m_loginTime = sEntityList.GetStamp();
    }
}

bool Character::isOffline(uint32 charID) {
    //return m_db.isOffline(charID);
    if (m_pClient)
		return false;
	else
		return true;
}

// functions and methods for standings system
/** @todo (allan)  this system is not complete... */
double Character::GetAgentStanding(uint32 toID, uint32 fromID) {
	return s_db.GetAgentStanding(toID, fromID);
}

double Character::GetAllianceStanding(uint32 toID, uint32 fromID) {
	return s_db.GetAllianceStanding(toID, fromID);
}

double Character::GetCharStanding(uint32 toID, uint32 fromID) {
	return s_db.GetCharStanding(toID, fromID);
}

double Character::GetCorpStanding(uint32 toID, uint32 fromID) {
	/*
	 *    double res = s_db.GetCorpStanding(toID, fromID);
	 *
	 *    if (res < 0)
	 *        res += ((10+res) * 0.04 * GetSkillLevel(skillDiplomacy));
	 *    else
	 *        res += ((10-res) * 0.04 * GetSkillLevel(skillConnections));
	 */
	return s_db.GetCorpStanding(toID, fromID);
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
	return s_db.GetNPCCorpStanding(toID, fromID);
}

double Character::GetFactionStanding(uint32 toID, uint32 fromID) {
	return s_db.GetFactionStanding(toID, fromID);
}

double Character::GetStandingChanges() {
	return s_db.GetStandingChanges(m_itemID);
}

void Character::SetAgentStanding(uint32 fromID, uint32 toID, double standing) {
	s_db.SetAgentStanding(fromID, toID, standing);
}

void Character::SetAllianceStanding(uint32 fromID, uint32 toID, double standing) {
	s_db.SetAllianceStanding(fromID, toID, standing);
}

void Character::SetCharStanding(uint32 fromID, uint32 toID, double standing) {
	s_db.SetCharStanding(fromID, toID, standing);
}

void Character::SetCorpStanding(uint32 fromID, uint32 toID, double standing) {
	s_db.SetCorpStanding(fromID, toID, standing);
}

void Character::SetNPCCorpStanding(uint32 fromID, uint32 toID, double standing) {
	s_db.SetNPCCorpStanding(fromID, toID, standing);
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
