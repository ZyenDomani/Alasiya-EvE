
 /**
  * @name RamMethods.cpp
  *   methods for R.A.M. activities
  *
  * @Author:         Allan
  * @date:          9Jan18
  */

#include "PyCallable.h"

#include "Client.h"
#include "manufacturing/Blueprint.h"
#include "manufacturing/RamMethods.h"
#include "station/StationDataMgr.h"

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

static const uint32 ramProductionTimeLimit = 60*60*24*30;   //30 days

/*
 *    UNKNOWN/NOT IMPLEMENTED EXCEPTIONS:
 ************************************
 *    RamRemoteInstalledItemImpounded             - impound of installedItem
 *    RamInstallJob_InstalledItemChanged          - some cache expiration??
 *    RamInstalledItemMustBeInInstallation        - where to use?
 *    RamStationIsNotConstructed                  - station building not implemented
 *    RamAccessDeniedWrongAlliance                - alliances not implemented
 */


void RamMethods::ActivityCheck(Client* const pClient, const Call_InstallJob& args, InventoryItemRef installedItem)
{
    const ItemType* pType(nullptr);
    BlueprintRef bRef = BlueprintRef::StaticCast( installedItem );
    if (bRef.get() == nullptr)
        return;     // make error here

    switch(args.activityID) {
        case EvERam::Activity::Manufacturing: {
            if (!bRef->infinite() and (bRef->runsRemaining() - args.runs) < 0)
                throw(PyException(MakeUserError("RamTooManyProductionRuns")));
            pType = &bRef->productType();
        } break;
        case EvERam::Activity::ResearchMaterial:
        case EvERam::Activity::ResearchTime: {
            if (bRef->copy())
                throw(PyException(MakeUserError("RamCannotResearchABlueprintCopy")));
            pType = &bRef->type();
        } break;
        case EvERam::Activity::Copying: {
            if (bRef->copy())
                throw(PyException(MakeUserError("RamCannotCopyABlueprintCopy")));
            pType = &bRef->type();
        } break;
        case EvERam::Activity::Invention: {
            if (!bRef->copy())
                throw(PyException(MakeUserError("RamCannotInventABlueprintOriginal")));

            uint32 pTypeID = RamProxyDB::GetTech2Blueprint(installedItem->typeID());
            if (pTypeID == 0)
                throw(PyException(MakeUserError("RamInventionNoOutput")));

            pType = &bRef->productType();
        }  break;
        case EvERam::Activity::ReverseEngineering:  // RE is ONLY at expermential POS module...cannot do RE in stations.  right now, this will never hit.
        case EvERam::Activity::ResearchTech:    // cannot find any reference to this.  not used?
        case EvERam::Activity::Duplicating:     // ancient pre-apoc 'copy' activity.  no longer used.
        default: {
            // not supported
            sLog.Error("RAM::InstallJob()", "Unsupported Activity %u sent by %s(%u).", args.activityID, pClient->GetName(), pClient->GetCharacterID());
            throw(PyException(MakeUserError("RamActivityInvalid")));
            //throw(PyException(MakeUserError("RamNoKnownOutputType")));
        }
    }

    if (!RamProxyDB::IsProducableBy(args.installationAssemblyLineID, pType->groupID()))
        throw(PyException(MakeUserError("RamBadEndProductForActivity")));
}

void RamMethods::JobsCheck(Character* pChar, const Call_InstallJob& args)
{
    if (args.activityID == EvERam::Activity::Manufacturing) {
        uint32 jobCount = RamProxyDB::CountManufacturingJobs(pChar->itemID());
        uint charMaxJobs = pChar->GetAttribute(AttrManufactureSlotLimit).get_int()
                            + pChar->GetSkillLevel(EvESkill::MassProduction)
                            + pChar->GetSkillLevel(EvESkill::AdvancedMassProduction);

        if (charMaxJobs <= jobCount) {
            std::map<std::string, PyRep *> exceptArgs;
            exceptArgs["current"] = new PyInt(jobCount);
            exceptArgs["max"] = new PyInt(charMaxJobs);
            throw(PyException(MakeUserError("MaxFactorySlotUsageReached", exceptArgs)));
        }
    } else {
        uint charMaxJobs = pChar->GetAttribute(AttrMaxLaborotorySlots).get_int()
                            + pChar->GetSkillLevel(EvESkill::LaboratoryOperation)
                            + pChar->GetSkillLevel(EvESkill::AdvancedLaboratoryOperation);

        uint32 jobCount = RamProxyDB::CountResearchJobs(pChar->itemID());
        if (charMaxJobs <= jobCount) {
            std::map<std::string, PyRep *> exceptArgs;
            exceptArgs["current"] = new PyInt(jobCount);
            exceptArgs["max"] = new PyInt(charMaxJobs);
            throw(PyException(MakeUserError("MaxResearchFacilitySlotUsageReached", exceptArgs)));
        }
    }
}

void RamMethods::InstallationCheck(Client*const pClient, int32 installationContainerID)
{
    uint32 regionID = sDataMgr.GetStationRegion(installationContainerID);
    if (!IsRegion(regionID))
        throw(PyException(MakeUserError("RamIsNotAnInstallation")));

    if (pClient->GetRegionID() != regionID)
        throw(PyException(MakeUserError("RamRangeLimitationRegion")));

    // RamStructureNotInSpace
    // RamStructureNotIsSolarsystem
    // RamRangeLimitation
    // RamRangeLimitationJumps
    // RamRangeLimitationJumpsNoSkill

    /*
     *        jumpsPerSkillLevel = {0: -1,
     *         1: 0,
     *         2: 5,
     *         3: 10,
     *         4: 20,
     *         5: 50}
     */
}

void RamMethods::AssemblyLineCheck(Client*const pClient, const Call_InstallJob& args)
{
    uint32 ownerID = 0;
    double minCharSec = 0, maxCharSec = 0;
    int8 restrictionMask, activity;

    // get properties
    if (!RamProxyDB::GetAssemblyLineVerifyProperties(args.installationAssemblyLineID, ownerID, minCharSec, maxCharSec, restrictionMask, activity))
        throw(PyException(MakeUserError("RamInstallationHasNoDefaultContent")));

    // check validity of activity
    if (activity < EvERam::Activity::Manufacturing or activity > EvERam::Activity::Invention)
        throw(PyException(MakeUserError("RamAssemblyLineHasNoActivity")));

    // check security rating if required
    if ((restrictionMask & EvERam::RestrictionMask::BySecurity) == EvERam::RestrictionMask::BySecurity) {
        if (minCharSec > pClient->GetSecurityRating())
            throw(PyException(MakeUserError("RamAccessDeniedSecStatusTooLow")));

        if (maxCharSec < pClient->GetSecurityRating())
            throw(PyException(MakeUserError("RamAccessDeniedSecStatusTooHigh")));

        // RamAccessDeniedCorpSecStatusTooHigh
        // RamAccessDeniedCorpSecStatusTooLow
    }

    // check standing if required
    if ((restrictionMask & EvERam::RestrictionMask::ByStanding) == EvERam::RestrictionMask::ByStanding) {
        // RamAccessDeniedCorpStandingTooLow
        // RamAccessDeniedStandingTooLow
    }
    if ((restrictionMask & EvERam::RestrictionMask::ByAlliance) == EvERam::RestrictionMask::ByAlliance) {
        if (ownerID != pClient->GetAllianceID())
            throw(PyException(MakeUserError("RamAccessDeniedWrongAlliance")));
    }
    if ((restrictionMask & EvERam::RestrictionMask::ByCorp) == EvERam::RestrictionMask::ByCorp) {
        if (ownerID != pClient->GetCorporationID())
            throw(PyException(MakeUserError("RamAccessDeniedWrongCorp")));
    }

    //RamCannotInstallForCorpByRole     -- You cannot install this job for your corporation as you do not have the correct role to do so

    if (args.isCorpJob) {
        int64 roles = pClient->GetCorpRole();
        if ((roles & Corp::Role::FactoryManager) != Corp::Role::FactoryManager)
            throw(PyException(MakeUserError("RamCannotInstallForCorpByRoleFactoryManager")));
        if (args.activityID == EvERam::Activity::Manufacturing) {
            if ((roles & Corp::Role::CanRentFactorySlot) != Corp::Role::CanRentFactorySlot)
                throw(PyException(MakeUserError("RamCannotInstallWithoutRentFactorySlot")));
        } else {
            if ((roles & Corp::Role::CanRentResearchSlot) != Corp::Role::CanRentResearchSlot)
                throw(PyException(MakeUserError("RamCannotInstallWithoutRentResearchSlot")));
        }
    }
}

void RamMethods::ItemPermissionCheck(Client*const pClient, const Call_InstallJob& args, InventoryItemRef installedItem)
{
        // ownership
        if (args.isCorpJob) {
            if (installedItem->ownerID() != pClient->GetCorporationID())
                throw(PyException(MakeUserError("RamCannotInstallItemForAnotherCorp")));
        } else {
            if (installedItem->ownerID() != pClient->GetCharacterID())
                throw(PyException(MakeUserError("RamCannotInstallItemForAnother")));
        }
}

void RamMethods::ItemLocationCheck(Client*const pClient, const Call_InstallJob& args, InventoryItemRef installedItem)
{
    if (IsStation(args.installationContainerID)) {
        if (installedItem->locationID() != args.installationContainerID) {
            if (args.installationContainerID == pClient->GetLocationID()) {
                std::map<std::string, PyRep *> exceptArgs;
                exceptArgs["location"] = new PyString(stDataMgr.GetStationName(args.installationContainerID));
                if (args.isCorpJob)
                    throw(PyException(MakeUserError("RamCorpInstalledItemWrongLocation", exceptArgs)));
                else
                    throw(PyException(MakeUserError("RamInstalledItemWrongLocation", exceptArgs)));
            } else
                throw(PyException(MakeUserError("RamRemoteInstalledItemNotInStation")));
        } else {
            if (args.isCorpJob) {
                if (!IsHangarFlag(installedItem->flag())) {
                    if (args.installationContainerID == pClient->GetLocationID()) {
                        std::map<std::string, PyRep *> exceptArgs;
                        exceptArgs["location"] = new PyString(stDataMgr.GetStationName(args.installationContainerID));
                        throw(PyException(MakeUserError("RamCorpInstalledItemWrongLocation", exceptArgs)));
                    } else
                        throw(PyException(MakeUserError("RamRemoteInstalledItemNotInOffice")));
                }
            } else {
                if (installedItem->flag() != flagHangar) {
                    if (args.installationInvLocationID == pClient->GetLocationID()) {
                        std::map<std::string, PyRep *> exceptArgs;
                        exceptArgs["location"] = new PyString(stDataMgr.GetStationName(args.installationContainerID));
                        throw(PyException(MakeUserError("RamInstalledItemWrongLocation", exceptArgs)));
                    } else {
                        throw(PyException(MakeUserError("RamRemoteInstalledItemInStationNotHangar")));
                    }
                }
            }
        }
    } else if (args.installationContainerID == pClient->GetShipID()) {
        if (pClient->GetChar()->flag() != flagPilot)
            throw(PyException(MakeUserError("RamAccessDeniedNotPilot")));

        if (args.isCorpJob and (installedItem->flag() == flagCargoHold))
            throw(PyException(MakeUserError("RamCorpInstalledItemNotInCargo")));

        if (installedItem->locationID() != args.installationContainerID)
            throw(PyException(MakeUserError("RamInstalledItemMustBeInShip")));
    } else {
        // this will be POS assembly modules and Outpost checks
        // RamInstalledItemBadLocationStructure
        // RamInstalledItemInStructureNotInContainer
        // RamInstalledItemInStructureUnknownLocation
        throw(PyException(MakeCustomError("R.A.M. at POS/Outpost not supported yet")));
    }
}

void RamMethods::LocationRolesCheck(Client* const pClient, int16 flagID)
{
    int64 roles = pClient->GetCorpRole();
    if ((flagID == flagHangar and ((roles & Corp::Role::HangarCanTake1) != Corp::Role::HangarCanTake1))
    or  (flagID == flagCorpHangar2 and ((roles & Corp::Role::HangarCanTake2) != Corp::Role::HangarCanTake2))
    or  (flagID == flagCorpHangar3 and ((roles & Corp::Role::HangarCanTake3) != Corp::Role::HangarCanTake3))
    or  (flagID == flagCorpHangar4 and ((roles & Corp::Role::HangarCanTake4) != Corp::Role::HangarCanTake4))
    or  (flagID == flagCorpHangar5 and ((roles & Corp::Role::HangarCanTake5) != Corp::Role::HangarCanTake5))
    or  (flagID == flagCorpHangar6 and ((roles & Corp::Role::HangarCanTake6) != Corp::Role::HangarCanTake6))
    or  (flagID == flagCorpHangar7 and ((roles & Corp::Role::HangarCanTake7) != Corp::Role::HangarCanTake7)))
        throw(PyException(MakeUserError("RamAccessDeniedToBOMHangar")));
}

void RamMethods::MaterialSkillsCheck(Client* const pClient, uint32 runs, const PathElement& bomLocation, const Rsp_InstallJob& rsp, const std::vector< EvERam::RequiredItem >& reqItems)
{
    std::map<uint16, InventoryItemRef> items;   // typeID, itemRef
    GetBOMItemsMap( bomLocation, items );

    for (auto cur : reqItems) {
        if (cur.isSkill) { // check skill (quantity is required level)
            if (pClient->GetChar()->GetSkillLevel(cur.typeID) < cur.quantity) {
                std::map<std::string, PyRep *> args;
                args["item"] = new PyInt(cur.typeID);
                args["skillLevel"] = new PyInt(cur.quantity);
                throw(PyException(MakeUserError("RamNeedSkillForJob", args)));
            }
        } else {
            uint32 qtyNeeded = (uint32)ceil(cur.quantity * rsp.materialMultiplier * runs);
            if (cur.damagePerJob == 1.0)
                qtyNeeded = (uint32)ceil(qtyNeeded * rsp.charMaterialMultiplier);
            std::map<uint16, InventoryItemRef>::iterator itr = items.find(cur.typeID);
            if (itr != items.end())
                if (itr->second->typeID() == cur.typeID)
                    if (itr->second->ownerID() == pClient->GetCharacterID()) {
                        if (itr->second->quantity() < qtyNeeded)
                            qtyNeeded -= itr->second->quantity();
                        else
                            qtyNeeded = 0;
                    }

            if (qtyNeeded) {
                std::map<std::string, PyRep *> args;
                args["item"] = new PyInt( cur.typeID );
                throw(PyException(MakeUserError("RamNeedMoreForJob", args)));
            }
        }
    }
}

void RamMethods::ProductionTimeCheck(uint32 productionTime)
{
    if (productionTime > ramProductionTimeLimit) {
        std::map<std::string, PyRep *> args;
        args["productionTime"] = new PyInt(productionTime);
        args["limit"] = new PyInt(ramProductionTimeLimit);
        throw(PyException(MakeUserError("RamProductionTimeExceedsLimits", args)));
    }
}

void RamMethods::CompleteJob(const Call_CompleteJob &args, Client* const pClient)
{
    // this isnt entirely right....if job is installed in ship, receiver must be pilot in active ship to complete job.
    if (args.containerID == pClient->GetShipID())
        if (pClient->GetChar()->flag() != flagPilot)
            throw(PyException(MakeUserError("RamCompletionMustBeInShip")));

    uint32 ownerID = 0;
    int64 endProductionTime = 0;
    int8 status = 0, restrictionMask = 0;
    if (!RamProxyDB::GetJobVerifyProperties(args.jobID, ownerID, endProductionTime, restrictionMask, status))
        throw(PyException(MakeUserError("RamCompletionNoSuchJob")));

    if (ownerID != pClient->GetCharacterID()) {
        if (ownerID == pClient->GetCorporationID()) {
            if ((pClient->GetCorpRole() & Corp::Role::FactoryManager) != Corp::Role::FactoryManager)
                throw(PyException(MakeUserError("RamCompletionAccessDeniedByCorpRole")));
        } else  // alliances not implemented
            throw(PyException(MakeUserError("RamCompletionAccessDenied")));
    }

    if (status != EvERam::CompletedStatus::InProgress)
        throw(PyException(MakeUserError("RamCompletionJobCompleted")));

    if (!args.cancel and (endProductionTime > GetFileTimeNow()))
        throw(PyException(MakeUserError("RamCompletionInProduction")));
}

bool RamMethods::Calculate(const Call_InstallJob &args, InventoryItemRef installedItem, Client* const pClient, Rsp_InstallJob &into)
{
    if (!RamProxyDB::GetAssemblyLineProperties(args.installationAssemblyLineID, into))
        return false;

    Character* pChar = pClient->GetChar().get();

    const ItemType* pType(nullptr);
    switch(args.activityID) {
        case EvERam::Activity::Manufacturing: {
            BlueprintRef bp = BlueprintRef::StaticCast( installedItem );
            pType = &bp->productType();

            // do these need the actual bp values?
            into.productionTime = bp->type().productionTime();
            into.materialMultiplier *= bp->materialMultiplier();
            into.timeMultiplier *= bp->timeMultiplier();
            into.charTimeMultiplier = pChar->GetAttribute(AttrManufactureTimeMultiplier).get_double();

            switch(pType->race()) {
                case raceCaldari: {
                    if (pChar->HasAttribute(AttrCaldariTechTimePercent))
                        into.charTimeMultiplier *= (pChar->GetAttribute(AttrCaldariTechTimePercent).get_float() /100);
                } break;
                case raceMinmatar:{
                    if (pChar->HasAttribute(AttrMinmatarTechTimePercent))
                        into.charTimeMultiplier *= (pChar->GetAttribute(AttrMinmatarTechTimePercent).get_float() / 100);
                } break;
                case raceAmarr: {
                    if (pChar->HasAttribute(AttrAmarrTechTimePercent))
                        into.charTimeMultiplier *= (pChar->GetAttribute(AttrAmarrTechTimePercent).get_float() / 100);
                } break;
                case raceGallente: {
                    if (pChar->HasAttribute(AttrGallenteTechTimePercent))
                        into.charTimeMultiplier *= (pChar->GetAttribute(AttrGallenteTechTimePercent).get_float() / 100);
                } break;
                case raceJove:
                case racePirate:
                default: {
                    // unknown
                } break;
            }
            break;
        }
        case EvERam::Activity::ResearchTime: {
            BlueprintRef bp = BlueprintRef::StaticCast( installedItem );
            pType = &installedItem->type();
            //TODO  implement PE_ResearchTime here
            into.productionTime = bp->type().researchProductivityTime();
            into.charTimeMultiplier = pChar->GetAttribute(AttrManufacturingTimeResearchSpeed).get_double();
            break;
        }
        case EvERam::Activity::ResearchMaterial: {
            BlueprintRef bp = BlueprintRef::StaticCast( installedItem );
            pType = &installedItem->type();
            //TODO  implement ME_ResearchTime here
            into.productionTime = bp->type().researchMaterialTime();
            into.charTimeMultiplier = pChar->GetAttribute(AttrMineralNeedResearchSpeed).get_double();
            break;
        }
        case EvERam::Activity::Copying: {
            BlueprintRef bp = BlueprintRef::StaticCast( installedItem );
            pType = &installedItem->type();
            // no ceil() here on purpose
            into.productionTime = (bp->type().researchCopyTime() / bp->type().maxProductionLimit()) * args.licensedProductionRuns;
            into.charTimeMultiplier = pChar->GetAttribute(AttrCopySpeedPercent).get_double();
            break;
        }
        case EvERam::Activity::Duplicating:
        case EvERam::Activity::ReverseEngineering:
        case EvERam::Activity::Invention:
        default: {
            pType = &installedItem->type();
            into.charTimeMultiplier = 1.0;
            break;
        }
    }

    if (!GetMultipliers(args.installationAssemblyLineID, pType->groupID(), into.materialMultiplier, into.timeMultiplier))
        return false;

    // calculate the remaining things
    into.charMaterialMultiplier = 1.0; //ch->GetAttribute(AttrResearchCostPercent).get_int();   << this is not used
    into.productionTime *= (int32)(into.timeMultiplier * into.charTimeMultiplier * args.runs);
    into.usageCost *= ceil(into.productionTime / 3600.0);
    into.cost = into.installCost + into.usageCost;

    // I "hope" this is right, simple tells client how soon will his job be started
    // Unfortunately, rounding done on client's side causes showing "Start time: 0 seconds" when he has to wait less than minute
    // I have no idea how to avoid this ...
    into.maxJobStartTime = RamProxyDB::GetNextFreeTime(args.installationAssemblyLineID);

    return true;
}

void RamMethods::EncodeBillOfMaterials(const std::vector<EvERam::RequiredItem> &reqItems, double materialMultiplier, double charMaterialMultiplier, uint32 runs, BillOfMaterials &into)
{
    PySafeDecRef( into.extras.lines );
    into.extras.lines = new PyList();
    PySafeDecRef( into.wasteMaterials.lines );
    into.wasteMaterials.lines = new PyList();
    PySafeDecRef( into.rawMaterials.lines );
    into.rawMaterials.lines = new PyList();

    for (auto cur : reqItems) {
        if (cur.isSkill) {
            into.skills[cur.typeID] = new PyInt(cur.quantity);
            continue;
        }

        // otherwise, make line for material list
        MaterialList_Line line;
        line.requiredTypeID = cur.typeID;
        line.quantity = (int32)ceil(cur.quantity * materialMultiplier * runs);
        line.damagePerJob = cur.damagePerJob;
        line.isSkillCheck = false;  // no idea what is this for
        line.requiresHP = false;    // no idea what is this for

        /** @todo update this shit.....  */
        // "Extra material" is not affected by skills, and return upon completion
        // "Raw material" is fully consumed and affected by skills/efficiency
        // "Waste Material" is amount of material wasted ...
        if (cur.extra) {
            into.extras.lines->AddItem( line.Encode() );
        } else {
            // if there are losses, make line for waste material list
            if (charMaterialMultiplier > 1.0) {
                MaterialList_Line wastage( line );  // simply copy origial line ...
                wastage.quantity = (int32)ceil(wastage.quantity * (charMaterialMultiplier - 1.0)); // ... and calculate proper quantity
                into.wasteMaterials.lines->AddItem( wastage.Encode() );
            }
            into.rawMaterials.lines->AddItem( line.Encode() );
        }
    }
}

void RamMethods::EncodeMissingMaterials(const std::vector<EvERam::RequiredItem> &reqItems, const PathElement &bomLocation, Client *const pClient, double materialMultiplier, double charMaterialMultiplier, int32 runs, std::map<int32, PyRep *> &into) {
    //query out what we need
    std::vector<InventoryItemRef> skills, items;

    //get the skills
    pClient->GetChar()->GetMyInventory()->GetItemsByFlag(flagSkill, skills);
    pClient->GetChar()->GetMyInventory()->GetItemsByFlag(flagSkillInTraining, skills);

    //get the items
    GetBOMItems( bomLocation, items );

    //now do the check
    uint32 qtyReq = 0;
    for (auto cur : reqItems) {
        qtyReq = cur.quantity;
        if (!cur.isSkill) {
            qtyReq = (uint32)ceil(qtyReq * materialMultiplier * runs);
            if (cur.damagePerJob == 1.0)
                qtyReq = (uint32)ceil(qtyReq * charMaterialMultiplier);
        }

        std::vector<InventoryItemRef>::iterator curi, endi;
        if (cur.isSkill) {
            curi = skills.begin();
            endi = skills.end();
        } else {
            curi = items.begin();
            endi = items.end();
        }
        /** @todo update this for corp usage */
        for (; curi != endi and qtyReq > 0; ++curi) {
            if ((*curi)->typeID() == cur.typeID and (*curi)->ownerID() == pClient->GetCharacterID()) {
                if (cur.isSkill)
                    qtyReq -= std::min(qtyReq, (*curi)->GetAttribute(AttrSkillLevel).get_uint32() );
                else
                    qtyReq -= std::min(qtyReq, (uint32)(*curi)->quantity() );
            }
        }

        if (qtyReq > 0)
            into[cur.typeID] = new PyInt(qtyReq);
    }
}
void RamMethods::GetBOMItems(const PathElement& bomLocation, std::vector< InventoryItemRef >& into)
{
    Inventory *inventory = sItemFactory.GetInventoryFromId( bomLocation.locationID );
    if (inventory != nullptr )
        inventory->GetItemsByFlag((EVEItemFlags)bomLocation.flag, into );
}

void RamMethods::GetBOMItemsMap(const PathElement& bomLocation, std::map< uint16, InventoryItemRef >& into)
{
    Inventory *inventory = sItemFactory.GetInventoryFromId( bomLocation.locationID );
    if (inventory != nullptr )
        inventory->GetTypesByFlag( (EVEItemFlags)bomLocation.flag, into );
}


bool RamMethods::GetMultipliers(const uint32 assemblyLineID, const uint32 productGroupID, double &materialMultiplier, double &timeMultiplier) {
    double tmpMat, tmpTime;
    if (!RamProxyDB::GetMultipliers(assemblyLineID, productGroupID, tmpMat, tmpTime))
        return false;

    materialMultiplier *= tmpMat;
    timeMultiplier *= tmpTime;
    return true;
}

/* each material required for a blueprint (from invTypeMaterials) the quantity is affected by ME research and skills.
 * Then there’s the extra materials, which come from the ramTypeRequirements table for that BP.
 * Next, any materials in ramTypeRequirements which are marked as recyclable, have their recycled materials (from invTypeMaterials) subtracted from the list of materials required for the produced item.
 * The remaining materials from invTypeMaterials are then modified by skills and ME research as follows;
 */
void RamMethods::GetAdjustedRamRequiredMaterials()
{
    // xmatls = ramTypeRequirements.extra
    // reqMatls = (invTypeMaterials.reqMatls - xmatls)
    // waste = reqMatls * charSkills (which is wastage multiplier based on skills)
    // totalReqMatls = (reqMatls * skill mods) + xmatls + waste
    // will need to update this using above formula for correct material list

}

const char* RamMethods::GetActivityName(int8 activityID)
{
    switch (activityID) {
        case EvERam::Activity::Copying:             return "Copying";
        case EvERam::Activity::Manufacturing:       return "Manufacturing";
        case EvERam::Activity::Invention:           return "Invention";
        case EvERam::Activity::ResearchMaterial:    return "Research ME";
        case EvERam::Activity::ResearchTime:        return "Research PE";
        case EvERam::Activity::ReverseEngineering:  return "Reverse Engineering";
        // these last 2 should never hit
        case EvERam::Activity::Duplicating:
        case EvERam::Activity::ResearchTech: {
            codelog(MANUF__ERROR, "RamMethods::GetActivityName - invalid activity sent: %u", activityID);
        } break;
    }
    return "Undefined";
}

