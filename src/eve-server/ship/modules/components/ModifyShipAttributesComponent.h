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

#ifndef MODIFY_SHIP_ATTRIBUTES_COMPONENT_H
#define MODIFY_SHIP_ATTRIBUTES_COMPONENT_H


#include "ship/Ship.h"
#include "ship/modules/ModuleDefs.h"


class GenericModule;

class ModifyShipAttributesComponent
{
public:
    ModifyShipAttributesComponent(GenericModule* mod, ShipItemRef shipRef);
    ~ModifyShipAttributesComponent()                    { /* do nothing here */ }

    void ModifyShipAttribute(uint16 targetAttrID, uint16 sourceAttrID, EVECalculationType type, bool stacking);
    void ModifyTargetShipAttribute(uint32 targetItemID, uint16 targetAttrID, uint16 sourceAttrID, EVECalculationType type, bool stacking);
    void ModifyNonStackingShipAttributes(uint16 targetAttrID, uint16 sourceAttrID, EVECalculationType type);

private:

    //internal access to owner
    GenericModule* m_Mod;
    ShipItemRef m_Ship;

    void _modifyShipAttributes(ShipItemRef shipRef, uint16 targetAttrID, uint16 sourceAttrID, EVECalculationType type, bool stacking);
    EvilNumber _calculateNewValue(ShipItemRef shipRef, uint16 targetAttrID, uint16 sourceAttrID, EVECalculationType type, GenericModule* mod, bool stacking);
    void SetAttribute(ShipItemRef shipRef, uint16 targetAttrID, EvilNumber newVal);
};

#endif
