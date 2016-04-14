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
#include "inventory/AttributeEnum.h"
#include "utils/EVE_Equations.h"

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

template<class _Ty>
_Ty *CharacterType::_LoadCharacterType(ItemFactory &factory, uint32 typeID, uint8 bloodlineID,
    // ItemType stuff:
    const ItemGroup &group, const TypeData &data,
    // CharacterType stuff:
    const ItemType &shipType, const CharacterTypeData &charData)
{
    // enough data for construction
    return new CharacterType( typeID, bloodlineID, group, data, shipType, charData );
}

/*
 * CharacterData
 */
CharacterData::CharacterData(   //uses v5
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
 * CharacterAppearance
 */

void CharacterAppearance::Build(uint32 ownerID, PyDict* data)
{
	PyList* colors = new PyList();
	PyList* modifiers = new PyList();
	PyObjectEx* appearance;
	PyList* sculpts = new PyList();

	colors = data->GetItemString("colors")->AsList();
	modifiers = data->GetItemString("modifiers")->AsList();
	appearance = data->GetItemString("appearance")->AsObjectEx();
	sculpts = data->GetItemString("sculpts")->AsList();

	PyList::const_iterator color_cur, color_end;
	color_cur = colors->begin();
	color_end = colors->end();

	for(; color_cur != color_end; color_cur++)
	{
		if((*color_cur)->IsObjectEx())
		{
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

	PyObjectEx_Type2* app_obj = (PyObjectEx_Type2*)appearance;
	PyTuple* app_tuple = app_obj->GetArgs()->AsTuple();

	m_db.SetAvatar(ownerID, app_tuple->GetItem(1));

	PyList::const_iterator modif_cur, modif_end;
	modif_cur = modifiers->begin();
	modif_end = modifiers->end();

	for(; modif_cur != modif_end; modif_cur++)
	{
		if((*modif_cur)->IsObjectEx())
		{
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

	PyList::const_iterator sculpt_cur, sculpt_end;
	sculpt_cur = sculpts->begin();
	sculpt_end = sculpts->end();

	for(; sculpt_cur != sculpt_end; sculpt_cur++)
	{
		if((*sculpt_cur)->IsObjectEx())
		{
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
 * CorpMemberInfo
 */
CorpMemberInfo::CorpMemberInfo(
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
 * FleetMember Info
 */
FleetMemberInfo::FleetMemberInfo(
    uint32 _fleetID,
    uint32 _wingID,
    uint32 _squadID,
	uint8 _fleetRole,
	uint8 _fleetBooster,
    uint8 _fleetJob)
: fleetID(_fleetID),
  wingID(_wingID),
  squadID(_squadID),
  fleetRole(_fleetRole),
  fleetBooster(_fleetBooster),
  fleetJob(_fleetJob)
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
    const CorpMemberInfo &_corpData)
: Owner(_factory, _characterID, _charType, _data),
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
  m_careerID(_charData.careerID),
  m_schoolID(_charData.schoolID),
  m_careerSpecialityID(_charData.careerSpecialityID),
  m_startDateTime(_charData.startDateTime),
  m_createDateTime(_charData.createDateTime),
  m_shipID(_charData.shipID),
  m_capsuleID(_charData.capsuleID)
{
    // allow characters to be only singletons
    assert(singleton());

    // Activate Save Info Timer with somewhat randomized timer value:
    //SetSaveTimerExpiry( MakeRandomInt( (10 * 60), (15 * 60) ) );        // Randomize save timer expiry to between 10 and 15 minutes
    //EnableSaveTimer();
    m_loginTime = sEntityList.GetStamp();
}

CharacterRef Character::Load(ItemFactory &factory, uint32 characterID) {
    return InventoryItem::Load<Character>( factory, characterID );
}

template<class _Ty>
RefPtr<_Ty> Character::_LoadCharacter(ItemFactory &factory, uint32 characterID,
    // InventoryItem stuff:
    const CharacterType &charType, const ItemData &data,
    // Character stuff:
    const CharacterData &charData, const CorpMemberInfo &corpData)
{
    // construct the item
    return CharacterRef( new Character( factory, characterID, charType, data, charData, corpData ) );
}

CharacterRef Character::Spawn(ItemFactory &factory,
    // InventoryItem stuff:
    ItemData &data,
    // Character stuff:
    CharacterData &charData, CorpMemberInfo &corpData)
{
    uint32 characterID = Character::_Spawn( factory, data, charData, corpData );
    if ( characterID == 0 ) return CharacterRef();

    CharacterRef charRef = Character::Load( factory, characterID );

    charRef.get()->SetAttribute(AttrIsOnline, 1);     // Is Online

    return charRef;
}

uint32 Character::_Spawn(ItemFactory &factory,
    // InventoryItem stuff:
    ItemData &data,
    // Character stuff:
    CharacterData &charData, CorpMemberInfo &corpData)
{
    // make sure it's a character
    const CharacterType *ct = factory.GetCharacterType(data.typeID);
    if (!ct) return 0;

    // make sure it's a singleton with qty 1
    if (!data.singleton || data.quantity != 1) {
        _log(ITEM__ERROR, "Tried to create non-singleton character %s.", data.name.c_str());
        return 0;
    }

    // first the item
    uint32 characterID = Owner::_Spawn(factory, data);
    if (characterID == 0) return 0;

    // then character
    if (!factory.db().NewCharacter(characterID, charData, corpData)) {
        // delete the item
        factory.db().DeleteItem(characterID);
        return 0;
    }

    return characterID;
}

bool Character::_Load() {
    if( !Owner::_Load() ) {
        sLog.Warning("Character::_Load","Owner::_Load returned false for char %u", itemID());
        return false;
    }

	bool bLoadSuccessful = false;

    if( !LoadContents( &m_factory ) ) {
        sLog.Warning("Character::_Load","LoadContents returned false for char %u", itemID());
        return false;
    }

    if( !m_db.LoadSkillQueue( itemID(), m_skillQueue ) ) {
        sLog.Warning("Character::_Load","LoadSkillQueue returned false for char %u", itemID());
        return false;
    }

    bLoadSuccessful = Owner::_Load();

	// Update Skill Queue and Total Skill Points Trained:
	if ( bLoadSuccessful )
        UpdateSkillQueue();

    if( !m_factory.db().LoadCertificates( itemID(), m_certificates ) ) {
        sLog.Warning("Character::_Load","LoadCertificates returned false for char %u", itemID());
        return false;
    }

	return bLoadSuccessful;
}

void Character::Delete() {
    // delete contents
    DeleteContents( &m_factory );

    // delete character record
    m_factory.db().DeleteCharacter(itemID());

    // let the parent care about the rest
    Owner::Delete();
}

bool Character::AlterBalance(double balanceChange) {
    if(balanceChange == 0)
        return true;

    double result = m_balance + balanceChange;

    //remember, this can take a negative amount...
    if (result < 0)
        return false;

    m_balance = result;

    //TODO: save some info to journal.
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

void Character::JoinCorporation(uint32 corporationID, const CorpMemberInfo &roles) {
	m_corporationID = corporationID;

	m_corpRole = roles.corpRole;
    m_corpAccountKey = roles.corpAccountKey;
    m_rolesAtAll = roles.rolesAtAll;
    m_rolesAtBase = roles.rolesAtBase;
    m_rolesAtHQ = roles.rolesAtHQ;
	m_rolesAtOther = roles.rolesAtOther;

    // Add new employment history record    -allan  25Mar14   update 20Jan15
    m_db.UpdateCharCorpRecords(itemID(), corporationID);

	SaveCharacter();

    Client* pClient = sEntityList.FindClientByCharID( itemID() );
    pClient->UpdateCorpSession(pClient->GetChar());
}

void Character::SetDescription(const char *newDescription) {
    m_description = newDescription;
    SaveCharacter();
}

void Character::SetAccountKey(int32 accountKey)
{
    m_corpAccountKey = accountKey;
    Client* pClient = sEntityList.FindClientByCharID( itemID() );
    pClient->UpdateCorpSession(pClient->GetChar());

    SaveCharacter();
}

uint32 Character::PickAlternateShip(uint32 locationID)
{
    return m_db.PickAlternateShip(itemID(), locationID);
}

void Character::SetFleetData(FleetMemberInfo &fleet)
{
    m_fleetID = fleet.fleetID;
    m_wingID = fleet.wingID;
    m_squadID = fleet.squadID;
    m_fleetRole = fleet.fleetRole;
    m_fleetBooster = fleet.fleetBooster;
    m_fleetJob = fleet.fleetJob;

    Client* pClient = sEntityList.FindClientByCharID( itemID() );
    pClient->UpdateFleetSession(pClient->GetChar());
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

bool Character::HasCertificate( uint32 certificateID ) const {
    for (uint32 i = 0; i < m_certificates.size(); i++) {
        if (m_certificates.at( i ).certificateID == certificateID) return true;
    }
    return false;
}

SkillRef Character::GetSkill(uint32 skillTypeID) const
{
    InventoryItemRef skill = GetByTypeFlag( skillTypeID, flagSkill );
    if (!skill)
        skill = GetByTypeFlag( skillTypeID, flagSkillInTraining );

    return SkillRef::StaticCast( skill );
}

uint8 Character::GetSkillLevel(uint32 skillTypeID, bool zeroForNotInjected /*true*/) const {
    SkillRef requiredSkill = GetSkill( skillTypeID );
    // First, check for existence of skill trained or in training:
    if (!requiredSkill) return (zeroForNotInjected ? 0 : -1);
    return requiredSkill->GetAttribute(AttrSkillLevel).get_int() ;
}

float Character::GetAgilitySkills(bool cap) {
	/*    Evasive Maneuvering  5% improved ship agility for all ships per skill level.
	 *    Spaceship Command   2% improved ship agility for all ships per skill level.
     *    Capital Ships   5% bonus per skill level to the agility of ships requiring Capital Ships
	 *    Advanced Spaceship Command    5% Bonus per skill level to the agility of ships requiring Advanced Spaceship Command
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

SkillRef Character::GetSkillInTraining() const {
    InventoryItemRef item;
    FindSingleByFlag(flagSkillInTraining, item);
    return SkillRef::StaticCast( item );
}

void Character::GetSkillsList(std::vector<InventoryItemRef> &skills) const
{
    FindByFlag( flagSkillInTraining, skills );
    FindByFlag( flagSkill, skills );
}

EvilNumber Character::GetSPPerMin(SkillRef skill)
{
	return SkillPointsPerMinute(GetAttribute(skill->GetAttribute(AttrPrimaryAttribute).get_int()), GetAttribute(skill->GetAttribute(AttrSecondaryAttribute).get_int()));
}

int64 Character::GetEndOfTraining() const {
    if (GetSkillInTraining())
        return GetSkillInTraining()->GetAttribute(AttrExpiryTime).get_int();
    return 0;
}

bool Character::InjectSkillIntoBrain(SkillRef skill) {
    Client* pClient = sEntityList.FindClientByCharID( itemID() );
    if (!pClient) return false;

    SkillRef oldSkill = GetSkill( skill->typeID() );
    if ( oldSkill ) {
        //TODO: build and send proper UserError for CharacterAlreadyKnowsSkill.
        pClient->SendNotifyMsg( "You already know this skill." );
        return false;
    }

    // TODO: based on config options (maybe later), check to see if another character owned by this characters account,
    // is training a skill.  If so, return. (flagID=61).

    if ( !skill->SkillPrereqsComplete( *this ) ) {
        // TODO: need to send back a response to the client.  need packet specs.
        _log( ITEM__TRACE, "%s (%u): Requested to train skill %u item %u but prereq not complete.", itemName().c_str(), itemID(), skill->typeID(), skill->itemID() );
        pClient->SendNotifyMsg( "Injection failed!  Skill prerequisites incomplete." );
        return false;
    }

    // are we injecting from a stack of skills?
    if( skill->quantity() > 1 ) {
        // split the stack to obtain single item
        InventoryItemRef single_skill = skill->Split( 1 );
        if( !single_skill ) {
            _log( ITEM__ERROR, "%s (%u): Unable to split stack of %s (%u).", itemName().c_str(), itemID(), skill->itemName().c_str(), skill->itemID() );
            return false;
        }
        // use single_skill ...
        single_skill->MoveInto( *this, flagSkill );
    } else   // use original skill
        skill->MoveInto( *this, flagSkill );

    skill->SetAttribute(AttrSkillPoints, 0);
    skill->SetAttribute(AttrSkillLevel, 0);
    SaveSkillHistory(skillEventSkillInjected, (double)Win32TimeNow(), itemID(), skill.get()->itemID(), 0, skill->GetAttribute(AttrSkillPoints).get_float(), GetTotalSP().get_float() );

    pClient->SendNotifyMsg( "Injection of skill complete." );
    return true;
}

void Character::AddToSkillQueue(uint32 typeID, uint8 level) {
    QueuedSkill qs;
		qs.typeID = typeID;
		qs.level = level;
    m_skillQueue.push_back( qs );
}

void Character::SendSkillComplete(Client* pClient, Skill* pSkill, uint8 oldLevel, uint8 newLevel, EvilNumber EN_Points, int64 newPoints, bool stopped) {
    /* there is a function for this at EVEAttributeManager.cpp:324
     *   but i'm not advanced enough to follow it for correct use.
     *   so, i wrote this one and put it here.
	 * this is part of the skill training "Completion Immenient" fix.
	 *		 -allan  8Mar15
     */
    PyTuple* tmp = nullptr;

    if (oldLevel != newLevel) {
        PyRep* oldLvl = new PyInt(oldLevel);
        PyRep* newLvl = new PyInt(newLevel);
        Notify_OnModuleAttributeChange omac1;
			omac1.ownerID = static_cast<int32>(this->itemID());
			omac1.itemKey = static_cast<int32>(pSkill->itemID());
			omac1.attributeID = AttrSkillLevel;
			omac1.time = static_cast<int64>(Win32TimeNow());
			omac1.oldValue = oldLvl;
			omac1.newValue = newLvl;
        tmp = omac1.Encode();
        pClient->QueueDestinyEvent( &tmp );
    }

    int64 oldPoints = 0;
    if (EN_Points.isFloat())
        oldPoints = static_cast<int64>(EN_Points.get_float());
    else
        oldPoints = EN_Points.get_int();

    if (oldPoints != newPoints) {
        PyRep* oldPts = new PyInt(static_cast<int32>(oldPoints));
        PyRep* newPts = new PyInt(static_cast<int32>(newPoints));
        Notify_OnModuleAttributeChange omac2;
			omac2.ownerID = static_cast<int32>(this->itemID());
			omac2.itemKey = static_cast<int32>(pSkill->itemID());
			omac2.attributeID = AttrSkillPoints;
			omac2.time = static_cast<int64>(Win32TimeNow());
			omac2.oldValue = oldPts;
			omac2.newValue = newPts;
        tmp = omac2.Encode();
        pClient->QueueDestinyEvent( &tmp );
    }

    if (stopped) {
        OnSkillTrainingStopped osst;
			osst.itemID = static_cast<int32>(pSkill->itemID());
			osst.silent = 0;    //look into this...why would it be silent?
        tmp = osst.Encode();
        //pClient->SendNotification("OnSkillTrainingStopped", "clientid", &tmp);
        pClient->QueueDestinyEvent(&tmp);
    } else {
        OnSkillTrained ost;
            ost.itemID = static_cast<int32>(pSkill->itemID());
        tmp = ost.Encode();
        //pClient->SendNotification("OnSkillTrained", "clientid", &tmp);
        pClient->QueueDestinyEvent(&tmp);
    }

    pClient->UpdateSkillTraining();
    PySafeDecRef(tmp);
}

bool Character::GrantCertificate( uint32 certificateID )
{
    cCertificates i;
		i.certificateID = certificateID;
		i.grantDate = Win32TimeNow();
		i.visibilityFlags = true;
    m_certificates.push_back( i );

    return true;
}

void Character::UpdateCertificate( uint32 certificateID, bool pub ) {
    for( uint32 i = 0; i < m_certificates.size(); i++ ) {
        if( m_certificates.at( i ).certificateID == certificateID ) {
            m_certificates.at( i ).visibilityFlags = pub ;
        }
    }
}

void Character::GetCertificates( Certificates &crt ) {
    crt = m_certificates;
}

void Character::ClearSkillQueue() {
    m_skillQueue.clear();
}

void Character::PauseSkillQueue() {
    m_db.SavePausedSkillQueue(itemID(), m_skillQueue);
}

void Character::LoadPausedSkillQueue() {
    m_db.LoadPausedSkillQueue(itemID(), m_skillQueue);
}

void Character::UpdateSkillQueue() {
    Client* pClient = sEntityList.FindClientByCharID(itemID());
	if (!pClient) return;

    SkillRef currentTraining = GetSkillInTraining();
    if (currentTraining) {
        if (m_skillQueue.empty()        // either queue is empty
            || currentTraining->typeID() != m_skillQueue.front().typeID ) {     //or skill with different typeID is in training ...
                uint8 oldLevel = currentTraining->GetAttribute(AttrSkillLevel).get_int();
                EvilNumber oldPoints = currentTraining->GetAttribute(AttrSkillPoints);
                EvilNumber nextLevelSP = currentTraining->GetSPForLevel(currentTraining->GetAttribute(AttrSkillLevel) + 1);
                EvilNumber skillPointsTrained = (nextLevelSP - (((currentTraining->GetAttribute(AttrExpiryTime) - EvilTimeNow()) / EvilTime_Minute) * GetSPPerMin(currentTraining)));

                currentTraining->SetAttribute(AttrSkillPoints, skillPointsTrained);

                SaveSkillHistory(skillEventTrainingCancelled, EvilTimeNow().get_float(), itemID(), currentTraining->typeID(), oldLevel, skillPointsTrained.get_float(), GetTotalSP().get_float() );
				SendSkillComplete(pClient, currentTraining.get(), oldLevel, oldLevel, oldPoints, skillPointsTrained.get_int(), true );

                currentTraining->SaveItem();        // Save changes to this skill before removing it from training:
                currentTraining->SetAttribute(AttrExpiryTime, 0);
                currentTraining->MoveInto( *this, flagSkill, true );

                // nothing currently in training (to be reset in later checks)
                currentTraining = SkillRef();
            }
    }

    while (!m_skillQueue.empty()) {                        // skills in queue to be trained
        if (!currentTraining) {                            //nothing being trained
            uint32 skillID = m_skillQueue.front().typeID;   //....get first skill in list
            currentTraining = GetSkill( skillID );
            if ( !currentTraining ) {
                _log( ITEM__ERROR, "%s (%u): Skill %u to train was not found.", itemName().c_str(), itemID(), skillID );
                m_skillQueue.erase( m_skillQueue.begin() );
                break;
            }

            if (currentTraining->GetAttribute(AttrSkillLevel) > 4) {  //check for skillLevel above max.
                currentTraining->SetAttribute(AttrExpiryTime, 0);
                currentTraining->MoveInto( *this, flagSkill, true );
                currentTraining->SaveItem();
                m_skillQueue.erase( m_skillQueue.begin() );
                break;
            }

            EvilNumber level = (currentTraining->GetAttribute(AttrSkillLevel) + 1);
            EvilNumber SPToNextLevel = currentTraining->GetSPForLevel(level);
            EvilNumber CurrentSP = currentTraining->GetAttribute(AttrSkillPoints);
            SPToNextLevel -= CurrentSP;
            EvilNumber timeTraining = (EvilTimeNow() + (EvilTime_Minute * (SPToNextLevel / GetSPPerMin(currentTraining))));

            currentTraining->MoveInto( *this, flagSkillInTraining );
            currentTraining->SetAttribute(AttrExpiryTime, timeTraining.get_float());

            SaveSkillHistory(skillEventTrainingStarted, EvilTimeNow().get_float(), itemID(), skillID, level.get_int(), CurrentSP.get_float(), GetTotalSP().get_float() );
            sLog.Warning( "skillHistory", "training started, skill: %u, level: %d", skillID, level.get_int());

            currentTraining->SaveItem();

            OnSkillStartTraining osst;
				osst.itemID = currentTraining->itemID();
				osst.endOfTraining = timeTraining.get_float();
            PyTuple* tmp = osst.Encode();
			pClient->QueueDestinyEvent( &tmp );
            PySafeDecRef( tmp );
            break;
        }

        //  NOTE:  This needs a periodic (persistant) check, not just for chars ingame.  API will need CURRENT skilltraining
        //    will have to set up something like this in api to pull data from db, and adjust for current time, as this cannot
        //    be called for clients NOT ingame.  (or not easily...)
        if ( currentTraining->GetAttribute(AttrExpiryTime).get_int() < Win32TimeNow() ) {
            // training has been finished
            uint8 oldLevel = currentTraining->GetAttribute(AttrSkillLevel).get_int();
            EvilNumber oldPoints = currentTraining->GetAttribute(AttrSkillPoints);
            uint8 level = oldLevel + 1;
            if (level > 5) level = 5;
            EvilNumber newPoints = currentTraining->GetSPForLevel( (EvilNumber)level );
            currentTraining->SetAttribute(AttrSkillLevel, level );
            currentTraining->SetAttribute(AttrSkillPoints, newPoints, true);

            EvilNumber completeTime = currentTraining->GetAttribute(AttrExpiryTime);
            if ( completeTime < 1 ) completeTime = EvilTimeNow();

            uint32 skillID = m_skillQueue.front().typeID;
			SaveSkillHistory(skillEventQueueTrainingCompleted, completeTime.get_float(), itemID(), skillID, level, currentTraining->GetAttribute(AttrSkillPoints).get_float(), GetTotalSP().get_float() );

			SendSkillComplete(pClient, currentTraining.get(), oldLevel, level, oldPoints, newPoints.get_int());
            sLog.Success( "skillHistory", "training complete, skill: %u, level: %u", skillID, level );

            currentTraining->SetAttribute(AttrExpiryTime, 0);
            currentTraining->MoveInto( *this, flagSkill, true );
            currentTraining->SaveItem();
            currentTraining = SkillRef();

            // erase skill from queue now that the level is complete
            m_skillQueue.erase( m_skillQueue.begin() );

            //  start training the next skill in queue when previous skill finished.....hackish persistance  -allan 7Apr14
            //  first, check for skills in queue...
            if ( m_skillQueue.empty() ) {
                // nothing else in queue... training done, so exit function.
                break;
            }

            skillID = m_skillQueue.front().typeID;           //  uint32
            currentTraining = GetSkill( skillID );           //  skillRef

            if (!currentTraining) break;
            if (currentTraining->GetAttribute(AttrSkillLevel) > 4) {  //check for skillLevel above max.
                currentTraining->SetAttribute(AttrExpiryTime, 0);
                currentTraining->MoveInto( *this, flagSkill, true );
                currentTraining->SaveItem();
                currentTraining = SkillRef();
                break;
            }

            level = (currentTraining->GetAttribute(AttrSkillLevel).get_int() + 1);
            EvilNumber SPToNextLevel = currentTraining->GetSPForLevel((EvilNumber)level);
            EvilNumber CurrentSP = currentTraining->GetAttribute(AttrSkillPoints);
            SPToNextLevel -= CurrentSP;
            EvilNumber timeTraining = (completeTime + (EvilTime_Minute * (SPToNextLevel / GetSPPerMin(currentTraining))));

            currentTraining->MoveInto( *this, flagSkillInTraining );
            currentTraining->SetAttribute(AttrExpiryTime, timeTraining.get_float());

            SaveSkillHistory(skillEventTrainingStarted, timeTraining.get_float(), itemID(), skillID, level, CurrentSP.get_float(), GetTotalSP().get_float() );
            sLog.Warning( "skillHistory", "persistant training started, skill: %u, level: %d", skillID, level );

            OnSkillStartTraining osst;
				osst.itemID = currentTraining->itemID();
				osst.endOfTraining = timeTraining.get_float();
            PyTuple *tmp = osst.Encode();
			pClient->QueueDestinyEvent( &tmp );
            PySafeDecRef( tmp );
        } else break;
    }

    if ( !m_skillQueue.empty() && currentTraining ) {
        _CalculateTotalSPTrained();             // Re-Calculate total SP trained and store in internal variable:
        SaveSkillQueue();                       // Save character skill data
        UpdateSkillQueueEndTime(m_skillQueue);  // and Queue end time:
    } else
        ClearSkillQueue();

	pClient->UpdateSkillTraining();				// update skill queue end time
    GetSkillQueue();                        	//update skill queue on client
}

//  this still needs work...in progress...see commented code for using <map> flatSkillQueue
void Character::UpdateSkillQueueEndTime(const SkillQueue &queue) {
    /**   this code is start for looping skillqueue for multiple levels of same skill.
    std::unordered_multimap<uint32, uint8> flatSkillQueue;
    const QueuedSkill &qs = queue;
    //if (flatSkillQueue.find(qs.typeID) != flatSkillQueue.end()){
    // flatSkillQueue.insert(std::make_pair(qs.typeID,qs));}
    //  else{ flatSkillQueue.find(qs.typeID)->second.level = qs.level;}

    //flatSkillQueue.insert(std::make_pair(qs.typeID,qs));
    */

    EvilNumber chrMinRemaining = 0;    // explicit init to 0
    for (uint8 i = 0; i < queue.size(); i++) {    // loop thru skills currently in queue
        const QueuedSkill& qs = queue[ i ];     // get skill id from queue
        SkillRef skill = Character::GetSkill( qs.typeID );   //make ref for current skill
        if (!skill)
            continue;

        chrMinRemaining += (skill->GetSPForLevel(qs.level) - skill->GetAttribute( AttrSkillPoints )) / GetSPPerMin(skill);
    }
    chrMinRemaining = (chrMinRemaining * EvilTime_Minute) + EvilTimeNow();

    m_db.UpdateSkillQueueEndTime(chrMinRemaining.get_int(), itemID());
}

PyDict *Character::CharGetInfo() {
    // this is char, skills, implants, boosters.

    /*
          [PyInt 1661059544]        << char id
          [PyObjectData Name: util.KeyVal]
            [PyDict 5 kvp]
              [PyString "itemID"]
              [PyInt 1661059544]
              [PyString "attributes"]
              [PyDict 69 kvp]       << char attributes here.  k,v
                [PyInt 4]
                [PyFloat 1]
              [PyString "invItem"]
              [PyPackedRow 36 bytes]
                ["itemID" => <1661059544> [I8]]
                ["typeID" => <1376> [I4]]
                ["ownerID" => <1> [I4]]
                ["locationID" => <1002332770557> [I8]]
                ["flagID" => <57> [I2]]     <<flagpilot
                ["quantity" => <-1> [I4]]
                ["groupID" => <1> [I2]]
                ["categoryID" => <1> [I2]]
                ["customInfo" => <empty string> [Str]]
              [PyString "time"]
              [PyIntegerVar 129520542425380610]
              [PyString "activeEffects"]
              [PyDict 0 kvp]
            */
    if( !LoadContents( &m_factory ) ) {
        codelog(ITEM__ERROR, "%s (%u): Failed to load contents for CharGetInfo", m_itemName.c_str(), m_itemID);
        return NULL;
    }

    PyDict *result = new PyDict;
    Rsp_CommonGetInfo_Entry entry;

    if(!Populate(entry))
        return NULL;
    result->SetItem(new PyInt(m_itemID), new PyObject("util.KeyVal", entry.Encode()));

    //now encode skills...
    std::vector<InventoryItemRef> skills;
    //find all the skills contained within ourself.
    FindByFlag( flagSkill, skills );
    FindByFlag( flagSkillInTraining, skills );

    //encode an entry for each one.
    std::vector<InventoryItemRef>::iterator cur = skills.begin();
    for(; cur != skills.end(); cur++) {
        if(!(*cur)->Populate(entry)) {
            codelog(ITEM__ERROR, "%s (%u): Failed to load skill item %u for CharGetInfo", m_itemName.c_str(), itemID(), (*cur)->itemID());
        } else {
            result->SetItem(new PyInt((*cur)->itemID()), new PyObject("util.KeyVal", entry.Encode()));
        }
    }

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
    // return skills from skill queue
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
    // sending 0, as done on retail, doesn't fuck up calculation for some reason
    // so we can take the same shortcut here
    tuple->SetItem(1, new PyInt(0));    // this is free points

    return tuple;
}

void Character::AddItem(InventoryItemRef item) {
    Inventory::AddItem( item );

    if( item->flag() == flagSkill
        || item->flag() == flagSkillInTraining ) {
        // Skill has been added ...
        if( item->categoryID() != EVEDB::invCategories::Skill ) {
            _log( ITEM__WARNING, "%s (%u): %s has been added with flag %d.", itemName().c_str(), itemID(), item->category().name().c_str(), (int)item->flag() );
        } else {
            SkillRef skill = SkillRef::StaticCast( item );

            if( !skill->singleton() ) {
                _log( ITEM__TRACE, "%s (%u): Injecting %s.", itemName().c_str(), itemID(), item->itemName().c_str() );

                // Make it singleton and set initial skill values.
                skill->ChangeSingleton( true );

                skill->SetAttribute(AttrSkillLevel, 0);
                skill->SetAttribute(AttrSkillPoints, 0);

                if( skill->flag() != flagSkillInTraining )
                    skill->SetAttribute(AttrExpiryTime, 0);
            }
        }
    }
}

void Character::SetActiveShip(uint32 shipID)
{
    m_shipID = shipID;
    m_db.SetCurrentShip(itemID(), shipID);
}

void Character::SetActivePod(uint32 podID)
{
    m_capsuleID = podID;
    m_db.SetCurrentPod(itemID(), podID);

}
void Character::ResetClone()
{
    m_db.ChangeCloneType(itemID(), 164);       // typeID = 164 is for Clone Grade Alpha
}

void Character::SaveCharacter() {
    _log( ITEM__TRACE, "Saving character info for %u.", itemID() );

    // Set current m_logonMinutes
    _GetLogonMinutes();

    // character data
    m_factory.db().SaveCharacter(
        itemID(),
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
            GetTotalSP().get_float(),
            m_corporationID,
            m_allianceID,
            m_warFactionID,
            m_stationID,
            m_solarSystemID,
            m_constellationID,
            m_regionID,
            m_ancestryID,
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
    m_factory.db().SaveCorpMemberInfo(
        itemID(),
        CorpMemberInfo(
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
    _log( ITEM__TRACE, "Saving full character info for %u.", itemID() );
	// First save basic character info:
	SaveCharacter();
    // Save this character's attributes:
    SaveAttributes();
    // Save currently training skill:
    if (GetSkillInTraining())
        GetSkillInTraining()->SaveItem();
    // Save skill queue:
    SaveSkillQueue();
    // and certificates:
    SaveCertificates();

    // TODO
    // Loop through all items owned by this Character and save each one
	// Loop through all contracts or other non-item things owned by this Character and save each one

//  we DO NOT need to iterate thru all skills and save them...
}

void Character::SaveSkillQueue() {
    _log( ITEM__TRACE, "Saving skill queue of character %u.", itemID() );

    // skill queue
    m_db.SaveSkillQueue( itemID(), m_skillQueue );
}

void Character::SaveCertificates() const {
    _log( ITEM__TRACE, "Saving Certificates of character %u", itemID() );

    m_factory.db().SaveCertificates( itemID(), m_certificates );
}

void Character::_CalculateTotalSPTrained() {
    // Loop through all skills trained and calculate total SP this character has trained so far
    EvilNumber totalSP = 0.0f;
    std::vector<InventoryItemRef> skills;
    GetSkillsList( skills );
    std::vector<InventoryItemRef>::iterator cur = skills.begin();
    for(; cur != skills.end(); cur++) {
        totalSP += cur->get()->GetAttribute( AttrSkillPoints );    // much cleaner and more accurate    -allan
    }

    m_totalSPtrained = totalSP;
}

EvilNumber Character::GetTotalSP() {
    // Loop through all skills trained and calculate total SP this character has trained so far
    EvilNumber totalSP = 0.0f;
    std::vector<InventoryItemRef> skills;
    GetSkillsList( skills );
    std::vector<InventoryItemRef>::iterator cur = skills.begin();
    for(; cur != skills.end(); cur++) {
        totalSP += cur->get()->GetAttribute( AttrSkillPoints );
    }

    return totalSP;
}

void Character::SaveSkillHistory(uint8 eventID, double logDate, uint32 characterID, uint32 skillTypeID, uint8 skillLevel, double relativePoints, double absolutePoints) {
    m_db.SaveSkillHistory(eventID, logDate, characterID, skillTypeID, skillLevel, relativePoints, absolutePoints);
}

PyObject* Character::GetSkillHistory() {
    return(m_db.GetSkillHistory( itemID() ));
}

void Character::PayBounty(CharacterRef cRef) {
    AlterBalance(m_db.PayBounty(cRef));
}

void Character::SetLoginTime() {
    m_loginTime = sEntityList.GetStamp();
    sLog.Blue( "Character::SetLoginTime()", "Setting loginTime to %u", m_loginTime );
}

void Character::_GetLogonMinutes() {
    //  get login time and set _logonMinutes       -allan
    uint32 loginMinutes = (sEntityList.GetStamp() - m_loginTime) /60;

    // some checks are done < 1m, so if this check has no minutes, keep original time and exit
    if (loginMinutes > 0) {
        m_logonMinutes += loginMinutes;
		SetLoginTime();
		sLog.Blue( "Character::_GetLogonMinutes()", "loginMinutes is %u", loginMinutes );
		sLog.Blue( "Character::_GetLogonMinutes()", "Updating m_logonMinutes to %u", m_logonMinutes );
    }
}

bool Character::isOffline(uint32 charID) {
    //return m_db.isOffline(charID);
	Client* pClient = sEntityList.FindClientByCharID(itemID());
	if (pClient)
		return false;
	else
		return true;
}

// functions and methods for standings system
//  NOTE FIXME  this system is not complete...
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
	return s_db.GetStandingChanges(itemID());
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

void Character::SaveStandingChanges(uint32 fromID, uint32 toID, uint32 direction, uint32 eventID, uint32 eventType, double amount, std::string msg) {
	s_db.SaveStandingChanges(fromID, toID, direction, eventID, eventType, amount, msg);
}

// functions and methods for map system
void Character::VisitSystem(uint32 solarSystemID) {
	m_db.VisitSystem(solarSystemID, itemID());
}

void Character::chkDynamicSystemID(uint32 solarSystemID) {
	/**  this ensures mapDynamicData.solarSystemID for `solarSystemID` is in the DB for later calls. -allan 16Mar14 */

	// seen some werid shit lately...not sure wtf is going on.  check to ensure solarSystemID REALLY IS a solarSystem...
	if (IsSolarSystem(solarSystemID))
		m_db.chkDynamicSystemID(solarSystemID);
	else
		sLog.Error("Character::chkDynamicSystemID","%s(%u): IsSolarSystem returned false for system %u",
				   itemName().c_str(), itemID(), solarSystemID);
}

/** the following functions rely on solarSystemID being in the mapDynamicData table.
  * the check is called before these are used, and solarSystemID is then verified for existance and added if needed.
  *   the function is as follows and is declared above...
  *         void SystemDB::chkDynamicSystemID(uint32 solarSystemID)
  *
  *  NOTE: these will have to be reset each server start.
  *        really should trunicate table on restart after everything is working.
  */

void Character::AddJumpToDynamicData(uint32 solarSystemID) {  /**jumpsHour, jumps24Hours */
	m_db.AddJumpToDynamicData(solarSystemID);
}

void Character::AddPilotToDynamicData(uint32 solarSystemID, bool isDocked, bool isLogin) {  /**pilotsDocked, pilotsInSpace */
	m_db.AddPilotToDynamicData(solarSystemID, isDocked, isLogin);
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
