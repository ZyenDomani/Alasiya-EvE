/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2016 The EVEmu Team
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
 *    Author:        Allan
 */

#ifndef _EVE_SHIP_MOD_SCANNER_H_
#define _EVE_SHIP_MOD_SCANNER_H_

#include "ship/modules/ActiveModule.h"


class SurveyScanner : public ActiveModule
{
public:
    SurveyScanner(InventoryItemRef item, ShipItemRef ship);
    virtual ~SurveyScanner() { }

    /* ActiveModule overrides */
    virtual void Activate(SystemEntity* pSE);
    virtual void Deactivate();
    virtual double DoCycle();
    virtual void StopCycle(bool abort=false);

protected:
    void _ProcessCycle() {}
    void _ShowCycle();
    //double _GetDuration();
    //double _GetCapNeed();
    void _SetCapNeed();

private:
    Character* pChar;

    bool m_firstRun;
};

#endif  //_EVE_SHIP_MOD_SCANNER_H_
