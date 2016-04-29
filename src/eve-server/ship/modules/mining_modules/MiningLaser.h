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
    Author:        AknorJaden
    Updates:    Allan
*/

#ifndef __MININGLASER_H__
#define __MININGLASER_H__

#include "ship/modules/ActiveModule.h"


class MiningLaser: public ActiveModule
{
public:
    MiningLaser( InventoryItemRef item, ShipItemRef ship );
    virtual ~MiningLaser() { }

    // GenericModule Methods:
	void Activate(SystemEntity * targetEntity);
	void Deactivate();
    double DoCycle();
    void StopCycle(bool abort=false);

protected:
	void _ProcessOreCycle(bool partial = false);
    void _ProcessIceCycle(bool partial = false);
    void _ProcessCloudCycle(bool partial = false);

    void _ShowCycle();
    double _GetDuration();
    //double _GetCapNeed();
    void _SetCapNeed();

	bool m_IsInitialCycle;
    double m_cycleStartTime;
    bool m_Stop_signal;
};

#endif
