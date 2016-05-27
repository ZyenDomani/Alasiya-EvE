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
    Author:        Zhur
    Updates:    Allan
*/

#ifndef __ASTEROID_H_INCL__
#define __ASTEROID_H_INCL__

#include "system/SystemEntity.h"

#if 0

CREATE TABLE sysAsteroidBelts (
    beltID INTEGER NOT NULL,
    nextGrowTime INTEGER UNSIGNED NOT NULL,
    CONSTRAINT FOREIGN KEY (beltID)
        REFERENCES mapDenormalize (`itemID`),
    PRIMARY KEY(beltID)
);

CREATE TABLE sysAsteroids
    (
    asteroidID INTEGER UNSIGNED NOT NULL auto_increment,
    locationID INTEGER NOT NULL,
    typeID INTEGER NOT NULL,
    oreVolume REAL NOT NULL,
    x REAL NOT NULL,
    y REAL NOT NULL,
    z REAL NOT NULL,
    CONSTRAINT FOREIGN KEY (locationID)
     REFERENCES `sysAsteroidBelts` (`beltID`),
    CONSTRAINT FOREIGN KEY (itemID)
     REFERENCES `entity` (`itemID`),
    PRIMARY KEY(asteroidID)
);
#endif


/**
 * ObjectSystemEntity which represents asteroid object in space
 */

class AsteroidSE
: public ObjectSystemEntity
{
public:
    AsteroidSE(InventoryItemRef self, PyServiceMgr &services, SystemManager *system);
    virtual ~AsteroidSE()                               { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual AsteroidSE* GetAsteroidSE()                 { return this; }
    /* class type tests. */
    virtual bool IsAsteroidSE()                         { return true; }

    /* SystemEntity interface */
    virtual void Process();
    virtual void EncodeDestiny( Buffer& into );
    virtual void MakeDamageState(DoDestinyDamageState &into);

    /* specific functions handled in this class. */
    void Grow();

protected:

private:
    Timer m_growTimer;

};

#endif /* !__ASTEROID__H__INCL__ */
