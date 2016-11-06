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
    Author:        Zhur, Bloody.Rabbit
    Updates:        Allan
*/

#ifndef __CHARACTER__H__INCL__
#define __CHARACTER__H__INCL__

#include "character/CharacterDB.h"
#include "character/Skill.h"
#include "inventory/ItemType.h"
#include "inventory/Inventory.h"
#include "inventory/InventoryDB.h"
#include "inventory/InventoryItem.h"
#include "standing/StandingDB.h"

/**
 * Simple container for raw character type data.
 */
class CharacterTypeData {
public:
    CharacterTypeData(
        const char* _bloodlineName = "",
        EVERace _race = (EVERace)0,
        const char* _desc = "",
        const char* _maleDesc = "",
        const char* _femaleDesc = "",
        uint32 _shipTypeID = 0,
        uint32 _corporationID = 0,
        uint8 _perception = 0,
        uint8 _willpower = 0,
        uint8 _charisma = 0,
        uint8 _memory = 0,
        uint8 _intelligence = 0,
        const char* _shortDesc = "",
        const char* _shortMaleDesc = "",
        const char* _shortFemaleDesc = ""
    );

    // Content:
    std::string bloodlineName;
    EVERace race;
    std::string description;
    std::string maleDescription;
    std::string femaleDescription;
    uint32 shipTypeID;
    uint32 corporationID;

    uint8 perception;
    uint8 willpower;
    uint8 charisma;
    uint8 memory;
    uint8 intelligence;

    std::string shortDescription;
    std::string shortMaleDescription;
    std::string shortFemaleDescription;
};

/**
 * Class which maintains character type data.
 */
class CharacterType
: public ItemType
{
    friend class ItemType; // to let it construct us
public:
    /**
     * Loads and returns new CharacterType.
     *
     * @param[in] factory
     * @param[in] characterTypeID ID of character type to load.
     * @return Pointer to new object, NULL if failed.
     */
    static CharacterType* Load(ItemFactory& factory, uint32 characterTypeID);

    /*
     * Access functions:
     */
    uint32 bloodlineID() const { return m_bloodlineID; }

    const std::string& bloodlineName() const { return m_bloodlineName; }
    const std::string& description() const { return m_description; }
    const std::string& maleDescription() const { return m_maleDescription; }
    const std::string& femaleDescription() const { return m_femaleDescription; }
    const ItemType& shipType() const { return m_shipType; }
    uint32 shipTypeID() const { return shipType().id(); }
    uint32 corporationID() const { return m_corporationID; }

    uint8 perception() const { return m_perception; }
    uint8 willpower() const { return m_willpower; }
    uint8 charisma() const { return m_charisma; }
    uint8 memory() const { return m_memory; }
    uint8 intelligence() const { return m_intelligence; }

    const std::string& shortDescription() const { return m_shortDescription; }
    const std::string& shortMaleDescription() const { return m_shortMaleDescription; }
    const std::string& shortFemaleDescription() const { return m_shortFemaleDescription; }

protected:
    CharacterType(
        uint32 _id,
        uint8 _bloodlineID,
        // ItemType stuff:
        const ItemGroup& _group,
        const TypeData& _data,
        // CharacterType stuff:
        const ItemType& _shipType,
        const CharacterTypeData& _charData
    );

    /**
     * Member functions
     */
    using ItemType::_Load;

    // Template loader:
    template<class _Ty>
    static _Ty* _LoadType(ItemFactory& factory, uint32 typeID,
        // ItemType stuff:
        const ItemGroup& group, const TypeData& data)
    {
        // check we are really loading a character type
        if( group.id() != EVEDB::invGroups::Character ) {
            sLog.Error("Character", "Load of character type %u requested, but it's %s.", typeID, group.name().c_str() );
            return NULL;
        }

        // query character type data
        uint32 bloodlineID;
        CharacterTypeData charData;
        if( !factory.db().GetCharacterType(typeID, bloodlineID, charData) )
            return NULL;

        // load ship type
        const ItemType* shipType = factory.GetType( charData.shipTypeID );
        if( shipType == NULL )
            return NULL;

        return _Ty::template _LoadCharacterType<_Ty>( factory, typeID, bloodlineID, group, data, *shipType, charData );
    }

    // Actual loading stuff:
    template<class _Ty>
    static _Ty* _LoadCharacterType(ItemFactory& factory, uint32 typeID, uint8 bloodlineID,
        // ItemType stuff:
        const ItemGroup& group, const TypeData& data,
        // CharacterType stuff:
        const ItemType& shipType, const CharacterTypeData& charData
    );

    /*
     * Data members
     */
    uint8 m_bloodlineID;

    std::string m_bloodlineName;
    std::string m_description;
    std::string m_maleDescription;
    std::string m_femaleDescription;
    const ItemType& m_shipType;
    uint32 m_corporationID;

    uint8 m_perception;
    uint8 m_willpower;
    uint8 m_charisma;
    uint8 m_memory;
    uint8 m_intelligence;

    std::string m_shortDescription;
    std::string m_shortMaleDescription;
    std::string m_shortFemaleDescription;
};

/**
 * Container for character appearance stuff.
 */
class CharacterAppearance {
public:
   uint32 colorID;
   uint32 colorNameA;
   uint32 colorNameBC;
   double weight;
   double gloss;

   uint32 modifierLocationID;
   uint32 paperdollResourceID;
   uint32 paperdollResourceVariation;

   uint32 sculptID;
   double weightUpDown;
   double weightLeftRight;
   double weightForwardBack;

   void Build(uint32 ownerID, PyDict* data);

private:
	CharacterDB m_db;
};

/**
 * * Container for raw character data.
 * v6
 */
class CharacterData {
public:
    CharacterData(
        uint32 _accountID = 0,
        const char* _title = "",
        const char* _desc = "",
        bool _gender = false,
        double _bounty = 0.0,
        double _balance = 0.0,
        double _aurBalance = 0.0,
        double _securityRating = 0.0,
        uint32 _logonMinutes = 0,
        double _skillPoints = 0,
        uint32 _corporationID = 0,
        uint32 _allianceID = 0,
        uint32 _warFactionID = 0,
        uint32 _stationID = 0,
        uint32 _solarSystemID = 0,
        uint32 _constellationID = 0,
        uint32 _regionID = 0,
        uint32 _ancestryID = 0,
        uint8 _bloodlineID = 0,
        uint8 _raceID = 0,
        uint32 _careerID = 0,
        uint32 _schoolID = 0,
        uint32 _careerSpecialityID = 0,
        uint64 _startDateTime = 0,
        uint64 _createDateTime = 0,
        uint32 _shipID = 0,
        uint32 _capsuleID = 0 );
    bool gender;

    uint8 bloodlineID;
    uint8 raceID;
    uint32 accountID;
    uint32 shipID;
    uint32 capsuleID;
    uint32 logonMinutes;
    uint32 corporationID;
    uint32 allianceID;
    uint32 warFactionID;
    uint32 stationID;
    uint32 solarSystemID;
    uint32 constellationID;
    uint32 regionID;
    uint32 ancestryID;
    uint32 careerID;
    uint32 schoolID;
    uint32 careerSpecialityID;

    uint64 startDateTime;
    uint64 createDateTime;

    double bounty;
    double balance;
    double aurBalance;
    double securityRating;
    double skillPoints;

    std::string title;
    std::string description;
};

/**
 * Container for some corporation-membership related stuff.
 */
class CorpData {
public:
    CorpData(
        uint32 _corpHQ = 0,
        int32 _corpAccountKey = 0,
        uint64 _corpRole = 0,
        uint64 _rolesAtAll = 0,
        uint64 _rolesAtBase = 0,
        uint64 _rolesAtHQ = 0,
        uint64 _rolesAtOther = 0
    );

    uint32 corpHQ;    //this really doesn't belong here...
    int32 corpAccountKey;
    uint64 corpRole;
    uint64 rolesAtAll;
    uint64 rolesAtBase;
    uint64 rolesAtHQ;
    uint64 rolesAtOther;
};

/**
 * Class representing character.
 */
class Character
: public InventoryItem
{
    friend class InventoryItem;    // to let it construct us
public:
    /**
     * Loads character.
     *
     * @param[in] factory
     * @param[in] characterID ID of character to load.
     * @return Pointer to new Character object; NULL if failed.
     */
    static CharacterRef Load(ItemFactory& factory, uint32 characterID);
    /**
     * Spawns new character.
     *
     * @param[in] factory
     * @param[in] data ItemData (data for entity table) for new character.
     * @param[in] charData Character data for new character.
     * @param[in] appData Appearance data for new character.
     * @param[in] corpData Corporation membership data for new character.
     * @return Pointer to new Character object; NULL if failed.
     */
    static CharacterRef Spawn(ItemFactory& factory, ItemData& data, CharacterData& charData, CorpData& corpData);

    static CharacterRef Spawn(ItemFactory& factory, ItemData& data) {
        uint32 charID = InventoryItem::CreateItemID( factory, data );
        if( charID == 0 ) return CharacterRef();
        return Character::Load( factory, charID );
    }
    /**
     * Primary public interface:
     */
    bool AlterBalance(double balanceChange);
    void SetLocation(uint32 stationID, uint32 solarSystemID, uint32 constellationID, uint32 regionID);
	void JoinCorporation(uint32 corporationID, const CorpData& roles);
    void SetDescription(const char *newDescription);
    void SetAccountKey(int32 accountKey);
    void SetFleetData(FleetData& fleet);
    uint32 PickAlternateShip(uint32 locationID);

    virtual void Delete();
    void SetClient(Client* pClient)                     { m_pClient = pClient; }
    Client* GetClient()                                 { return m_pClient; }

    typedef CharacterDB::QueuedSkill QueuedSkill;   // structure of <uint32 typeID, uint8 level>
    typedef CharacterDB::SkillQueue SkillQueue;     // vector of QueuedSkill

    void AddItem(InventoryItemRef item);

    /**
     * Checks whether character has the skill.
     *
     * @param[in] skillTypeID ID of skill type to be checked.
     * @return True if character has the skill, false if doesn't.
     */
    bool HasSkill(uint32) const;
    /**
     * Checks whether the character has the skill, and if so, if it has been trained to the level specified.
     *
     * @param[in] skillTypeID ID of skill type to be checked
     * @param[in] skillLevel Level of the skill to be checked to see if it is trained already to at least this level
     * @return True if character has the skill AND that skill has been trained to at least the level specified, False otherwise
     */
    bool HasSkillTrainedToLevel(uint32 skillTypeID, uint32 skillLevel) const;
    /**
     * Returns skill.
     *
     * @param[in] skillTypeID ID of skill type to be returned.
     * @return Pointer to Skill object; NULL if skill was not found.
     */
    SkillRef GetSkill(uint32 skillTypeID) const;
    /**
     * Gets level of skill that is trained.
     *
     * @param[in] skillTypeID ID of skill type to be checked
     * @param[in] zeroForNotInjected true if method should return 0 for un injected skills,
     *  false if it should return -1
     * @return value 0..5 - the level of skill trained, or, if it was not injected,
     *  0 if zeroForNotInjected.is true, -1 otherwise
     */
    int8             GetSkillLevel(uint32 skillTypeID, bool zeroForNotInjected=true) const;
    /**
     * Get ship agility modifier
     *
     * @param[in] cap boolean to add capital ship skills also.
     * @return total modifier for ship agility
     */
    float           GetAgilitySkills(bool cap=false);
    /**
     * Returns skill currently in training.
     *
     * @param[in] newref Whether new reference should be returned.
     * @return Pointer to Skill object; NULL if skill was not found.
     */
    SkillRef        GetSkillInTraining() const;
    /**
     * Returns entire list of skills learned by this character
     *
     * @param[in] empty std::vector<InventoryItemRef> which is populated with list of skills
     */
    void            GetSkillsList(std::vector<InventoryItemRef>& skills) const;

    /**
     * Calculates Total Skillpoints the character has trained
     *
     * @return Skillpoints the character has trained
     */
    EvilNumber      GetTotalSPTrained() { return m_totalSPtrained; };
    /**
     * Calculates Skillpoints per minute rate.
     *
     * @param[in] skill Skill for which the rate is calculated.
     * @return Skillpoints per minute rate.
     */
    EvilNumber      GetSPPerMin(SkillRef skill);
    /**
     * @return Timestamp at which current skill training finishes.
     */
    int64           GetEndOfTraining() const;

    /* InjectSkillIntoBrain(InventoryItem* skill)
     *
     * Perform injection of passed skill into the character.
     * @author xanarox
     * @param InventoryItem
     */
    bool            InjectSkillIntoBrain(SkillRef skill);
    /*
     * GM Version, allows level set
     */
    bool            InjectSkillIntoBrain(SkillRef skill, uint8 level);
    /* AddSkillToSkillQueue()
     *
     * This will add a skill into the skill queue.
     * @author xanarox
     */
    void            AddToSkillQueue(uint32 typeID, uint8 level);
    void            ClearSkillQueue();
    void            PauseSkillQueue();
    void            LoadPausedSkillQueue();
    void            UpdateSkillQueue();
    /**
     * Update skill training end time on char select screen.
     * @author allan
     */
    void            UpdateSkillQueueEndTime( const SkillQueue& queue);
    /**
     * Send Skill Completion Info to client.
     * @author allan
	 * @param[in] pSkill  pointer to completed skill object
     * @param[in] oldLevel  previous level (can be 0)
     * @param[in] newLevel  level just completed
	 * @param[in] EN_Points  previous skill point value
     * @param[in] newPoints  current skill point value
	 * @param[in] stopped	is training not finished?
     */
    void            SendSkillComplete(Skill* pSkill, uint8 oldLevel, uint8 newLevel, EvilNumber EN_Points, int64 newPoints, bool stopped=false);

    PyRep* GetSkillHistory();
	EvilNumber      GetTotalSP();

    // Certificates:
    /** @todo  this whole certificate thing needs to be updated */
    /*
    struct CharCerts {
        uint32 certificateID;
        uint64 grantDate;
        bool visibilityFlags;
    };
    typedef std::vector<CharCerts> Certificates;
    */
    typedef CharacterDB::CharCerts cCertificates;   //structure of CharCerts<uint32 certificateID, uint64 grantDate, bool visibilityFlags>
    typedef CharacterDB::Certificates Certificates; // vector of CharCerts

    /* GrantCertificate( uint32 certificateID )
     *
     * This will add a certificate into the character
     * @author almamu
     */
    bool GrantCertificate( uint32 certificateID );
    /* UpdateCertificate( uint32 certificateID, bool pub )
     *
     * This will change the public status of the certificate
     * @author almamu
     */
    void UpdateCertificate( uint32 certificateID, bool pub );
    /* HasCertificate( uint32 certificateID )
     *
     * This will check if the player has a certificate
     * @author almamu
     */
    bool HasCertificate( uint32 certificateID ) const;
    /* GetCertificates( )
     *
     * This will get the char's certificates
     * @author almamu
     */
    void GetCertificates( Certificates& crt );

    /*
     * Primary public packet builders:
     */
    PyDict* GetCharInfo();
    PyObject* GetDescription() const;
    /* GetSkillQueue()
     *
     * This will get the skills from the skill queue for a character.
     * @author xanarox
    */
    PyTuple* GetSkillQueue();

    /*
     * Public fields:
     */
    const CharacterType&    type() const                        { return static_cast<const CharacterType& >(InventoryItem::type()); }
    uint32                  bloodlineID() const                 { return type().bloodlineID(); }
    EVERace                 race() const                        { return type().race(); }

    // Account:
    uint32                  accountID() const                   { return m_accountID; }

    const std::string&      title() const                       { return m_title; }
    const std::string&      description() const                 { return m_description; }
    bool                    gender() const                      { return m_gender; }

    double                  bounty() const                      { return m_bounty; }
    double                  balance() const                     { return m_balance; }
    double                  aurBalance() const                  { return m_aurBalance; }
    double                  GetSecurityRating() const           { return m_securityStatus; }
    uint32					loginTime() const                   { return m_loginTime; }
    uint32                  logonMinutes() const                { return m_logonMinutes; }

    /**
     *  This is used to modifiy a characters Security Status
     *  @in amount to adjust m_securityStatus
     */
    void                    secStatusChange( double amount )    { m_securityStatus += amount; }

    // Corporation:
    uint32                  corporationID() const               { return m_corporationID; }
    uint32                  corporationHQ() const               { return m_corpHQ; }
    uint32                  allianceID() const                  { return m_allianceID; }
    uint32                  warFactionID() const                { return m_warFactionID; }
    int32                   corpAccountKey() const              { return m_corpAccountKey; }

    // Corporation role:
    uint64                  corpRole() const                    { return m_corpRole; }
    uint64                  rolesAtAll() const                  { return m_rolesAtAll; }
    uint64                  rolesAtBase() const                 { return m_rolesAtBase; }
    uint64                  rolesAtHQ() const                   { return m_rolesAtHQ; }
    uint64                  rolesAtOther() const                { return m_rolesAtOther; }

    // Fleet:
    uint32                  fleetID() const                     { return /*m_fleetID*/0; }
    uint32                  wingID() const                      { return m_wingID; }
    uint32                  squadID() const                     { return m_squadID; }
    uint8                   fleetRole() const                   { return m_fleetRole; }
    uint8                   fleetBooster() const                { return m_fleetBooster; }
    uint8                   fleetJob() const                    { return m_fleetJob; }

    // Current location:
    uint32                  stationID() const                   { return m_stationID; }
    uint32                  solarSystemID() const               { return m_solarSystemID; }
    uint32                  constellationID() const             { return m_constellationID; }
    uint32                  regionID() const                    { return m_regionID; }

    // Ancestry, career:
    uint32                  ancestryID() const                  { return m_ancestryID; }
    uint32                  careerID() const                    { return m_careerID; }
    uint32                  schoolID() const                    { return m_schoolID; }
    uint32                  careerSpecialityID() const          { return m_careerSpecialityID; }

    // Some important dates:
    uint64                  startDateTime() const               { return m_startDateTime; }
    uint64                  createDateTime() const              { return m_createDateTime; }

    uint32                  shipID() const                      { return m_shipID; }
    uint32                  capsuleID() const                   { return m_capsuleID; }

    void                    SetActiveShip(uint32 shipID);
    void                    SetActivePod(uint32 podID);
    void                    ResetClone();

    void                    PayBounty(CharacterRef cRef);
    void                    LogKill(CharKillData data)          { m_db.SaveKillOrLoss(data); }

    //  saves
    void                    SaveCharacter();
    void                    SaveFullCharacter();
    void                    SaveSkillQueue();
    void                    SaveCertificates();
    void                    SaveSkillHistory(uint8 eventID, uint64 logDate, uint32 characterID, uint32 skillTypeID, uint8 skillLevel, double relativePoints, double absolutePoints);

    bool                    isOffline(uint32 online);
    void                    SetLoaded(bool set=false)   { m_loaded = set; }

    void                    SetLoginTime();

	//  Standings functions
	//     toID = me|myCorp|myAlliance.  fromID = char|agent|corp|faction|alliance
	double 					GetAgentStanding(uint32 toID, uint32 fromID);
	double 					GetAllianceStanding(uint32 toID, uint32 fromID);
	double 					GetCharStanding(uint32 toID, uint32 fromID);
	double 					GetCorpStanding(uint32 toID, uint32 fromID);
	double 					GetNPCCorpStanding(uint32 toID, uint32 fromID);
	double 					GetFactionStanding(uint32 toID, uint32 fromID);
	double 					GetStandingChanges();
	void 					SetAgentStanding(uint32 toID, uint32 fromID, double standing);
	void 					SetAllianceStanding(uint32 toID, uint32 fromID, double standing);
	void 					SetCharStanding(uint32 toID, uint32 fromID, double standing);
	void 					SetCorpStanding(uint32 toID, uint32 fromID, double standing);
	void 					SetNPCCorpStanding(uint32 toID, uint32 fromID, double standing);
	void 					SaveStandingChanges(uint32 fromID,
												uint32 toID,
												uint32 eventType,
												double amount,
												std::string msg);

    //  Dynamic Data
	void                    VisitSystem(uint32 solarSystemID);
	void                    chkDynamicSystemID(uint32 solarSystemID);
    void                    AddJumpToDynamicData(uint32 solarSystemID);
    void                    AddPilotToDynamicData(uint32 solarSystemID, bool isAdd = false, bool isDocked = false, bool isLogin = false);
	void                    AddKillToDynamicData(uint32 solarSystemID);
	void                    AddPodKillToDynamicData(uint32 solarSystemID);
	void                    AddFactionKillToDynamicData(uint32 solarSystemID);


    virtual bool _Load();

protected:
    Character(
        ItemFactory& _factory,
        uint32 _characterID,
        // InventoryItem stuff:
        const CharacterType& _charType,
        const ItemData& _data,
        // Character stuff:
        const CharacterData& _charData,
        const CorpData& _corpData
    );
    virtual ~Character();

    /*
     * Member functions:
     */
    using InventoryItem::_Load;
    static uint32 _Spawn(ItemFactory& factory, ItemData& data, CharacterData& charData, CorpData& corpData)  { }


    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem(ItemFactory& factory, uint32 characterID, const ItemType& type, const ItemData& data) {
        if( type.groupID() != EVEDB::invGroups::Character ) {
            sLog.Error("Character", "Trying to load %s as Character.", type.group().name().c_str() );
            return RefPtr<_Ty>();
        }
        CharacterData charData;
        if( !factory.db().GetCharacter( characterID, charData ) )
            return RefPtr<_Ty>();

        CorpData corpData;
        if( !factory.db().GetCorpData( characterID, corpData ) )
            return RefPtr<_Ty>();

        // cast the type
        const CharacterType& charType = static_cast<const CharacterType& >( type );

        return _Ty::template CreateCharacter<_Ty>( factory, characterID, charType, data, charData, corpData );
    }

    // Actual creation method:
    template<class _Ty>
    static RefPtr<_Ty> CreateCharacter(ItemFactory& factory, uint32 characterID,
        const CharacterType& charType, const ItemData& data,
        const CharacterData& charData, const CorpData& corpData
    );

    void _CalculateTotalSPTrained();

    void _GetLogonMinutes();

private:
    /*
     * Data members
     */
    uint32 m_accountID;

    std::string m_title;
    std::string m_description;
    bool m_gender;

    double m_bounty;
    double m_balance;
    double m_aurBalance;
    double m_securityStatus;
	uint32 m_loginTime;
    uint32 m_logonMinutes;

    uint32 m_corporationID;
    uint32 m_corpHQ;
    uint32 m_allianceID;
    uint32 m_warFactionID;

    int32 m_corpAccountKey;
    uint64 m_corpRole;
    uint64 m_rolesAtAll;
    uint64 m_rolesAtBase;
    uint64 m_rolesAtHQ;
    uint64 m_rolesAtOther;

    uint32 m_fleetID;
    uint32 m_wingID;
    uint32 m_squadID;
    uint8 m_fleetRole;
    uint8 m_fleetBooster;
    uint8 m_fleetJob;

    uint32 m_stationID;
    uint32 m_solarSystemID;
    uint32 m_constellationID;
    uint32 m_regionID;

    uint32 m_ancestryID;
    uint8  m_bloodlineID;
    uint8  m_raceID;
    uint32 m_careerID;
    uint32 m_schoolID;
    uint32 m_careerSpecialityID;

    uint64 m_startDateTime;
    uint64 m_createDateTime;

    uint32 m_shipID;
    uint32 m_capsuleID;

    // Skill queue:
    SkillQueue m_skillQueue;
    EvilNumber m_totalSPtrained;
    uint32 m_freePoints;

    Certificates m_certificates;

	CharacterDB m_db;
    StandingDB s_db;

    Client* m_pClient;

    bool m_loaded;      /* to avoid multiple load calls (_Load is called ~4x) */
};

#endif /* !__CHARACTER__H__INCL__ */

