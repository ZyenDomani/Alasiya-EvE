/**
 * @name StationDB.h
 *      database methods for station data
 *
 * @author Allan
 * @date 14 December 2017
 *
 */


#ifndef EVE_STATION_STATIONDB_H
#define EVE_STATION_STATIONDB_H

#include "ServiceDB.h"
#include "inventory/ItemType.h"


class StationDB
: public ServiceDB
{
public:
    void UpdateOfficeData(OfficeData& data);

    static PyRep* GetOffices(uint32 stationID);

    static uint32 CreateOffice(ItemData& idata, OfficeData& odata);
    static bool GetOfficeData(uint32 officeID, OfficeData& odata);
    static void GetStationData(DBQueryResult& res);
    static void GetStationSystem(DBQueryResult& res);
    static void GetStationRegion(DBQueryResult& res);
    static void GetStationOfficeData(DBQueryResult& res);
    static void GetOperationServiceIDs(DBQueryResult& res);
    static void GetStationConstellation(DBQueryResult& res);

    static uint32 GetOfficeCount(uint32 corpID);

    static void LoadOffices(OwnerData &od, std::vector<uint32> &into);

};

#endif  // EVE_STATION_STATIONDB_H
