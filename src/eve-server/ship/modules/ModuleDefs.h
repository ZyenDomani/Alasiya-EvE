/*
    ------------------------------------------------------------------------------------
    This file is for decoding the module states for Alasiya-EvE
    Copyright 2017  Alasiya-EVEmu Team
    ------------------------------------------------------------------------------------
    Author:     Allan
    Based on original idea and code from Aknor Jaden and Luck
*/

#ifndef MODULE_DEFS_H
#define MODULE_DEFS_H

#include "./utils/EvilNumber.h"

namespace ModStates {
    // internal charge states
    enum ChargeStates
    {
        CHG_UNLOADED                = 0,
        CHG_LOADED                  = 1,
        CHG_LOADING                 = 2,
        CHG_RELOADING               = 3
    };

    // internal module states
    // to be used in conjunction with effectState (when to apply effect)
        /* 'Online' is used for:
        * ACTIVE modules fitted and online, but not activated (using the PASSIVE effects only where applicable)
        * PASSSIVE modules fitted and online
        * RIG modules fitted (always online when fit)
        */
    enum ModuleStates
    {
        MOD_UNFITTED                = 0,
        MOD_OFFLINE                 = 1,    // module fitted, but NOT put online yet - NOT used for rigs
        MOD_ONLINE                  = 2,    // module online  - rigs are either online or unfitted.
        MOD_ACTIVATED               = 3,    // used only for activated ACTIVE modules (Overloaded mode is calculated separately)
        MOD_DEACTIVATING            = 4     // module transistioning from MOD_ACTIVATED to MOD_OFFLINE
    };

    enum ModulePowerLevel
    {
        MODULE_BANK_UNDEFINED       = 0,
        MODULE_BANK_LOW_POWER       = 1,
        MODULE_BANK_MEDIUM_POWER    = 2,
        MODULE_BANK_HIGH_POWER      = 3,
        MODULE_BANK_RIG             = 4,
        MODULE_BANK_SUBSYSTEM       = 5
    };
}

#endif
