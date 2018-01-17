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

#include "character/CertificateMgrService.h"
#include "character/CharacterDB.h"
#include "character/Skill.h"
#include "inventory/ItemType.h"
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
    static CharacterType* Load( uint32 characterTypeID);

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
    static _Ty* _LoadType( uint32 typeID, const ItemGroup& group, const TypeData& data)
    {
        // check we are really loading a character type
        if( group.id() != EVEDB::invGroups::Character ) {
            sLog.Error("Character", "Load of character type %u requested, but it's %s.", typeID, group.name().c_str() );
            return nullptr;
        }

        // query character type data
        uint32 bloodlineID;
        CharacterTypeData charData;
        if( !sItemFactory.db()->GetCharacterType(typeID, bloodlineID, charData) )
            return nullptr;

        // load ship type
        const ItemType* type = sItemFactory.GetType( charData.shipTypeID );
        if( type == nullptr )
            return nullptr;

        return new CharacterType( typeID, bloodlineID, group, data, *type, charData );
    }

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
 * Class representing character.
 */
class Character
: public InventoryItem
{
    friend class InventoryItem;    // to let it construct us
public:

    /**
     * Primary public interface:
     */
    bool AlterBalance(double amount, uint8 type);
    void SetLocation(uint32 stationID, uint32 solarSystemID, uint32 constellationID, uint32 regionID);
	void JoinCorporation(const CorpData& data);
    void SetDescription(const char *newDescription);
    void SetAccountKey(int32 accountKey);
    void SetFleetData(CharFleetData& fleet);
    uint32 PickAlternateShip(uint32 locationID);

    virtual void Delete();
    void SetClient(Client* pClient)                     { m_pClient = pClient; }
    Client* GetClient()                                 { return m_pClient; }

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
     * Get char's Research and Manufacturing skills
     *
     * @param[in] none
     * @return Python wire object
     */
    PyRep*          GetRAMSkills();

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

    double      GetTotalSPTrained() { return m_charData.skillPoints; };
    /**
     * Calculates Skillpoints per minute rate.
     *
     * @param[in] skill Skill for which the rate is calculated.
     * @return Skillpoints per minute rate.
     */
    EvilNumber      GetSPPerMin(Skill* skill);
    /**
     * @return Timestamp at which current skill training finishes.
     */
    double GetEndOfTraining() const;

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

    /* GrantCertificate( uint32 certificateID )
     *
     * This will add a certificate into the character
     * @author almamu
     */
    void GrantCertificate( uint32 certificateID );
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
    void GetCertificates( CertVector& crt );

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
    uint32                  accountID() const                   { return m_charData.accountID; }

    const std::string&      title() const                       { return m_charData.title; }
    const std::string&      description() const                 { return m_charData.description; }
    bool                    gender() const                      { return m_charData.gender; }

    double                  bounty() const                      { return m_charData.bounty; }
    double                  balance(uint8 type);
    double                  GetSecurityRating() const           { return m_charData.securityRating; }
    uint32					loginTime() const                   { return m_loginTime; }
    uint32                  logonMinutes() const                { return m_charData.logonMinutes; }
    uint16                  OnlineTime();

    void                    secStatusChange( double amount )    { m_charData.securityRating += amount; }

    // Corporation:
    void                    UpdateCorpData(CorpData data);
    CorpData                GetCorpData()                       { return m_corpData; }
    std::string             corpTicker() const                  { return m_corpData.ticker; }
    uint32                  corporationID() const               { return m_corpData.corporationID; }
    uint32                  corporationHQ() const               { return m_corpData.corpHQ; }
    uint32                  allianceID() const                  { return m_corpData.allianceID; }
    uint32                  warFactionID() const                { return m_corpData.warFactionID; }
    int32                   corpAccountKey() const              { return m_corpData.corpAccountKey; }
    double                  corpTaxRate() const                 { return m_corpData.taxRate; }
    void                    SetCorpHQ(uint32 stationID)         { m_corpData.corpHQ = stationID; UpdateCorpData(m_corpData);}

    // Corporation role:
    int64                  corpRole() const                     { return m_corpData.corpRole; }
    int64                  rolesAtAll() const                   { return m_corpData.rolesAtAll; }
    int64                  rolesAtBase() const                  { return m_corpData.rolesAtBase; }
    int64                  rolesAtHQ() const                    { return m_corpData.rolesAtHQ; }
    int64                  rolesAtOther() const                 { return m_corpData.rolesAtOther; }

    // Fleet:
    int64                   fleetJoinTime()                     { return m_fleetData.joinTime; }
    int32                   fleetID() const                     { return m_fleetData.fleetID; }
    int32                   wingID() const                      { return m_fleetData.wingID; }
    int32                   squadID() const                     { return m_fleetData.squadID; }
    int8                    fleetRole() const                   { return m_fleetData.role; }
    int8                    fleetBooster() const                { return m_fleetData.booster; }
    int8                    fleetJob() const                    { return m_fleetData.job; }

    // Current location:
    uint32                  stationID() const                   { return m_charData.stationID; }
    uint32                  solarSystemID() const               { return m_charData.solarSystemID; }
    uint32                  constellationID() const             { return m_charData.constellationID; }
    uint32                  regionID() const                    { return m_charData.regionID; }

    // Ancestry, career:
    uint32                  ancestryID() const                  { return m_charData.ancestryID; }
    uint32                  careerID() const                    { return m_charData.careerID; }
    uint32                  schoolID() const                    { return m_charData.schoolID; }
    uint32                  careerSpecialityID() const          { return m_charData.careerSpecialityID; }

    // Some important dates:
    int64                  startDateTime() const               { return m_charData.startDateTime; }
    int64                  createDateTime() const              { return m_charData.createDateTime; }

    uint32                  shipID() const                      { return m_charData.shipID; }
    uint32                  capsuleID() const                   { return m_charData.capsuleID; }

    void                    SetActiveShip(uint32 shipID);
    void                    SetActivePod(uint32 podID);
    void                    ResetClone();

    void                    PayBounty(CharacterRef cRef);
    void                    LogKill(CharKillData data)          { m_db.SaveKillOrLoss(data); }

    //  saves
    void                    LogOut();
    void                    SaveBookMarks();
    void                    SaveCharacter();
    void                    SaveFullCharacter();
    void                    SaveSkillQueue();
    void                    SaveCertificates();
    void                    SaveSkillHistory(uint16 eventID, double logDate, uint32 characterID, uint32 skillTypeID, uint8 skillLevel, double absolutePoints);

    void                    SetLoaded(bool set=false)   { m_loaded = set; }

    void                    SetLoginTime();
    void                    SetLogonMinutes();

	//  Standings functions
	//     toID = me|myCorp|myAlliance.  fromID = char|agent|corp|faction|alliance
    double 					GetStanding(uint32 toID, uint32 fromID);    // NOTE:  this is NOT adjusted for skills
    double                  GetNPCCorpStanding(uint32 toID, uint32 fromID);
	double 					GetStandingChanges();
	void 					SetStanding(uint32 toID, uint32 fromID, double standing);
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

    // character skill, implant and booster effects.  parsed on char load.  applied on ship init in space (with all other ship-related effects)
    // NOTE:  implants and boosters not implemented yet
    void                    ProcessEffects();
    void                    ResetModifiers();   // this will reset ALL char and skill attribs and modifier maps to default

protected:
    Character(
        uint32 _characterID,
        // InventoryItem stuff:
        const CharacterType& _charType,
        const ItemData& _data,
        // Character stuff:
        const CharacterData& _charData,
        const CorpData& _corpData
    );
    virtual ~Character();

    void VerifySP();

    void LoadBookmarks();

    /*
     * templated loading system
     */
public:
    static CharacterRef Load( uint32 characterID);
    static CharacterRef Spawn( CharacterData& charData, CorpData& corpData);
    virtual bool _Load();

protected:
    using InventoryItem::_Load;

    static uint32 _Spawn( ItemData& data, CharacterData& charData, CorpData& corpData)  { }

    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem( uint32 characterID, const ItemType& type, const ItemData& data) {
        if( type.groupID() != EVEDB::invGroups::Character ) {
            sLog.Error("Character", "Trying to load %s as Character.", type.group().name().c_str() );
            if (sConfig.server.StackTrace)
                EvE::traceStack();
            return RefPtr<_Ty>();
        }
        CharacterData charData;
        if( !sItemFactory.db()->GetCharacterData( characterID, charData ) )
            return RefPtr<_Ty>();

        CorpData corpData;
        if( !sItemFactory.db()->GetCorpData( characterID, corpData ) )
            return RefPtr<_Ty>();

        // cast the type
        const CharacterType& charType = static_cast<const CharacterType& >( type );

        // construct the character item
        return CharacterRef( new Character( characterID, charType, data, charData, corpData ) );
    }


private:
    CertificateMgrDB m_cdb;
    CharacterDB m_db;
    StandingDB s_db;

    Client* m_pClient;

    CorpData m_corpData;
    CharacterData m_charData;
    CharFleetData m_fleetData;

    // Skill queue:
    SkillQueue m_skillQueue;
    uint32 m_freePoints;

    CertVector m_certificates;

    bool m_loaded;      /* to avoid multiple load calls (_Load is called ~4x) */

    uint32 m_loginTime;

};

#endif /* !__CHARACTER__H__INCL__ */

