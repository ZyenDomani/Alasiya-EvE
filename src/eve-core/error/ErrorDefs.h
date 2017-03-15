
 /**
  * @name ErrorDefs.h
  *   system for managing error defs for server
  * this header defines error codes for interperting errors within the running server
  * @Author:         Allan
  * @date:   ?? May 2015
  */



#ifndef EVE_CORE_ERROR_DEFS_H_
#define EVE_CORE_ERROR_DEFS_H_

//  Alasiya EvE error code defs
/*  Error Code layout  (wip)
 *    code # ABCDE
 *      A = category (0-server, 1-player, 2-command, 3-destiny, 4-cosmic mgr, 5-market, 6-inventory, 7-service, 8-, 9-other)
 *      B = system (0-character, 1-location, 2-system, 3-item, 4-, 5-ship, 6-, 7-, 8-, 9-effects)
 *      C = subsystem (0-other, 1-self, 2-target, 3-charge, 4-calculate, 5-modules, 6-pilot, 7-skill, 8-bind, 9-timer)
 *      D = type (0-create, 1-destroy, 2-check, 3-change, 4-move, 5-, 6-, 7-, 8-, 9-)
 *      E = error (0-undef, 1-not init, 2-oob, 3-not found, 4-offline, 5-, 6-, 7-, 8-invalid, 9-not implemented)
 *
 *
 *   NOTE:  all defs will need to be redefined in code due to updates to defs above
 *
 * Ref: ServerError 02808. << CorpRegistryService::_CreateBoundObject - object is not tuple
 * Ref: ServerError 10781. << Client::BoardShip()
 * Ref: ServerError 10928. << Client::MoveTimer() - no move pending
 * Ref: ServerError 15610. << ShipItem::GetEffectiveness() - effectiveness <= 0
 * Ref: ServerError 15611. << ShipItem::GetEffectiveness() - module has invalid state
 * Ref: ServerError 15623. << MSAC::ModifyTargetShipAttribute() - no target
 * Ref: ServerError 15693. << EnergyTurret::Activate() - Cannot find loaded charge for this module
 * Ref: ServerError 25163. << ShipItem::ValidateAddItem() - FitModuleSkillCheck = false
 * Ref: ServerError 25165. << ShipItem::ValidateAddItem() - ValidateItemSpecifics = false
 * Ref: ServerError 25153. << m_ModuleManager->GetModule(flag) = nullptr
 * Ref: ServerError 25164. << ModuleManager::Activate() - module offline
 * Ref: ServerError 25263. << ModuleManager::Activate() - pSE == NULL
 * Ref: ServerError 25268. << ModuleManager::Activate() - targetID == 0
 * Ref: ServerError 31110. << commandDispatcher::Execute()
 * Ref: ServerError 35412. << DestinyManager::_Orbit() - distance checks oob
 * Ref: ServerError 65282. << ShipItem::ModifyHoldVolumeByFlag() - flag not in map
 * Ref: ServerError 75520. << Ship::InsureShip() - fraction is 0
 * Ref: ServerError 75521. << Ship::InsureShip() - fraction is < 0.05 -- ship is insured @ 30% (which gives error in client)
 */
//  client->SendErrorMsg("Internal Server Error.  Ref: ServerError 65202"));


#endif  //EVE_CORE_ERROR_DEFS_H_
