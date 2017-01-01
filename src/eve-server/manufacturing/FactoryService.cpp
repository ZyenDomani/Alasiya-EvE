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

/** @todo  maybe make this a static data object when the BP is created,
 * as this is called on EVERY "show info" of the blueprint
 */
PyResult FactoryService::Handle_GetMaterialsForTypeWithActivity(PyCallArgs &call) {
	Call_SingleIntegerArg args;
    if(!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode args.");
        return nullptr;
    }

    // dunno how to do this better...
    PyList* matlListManuf = new PyList();
    PyList* skillListManuf = new PyList();
    PyList* matlListTech = new PyList();
    PyList* skillListTech = new PyList();
    PyList* matlListTE = new PyList();
    PyList* skillListTE = new PyList();
    PyList* matlListME = new PyList();
    PyList* skillListME = new PyList();
    PyList* matlListCopy = new PyList();
    PyList* skillListCopy = new PyList();
    PyList* matlListDup = new PyList();
    PyList* skillListDup = new PyList();
    PyList* matlListRE = new PyList();
    PyList* skillListRE = new PyList();
    PyList* matlListInvent = new PyList();
    PyList* skillListInvent = new PyList();

    /*  not sure what the 'extras' field is for yet.  something to do with adding materials to required production list
     * it uses the .Index(columnName) method from CRowSet class in client.
     *  as i dont have data for it yet, just add an empty CRow object as the 'extras' data.
     * this WORKS!!  bp BOM tab is now working!!
     */
    DBQueryResult mtRes;
    PyList* extras = new PyList();

    DBRowDescriptor* header = new DBRowDescriptor;
        header->AddColumn( "quantity",          DBTYPE_I4 );
        header->AddColumn( "requiredTypeID",    DBTYPE_I4 );
        header->AddColumn( "damagePerJob",      DBTYPE_R4 );

    // ramMaterials is only for manufacturing the bp product  NOTE: this is always populated
    const BlueprintType* bpType = call.client->services().item_factory->GetBlueprintType(args.arg);
    std::vector<ramMaterials> ramMatls;
    sDataMgr.GetRamMaterials(bpType->productTypeID(), ramMatls);
    for (auto cur : ramMatls) {
        PyPackedRow* row = new PyPackedRow( header );
        row->SetField( "quantity",        new PyInt(cur.quantity));
        row->SetField( "requiredTypeID",  new PyInt(cur.materialTypeID));
        row->SetField( "damagePerJob",    new PyFloat(1.0));
        matlListManuf->AddItem(row);
    }
    PyDict* Manufacturing = new PyDict();
    Manufacturing->SetItemString("rawMaterials", matlListManuf);

    // booleans to only set items that are populated  NOTE: manuf is always populated
    bool copy = false, invent = false, dup = false, me = false, re = false, te = false, tech = false;
    //  get skills and materials for R.A.M.
    std::vector<ramRequirements> ramReqs;
    sDataMgr.GetRamRequirements(args.arg, ramReqs);
    bool skill = false;
    for (auto cur : ramReqs) {
        skill = false;
        PyPackedRow* row = new PyPackedRow( header );
        row->SetField( "quantity",        new PyInt(cur.quantity));
        row->SetField( "requiredTypeID",  new PyInt(cur.requiredTypeID));
        row->SetField( "damagePerJob",    new PyFloat(cur.damagePerJob));
        if (sDataMgr.IsSkillTypeID(cur.requiredTypeID))
            skill = true;
        /*  the ramRequirements table holds ALL skill/item data for all aspects of RAM per Blueprint.
         * the activityID field is the only separator for the activity types
         * this is ugly, but i dont know of a better way to sort the items yet.
         */
        switch(cur.activityID) {
            case ramActivityCopying: { //5
                copy = true;
                if (skill)
                    skillListCopy->AddItem(row);
                else
                    matlListCopy->AddItem(row);
            } break;
            case ramActivityInvention: { //8
                invent = true;
                if (skill)
                    skillListInvent->AddItem(row);
                else
                    matlListInvent->AddItem(row);
            } break;
            case ramActivityDuplicating:{ //6
                dup = true;
                if (skill)
                    skillListDup->AddItem(row);
                else
                    matlListDup->AddItem(row);
            } break;
            case ramActivityManufacturing:{ //1
                if (skill)
                    skillListManuf->AddItem(row);
                else
                    matlListManuf->AddItem(row);
            } break;
            case ramActivityReverseEngineering:{ //7
                re = true;
                if (skill)
                    skillListRE->AddItem(row);
                else
                    matlListRE->AddItem(row);
            } break;
            case ramActivityResearchingTechnology:{ //2
                tech = true;
                if (skill)
                    skillListTech->AddItem(row);
                else
                    matlListTech->AddItem(row);
            } break;
            case ramActivityResearchingTimeProductivity:{ //3
                te = true;
                if (skill)
                    skillListTE->AddItem(row);
                else
                    matlListTE->AddItem(row);
            } break;
            case ramActivityResearchingMaterialProductivity:{ //4
                me = true;
                if (skill)
                    skillListME->AddItem(row);
                else
                    matlListME->AddItem(row);
            } break;
        }
    }

    // this is the response.  test for populated items, and create an ItemString in the dict for that item.
    // items not populated and set in the response dict will not be shown in the BP info.
    PyDict* rsp = new PyDict();

    // this should stay empty
    //activityNone = 0
    //rsp->SetItem(0, new PyDict());

    // manuf is always populated
    //activityManufacturing = 1
    Manufacturing->SetItemString("skills", skillListManuf);
    Manufacturing->SetItemString("extras", DBResultToCRowset(mtRes));
    rsp->SetItem(new PyInt(1), new PyObject("util.KeyVal", Manufacturing));

    if (tech) {
        //activityResearchingTechnology = 2
        PyDict* ResearchingTechnology = new PyDict();
        ResearchingTechnology->SetItemString("skills", skillListTech);
        ResearchingTechnology->SetItemString("rawMaterials", matlListTech);
        ResearchingTechnology->SetItemString("extras", DBResultToCRowset(mtRes));
        rsp->SetItem(new PyInt(2), new PyObject("util.KeyVal", ResearchingTechnology));
    }
    if (te) {
        //activityResearchingTimeProductivity = 3
        PyDict* ResearchingTimeProductivity = new PyDict();
        ResearchingTimeProductivity->SetItemString("skills", skillListTE);
        ResearchingTimeProductivity->SetItemString("rawMaterials", matlListTE);
        ResearchingTimeProductivity->SetItemString("extras", DBResultToCRowset(mtRes));
        rsp->SetItem(new PyInt(3), new PyObject("util.KeyVal", ResearchingTimeProductivity));
    }
    if (me) {
        //activityResearchingMaterialProductivity = 4
        PyDict* ResearchingMaterialProductivity = new PyDict();
        ResearchingMaterialProductivity->SetItemString("skills", skillListME);
        ResearchingMaterialProductivity->SetItemString("rawMaterials", matlListME);
        ResearchingMaterialProductivity->SetItemString("extras", DBResultToCRowset(mtRes));
        rsp->SetItem(new PyInt(4), new PyObject("util.KeyVal", ResearchingMaterialProductivity));
    }
    if (copy) {
        //activityCopying = 5
        PyDict* Copying = new PyDict();
        Copying->SetItemString("skills", skillListCopy);
        Copying->SetItemString("rawMaterials", matlListCopy);
        Copying->SetItemString("extras", DBResultToCRowset(mtRes));
        rsp->SetItem(new PyInt(5), new PyObject("util.KeyVal", Copying));
    }
    if (dup) {
        //activityDuplicating = 6
        PyDict* Duplicating = new PyDict();
        Duplicating->SetItemString("skills", skillListDup);
        Duplicating->SetItemString("rawMaterials", matlListDup);
        Duplicating->SetItemString("extras", DBResultToCRowset(mtRes));
        rsp->SetItem(new PyInt(6), new PyObject("util.KeyVal", Duplicating));
    }
    if (re) {
        //activityReverseEngineering = 7
        PyDict* ReverseEngineering = new PyDict();
        ReverseEngineering->SetItemString("skills", skillListRE);
        ReverseEngineering->SetItemString("rawMaterials", matlListRE);
        ReverseEngineering->SetItemString("extras", DBResultToCRowset(mtRes));
        rsp->SetItem(new PyInt(7), new PyObject("util.KeyVal", ReverseEngineering));
    }
    if (invent) {
        //activityInvention = 8
        PyDict* Invention = new PyDict();
        Invention->SetItemString("skills", skillListInvent);
        Invention->SetItemString("rawMaterials", matlListInvent);
        Invention->SetItemString("extras", DBResultToCRowset(mtRes));
        rsp->SetItem(new PyInt(8), new PyObject("util.KeyVal", Invention));
    }

    PyIncRef(rsp);
    if (is_log_enabled(MANUF__DUMP))
        rsp->Dump(MANUF__DUMP, "   ");

    return rsp;
}

PyResult FactoryService::Handle_GetMaterialCompositionOfItemType(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if(!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode args.");
        return nullptr;
    }

    return m_db.GetMaterialCompositionOfItemType(args.arg);
}

