
 /**
  * @name GenericModule.cpp
  *   base module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */

/*
# Ship Module Logging:
SHIP__MODULE_ERROR=1
# Charge not found reported in SHIP__MODULE_WARNING
SHIP__MODULE_WARNING=1
SHIP__MODULE_MESSAGE=0
# Mod Create/Populate and Undocking "OnlineModules" list dumped in SHIP__MODULE_INFO
SHIP__MODULE_INFO=1
# group tests, set module item online/offline, and Salvaging chance msgs in SHIP__MODULE_DEBUG
SHIP__MODULE_DEBUG=1
# Mod timer setting, Online/Offline calls and Effects msgs reported in SHIP__MODULE_TRACE
SHIP__MODULE_TRACE=1
*/
#include "Client.h"
#include "ship/Ship.h"
#include "ship/modules/GenericModule.h"

GenericModule::GenericModule( InventoryItemRef item, ShipItemRef ship )
{
    m_repeat = 0;

    m_modRef = item;
    m_shipRef = ship;
    m_chargeRef = InventoryItemRef(nullptr);

    m_ModuleState = Module::State::Unfitted;
    m_ChargeState = Module::State::Unloaded;

    m_linked = false;
    m_linkMaster = false;
    m_isWarpSafe = false;
    m_overLoaded = false;
    m_chargeLoaded = false;

    m_hiPower = false;
    m_medPower = false;
    m_loPower = false;
    m_rigSlot = false;
    m_subSystem = false;
    m_turret = false;
    m_launcher = false;

    if (item->type().HasEffect(EVEEffectID::loPower)) {
        m_loPower = true;
    } else if (item->type().HasEffect(EVEEffectID::medPower)) {
        m_medPower = true;
    } else if (item->type().HasEffect(EVEEffectID::hiPower)) {
        m_hiPower = true;
        if (item->type().HasEffect(EVEEffectID::turretFitted))
            m_turret = true;
        else if (item->type().HasEffect(EVEEffectID::launcherFitted))
            m_launcher = true;
    } else if (item->type().HasEffect(EVEEffectID::rigSlot)) {
        m_rigSlot = true;
    } else if (item->type().HasEffect(EVEEffectID::subSystem)) {
        m_subSystem = true;
    }

    _log(SHIP__MODULE_DEBUG, "Created GenericModule %p for item %s (%u).", this, item->itemName().c_str(), item->itemID());
}

GenericModule::~GenericModule()
{
    Offline();
}

// this function must NOT throw
void GenericModule::Online()
{
    if (m_ModuleState == Module::State::Unfitted) {
        _log(SHIP__MODULE_ERROR, "GenericModule::Online() called for unfitted module %u(%s).",itemID(), m_modRef->itemName().c_str());
        return;
    }
    if (m_ModuleState != Module::State::Offline) {
        _log(SHIP__MODULE_MESSAGE, "GenericModule::Online() called for non-offline module %u(%s).  State is %s", \
                itemID(), m_modRef->itemName().c_str(), GetModuleStateName(m_ModuleState).c_str());
        return;     // already online
    }

    if (GetAttribute(AttrDamage) >= GetAttribute(AttrHP)) {
        m_shipRef->GetPilot()->SendNotifyMsg("Your %s is too damaged to be put online.", m_modRef->itemName().c_str());
        return;
        /*{'messageKey': 'ModuleTooDamagedToBeOnlined', 'dataID': 17878773, 'suppressable': False, 'bodyID': 257752, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2303}
         *   u'ModuleTooDamagedToBeOnlinedBody'}(u'The module is too damaged to be onlined'
         */
    }
    // check PG and CPU usage to see if we have enough to online this module
    // throwing an error negates further processing
    EvilNumber cpuNeed = (m_shipRef->GetAttribute(AttrCpuLoad) + GetAttribute(AttrCpu));
    if (cpuNeed  > m_shipRef->GetAttribute(AttrCpuOutput)) {
        if (!m_shipRef->GetPilot()->IsLogin()) {
            float require = GetAttribute(AttrCpu).get_float();
            float total = m_shipRef->GetAttribute(AttrCpuOutput).get_float();
            float remaining = total - m_shipRef->GetAttribute(AttrCpuLoad).get_float();
            std::string str = "To bring " + m_modRef->itemName() + " online requires %.2f cpu units, ";
            str += "but only %.2f of the %.2f units that your computer produces are still available.";
            m_shipRef->GetPilot()->SendNotifyMsg(str.c_str(), require, remaining, total);
            /*
            std::map<std::string, PyRep *> args;
            args["moduleType"] = new PyInt(typeID());
            args["require"] = new PyFloat(require);
            args["remaining"] = new PyFloat(remaining);
            args["total"] = new PyFloat(total);
            throw PyException( MakeUserError("NotEnoughCpu", args));
            */
            /*u'NotEnoughCpuBody'}
             * (u'To bring {[item]moduleType.name} online requires {[numeric]require, decimalPlaces=2} cpu units, but only {[numeric]remaining, decimalPlaces=2} of the {[numeric]total, decimalPlaces=2} units that your computer produces are still available.', None,
             * {u'{[numeric]remaining, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'remaining'},
             * u'{[item]moduleType.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleType'},
             * u'{[numeric]total, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'total'},
             * u'{[numeric]require, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'require'}})
             */
        }
        return;
    }
    EvilNumber pgNeed = (m_shipRef->GetAttribute(AttrPowerLoad) + GetAttribute(AttrPower));
    if (pgNeed > m_shipRef->GetAttribute(AttrPowerOutput)) {
        if (!m_shipRef->GetPilot()->IsLogin()) {
            float require = GetAttribute(AttrPower).get_float();
            float total = m_shipRef->GetAttribute(AttrPowerOutput).get_float();
            float remaining = total - m_shipRef->GetAttribute(AttrPowerLoad).get_float();
            std::string str = "To bring " + m_modRef->itemName() + " online requires %.2f power units, ";
            str += "but only %.2f of the %.2f units that your power core produces are still available.";
            m_shipRef->GetPilot()->SendNotifyMsg(str.c_str(), require, remaining, total);
            /*
            std::map<std::string, PyRep *> args;
            args["moduleType"] = new PyInt(typeID());
            args["require"] = new PyFloat(GetAttribute(AttrPower).get_float());
            args["remaining"] = new PyFloat(m_shipRef->GetAttribute(AttrPowerOutput).get_float() - m_shipRef->GetAttribute(AttrPowerLoad).get_float());
            args["total"] = new PyFloat(m_shipRef->GetAttribute(AttrPowerOutput).get_float());
            throw PyException( MakeUserError("NotEnoughPower", args));
            */
        }
        return;
    }

    // update avalible ship resources.
    m_shipRef->SetAttribute(AttrCpuLoad, cpuNeed);
    m_shipRef->SetAttribute(AttrPowerLoad, pgNeed);

    // clear map before adding new shit...avoids duplicating
    //ClearModifiers(); // ClearModifiers DELETES AttrIsOnline and all ship-modified attribs from the map!!  (elusive error)
    m_modRef->PutOnline(isRig());
    m_ModuleState = Module::State::Online;
    _log(SHIP__MODULE_TRACE, "GenericModule::Online() - %u(%s) cpu: %.2f, pg: %.2f",itemID(), m_modRef->itemName().c_str(), cpuNeed.get_float(), pgNeed.get_float());

    ProcessEffects(Effects::dgmStatePassive, true);
    ProcessEffects(Effects::dgmStateOnline, true);
    if (m_ChargeState == Module::State::Loaded) {
        if (m_chargeRef.get() == nullptr) {
            _log(SHIP__MODULE_ERROR, "GenericModule::Online() - module %u(%s) has ChargeState(ChargeStates::CHG_LOADED) but m_chargeRef = NULL.", \
                    itemID(), m_modRef->itemName().c_str());
        } else {
            _log(SHIP__MODULE_INFO, "GenericModule::Online() - module %u(%s) loading charge %s.", itemID(), m_modRef->itemName().c_str(), m_chargeRef->itemName().c_str());
            m_chargeLoaded = true;
            m_chargeRef->ClearModifiers();
            for (auto it : m_chargeRef->type().m_stateFxMap) {
                fxData data;
                data.action = Effects::Action::dgmActInvalid;
                data.srcRef = m_chargeRef;
                data.math = data.targLoc = data.targAttr = data.srcAttr = data.grpID = data.typeID = data.fxSrc = 0;
                sFxProc.ParseExpression(m_modRef.get(), sFxDataMgr.GetExpression(it.second.preExpression), data, this);
            }
            //if (m_shipRef->GetPilot()->IsLogin())
            //sFxProc.ApplyEffects(m_chargeRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), m_shipRef->GetPilot()->IsInSpace());
        }
    }
    // process passive and online effects AFTER charge is loaded and charge effects are applied. (in the case of charge modifying module - elusive error)
    sFxProc.ApplyEffects(m_modRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), m_shipRef->GetPilot()->IsInSpace());
}

void GenericModule::Offline()
{
    if (m_ModuleState == Module::State::Offline) {
        _log(SHIP__MODULE_WARNING, "GenericModule::Offline() called for offline module %u(%s).",itemID(), m_modRef->itemName().c_str());
        return;
    }
    if (m_ModuleState == Module::State::Unfitted) {
        _log(SHIP__MODULE_WARNING, "GenericModule::Offline() called for unfitted module %u(%s).",itemID(), m_modRef->itemName().c_str());
        return;
    }
    if (m_ModuleState == Module::State::Deactivating) {
        _log(SHIP__MODULE_MESSAGE, "GenericModule::Offline() called for deactivating module %u(%s).",itemID(), m_modRef->itemName().c_str());
        m_ModuleState = Module::State::Offline;
        m_modRef->PutOffline();
        return;
    }

    m_isWarpSafe = false;
    m_ModuleState = Module::State::Deactivating;

    /* code for offlining module before MOD_OFFLINE state is set. */
    EvilNumber cpuNeed = (m_shipRef->GetAttribute(AttrCpuLoad) - GetAttribute(AttrCpu));
    EvilNumber pgNeed = (m_shipRef->GetAttribute(AttrPowerLoad) - GetAttribute(AttrPower));
    m_shipRef->SetAttribute(AttrCpuLoad, cpuNeed);
    m_shipRef->SetAttribute(AttrPowerLoad, pgNeed);

    _log(SHIP__MODULE_TRACE, "GenericModule::Offline() - %u(%s) cpu: %.2f, pg: %.2f",itemID(), m_modRef->itemName().c_str(), cpuNeed.get_float(), pgNeed.get_float());

    // module MUST be cleared before loading fx to remove.
    m_modRef->ClearModifiers();
    if (m_ChargeState == Module::State::Loaded) {
        if (m_chargeRef.get() == nullptr) {
            _log(SHIP__MODULE_ERROR, "GenericModule::Offline() - module %u(%s) has ChargeState(ChargeStates::CHG_LOADED) but m_chargeRef = NULL.", \
                    itemID(), m_modRef->itemName().c_str());
        } else {
            m_chargeRef->ClearModifiers();
            /** @todo  this isnt right.  need to remove EXISTING modifier data.....NOT this new data.
             *    also, DONT reset modifiermap before remoing, to use existing, modified data
             */
            for (auto it : m_chargeRef->type().m_stateFxMap) {
                fxData data;
                data.action = Effects::Action::dgmActInvalid;
                data.srcRef = m_chargeRef;
                data.math = data.targLoc = data.targAttr = data.srcAttr = data.grpID = data.typeID = data.fxSrc = 0;
                sFxProc.ParseExpression(m_chargeRef.get(), sFxDataMgr.GetExpression(it.second.postExpression), data, this);
            }
            //if (m_shipRef->GetPilot()->IsInSpace())
            sFxProc.ApplyEffects(m_chargeRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), m_shipRef->GetPilot()->IsInSpace());
        }
    }

    ProcessEffects(Effects::dgmStatePassive, false);
    ProcessEffects(Effects::dgmStateOnline, false);
    /** @todo  this isnt right.  need to remove EXISTING modifier data.....NOT this new data.
     *    also, DONT reset modifiermap before remoing, to use existing, modified data
     */
    sFxProc.ApplyEffects(m_modRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), m_shipRef->GetPilot()->IsInSpace());

    m_ModuleState = Module::State::Offline;
    m_modRef->PutOffline();
    // fix for not being able to Online modules in fit window when docked
    if (m_shipRef->IsDocking())
        m_modRef->SetAttribute(AttrIsOnline, true, true);
}

void GenericModule::Overload()
{
    // need to clear item's effectMap here to avoid duplicating.
    m_modRef->m_modifiers.clear();
    ProcessEffects(Effects::dgmStateOverloaded, true);
    sFxProc.ApplyEffects(m_modRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), true);
}

void GenericModule::DeOverload()
{
    // need to clear item's effectMap here to avoid duplicating.
    m_modRef->m_modifiers.clear();
    ProcessEffects(Effects::dgmStateOverloaded, false);
    sFxProc.ApplyEffects(m_modRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), true);
}

void GenericModule::ProcessEffects(Effects::State state, bool active/*false*/)
{
    // get module/charge pre/post effects in state x
    std::map<uint16, Effect> effectMap;
    m_modRef->type().GetEffectMap(state, effectMap);
    _log(EFFECTS__TRACE, "GenericModule::ProcessEffects() called for %s. effects: %u, state: %s, online: %s", \
            m_modRef->itemName().c_str(), effectMap.size(), sFxProc.GetStateName(state).c_str(), (active ? "true" : "false"));
    fxData data;
    data.action = Effects::Action::dgmActInvalid;
    data.srcRef = m_modRef;
    for (auto it : effectMap) {
        if (it.first == 16)    // skip the online effect.  this is done internally elsewhere
            continue;
        data.math = data.targLoc = data.targAttr = data.srcAttr = data.grpID = data.typeID = data.fxSrc = 0;
        /* module and charge effects will be added/removed from it's item
         * active/overload/gang/other effects will be applied and removed when called.
         */
        if (active)
            sFxProc.ParseExpression(m_modRef.get(), sFxDataMgr.GetExpression(it.second.preExpression), data, this);
        else
            sFxProc.ParseExpression(m_modRef.get(), sFxDataMgr.GetExpression(it.second.postExpression), data, this);
    }
}

// not used
void GenericModule::Repair(EvilNumber amount)
{
    if (GetAttribute(AttrDamage) > 0) {
        EvilNumber newAmount = GetAttribute(AttrDamage) - amount;
        if (newAmount < 0)
            newAmount = 0;
        SetAttribute(AttrDamage, newAmount);
    }
    _log(SHIP__MODULE_DAMAGE, "GenericModule::Repair() - %s repaired %u damage.  new damage %u",  \
                m_modRef->itemName().c_str(), amount, GetAttribute(AttrDamage).get_int());
}

std::string GenericModule::GetModuleStateName(int8 state)
{
    switch(state) {
        case Module::State::Unloaded:       return "Unloaded";      break;
        case Module::State::Loaded:         return "Loaded";        break;
        case Module::State::Loading:        return "Loading";       break;
        case Module::State::Reloading:      return "Reloading";     break;
        case Module::State::Unfitted:       return "Unfitted";      break;
        case Module::State::Offline:        return "Offline";       break;
        case Module::State::Online:         return "Online";        break;
        case Module::State::Activated:      return "Activated";     break;
        case Module::State::Deactivating:   return "Deactivating";  break;
    }
}
