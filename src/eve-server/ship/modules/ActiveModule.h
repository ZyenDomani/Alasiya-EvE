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

#ifndef ACTIVE_MODULES_H
#define ACTIVE_MODULES_H

#include "ship/modules/GenericModule.h"
#include "ship/modules/components/ActiveModuleProcessingComponent.h"


class ActiveModule : public GenericModule
{
public:
    ActiveModule(InventoryItemRef item, ShipRef ship);
    virtual ~ActiveModule();

    virtual bool IsActiveModule() const                     { return true; }

	virtual void Process();
    virtual void Load(InventoryItemRef charge);
    virtual void Unload();
    virtual void Overload();
    virtual void DeOverload();
    virtual void Deactivate();
    virtual void Activate(SystemEntity* targetEntity);
    virtual bool IsOverloaded()                             { return m_overLoaded; }
    virtual InventoryItemRef GetLoadedChargeRef()           { return m_chargeRef; }

    // generic DoCycle() here is for active modules that only affect ship on Activate/Deactivate (not recurring on each cycle)
    //  for those modules that perform action on DoCycle(), they will override this call in their class implementation
    virtual double DoCycle();
    virtual void StopCycle(bool abort=false)                { /* Do nothing here */ }

    // GenericModule access function overriders
    bool ShipHasCapCharge()                                 { return (_GetCapNeed() <  m_Ship->GetAttribute(AttrCapacitorCharge).get_float()); }
    bool RequiresTarget();
    uint32 GetTargetID()                                    { return m_targetID; }
    SystemEntity* GetTarget()                               { return m_targetEntity; }

    // for modules that have charges
    bool isLoaded()                                         { return m_chargeLoaded; }

	/* common method for all modules that have a visual effect when active */
	void DoEffect(std::string effect, bool active);

protected:
    uint32 m_targetID;                                      //passed to us by activate
	SystemEntity* m_targetEntity;	                        // we do not own this
	ActiveModuleProcessingComponent* m_AMPC;    // we do not own this

	InventoryItemRef m_chargeRef;		                    // we do not own this
	bool m_overLoaded;
	bool m_chargeLoaded;

    virtual void _ProcessCycle()                            { /* Do nothing here */ }
    virtual void _ShowCycle()                               { /* Do nothing here */ }
    //  these should be overridden in derived clases to use skills and other factors as needed as this returns default attribute only.
    virtual void _SetCapNeed()                              { /* Do nothing here */ }
    virtual double _GetDuration()                           { return m_Item->GetAttribute(AttrDuration).get_float(); }
    virtual double _GetCapNeed()                            { return GetAttribute(AttrCapacitorNeed).get_float(); }
};


#endif