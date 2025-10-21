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

#include "../eve-server.h"

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

PyResult SkillMgrBound::Handle_GetRespecInfo(PyCallArgs& call) {
    return m_db.GetRespecInfo(call.client->GetCharacterID());
}

PyResult SkillMgrBound::Handle_GetSkillQueueAndFreePoints(PyCallArgs &call) {
    return call.client->GetChar()->SendSkillQueue();
}

PyResult SkillMgrBound::Handle_GetEndOfTraining(PyCallArgs &call) {
    return new PyLong(call.client->GetChar()->GetEndOfTraining());
}

PyResult SkillMgrBound::Handle_GetSkillHistory(PyCallArgs& call) {
    return call.client->GetChar()->GetSkillHistory();
}

PyResult SkillMgrBound::Handle_CharStopTrainingSkill(PyCallArgs &call) {
    // called when pausing skill queue
    call.client->GetChar()->PauseSkillQueue();
    // returns nothing
    return nullptr;
}

PyResult SkillMgrBound::Handle_CharStartTrainingSkill(PyCallArgs& call) {
    // sm.GetService('godma').GetSkillHandler().CharStartTrainingSkill(skillX.itemID, skillX.locationID)
    Call_TwoIntegerArgs args;
    if (!args.Decode(call.tuple))
    {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
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
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
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
            _log(ITEM__ERROR, "%s: failed to load skill %u for injection.", call.client->GetName(), cur);
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
        cRef->AddToSkillQueue(el.typeID, el.level);
    }

    cRef->UpdateSkillQueueEndTime();
    PyTuple* tmp = new PyTuple(1);
        tmp->SetItem(0, new PyString("OnSkillTrainingSaved"));
    call.client->QueueDestinyEvent(&tmp);
    return nullptr;
}

PyResult SkillMgrBound::Handle_CharStartTrainingSkillByTypeID(PyCallArgs& call)
{
    // called when skill queue empty or paused
    // sends skill typeID to start training
    SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
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

    if (!m_db.ReportRespec(call.client->GetCharacterID()))
        return nullptr;

    CharacterRef cRef(call.client->GetChar());

    // type() is base, *Bonus are ancestry + bloodline, Custom* is player-set remap points
    //  remove these from sent values to get player's remap points
    uint8 intelligence(args.intelligence);
        intelligence -= cRef->type().GetAttribute(AttrIntelligence).get_uint32();
        intelligence -= cRef->GetAttribute(AttrIntelligenceBonus).get_uint32();
    uint8 perception(args.perception);
        perception -= cRef->type().GetAttribute(AttrPerception).get_uint32();
        perception -= cRef->GetAttribute(AttrPerceptionBonus).get_uint32();
    uint8 charisma(args.charisma);
        charisma -= cRef->type().GetAttribute(AttrCharisma).get_uint32();
        charisma -= cRef->GetAttribute(AttrCharismaBonus).get_uint32();
    uint8 willpower(args.willpower);
        willpower -= cRef->type().GetAttribute(AttrWillpower).get_uint32();
        willpower -= cRef->GetAttribute(AttrWillpowerBonus).get_uint32();
    uint8 memory(args.memory);
        memory -= cRef->type().GetAttribute(AttrMemory).get_uint32();
        memory -= cRef->GetAttribute(AttrMemoryBonus).get_uint32();

    // set character attributes to total remapped values
    cRef->SetAttribute(AttrCharisma, args.charisma);
    cRef->SetAttribute(AttrIntelligence, args.intelligence);
    cRef->SetAttribute(AttrMemory, args.memory);
    cRef->SetAttribute(AttrPerception, args.perception);
    cRef->SetAttribute(AttrWillpower, args.willpower);

    // custom is player's remapping points
    cRef->SetAttribute(AttrCustomCharismaBonus, charisma);
    cRef->SetAttribute(AttrCustomIntelligenceBonus, intelligence);
    cRef->SetAttribute(AttrCustomMemoryBonus, memory);
    cRef->SetAttribute(AttrCustomPerceptionBonus, perception);
    cRef->SetAttribute(AttrCustomWillpowerBonus, willpower);

    cRef->SaveAttributes();

    // no return value
    return nullptr;
}

PyResult SkillMgrBound::Handle_GetCharacterAttributeModifiers(PyCallArgs &call)
{
    SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    InventoryItemRef iRef(call.client->GetChar()->GetImplantForAttrib(args.arg));
    // modified client code for this
    //  for typeID, value in modifiers:
    PyList* list(new PyList());
    PyTuple* tuple(new PyTuple(2));
    if (iRef.get() != nullptr) {
        tuple->SetItem(0, new PyInt(iRef->typeID()));
        tuple->SetItem(1, iRef->GetAttribute(args.arg + 11).GetPyObject());
        list->AddItem(tuple);
    } else {
        tuple->SetItem(0, PyStatic.NewZero());
        tuple->SetItem(1, PyStatic.NewZero());
        list->AddItem(tuple);
    }

    list->Dump(CHARACTER__TRACE, "    ");
    return list;
}

PyResult SkillMgrBound::Handle_CharAddImplant(PyCallArgs& call) {
    //client verifies implant, slot and skillInTraining.  sends itemid
    SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    InventoryItemRef iRef(sItemFactory.GetItemRefFromID(args.arg));
    if (iRef.get() == nullptr) {
        _log(CHARACTER__ERROR, "CharAddImplant - iRef not found for itemID %i", args.arg);
        return nullptr;
    }

    CharacterRef cRef(call.client->GetChar());
    uint8 implantSlot(iRef->GetAttribute(AttrImplantness).get_uint32());  //implant slot
    if (!cRef->IsSlotAvaliable(implantSlot)) {
        throw CustomError("You cannot install the <b>%s</b> because <b>%s</b> is installed in slot %u.", \
                        iRef->name(), cRef->GetImplantAtSlot(implantSlot)->name(), implantSlot);
    }

    // test skill level, if applicable
    if (iRef->GetAttribute(AttrRequiredSkill1Level).get_uint32() > cRef->GetSkillLevel(iRef->GetAttribute(AttrRequiredSkill1).get_uint32())) {
        throw CustomError("The implant <b>%s</b> requires the <b>%s</b> skill trained to level %u", \
                        iRef->name(), sDataMgr.GetSkillName(iRef->GetAttribute(AttrRequiredSkill1).get_uint32()), \
                        iRef->GetAttribute(AttrRequiredSkill1Level).get_uint32());
    }

    if (iRef->GetAttribute(AttrRequiredSkill2Level).get_uint32() > cRef->GetSkillLevel(iRef->GetAttribute(AttrRequiredSkill2).get_uint32())) {
        throw CustomError("The implant <b>%s</b> requires the <b>%s</b> skill trained to level %u", \
                        iRef->name(), sDataMgr.GetSkillName(iRef->GetAttribute(AttrRequiredSkill2).get_uint32()), \
                        iRef->GetAttribute(AttrRequiredSkill2Level).get_uint32());
    }

    _log(CHARACTER__MESSAGE, "CharAddImplant - Adding %s(%i) to %s(%u) in slot %u", \
            iRef->name(), args.arg, cRef->name(), cRef->itemID(), implantSlot);

    cRef->AddImplant(implantSlot, iRef);

    // for learning implants, client does NOT update effects.

    //{'FullPath': u'UI/Messages', 'messageID': 259217, 'label': u'PrereqImplantMissingBody'}(u'Attempting to use this implant without the aid of a {typeName} will destroy your cerebral cortex. Please consider alternate methods of suicide.', None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})
    //{'FullPath': u'UI/Messages', 'messageID': 259604, 'label': u'ImplantHasSkillPrerequisitesBody'}(u'The implant {[item]item.name} requires the following {[numeric]skillCount -> "skill", "skills"}: {requiredSkills}.', None, {u'{[numeric]skillCount -> "skill", "skills"}': {'conditionalValues': [u'skill', u'skills'], 'variableType': 9, 'propertyName': None, 'args': 320, 'kwargs': {}, 'variableName': 'skillCount'}, u'{requiredSkills}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'requiredSkills'}, u'{[item]item.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'item'}})
    //{'FullPath': u'UI/Messages', 'messageID': 259217, 'label': u'PrereqImplantMissingBody'}(u'Attempting to use this implant without the aid of a {typeName} will destroy your cerebral cortex. Please consider alternate methods of suicide.', None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}})

    // i've noticed some offer bonus' for missions...27152, 27153
    // also look at hacking implants...27196

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

    InventoryItemRef iRef(sItemFactory.GetItemRefFromID(args.arg));
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
    if (sConfig.character.DeleteImplantOnRemoval)
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

    InventoryItemRef iRef(sItemFactory.GetItemRefFromID(args.arg1));
    if (iRef.get() == nullptr) {
        _log(CHARACTER__ERROR, "CharRemoveImplant - iRef not found for itemID %i", args.arg1);
        return nullptr;
    }

    CharacterRef cRef(call.client->GetChar());
    uint8 boosterSlot(iRef->GetAttribute(AttrBoosterness).get_uint32());  //booster slot
    if (!cRef->IsBoosterSlotAvaliable(boosterSlot)) {
        //{'FullPath': u'UI/Messages', 'messageID': 259242, 'label': u'OnlyOneBoosterActiveBody'}(u'You cannot consume the {typeName} as you are already using another similar booster {typeName2}.', None, {u'{typeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName'}, u'{typeName2}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'typeName2'}})
        throw CustomError("You cannot consume a %s as you are already using %s - a similar booster.", \
                        iRef->name(), cRef->GetBoosterAtSlot(boosterSlot)->name());
    }


    /* not sure if these need to be applied in code or from effects...
     *    AttrBoosterDuration = 330,
     *    AttrBoosterMaxCharAgeHours = 1647,
     */

    // remove item from inventory
    return nullptr;
}