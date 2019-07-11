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
    Updates:    Allan (rewrite)
*/

#include "eve-server.h"

#include "StaticDataMgr.h"
#include "manufacturing/RamProxyDB.h"

/** @todo  go thru and update/optimize this class */

PyRep *RamProxyDB::GetJobs2(const int32 ownerID, const bool completed)
{
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        " job.jobID,"
        " job.assemblyLineID,"
        " assemblyLine.containerID,"
        " job.installedItemID,"
        " installedItem.typeID AS installedItemTypeID,"
        " installedItem.ownerID AS installedItemOwnerID,"
        " blueprint.productivityLevel AS installedItemProductivityLevel,"
        " blueprint.materialLevel AS installedItemMaterialLevel,"
        // quite ugly, but lets us use DBResultToRowset
        " IF(assemblyLine.activityID = 1, blueprintType.productTypeID, installedItem.typeID) AS outputTypeID,"
        " job.outputFlag,"
        " job.installerID,"
        " assemblyLine.activityID,"
        " job.runs,"
        " job.installTime,"
        " job.beginProductionTime,"
        " job.pauseProductionTime,"
        " job.endProductionTime,"
        " job.completedStatusID != 0 AS completed,"
        " job.licensedProductionRuns,"
        " job.installedInSolarSystemID,"
        " job.completedStatusID AS completedStatus,"
        " station.stationTypeID AS containerTypeID,"
        " station.solarSystemID AS containerLocationID"
        " FROM ramJobs AS job"
        " LEFT JOIN entity AS installedItem ON job.installedItemID = installedItem.itemID"
        " LEFT JOIN ramAssemblyLines AS assemblyLine ON job.assemblyLineID = assemblyLine.assemblyLineID"
        " LEFT JOIN invBlueprints AS blueprint ON installedItem.itemID = blueprint.itemID"
        " LEFT JOIN invBlueprintTypes AS blueprintType ON installedItem.typeID = blueprintType.blueprintTypeID"
        " LEFT JOIN ramAssemblyLineStations AS station ON assemblyLine.containerID = station.stationID"
        " WHERE job.ownerID = %u"
        " AND job.completedStatusID %s 0"
        " GROUP BY job.jobID",
        ownerID, (completed ? "!=" : "=") ))
    {
        _log(DATABASE__ERROR, "Failed to query jobs for owner %u: %s", ownerID, res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

PyRep *RamProxyDB::AssemblyLinesSelectPublic(const uint32 regionID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        " station.stationID AS containerID,"
        " station.stationTypeID AS containerTypeID,"
        " station.solarSystemID AS containerLocationID,"
        " station.assemblyLineTypeID,"
        " station.quantity,"
        " station.ownerID,"
		" types.activityID"
        " FROM ramAssemblyLineStations AS station"
        " LEFT JOIN crpNPCCorporations AS corp ON station.ownerID = corp.corporationID"
		" LEFT JOIN ramAssemblyLineTypes as types ON station.assemblyLineTypeID = types.assemblyLineTypeID"
        " WHERE station.ownerID = corp.corporationID"
        " AND station.regionID = %u",
        regionID))
    {
        _log(DATABASE__ERROR, "Failed to query public assembly lines for region %u: %s.", regionID, res.error.c_str());
        return NULL;
    }

    return DBResultToCRowset(res);
}

PyRep *RamProxyDB::AssemblyLinesSelectPersonal(const uint32 charID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        " station.stationID AS containerID,"
        " station.stationTypeID AS containerTypeID,"
        " station.solarSystemID AS containerLocationID,"
        " station.assemblyLineTypeID,"
        " station.quantity,"
        " station.ownerID"
        " FROM ramAssemblyLineStations AS station"
        " LEFT JOIN ramAssemblyLines AS line ON station.stationID = line.containerID AND station.assemblyLineTypeID = line.assemblyLineTypeID AND station.ownerID = line.ownerID"
        " WHERE station.ownerID = %u"
        " AND (line.restrictionMask & %u) = %u",
        charID, (EvERam::RestrictionMask::ByCorp | EvERam::RestrictionMask::ByAlliance), (EvERam::RestrictionMask::ByCorp | EvERam::RestrictionMask::ByAlliance)))
    {
        _log(DATABASE__ERROR, "Failed to query personal assembly lines for char %u: %s.", charID, res.error.c_str());
        return NULL;
    }

    return DBResultToCRowset(res);
}

PyRep *RamProxyDB::AssemblyLinesSelectPrivate(const uint32 charID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        " station.stationID AS containerID,"
        " station.stationTypeID AS containerTypeID,"
        " station.solarSystemID AS containerLocationID,"
        " station.assemblyLineTypeID,"
        " station.quantity,"
        " station.ownerID"
        " FROM ramAssemblyLineStations AS station"
        " LEFT JOIN ramAssemblyLines AS line ON station.stationID = line.containerID AND station.assemblyLineTypeID = line.assemblyLineTypeID AND station.ownerID = line.ownerID"
        " WHERE station.ownerID = %u",
        charID))
    {
        _log(DATABASE__ERROR, "Failed to query private assembly lines for char %u: %s.", charID, res.error.c_str());
        return NULL;
    }

    return DBResultToCRowset(res);
}

/** @todo  need to add check/query for POS assembly modules here */
PyRep *RamProxyDB::AssemblyLinesSelectCorporation(const uint32 corpID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        " station.stationID AS containerID,"
        " station.stationTypeID AS containerTypeID,"
        " station.solarSystemID AS containerLocationID,"
        " station.assemblyLineTypeID,"
        " station.quantity,"
        " station.ownerID"
        " FROM ramAssemblyLineStations AS station"
        " LEFT JOIN ramAssemblyLines AS line ON station.stationID = line.containerID AND station.assemblyLineTypeID = line.assemblyLineTypeID AND station.ownerID = line.ownerID"
        " WHERE station.ownerID = %u"
        " AND (line.restrictionMask & %u) = %u",
        corpID, EvERam::RestrictionMask::ByCorp, EvERam::RestrictionMask::ByCorp))
    {
        _log(DATABASE__ERROR, "Failed to query corporation assembly lines for corp %u: %s.", corpID, res.error.c_str());
        return NULL;
    }

    return DBResultToCRowset(res);
}

/** @todo  need to add check/query for POS assembly modules here */
PyRep *RamProxyDB::AssemblyLinesSelectAlliance(const int32 allianceID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        " station.stationID AS containerID,"
        " station.stationTypeID AS containerTypeID,"
        " station.solarSystemID AS containerLocationID,"
        " station.assemblyLineTypeID,"
        " station.quantity,"
        " station.ownerID"
        " FROM ramAssemblyLineStations AS station"
        " LEFT JOIN crpCorporation AS crp ON station.ownerID = crp.corporationID"
        " LEFT JOIN ramAssemblyLines AS line ON station.stationID = line.containerID AND station.assemblyLineTypeID = line.assemblyLineTypeID AND station.ownerID = line.ownerID"
        " WHERE crp.allianceID = %u"
        " AND (line.restrictionMask & %u) = %u",
        allianceID, EvERam::RestrictionMask::ByAlliance, EvERam::RestrictionMask::ByAlliance))
    {
        _log(DATABASE__ERROR, "Failed to query alliance assembly lines for alliance %u: %s.", allianceID, res.error.c_str());
        return NULL;
    }

    return DBResultToCRowset(res);
}

/** @todo  need to add check/query for POS assembly modules here */
PyRep *RamProxyDB::AssemblyLinesGet(const uint32 containerID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        " assemblyLineID,"
        " assemblyLineTypeID,"
        " containerID,"
        " nextFreeTime,"
        " costInstall,"
        " costPerHour,"
        " restrictionMask,"
        " discountPerGoodStandingPoint,"
        " surchargePerBadStandingPoint,"
        " minimumStanding,"
        " minimumCharSecurity,"
        " minimumCorpSecurity,"
        " maximumCharSecurity,"
        " maximumCorpSecurity"
        " FROM ramAssemblyLines"
        " WHERE containerID = %u",
        containerID)) {
        _log(DATABASE__ERROR, "Failed to query assembly lines for container %u: %s.", containerID, res.error.c_str());
        return NULL;
    }

    return DBResultToCRowset(res);
}

/** @todo  need to add check/query for POS assembly modules here */
bool RamProxyDB::GetAssemblyLineProperties(const uint32 assemblyLineID, Rsp_InstallJob& into) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        " alt.baseMaterialMultiplier,"
        " alt.baseTimeMultiplier,"
        " al.costInstall,"
        " al.costPerHour"
        " FROM ramAssemblyLines AS al"
        " LEFT JOIN ramAssemblyLineTypes AS alt ON al.assemblyLineTypeID = alt.assemblyLineTypeID"
        " WHERE al.assemblyLineID = %u",
        assemblyLineID))
    {
        _log(DATABASE__ERROR, "Failed to query properties for assembly line %u: %s.", assemblyLineID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__ERROR, "No properties found for assembly line %u.", assemblyLineID);
        return false;
    }

    into.materialMultiplier = row.GetDouble(0);
    into.timeMultiplier     = row.GetDouble(1);
    into.installCost        = row.GetDouble(2);
    into.usageCost          = row.GetDouble(3);

    return true;
}

/** @todo  need to add check/query for POS assembly modules here */
bool RamProxyDB::GetAssemblyLineVerifyProperties(const uint32 assemblyLineID, uint32 &ownerID, double &minCharSecurity, double &maxCharSecurity,
                                                 int8 &restrictionMask, int8 &activity) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        " ownerID,"
        " minimumCharSecurity,"
        " maximumCharSecurity,"
        " restrictionMask,"
        " activityID"
        " FROM ramAssemblyLines"
        " WHERE assemblyLineID = %u",
        assemblyLineID))
    {
        _log(DATABASE__ERROR, "Failed to query verify properties for assembly line %u: %s.", assemblyLineID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__ERROR, "No verify properties found for assembly line %u.", assemblyLineID);
        return false;
    }

    ownerID = row.GetUInt(0);
    minCharSecurity = row.GetDouble(1);
    maxCharSecurity = row.GetDouble(2);
    restrictionMask = row.GetInt(3);
    activity        = row.GetInt(4);

    return true;
}

bool RamProxyDB::InstallJob(const uint32 ownerID, const  uint32 installerID,
        const uint32 assemblyLineID, const uint32 installedItemID,
        const int64 beginProductionTime, const int64 endProductionTime,
        const char *description, const int32 runs, const EVEItemFlags outputFlag,
        const uint32 installedInSolarSystem, const int32 licensedProductionRuns) {
    DBerror err;

    // insert job
    if (!sDatabase.RunQuery(err,
        "INSERT INTO ramJobs"
        " (ownerID, installerID, assemblyLineID, installedItemID, installTime, beginProductionTime, endProductionTime, description, runs, outputFlag,"
        " completedStatusID, installedInSolarSystemID, licensedProductionRuns)"
        " VALUES"
        " (%u, %u, %u, %u, %f, %lli, %lli, '%s', %i, %i, 0, %u, %i)",
        ownerID, installerID, assemblyLineID, installedItemID, GetFileTimeNow(), beginProductionTime, endProductionTime, description,
        runs, (int)outputFlag, installedInSolarSystem, licensedProductionRuns))
    {
        _log(DATABASE__ERROR, "Failed to insert new job to database: %s.", err.c_str());
        return false;
    }

    // update nextFreeTime
    if (!sDatabase.RunQuery(err,
        "UPDATE ramAssemblyLines"
        " SET nextFreeTime = %" PRIi64
        " WHERE assemblyLineID = %u",
        endProductionTime, assemblyLineID))
    {
        _log(DATABASE__ERROR, "Failed to update next free time for assembly line %u: %s.", assemblyLineID, err.c_str());
        return false;
    }

    return true;
}

bool RamProxyDB::IsProducableBy(const uint32 assemblyLineID, const uint32 groupID) {
    double tmp;
    return RamProxyDB::GetMultipliers(assemblyLineID, groupID, tmp, tmp);
}

uint32 RamProxyDB::CountManufacturingJobs(const uint32 installerID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        " COUNT(job.jobID)"
        " FROM ramJobs AS job"
        " LEFT JOIN ramAssemblyLines AS line USING (assemblyLineID)"
        " WHERE job.installerID = %u"
        " AND job.completedStatusID = 0"
        " AND line.activityID = 1",
        installerID))
    {
        _log(DATABASE__ERROR, "Failed to count manufacturing jobs for installer %u.", installerID);
        return 0;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__ERROR, "No rows returned while counting manufacturing jobs for installer %u.", installerID);
        return 0;
    }

    return row.GetUInt(0);
}
uint32 RamProxyDB::CountResearchJobs(const uint32 installerID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        " COUNT(job.jobID)"
        " FROM ramJobs AS job"
        " LEFT JOIN ramAssemblyLines AS line USING (assemblyLineID)"
        " WHERE job.installerID = %u"
        " AND job.completedStatusID = 0"
        " AND line.activityID != 1",    // is this accurate?
        installerID))
    {
        _log(DATABASE__ERROR, "Failed to count research jobs for installer %u.", installerID);
        return 0;
    }

    DBResultRow row;
    if (!res.GetRow(row))
        return 0;

    return row.GetUInt(0);
}

bool RamProxyDB::GetJobProperties(const uint32 jobID, uint32& installedItemID, uint32& ownerID, EVEItemFlags& outputFlag, int32& runs, int32& licensedProductionRuns, int8& activity) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT job.installedItemID, job.ownerID, job.outputFlag, job.runs, job.licensedProductionRuns, assemblyLine.activityID"
        " FROM ramJobs AS job"
        " LEFT JOIN ramAssemblyLines AS assemblyLine USING (assemblyLineID)"
        " WHERE job.jobID = %u",
        jobID))
    {
        _log(DATABASE__ERROR, "Failed to query properties of job %u: %s.", jobID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__ERROR, "No properties found for job %u.", jobID);
        return false;
    }

    installedItemID         = row.GetUInt(0);
    ownerID                 = row.GetUInt(1);
    outputFlag              = (EVEItemFlags)row.GetUInt(2);
    runs                    = row.GetInt(3);
    licensedProductionRuns  = row.GetInt(4);
    activity                = row.GetUInt(5);

    return true;
}

bool RamProxyDB::GetJobVerifyProperties(const uint32 jobID, uint32 &ownerID, int64 &endProductionTime, int8 &restrictionMask, int8 &status) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
                "SELECT job.ownerID, job.endProductionTime, job.completedStatusID, line.restrictionMask"
                " FROM ramJobs AS job"
                " LEFT JOIN ramAssemblyLines AS line ON line.assemblyLineID = job.assemblyLineID"
                " WHERE job.jobID = %u",
                jobID))
    {
        _log(DATABASE__ERROR, "Unable to query completion properties for job %u: %s", jobID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__ERROR, "No completion properties found for job %u.", jobID);
        return false;
    }

    ownerID = row.GetUInt(0);
    endProductionTime = row.GetInt64(1);
    status = row.GetInt(2);
    restrictionMask = row.GetInt(3);

    return true;
}

bool RamProxyDB::CompleteJob(const uint32 jobID, const int8 completedStatus) {
    DBerror err;

    if (!sDatabase.RunQuery(err, "UPDATE ramJobs SET completedStatusID = %i WHERE jobID = %u", completedStatus, jobID)) {
        _log(DATABASE__ERROR, "Failed to complete job %u (completed status = %i): %s.", jobID, completedStatus, err.c_str());
        return false;
    }

    return true;
}

uint32 RamProxyDB::GetTech2Blueprint(const uint32 blueprintTypeID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res, "SELECT blueprintTypeID FROM invBlueprintTypes WHERE parentBlueprintTypeID = %u", blueprintTypeID)) {
        _log(DATABASE__ERROR, "Unable to get T2 type for type ID %u: %s", blueprintTypeID, res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (!res.GetRow(row))
        return 0;

    return row.GetUInt(0);
}

int64 RamProxyDB::GetNextFreeTime(const uint32 assemblyLineID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT nextFreeTime FROM ramAssemblyLines WHERE assemblyLineID = %u", assemblyLineID)) {
        _log(DATABASE__ERROR, "Failed to query next free time for assembly line %u: %s.", assemblyLineID, res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__ERROR, "Assembly line %u not found.", assemblyLineID);
        return 0;
    }

    return (row.IsNull(0) ? 0 : row.GetInt64(0));
}

bool RamProxyDB::GetMultipliers(const uint32 assemblyLineID, uint32 groupID, double &materialMultiplier, double &timeMultiplier) {
    DBQueryResult res;

    // check table ramAssemblyLineTypeDetailPerGroup first
    if (!sDatabase.RunQuery(res,
        "SELECT materialMultiplier, timeMultiplier"
        " FROM ramAssemblyLineTypeDetailPerGroup"
        " LEFT JOIN ramAssemblyLines USING (assemblyLineTypeID)"
        " WHERE assemblyLineID = %u"
        " AND groupID = %u",
        assemblyLineID, groupID))
    {
        _log(DATABASE__ERROR, "Failed to check producability of group %u by line %u: %s", groupID, assemblyLineID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (res.GetRow(row)) {
        materialMultiplier = row.GetDouble(0);
        timeMultiplier = row.GetDouble(1);
        return true;
    }

    //res.Reset();

    // then ramAssemblyLineTypeDetailPerCategory
    if (!sDatabase.RunQuery(res,
        "SELECT materialMultiplier, timeMultiplier"
        " FROM ramAssemblyLineTypeDetailPerCategory"
        " LEFT JOIN ramAssemblyLines USING (assemblyLineTypeID)"
        " LEFT JOIN invGroups USING (categoryID)"
        " WHERE assemblyLineID = %u"
        " AND groupID = %u",
        assemblyLineID, groupID))
    {
        _log(DATABASE__ERROR, "Failed to check producability of group %u by line %u: %s", groupID, assemblyLineID, res.error.c_str());
        return false;
    }

    if (res.GetRow(row)) {
        materialMultiplier = row.GetDouble(0);
        timeMultiplier = row.GetDouble(1);
        return true;
    }

    return false;
}

