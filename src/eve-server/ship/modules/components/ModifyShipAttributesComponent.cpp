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

#include "ship/modules/GenericModule.h"


ModifyShipAttributesComponent::ModifyShipAttributesComponent(GenericModule * mod, ShipRef shipRef)
: m_Mod( mod ), m_Ship( shipRef )
{

}

ModifyShipAttributesComponent::~ModifyShipAttributesComponent()
{
    //nothing to do yet
}

// set online attributes, as they are not classed in power levels, nor are stackable, and wont work right with "module-type" calculations
void ModifyShipAttributesComponent::SetOnlineAttributes(uint32 targetAttrID, uint32 sourceAttrID, EVECalculationType type)
{
    if (!m_Ship->SetAttribute(targetAttrID, CalculateNewAttributeValue(m_Ship->GetAttribute(targetAttrID), m_Mod->GetAttribute(sourceAttrID), type)))
        sLog.Error("MSAC::SetOnlineAttributes()","Failed to set attribute %u on ship %u", targetAttrID, m_Ship->itemID());
}

//modify our ship
void ModifyShipAttributesComponent::ModifyShipAttribute(uint32 targetAttrID, uint32 sourceAttrID, EVECalculationType type)
{
    _modifyShipAttributes(m_Ship, targetAttrID, sourceAttrID, type);
}

//modify target ship
void ModifyShipAttributesComponent::ModifyTargetShipAttribute(uint32 targetItemID, uint32 targetAttrID, uint32 sourceAttrID, EVECalculationType type )
{
    //find the ship
    ShipRef target = m_Ship->GetItemFactory()->GetShip(targetItemID);

    //check if we found the ship
    if (target) {
    //modify the attributes properly
    _modifyShipAttributes(target, targetAttrID, sourceAttrID, type);
    } else
        sLog.Error("MSAC","Failed to find target ship %u", targetItemID);
}


// /////////////// PRIVATE METHODS ///////////////////

// implements a rudimentary but working stacking penalty.  Currently only penalizes for stacking same item,
// but should penalize for modifying the same attribute, with some exceptions.  These exceptions are why
// it has not been implemented fully, as more data is needed and this is just a proof of concept.
// No module code will have to be changed to implement the fully functional stacking penalty
void ModifyShipAttributesComponent::_modifyShipAttributes(ShipRef shipRef, uint32 targetAttrID, uint32 sourceAttrID, EVECalculationType type)
{
    std::vector<GenericModule*> mods = m_Ship->GetStackedItems(m_Mod->typeID(), m_Mod->GetModulePowerLevel());
    _log(SHIP__MODULE_TRACE, "MSAC::_modifyShipAttributes() -  mods vector has %u item(s) for attribute: %u", mods.size(), targetAttrID);
    EvilNumber newVal = _calculateNewValue(shipRef, targetAttrID, sourceAttrID, type, mods);
    SetAttribute(shipRef, targetAttrID, newVal);
}

EvilNumber ModifyShipAttributesComponent::_calculateNewValue(ShipRef shipRef, uint32 targetAttrID, uint32 sourceAttrID, EVECalculationType type, std::vector<GenericModule*> mods)
{
    //based on http://wiki.eve-id.net/Stacking
    //EVEDev had a mistake in their formula, however I have corrected it and verified my results in excel

    std::vector<GenericModule*> sortedMods = _sortModules(targetAttrID, mods);

    EvilNumber currentVal = 0, finalVal = 0;
    //if (mods.size() > 1) {
        //first we must reset the attribute in order to properly recalculate the attribute
        //  but ONLY if there are multiple effects on this attribute.  (no clue how to do this yet)
        shipRef->ResetAttribute(targetAttrID, false);
    //}
    EvilNumber startVal = shipRef->GetAttribute(targetAttrID);  //start value
    //iterate through all the modules, largest first
    for(size_t i = 0; i < mods.size(); i++) {
        currentVal = mods[i]->GetAttribute(sourceAttrID);
        finalVal = _calculateNewAttributeValue(currentVal, startVal, type, (int)i+1 );
        startVal = finalVal; //set the starting value as the calculated value
    }

    return finalVal;
}

//calculate the new value including the stacking penalty
EvilNumber ModifyShipAttributesComponent::_calculateNewAttributeValue( EvilNumber attrMod, EvilNumber attrVal, EVECalculationType type, int stackNumber )
{
    EvilNumber effectiveness = exp(-pow(((stackNumber - 1)/2.67),2));  //fixed  -allan  20Dec15
    attrMod *= effectiveness;
    EvilNumber val = CalculateNewAttributeValue(attrVal, attrMod, type);
   // _log(SHIP__MODULE_TRACE, "MSAC::_calculateNewAttributeValue() -  attrVal: %f - attrMod: %f - newVal: %f - type: %i", \
            attrVal.get_float(), attrMod.get_float(), val.get_float(), (int)type);
    return val;
}

//sorts a vector of modules in descending order by arbitrary attribute.  That is array[0] > array[1]
std::vector<GenericModule*> ModifyShipAttributesComponent::_sortModules(uint32 sortAttrID, std::vector<GenericModule*> mods) {
    // if there is only one module, no sorting required...
    if (mods.size() < 2)
        return mods;

    //begin basic bubble sort - this needs to be checked thoroughly for bugs
    bool done = false;
    GenericModule* tmp = nullptr;

    while (!done) { //check if sorted
        done = true;  //assume sorted
        for ( int i = 0, i2 = 1; (i < mods.size()) && (i2 < mods.size()); i++, i2++)  //iterate though list
            if( mods[i]->GetAttribute(sortAttrID) > mods[i2]->GetAttribute(sortAttrID) ) {  //check if each pair is sorted
                //it's not, so flip the values
                tmp = mods[i];
                mods[i] = mods[i2];
                mods[i2] = tmp;
                done = false;  //we weren't sorted, so now go back and check if we are
            }
    }

    return mods;  //return sorted list
}

// this method will check resist values for fuzzy logic and correct if needed.
void ModifyShipAttributesComponent::SetAttribute(ShipRef shipRef, uint32 targetAttrID, EvilNumber newVal)
{
    // basic check for ship resistance attrubutes (fuzzy logic range check)
    if ((targetAttrID >= AttrKineticDamageResonance) && (targetAttrID <= AttrExplosiveDamageResonance)
        || (targetAttrID == AttrEmDamageResonance)
        || ((targetAttrID >= AttrArmorEmDamageResonance) && (targetAttrID <= AttrShieldThermalDamageResonance)))
    {
        if (newVal < 0) newVal = 0;
        if (newVal > 1) newVal = 1;
    }

    //set the attribute for the ship with the new modifier
    if (!shipRef->SetAttribute(targetAttrID, newVal))
        sLog.Error("MSAC::SetOnlineAttributes()","Failed to set attribute %u on ship %u", targetAttrID, m_Ship->itemID());
}



