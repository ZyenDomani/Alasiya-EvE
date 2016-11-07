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
    Author:     Allan
*/

#include "ship/modules/GenericModule.h"
#include "ship/modules/components/ModifyModuleAttributesComponent.h"
#include "ship/modules/components/ModifyShipAttributesComponent.h"

GenericModule::GenericModule( InventoryItemRef item, ShipItemRef ship )
{
    m_Item = item;
    m_Ship = ship;

    m_Effects = new ModuleEffects(m_Item.get());
    m_MMAC = new ModifyModuleAttributesComponent(this);
    m_MSAC = new ModifyShipAttributesComponent(this, ship);

    m_ModuleState = MOD_UNFITTED;
    m_ChargeState = MOD_UNLOADED;

    m_repeat = 0;
    // incase module item has AttrIsOnline set to true....it shouldn't but this is a catchall.
    m_Item->PutOffline();
}

GenericModule::~GenericModule()
{
    m_Item->PutOffline();
    //delete members
    SafeDelete(m_Effects);
    SafeDelete(m_MMAC);
    SafeDelete(m_MSAC);
}

/** @todo  this needs to be updated (as all module effects methods) to test for targetGroupIDs
 * GetTargetIDList() from ModuleEffects gives a vector of groupIDs each effect works on.
 * these need to be retrieved and checked against current target when module activated.
 * here, in Online(), targetGroupIDs should be checked for a value < 50, in which case this means
 * a category is the intended target (common ones are '6' for "ship" and '32' for "subsystem")
 * onlining passives should give a target group, either ship or another module (or group of modules)
 * to adjust attributes for.
 * ....this will get complicated.  -allan 12April16
 */
void GenericModule::Online()
{
    if (m_ModuleState == MOD_UNFITTED)
        return;  // make error here for online called for unfitted module?  isnt this error printed elsewhere? -nope

    if (m_ModuleState != MOD_OFFLINE)
        return;     // already online

    m_Item->PutOnline(isRig());
    m_ModuleState = MOD_ONLINE; // this must be set to online before calling msac or mmac.

    EVECalculationType ecType = CALC_NONE;
    bool stacking = false;
    typeTargetGroupIDlist targetIDs;
    uint32 targetAttrID = 0, sourceAttrID = 0, testID = 0, groupID = m_Item->groupID();
    std::map<uint32, std::shared_ptr<MEffect>>::const_iterator itr = m_Effects->GetOnlineEffectsBegin();
    _log(SHIP__MODULE_TRACE, "GenericModule::Online() -  there are %u effects to process", m_Effects->GetOnlineEffectsSize() );
    for (; itr != m_Effects->GetOnlineEffectsEnd(); itr++) {
        uint32 cur = 0, ids = itr->second->GetSizeOfAttributeList();
        _log(SHIP__MODULE_TRACE, "GenericModule::Online() -  there are %u attributes in effect %u", ids, itr->first );
        while (cur < ids) {
            if (itr->first != effectOnline) {  // effect Online.  this sets CPU and PG usage
                testID = itr->second->GetTargetGroup(cur);
                _log(SHIP__MODULE_DEBUG, "GenericModule::Online() - testing: %u %s %u", testID, (testID == groupID ? "==" : "!="), groupID);
                if ((testID != 0) && (groupID != testID)) {
                    ++cur;
                    continue;
                }
            }
            // vector<uint32> of targetgroups (or targetCategorys if id < 50)
            /* this isnt right yet....targetIDsList is vector, but i need size for it to iterate */
            stacking = itr->second->GetStackingPenalty(cur);
            targetIDs = itr->second->GetTargetIDList(cur);
            targetAttrID = itr->second->GetTargetAttributeID(cur);
            sourceAttrID = itr->second->GetSourceAttributeID(cur);
            ecType = itr->second->GetCalculationType(cur);
            _log(SHIP__MODULE_TRACE, "GenericModule::Online() - effect %u[%u] - %u targetIDs, attrib:%u, source:%u, ecType:%i", \
                        itr->first, cur, targetIDs.size(), targetAttrID, sourceAttrID, (int8)ecType);
            m_MSAC->ModifyShipAttribute(targetAttrID, sourceAttrID, ecType, stacking);
            ++cur;
            targetIDs.clear();
        }
    }
}

void GenericModule::Offline()
{
    if (m_ModuleState == MOD_OFFLINE)
        return; // make console note about offline call to offline module?  code trace, maybe?
    if (m_ModuleState == MOD_UNFITTED)
        return;  // make error here for offline called for unfitted module?  isnt this error printed elsewhere?
    if (m_ModuleState == MOD_DEACTIVATING)
        return;     // already deactivating

    m_ModuleState = MOD_DEACTIVATING;
    EVECalculationType ecType = CALC_NONE;
    bool stacking = false;
    typeTargetGroupIDlist targetIDs;
    uint32 targetAttrID = 0, sourceAttrID = 0, testID = 0, groupID = m_Item->groupID();
    std::map<uint32, std::shared_ptr<MEffect>>::const_iterator itr = m_Effects->GetOnlineEffectsBegin();
    _log(SHIP__MODULE_TRACE, "GenericModule::Offline() -  there are %u effects to process", m_Effects->GetOnlineEffectsSize() );
    for (; itr != m_Effects->GetOnlineEffectsEnd(); itr++) {
        uint32 cur = 0, ids = itr->second->GetSizeOfAttributeList();
        _log(SHIP__MODULE_TRACE, "GenericModule::Offline() -  there are %u attributes in effect %u", ids, itr->first );
        while (cur < ids) {
            if (itr->first != effectOnline) {  // effect Online.  this sets CPU and PG usage
                testID = itr->second->GetTargetGroup(cur);
                _log(SHIP__MODULE_DEBUG, "GenericModule::Offline() - testing: %u %s %u", testID, (testID == groupID ? "==" : "!="), groupID);
                if ((testID != 0) && (groupID != testID)) {
                    ++cur;
                    continue;
                }
            }
            // vector<uint32> of targetgroups (or targetCategorys if id < 50)
            /* this isnt right yet....targetIDsList is vector, but i need size for it to iterate */
            stacking = itr->second->GetStackingPenalty(cur);
            targetIDs = itr->second->GetTargetIDList(cur);
            targetAttrID = itr->second->GetTargetAttributeID(cur);
            sourceAttrID = itr->second->GetSourceAttributeID(cur);
            ecType = itr->second->GetReverseCalculationType(cur);
            _log(SHIP__MODULE_TRACE, "GenericModule::Offline() - effect %u[%u] - %u targetIDs, attrib:%u, source:%u, ecType:%i", \
                        itr->first, cur, targetIDs.size(), targetAttrID, sourceAttrID, (int8)ecType);
            m_MSAC->ModifyShipAttribute(targetAttrID, sourceAttrID, ecType, stacking);
            ++cur;
            targetIDs.clear();
        }
    }

    m_ModuleState = MOD_OFFLINE;
    m_Item->PutOffline();
}

