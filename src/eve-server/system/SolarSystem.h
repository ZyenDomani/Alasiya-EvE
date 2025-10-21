/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
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
    Author:        Bloody.Rabbit
*/

#ifndef __SOLAR_SYSTEM__H__INCL__
#define __SOLAR_SYSTEM__H__INCL__


#include "EVEServerConfig.h"
#include "system/Celestial.h"

/**
 * CelestialObject which represents solar system.
 */
class SolarSystem
: public CelestialObject
{
    friend class InventoryItem; // to let it construct us
    friend class CelestialObject; // to let it construct us
public:
    /**
     * Loads solar system from DB.
     *
     * @param[in] factory
     * @param[in] solarSystemID ID of solar system to load.
     * @return Pointer to new solar system object; NULL if failed.
     */
    static SolarSystemRef Load( uint32 solarSystemID);

    /*
     * Public Fields:
     */
    const GPoint &      minPosition() const             { return m_minPosition; }
    const GPoint &      maxPosition() const             { return m_maxPosition; }
    float               luminosity() const              { return m_luminosity; }

    bool                border() const                  { return m_border; }
    bool                fringe() const                  { return m_fringe; }
    bool                corridor() const                { return m_corridor; }
    bool                hub() const                     { return m_hub; }
    bool                international() const           { return m_international; }
    bool                regional() const                { return m_regional; }
    bool                constellation() const           { return m_constellation; }

    float               security() const                { return m_security; }
    uint32              factionID() const               { return m_factionID; }
    int64               ssRadius() const                { return m_radius; }
    const std::string & securityClass() const           { return m_securityClass; }

    // Solar System Inventory Functions:
    void AddItemToInventory(InventoryItemRef iRef);
    void RemoveItemFromInventory(InventoryItemRef iRef);

protected:
    SolarSystem(
        uint32 _solarSystemID,
        // InventoryItem stuff:
        const ItemType &_type,
        const ItemData &_data,
        // CelestialObject stuff:
        const CelestialObjectData &_cData,
        // SolarSystem stuff:
        const SolarSystemData &_ssData
    );
    virtual ~SolarSystem();

    /*
     * Member functions:
     */
    using InventoryItem::_Load;
    virtual bool _Load();

    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem( uint32 solarSystemID, const ItemType &type, const ItemData &data) {
        if (type.groupID() != EVEDB::invGroups::Solar_System) {
            _log(ITEM__ERROR, "Trying to load %s as SolarSystem.", sDataMgr.GetCategoryName(type.categoryID()));
            if (sConfig.server.StackTrace)
                EvE::traceStack();
            return RefPtr<_Ty>(nullptr);
        }

        // load celestial data
        CelestialObjectData cData = CelestialObjectData();
        if (!SystemDB::GetCelestialObjectData(solarSystemID, cData))
            return RefPtr<_Ty>(nullptr);

        // load solar system data
        SolarSystemData ssData = SolarSystemData();
        if (!sDataMgr.GetSolarSystemData(solarSystemID, ssData))
            return RefPtr<_Ty>(nullptr);

        return SolarSystemRef( new SolarSystem(solarSystemID, type, data, cData, ssData ) );
    }

    /*
     * Data members:
     */

    /*    Border = Borders another Region or Constellation
     *    Fringe = 1 connection to this system (dead end system)
     *    Corridor = 2 connections to this system (in one side and out the other)
     *    Hub = 3+ connections to this system
     *    Regional = borders another region
     *    Constellation = borders another constellation
     *    International = always has Border/Constellation, almost always Regional
     *    Security = If it is positive, floor to nearest 1/10th gives the in-game security level. 0 or lower are 0.0 in-game.
     */

    bool m_border :1;
    bool m_fringe :1;
    bool m_corridor :1;
    bool m_hub :1;
    bool m_international :1;
    bool m_regional :1;
    bool m_constellation :1;

    uint32 m_factionID;

    int64 m_radius;

    float m_security;
    float m_luminosity;

    std::string m_securityClass;
    GPoint m_minPosition;
    GPoint m_maxPosition;
};

#endif /* !__SOLAR_SYSTEM__H__INCL__ */

