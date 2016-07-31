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
    Author:        Aknor Jaden, Luck    (original code)
    Updates:    Allan   (reworked and implemented)
*/

/* major updates to clean up code and implement basic memory management (remove naked 'new')  -allan 9Mar16 */

#ifndef MODULE_EFFECTS_H
#define MODULE_EFFECTS_H

#include "ship/modules/ModuleDB.h"
#include "ship/modules/ModuleDefs.h"
#include "utils/Singleton.h"


typedef std::vector<uint32> typeTargetGroupIDlist;

class MEffect
{
public:
    MEffect(uint32 effectID);
    ~MEffect();

    //accessors for selected effect
    bool IsEffectLoaded()                                       { return m_EffectLoaded; }
    bool IsEffectsInfoLoaded()                                  { return m_EffectsInfoLoaded; }

    bool GetIsOffensive()                                       { return (m_EffectID == 0) ? false : m_IsOffensive == 1; }
    bool GetIsAssistance()                                      { return (m_EffectID == 0) ? false : m_IsAssistance == 1; }
    bool GetDisallowAutoRepeat()                                { return (m_EffectID == 0) ? false : m_DisallowAutoRepeat == 1; }
    bool GetIsPublished()                                       { return (m_EffectID == 0) ? false : m_Published == 1; }
    bool GetIsWarpSafe()                                        { return (m_EffectID == 0) ? false : m_IsWarpSafe == 1; }
    bool GetRangeChance()                                       { return (m_EffectID == 0) ? false : m_RangeChance == 1; }
    bool GetPropulsionChance()                                  { return (m_EffectID == 0) ? false : m_PropulsionChance == 1; }
    bool GetElectronicChance()                                  { return (m_EffectID == 0) ? false : m_ElectronicChance == 1; }

    uint32 GetEffectID()                                        { return (m_EffectID == 0) ? 0 : m_EffectID; }
    uint32 GetEffectCategory()                                  { return (m_EffectID == 0) ? 0 : m_EffectCategory; }
    uint32 GetPreExpression()                                   { return (m_EffectID == 0) ? 0 : m_PreExpression; }
    uint32 GetPostExpression()                                  { return (m_EffectID == 0) ? 0 : m_PostExpression; }
    uint32 GetIconID()                                          { return (m_EffectID == 0) ? 0 : m_IconID; }
    uint32 GetDurationAttributeID()                             { return (m_EffectID == 0) ? 0 : m_DurationAttributeID; }
    uint32 GetTrackingSpeedAttributeID()                        { return (m_EffectID == 0) ? 0 : m_TrackingSpeedAttributeID; }
    uint32 GetDischargeAttributeID()                            { return (m_EffectID == 0) ? 0 : m_DischargeAttributeID; }
    uint32 GetRangeAttributeID()                                { return (m_EffectID == 0) ? 0 : m_RangeAttributeID; }
    uint32 GetFalloffAttributeID()                              { return (m_EffectID == 0) ? 0 : m_FalloffAttributeID; }
    uint32 GetDistribution()                                    { return (m_EffectID == 0) ? 0 : m_Distribution; }
    uint32 GetNpcUsageChanceAttributeID()                       { return (m_EffectID == 0) ? 0 : m_NpcUsageChanceAttributeID; }
    uint32 GetNpcActivationChanceAttributeID()                  { return (m_EffectID == 0) ? 0 : m_NpcActivationChanceAttributeID; }
    uint32 GetFittingUsageChanceAttributeID()                   { return (m_EffectID == 0) ? 0 : m_FittingUsageChanceAttributeID; }

    std::string GetGuid()                                       { return (m_EffectID == 0) ? std::string("") : m_Guid; }
    std::string GetSfxName()                                    { return (m_EffectID == 0) ? std::string("") : m_DisplayName; }
    std::string GetEffectName()                                 { return (m_EffectID == 0) ? std::string("") : m_EffectName; }
    std::string GetDisplayName()                                { return (m_EffectID == 0) ? std::string("") : m_DisplayName; }

    //accessors for the data loaded, if any, from the dgmEffectsInfo table:
    uint32 GetSizeOfAttributeList()                             { return ((m_EffectID == 0) || (!m_EffectsInfoLoaded)) ? 0 : m_numOfIDs; }
    uint32 GetSourceAttributeID(uint32 index)                   { return ((m_EffectID == 0) || (!m_EffectsInfoLoaded)) ? 0 : m_SourceAttributeIDs.at(index); }
    uint32 GetTargetAttributeID(uint32 index)                   { return ((m_EffectID == 0) || (!m_EffectsInfoLoaded)) ? 0 : m_TargetAttributeIDs.at(index); }
    uint32 GetStackingPenalty(uint32 index)                     { return ((m_EffectID == 0) || (!m_EffectsInfoLoaded)) ? 0 : m_StackingPenalty.at(index); }
    uint32 GetEffectState()                                     { return ((m_EffectID == 0) || (!m_EffectsInfoLoaded)) ? 0 : m_EffectState.at(0); }
    uint32 GetTargetGroup(uint32 index)                         { return ((m_EffectID == 0) || (!m_EffectsInfoLoaded)) ? 0 : m_targetGroup.at(index); }
	uint32 GetTargetType(uint32 index)                          { return ((m_EffectID == 0) || (!m_EffectsInfoLoaded)) ? 0 : m_targetType.at(index); }
    EVECalculationType GetCalculationType(uint32 index)         { return ((m_EffectID == 0) || (!m_EffectsInfoLoaded)) ? (EVECalculationType)0 : (EVECalculationType)m_CalculationTypeIDs.at(index);}
    EVECalculationType GetReverseCalculationType(uint32 index)  { return ((m_EffectID == 0) || (!m_EffectsInfoLoaded)) ? (EVECalculationType)0 : (EVECalculationType)m_ReverseCalculationTypeIDs.at(index);}

    typeTargetGroupIDlist GetTargetIDList(uint32 index);

private:
    void _Populate(uint32 effectID);

    bool m_EffectLoaded;
    bool m_EffectsInfoLoaded;

    uint32 m_EffectID;
    uint32 m_numOfIDs;
    uint32 m_EffectCategory;
    uint32 m_PreExpression;
    uint32 m_PostExpression;
    uint32 m_IconID;
    uint32 m_IsOffensive;
    uint32 m_IsAssistance;
    uint32 m_DurationAttributeID;
    uint32 m_TrackingSpeedAttributeID;
    uint32 m_DischargeAttributeID;
    uint32 m_RangeAttributeID;
    uint32 m_FalloffAttributeID;
    uint32 m_DisallowAutoRepeat;
    uint32 m_Published;
    uint32 m_IsWarpSafe;
    uint32 m_RangeChance;
    uint32 m_ElectronicChance;
    uint32 m_PropulsionChance;
    uint32 m_Distribution;
    uint32 m_NpcUsageChanceAttributeID;
    uint32 m_NpcActivationChanceAttributeID;
    uint32 m_FittingUsageChanceAttributeID;

    std::vector<uint32> m_targetGroup;
    std::vector<uint32> m_targetType;
    std::vector<uint32> m_StackingPenalty;
    std::vector<uint32> m_SourceAttributeIDs;
    std::vector<uint32> m_TargetAttributeIDs;
    std::vector<uint32> m_CalculationTypeIDs;
    std::vector<uint32> m_EffectState;
    std::vector<uint32> m_ReverseCalculationTypeIDs;

    std::string m_Guid;
    std::string m_SfxName;
    std::string m_EffectName;
    std::string m_DisplayName;

    std::map<uint32, std::string> m_Descriptions;
    std::map<uint32, typeTargetGroupIDlist> m_TargetGroupIDlists;
};


class TypeEffectsList
{
public:
	TypeEffectsList(uint32 effectID);
	~TypeEffectsList();

	bool HasEffect(uint32 effectID);
	uint32 GetEffectCount()                                     { return m_typeEffectsList.size(); }
	void GetEffectsList(std::map<uint32,uint32> * effectsList);

protected:
	std::map<uint32,uint32> m_typeEffectsList;
};

// This class is a singleton object, containing all Effects loaded from dgmEffects table as memory objects of type MEffect:
class DGM_Effects_Table
: public Singleton< DGM_Effects_Table >
{
public:
    DGM_Effects_Table();
    ~DGM_Effects_Table();

    // Initializes the Table:
    int Initialize();

    // Returns pointer to MEffect object corresponding to the effectID supplied:
    std::shared_ptr<MEffect> GetEffect(uint32 effectID);

protected:
    void _Populate();

    std::map<uint32, std::shared_ptr<MEffect>> m_EffectsMap;
};

#define sDGM_Effects_Table \
    ( DGM_Effects_Table::get() )



//class contained by all modules that is populated on construction of the module
//this will contain all information about the effects of the module
class InventoryItem;
class ModuleEffects
{
public:
    ModuleEffects(InventoryItem* pItem);
    ~ModuleEffects();

    //useful accessors - probably a better way to do this, but at least it's fast
    //  found a better way, and faster.  -allan 22Dec15
    bool isLowSlot()                                            { return m_loPower; }
    bool isMediumSlot()                                         { return m_medPower; }
    bool isHighSlot()                                           { return m_hiPower; }
    bool isRig()                                                { return m_rigSlot; }
    bool isSubSystem()                                          { return m_subSystem; }
    bool needsTarget()                                          { return m_targReq; }
    bool isWarpSafe()                                           { return m_warpSafe; }

    bool HasEffect(uint32 effectID);
    bool HasDefaultEffect()                                     { return (m_defaultEffect ? true : false); }
    MEffect* GetDefaultEffect()                                 { return m_defaultEffect; }
    MEffect* GetEffect(uint32 effectID);

    // online/offline effects are common for ALL modules/systems and implemented in GenericModule() class
    typedef std::map<uint32, std::shared_ptr<MEffect>>::const_iterator itrDef;
    itrDef GetOnlineEffectsBegin()                              { return m_OnlineEffects.begin(); }
    itrDef GetOnlineEffectsEnd()                                { return m_OnlineEffects.end(); }
    size_t GetOnlineEffectsSize()                               { return m_OnlineEffects.size(); }

    itrDef GetActiveEffectsBegin()                              { return m_ActiveEffects.begin(); }
    itrDef GetActiveEffectsEnd()                                { return m_ActiveEffects.end(); }
    size_t GetActiveEffectsSize()                               { return m_ActiveEffects.size(); }

    // these are not implemented yet......
    itrDef GetOverloadEffectsBegin()                            { return m_OverloadEffects.begin(); }
    itrDef GetOverloadEffectsEnd()                              { return m_OverloadEffects.end(); }
    size_t GetOverloadEffectsSize()                             { return m_OverloadEffects.size(); }

    itrDef GetGangEffectsBegin()                                { return m_GangEffects.begin(); }
    itrDef GetGangEffectsEnd()                                  { return m_GangEffects.end(); }
    size_t GetGangEffectsSize()                                 { return m_GangEffects.size(); }

    itrDef GetFleetEffectsbegin()                               { return m_FleetEffects.begin(); }
    itrDef GetFleetEffectsEnd()                                 { return m_FleetEffects.end(); }
    size_t GetFleetEffectsSize()                                { return m_FleetEffects.size(); }


private:
    void _populate();

    MEffect* m_defaultEffect;

    InventoryItem* m_pItem;

    //data members
    std::map<uint32, std::shared_ptr<MEffect>> m_GangEffects;
    std::map<uint32, std::shared_ptr<MEffect>> m_FleetEffects;
    std::map<uint32, std::shared_ptr<MEffect>> m_OnlineEffects;
    std::map<uint32, std::shared_ptr<MEffect>> m_ActiveEffects;
    std::map<uint32, std::shared_ptr<MEffect>> m_OverloadEffects;

    //cached stuff
    bool m_hiPower, m_medPower, m_loPower, m_rigSlot, m_subSystem, m_warpSafe, m_targReq;

};

#endif /* MODULE_EFFECTS_H */
//////////////////////////////////////////////////////////////////////////
