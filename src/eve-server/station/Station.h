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
    Author:        Bloody.Rabbit
*/

#ifndef __STATION__H__INCL__
#define __STATION__H__INCL__

#include "inventory/ItemType.h"
#include "system/Celestial.h"

/**
 * Station type data container.
 */
class StationTypeData {
public:
    StationTypeData(
        uint32 _dockingBayGraphicID = 0,
        uint32 _hangarGraphicID = 0,
        const GPoint &_dockEntry = GPoint(0, 0, 0),
        const GVector &_dockOrientation = GVector(0, 0, 0),
        uint32 _operationID = 0,
        uint32 _officeSlots = 0,
        double _reprocessingEfficiency = 0.0,
        bool _conquerable = false
    );

    // Data members:
    uint32 dockingBayGraphicID;
    uint32 hangarGraphicID;

    GPoint dockEntry;
    GVector dockOrientation;

    uint32 operationID;
    uint32 officeSlots;
    double reprocessingEfficiency;
    bool conquerable;
};

/**
 * Type of station.
 */
class StationType
: public ItemType
{
    friend class ItemType; // to let it construct us
public:
    /**
     * Loads station type.
     *
     * @param[in] factory
     * @param[in] stationTypeID ID of station type to load.
     * @return Pointer to new StationType object; NULL if failed.
     */
    static StationType *Load(ItemFactory &factory, uint32 stationTypeID);

    /*
     * Access methods:
     */
    uint32      dockingBayGraphicID() const { return m_dockingBayGraphicID; }
    uint32      hangarGraphicID() const { return m_hangarGraphicID; }

    GPoint      dockEntry() const { return m_dockEntry; }
    GVector     dockOrientation() const { return m_dockOrientation; }

    uint32      operationID() const { return m_operationID; }
    uint32      officeSlots() const { return m_officeSlots; }
    double      reprocessingEfficiency() const { return m_reprocessingEfficiency; }
    bool        conquerable() const { return m_conquerable; }

protected:
    StationType(
        uint32 _id,
        // ItemType stuff:
        const ItemGroup &_group,
        const TypeData &_data,
        // StationType stuff:
        const StationTypeData &_stData
    );

    /*
     * Member functions:
     */
    using ItemType::_Load;

    // Template loader:
    template<class _Ty>
    static _Ty *_LoadType(ItemFactory &factory, uint32 stationTypeID, const ItemGroup &group, const TypeData &data)
    {
        if (group.id() != EVEDB::invGroups::Station) {
            _log( ITEM__ERROR, "Trying to load %s as Station.", group.name().c_str() );
            if (sConfig.server.StackTrace)
                EvE::traceStack();
            return nullptr;
        }

        // get station type data
        StationTypeData stData;
        if( !factory.db().GetStationType(stationTypeID, stData) )
            return nullptr;

        return _Ty::template _LoadStationType<_Ty>( factory, stationTypeID, group, data, stData );
    }

    // Actual loading stuff:
    template<class _Ty>
    static _Ty *_LoadStationType(ItemFactory &factory, uint32 stationTypeID, const ItemGroup &group, const TypeData &data, const StationTypeData &stData)
    {
        // ready to create
        return new StationType( stationTypeID, group, data, stData );
    }

    /*
     * Data members:
     */
    uint32 m_dockingBayGraphicID;
    uint32 m_hangarGraphicID;

    GPoint m_dockEntry;
    GVector m_dockOrientation;

    uint32 m_operationID;
    uint32 m_officeSlots;
    double m_reprocessingEfficiency;
    bool m_conquerable;
};

/**
 * Data container for station.
 */
class StationInfo {
public:
    StationInfo(
        uint32 _security = 0,
        double _dockingCostPerVolume = 0.0,
        double _maxShipVolumeDockable = 0.0,
        uint32 _officeRentalCost = 0,
        uint32 _operationID = 0,
        double _reprocessingEfficiency = 0.0,
        double _reprocessingStationsTake = 0.0,
        EVEItemFlags _reprocessingHangarFlag = (EVEItemFlags)0
    );

    // Data members:
    uint32 security;
    double dockingCostPerVolume;
    double maxShipVolumeDockable;
    uint32 officeRentalCost;
    uint32 operationID;

    double reprocessingEfficiency;
    double reprocessingStationsTake;
    EVEItemFlags reprocessingHangarFlag;
};

/**
 * CelestialObject which represents station.
 */
class StationItem
: public CelestialObject
{
    friend class InventoryItem; // to let it construct us
    friend class CelestialObject; // to let it construct us
protected:
    StationItem(
        ItemFactory &_factory,
        uint32 _stationID,
        // InventoryItem stuff:
        const StationType &_type,
        const ItemData &_data,
        // CelestialObject stuff:
        const CelestialObjectData &_cData,
        // Station stuff:
        const StationInfo &_stData
    );
    virtual ~StationItem()                              { /* do nothing here */ }

public:
    /**
     * Loads station.
     *
     * @param[in] factory
     * @param[in] stationID ID of station to load.
     * @return Pointer to new Station object; NULL if fails.
     */
    static StationItemRef Load(ItemFactory &factory, uint32 stationID);

    /*
     * Access methods:
     */
    uint32 security() const { return m_security; }
    double dockingCostPerVolume() const { return m_dockingCostPerVolume; }
    double maxShipVolumeDockable() const { return m_maxShipVolumeDockable; }
    uint32 officeRentalCost() const { return m_officeRentalCost; }
    uint32 operationID() const { return m_operationID; }

    double reprocessingEfficiency() const { return m_reprocessingEfficiency; }
    double reprocessingStationsTake() const { return m_reprocessingStationsTake; }
    EVEItemFlags reprocessingHangarFlag() const { return m_reprocessingHangarFlag; }

    StationType* GetStationType() { return &m_stationType; }

protected:
    /*
     * Member functions:
     */
    using InventoryItem::_Load;
    virtual bool _Load();

    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem(ItemFactory &factory, uint32 stationID, const ItemType &type, const ItemData &data)
    {
        if( type.groupID() != EVEDB::invGroups::Station )
        {
            _log( ITEM__ERROR, "Trying to load %s as Station.", type.group().name().c_str() );
            if (sConfig.server.StackTrace)
                EvE::traceStack();
            return RefPtr<_Ty>();
        }
        // cast the type
        const StationType &stType = static_cast<const StationType &>( type );

        // load celestial data
        CelestialObjectData cData;
        if (!factory.db().GetCelestialObject(stationID, cData))
            return RefPtr<_Ty>();

        // load station data
        StationInfo stData;
        if( !factory.db().GetStation( stationID, stData ) )
            return RefPtr<_Ty>();

        return _Ty::template _LoadStation<_Ty>( factory, stationID, stType, data, cData, stData );
    }

    // Actual loading stuff:
    template<class _Ty>
    static RefPtr<_Ty> _LoadStation(ItemFactory &factory, uint32 stationID, const StationType &type, const ItemData &data,
        const CelestialObjectData &cData, const StationInfo &stData)
    {
        // ready to create
        return StationItemRef( new StationItem( factory, stationID, type, data, cData, stData ) );
    }

    static uint32 CreateItemID(ItemFactory &factory, ItemData &data);

    /*
     * Data members:
     */
    StationType m_stationType;
    uint32 m_security;
    double m_dockingCostPerVolume;
    double m_maxShipVolumeDockable;
    uint32 m_officeRentalCost;
    uint32 m_operationID;

    double m_reprocessingEfficiency;
    double m_reprocessingStationsTake;
    EVEItemFlags m_reprocessingHangarFlag;
};


/**
 * StaticSystemEntity which represents Station object in space
 */
class PyServiceMgr;
class SystemManager;

class StationSE
: public StaticSystemEntity
{
public:
    StationSE(StationItemRef station, PyServiceMgr &services, SystemManager* system);
    virtual ~StationSE()                                { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual StationSE* GetStationSE()                   { return this; }
    /* Static */
    virtual bool IsStationSE()                          { return true; }

    /* virtual functions to be overridden in derived classes */
    //virtual void Process();
    virtual PyDict* MakeSlimItem();
    virtual void EncodeDestiny( Buffer& into );

    /* specific functions handled here. */
    void UnloadStation();

};

#endif /* !__STATION__H__INCL__ */


