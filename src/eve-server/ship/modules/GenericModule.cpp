
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
    m_chargeRef = InventoryItemRef();

    m_ModuleState = ModStates::ModuleStates::MOD_UNFITTED;
    m_ChargeState = ModStates::ChargeStates::CHG_UNLOADED;

    m_overLoaded = false;
    m_chargeLoaded = false;

    m_hiPower = false;
    m_medPower = false;
    m_loPower = false;
    m_rigSlot = false;
    m_subSystem = false;
    m_turret = false;
    m_launcher = false;

    if (item->type().HasEffect(EVEEffectID::loPower))
        m_loPower = true;
    else if (item->type().HasEffect(EVEEffectID::medPower))
        m_medPower = true;
    else if (item->type().HasEffect(EVEEffectID::hiPower))
        m_hiPower = true;
    else if (item->type().HasEffect(EVEEffectID::rigSlot))
        m_rigSlot = true;
    else if (item->type().HasEffect(EVEEffectID::subSystem))
        m_subSystem = true;

    if (item->type().HasEffect(EVEEffectID::turretFitted))
        m_turret = true;
    else if (item->type().HasEffect(EVEEffectID::launcherFitted))
        m_launcher = true;

    _log(SHIP__MODULE_DEBUG, "Created GenericModule %p for item %s (%u).", this, item->itemName().c_str(), item->itemID());
}

GenericModule::~GenericModule()
{
    Offline();
}

void GenericModule::Online()
{
    if (m_ModuleState == ModStates::ModuleStates::MOD_UNFITTED) {
        _log(SHIP__MODULE_ERROR, "GenericModule::Online() called for unfitted module %u(%s).",m_modRef->itemID(), m_modRef->itemName().c_str());
        return;
    }
    if (m_ModuleState != ModStates::ModuleStates::MOD_OFFLINE) {
        _log(SHIP__MODULE_MESSAGE, "GenericModule::Online() called for non-offline module %u(%s).  State is %s", \
                m_modRef->itemID(), m_modRef->itemName().c_str(), GetModuleStateName(m_ModuleState).c_str());
        return;     // already online
    }
    // check for avalible resources to online this module.
    EvilNumber cpuNeed = (m_shipRef->GetAttribute(AttrCpuLoad) + GetAttribute(AttrCpu));
    if (cpuNeed < 0) {
        ; // make error for not enough cpu
        m_modRef->PutOffline();
        return;
    }
    EvilNumber pgNeed = (m_shipRef->GetAttribute(AttrPowerLoad) + GetAttribute(AttrPower));
    if (cpuNeed < 0) {
        ; // make error for not enough pg
        m_modRef->PutOffline();
        return;
    }
    m_shipRef->SetAttribute(AttrCpuLoad, cpuNeed);
    m_shipRef->SetAttribute(AttrPowerLoad, pgNeed);

    // clear map before adding new shit...avoids duplicating
    //m_modRef->ClearModifiers(); // ClearModifiers DELETES AttrIsOnline and all ship-modified attribs from the map!!  (elusive error)
    m_modRef->PutOnline(isRig());
    m_ModuleState = ModStates::ModuleStates::MOD_ONLINE;
    _log(SHIP__MODULE_TRACE, "GenericModule::Online() - %u(%s) cpu: %.2f, pg: %.2f",m_modRef->itemID(), m_modRef->itemName().c_str(), cpuNeed.get_float(), pgNeed.get_float());

    // process passive and online effects AFTER charge is loaded and charge effects are applied. (in the case of charge modifying module - elusive error)
    ProcessEffects(Effects::dgmStatePassive, true);
    ProcessEffects(Effects::dgmStateOnline, true);
    sFxProc.ApplyEffects(m_modRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), m_shipRef->GetPilot()->IsInSpace());

    if (m_ChargeState == ModStates::ChargeStates::CHG_LOADED) {
        if (m_chargeRef.get() == nullptr) {
            _log(SHIP__MODULE_ERROR, "GenericModule::Online() - module %u(%s) has ChargeState(ChargeStates::CHG_LOADED) but m_chargeRef = NULL.", \
                    m_modRef->itemID(), m_modRef->itemName().c_str());
        } else {
            _log(SHIP__MODULE_INFO, "GenericModule::Online() - module %u(%s) loading charge %s.", m_modRef->itemID(), m_modRef->itemName().c_str(), m_chargeRef->itemName().c_str());
            m_chargeLoaded = true;
            m_chargeRef->ClearModifiers();
            for (auto it : m_chargeRef->type().m_stateFxMap) {
                fxData data;
                data.action = Effects::Action::dgmActInvalid;
                data.srcRef = m_chargeRef;
                data.math = data.targLoc = data.targAttr = data.srcAttr = data.grpID = data.typeID = data.fxSrc = 0;
                sFxProc.ParseExpression(m_chargeRef.get(), sFxDataMgr.GetExpression(it.second.preExpression), data, this);
            }
            //if (m_shipRef->GetPilot()->IsLogin())
                sFxProc.ApplyEffects(m_chargeRef.get(), m_shipRef->GetPilot()->GetChar().get(), m_shipRef.get(), m_shipRef->GetPilot()->IsInSpace());
        }
    }
}

void GenericModule::Offline()
{
    if (m_ModuleState == ModStates::ModuleStates::MOD_OFFLINE) {
        _log(SHIP__MODULE_WARNING, "GenericModule::Offline() called for offline module %u(%s).",m_modRef->itemID(), m_modRef->itemName().c_str());
        return;
    }
    if (m_ModuleState == ModStates::ModuleStates::MOD_UNFITTED) {
        _log(SHIP__MODULE_WARNING, "GenericModule::Offline() called for unfitted module %u(%s).",m_modRef->itemID(), m_modRef->itemName().c_str());
        return;
    }
    if (m_ModuleState == ModStates::ModuleStates::MOD_DEACTIVATING) {
        _log(SHIP__MODULE_MESSAGE, "GenericModule::Offline() called for deactivating module %u(%s).",m_modRef->itemID(), m_modRef->itemName().c_str());
        m_ModuleState = ModStates::ModuleStates::MOD_OFFLINE;
        m_modRef->PutOffline();
        return;
    }

    m_ModuleState = ModStates::ModuleStates::MOD_DEACTIVATING;

    /* code for offlining module before MOD_OFFLINE state is set. */
    EvilNumber cpuNeed = (m_shipRef->GetAttribute(AttrCpuLoad) - GetAttribute(AttrCpu));
    EvilNumber pgNeed = (m_shipRef->GetAttribute(AttrPowerLoad) - GetAttribute(AttrPower));
    m_shipRef->SetAttribute(AttrCpuLoad, cpuNeed);
    m_shipRef->SetAttribute(AttrPowerLoad, pgNeed);

    _log(SHIP__MODULE_TRACE, "GenericModule::Offline() - %u(%s) cpu: %.2f, pg: %.2f",m_modRef->itemID(), m_modRef->itemName().c_str(), cpuNeed.get_float(), pgNeed.get_float());

    m_modRef->ClearModifiers();
    if (m_ChargeState == ModStates::ChargeStates::CHG_LOADED) {
        if (m_chargeRef.get() == nullptr) {
            _log(SHIP__MODULE_ERROR, "GenericModule::Offline() - module %u(%s) has ChargeState(ChargeStates::CHG_LOADED) but m_chargeRef = NULL.", \
                    m_modRef->itemID(), m_modRef->itemName().c_str());
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

    m_ModuleState = ModStates::ModuleStates::MOD_OFFLINE;
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

void GenericModule::ProcessEffects(Effects::State state, bool online/*false*/)
{
    // get module/charge pre/post effects in state x
    std::map<uint16, Effect> effectMap;
    m_modRef->type().GetEffectMap(state, effectMap);
    _log(EFFECTS__TRACE, "GenericModule::ProcessEffects() called for %s. effects: %u, state: %s, online: %s", \
            m_modRef->itemName().c_str(), effectMap.size(), sFxProc.GetStateName(state).c_str(), (online ? "true" : "false"));
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
        if (online)
            sFxProc.ParseExpression(m_modRef.get(), sFxDataMgr.GetExpression(it.second.preExpression), data, this);
        else
            sFxProc.ParseExpression(m_modRef.get(), sFxDataMgr.GetExpression(it.second.postExpression), data, this);
    }
}

std::string GenericModule::GetChargeStateName(ModStates::ChargeStates state)
{
    switch(state) {
        case ModStates::ChargeStates::CHG_UNLOADED:     return "Unloaded";  break;
        case ModStates::ChargeStates::CHG_LOADED:       return "Loaded";    break;
        case ModStates::ChargeStates::CHG_LOADING:      return "Loading";   break;
        case ModStates::ChargeStates::CHG_RELOADING:    return "Reloading"; break;
    }
}

std::string GenericModule::GetModuleStateName(ModStates::ModuleStates state)
{
    switch(state) {
        case ModStates::ModuleStates::MOD_UNFITTED:     return "Unfitted";      break;
        case ModStates::ModuleStates::MOD_OFFLINE:      return "Offline";       break;
        case ModStates::ModuleStates::MOD_ONLINE:       return "Online";        break;
        case ModStates::ModuleStates::MOD_ACTIVATED:    return "Activated";     break;
        case ModStates::ModuleStates::MOD_DEACTIVATING: return "Deactivating";  break;
    }
}
