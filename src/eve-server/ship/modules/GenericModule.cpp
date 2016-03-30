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
#include "ship/modules/components/ModifyShipAttributesComponent.h"

GenericModule::GenericModule( InventoryItemRef item, ShipRef ship )
{
    m_Item = item;
    m_Ship = ship;

    m_Effects = new ModuleEffects(m_Item.get());
    m_ModShipAttrComp = new ModifyShipAttributesComponent(this, ship);

    m_ModuleState = MOD_UNFITTED;
    m_ChargeState = MOD_UNLOADED;
}

GenericModule::~GenericModule()
{
    //delete members
    SafeDelete(m_Effects);
    SafeDelete(m_ModShipAttrComp);
}

void GenericModule::Online()
{
    m_Item->PutOnline();

    EVECalculationType ecType = CALC_NONE;
    uint32 targetAttrID = 0, sourceAttrID = 0, testID = 0, groupID = m_Item->groupID();
    std::map<uint32, std::shared_ptr<MEffect>>::const_iterator itr = m_Effects->GetOnlineEffectsBegin();
    _log(SHIP__MODULE_TRACE, "GenericModule::Online() -  there are %u effects to process", m_Effects->GetOnlineEffectsSize() );
    for (; itr != m_Effects->GetOnlineEffectsEnd(); itr++) {
        uint32 cur = 0, ids = itr->second->GetSizeOfAttributeList();
        _log(SHIP__MODULE_TRACE, "GenericModule::Online() -  there are %u attributes in effect %u", ids, itr->first );
        while (cur < ids) {
            if (itr->first != 16) {  // effect Online.  this sets CPU and PG usage
                testID = itr->second->GetAffectingID(cur);
                if (groupID != testID) {
                    ++cur;
                    continue;
                }
            }
            targetAttrID = itr->second->GetTargetAttributeID(cur);
            sourceAttrID = itr->second->GetSourceAttributeID(cur);
            ecType = itr->second->GetCalculationType(cur);
            _log(SHIP__MODULE_TRACE, "GenericModule::Online() - effect %u[%u] - modify attr target:%u, source:%u, ecType:%i", \
                 itr->first, cur, targetAttrID, sourceAttrID, (int8)ecType);
            // now process attribute changes, with simple stacking penalities applied.
            if (itr->first == 16)   // effect Online.  this sets CPU and PG usage
                m_ModShipAttrComp->SetOnlineAttributes(targetAttrID, sourceAttrID, ecType);
            else
                m_ModShipAttrComp->ModifyShipAttribute(targetAttrID, sourceAttrID, ecType);
            ++cur;
        }
    }
}

void GenericModule::Offline()
{
    EVECalculationType ecType = CALC_NONE;
    uint32 targetAttrID = 0, sourceAttrID = 0, testID = 0, groupID = m_Item->groupID();
    std::map<uint32, std::shared_ptr<MEffect>>::const_iterator itr = m_Effects->GetOnlineEffectsBegin();
    _log(SHIP__MODULE_TRACE, "GenericModule::Offline() -  there are %u effects to process", m_Effects->GetOnlineEffectsSize() );
    for (; itr != m_Effects->GetOnlineEffectsEnd(); itr++) {
        uint32 cur = 0, ids = itr->second->GetSizeOfAttributeList();
        _log(SHIP__MODULE_TRACE, "GenericModule::Offline() -  there are %u attributes in effect %u", ids, itr->first );
        while (cur < ids) {
            if (itr->first != 16) {  // effect Offline.  this sets CPU and PG usage
                testID = itr->second->GetAffectingID(cur);
                if (groupID != testID) {
                    ++cur;
                    continue;
                }
            }
            targetAttrID = itr->second->GetTargetAttributeID(cur);
            sourceAttrID = itr->second->GetSourceAttributeID(cur);
            ecType = itr->second->GetReverseCalculationType(cur);
            _log(SHIP__MODULE_TRACE, "GenericModule::Offline() - effect %u[%u] - modify attr target:%u, source:%u, ecType:%i", \
                 itr->first, cur, targetAttrID, sourceAttrID, (int8)ecType);
            if (itr->first == 16)   // effect Online.  this sets CPU and PG usage
                m_ModShipAttrComp->SetOnlineAttributes(targetAttrID, sourceAttrID, ecType);
            else
                m_ModShipAttrComp->ModifyShipAttribute(targetAttrID, sourceAttrID, ecType);
            ++cur;
        }
    }

    m_Item->PutOffline();
}
