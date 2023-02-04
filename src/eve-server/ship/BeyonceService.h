/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2023 Alasiya-EvE (by Allan)
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


#ifndef __BEYONCE_SERVICE_H_INCL__
#define __BEYONCE_SERVICE_H_INCL__

#include "ship/ShipDB.h"
#include "PyService.h"

class BeyonceService
: public PyService {
public:
    BeyonceService(PyServiceMgr *mgr);
    virtual ~BeyonceService();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    ShipDB m_db;

    //PyCallable_DECL_CALL()
    PyCallable_DECL_CALL(GetFormations)

    //overloaded in order to support bound objects:
    virtual PyBoundObject *CreateBoundObject(Client *pClient, const PyRep *bind_args);
};


#endif

/*
 * {'FullPath': u'UI/Messages', 'messageID': 260057, 'label': u'CantWarpAfterShipBody'}(u'Your ship does not have the equipment to warp after another ship.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 260058, 'label': u'CantWarpWhileCloakedBody'}(u'You cannot warp while you are cloaked.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 260080, 'label': u'CantWarpAfterThatBody'}(u'You are unable to align or warp to the selected object because your warp drive is unable to lock onto it.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 278168, 'label': u'CantWarpAwayGlobalCriminalFlagTitle'}(u'Cannot Warp', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 278169, 'label': u'CantWarpAwayGlobalCriminalFlagBody'}(u'Your warp engines have been disabled due to your recent criminal activity. The criminal timer will run out in {time}.', None, {u'{time}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'time'}})
 * {'FullPath': u'UI/Messages', 'messageID': 256672, 'label': u'WarpTooShortBody'}(u'Something is affecting your warp, causing your warp distance to be too short.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 256744, 'label': u'WarpPathDisruptedBody'}(u'You cannot enter warp because {object} is intersecting your warp path. ', None, {u'{object}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'object'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258023, 'label': u'WarpScrambledByWarpDisruptionFieldGeneratorBody'}(u'You are within a warp disruption zone. Get {range} meters from {[character]pilot.nameWithPossessive} {[item]ship.name} to warp.', None, {u'{[item]ship.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'ship'}, u'{range}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'range'}, u'{[character]pilot.nameWithPossessive}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'nameWithPossessive', 'args': 0, 'kwargs': {}, 'variableName': 'pilot'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258771, 'label': u'WarpDestinationGoneBody'}(u'Problems with locking onto warp destination. It might have vanished.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258783, 'label': u'WarpScrambledBody'}(u'External factors are preventing your warp drive from responding to this command.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258796, 'label': u'WarpingWithAvailablePowerBody'}(u'There is insufficient power to warp all the way to the target, proceeding to warp as far as power permits.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258797, 'label': u'WarpingWithinGlobalDisruptorsBody'}(u'You cannot warp there because natural phenomena are disrupting the warp.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258915, 'label': u'WarpToStargateWhileFlaggedBody'}(u'You are warping to a stargate owned by {owner} whom you have recently engaged in combat. Customs police are operating on a low tolerance policy and will attack you if they are present.<br><br>Do you wish to proceed with this dangerous action?', None, {u'{owner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'owner'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258928, 'label': u'WarpDisruptedBody'}(u'You are within a warp disruption zone. Get {range} meters from {object} to warp.', None, {u'{object}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'object'}, u'{range}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'range'}})
 *
 * {'FullPath': u'UI/Messages', 'messageID': 258929, 'label': u'WarpScrambledSuccessBody'}(u'You have started trying to warp scramble {scrambled}.', None, {u'{scrambled}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'scrambled'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258933, 'label': u'WarpScrambledOtherByBody'}(u'{scrambler} has started trying to warp scramble "{scrambled}"', None, {u'{scrambled}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'scrambled'}, u'{scrambler}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'scrambler'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258954, 'label': u'WarpScrambledByBody'}(u'{scrambler} has started trying to warp scramble you!', None, {u'{scrambler}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'scrambler'}})
 *
 * {'FullPath': u'UI/Agents', 'messageID': 235586, 'label': u'TooCloseToWarp'}(u'Too close to warp', None, None)
 * 
 */

