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
    Author:        Luck
    Updates:    Allan
*/

#include "ship/modules/components/ModifyShipAttributesComponent.h"

#include "Client.h"
#include "ship/modules/GenericModule.h"


ModifyShipAttributesComponent::ModifyShipAttributesComponent(GenericModule* mod, ShipItemRef shipRef)
: m_Mod(mod), m_Ship(shipRef)
{
}

// set attributes that are not stackable here...calibration, PG, CPU, etc.
void ModifyShipAttributesComponent::ModifyNonStackingShipAttributes(uint32 targetAttrID, uint32 sourceAttrID, EVECalculationType type) {
    EvilNumber newVal = CalculateNewAttributeValue(m_Ship->GetAttribute(targetAttrID), m_Mod->GetAttribute(sourceAttrID), type);
    if (!m_Ship->SetAttribute(targetAttrID, newVal))
        sLog.Error("MSAC::SetOnlineAttributes()","Failed to set attribute %u to %f on ship %u", targetAttrID, newVal.get_float(), m_Ship->itemID());
}

void ModifyShipAttributesComponent::ModifyShipAttribute(uint32 targetAttrID, uint32 sourceAttrID, EVECalculationType type) {
    _modifyShipAttributes(m_Ship, targetAttrID, sourceAttrID, type);
}

void ModifyShipAttributesComponent::ModifyTargetShipAttribute(uint32 targetItemID, uint32 targetAttrID, uint32 sourceAttrID, EVECalculationType type ) {
    ShipItemRef target = m_Ship->GetItemFactory()->GetShip(targetItemID);
    if (target)
        _modifyShipAttributes(target, targetAttrID, sourceAttrID, type);
    else
        sLog.Error("MSAC","Failed to find target ship %u", targetItemID);
}

/* rewrote attrib calculations and implemented true stacking penality, with checks for exceptions.  -allan 13April16  */
void ModifyShipAttributesComponent::_modifyShipAttributes(ShipItemRef shipRef, uint32 targetAttrID, uint32 sourceAttrID, EVECalculationType type)
{
    EvilNumber newVal = _calculateNewValue(shipRef, targetAttrID, sourceAttrID, type, m_Mod);
    SetAttribute(shipRef, targetAttrID, newVal);
}

EvilNumber ModifyShipAttributesComponent::_calculateNewValue(ShipItemRef shipRef, uint32 targetAttrID, uint32 sourceAttrID, EVECalculationType type, GenericModule* mod)
{
    uint8 stackSize = 1;   // default.  changed later if necessary 
    double effectiveness = 1;   // default.  changed later if necessary
    EvilNumber modVal = mod->GetAttribute(sourceAttrID), startVal = shipRef->GetAttribute(targetAttrID);
    /* check for attribs that are NOT penalized here, and bypass stacking method. */
    /* note:  DCU, rigs and subsystems do not use this method */
    if ((targetAttrID != AttrWarpFactor) or (sourceAttrID != AttrCargoCapacityMultiplier)
        or (targetAttrID != AttrMiningAmount) or (targetAttrID != AttrCpuOutput)
        or (targetAttrID != AttrPowerOutput) or (targetAttrID != AttrRechargeRate)
        or (targetAttrID != AttrCapacitorCapacity) or (targetAttrID != AttrHP)
        or (targetAttrID != AttrShieldCapacity) or (targetAttrID != AttrArmorHP)
        or (targetAttrID != AttrAccessDifficulty)
        or (targetAttrID != AttrDuration)   // weapons use attrSpeed, which IS penalized.
    ) {
        std::map<uint16, uint8>::iterator itr = m_attribMap.find(targetAttrID);
        if (itr != m_attribMap.end()) {
            /** @todo   verify these module states  -enable/code passive, gang, fleet and deactivating states*/
            if (mod->GetModuleState() == MOD_ONLINE) {
                stackSize = ++itr->second;
            } else if ((mod->GetModuleState() == MOD_OFFLINE)
                        or (mod->GetModuleState() == MOD_DEACTIVATING))
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

    if (mod->GetModuleState() == MOD_ONLINE) { // set stacking penality here for reference when going offline (in above check).
        effectiveness = exp(-pow(((stackSize - 1)/2.67),2));  //stacking calculation fixed  -allan  20Dec15
        mod->SetEffectiveness(targetAttrID, effectiveness);
    } else if (mod->GetModuleState() == MOD_OFFLINE) {
        ; // not sure what to do here yet...maybe nothing, as above 'find' should get stacking penality saved when module went online
    }
    if (effectiveness <= 0) {   /* this should never happen */
        codelog(SHIP__MODULE_ERROR, "MSAC::_calculateNewValue() -  effectiveness <= 0");
        //mod->GetShipRef()->GetPilot()->SendErrorMsg("Internal Server Error.  Ref: ServerError 25610");
    }
    modVal *= effectiveness;
    EvilNumber newVal = CalculateNewAttributeValue(startVal, modVal, type);
    _log(SHIP__MODULE_TRACE, "MSAC::_calculateNewValue() -  origVal:%f, Mod:%f, newVal:%f, stackSize:%u, effective:%f, type:%i", \
                startVal.get_float(), modVal.get_float(), newVal.get_float(), stackSize, effectiveness, (int)type);

    return newVal;
}


// this method will check resist values for fuzzy logic and correct if needed.
void ModifyShipAttributesComponent::SetAttribute(ShipItemRef shipRef, uint32 targetAttrID, EvilNumber newVal)
{
    // basic check for ship resistance attrubutes (fuzzy logic range check)
    if ((targetAttrID >= AttrKineticDamageResonance) and (targetAttrID <= AttrExplosiveDamageResonance)
        or (targetAttrID == AttrEmDamageResonance)
        or ((targetAttrID >= AttrArmorEmDamageResonance) and (targetAttrID <= AttrShieldThermalDamageResonance)))
    {
        if (newVal < 0) newVal = 0;
        if (newVal > 1) newVal = 1;
    }

    //set the attribute for the ship with the new modifier
    if (!shipRef->SetAttribute(targetAttrID, newVal))
        sLog.Error("MSAC::SetOnlineAttributes()","Failed to set attribute %u to %f on ship %u", targetAttrID, newVal, m_Ship->itemID());
}



