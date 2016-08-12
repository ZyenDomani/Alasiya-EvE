
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

#ifndef EVE_SHIP_MODULES_TURRENTMODULE_H
#define EVE_SHIP_MODULES_TURRENTMODULE_H

#include "ship/modules/ActiveModule.h"


class TurrentModule : public ActiveModule
{
public:
    TurrentModule(InventoryItemRef item, ShipItemRef shipRef);
    virtual ~TurrentModule()                                { /* do nothing here */ }

    //  class type helpers.  public for anyone to access.
    virtual bool IsTurrentModule()                          { return true; }

    //  functions to be handled in derived classes as needed
    virtual void LoadCharge(InventoryItemRef charge);
    virtual void UnloadCharge();
    virtual void Overload();
    virtual void DeOverload();

protected:
    //  these are  pre-calculated values to eliminate previous code calculating on EVERY CALL
    //  ship modifiers are not implemented yet.  skill modifiers are hacked here until i get skillModifierTables working
    float m_timerTime;

    double m_kinetic                                        = 0;
    double m_thermal                                        = 0;
    double m_em                                             = 0;
    double m_explosive                                      = 0;
};


#endif  // EVE_SHIP_MODULES_TURRENTMODULE_H