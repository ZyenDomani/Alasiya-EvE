/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2014 The EVEmu Team
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
    Author:        Allan
*/

#ifndef EVEMU_SYSTEM_WORMHOLESVC_H_
#define EVEMU_SYSTEM_WORMHOLESVC_H_

#include "PyService.h"
//#include "system/SystemDB.h"

class WormHoleSvc
: public PyService
{
public:
    WormHoleSvc(PyServiceMgr *mgr);
    virtual ~WormHoleSvc();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    //WormholeDB m_db;
    PyCallable_DECL_CALL(WormholeJump);
};

#endif  // EVEMU_SYSTEM_WORMHOLESVC_H_
/**
                wormholeClasses = {0: 'UI/Wormholes/Classes/Space',
                 1: 'UI/Wormholes/Classes/UnknownSpace',
                 2: 'UI/Wormholes/Classes/UnknownSpace',
                 3: 'UI/Wormholes/Classes/UnknownSpace',
                 4: 'UI/Wormholes/Classes/UnknownSpace',
                 5: 'UI/Wormholes/Classes/DeepUnknownSpace',
                 6: 'UI/Wormholes/Classes/DeepUnknownSpace',
                 7: 'UI/Wormholes/Classes/HighSecuritySpace',
                 8: 'UI/Wormholes/Classes/LowSecuritySpace',
                 9: 'UI/Wormholes/Classes/NullSecuritySpace'}



            if invtype.groupID == const.groupWormhole:
                desc2 = ''
                slimItem = sm.StartService('michelle').GetItem(itemID)
                if slimItem:
                    wormholeClasses = {0: 'UI/Wormholes/Classes/UnknownSpaceDescription',
                     1: 'UI/Wormholes/Classes/UnknownSpaceDescription',
                     2: 'UI/Wormholes/Classes/UnknownSpaceDescription',
                     3: 'UI/Wormholes/Classes/UnknownSpaceDescription',
                     4: 'UI/Wormholes/Classes/DangerousUnknownSpaceDescription',
                     5: 'UI/Wormholes/Classes/DangerousUnknownSpaceDescription',
                     6: 'UI/Wormholes/Classes/DeadlyUnknownSpaceDescription',
                     7: 'UI/Wormholes/Classes/HighSecuritySpaceDescription',
                     8: 'UI/Wormholes/Classes/LowSecuritySpaceDescription',
                     9: 'UI/Wormholes/Classes/NullSecuritySpaceDescription'}
                    wClass = localization.GetByLabel(wormholeClasses.get(slimItem.otherSolarSystemClass))
                    if slimItem.wormholeAge >= 3:
                        wAge = localization.GetByLabel('UI/InfoWindow/WormholeAgeAboutToClose')
                    elif slimItem.wormholeAge >= 2:
                        wAge = localization.GetByLabel('UI/InfoWindow/WormholeAgeReachingTheEnd')
                    elif slimItem.wormholeAge >= 1:
                        wAge = localization.GetByLabel('UI/InfoWindow/WormholeAgeStartedDecaying')
                    elif slimItem.wormholeAge >= 0:
                        desc2 += localization.GetByLabel('UI/InfoWindow/WormholeAgeNew') + '<br>'
                        wAge = localization.GetByLabel('UI/InfoWindow/WormholeAgeNew')
                    else:
                        wAge = ''
                    if slimItem.wormholeSize < 0.5:
                        remaining = localization.GetByLabel('UI/InfoWindow/WormholeSizeStabilityCriticallyDisrupted')
                    elif slimItem.wormholeSize < 1:
                        remaining = localization.GetByLabel('UI/InfoWindow/WormholeSizeStabilityReduced')
                    else:
                        remaining = localization.GetByLabel('UI/InfoWindow/WormholeSizeNotDisrupted')
                    desc = localization.GetByLabel('UI/InfoWindow/WormholeDescription', wormholeName=desc, wormholeClass=wClass, wormholeAge=wAge, remaining=remaining)


                    
                 */