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

/** @todo  this code needs to be walked thru and cleaned up.  WIP - allan 12June16 */

#include "eve-server.h"

#include "ship/modules/ModuleEffects.h"
#include "inventory/InventoryItem.h"

// ////////////////// MEffect Class ///////////////////////////
MEffect::MEffect(uint32 effectID)
{
    m_EffectID = effectID;
    m_Guid = "";
    m_SfxName = "";
    m_EffectName = "";
    m_DisplayName = "";
    m_IconID = 0;
    m_numOfIDs = 0;
    m_Published = 0;
    m_IsWarpSafe = 0;
    m_RangeChance = 0;
    m_IsOffensive = 0;
    m_IsAssistance = 0;
    m_Distribution = 0;
    m_PreExpression = 0;
    m_EffectCategory = 0;
    m_PostExpression = 0;
    m_RangeAttributeID = 0;
    m_ElectronicChance = 0;
    m_PropulsionChance = 0;
    m_FalloffAttributeID = 0;
    m_DisallowAutoRepeat = 0;
    m_DurationAttributeID = 0;
    m_DischargeAttributeID = 0;
    m_TrackingSpeedAttributeID = 0;
    m_NpcUsageChanceAttributeID = 0;
    m_FittingUsageChanceAttributeID = 0;
    m_NpcActivationChanceAttributeID = 0;

	m_EffectLoaded = false;
	m_EffectsInfoLoaded = false;

    m_targetGroup.clear();
    m_targetType.clear();
    m_SourceAttributeIDs.clear();
    m_TargetAttributeIDs.clear();
    m_TargetGroupIDlists.clear();
    m_CalculationTypeIDs.clear();
    m_EffectState.clear();
    m_ReverseCalculationTypeIDs.clear();
    m_StackingPenalty.clear();

	_Populate(effectID);
}

MEffect::~MEffect() {
    m_TargetGroupIDlists.clear();

    m_targetGroup.clear();
    m_targetType.clear();
    m_SourceAttributeIDs.clear();
    m_TargetAttributeIDs.clear();
    m_CalculationTypeIDs.clear();
    m_EffectState.clear();
    m_ReverseCalculationTypeIDs.clear();
    m_StackingPenalty.clear();
}

void MEffect::_Populate(uint32 effectID)
{
    if (m_EffectLoaded)
        return;
    DBQueryResult* res = new DBQueryResult();
    ModuleDB::GetDgmEffectsInfo(effectID, *res);

    // Initialize the new tables
	if (res->GetRowCount() > 0) {
		int count = 0;
		std::string targetGroupIDs;
        typeTargetGroupIDlist tgtGrpIDsList;

        DBResultRow row;
		while (res->GetRow(row)) {
            m_targetType.push_back(row.GetUInt(7));
            m_EffectState.push_back(row.GetUInt(6));
            m_targetGroup.push_back(row.GetUInt(8));
            m_StackingPenalty.push_back(row.GetUInt(5));
			m_SourceAttributeIDs.push_back(row.GetUInt(0));
			m_TargetAttributeIDs.push_back(row.GetUInt(1));
			m_CalculationTypeIDs.push_back(row.GetUInt(2));
            m_ReverseCalculationTypeIDs.push_back(row.GetUInt(3));

            targetGroupIDs = row.GetText(4);
			if (!targetGroupIDs.empty()) {
				// targetGroupIDs string is not empty, so extract one number at a time until it is
				int pos = 0;
				std::string tempString = "";

				pos = targetGroupIDs.find_first_of(';');
				if (pos < 0)
					pos = targetGroupIDs.length()-1;	// we did not find any ';' characters, so targetGroupIDs contains only one number
				tempString = targetGroupIDs.substr(0,pos);

				while ((pos = targetGroupIDs.find_first_of(';')) > 0 ) {
					tempString = targetGroupIDs.substr(0,pos);
					tgtGrpIDsList.insert(tgtGrpIDsList.begin(), (atoi(tempString.c_str())));
					targetGroupIDs = targetGroupIDs.substr(pos+1,targetGroupIDs.length()-1);
				}

				// Get final number now that there are no more separators to find:
				if (!targetGroupIDs.empty())
					tgtGrpIDsList.insert(tgtGrpIDsList.begin(), (atoi(targetGroupIDs.c_str())));

                m_TargetGroupIDlists.insert(std::pair<uint32, typeTargetGroupIDlist>(count, tgtGrpIDsList));
			}
			++count;
		}

		if (!count) {
            sLog.Warning("MEffect::_Populate()","Could not populate effect information for effectID: %u from the 'dgmEffectsInfo' table as the SQL query returned ZERO rows", effectID);
			m_EffectsInfoLoaded = false;
		} else {
			m_numOfIDs = count;
			m_EffectsInfoLoaded = true;
		}
        m_EffectLoaded = true;
	} else
        m_EffectsInfoLoaded = false;

    //cleanup
    SafeDelete(res);
}

typeTargetGroupIDlist MEffect::GetTargetIDList(uint32 index) {
    typeTargetGroupIDlist nil;
    if (!m_EffectID or !m_EffectsInfoLoaded)
        return nil;
    std::map<uint32, typeTargetGroupIDlist>::iterator cur = m_TargetGroupIDlists.find(index);
    if (cur != m_TargetGroupIDlists.end())
        return cur->second;

    return nil;
}

// ////////////////////// DGM_Type_Effects_Table Class ////////////////////////////
TypeEffectsList::TypeEffectsList(uint32 typeID)
{
    uint32 effectID = 0, isDefault = 0;

    m_typeEffectsList.clear();

    //first get list of all effects from dgmTypeEffects table for the given typeID
    DBQueryResult* res = new DBQueryResult();
    ModuleDB::GetDgmTypeEffects(typeID, *res);

	//go through and insert each effectID into the list
    DBResultRow row;
    while (res->GetRow(row) ) {
		effectID = row.GetUInt(0);
		isDefault = row.GetUInt(1);
		m_typeEffectsList.insert(std::pair<uint32,uint32>(effectID, isDefault));
        _log(SHIP__MODULE_TRACE, "Effects List - effect %u inserted for typeID %u", effectID, typeID);
    }

    //cleanup
    SafeDelete(res);
}

TypeEffectsList::~TypeEffectsList()
{
    m_typeEffectsList.clear();
}

bool TypeEffectsList::HasEffect(uint32 effectID)
{
    if (m_typeEffectsList.find(effectID) != m_typeEffectsList.end())
        return true;

    return false;
}

void TypeEffectsList::GetEffectsList(std::map<uint32,uint32>*  effectsList)
{
    effectsList->clear();
    for (auto cur : m_typeEffectsList)
		effectsList->insert(std::pair<uint32,uint32>(cur.first, cur.second));
}


// ////////////////////// DGM_Effects_Table Class ////////////////////////////
DGM_Effects_Table::DGM_Effects_Table()
{
    m_EffectsMap.clear();
}

DGM_Effects_Table::~DGM_Effects_Table()
{
    m_EffectsMap.clear();
}

int DGM_Effects_Table::Initialize()
{
    _Populate();

    return 1;
}

void DGM_Effects_Table::_Populate()
{
    double start = GetTimeMSeconds();
    DBQueryResult* res = new DBQueryResult();
    ModuleDB::GetAllDgmEffects(*res);

    uint32 effectID = 0;
    DBResultRow row;
    while (res->GetRow(row)) {
        effectID = row.GetInt(0);
        m_EffectsMap.insert(std::pair<uint32, std::shared_ptr<MEffect>>(effectID, std::make_shared<MEffect>(effectID)));
    }

    //cleanup
    SafeDelete(res);
    sLog.Cyan("    Effects Table", "%u effects objects loaded in %.3fms", m_EffectsMap.size(), (GetTimeMSeconds() - start));
}

std::shared_ptr<MEffect> DGM_Effects_Table::GetEffect(uint32 effectID)
{
    std::map<uint32, std::shared_ptr<MEffect>>::iterator mEffectMapIterator = m_EffectsMap.find(effectID);
    if (mEffectMapIterator != m_EffectsMap.end())
        return mEffectMapIterator->second;

    return nullptr;
}


// ////////////////////// ModuleEffects Class ////////////////////////////

ModuleEffects::ModuleEffects(InventoryItem* pItem)
 :
m_pItem(pItem)
{
    m_defaultEffect = nullptr;

    m_hiPower = m_medPower = m_loPower = m_rigSlot = m_subSystem = m_warpSafe = m_targReq = false;

    m_GangEffects.clear();
    m_FleetEffects.clear();
    m_OnlineEffects.clear();
    m_ActiveEffects.clear();
    m_OverloadEffects.clear();

    _populate();

    _log(SHIP__MODULE_INFO, "ModuleEffects::ModuleEffects() - created for %s (typeID %u)", pItem->itemName().c_str(), pItem->typeID() );
}

ModuleEffects::~ModuleEffects()
{
    m_pItem = nullptr;

    m_GangEffects.clear();
    m_FleetEffects.clear();
    m_OnlineEffects.clear();
    m_ActiveEffects.clear();
    m_OverloadEffects.clear();
}

bool ModuleEffects::HasEffect(uint32 effectID)
{
    if ( m_OnlineEffects.find(effectID) != m_OnlineEffects.end() )
        return true;
    if ( m_ActiveEffects.find(effectID) != m_ActiveEffects.end() )
        return true;
    if ( m_OverloadEffects.find(effectID) != m_OverloadEffects.end() )
        return true;
    if ( m_FleetEffects.find(effectID) != m_FleetEffects.end() )
        return true;
    if ( m_GangEffects.find(effectID) != m_GangEffects.end() )
        return true;
    return false;
}

MEffect* ModuleEffects::GetEffect( uint32 effectID )
{
    std::map<uint32, std::shared_ptr<MEffect>>::const_iterator cur;
    if ( (cur = m_OnlineEffects.find(effectID)) != m_OnlineEffects.end() )
		return cur->second.get();
    if ( (cur = m_ActiveEffects.find(effectID)) != m_ActiveEffects.end() )
        return cur->second.get();
    if ( (cur = m_OverloadEffects.find(effectID)) != m_OverloadEffects.end() )
        return cur->second.get();
    if ( (cur = m_FleetEffects.find(effectID)) != m_FleetEffects.end() )
        return cur->second.get();
    if ( (cur = m_GangEffects.find(effectID)) != m_GangEffects.end() )
        return cur->second.get();
    return nullptr;
}


// ////////////////// PRIVATE MEMBERS /////////////////////////

void ModuleEffects::_populate()
{
    TypeEffectsList* myTypeEffectsListPtr = new TypeEffectsList(m_pItem->typeID());

	std::map<uint32,uint32> effectsList;
	myTypeEffectsListPtr->GetEffectsList(&effectsList);

    std::shared_ptr<MEffect> mEffectPtr = nullptr;
    m_defaultEffect = nullptr;     // Set this to NULL until the default effect is found, if there is any
    uint32 effectID = 0, groupID = m_pItem->groupID(), testID = 0, state = 0;

    //go through and find each effect, then add pointer to effect to our own map
    for (auto cur : effectsList) {
        switch (cur.first) {
            // will have to look into more modules that need a target and add as needed.
            case 10:    // target attack
            case 40:    // launcher
            case 42:    // turrent
            case 47:    // cargo scanner
            case 67:    // mining laser
            case 2255:  // tractor beam
                m_targReq = true;
                continue;
            // We do not need MEffect objects for these effectIDs, but do need slot type.
            case 11:    // loPower
                m_loPower = true;
                continue;
            case 12:    // hiPower
                m_hiPower = true;
                continue;
            case 13:    // medPower
                m_medPower = true;
                continue;
            case 3772:  // subsystem
                m_subSystem = true;
                continue;
            case 2663:  // rig - this effectID also sets calibration used on ship, so the effect is needed, if only once to avoid complicated code later.
                m_rigSlot = true;
            default: {
                effectID = cur.first;
                mEffectPtr = sDGM_Effects_Table.GetEffect(effectID);
                if (!mEffectPtr)
                    continue;
                if (!mEffectPtr->IsEffectLoaded())
                    continue;
                if (!m_warpSafe)
                    m_warpSafe = mEffectPtr->GetIsWarpSafe();

                uint32 size = mEffectPtr->GetSizeOfAttributeList();
                _log(SHIP__MODULE_TRACE, "ModuleEffects::_populate() - effectID: %u size: %u", effectID, size);
                for (int i=0; i < size; i++) {
                    if (effectID == 16)
                        testID = groupID;  //  check(hack) for "Online Effect" (#16) since targetGroup "0" means "all groups"
                    else
                        testID = mEffectPtr->GetTargetGroup(i);

                    _log(SHIP__MODULE_DEBUG, "ModuleEffects::_populate() - testing: %u %s %u", testID, (testID == groupID ? "==" : "!="), groupID);

                    // verify this effect is for current module's groupID (avoid previous clusterfuck...well, one of the many.)
                    //  or check for "all groups"
                    if ((testID != 0) and (groupID != testID))
                        continue;

                    if (cur.second)
                        m_defaultEffect = mEffectPtr.get();

                    state = mEffectPtr->GetEffectState();
                    if (state & EFFECT_UNFITTED)
                        _log(SHIP__MODULE_ERROR, "ModuleEffects::_populate() - Illegal value '%u' obtained from the 'effectAppliedInState' field of the 'dgmEffectsInfo' table", mEffectPtr->GetEffectState());
                    if (state & EFFECT_ONLINE)
                        m_OnlineEffects.insert(std::pair<uint32, std::shared_ptr<MEffect>>(effectID, mEffectPtr));
                    if (state & EFFECT_ACTIVATED)
                        m_ActiveEffects.insert(std::pair<uint32, std::shared_ptr<MEffect>>(effectID, mEffectPtr));
                    if (state & EFFECT_OVERLOADED)
                        m_OverloadEffects.insert(std::pair<uint32, std::shared_ptr<MEffect>>(effectID, mEffectPtr));
                    if (state & EFFECT_GANG)
                        m_GangEffects.insert(std::pair<uint32, std::shared_ptr<MEffect>>(effectID, mEffectPtr));
                    if (state & EFFECT_FLEET)
                        m_FleetEffects.insert(std::pair<uint32, std::shared_ptr<MEffect>>(effectID, mEffectPtr));
                }
            }
        }
    }
    // at this point, the loaded effects for this module should be ONLY for this module group.
    _log(SHIP__MODULE_INFO, "ModuleEffects::_populate() - created %u Online, %u Active, %u OverLoaded, %u Gang, and %u Fleet effects for %s (typeID %u)", \
         m_OnlineEffects.size(), m_ActiveEffects.size(), m_OverloadEffects.size(), \
         m_GangEffects.size(), m_FleetEffects.size(), m_pItem->itemName().c_str(), m_pItem->typeID() );

    SafeDelete(myTypeEffectsListPtr);
}
