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
    Author:     Zhur
    Impelemtation: Positron96
    Rewrite:    Allan
*/

/** @todo  go thru and update/optimize this class */

#include "eve-server.h"

#include "../../eve-common/EVE_Calendar.h"

#include "PyServiceCD.h"
#include "StatisticMgr.h"
#include "StaticDataMgr.h"
#include "account/AccountService.h"
#include "manufacturing/Blueprint.h"
#include "manufacturing/RamMethods.h"
#include "manufacturing/RamProxyService.h"
#include "station/StationDataMgr.h"
#include "system/CalendarDB.h"

PyCallable_Make_InnerDispatcher(RamProxyService)

RamProxyService::RamProxyService(PyServiceMgr *mgr)
: PyService(mgr, "ramProxy"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(RamProxyService, AssemblyLinesGet);
    PyCallable_REG_CALL(RamProxyService, AssemblyLinesSelect);
    PyCallable_REG_CALL(RamProxyService, AssemblyLinesSelectPublic);
    PyCallable_REG_CALL(RamProxyService, AssemblyLinesSelectPrivate);
    PyCallable_REG_CALL(RamProxyService, AssemblyLinesSelectCorp);
    PyCallable_REG_CALL(RamProxyService, AssemblyLinesSelectAlliance);
    PyCallable_REG_CALL(RamProxyService, GetJobs2);
    PyCallable_REG_CALL(RamProxyService, InstallJob);
    PyCallable_REG_CALL(RamProxyService, CompleteJob);
    PyCallable_REG_CALL(RamProxyService, GetRelevantCharSkills);
    PyCallable_REG_CALL(RamProxyService, UpdateAssemblyLineConfigurations);
}

RamProxyService::~RamProxyService() {
    delete m_dispatch;
}

/*
 * # Manufacturing Logging:
 * MANUF__ERROR
 * MANUF__WARNING
 * MANUF__MESSAGE
 * MANUF__INFO
 * MANUF__DEBUG
 * MANUF__TRACE
 * MANUF__DUMP
 */

PyResult RamProxyService::Handle_GetRelevantCharSkills(PyCallArgs &call) {
    return call.client->GetChar()->GetRAMSkills();
}

PyResult RamProxyService::Handle_AssemblyLinesSelectPublic(PyCallArgs &call) {
    return FactoryDB::AssemblyLinesSelectPublic(call.client->GetRegionID());
}

PyResult RamProxyService::Handle_AssemblyLinesSelectPrivate(PyCallArgs &call) {
    return FactoryDB::AssemblyLinesSelectPrivate(call.client->GetCharacterID());
}

PyResult RamProxyService::Handle_AssemblyLinesSelectCorp(PyCallArgs &call) {
    /** @todo  this needs to search db for POS arrays based on corp */
    return FactoryDB::AssemblyLinesSelectCorporation(call.client->GetCorporationID());
}

PyResult RamProxyService::Handle_AssemblyLinesSelectAlliance(PyCallArgs &call) {
    /** @todo  this needs to search db for POS arrays based on alliance */
    return FactoryDB::AssemblyLinesSelectAlliance(call.client->GetAllianceID());
}

PyResult RamProxyService::Handle_AssemblyLinesGet(PyCallArgs &call) {
    Call_SingleIntegerArg arg;  // containerID (stationID)
    if (!arg.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Unable to decode args.");
        return nullptr;
    }

    return FactoryDB::AssemblyLinesGet(arg.arg);
}

PyResult RamProxyService::Handle_AssemblyLinesSelect(PyCallArgs &call) {
    Call_AssemblyLinesSelect args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    if (args.filter == "region")
        return FactoryDB::AssemblyLinesSelectPublic(call.client->GetRegionID());
    else if (args.filter == "char")
        return FactoryDB::AssemblyLinesSelectPersonal(call.client->GetCharacterID());
    else if (args.filter == "corp")
        return FactoryDB::AssemblyLinesSelectCorporation(call.client->GetCorporationID());
    else if (args.filter == "alliance")
        return FactoryDB::AssemblyLinesSelectAlliance(call.client->GetAllianceID());

    _log(SERVICE__ERROR, "Unknown filter '%s'.", args.filter.c_str());
    return nullptr;
}

PyResult RamProxyService::Handle_GetJobs2(PyCallArgs &call) {
    Call_GetJobs2 args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    if (args.ownerID == call.client->GetCorporationID())
        if ((call.client->GetCorpRole() & Corp::Role::FactoryManager) != Corp::Role::FactoryManager) {
            // what other roles (if any) can view corp factory jobs?
            call.client->SendInfoModalMsg("You cannot view your corporation's jobs because you are not a Factory Manager.");
            return nullptr;
        }

    return FactoryDB::GetJobs2(args.ownerID, args.completed);
}

/** @todo update this for corp usage */
/** @todo  add missing/unhandled indy types (RE, invention, ??)  */
PyResult RamProxyService::Handle_InstallJob(PyCallArgs &call) {
    //_log(MANUF__DUMP, "RamProxyService::Handle_InstallJob() - size %u", call.tuple->size() );
    //call.Dump(MANUF__DUMP);

    Call_InstallJob args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    _log(MANUF__INFO, "RamProxyService::Handle_InstallJob() - %s", sRamMthd.GetActivityName(args.activityID));

    // check character job count - will throw if fail
    sRamMthd.JobsCheck(call.client->GetChar().get(), args);

    // load job Blueprint
    InventoryItemRef installedItem = sItemFactory.GetItem( args.installedItemID );
    if (installedItem.get() == nullptr) {
        // this means item/location not loaded.
        //  get data from installedItem named args and continue
        // this will require some work/rewriting

        //RamBlueprintAlreadyInstalled
        // this is InventoryItem details of the bp being installed.
        if (call.byname.find("installedItem") != call.byname.end()) {
            // use this for stations that are NOT loaded.
            /*  on remote jobs, may have to use this data to:
             * split stacks
             * send to proper places
             * verify containers
             * verify access
             * etc.
             */
            /*
             23:29:22 [ManufDump]       Args:  Dictionary: 11 entries
             23:29:22 [ManufDump]       Args:   [ 0]   Key:     String: 'categoryID'
             23:29:22 [ManufDump]       Args:   [ 0] Value:    Integer: 9
             23:29:22 [ManufDump]       Args:   [ 1]   Key:     String: 'itemID'
             23:29:22 [ManufDump]       Args:   [ 1] Value:    Integer: 140000623
             23:29:22 [ManufDump]       Args:   [ 2]   Key:     String: 'typeID'
             23:29:22 [ManufDump]       Args:   [ 2] Value:    Integer: 785
             23:29:22 [ManufDump]       Args:   [ 3]   Key:     String: 'singleton'
             23:29:22 [ManufDump]       Args:   [ 3] Value:    Integer: 1
             23:29:22 [ManufDump]       Args:   [ 4]   Key:     String: 'stacksize'
             23:29:22 [ManufDump]       Args:   [ 4] Value:    Integer: 1
             23:29:22 [ManufDump]       Args:   [ 5]   Key:     String: 'flagID'
             23:29:22 [ManufDump]       Args:   [ 5] Value:    Integer: 4
             23:29:22 [ManufDump]       Args:   [ 6]   Key:     String: 'customInfo'
             23:29:22 [ManufDump]       Args:   [ 6] Value:    WString: ''
             23:29:22 [ManufDump]       Args:   [ 7]   Key:     String: 'ownerID'
             23:29:22 [ManufDump]       Args:   [ 7] Value:    Integer: 90000000
             23:29:22 [ManufDump]       Args:   [ 8]   Key:     String: 'groupID'
             23:29:22 [ManufDump]       Args:   [ 8] Value:    Integer: 134
             23:29:22 [ManufDump]       Args:   [ 9]   Key:     String: 'locationID'
             23:29:22 [ManufDump]       Args:   [ 9] Value:    Integer: 60014140
             23:29:22 [ManufDump]       Args:   [10]   Key:     String: 'itemName'
             23:29:22 [ManufDump]       Args:   [10] Value:    WString: 'Miner I Blueprint'
             */

            PyDict* dict = call.byname["installedItem"]->AsDict();
            installedItem = sItemFactory.GetItem( PyRep::IntegerValueU32(dict->GetItemString("itemID")) );
            if (installedItem.get() == nullptr) {
                // make error here.....
                throw(PyException(MakeUserError("RamActivityRequiresABlueprint")));
            }
        }
        _log(MANUF__ERROR, "installedItem dict incomplete");
        throw(PyException(MakeUserError("Remote Job Installation Not Functional at this time.")));
        return nullptr;
    }
    if (installedItem->categoryID() != EVEDB::invCategories::Blueprint)
        throw(PyException(MakeUserError("RamActivityRequiresABlueprint")));

    // installedItem is bp.  change ref to bpRef
    BlueprintRef bpRef = BlueprintRef::StaticCast(installedItem);

    // check assy line activity
    sRamMthd.ActivityCheck(call.client, args, bpRef);

    // if output flag not set, put it where it was
    if (args.flagOutput == flagNone)
        args.flagOutput = bpRef->flag();

    // check permissions and corp roles, if applicable
    sRamMthd.LinePermissionCheck(call.client, args);
    sRamMthd.ItemPermissionCheck(call.client, args, bpRef);

    // if corp item, check location access
    if (args.isCorpJob)
        sRamMthd.LocationRolesCheck(call.client, bpRef->flag());

    // decode path to BOM location
    PathElement pathBomLocation;
    if (!pathBomLocation.Decode( args.bomPath->GetItem(0))) {
        _log(SERVICE__ERROR, "Failed to decode BOM location.");
        return nullptr;
    }

    // check bom location access
    if (args.isCorpJob)
        sRamMthd.LocationRolesCheck(call.client, pathBomLocation.flag);

    /*  the first item in bomLocationData list is list of location data, which is same for both, although the data itself is different based on many other factors
                    invLocation = [locationid, invLocationGroupID]
     *  the next item in list is a "path" which can be a list of 1 or 3 items...this is a tricky one.
     * on personal jobs, there is one arg here...a list of 2 items
                    path = [invLocation, flagInput]]
                bomLocationData = [[session.locationid, invLocationGroupID], path, []]

     * on corp jobs, there is a list of 3 items here, which are lists of the following...
                    path = []
                    path.append([quoteData.containerID, const.ownerStation, 0])
                    path.append([officeFolderID, session.corpid, const.flagHangar])
                    path.append([officeID, session.corpid, flagInput])
                    bomLocationData = [invLocation, path, []]
     */
    /*  not correct.  will need more work....
    PathElement path;
    if (!path.Decode(args.bomPath->GetItem(1))) {
        _log(SERVICE__ERROR, "Failed to decode last element of BOM location.");
        return nullptr;
    }*/
    // verify this....
    InventoryItemRef lastContItem = sItemFactory.GetItem(pathBomLocation.locationID);
    if (lastContItem.get() == nullptr)  {
        _log(MANUF__WARNING, "lastContItem null at bomLocationData.locationID %i", pathBomLocation.locationID);
        //throw(PyException(MakeUserError("RamInstalledItemWrongLocation")));
    }
    uint32 solarSystemID = lastContItem->locationID();
    if (!IsSolarSystem(solarSystemID)) {
        _log(MANUF__WARNING, "solarSystemID %u invalid for lastContItem %u at bomLocationData.locationID %i",
                solarSystemID, lastContItem->itemID(), pathBomLocation.locationID);
        //throw(PyException(MakeUserError("RamInstalledItemBadLocation")));
    }

    // calculates bp modifiers; Rsp_InstallJob is used as container while building response to job quote.
    Rsp_InstallJob rsp;
    if (!sRamMthd.Calculate(args, bpRef, call.client->GetChar().get(), rsp)){
        _log(MANUF__ERROR, "Could not Calculate() on %s for %s(%u)", bpRef->itemName().c_str(), call.client->GetName(), call.client->GetCharacterID());
        return nullptr;
    }

    // sent as assy line.nextFreeTime + 1m  (a previous client call asks for assy line nextFreeTime, displayed in window)
    if (call.byname.find("maxJobStartTime") != call.byname.end())
        if (rsp.maxJobStartTime > PyRep::IntegerValue(call.byname["maxJobStartTime"]))
            throw(PyException(MakeUserError("RamProductionTimeExceedsLimits")));

    //RamCannotGuaranteeStartTime  // timeslot taken by another char while installing this one

    // query required items for activity from static data (and not db hit)
    std::vector<EvERam::RequiredItem> reqItems;
    sDataMgr.GetRamRequiredItems(bpRef->typeID(), (int8)args.activityID, reqItems);

    // verify installer has skills and all needed materials are present in bp's location
    sRamMthd.MaterialSkillsCheck(call.client, args.runs, pathBomLocation, rsp, reqItems);

    // quoteOnly is sent for all jobs before installation to approve price and timeframe
    if (PyRep::IntegerValueU32(call.byname["quoteOnly"])) {
        _log(MANUF__INFO, "quoteOnly = true");
        sRamMthd.EncodeBillOfMaterials(reqItems, rsp.materialMultiplier, rsp.charMaterialMultiplier, args.runs, rsp.bom);
        sRamMthd.EncodeMissingMaterials(reqItems, pathBomLocation, call.client, rsp.materialMultiplier, rsp.charMaterialMultiplier, args.runs, rsp.missingMaterials);

        // this value is halved in client code. (removed in client update patch 5Nov20)
        //rsp.charTimeMultiplier *= 2;
        return rsp.Encode();
    }

    // at this point, it is a real job installation.  check everything else
    sRamMthd.ProductionTimeCheck(rsp.productionTime);

    int64 beginProductionTime = GetFileTimeNow();
    if (beginProductionTime < rsp.maxJobStartTime)
        beginProductionTime = rsp.maxJobStartTime;

    // do some activity-specific actions
    switch (args.activityID) {
        case EvERam::Activity::Manufacturing: {
            // decrease licensed production runs
            if (!bpRef->infinite())
                bpRef->UpdateRuns(-1);
        } break;
        // others are verified in ActivityCheck() above.
    }

    if (bpRef->quantity() > 1) {
        BlueprintRef iRef = bpRef->SplitBlueprint(1);
        if (iRef.get() == nullptr) {
            _log(MANUF__WARNING, "Failed to split %s for %s.", bpRef->name(), sRamMthd.GetActivityName(args.activityID));
            throw(PyException(MakeUserError("RamActivityRequiresABlueprint")));
        }
        bpRef = iRef;
    }

    uint32 locationID = bpRef->locationID();
    // change to singleton now that bp has been used (this 'unpackages' it)
    bpRef->ChangeSingleton(true, true);
    bpRef->Move(locationID, flagFactoryBlueprint, true);

    // query all items contained in "Bill of Materials" location
    std::vector<InventoryItemRef> items;
    sRamMthd.GetBOMItems( pathBomLocation, items );

    std::vector<EvERam::RequiredItem>::iterator itemItr = reqItems.begin();
    for (; itemItr != reqItems.end(); ++itemItr) {
        if (itemItr->isSkill)
            continue;       // not interested

        // calculate needed quantity
        uint32 qtyNeeded = (uint32)ceil(itemItr->quantity * rsp.materialMultiplier * args.runs);
        if (itemItr->damagePerJob == 1)
            qtyNeeded = (uint32)ceil(qtyNeeded * rsp.charMaterialMultiplier);   // skill multiplier is applied only on fully consumed materials

        // consume required materials
        /** @todo update this for corp usage.
         * need to verify char can access mat'l location
         */
        std::vector<InventoryItemRef>::iterator refItr = items.begin();
        for (; refItr != items.end(); ++refItr)
            if (((*refItr)->typeID() == itemItr->typeID) and ((*refItr)->ownerID() == call.client->GetCharacterID())) {
                if (qtyNeeded >= (*refItr)->quantity()) {
                    qtyNeeded -= (*refItr)->quantity();
                    (*refItr)->Delete();
                } else {
                    (*refItr)->AlterQuantity(-qtyNeeded);
                    break;  // we are done, stop searching
                }
            }
    }

    if (args.activityID == EvERam::Activity::Invention) {
        // im sure there is more to do here......

        /** @todo do something constructive with this data...
        // this is populated for t2 bpc
        uint16 outputType(0), baseItemType(0), decryptorType(0);
        if (call.byname.find("inventionItems") != call.byname.end()) {
            PyDict* dict = call.byname["inventionItems"]->AsDict();
            outputType = PyRep::IntegerValueU32(dict->GetItemString("outputType"));
            baseItemType = PyRep::IntegerValueU32(dict->GetItemString("baseItemType"));
            decryptorType = PyRep::IntegerValueU32(dict->GetItemString("decryptorType"));
        }
        // this is populated for t2 bpc
        if (call.byname.find("inventionOutputItemID") != call.byname.end()) {
            // this is the bp typeID to create....should we test to see if they're the same?
            outputType = PyRep::IntegerValueU32(call.byname["inventionOutputItemID"]);
        }
        */
    }

    // approved job cost from quote
    float cost(PyRep::IntegerValue(call.byname["authorizedCost"]));

    // pay for assembly lines...take the money, send wallet blink event record the transaction in journal.
    std::string reason = "DESC: Installing ";
    reason += sRamMthd.GetActivityName(args.activityID);
    reason += " job in ";
    if (IsStation(locationID))
        reason += stDataMgr.GetStationName(locationID);
    else    // test for POS after that system is more complete...
        reason += "Unknown Location";
    if (args.isCorpJob) {
        reason += " by ";
        reason += call.client->GetName();
    }
    AccountService::TranserFunds(call.client->GetCharacterID(),
                                 stDataMgr.GetOwnerID(locationID),
                                 cost,
                                 reason.c_str(),
                                 Journal::EntryType::FactorySlotRentalFee,
                                 locationID,
                                 Account::KeyType::Cash);

    // register/save job to assy line.
    // is 'description' used ??  they are all 'blah' from client
    uint32 jobID = FactoryDB::InstallJob(
                          (args.isCorpJob ? call.client->GetCorporationID() : call.client->GetCharacterID()),
                          call.client->GetCharacterID(),
                          args.installationAssemblyLineID,
                          bpRef->itemID(),
                          beginProductionTime,
                          beginProductionTime + rsp.productionTime * EvE::Time::Second,
                          args.description.c_str(),
                          args.runs,
                          (EVEItemFlags)args.flagOutput,
                          solarSystemID,
                          args.licensedProductionRuns);

    if (jobID < 1) {
        _log(MANUF__ERROR, "Could not InstallJob for %s", bpRef->itemName().c_str());
        // make client error here...
        return nullptr;
    }

    // check client settings for adding job end to calendar
    std::string title = sRamMthd.GetActivityName(args.activityID);
    if (IsStation(locationID)) {
        title += " in ";
        title += stDataMgr.GetStationName(locationID);
    }

    std::string description = "Completion of ";
    description += sRamMthd.GetActivityName(args.activityID);
    description += " job in ";
    if (IsStation(locationID))
        description += stDataMgr.GetStationName(locationID);
    else    // test for POS after that system is more complete...
        description += "Unknown Location";
    description += ".";

    if (args.isCorpJob) {
        description += "<BR>This ";
        description += sRamMthd.GetActivityName(args.activityID);
        description += " job was installed by ";
        description += call.client->GetName();
        description += " on ";
        description += currentDateTime();
        description += " server time.";
    }

    uint32 eventID = CalendarDB::SaveSystemEvent(args.isCorpJob?call.client->GetCorporationID() : call.client->GetCharacterID(),
                                stDataMgr.GetOwnerID(locationID), beginProductionTime + rsp.productionTime * EvE::Time::Second,
                                Calendar::AutoEvent::RAMJob, title, description);

    //force calendar reload (if corp job, update all online members, also)
    call.client->SendNotification("OnReloadCalendar", "charid", new PyTuple(0), true);  // this is sequenced

    FactoryDB::SetJobEventID(jobID, eventID);

    // we may need a separate table for invention jobs to store it's specific data....

    // increment statistic counter
    sStatMgr.Increment(Stat::ramJobs);
    return nullptr;
}

PyResult RamProxyService::Handle_CompleteJob(PyCallArgs &call) {
    /*
     * 23:35:54 [ManufDump] RamProxyService::Handle_CompleteJob() - size 3
     * 23:35:54 [ManufDump]   Call Arguments:
     * 23:35:54 [ManufDump]      Tuple: 3 elements
     * 23:35:54 [ManufDump]       [ 0]   List: 3 elements
     * 23:35:54 [ManufDump]       [ 0]   [ 0]   List: 2 elements
     * 23:35:54 [ManufDump]       [ 0]   [ 0]   [ 0]    Integer: 60014140   invLocation
     * 23:35:54 [ManufDump]       [ 0]   [ 0]   [ 1]    Integer: 15         invLocationGroup
     * 23:35:54 [ManufDump]       [ 0]   [ 1]   List: Empty                 path  [eve.session.locationid, eve.session.charid, const.flagHangar]
     * 23:35:54 [ManufDump]       [ 0]   [ 2]   List: 1 elements
     * 23:35:54 [ManufDump]       [ 0]   [ 2]   [ 0]    Integer: 60014140   containerID
     * 23:35:54 [ManufDump]       [ 1]    Integer: 2                        jobID
     * 23:35:54 [ManufDump]       [ 2]    Boolean: false                    cancel
     */
    _log(MANUF__DUMP, "RamProxyService::Handle_CompleteJob() - size %u", call.tuple->size() );
    call.Dump(MANUF__DUMP);

    Call_CompleteJob args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    EvERam::JobProperties data = EvERam::JobProperties();
    if (!FactoryDB::GetJobProperties(args.jobID, data))
        throw(PyException(MakeUserError("RamCompletionNoSuchJob")));

    sRamMthd.VerifyCompleteJob(args, data, call.client);

    // return item
    InventoryItemRef installedItem = sItemFactory.GetItem(data.itemID);
    if (installedItem.get() == nullptr)
        return nullptr;
    installedItem->Move(installedItem->locationID(), data.outputFlag, true);

    // does an aborted job return the installed item?
    //  is it immediate or after time expiry?
    FactoryDB::CompleteJob(args.jobID, (args.cancel ? EvERam::Status::Abort : EvERam::Status::Delivered));

    // return materials which weren't consumed
    std::vector<EvERam::RequiredItem> reqItems;
    sDataMgr.GetRamReturns(installedItem->typeID(), data.activity, reqItems);
    for (auto cur : reqItems) {
        // what about items where damage < 1.0?  (there are some...)
        uint32 quantity = (cur.quantity * data.jobRuns * (1 - cur.damagePerJob));
        if (quantity == 0)
            quantity = 1;

        ItemData idata(cur.typeID, data.ownerID, locTemp, flagNone, quantity);
        InventoryItemRef iRef = sItemFactory.SpawnItem( idata );
        if (iRef.get() != nullptr)
            iRef->Move(args.containerID, data.outputFlag, true);
    }

    if (args.cancel) {
        // what needs to be done to cancel a job?

        // if job event in calendar, set to deleted for canceled job.
        CalendarDB::DeleteEvent(data.eventID);
    } else {
        BlueprintRef bp = BlueprintRef::StaticCast( installedItem );
        switch(data.activity) {
            case EvERam::Activity::Manufacturing: {
                ItemData idata(bp->productTypeID(), data.ownerID, locTemp, flagNone, (bp->productType().portionSize() * data.jobRuns));
                InventoryItemRef iRef = sItemFactory.SpawnItem( idata );
                if (iRef.get() != nullptr)
                    iRef->Move(args.containerID, data.outputFlag, true);
            } break;
            case EvERam::Activity::ResearchTime: {
                bp->UpdatePE( data.jobRuns );
            } break;
            case EvERam::Activity::ResearchMaterial: {
                bp->UpdateME( data.jobRuns );
            } break;
            case EvERam::Activity::Copying: {
                /** @todo calculate and apply copy chance here */
                ItemData idata(installedItem->typeID(), data.ownerID, locTemp, flagNone, data.jobRuns);
                EvERam::bpData bpdata = EvERam::bpData();
                    bpdata.copy   = true;
                    bpdata.runs   = data.licensedRuns;
                    bpdata.mLevel = bp->mLevel();
                    bpdata.pLevel = bp->pLevel();
                BlueprintRef copy = Blueprint::Spawn(idata, bpdata); //BlueprintRef(nullptr);
                copy->Move(args.containerID, data.outputFlag, true);
                /*
                while (data.jobRuns) {
                    //wtf?  not sure if i like this but cant think of a better way right now...
                    // stack bpc??
                    copy = Blueprint::Spawn(idata, bpdata);
                    if (copy.get() != nullptr)
                        copy->Move(args.containerID, data.outputFlag, true);
                    --data.jobRuns;
                }
                */
            } break;
            /* todo */
            case EvERam::Activity::Invention: {
                /*  base invention data...
                 *
                 * required items:
                 * bpc of t1 item (will consume 1 run)
                 * 2 types of datacores (all consumed)
                 * T3 only: ancient relic (wrecked, malfunction, intact)
                 *
                 * optional items:
                 * decryptor  (consumed)
                 *   these modify chance, me, pe, runs on output bpc
                 *
                 * on success:  (see modifiers below)
                 * T2: ship and rig bpc have 1 run, others are 10
                 * T3: runs depend on relic (wrecked - 3, malfunction - 10, intact - 20)
                 * me -2, pe -4
                 *
                 */

    /** @todo  this needs a return for invention
     *
            result = sm.ProxySvc('ramProxy').CompleteJob(installationLocationData, jobdata.jobID, cancel)
            if hasattr(result, 'messageLabel'):
                inventionResultLabel = localization.GetByLabel(result.messageLabel)
                if result.jobCompletedSuccessfully:
                    eve.Message('RamInventionJobSucceeded', {'info': inventionResultLabel,
                     'me': result.outputME,
                     'pe': result.outputPE,
                     'runs': result.outputRuns,
                     'type': result.outputTypeID,
                     'typeid': str(result.outputTypeID),
                     'itemid': str(result.outputItemID)})
                else:
                    eve.Message('RamInventionJobFailed', {'info': inventionResultLabel})
        */
                PyDict* dict = new PyDict();
                if (1) {
                    dict->SetItemString("messageLabel", new PyString("UI/ScienceAndIndustry/ScienceAndIndustryWindow/RamInventionJobSucceeded"));
                    dict->SetItemString("jobCompletedSuccessfully", new PyBool(true));
                    dict->SetItemString("outputME", new PyInt(0));
                    dict->SetItemString("outputPE", new PyInt(0));
                    dict->SetItemString("outputRuns", new PyInt(0));
                    dict->SetItemString("outputTypeID", new PyInt(0));
                    dict->SetItemString("outputItemID", new PyInt(0));
                } else {
                    dict->SetItemString("messageLabel", new PyString("UI/ScienceAndIndustry/ScienceAndIndustryWindow/RamInventionJobFailed"));
                    dict->SetItemString("jobCompletedSuccessfully", new PyBool(false));
                }

                /*  invention base chances
                 *  0.18  freighter
                 *  0.22  bs, wrecked relic
                 *  0.26  cru, bc, barge, indy
                 *  0.30  frig, dessy, malfunction relic
                 *  0.34  module, rig, ammo, intact relic
                 *   100  perpetual motion unit?
                 */

                /* invention result outcomes:  (proposed in phoebe)
                 *
                 * success:
                 *      execeptional    - ME +2, PE +3
                 *      great           - ME +1, PE +2
                 *      good            - PE +1
                 *      standard        - no modifier
                 *
                 * failure:
                 *      standard        - return 50% datacores
                 *      poor            - return 25% datacores
                 *      terrible        - return 10% datacores
                 *      critical        - return no datacores
                 *
                 */

                // not sure the semantics on this...its' on both succeed and fail
                // this *could* be the part about your skills
                /*
(251331, `This job was so easy you feel you could do it again in your sleep.`)
(251332, `You have a good feeling this job is perfectly suited to someone of your talents.`)
(251333, `Completing this job was fairly comfortable for you and didn't tax your talents too much.`)
(251334, `You're happy with your success, for succeeding this job was far from certain.`)
(251335, `You succeeded, but you have a nagging feeling that you can't count on it every time.`)
(251336, `Despite valiant efforts you failed the job with success at your fingertips.`)
(251338, `You've got a good feel for this job, even if nothing of value came out of it this time.`)
(251339, `Although you have a firm understanding of the basics of this job you were never close to a solution.`)
(251340, `This is far from an impossible job, but one that might require a few tries before succeeding.`)
(251341, `This job requires lot of diligence and hard work on your part if you want to succeed.`)
(251342, `You feel a bit out of your league, succeeding this job requires a fair bit of luck.`)
(251343, `You never saw the light, this job is almost impossible for you to complete.`)

{'FullPath': u'UI/ScienceAndIndustry/Invention', 'messageID': 251331, 'label': u'InventionResultSuccessCouldDoInSleep'}(u'This job was so easy you feel you could do it again in your sleep.', None, None)
{'FullPath': u'UI/ScienceAndIndustry/Invention', 'messageID': 251332, 'label': u'InventionResultSuccessSuitedToYourTalets'}(u'You have a good feeling this job is perfectly suited to someone of your talents.', None, None)
{'FullPath': u'UI/ScienceAndIndustry/Invention', 'messageID': 251333, 'label': u'InventionResultSuccessDidntTaxTooMuch'}(u"Completing this job was fairly comfortable for you and didn't tax your talents too much.", None, None)
{'FullPath': u'UI/ScienceAndIndustry/Invention', 'messageID': 251334, 'label': u'InventionResultSuccessFarFromCertain'}(u"You're happy with your success, for succeeding this job was far from certain.", None, None)
{'FullPath': u'UI/ScienceAndIndustry/Invention', 'messageID': 251335, 'label': u'InventionResultSuccessCantCountOnIt'}(u"You succeeded, but you have a nagging feeling that you can't count on it every time.", None, None)
{'FullPath': u'UI/ScienceAndIndustry/Invention', 'messageID': 251336, 'label': u'InventionResultFailedSuccessAtFingertips'}(u'Despite valiant efforts you failed the job with success at your fingertips.', None, None)
{'FullPath': u'UI/ScienceAndIndustry/Invention', 'messageID': 251338, 'label': u'InventionResultFailedFeltGoodAboutIt'}(u"You've got a good feel for this job, even if nothing of value came out of it this time.", None, None)
{'FullPath': u'UI/ScienceAndIndustry/Invention', 'messageID': 251339, 'label': u'InventionResultFailedNeverClose'}(u'Although you have a firm understanding of the basics of this job you were never close to a solution.', None, None)
{'FullPath': u'UI/ScienceAndIndustry/Invention', 'messageID': 251340, 'label': u'InventionResultFailedRequiresAFewTries'}(u'This is far from an impossible job, but one that might require a few tries before succeeding.', None, None)
{'FullPath': u'UI/ScienceAndIndustry/Invention', 'messageID': 251341, 'label': u'InventionResultFailedRequiresHardWork'}(u'This job requires lot of diligence and hard work on your part if you want to succeed.', None, None)
{'FullPath': u'UI/ScienceAndIndustry/Invention', 'messageID': 251342, 'label': u'InventionResultFailedRequiresLuck'}(u'You feel a bit out of your league, succeeding this job requires a fair bit of luck.', None, None)
{'FullPath': u'UI/ScienceAndIndustry/Invention', 'messageID': 251343, 'label': u'InventionResultFailedImpossible'}(u'You never saw the light, this job is almost impossible for you to complete.', None, None)
{
*/
                /* {'messageKey': 'ProductionFailure', 'dataID': 17875197, 'suppressable': False, 'bodyID': 256420, 'messageType': 'hint', 'urlAudio': '', 'urlIcon': '', 'titleID': 256419, 'messageID': 3741}
                 * {'messageKey': 'ProductionDuplicationFailure', 'dataID': 17875202, 'suppressable': False, 'bodyID': 256422, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 256421, 'messageID': 3739}
                 * {'messageKey': 'ProductionInventionFailure', 'dataID': 17875207, 'suppressable': False, 'bodyID': 256424, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 256423, 'messageID': 3740}
                 *
                 * {'FullPath': u'UI/Messages', 'messageID': 256419, 'label': u'ProductionFailureTitle'}(u'Production Job Will Fail', None, None)
                 * {'FullPath': u'UI/Messages', 'messageID': 256420, 'label': u'ProductionFailureBody'}(u'There is no chance of that action succeeding.', None, None)
                 * {'FullPath': u'UI/Messages', 'messageID': 256421, 'label': u'ProductionDuplicationFailureTitle'}(u'Production Job Will Fail', None, None)
                 * {'FullPath': u'UI/Messages', 'messageID': 256422, 'label': u'ProductionDuplicationFailureBody'}(u'"{[item]type.name}" can not be duplicated, there is no chance of it succeeding.', None, {u'{[item]type.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'type'}})
                 * {'FullPath': u'UI/Messages', 'messageID': 256423, 'label': u'ProductionInventionFailureTitle'}(u'Production Job Will Fail', None, None)
                 * {'FullPath': u'UI/Messages', 'messageID': 256424, 'label': u'ProductionInventionFailureBody'}(u'"{[item]type.name}" can not be invented, there is no chance of it succeeding.', None, {u'{[item]type.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'type'}})
                 *
                 */
                /*
            if hasattr(result, 'message'):
                eve.Message(result.message.msg, result.message.args)
                */
                PyDict* msg = new PyDict();
                    msg->SetItemString("msg", new PyInt(0));
                    msg->SetItemString("args", new PyInt(0));
                dict->SetItemString("message", msg);
                return dict;
            } break;
            case EvERam::Activity::ReverseEngineering:
            /* unsupported */
            case EvERam::Activity::ResearchTech:
            case EvERam::Activity::Duplicating:
            default: {
                _log(MANUF__WARNING, "Activity %u is currently unsupported.", data.activity);
                throw(PyException(MakeUserError("RamActivityInvalid")));
            } break;
        }
    }

    // there is more to this.  also could be not needed, as it checks for 'none'
    // result.message.msg = "event";
    // result.message.args = ??
    return PyStatic.NewNone();
}

PyResult RamProxyService::Handle_UpdateAssemblyLineConfigurations(PyCallArgs &call) {
    _log(MANUF__DUMP, "RamProxyService::Handle_UpdateAssemblyLineConfigurations() - size %u", call.tuple->size() );
    call.Dump(MANUF__DUMP);

    //RamConfigAssemblyLinesAccessDenied
    //RamConfigAssemblyLinesInsuficientAccess

    return nullptr;
}
