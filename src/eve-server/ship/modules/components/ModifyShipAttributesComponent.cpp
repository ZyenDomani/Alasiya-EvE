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
    Author:        Luck   (original code)
    Updates:    Allan     (reworked and implemented)
*/

#include "ship/modules/components/ModifyShipAttributesComponent.h"

#include "Client.h"
#include "ship/modules/GenericModule.h"


ModifyShipAttributesComponent::ModifyShipAttributesComponent(GenericModule* mod, ShipItemRef shipRef)
: m_Mod(mod), m_Ship(shipRef)
{
}

void ModifyShipAttributesComponent::ModifyShipAttribute(uint16 targetAttrID, uint16 sourceAttrID, EVECalculationType type, bool stacking) {
    _modifyShipAttributes(m_Ship, targetAttrID, sourceAttrID, type, stacking);
}

void ModifyShipAttributesComponent::ModifyTargetShipAttribute(uint32 targetItemID, uint16 targetAttrID, uint16 sourceAttrID, EVECalculationType type, bool stacking) {
    ShipItemRef target = m_Ship->GetItemFactory()->GetShip(targetItemID);
    if (target)
        _modifyShipAttributes(target, targetAttrID, sourceAttrID, type, stacking);
    else {
        _log(SHIP__ERROR, "MSAC::ModifyTargetShipAttribute() - %s(%u): Failed to find target ship %u", \
                m_Ship->itemName().c_str(), m_Ship->itemID(), targetItemID);
        if (m_Ship->HasPilot())
            m_Ship->GetPilot()->SendErrorMsg("Internal Server Error - Cannot find target.  Ref: ServerError 15623");
    }
}

/* rewrote attrib calculations and implemented true stacking penality, with checks for exceptions.  -allan 13April16  */
void ModifyShipAttributesComponent::_modifyShipAttributes(ShipItemRef shipRef, uint16 targetAttrID, uint16 sourceAttrID, EVECalculationType type, bool stacking)
{
    EvilNumber newVal = _calculateNewValue(shipRef, targetAttrID, sourceAttrID, type, m_Mod, stacking);
    SetAttribute(shipRef, targetAttrID, newVal);
}

EvilNumber ModifyShipAttributesComponent::_calculateNewValue(ShipItemRef shipRef, uint16 targetAttrID, uint16 sourceAttrID, EVECalculationType type, GenericModule* mod, bool stacking)
{
    EvilNumber modVal = mod->GetAttribute(sourceAttrID), startVal = shipRef->GetAttribute(targetAttrID);

    double effectiveness = 1;
    /* check for attribs that are NOT penalized here, and bypass stacking method. */
    if ((stacking) or (targetAttrID != AttrWarpFactor) or (sourceAttrID != AttrCargoCapacityMultiplier)
        or (targetAttrID != AttrMiningAmount) or (targetAttrID != AttrCpuOutput)
        or (targetAttrID != AttrPowerOutput) or (targetAttrID != AttrRechargeRate)
        or (targetAttrID != AttrCapacitorCapacity) or (targetAttrID != AttrHP)
        or (targetAttrID != AttrShieldCapacity) or (targetAttrID != AttrArmorHP)
        or (targetAttrID != AttrAccessDifficulty)
        or (targetAttrID != AttrDuration)) {  // weapons use attrSpeed, which IS penalized.
            effectiveness = m_Ship->GetEffectiveness(targetAttrID, mod->GetModuleState());
    }

    modVal *= effectiveness;
    EvilNumber newVal = CalculateNewAttributeValue(startVal, modVal, type);
    _log(SHIP__MODULE_TRACE, "MSAC::_calculateNewValue() -  origVal:%f, Mod:%f, newVal:%f, effective:%f, type:%i", \
                startVal.get_float(), modVal.get_float(), newVal.get_float(), effectiveness, (int)type);
    return newVal;
}

// this method will check resist values for fuzzy logic and correct if needed.
void ModifyShipAttributesComponent::SetAttribute(ShipItemRef shipRef, uint16 targetAttrID, EvilNumber newVal)
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

