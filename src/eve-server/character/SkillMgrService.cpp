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
    Author:     Zhur
    Updates:    Allan
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "character/SkillMgrService.h"
#include "effects/EffectsDataMgr.h"
#include "effects/EffectsProcessor.h"

PyCallable_Make_InnerDispatcher(SkillMgrService)
PyCallable_Make_InnerDispatcher(SkillMgrBound)

SkillMgrService::SkillMgrService(PyServiceMgr *mgr)
: PyService(mgr, "skillMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);
}

SkillMgrService::~SkillMgrService() {
    delete m_dispatch;
}

PyBoundObject *SkillMgrService::CreateBoundObject(Client *pClient, const PyRep *bind_args) {
    _log(CLIENT__MESSAGE, "SkillMgrService bind request for:");
    bind_args->Dump(CLIENT__MESSAGE, "    ");

    return(new SkillMgrBound(m_manager, m_db));
}

SkillMgrBound::SkillMgrBound(PyServiceMgr *mgr, CharacterDB &db)
: PyBoundObject(mgr),
  m_dispatch(new Dispatcher(this)),
  m_db(db)
{
    _SetCallDispatcher(m_dispatch);

    m_strBoundObjectName = "SkillMgrBound";

    PyCallable_REG_CALL(SkillMgrBound, GetRespecInfo);
    PyCallable_REG_CALL(SkillMgrBound, GetSkillQueueAndFreePoints);
    PyCallable_REG_CALL(SkillMgrBound, GetEndOfTraining);
    PyCallable_REG_CALL(SkillMgrBound, GetSkillHistory);
    PyCallable_REG_CALL(SkillMgrBound, CharStopTrainingSkill);
    PyCallable_REG_CALL(SkillMgrBound, CharStartTrainingSkill);
    PyCallable_REG_CALL(SkillMgrBound, AddToEndOfSkillQueue);
    PyCallable_REG_CALL(SkillMgrBound, InjectSkillIntoBrain);
    PyCallable_REG_CALL(SkillMgrBound, SaveSkillQueue);
    PyCallable_REG_CALL(SkillMgrBound, CharStartTrainingSkillByTypeID);
    PyCallable_REG_CALL(SkillMgrBound, RespecCharacter);
    PyCallable_REG_CALL(SkillMgrBound, GetCharacterAttributeModifiers);
    PyCallable_REG_CALL(SkillMgrBound, CharAddImplant);
    PyCallable_REG_CALL(SkillMgrBound, RemoveImplantFromCharacter);
    PyCallable_REG_CALL(SkillMgrBound, CharUseBooster);
}

SkillMgrBound::~SkillMgrBound()
{
    delete m_dispatch;
}

PyResult SkillMgrBound::Handle_GetRespecInfo( PyCallArgs& call ) {
    return m_db.GetRespecInfo(call.client->GetCharacterID());
}

PyResult SkillMgrBound::Handle_GetSkillQueueAndFreePoints(PyCallArgs &call) {
    return call.client->GetChar()->SendSkillQueue();
}

PyResult SkillMgrBound::Handle_GetEndOfTraining(PyCallArgs &call) {
    return new PyLong( call.client->GetChar()->GetEndOfTraining() );
}

PyResult SkillMgrBound::Handle_GetSkillHistory( PyCallArgs& call ) {
    return call.client->GetChar()->GetSkillHistory();
}

PyResult SkillMgrBound::Handle_CharStopTrainingSkill(PyCallArgs &call) {
    // called when pausing skill queue
    call.client->GetChar()->PauseSkillQueue();
    // returns nothing
    return nullptr;
}

PyResult SkillMgrBound::Handle_CharStartTrainingSkill( PyCallArgs& call ) {
    // sm.GetService('godma').GetSkillHandler().CharStartTrainingSkill(skillX.itemID, skillX.locationID)
    Call_TwoIntegerArgs args;
    if ( !args.Decode( call.tuple ) )
    {
        codelog( SERVICE__ERROR, "%s: Failed to decode arguments.", GetName() );
        return nullptr;
    }

    _log(SKILL__WARNING, "Called CharStartTrainingSkill for itemID %i in location %i", args.arg1, args.arg2);
    return nullptr;
}

PyResult SkillMgrBound::Handle_AddToEndOfSkillQueue(PyCallArgs &call) {
    //  sm.StartService('godma').GetSkillHandler().AddToEndOfSkillQueue(skillID, nextLevel)
    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    CharacterRef cRef(call.client->GetChar());
    cRef->AddToSkillQueue(args.arg1, args.arg2);
    cRef->UpdateSkillQueueEndTime();
    return nullptr;
}

PyResult SkillMgrBound::Handle_InjectSkillIntoBrain(PyCallArgs &call)
{
    Call_InjectSkillIntoBrain args;
    if (!args.Decode(&call.tuple)) {
        codelog( SERVICE__ERROR, "%s: Failed to decode arguments.", GetName() );
        return nullptr;
    }

    // make a list of skills successfully injected to display after injection
    // name, ret value  where 1=success, 2=prereqs, 3=already known, 4=split fail, 5=load fail
    std::map<std::string, uint8> skills;
    SkillRef skillRef(nullptr);
    CharacterRef cRef(call.client->GetChar());
    for (auto &cur : args.skills)  {
        skillRef = sItemFactory.GetSkillRef(cur);
        if (skillRef.get() == nullptr) {
            _log( ITEM__ERROR, "%s: failed to load skill %u for injection.", call.client->GetName(), cur);
            std::string str = "Invalid Name #";
            str += std::to_string(cur);
            skills.emplace(str, 5);
            continue;
        }

        skills.emplace(skillRef->itemName(), cRef->InjectSkillIntoBrain(skillRef));
    }

    // build and populate status reply
    if (skills.empty())
        return nullptr;

    if (skills.size() == 1) {
        std::string status;
        switch (skills.begin()->second) {
            //1=success, 2=prereqs, 3=already known, 4=split fail, 5=load fail
            case 1: status = "<color=green>Success.</color>"; break;
            case 2: status = "<color=red>Failed:</color> <color=yellow>Prerequisites incomplete.</color>"; break;
            case 3: status = "<color=red>Failed:</color> <color=cyan>Skill already known.</color>"; break;
            case 4: status = "<color=red>Failed:</color> <color=red>Stack split failure.</color>"; break;
            case 5: status = "<color=red>Failed:</color> <color=maroon>Skill loading failure.</color>"; break;
            default: status = "<color=red>Failed:</color> <color=red>Unknown Error.</color>"; break;
        }
        call.client->SendInfoModalMsg("Injection of %s:  %s", skills.begin()->first.c_str(), status.c_str());
    } else {
        std::string status;
        std::ostringstream str;
        str.clear();
        str << "The Injection of %u skills for %s has resulted in the following outcome.<br><br>"; //40

        for (auto &cur : skills) {
            switch (cur.second) {
                //1=success, 2=prereqs, 3=already known, 4=split fail, 5=load fail
                case 1: status = "<color=green>Success.</color>"; break;
                case 2: status = "<color=red>Failed:</color> <color=yellow>Prerequisites incomplete.</color>"; break;
                case 3: status = "<color=red>Failed:</color> <color=cyan>Skill already known.</color>"; break;
                case 4: status = "<color=red>Failed:</color> <color=red>Stack split failure.</color>"; break;
                case 5: status = "<color=red>Failed:</color> <color=maroon>Skill loading failure.</color>"; break;
                default: status = "<color=red>Failed:</color> <color=red>Unknown Error.</color>"; break;
            }
            str << cur.first << " - " << status << "<br>"; //40 for name, 80 for status (120)
        }

        int size = skills.size() * 120;
        size += 100;    // for header, including char name
        char reply[size];
        snprintf(reply, size, str.str().c_str(), skills.size(), call.client->GetName());

        call.client->SendInfoModalMsg(reply);
    }

    PyTuple* tmp = new PyTuple(1);
    tmp->SetItem(0, new PyString("OnSkillInjected"));
    call.client->QueueDestinyEvent(&tmp);
    return nullptr;
}

PyResult SkillMgrBound::Handle_SaveSkillQueue(PyCallArgs &call) {
    // called when previously-set skill queue changed
    Call_SaveSkillQueue args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    // xml decode will now check for and fix level being a float instead of int and leading to client freakout
    CharacterRef cRef(call.client->GetChar());
    _log(SKILL__QUEUE, "%s(%u) calling SaveSkillQueue()", cRef->name(), cRef->itemID());
    cRef->ClearSkillQueue(true);
    SkillQueue_Element el;
    std::vector<PyRep*>::const_iterator cur = args.queue->begin(), end = args.queue->end();
    for (; cur != end; cur++) {
        if (!el.Decode(*cur))         {
            _log(SERVICE__ERROR, "%s: Failed to decode element of SkillQueue. Skipping.", call.client->GetName());
            continue;
        }
        cRef->AddToSkillQueue( el.typeID, el.level );
    }

    cRef->UpdateSkillQueueEndTime();
    PyTuple* tmp = new PyTuple(1);
        tmp->SetItem(0, new PyString("OnSkillTrainingSaved"));
    call.client->QueueDestinyEvent(&tmp);
    return nullptr;
}

PyResult SkillMgrBound::Handle_CharStartTrainingSkillByTypeID( PyCallArgs& call )
{
    // called when skill queue empty or paused
    // sends skill typeID to start training
    SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog( SERVICE__ERROR, "%s: Failed to decode arguments.", GetName() );
        return nullptr;
    }

    call.client->GetChar()->LoadPausedSkillQueue(args.arg);
    return nullptr;
}

PyResult SkillMgrBound::Handle_RespecCharacter(PyCallArgs &call)
{
    Call_RespecCharacter args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    CharacterRef cRef(call.client->GetChar());
    /* this is done in client
    if (cRef->GetSkillInTraining() != nullptr)
        throw UserError("RespecSkillInTraining");
    */

    // return early if this is an illegal call
    if (!m_db.ReportRespec(call.client->GetCharacterID()))
        return nullptr;
    //uint8 multiplier(sConfig.character.statMultiplier);
    cRef->SetAttribute(AttrCharisma, args.charisma);
    cRef->SetAttribute(AttrIntelligence, args.intelligence);
    cRef->SetAttribute(AttrMemory, args.memory);
    cRef->SetAttribute(AttrPerception, args.perception);
    cRef->SetAttribute(AttrWillpower, args.willpower);
    cRef->SaveAttributes();

    // no return value
    return nullptr;
}

PyResult SkillMgrBound::Handle_GetCharacterAttributeModifiers(PyCallArgs &call)
{
    //  for (itemID, typeID, operation, value,) in modifiers:

    /*
     * client sends attrib# of stat in question...
     *            [PyString "GetCharacterAttributeModifiers"]
     *            [PyTuple 1 items]
     *              [PyInt 165]
     * we return this...
     *        [PyList 1 items]
     *          [PyTuple 4 items]
     *            [PyIntegerVar 1866309449]   << implantID
     *            [PyInt 9943]                << implantTypeID
     *            [PyInt 2]                   << operation
     *            [PyFloat 3]                 << value
     */
    SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    CharacterRef cRef(call.client->GetChar());
    PyList* list = new PyList();
    // for each implant, make tuple and put into list
    PyTuple* tuple = new PyTuple(4);
        tuple->SetItem(0, PyStatic.NewZero());   //implantID
        tuple->SetItem(1, PyStatic.NewZero());   //implantTypeID
        tuple->SetItem(2, PyStatic.NewZero());   //operation
        tuple->SetItem(3, PyStatic.NewZero());   //value
        list->AddItem(tuple);

    return list;
}

PyResult SkillMgrBound::Handle_CharAddImplant( PyCallArgs& call )
{
    //client verifies implant, slot and skillInTraining.  sends itemid
    SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    InventoryItemRef iRef = sItemFactory.GetItemRefFromID(args.arg);
    if (iRef.get() == nullptr) {
        _log(CHARACTER__ERROR, "CharAddImplant - iRef not found for itemID %i", args.arg);
        return nullptr;
    }

    CharacterRef cRef(call.client->GetChar());
    uint8 implantSlot(iRef->GetAttribute(AttrImplantness).get_uint32());  //implant slot
    if (!cRef->IsSlotAvaliable(implantSlot)) {
        throw UserError("OnlyOneImplantActiveBody")
        .AddFormatValue("typeName", new PyString(iRef->itemName()));
    }

    // test skill level, if applicable
    if (iRef->GetAttribute(AttrRequiredSkill1Level).get_uint32() > cRef->GetSkillLevel(AttrRequiredSkill1)) {
        throw CustomError("The implant %s requires the %s skill trained to level %u", \
                        iRef->name(), sDataMgr.GetSkillName(AttrRequiredSkill1), \
                        iRef->GetAttribute(AttrRequiredSkill1Level).get_uint32());
    }

    if (iRef->GetAttribute(AttrRequiredSkill2Level).get_uint32() > cRef->GetSkillLevel(AttrRequiredSkill2)) {
        throw CustomError("The implant %s requires the %s skill trained to level %u", \
                        iRef->name(), sDataMgr.GetSkillName(AttrRequiredSkill2), \
                        iRef->GetAttribute(AttrRequiredSkill2Level).get_uint32());
    }

    _log(CHARACTER__MESSAGE, "CharAddImplant - Adding %s(%i) to %s(%u) in slot %u", \
            iRef->name(), args.arg, cRef->name(), cRef->itemID(), implantSlot);

    cRef->AddImplant(implantSlot, iRef);

    //{'FullPath': u'UI/Messages', 'messageID': 259217, 'label': u'PrereqImplantMissingBody'}(u'Attempting to use this implant without the aid of a {typeName} will destroy your cerebral cortex. Please consider alternate methods of suicide.', None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
    //{'FullPath': u'UI/Messages', 'messageID': 259604, 'label': u'ImplantHasSkillPrerequisitesBody'}(u'The implant {[item]item.name} requires the following {[numeric]skillCount -> "skill", "skills"}: {requiredSkills}.', None, {u'{[numeric]skillCount -> "skill", "skills"}': {'conditionalValues': [u'skill', u'skills'], 'variableType': 9, 'propertyName': None, 'args': 320, 'kwargs': {}, 'variableName': 'skillCount'}, u'{requiredSkills}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'requiredSkills'}, u'{[item]item.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'item'}})
    //{'FullPath': u'UI/Messages', 'messageID': 259217, 'label': u'PrereqImplantMissingBody'}(u'Attempting to use this implant without the aid of a {typeName} will destroy your cerebral cortex. Please consider alternate methods of suicide.', None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})

    /*  these are some kind of modifers for full sets....
     *    AttrImplantSetBloodraider = 799,
     *    AttrImplantSetSerpentis = 802,
     *    AttrImplantSetSerpentis2 = 803,
     *    AttrImplantSetGuristas = 838,
     *    AttrImplantSetAngel = 863,
     *    AttrImplantSetSansha = 864,
     *    AttrImplantBonusVelocity = 1076,
     *    AttrImplantSetThukker = 1282,
     *    AttrImplantSetSisters = 1284,
     *    AttrImplantSetSyndicate = 1291,
     *    AttrImplantSetORE = 1292,
     *    AttrImplantSetMordus = 1293,
     *    AttrImplantSetImperialNavy = 1550,
     *    AttrImplantSetCaldariNavy = 1552,
     *    AttrImplantSetFederationNavy = 1553,
     *    AttrImplantSetRepublicFleet = 1554,
     *    AttrImplantSetLGImperialNavy = 1569,
     *    AttrImplantSetLGFederationNavy = 1570,
     *    AttrImplantSetLGCaldariNavy = 1571,
     *    AttrImplantSetLGRepublicFleet = 1572,
     *    AttrimplantSetChristmas = 1799,
     *
     */


    // remove item from inventory

    return nullptr;
}

PyResult SkillMgrBound::Handle_RemoveImplantFromCharacter(PyCallArgs& call)
{
    //sends itemid
    SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    InventoryItemRef iRef = sItemFactory.GetItemRefFromID(args.arg);
    if (iRef.get() == nullptr) {
        _log(CHARACTER__ERROR, "CharRemoveImplant - iRef not found for itemID %i", args.arg);
        return nullptr;
    }

    CharacterRef cRef(call.client->GetChar());
    uint8 implantSlot(iRef->GetAttribute(AttrImplantness).get_uint32());  //implant slot
    _log(CHARACTER__MESSAGE, "CharRemoveImplant - Removing %s(%i) from %s(%u)", \
            iRef->name(), args.arg, cRef->name(), cRef->itemID());

    cRef->RemoveImplant(implantSlot);
    // delete implant
    iRef->Delete();

    return nullptr;
}

PyResult SkillMgrBound::Handle_CharUseBooster(PyCallArgs& call)
{
    //GetSkillHandler().CharUseBooster(invItem.itemID, invItem.locationID)

    // client only verifies booster groupID and slot.  sends itemID, locationID (why locID?)
    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    InventoryItemRef iRef = sItemFactory.GetItemRefFromID(args.arg1);
    if (iRef.get() == nullptr) {
        _log(CHARACTER__ERROR, "CharRemoveImplant - iRef not found for itemID %i", args.arg1);
        return nullptr;
    }

    CharacterRef cRef(call.client->GetChar());
    uint8 boosterSlot(iRef->GetAttribute(AttrBoosterness).get_uint32());  //booster slot
    if (!cRef->IsBoosterSlotAvaliable(boosterSlot)) {
        //{'FullPath': u'UI/Messages', 'messageID': 259242, 'label': u'OnlyOneBoosterActiveBody'}(u'You cannot consume the {typeName} as you are already using another similar booster {typeName2}.', None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}, u'{typeName2}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName2'}})
        throw UserError("OnlyOneBoosterActiveBody")
        .AddFormatValue("typeName", new PyString(iRef->itemName()));
    }


    /* not sure if these need to be applied in code or from effects...
     *    AttrBoosterDuration = 330,
     *    AttrBoosterShieldBoostAmountPenalty = 616,
     *    AttrBoosterEffectChance1 = 1089,                    //fittingUsageChanceAttributeID in dgmEffects table
     *    AttrBoosterEffectChance2 = 1090,                    //fittingUsageChanceAttributeID in dgmEffects table
     *    AttrBoosterEffectChance3 = 1091,                    //fittingUsageChanceAttributeID in dgmEffects table
     *    AttrBoosterEffectChance4 = 1092,                    //fittingUsageChanceAttributeID in dgmEffects table
     *    AttrBoosterEffectChance5 = 1093,                    //fittingUsageChanceAttributeID in dgmEffects table
     *    AttrBoosterAttribute1 = 1099,
     *    AttrBoosterAttribute2 = 1100,
     *    AttrBoosterAttribute3 = 1101,
     *    AttrBoosterAttribute4 = 1102,
     *    AttrBoosterAttribute5 = 1103,
     *    AttrBoosterChanceBonus = 1125,
     *    AttrBoosterAttributeModifier = 1126,
     *    AttrBoosterArmorHPPenalty = 1141,
     *    AttrBoosterArmorRepairAmountPenalty = 1142,
     *    AttrBoosterShieldCapacityPenalty = 1143,
     *    AttrBoosterTurretOptimalRange = 1144,
     *    AttrBoosterTurretTrackingPenalty = 1145,
     *    AttrBoosterTurretFalloffPenalty = 1146,
     *    AttrBoosterAOEVelocityPenalty = 1147,
     *    AttrBoosterMissileVelocityPenalty = 1148,
     *    AttrBoosterMissileAOECloudPenalty = 1149,
     *    AttrBoosterCapacitorCapacityPenalty = 1150,
     *    AttrBoosterMaxVelocityPenalty = 1151,
     *    AttrBoosterMaxCharAgeHours = 1647,
     */

    // remove item from inventory
    return nullptr;
}