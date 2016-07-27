//  borrowed file parts.  -allan
/**
 * =========================================================================
 * File        : lib_errors.h
 * Project     : 0 A.D.
 * Description : error handling system: defines error codes, associates
 *             : them with descriptive text, simplifies error notification.
 * =========================================================================
 */

// license: GPL; see lib/license.txt

/**

Error handling system


Introduction
------------

This module defines error codes, translates them to/from other systems
(e.g. errno), provides several macros that simplify returning errors /
checking if a function failed, and associates codes with descriptive text.


Why Error Codes?
----------------

To convey information about what failed, the alternatives are unique
integral codes and direct pointers to descriptive text. Both occupy the
same amount of space, but codes are easier to internationalize.


Method of Propagating Errors
----------------------------

When a low-level function has failed, this must be conveyed to the
higher-level application logic across several functions on the call stack.
There are two alternatives:
1) check at each call site whether a function failed;
   if so, return to the caller.
2) throw an exception.

We will discuss the advantages and disadvantages of exceptions,
which mirror those of call site checking.
- performance: they shouldn't be used in time-critical code.
+ predictability: exceptions can come up almost anywhere,
  so it is hard to say what execution path will be taken.
- interoperability: not compatible with other languages.
+ readability: cleans up code by separating application logic and
  error handling. however, this is also a disadvantage because it
  may be difficult to see at a glance if a piece of code does
  error checking at all.
+ visibility: errors are more likely to be seen than relying on
  callers to check return codes; less reliant on discipline.

Both have their place. Our recommendation is to throw error code
exceptions when checking call sites and propagating errors becomes tedious.
However, inter-module boundaries should always return error codes for
interoperability with other languages.

Simplifying Call-Site Checking
------------------------------

As mentioned above, this approach requires discipline. We provide
macros to simplify this task: function calls can be wrapped in an
"enforcer" that checks whether they succeeded and can take action
(e.g. returning to caller or warning user) as appropriate.

Consider the following example:
LibError ret = doWork();
if(ret != INFO::OK) { warnUser(ret); return ret; }
This can be replaced by:
CHECK_ERR(doWork());

This provides a visible sign that the code handles errors,
automatically propagates errors back to the caller, and most importantly,
allows warning the user whenever an error occurs.
Thus, no errors can be swept under the carpet by failing to
check return value or catch(...) all exceptions.


When to warn the user?
----------------------

When a function fails, there are 2 places we can raise a warning:
as soon as the error condition is known, or in the higher-level caller.
The former is the WARN_RETURN(ERR::FAIL) approach, while the latter
corresponds to the example above.

We prefer the former because it is easier to ensure that all
possible return paths have been covered: search for all "return ERR::*"
that are not followed by a "// NOWARN" comment. Also, the latter approach
raises the question of where exactly to issue the warning.
Clearly API-level routines must raise the warning, but sometimes they will
want to call each other. Multiple warnings along the call stack ensuing
from the same root cause are not nice.

Note the special case of "validator" functions that e.g. verify the
state of an object: we now discuss pros/cons of just returning errors
without warning, and having their callers take care of that.
+ they typically have many return paths (-> increased code size)
- this is balanced by validators that have many call sites.
- we want all return statements wrapped for consistency and
easily checking if any were forgotten
- adding // NOWARN to each validator return statement would be tedious.
- there is no advantage to checking at the call site; call stack indicates
which caller of the validator failed anyway.
Validator functions should therefore also use WARN_RETURN.


Numbering Scheme
----------------

Each module header defines its own error codes to avoid a full rebuild
whenever a new obscure code is added.

Error codes start at -100000 (warnings are positive, but reserves a
negative value; absolute values are unique). This avoids collisions
with all known error code schemes.

Each header gets 100 possible values; the tens value may be
used to denote groups within that header.

The subsystem is denoted by the ten-thousands digit: 1 for file,
2 for other resources (e.g. textures), 3 for sysdep, ..

To summarize: +/-1SHHCC (S=subsystem, HH=header, CC=code number)
*/

#ifndef EVE_CORE_ERROR_DEFS_H_
#define EVE_CORE_ERROR_DEFS_H_

//  Alasiya EvE error code defs
/*  Error Code layout  (wip)
 *    code # ABCDE
 *      A = category (0-server, 1-player, 2-command, 3-destiny, 4-cosmic mgr, 5-market, 6-inventory, 7-service, 8-, 9-other,)
 *      B = system (0-character, 1-location, 2-system, 3-item, 4-, 5-ship, 6-, 7-, 8-, 9-)
 *      C = subsystem (0-create, 1-destroy, 2-move, 3-change, 4-, 5-insurance, 6-modules, 7-pilot, 8-, 9-)
 *      D = type (0-null, 1-calculate, 2-, 3-, 4-, 5-, 6-, 7-, 8-self, 9-target)
 *      E = error (0-undef, 1-not init, 2-oob, 3-, 4-, 5-, 6-, 7-, 8-, 9-not implemented)
 *
 *
 * Ref: ServerError 12321. << Client::BoardShip()
 * Ref: ServerError 25610. << MSAC::_calculateNewValue() - effectiveness is 0
 * Ref: ServerError 25620. << MMAC::_calculateNewValue() - effectiveness is 0
 * Ref: ServerError 31110. << commandDispatcher::Execute()
 * Ref: ServerError 35412. << DestinyManager::_Orbit() - distance checks oob
 * Ref: ServerError 65282. << ShipItem::ModifyHoldVolumeByFlag() - flag not in map
 * Ref: ServerError 75520. << Ship::InsureShip() - fraction is 0
 * Ref: ServerError 75521. << Ship::InsureShip() - fraction is < 0.05 -- ship is insured @ 30% (which gives error in client)
 */
//  client->SendErrorMsg("Internal Server Error.  Ref: ServerError 65202"));


#endif  //EVE_CORE_ERROR_DEFS_H_
