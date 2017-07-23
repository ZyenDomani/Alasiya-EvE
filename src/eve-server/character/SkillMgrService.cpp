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
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "character/SkillMgrService.h"

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

PyBoundObject *SkillMgrService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
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

    PyCallable_REG_CALL(SkillMgrBound, InjectSkillIntoBrain);
    PyCallable_REG_CALL(SkillMgrBound, CharStartTrainingSkillByTypeID);
    PyCallable_REG_CALL(SkillMgrBound, CharStopTrainingSkill);
    PyCallable_REG_CALL(SkillMgrBound, GetEndOfTraining);
    PyCallable_REG_CALL(SkillMgrBound, GetSkillHistory);
    PyCallable_REG_CALL(SkillMgrBound, CharAddImplant);
    PyCallable_REG_CALL(SkillMgrBound, RemoveImplantFromCharacter);
    PyCallable_REG_CALL(SkillMgrBound, GetSkillQueueAndFreePoints);
    PyCallable_REG_CALL(SkillMgrBound, SaveSkillQueue);
    PyCallable_REG_CALL(SkillMgrBound, AddToEndOfSkillQueue);
    PyCallable_REG_CALL(SkillMgrBound, GetRespecInfo);
    PyCallable_REG_CALL(SkillMgrBound, RespecCharacter);
    PyCallable_REG_CALL(SkillMgrBound, GetCharacterAttributeModifiers);
}

SkillMgrBound::~SkillMgrBound()
{
    delete m_dispatch;
}

/** @todo redesign this so this is not needed */
void SkillMgrBound::Release()
{
    delete this;
}

PyResult SkillMgrBound::Handle_GetCharacterAttributeModifiers(PyCallArgs &call) {
    sLog.White( "SkillMgrBound::Handle_GetCharacterAttributeModifiers()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);
    // expected data: for (itemID, typeID, operation, value,) in modifiers:
    /*
     * client sends attrib# of stat in question...
            [PyString "GetCharacterAttributeModifiers"]
            [PyTuple 1 items]
              [PyInt 165]
     * we return this...
        [PyList 1 items]
          [PyTuple 4 items]
            [PyIntegerVar 1866309449]   << implantID
            [PyInt 9943]                << implantTypeID
            [PyInt 2]                   << operation
            [PyFloat 3]                 << value
            */
    return new PyList;
}

PyResult SkillMgrBound::Handle_CharStopTrainingSkill(PyCallArgs &call) {
//  look into this again, redesign so these calls arent needed.....
    CharacterRef ch = call.client->GetChar();

    // clear & update
    ch->PauseSkillQueue();  // this saves current queue to chrPausedSkillQueue as the next line deletes it.
    ch->ClearSkillQueue();
    ch->UpdateSkillQueue();

    return ch->GetSkillQueue();
 }

PyResult SkillMgrBound::Handle_GetEndOfTraining(PyCallArgs &call) {
    return new PyLong( call.client->GetChar()->GetEndOfTraining() );
}

PyResult SkillMgrBound::Handle_GetSkillHistory( PyCallArgs& call ) {
    return (call.client->GetChar()->GetSkillHistory());
}

PyResult SkillMgrBound::Handle_CharAddImplant( PyCallArgs& call )
{
    sLog.White( "SkillMgrBound::Handle_CharAddImplant()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);
    //takes itemid
    Call_SingleIntegerArg args;
    if( !args.Decode( &call.tuple ) )
    {
        codelog( CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName() );
        return nullptr;
    }

    return nullptr;
}

PyResult SkillMgrBound::Handle_RemoveImplantFromCharacter( PyCallArgs& call )
{
    sLog.White( "SkillMgrBound::Handle_RemoveImplantFromCharacter()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);
    //takes itemid
    Call_SingleIntegerArg args;
    if( !args.Decode( &call.tuple ) )
    {
        codelog( CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName() );
        return nullptr;
    }

    return nullptr;
}

PyResult SkillMgrBound::Handle_GetSkillQueueAndFreePoints(PyCallArgs &call) {
    // returns list of skills currently in the skill queue.
    return call.client->GetChar()->GetSkillQueue();
}

PyResult SkillMgrBound::Handle_SaveSkillQueue(PyCallArgs &call) {
    Call_SaveSkillQueue args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    CharacterRef ch = call.client->GetChar();
    ch->ClearSkillQueue();
    SkillQueue_Element el;
    std::vector<PyRep*>::const_iterator cur = args.queue->begin();
    for (; cur != args.queue->end(); cur++) {
        if (!el.Decode(*cur))         {
            _log(CLIENT__ERROR, "%s: Failed to decode element of SkillQueue (%u). Skipping.", call.client->GetName(), *cur);
            continue;
        }
        ch->AddToSkillQueue( el.typeID, el.level );
    }
    ch->UpdateSkillQueue();
    return nullptr;
}

PyResult SkillMgrBound::Handle_AddToEndOfSkillQueue(PyCallArgs &call) {
    Call_TwoIntegerArgs args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    CharacterRef ch = call.client->GetChar();
    ch->AddToSkillQueue(args.arg1, args.arg2);
    ch->UpdateSkillQueue();

    return nullptr;
}

PyResult SkillMgrBound::Handle_RespecCharacter(PyCallArgs &call)
{
    sLog.White( "SkillMgrBound::Handle_RespecCharacter()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);
    Call_RespecCharacter spec;
    if (!spec.Decode(call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

	CharacterRef cref = call.client->GetChar();
    //if (mySE->HasPilot() and mySE->GetPilot()->CanThrow())
    if (cref->GetSkillInTraining())
		throw(PyException(MakeUserError("RespecSkillInTraining")));

    // return early if this is an illegal call
    if (!m_db.ReportRespec(call.client->GetCharacterID()))
        return nullptr;
    uint8 multiplier = sConfig.character.statMultiplier;
    cref->SetAttribute(AttrCharisma, spec.charisma * multiplier);
    cref->SetAttribute(AttrIntelligence, spec.intelligence * multiplier);
    cref->SetAttribute(AttrMemory, spec.memory * multiplier);
    cref->SetAttribute(AttrPerception, spec.perception * multiplier);
    cref->SetAttribute(AttrWillpower, spec.willpower * multiplier);
    cref->SaveAttributes();

    // no return value
    return nullptr;
}

PyResult SkillMgrBound::Handle_GetRespecInfo( PyCallArgs& call )
{
    uint32 freeRespecs = 0;
    uint64 lastRespec = 0, nextRespec = 0;
    m_db.GetRespecInfo(call.client->GetCharacterID(), freeRespecs, lastRespec, nextRespec);

    PyDict* result = new PyDict;
    result->SetItemString( "lastRespecDate", new PyInt( lastRespec ) );
    result->SetItemString( "freeRespecs", new PyInt( freeRespecs ) );
    result->SetItemString( "nextTimedRespec", new PyLong( nextRespec ) );

    return result;
}

//13:43:18 L SkillMgrBound::Handle_CharStartTrainingSkillByTypeID(): size= 1, 0 = Integer(3308) <- this is skill#
PyResult SkillMgrBound::Handle_CharStartTrainingSkillByTypeID( PyCallArgs& call )
{
    CharacterRef ch = call.client->GetChar();

    ch->LoadPausedSkillQueue();
    ch->UpdateSkillQueue();
    //ch->GetSkillQueue();

    return nullptr;
}

PyResult SkillMgrBound::Handle_InjectSkillIntoBrain(PyCallArgs &call)
{
    Call_InjectSkillIntoBrain args;
    if (!args.Decode(&call.tuple)) {
        codelog( CLIENT__ERROR, "%s: failed to decode arguments", call.client->GetName() );
        return nullptr;
    }

    CharacterRef ch = call.client->GetChar();

    for (auto cur : args.skills)  {
        SkillRef skill = m_manager->item_factory->GetSkill(cur);
        if (!skill) {
            codelog( ITEM__ERROR, "%s: failed to load skill item %u for injection.", call.client->GetName(), cur );
            continue;
        }

        if (!ch->InjectSkillIntoBrain(skill)) {
            /** @todo build and send UserError about injection failure. */
            codelog(ITEM__ERROR, "%s: Injection of skill %u failed", call.client->GetName(), skill->itemID() );
        }
    }

    return nullptr;
}
