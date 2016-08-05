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
    Updates:        Allan
*/

#ifndef __CHARACTERDB_H_INCL__
#define __CHARACTERDB_H_INCL__

#include "ServiceDB.h"

class PyObject;
class PyString;
class PyObjectEx;
class CharacterData;
class CharacterAppearance;
class CharKillData;
class ItemFactory;
class InventoryItem;
class Client;

class CharacterDB : public ServiceDB
{
public:
    CharacterDB();

    PyRep *GetCharacterList(uint32 accountID);
    PyRep *GetCharSelectInfo(uint32 characterID);
    void UpdateCharCorpRecords(uint32 charID, uint32 corpID);
    void SetAvatar(uint32 charID, PyRep* hairDarkness);
	void SetAvatarColors(uint32 charID, uint32 colorID, uint32 colorNameA, uint32 colorNameBC, double weight, double gloss);
	void SetAvatarModifiers(uint32 charID, PyRep* modifierLocationID,  PyRep* paperdollResourceID, PyRep* paperdollResourceVariation);
	void SetAvatarSculpts(uint32 charID, PyRep* sculptLocationID, PyRep* weightUpDown, PyRep* weightLeftRight, PyRep* weightForwardBack);
    PyRep *GetCharPublicInfo(uint32 characterID);
    PyRep *GetCharPublicInfo3(uint32 characterID);
    PyRep *GetInfoWindowDataForChar(uint32 characterID);
    //PyObject *GetAgentPublicInfo(uint32 agentID);
    PyRep *GetOwnerNoteLabels(uint32 charID);
    PyRep *GetOwnerNote(uint32 charID, uint32 noteID);
    uint32 PickAlternateShip(uint32 charID, uint32 locationID);
    void SetCurrentShip(uint32 charID, uint32 shipID);
    void SetCurrentPod(uint32 charID, uint32 podID);

    bool ChangeCloneType(uint32 characterID, uint32 typeID);
	bool GetCharClones(uint32 characterID, std::vector<uint32> &into);
    bool GetActiveClone(uint32 characterID, uint32 &itemID);
    bool GetActiveCloneType(uint32 characterID, uint32 &typeID);
    void GetCharacterData(uint32 characterID, std::map<std::string, uint64> &characterDataMap);
	bool GetCharHomeStation(uint32 characterID, uint32 &stationID);

    bool ValidateCharName(const char *name);
    /**
     * add_name_validation_set
     *
     * This method will add a character name and ID to the name validation set
     * for use in checking character names at creation and login.
     *
     * @param[in] name
     * @param[in] characterID
     * @return true if adding is successful and false if it was not.
     * @author Captnoord, Firefoxpdm
     */
    bool add_name_validation_set(const char* name, uint32 characterID);
    /**
     * del_name_validation_set
     *
     * This method will remove a entry from the name validation set based
     * on the passed characterID
     *
     * @param[in] characterID
     * @return true if the deletion was successful and false if a error occurred.
     * @author Captnoord, Firefoxpdm
     */
    bool del_name_validation_set(uint32 characterID);
    bool GetCharItems(uint32 characterID, std::vector<uint32> &into);
    bool GetLocationByStation(uint32 staID, CharacterData &cdata);
    bool GetCareerStationByCorporation(uint32 corporationID, uint32 &stationID);
    bool GetCareerBySchool(uint32 schoolID, uint8 &raceID, uint32 &careerID);
    bool GetCorporationBySchool(uint32 schoolID, uint32 &corporationID);
    bool GetLocationCorporationByCareer(CharacterData &cdata);
    bool DoesCorporationExist(uint32 corpID);

    /**
     * Obtains attribute bonuses for given ancestry.
     *
     * @param[in] ancestryID ID of ancestry.
     * @param[out] intelligence Bonus to intelligence.
     * @param[out] charisma Bonus to charisma.
     * @param[out] perception Bonus to perception.
     * @param[out] memory Bonus to memory.
     * @param[out] willpower Bonus to willpower.
     * @return True if operation succeeded, false if failed.
     */
    bool GetAttributesFromAncestry(uint32 ancestryID, uint8 &intelligence, uint8 &charisma, uint8 &perception, uint8 &memory, uint8 &willpower);

    bool        GetBaseSkills(std::map<uint32, uint32> &into);
    bool        GetSkillsByRace(uint32 raceID, std::map<uint32, uint32> &into);
    bool        GetSkillsByCareer(uint32 careerID, std::map<uint32, uint32> &into);

    /**
     * Retrieves the character note from the database as a PyString pointer.
     *
     * @author LSMoura
     */
    PyString *GetNote(uint32 ownerID, uint32 itemID);

    /**
     * Stores the character note on the database, given the ownerID and itemID and the string itself.
     *
     * If the String is null or size zero, the entry is removed from the database.
     *
     * @return boolean true if success.
     *
     * @author LSMoura
     */
    bool SetNote(uint32 ownerID, uint32 itemID, const char *str);

    uint32 AddOwnerNote(uint32 charID, const std::string &label, const std::string &content);
    bool EditOwnerNote(uint32 charID, uint32 noteID, const std::string &label, const std::string &content);

    uint64 PrepareCharacterForDelete(uint32 accountID, uint32 charID);
    void CancelCharacterDeletePrepare(uint32 accountID, uint32 charID);
    PyRep* DeleteCharacter(uint32 accountID, uint32 charID);

    bool ReportRespec(uint32 characterId);
    bool GetRespecInfo(uint32 characterId, uint32& out_freeRespecs, uint64& out_lastRespec, uint64& out_nextRespec);

    // Skill queue:
    struct QueuedSkill {
        uint32 typeID;
        uint8 level;
    };
    typedef std::vector<QueuedSkill> SkillQueue;

    /**
     * Loads skill queue.
     *
     * @param[in] characterID ID of character whose queue should be loaded.
     * @param[in] into SkillQueue into which loaded data should be stored.
     * @return True if load succeeds, false if fails.
     */
    bool        LoadSkillQueue(uint32 characterID, SkillQueue &into);
    bool        LoadPausedSkillQueue(uint32 characterID, SkillQueue &into);
    /**
     * Saves skill queue.
     *
     * @param[in] characterID ID of character whose skill queue is saved.
     * @param[in] queue Queue to save.
     * @return True if save succeeds, false if fails.
     */
    bool        SaveSkillQueue(uint32 characterID, SkillQueue &queue);
    bool        SavePausedSkillQueue(uint32 characterID, SkillQueue &queue);
    void        SaveSkillHistory(uint8 eventID, double logDate, uint32 characterID, uint32 skillTypeID, uint8 skillLevel, double relativePoints, double absolutePoints);
    PyRep*      GetSkillHistory(uint32 characterID);
    void        UpdateSkillQueueEndTime(uint64 endtime, uint32 charID);

    /** Certificates */
    struct CharCerts {
        uint32 certificateID;
        uint64 grantDate;
        bool visibilityFlags;
    };
    typedef std::vector<CharCerts> Certificates;
    bool LoadCertificates( uint32 characterID, Certificates &into );
    bool SaveCertificates( uint32 characterID, const Certificates &from );
    void AddCertificate(uint32 charID, CharCerts cert);
    void UpdateCertificate(uint32 charID, uint32 certificateID, bool pub);

	bool 		isOffline(uint32 characterID);

	void 		addOwnerCache(uint32 ownerID, std::string ownerName, uint32 typeID);

	PyRep*      GetBounty(uint32 charID, uint32 ownerID);
    PyRep*      GetTopBounties();
    void        AddBounty(uint32 charID, uint32 ownerID, uint32 amount);
    uint32      PayBounty(CharacterRef cRef);

    void        SaveKillOrLoss(CharKillData& data);
    PyRep* GetKillOrLoss(uint32 charID);

	// for dynamic db functions    -allan
	void        VisitSystem(uint32 solarSystemID, uint32 charID);
	void        chkDynamicSystemID(uint32 solarSystemID);
	void        AddJumpToDynamicData(uint32 solarSystemID);
    void        AddPilotToDynamicData(uint32 solarSystemID, bool isAdd, bool isDocked, bool isLogin);
	void        AddKillToDynamicData(uint32 solarSystemID);
	void        AddPodKillToDynamicData(uint32 solarSystemID);
	void        AddFactionKillToDynamicData(uint32 solarSystemID);
    void        GetActivePilotsFromDynamicData(uint32 solarSystemID, uint16 &pilotsDocked, uint16 &pilotsInSpace);


private:
    /**
     * djb2 algorithm taken from http://www.cse.yorku.ca/~oz/hash.html slightly modified
     *
     * @param[in] str string that needs to be hashed.
     * @return djb2 hash of the string.
     */
    uint32 djb2_hash(const char* str);

    /**
     * load_name_validation_set
     * This method will load up all character names into a set for validating
     * character names.
     *
     * @author Captnoord, Firefoxpdm
     */
    void load_name_validation_set();

    /* set only for validation */
    typedef std::set<uint32>            CharValidationSet;
    typedef CharValidationSet::iterator    CharValidationSetItr;
    CharValidationSet mNameValidation;

    /* helper object for deleting ( wasting mem here ) */
    typedef std::map<uint32, std::string>    CharIdNameMap;
    typedef CharIdNameMap::iterator            CharIdNameMapItr;
    CharIdNameMap mIdNameContainer;
};

#endif
