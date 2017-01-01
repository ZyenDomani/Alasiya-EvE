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
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "StaticDataMgr.h"
#include "manufacturing/Blueprint.h"
#include "manufacturing/FactoryService.h"

PyCallable_Make_InnerDispatcher(FactoryService)

FactoryService::FactoryService(PyServiceMgr *mgr)
: PyService(mgr, "factory"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(FactoryService, GetBlueprintAttributes);
    PyCallable_REG_CALL(FactoryService, GetMaterialsForTypeWithActivity);
    PyCallable_REG_CALL(FactoryService, GetMaterialCompositionOfItemType);
}

FactoryService::~FactoryService() {
    delete m_dispatch;
}

PyResult FactoryService::Handle_GetBlueprintAttributes(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if(!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode args.");
        return nullptr;
    }

    BlueprintRef b = m_manager->item_factory->GetBlueprint( args.arg );
    if( !b )
        return nullptr;

    return b->GetBlueprintAttributes();
}

/** @todo  this still needs work....not showing tabs  */
PyResult FactoryService::Handle_GetMaterialsForTypeWithActivity(PyCallArgs &call) {
	Call_SingleIntegerArg args;
    if(!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode args.");
        return nullptr;
    }

    PyList* extras = new PyList();
    PyList* matllist = new PyList();
    PyList* skilllist = new PyList();

    PyDict* Copying = new PyDict();
    PyDict* Invention = new PyDict();
    PyDict* Duplicating = new PyDict();
    PyDict* Manufacturing = new PyDict();
    PyDict* ReverseEngineering = new PyDict();
    PyDict* ResearchingTechnology = new PyDict();
    PyDict* ResearchingTimeProductivity = new PyDict();
    PyDict* ResearchingMaterialProductivity = new PyDict();

    //  get skills and materials for R.A.M.
    std::vector<ramRequirements> ramReqs;
    sDataMgr.GetRamRequirements(args.arg, ramReqs);
    for (auto cur : ramReqs) {
        PyDict* linedata = new PyDict();
        linedata->SetItemString("quantity",         new PyInt(cur.quantity));
        linedata->SetItemString("requiredTypeID",   new PyInt(cur.requiredTypeID));
        linedata->SetItemString("damagePerJob",     new PyFloat(cur.damagePerJob));
        if (sDataMgr.IsSkillTypeID(cur.requiredTypeID))
            skilllist->AddItem(linedata);
        /*
        else
            matllist->AddItem(linedata);
        switch(cur.activityID) {
            case ramActivityCopying: {
                if (skill)
                    Copying->AddItem(skilllist);
                else
                    Copying->AddItem(matllist);
            } break;
            case ramActivityInvention: {
                if (skill)
                    Invention->AddItem(skilllist);
                else
                    Invention->AddItem(matllist);
            } break;
            case ramActivityDuplicating:{
                if (skill)
                    Duplicating->AddItem(skilllist);
                else
                    Duplicating->AddItem(matllist);
            } break;
            case ramActivityManufacturing:{
                if (skill)
                    Manufacturing->AddItem(skilllist);
                else
                    Manufacturing->AddItem(matllist);
            } break;
            case ramActivityReverseEngineering:{
                if (skill)
                    ReverseEngineering->AddItem(skilllist);
                else
                    ReverseEngineering->AddItem(matllist);
            } break;
            case ramActivityResearchingTechnology:{
                if (skill)
                    ResearchingTechnology->AddItem(skilllist);
                else
                    ResearchingTechnology->AddItem(matllist);
            } break;
            case ramActivityResearchingTimeProductivity:{
                if (skill)
                    ResearchingTimeProductivity->AddItem(skilllist);
                else
                    ResearchingTimeProductivity->AddItem(matllist);
            } break;
            case ramActivityResearchingMaterialProductivity:{
                if (skill)
                    ResearchingMaterialProductivity->AddItem(skilllist);
                else
                    ResearchingMaterialProductivity->AddItem(matllist);
            } break;
        }
        matllist->clear();
        skilllist->clear();
        */
    }

    const BlueprintType* bpType = call.client->services().item_factory->GetBlueprintType(args.arg);
    std::vector<ramMaterials> ramMatls;
    sDataMgr.GetRamMaterials(bpType->productTypeID(), ramMatls);
    for (auto cur : ramMatls) {
        PyDict* linedata = new PyDict();
        linedata->SetItemString("requiredTypeID",   new PyInt(cur.materialTypeID));
        linedata->SetItemString("quantity",         new PyInt(cur.quantity));
        linedata->SetItemString("damagePerJob",     new PyFloat(1.0));
        matllist->AddItem(linedata);
    }

    Manufacturing->SetItemString("skills", skilllist);
    Manufacturing->SetItemString("rawMaterials", matllist);
    Manufacturing->SetItemString("extras", extras);

    PyTuple *tuple = new PyTuple(9);
    //activityNone = 0
    tuple->SetItem(0, new PyDict());  // this should stay empty
    //activityManufacturing = 1
    tuple->SetItem(1, Manufacturing);
    //activityResearchingTechnology = 2
    tuple->SetItem(2, ResearchingTechnology);
    //activityResearchingTimeProductivity = 3
    tuple->SetItem(3, ResearchingTimeProductivity);
    //activityResearchingMaterialProductivity = 4
    tuple->SetItem(4, ResearchingMaterialProductivity );
    //activityCopying = 5
    tuple->SetItem(5, Copying);
    //activityDuplicating = 6
    tuple->SetItem(6, Duplicating);
    //activityReverseEngineering = 7
    tuple->SetItem(7, ReverseEngineering);
    //activityInvention = 8
    tuple->SetItem(8, Invention);

    PyIncRef(tuple);
    if (is_log_enabled(MANUF__DUMP))
        tuple->Dump(MANUF__DUMP, "   ");
    return tuple;
}

PyResult FactoryService::Handle_GetMaterialCompositionOfItemType(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if(!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode args.");
        return nullptr;
    }

    return m_db.GetMaterialCompositionOfItemType(args.arg);
}

