
 /**
  * @name FactoryDB.cpp
  *   db query methods for R.A.M. activities
  *
  * @Author:         Allan
  * @date:          9Jan18
  */


#include "eve-server.h"

#include "EVEServerConfig.h"
#include "manufacturing/FactoryDB.h"

bool FactoryDB::IsProducableBy(const uint32 assemblyLineID, const uint32 groupID) {
    double tmp;
    return FactoryDB::GetMultipliers(assemblyLineID, groupID, tmp, tmp);
}

void FactoryDB::GetRAMMaterials(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT typeID, materialTypeID, quantity FROM invTypeMaterials"))
        codelog(DATABASE__ERROR, "Error in GetRAMMaterials query: %s", res.error.c_str());
}

void FactoryDB::GetRAMRequirements(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT typeID, activityID, requiredTypeID, quantity, damagePerJob, extra FROM ramTypeRequirements"))
        codelog(DATABASE__ERROR, "Error in GetRAMRequirements query: %s", res.error.c_str());
}

bool FactoryDB::DeleteBlueprint(uint32 blueprintID) {
    DBerror err;
    if(!sDatabase.RunQuery(err, "DELETE FROM invBlueprints WHERE itemID=%u", blueprintID)) {
        _log(DATABASE__ERROR, "Failed to delete blueprint %u: %s.", blueprintID, err.c_str());
        return false;
    }
    return true;
}


PyRep* FactoryDB::GetMaterialCompositionOfItemType(const uint32 typeID) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT requiredTypeID AS typeID, quantity"
        " FROM ramTypeRequirements"
        " WHERE typeID = (SELECT blueprintTypeID FROM invBlueprintTypes WHERE productTypeID = %u)"
        " AND activityID = 1 AND damagePerJob = 1",
        typeID))
    {
        _log(DATABASE__ERROR, "Could not retrieve material composition for type %u : %s", typeID, res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

bool FactoryDB::SaveBlueprintData(uint32 blueprintID, EvERam::bpData& data) {
    DBerror err;
    if(!sDatabase.RunQuery(err,
        "INSERT INTO invBlueprints"
        "  (itemID, copy, mLevel, pLevel, runs)"
        " VALUES"
        "  (%u, %u, %i, %i, %i)"
        "ON DUPLICATE KEY UPDATE "
        "mLevel=VALUES(mLevel), "
        "pLevel=VALUES(pLevel), "
        "runs=VALUES(runs) ",
        blueprintID, data.copy, data.mLevel, data.pLevel, data.runs))
    {
        codelog(DATABASE__ERROR, "Error in SaveBlueprint query: %s.", err.c_str());
        return false;
    }

    return true;
}

bool FactoryDB::GetBlueprint(uint32 blueprintID, EvERam::bpData& into) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  copy,"
        "  mLevel,"
        "  pLevel,"
        "  runs"
        " FROM invBlueprints"
        " WHERE itemID=%u",
        blueprintID))
    {
        codelog(DATABASE__ERROR, "Error in GetBlueprint query: %s.", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "Blueprint %u not found.", blueprintID);
        return false;
    }

    into.copy = row.GetBool(0);
    into.mLevel = row.GetInt(1);
    into.pLevel = row.GetInt(2);
    into.runs = row.GetInt(3);

    return true;
}

void FactoryDB::GetBlueprintType(DBQueryResult& res) {
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  blueprintTypeID,"
        "  parentBlueprintTypeID,"
        "  productTypeID,"
        "  productionTime,"
        "  techLevel,"
        "  researchProductivityTime,"
        "  researchMaterialTime,"
        "  researchCopyTime,"
        "  researchTechTime,"
        "  productivityModifier,"
        "  materialModifier,"
        "  wasteFactor,"
        "  maxProductionLimit, "
        "  chanceOfRE,"
        "  g.categoryID"
        " FROM invBlueprintTypes AS bt"
        "  LEFT JOIN invTypes AS t ON t.typeID = bt.blueprintTypeID"
        "  LEFT JOIN invGroups AS g USING (groupID)"
        " WHERE t.published = 1" ))
    {
        codelog(DATABASE__ERROR, "Error in GetBlueprintType query: %s.", res.error.c_str());
    }
}

PyRep *FactoryDB::GetJobs2(const int32 ownerID, const bool completed)
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

PyRep *FactoryDB::AssemblyLinesSelectPublic(const uint32 regionID) {
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

PyRep *FactoryDB::AssemblyLinesSelectPersonal(const uint32 charID) {
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

PyRep *FactoryDB::AssemblyLinesSelectPrivate(const uint32 charID) {
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
PyRep *FactoryDB::AssemblyLinesSelectCorporation(const uint32 corpID) {
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
PyRep *FactoryDB::AssemblyLinesSelectAlliance(const int32 allianceID) {
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
PyRep *FactoryDB::AssemblyLinesGet(const uint32 containerID) {
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
bool FactoryDB::GetAssemblyLineProperties(const uint32 assemblyLineID, Rsp_InstallJob& into) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        " alt.baseMaterialMultiplier,"
        " alt.baseTimeMultiplier,"
        " al.costInstall,"
        " al.costPerHour"
        " FROM ramAssemblyLines AS al"
        " LEFT JOIN ramAssemblyLineTypes AS alt USING (assemblyLineTypeID)"
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

    into.materialMultiplier = row.GetFloat(0);
    into.timeMultiplier     = row.GetFloat(1);
    into.installCost        = row.GetFloat(2);
    into.usageCost          = row.GetFloat(3);

    return true;
}

/** @todo  need to add check/query for POS assembly modules here */
bool FactoryDB::GetAssemblyLineVerifyProperties(const uint32 assemblyLineID, uint32 &ownerID, double &minCharSecurity, double &maxCharSecurity,
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
    minCharSecurity = row.GetFloat(1);
    maxCharSecurity = row.GetFloat(2);
    restrictionMask = row.GetInt(3);
    activity        = row.GetInt(4);

    return true;
}

bool FactoryDB::InstallJob(const uint32 ownerID, const  uint32 installerID,
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
        " SET nextFreeTime = %lli"
        " WHERE assemblyLineID = %u",
        endProductionTime, assemblyLineID))
    {
        _log(DATABASE__ERROR, "Failed to update next free time for assembly line %u: %s.", assemblyLineID, err.c_str());
        return false;
    }

    return true;
}

uint32 FactoryDB::CountManufacturingJobs(const uint32 installerID) {
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
uint32 FactoryDB::CountResearchJobs(const uint32 installerID) {
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

bool FactoryDB::GetJobProperties(const uint32 jobID, uint32& installedItemID, uint32& ownerID, EVEItemFlags& outputFlag, int32& runs, int32& licensedProductionRuns, int8& activity) {
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

bool FactoryDB::GetJobVerifyProperties(const uint32 jobID, uint32 &ownerID, int64 &endProductionTime, int8 &restrictionMask, int8 &status) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
                "SELECT job.ownerID, job.endProductionTime, job.completedStatusID, line.restrictionMask"
                " FROM ramJobs AS job"
                " LEFT JOIN ramAssemblyLines AS line USING (assemblyLineID)"
                " WHERE job.jobID = %u",
                jobID))
    {
        _log(DATABASE__ERROR, "Unable to query completion properties for job %u: %s", jobID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(MANUF__WARNING, "No completion properties found for job %u.", jobID);
        return false;
    }

    ownerID = row.GetUInt(0);
    endProductionTime = row.GetInt64(1);
    status = row.GetInt(2);
    restrictionMask = row.GetInt(3);

    return true;
}

bool FactoryDB::CompleteJob(const uint32 jobID, const int8 completedStatus) {
    DBerror err;

    if (!sDatabase.RunQuery(err, "UPDATE ramJobs SET completedStatusID = %i WHERE jobID = %u", completedStatus, jobID)) {
        _log(DATABASE__ERROR, "Failed to complete job %u (completed status = %i): %s.", jobID, completedStatus, err.c_str());
        return false;
    }

    return true;
}

uint32 FactoryDB::GetTech2Blueprint(const uint32 blueprintTypeID) {
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

int64 FactoryDB::GetNextFreeTime(const uint32 assemblyLineID) {
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

bool FactoryDB::GetMultipliers(const uint32 assemblyLineID, uint32 groupID, double &materialMultiplier, double &timeMultiplier) {
    DBQueryResult res;

    // check table ramAssemblyLineTypeDetailPerGroup first  (all materialMultiplier = 1)
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
        materialMultiplier *= row.GetDouble(0);
        timeMultiplier *= row.GetDouble(1);
        return true;
    }

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
        materialMultiplier *= row.GetDouble(0);
        timeMultiplier *= row.GetDouble(1);
        return true;
    }

    return false;
}

