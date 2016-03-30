/*
 *    File:  MapConnections.cpp
 *  Author:  Allan
 * Purpose:  This file was created to hold functions specific to the mapConnections DB table.
 *      The table is used to supply map connection data to the client for drawing maps, and is not supplied by CCP.
 *      The functions in this file will gather approprate data from the DB, and populate mapConnections with the data needed.
 *
 * Created on June 29, 2014, 12:40
 * Updated on December 27, 2014, 12:01
 * Updated/Corrected cType info on July 11, 2015, 13:20
 */

#include "eve-server.h"
#include "MapConnections.h"


MapCon::MapCon()
{
}

void MapCon::PopulateConnections() {
    double starttime = GetTimeMSeconds();
    sLog.Warning("PopulateConnections()", "Populating mapConnections.");
    uint16 loops = 14335;   //14334
    uint16 count = 1;
    uint16 ctype = 3;
    uint32 fromreg = 0, fromcon = 0, fromsol = 0, stargateID = 0, celestialID = 0, tosol = 0, tocon = 0, toreg = 0;

    DBQueryResult res;
    DBResultRow row;

    while ( count < loops ) {
        sDatabase.RunQuery(res, "SELECT fromreg, fromcon, tocon, toreg FROM mapConnections WHERE AI = %u", count );

        res.GetRow(row);
        fromreg = row.GetUInt(0);
        fromcon = row.GetUInt(1);
        tocon = row.GetUInt(2);
        toreg = row.GetUInt(3);

        if (( fromreg == toreg ) && ( fromcon == tocon ))
            ctype = 2;
        else if ( fromreg == toreg )
            ctype = 1;
        else
            ctype = 3;

        DBerror err;
        sDatabase.RunQuery(err, "UPDATE mapConnections SET ctype = %u WHERE AI = %u", ctype, count);

        res.Reset();
        count++;
    }

    sLog.Success("PopulateConnections()", "mapConnections Populated in %.3f s.  Please disable this function.", (GetTimeMSeconds() -starttime));
}
