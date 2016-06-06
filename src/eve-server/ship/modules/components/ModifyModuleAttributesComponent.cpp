/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2011 The EVEmu Team
 *    For the latest information visit http://evemu.org
 *    ------------------------------------------------------------------------------------
 *    This program is free software; you can redistribute it and/or modify it under
 *    the terms of the GNU Lesser General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option) any later
 *    version.
 *
 *    This program is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License along with
 *    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 *    http://www.gnu.org/copyleft/lesser.txt.
 *    ------------------------------------------------------------------------------------
 *    Author:     Allan
 */

#include "ship/modules/components/ModifyModuleAttributesComponent.h"

#include "Client.h"
#include "ship/modules/GenericModule.h"

/** @todo  this whole class will need verification */

ModifyModuleAttributesComponent::ModifyModuleAttributesComponent(GenericModule* mod)
: m_Mod(mod)
{
}

// set attributes that are not stackable here...calibration, PG, CPU, etc.
void ModifyModuleAttributesComponent::ModifyNonStackingModuleAttributes(GenericModule* targetMod, uint32 targetAttrID, uint32 sourceAttrID, EVECalculationType type) {
    EvilNumber newVal = CalculateNewAttributeValue(targetMod->GetAttribute(targetAttrID), m_Mod->GetAttribute(sourceAttrID), type);
    if (!targetMod->getItem()->SetAttribute(targetAttrID, newVal))
        sLog.Error("MMAC::ModifyNonStackingModuleAttributes()","Failed to set attribute %u to %f on module %u", targetAttrID, newVal.get_float(), targetMod->itemID());
}

void ModifyModuleAttributesComponent::ModifyModuleAttribute(GenericModule* targetMod, uint32 targetAttrID, uint32 sourceAttrID, EVECalculationType type) {
    _modifyModuleAttributes(targetMod, targetAttrID, sourceAttrID, type);
}

/* rewrote attrib calculations and implemented true stacking penality, with checks for exceptions.  -allan 13April16  */
void ModifyModuleAttributesComponent::_modifyModuleAttributes(GenericModule* targetMod, uint32 targetAttrID, uint32 sourceAttrID, EVECalculationType type)
{
    uint8 stackSize = 1;   // default.  changed later if necessary
    double effectiveness = 1;   // default.  changed later if necessary
    EvilNumber modVal = m_Mod->GetAttribute(sourceAttrID), startVal = targetMod->GetAttribute(targetAttrID);
    /* check for attribs that are NOT penalized here, and bypass stacking method. */
    /* note:  DCU, rigs and subsystems do not use this method */
    if ((targetAttrID != AttrWarpFactor) or (sourceAttrID != AttrCargoCapacityMultiplier)) {
        std::map<uint16, uint8>::iterator itr = m_attribMap.find(targetAttrID);
        if (itr != m_attribMap.end()) {
            /** @todo   verify these module states  */
            if (m_Mod->GetModuleState() == MOD_ONLINE) {
                stackSize = ++itr->second;
            } else if ((m_Mod->GetModuleState() == MOD_OFFLINE)
                        or (m_Mod->GetModuleState() == MOD_DEACTIVATING))
                /** @todo  implement the difference between MOD_OFFLINE (not enabled) and MOD_DEACTIVATING (was online/active, told to shutdown) */
            {
                effectiveness = itr->second;
                if (itr->second == 1)
                    m_attribMap.erase(itr);
                else
                    stackSize = --itr->second;
            } else {
                ; // make error here for invalid module state?
            }
        } else
            m_attribMap.emplace(targetAttrID, 1);
    }

    if (m_Mod->GetModuleState() == MOD_ONLINE) { // set stacking penality here for reference when going offline (in above check).
        effectiveness = exp(-pow(((stackSize - 1)/2.67),2));  //stacking calculation fixed  -allan  20Dec15
        m_Mod->SetEffectiveness(targetAttrID, effectiveness);
    } else if (m_Mod->GetModuleState() == MOD_OFFLINE) {
        ; // not sure what to do here yet...maybe nothing, as above 'find' should get stacking penality saved when module went online
    }
    if (effectiveness <= 0) {   /* this should never happen */
        codelog(SHIP__MODULE_ERROR, "MMAC::_modifyModuleAttributes() -  effectiveness <= 0");
        //targetMod->GetShipRef()->GetPilot()->SendErrorMsg("Internal Server Error.  Ref: ServerError 25620");
    }
    modVal *= effectiveness;
    EvilNumber newVal = CalculateNewAttributeValue(startVal, modVal, type);
    _log(SHIP__MODULE_TRACE, "MMAC::_modifyModuleAttributes() -  origVal:%f, Mod:%f, newVal:%f, stackSize:%u, effective:%f, type:%i", \
    startVal.get_float(), modVal.get_float(), newVal.get_float(), stackSize, effectiveness, (int)type);

    SetAttribute(targetMod, targetAttrID, newVal);
}


// this method will check resist values for fuzzy logic and correct if needed.
void ModifyModuleAttributesComponent::SetAttribute(GenericModule* targetMod, uint32 targetAttrID, EvilNumber newVal)
{
    // basic check for ship resistance attrubutes (fuzzy logic range check)
    // not sure if this is needed here or not....
    if ((targetAttrID >= AttrKineticDamageResonance) and (targetAttrID <= AttrExplosiveDamageResonance)
        or (targetAttrID == AttrEmDamageResonance)
        or ((targetAttrID >= AttrArmorEmDamageResonance) and (targetAttrID <= AttrShieldThermalDamageResonance)))
    {
        if (newVal < 0) newVal = 0;
        if (newVal > 1) newVal = 1;
        sLog.Warning("MMAC::SetAttribute()","attribute %u 'newVal' was corrected", targetAttrID);
    }

    //set the modified attribute for the target module
    if (!targetMod->getItem()->SetAttribute(targetAttrID, newVal))
        sLog.Error("MMAC::SetAttribute()","Failed to set attribute %u to %f on module %u", targetAttrID, newVal, targetMod->itemID());
}



