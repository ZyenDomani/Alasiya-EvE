
 /**
  * @name GenericModule.cpp
  *   base module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */

/*
# Ship Module Logging:
MODULE__ERROR=1
# Charge not found reported in MODULE__WARNING
MODULE__WARNING=1
MODULE__MESSAGE=0
# Mod Create/Populate and Undocking "OnlineModules" list dumped in MODULE__INFO
MODULE__INFO=1
# group tests, set module item online/offline, and Salvaging chance msgs in MODULE__DEBUG
MODULE__DEBUG=1
# Mod timer setting, Online/Offline calls and Effects msgs reported in MODULE__TRACE
MODULE__TRACE=1
*/
#include "Client.h"
#include "ship/modules/GenericModule.h"
#include "ship/modules/ActiveModule.h"


GenericModule::GenericModule(ModuleItemRef mRef, ShipItemRef sRef)
: m_repeat(0),
m_modRef(mRef),
m_shipRef(sRef),
m_chargeRef(InventoryItemRef(nullptr)),
m_ModuleState(Module::State::Unfitted),
m_ChargeState(Module::State::Unloaded),
m_linked(false),
m_linkMaster(false),
m_isWarpSafe(false),
m_overLoaded(false),
m_chargeLoaded(false),
m_hiPower(false),
m_medPower(false),
m_loPower(false),
m_rigSlot(false),
m_subSystem(false),
m_turret(false),
m_launcher(false)
{
    if (mRef->type().HasEffect(EVEEffectID::loPower)) {
        m_loPower = true;
    } else if (mRef->type().HasEffect(EVEEffectID::medPower)) {
        m_medPower = true;
    } else if (mRef->type().HasEffect(EVEEffectID::hiPower)) {
        m_hiPower = true;
        if (mRef->type().HasEffect(EVEEffectID::turretFitted))
            m_turret = true;
        else if (mRef->type().HasEffect(EVEEffectID::launcherFitted))
            m_launcher = true;
    } else if (mRef->type().HasEffect(EVEEffectID::rigSlot)) {
        m_rigSlot = true;
    } else if (mRef->type().HasEffect(EVEEffectID::subSystem)) {
        m_subSystem = true;
    }

    _log(MODULE__DEBUG, "Created GenericModule %p for item %s (%u).", this, mRef->name(), mRef->itemID());
}

GenericModule::~GenericModule()
{
    // i dont think we need this here...
    //Offline();
}

// this function must NOT throw
void GenericModule::Online()
{
    if (m_ModuleState == Module::State::Unfitted) {
        _log(MODULE__ERROR, "GenericModule::Online() called for unfitted module %u(%s).",itemID(), m_modRef->name());
        return;
    }
    if (m_ModuleState != Module::State::Offline) {
        _log(MODULE__MESSAGE, "GenericModule::Online() called for non-offline module %u(%s).  State is %s", \
                itemID(), m_modRef->name(), GetModuleStateName(m_ModuleState));
        return;     // already online
    }

    if (GetAttribute(AttrDamage) >= GetAttribute(AttrHP)) {
        m_shipRef->GetPilot()->SendNotifyMsg("Your %s is too damaged to be put online.", m_modRef->name());
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
            m_modRef->SetOnline(false, isRig());
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
            m_modRef->SetOnline(false, isRig());
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

    // update available ship resources.
    m_shipRef->SetAttribute(AttrCpuLoad, cpuNeed);
    m_shipRef->SetAttribute(AttrPowerLoad, pgNeed);

    // clear map before adding new shit...avoids duplicating
    //ClearModifiers(); // ClearModifiers DELETES AttrOnline and all ship-modified attribs from the map!!  (elusive error)
    m_modRef->SetOnline(true, isRig());
    m_ModuleState = Module::State::Online;
    _log(MODULE__MESSAGE, "GenericModule::Online() - %u(%s) cpu: %.2f, pg: %.2f", itemID(), m_modRef->name(), cpuNeed.get_float(), pgNeed.get_float());

    ProcessEffects(FX::State::Passive, true);
    ProcessEffects(FX::State::Online, true);
    if (m_ChargeState == Module::State::Loaded) {
        if (m_chargeRef.get() == nullptr) {
            _log(MODULE__ERROR, "GenericModule::Online() - module %u(%s) has ChargeState(CHG_LOADED) but m_chargeRef = NULL.", \
                    itemID(), m_modRef->name());
        } else {
            _log(MODULE__MESSAGE, "GenericModule::Online() - module %u(%s) loading charge %s.", itemID(), m_modRef->name(), m_chargeRef->name());
            m_chargeLoaded = true;
            m_chargeRef->ClearModifiers();
            for (auto it : m_chargeRef->type().m_stateFxMap) {
                fxData data = fxData();
                data.action = FX::Action::Invalid;
                data.srcRef = m_chargeRef;
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
    switch(m_ModuleState) {
        case Module::State::Unfitted: {
            m_modRef->SetOnline(false, isRig());
            _log(MODULE__WARNING, "GenericModule::Offline() called for unfitted module %u(%s).",itemID(), m_modRef->name());
            return;
        }
        case Module::State::Offline: {
            m_modRef->SetOnline(false, isRig());
            _log(MODULE__WARNING, "GenericModule::Offline() called for offline module %u(%s).",itemID(), m_modRef->name());
            return;
        }
            // these two should only be called for activeModules...
        case Module::State::Deactivating: {
            _log(MODULE__MESSAGE, "GenericModule::Offline() called for deactivating module %u(%s).",itemID(), m_modRef->name());
            if (IsActiveModule())
                GetActiveModule()->AbortCycle();
        }
        case Module::State::Activated: {
            _log(MODULE__MESSAGE, "GenericModule::Offline() called for active module %u(%s).",itemID(), m_modRef->name());
            if (IsActiveModule())
                GetActiveModule()->AbortCycle();
        }
    }

    m_isWarpSafe = false;
    m_ModuleState = Module::State::Deactivating;

    /* code for offlining module before MOD_OFFLINE state is set. */
    EvilNumber cpuNeed = (m_shipRef->GetAttribute(AttrCpuLoad) - GetAttribute(AttrCpu));
    EvilNumber pgNeed = (m_shipRef->GetAttribute(AttrPowerLoad) - GetAttribute(AttrPower));
    m_shipRef->SetAttribute(AttrCpuLoad, cpuNeed);
    m_shipRef->SetAttribute(AttrPowerLoad, pgNeed);

    _log(MODULE__MESSAGE, "GenericModule::Offline() - %u(%s) cpu: %.2f, pg: %.2f",itemID(), m_modRef->name(), cpuNeed.get_float(), pgNeed.get_float());

    // module MUST be cleared before loading fx to remove.
    m_modRef->ClearModifiers();
    if (m_ChargeState == Module::State::Loaded) {
        if (m_chargeRef.get() == nullptr) {
            _log(MODULE__ERROR, "GenericModule::Offline() - module %u(%s) has ChargeState(CHG_LOADED) but m_chargeRef = NULL.", \
                    itemID(), m_modRef->name());
        } else {
            m_chargeRef->ClearModifiers();
            /** @todo  this isnt right.  need to remove EXISTING modifier data.....NOT this new data.
             *    also, DONT reset modifiermap before removing, to use existing, modified data
             */
            for (auto it : m_chargeRef->type().m_stateFxMap) {
                fxData data = fxData();
                data.action = FX::Action::Invalid;
                data.srcRef = m_chargeRef;
                sFxProc.ParseExpression(m_chargeRef.get(), sFxDataMgr.GetExpression(it.second.postExpression), data, this);
            }
            //if (m_shipRef->GetPilot()->IsInSpace())
            sFxProc.ApplyEffects(m_chargeRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), m_shipRef->GetPilot()->IsInSpace());
        }
    }

    ProcessEffects(FX::State::Passive, false);
    ProcessEffects(FX::State::Online, false);
    /** @todo  this isnt right.  need to remove EXISTING modifier data.....NOT this new data.
     *    also, DONT reset modifiermap before remoing, to use existing, modified data
     */
    sFxProc.ApplyEffects(m_modRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), m_shipRef->GetPilot()->IsInSpace());

    m_ModuleState = Module::State::Offline;
    m_modRef->SetOnline(false, isRig());
}

void GenericModule::Overload()
{
    // need to clear item's effectMap here to avoid duplicating.
    m_modRef->m_modifiers.clear();
    ProcessEffects(FX::State::Overloaded, true);
    sFxProc.ApplyEffects(m_modRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), true);
}

void GenericModule::DeOverload()
{
    // need to clear item's effectMap here to avoid duplicating.
    m_modRef->m_modifiers.clear();
    ProcessEffects(FX::State::Overloaded, false);
    sFxProc.ApplyEffects(m_modRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), true);
}

void GenericModule::ProcessEffects(int8 state, bool active/*false*/)
{
    // get module/charge pre/post effects in state x
    std::map<uint16, Effect> effectMap;
    m_modRef->type().GetEffectMap(state, effectMap);
    _log(EFFECTS__TRACE, "GenericModule::ProcessEffects() called for %s. effects: %u, state: %s, online: %s", \
            m_modRef->name(), effectMap.size(), sFxProc.GetStateName(state), (active ? "true" : "false"));
    for (auto it : effectMap) {
        if (it.first == 16)    // skip the online effect.  this is done internally elsewhere
            continue;
        fxData data = fxData();
        data.action = FX::Action::Invalid;
        data.srcRef = m_modRef;
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
        if (newAmount < EvilZero)
            newAmount = EvilZero;
        SetAttribute(AttrDamage, newAmount);
    }
    _log(MODULE__DAMAGE, "GenericModule::Repair() - %s repaired %u damage.  new damage %u", m_modRef->name(), amount, GetAttribute(AttrDamage).get_int());
}

const char* GenericModule::GetModuleStateName(int8 state)
{
    using namespace Module;
    switch(state) {
        case State::Unloaded:       return "Unloaded";
        case State::Loaded:         return "Loaded";
        case State::Loading:        return "Loading";
        case State::Reloading:      return "Reloading";
        case State::Unfitted:       return "Unfitted";
        case State::Offline:        return "Offline";
        case State::Online:         return "Online";
        case State::Activated:      return "Activated";
        case State::Deactivating:   return "Deactivating";
        default:                    return "Invalid";
    }
}
