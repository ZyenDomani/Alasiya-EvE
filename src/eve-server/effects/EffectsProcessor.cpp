/**
 * @name EffectsProcessor.cpp
 *   This file is for decoding and processing the effect data
 *   Copyright 2017  Alasiya-EVEmu Team
 *
 * @Author:    Allan
 * @date:      24 January 2017
 *
 */

#include "Client.h"
#include "effects/EffectsActions.h"
#include "effects/EffectsProcessor.h"
#include "inventory/InventoryItem.h"
#include "character/Character.h"
#include "ship/Ship.h"
#include "ship/modules/GenericModule.h"

/*
 * # Effects Logging:
 * EFFECTS=0
 * EFFECTS__ERROR=1
 * EFFECTS__WARNING=0
 * EFFECTS__MESSAGE=0
 * EFFECTS__DEBUG=0
 * EFFECTS__TRACE=0
 */

void FxProc::ParseExpression(InventoryItem* pItem, Expression expression, fxData& data, GenericModule* pMod/*nullptr*/)
{
    bool skill = false, core = false, self = false, module = false, charge = false, isRig = false, subSys = false;
    switch (data.srcRef->categoryID()) {
        case EVEDB::invCategories::Module: {
            switch (data.srcRef->groupID()) {
                case EVEDB::invGroups::Rig_Armor:
                case EVEDB::invGroups::Rig_Astronautic:
                case EVEDB::invGroups::Rig_Drones:
                case EVEDB::invGroups::Rig_Electronics:
                case EVEDB::invGroups::Rig_Electronics_Superiority:
                case EVEDB::invGroups::Rig_Energy_Grid:
                case EVEDB::invGroups::Rig_Energy_Weapon:
                case EVEDB::invGroups::Rig_Hybrid_Weapon:
                case EVEDB::invGroups::Rig_Launcher:
                case EVEDB::invGroups::Rig_Mining:
                case EVEDB::invGroups::Rig_Projectile_Weapon:
                case EVEDB::invGroups::Rig_Security_Transponder:
                case EVEDB::invGroups::Rig_Shield: {
                    isRig = true;
                }
            }
            module = true;
        } break;
        case EVEDB::invCategories::Charge: {
            charge = true;
        } break;
        case  EVEDB::invCategories::Skill:
        case  EVEDB::invCategories::Implant: {  // cat::implant also covers grp::booster
            switch (data.srcRef->groupID()) {
                case EVEDB::invGroups::Mechanic:
                case EVEDB::invGroups::Electronics:
                case EVEDB::invGroups::Engineering:
                case EVEDB::invGroups::Navigation: {
                    if ((data.srcRef->typeID() == EVEDB::invTypes::typeTargeting)
                        or (data.srcRef->typeID() == EVEDB::invTypes::typeMultitasking))
                        core = false;
                    else
                        core = true;    // define core skills that may/may not be processed correctly (!HasReqSkill(thisType))
                } break;
            }
            skill = true;
        } break;
        case EVEDB::invCategories::Subsystem: {
            subSys = true;
        } break;
    }
    if (pItem == data.srcRef.get())
        self = true;
    using namespace Effects;
    switch(expression.operandID) {
        // these return the given expressionValue
        case operandDEFBOOL:   //23  this evaulates to 'true' (Bool(1))
        case operandDEFINT: {  //27  this is used as  0,1,2,{raceID}
            //  seems to be called only to online/offline modules (and screws up my Online/Offline code...)
            /*
            int8 value = atoi(expression.expressionValue.c_str());
            if (module) {
                if (value == 0)
                    data.srcRef->PutOffline();
                if (value == 1)
                    data.srcRef->PutOnline(isRig);
                return; // we are done at this point
            } */
        } break;
        case operandDEFASSOCIATION: { //21
            data.math = GetAssociationEnum(expression.expressionValue);
            if (data.math > MaxMathMethod) {
                Operand operand = sFxDataMgr.GetOperand(expression.operandID);
                _log(EFFECTS__ERROR, "FxProc::ParseExpression(): out of range mathOp: %s(%i) for operand %u (%s).", \
                        GetMathMethodName(data.math).c_str(), data.math, expression.operandID, operand.operandKey.c_str());
            }
        } break;
        case operandDEFENVIDX: {     //24
            data.targLoc = GetEnvironmentEnum(expression.expressionValue);
            if (data.targLoc > MaxTargLocation) {
                Operand operand = sFxDataMgr.GetOperand(expression.operandID);
                _log(EFFECTS__ERROR, "FxProc::ParseExpression(): out of range targLoc: %i for operand %u (%s).", \
                        data.targAttr, expression.operandID, operand.operandKey.c_str());
            }
        } break;
        // these provide the given expression*ID
        case operandDEFATTRIBUTE: {  //22
            if (expression.expressionAttributeID) {
                if (data.targAttr)  // always processed first
                    data.srcAttr = expression.expressionAttributeID;
                else
                    data.targAttr = expression.expressionAttributeID;
            } else
                _log(EFFECTS__ERROR, "FxProc::ParseExpression(): opATTR called with no expressionAttributeID defined");
        } break;
        case operandDEFGROUP: {      //26
            data.fxSrc = dgmSrcGroup;
            if (expression.expressionGroupID) {
                data.grpID = expression.expressionGroupID;
            } else if (expression.expressionValue != "") {
                ;   // will have to figure out how to do this one.
                _log(EFFECTS__WARNING, "FxProc::ParseExpression(): opGROUP using expressionValue %s called by %s",\
                        expression.expressionValue.c_str(), expression.expressionName.c_str());
            } else {
                _log(EFFECTS__ERROR, "FxProc::ParseExpression(): opGROUP called with no expressionGroupID or expressionValue defined");
            }
        } break;
        case operandDEFTYPEID: {     //29
            if (skill)
                data.fxSrc = dgmSrcSkill;
            if (expression.expressionTypeID) {
                data.typeID = expression.expressionTypeID;
            } else if (expression.expressionValue != "") {
                ;   // will have to figure out how to do this one.
                _log(EFFECTS__WARNING, "FxProc::ParseExpression(): opTYPEID using expressionValue %s", expression.expressionValue.c_str());
            } else {
                _log(EFFECTS__ERROR, "FxProc::ParseExpression(): opTYPEID called with no expressionTypeID or expressionValue defined");
            }
        } break;
        // do as stated
        case operandCOMBINE: { //17, %(arg1)s); (%(arg2)s      --executes two statements
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data, pMod);
            fxData data1;
            data1.action = Effects::Action::dgmActInvalid;
            data1.srcRef = data.srcRef;
            data1.math = data1.targLoc = data1.fxSrc = data1.targAttr = data1.srcAttr = data1.grpID = data1.typeID = 0;
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data1, pMod);
        } break;
        case operandGETTYPE: { //36, %(arg1)s.GetTypeID()  --used by SRLG in AORSM
            if (!data.typeID)
                data.typeID = data.srcRef->typeID();    // get items on ship that require SkillItem in srcRef
        } break;
        case operandLG: {    //48, %(arg1)s.LocationGroup.%(arg2)s  -- specify a group by grpID for a location'  used by ALGM
            data.fxSrc = dgmSrcGroup;
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data, pMod);   //source
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data, pMod);   //groupID
            //_log(EFFECTS__TRACE, "FxProc::ParseExpression(): LocationGroup: setting fxSrc to Group.  set: %s, skill: %s, module: %s, charge: %s, grp: %u", \
                    (self? "true" : "false"), (skill ? "true" : "false"), (module? "true" : "false"), (charge ? "true" : "false"), data.grpID);
        } break;
        case operandLS: {    //49, %(arg1)s.SkillRequiredLocationGroup[%(arg2)s]  --  specify a group by skillID for a location   used by ALRSM and AORSM
            data.fxSrc = dgmSrcSkill;
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data, pMod);   //source
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data, pMod);   //skillID
            //_log(EFFECTS__TRACE, "FxProc::ParseExpression(): SRLG: setting fxSrc to Skill.  set: %s, skill: %s, module: %s, charge: %s", \
                    (self? "true" : "false"), (skill ? "true" : "false"), (module? "true" : "false"), (charge ? "true" : "false"));
        } break;
        case operandATT:     //12, %(arg1)s->%(arg2)s               --(item:attribID)
        case operandEFF:     //31, %(arg2)s.%(arg1)s                --define association type
        case operandGA:      //34, %(arg1)s.%(arg2)s                --GetAttribute      (no known uses)
        case operandGET:     //35, %(arg1)s.%(arg2)s()              --used a lot.  eg. Get(Ship:101) means 'get attribute 101 on ShipItem'
        case operandIA: {    //40, %(arg1)s                         --used by AGIM
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data);
            if (expression.arg2)
                ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data);
        } break;
        case operandGM: {    //37, %(arg1)s.GetModule(%(arg2)s)      --used by subsystems as (GetModule(Ship:201):55)
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data, pMod);
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data, pMod);
        } break;
        // effect function calls.
        // here is where we'll actually add the modifier data to the item's map
        case operandAIM: {   //6,  AddItemModifier(env,%(arg1)s, %(arg2)s)
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data, pMod);
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data, pMod);
            if (charge) {
                data.fxSrc = dgmSrcSelf;
                if (data.targLoc != dgmTargLocOther)
                    data.targLoc = dgmTargLocShip;
            } else if (skill) {
                if (!data.fxSrc)
                    data.fxSrc = dgmSrcSkill;
                if (data.targLoc == dgmTargLocShip)
                    if ((!data.typeID) and (core))
                        data.targLoc = dgmTargLocOther;     // this skill is a core skill, and is not "required" so needs a special check to apply correctly
            }
            pItem->AddModifier(data);
        } break;
        case operandALGM:    //7,  (%(arg1)s).AddLocationGroupModifier (%(arg2)s)
        case operandALM:     //8,  (%(arg1)s).AddLocationModifier (%(arg2)s)
        case operandALRSM:   //9,  (%(arg1)s).AddLocationRequiredSkillModifier(%(arg2)s)
        case operandAORSM: { //11, (%(arg1)s).AddOwnerRequiredSkillModifier(%(arg2)s)
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data, pMod);
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data, pMod);
            if ((skill) and (!data.fxSrc))
                data.fxSrc = dgmSrcSkill;
            pItem->AddModifier(data);
        } break;
        case operandAGRSM:   //5,  [%(arg1)s].AGRSM(%(arg2)s)    --AddGangRequiredSkillModifier
        case operandAGIM: {  //3,  [%(arg1)s].AGIM(%(arg2)s)        --AddGangShipModifier
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data, pMod);
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data, pMod);
            data.fxSrc = dgmSrcGang;
            pItem->AddModifier(data);
        } break;
        case operandRSA: {   //64, %(arg1)s.%(arg2)s      -- used by AGRSM
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data, pMod);
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data, pMod);
        } break;
        // remove modifier calls only enabled for modules and charges.  will implement for implants and boosters when those systems are written.
        case operandRGGM:    //54, [%(arg1)s].RemoveGangGroupModifier(%(arg2)s)
        case operandRGIM:    //55, [%(arg1)s].RemoveGangShipModifier(%(arg2)s)
        case operandRGORSM:  //56, [%(arg1)s].RemoveGangOwnerRequiredSkillModifier(%(arg2)s)
        case operandRGRSM: { //57, [%(arg1)s].RemoveGangRequiredSkillModifier(%(arg2)s)
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data, pMod);
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data, pMod);
            data.fxSrc = dgmSrcGang;
            pItem->RemoveModifier(data);
        } break;
        case operandRIM: {   //58, (%(arg1)s).RemoveItemModifier (%(arg2)s)
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data, pMod);
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data, pMod);
            if (charge) {
                data.fxSrc = dgmSrcSelf;
                if (data.targLoc != dgmTargLocOther)
                    data.targLoc = dgmTargLocShip;
            } else if (skill) {
                if (!data.fxSrc)
                    data.fxSrc = dgmSrcSkill;
                if (data.targLoc == dgmTargLocShip)
                    if ((!data.typeID) and (core))
                        data.targLoc = dgmTargLocOther;     // this skill is a core skill, and is not "required" so needs a special check to apply correctly
            }
            pItem->RemoveModifier(data);
        } break;
        case operandRLGM:    //59, (%(arg1)s).RemoveLocationGroupModifier (%(arg2)s)
        case operandRLM:     //60, (%(arg1)s).RemoveLocationModifier (%(arg2)s)
        case operandRLRSM:   //61, (%(arg1)s).RemoveLocationRequiredSkillModifier(%(arg2)s)
        case operandRORSM: { //62, (%(arg1)s).RemoveOwnerRequiredSkillModifier(%(arg2)s)
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data, pMod);
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data, pMod);
            if ((skill) and (!data.fxSrc))
                data.fxSrc = dgmSrcSkill;
            pItem->RemoveModifier(data);
        } break;
        /*
        // these will need more work to properly code conditionals here.
        //  that is not a priority, as they are only used by effect 16 (Online), which is kinda covered (hacked) in GenericModule class.
        case operandOR: {    //'%(arg1)s OR %(arg2)s'       -- used with 'if' in arg2 as 'y'.   ((if x then y) OR z)  (used as "else" or elif)
            fxData arg1;
                arg1.result = 0;
                arg1.srcRef = data.srcRef;
                arg1.math = arg1.targLoc = arg1.fxSrc = arg1.targAttr = arg1.srcAttr = arg1.grpID = arg1.typeID = 0;
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), arg1, pMod);
            if (!arg1.result) {
                fxData arg2;
                    arg2.result = 0;
                    arg2.srcRef = data.srcRef;
                    arg2.math = arg2.targLoc = arg2.fxSrc = arg2.targAttr = arg2.srcAttr = arg2.grpID = arg2.typeID = 0;
                ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), arg2, pMod);
            }
        } break;
        case operandAND: {   //'(%(arg1)s) AND (%(arg2)s)'  -- used with 'if' in arg1 as 'x'.   (if (x AND y) then ....)
            fxData arg1;
                arg1.result = 0;
                arg1.srcRef = data.srcRef;
                arg1.math = arg1.targLoc = arg1.fxSrc = arg1.targAttr = arg1.srcAttr = arg1.grpID = arg1.typeID = 0;
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), arg1, pMod);
            fxData arg2;
                arg2.result = 0;
                arg2.srcRef = data.srcRef;
                arg2.math = arg2.targLoc = arg2.fxSrc = arg2.targAttr = arg2.srcAttr = arg2.grpID = arg2.typeID = 0;
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), arg2, pMod);
        } break;
        case operandIF: {    //'If(%(arg1)s), Then (%(arg2)s)'    -- std conditional.  (if x then y)
            fxData arg1;
                arg1.result = 0;
                arg1.srcRef = data.srcRef;
                arg1.math = arg1.targLoc = arg1.fxSrc = arg1.targAttr = arg1.srcAttr = arg1.grpID = arg1.typeID = 0;
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), arg1, pMod);
            if (arg1.result) {
                fxData data1;
                data1.result = 0;
                data1.srcRef = data.srcRef;
                data1.math = data1.targLoc = data1.fxSrc = data1.targAttr = data1.srcAttr = data1.grpID = data1.typeID = 0;
                ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data1, pMod);
            }
        } break;
        // trivial attribute operations
        case operandADD: {      //1, (%(arg1)s)+(%(arg2)s)
            // this isnt complete.
            fxData arg1;
                arg1.result = 0;
                arg1.srcRef = data.srcRef;
                arg1.math = arg1.targLoc = arg1.fxSrc = arg1.targAttr = arg1.srcAttr = arg1.grpID = arg1.typeID = 0;
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), arg1, pMod);
            fxData arg2;
                arg2.result = 0;
                arg2.srcRef = data.srcRef;
                arg2.math = arg2.targLoc = arg2.fxSrc = arg2.targAttr = arg2.srcAttr = arg2.grpID = arg2.typeID = 0;
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), arg2, pMod);
            data.result = (arg1.result + arg2.result);
        } break;
        case operandGTE: {  //39    %(arg1)s>=%(arg2)s
            fxData arg1;
                arg1.result = 0;
                arg1.srcRef = data.srcRef;
                arg1.math = arg1.targLoc = arg1.fxSrc = arg1.targAttr = arg1.srcAttr = arg1.grpID = arg1.typeID = 0;
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), arg1, pMod);
            fxData arg2;
                arg2.result = 0;
                arg2.srcRef = data.srcRef;
                arg2.math = arg2.targLoc = arg2.fxSrc = arg2.targAttr = arg2.srcAttr = arg2.grpID = arg2.typeID = 0;
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), arg2, pMod);
            //  this needs work
            //if (arg1.srcRef->GetAttribute(arg1.srcAttr) >= arg2.targLoc->GetAttribute(arg2.targAttr))
            //    data.result = true;

        } break;
        case operandGT: {   //38    %(arg1)s> %(arg2)s

        } break;
        */

        case operandUE: {   //73    UserError(%(arg1)s)
            // not using this yet.
        } break;
        case operandSKILLCHECK: {   //67    SkillCheck(%(arg1)s)
            //data.result = true;
        } break;
        // module action method calls...not used.
        case operandATTACK: // 13,
        case operandCARGOSCAN: // 14,
        case operandCHEATTELEDOCK: // 15,
        case operandCHEATTELEGATE: // 16,
        case operandDECLOAKWAVE: // 19,
        case operandECMBURST: // 30,
        case operandEMPWAVE: // 32,
        case operandLAUNCH: // 44,
        case operandLAUNCHDEFENDERMISSILE: // 45,
        case operandLAUNCHDRONE: // 46,
        case operandLAUNCHFOFMISSILE: // 47,
        case operandMINE: // 50,
        case operandPOWERBOOST: // 53,
        case operandSHIPSCAN: // 66,
        case operandSURVEYSCAN: // 69,
        case operandTARGETHOSTILES: // 70,
        case operandTARGETSILENTLY: // 71,
        case operandTOOLTARGETSKILLS: // 72,
        case operandSPEEDBOOST: {   //75    Alasiya-specific operand to apply modified speed attribs to destiny variables and update bubble
            data.action = expression.operandID;
           // pItem->AddModifier(data);
        } break;
        default: {              // in case the op hasnt been defined, make a note here
            if (is_log_enabled(EFFECTS__UNDEFINED)) {
            std::ostringstream ret;
            Operand operand = sFxDataMgr.GetOperand(expression.operandID);
            ret << "Operand id:" << expression.operandID << " key:" << operand.operandKey;
            if (operand.format == "")
                ret << " - has not been defined.";
            else                // % {'arg1': arg1, 'arg2': arg2, 'value': expression.expressionValue}
                ret << " - should be added as " << operand.format.c_str();
                _log(EFFECTS__UNDEFINED, "FxProc::ParseExpression() - %s", ret.str().c_str());
            }
        } break;
    }
    /*
     * 22:32:33 E FxProc::ParseExpression: *** ERROR ***  Operand id:69 key:SURVEYSCAN - should be added as SurveyScan()
     * 21:51:29 [FxWarning] FxProc::ParseExpression() - *** ERROR ***  Operand id:J key:VERIFYTARGETGROUP - should be added as VerifyTargetGroup().
     * 12:21:02 [FxWarning] FxProc::ParseExpression() - *** ERROR ***  Operand id:4 key:OR - should be added as %(arg1)s OR %(arg2)s
     *        //02:48:33 E FxProc::ParseExpression: *** ERROR ***  Operand id:* key:INC - should be added as %(arg1)s+=self.%(arg2)se
     * 23:14:11 [FxWarning] FxProc::ParseExpression(): opGROUP using expressionValue     None called by     None
     *
     */
}

// attrib nerf and caps arent needed, from what ive seen while testing.
void FxProc::ApplyEffects(InventoryItem* pItem, Character* pChar, ShipItem* pShip, bool update/*false*/)
{
    bool isRig = false, subSys = false, charge = false;
    using namespace Effects;
    //uint8 action = Action::dgmActInvalid;
    for (auto cur : pItem->m_modifiers) {  // k,v of assoc, data<math, src, targLoc, targAttr, srcAttr, grpID, typeID>
        /*
        if (cur.second.action) {
            action = cur.second.action;
            continue;
        } */
        switch (cur.second.srcRef->groupID()) {
            case EVEDB::invGroups::Rig_Armor:
            case EVEDB::invGroups::Rig_Astronautic:
            case EVEDB::invGroups::Rig_Drones:
            case EVEDB::invGroups::Rig_Electronics:
            case EVEDB::invGroups::Rig_Electronics_Superiority:
            case EVEDB::invGroups::Rig_Energy_Grid:
            case EVEDB::invGroups::Rig_Energy_Weapon:
            case EVEDB::invGroups::Rig_Hybrid_Weapon:
            case EVEDB::invGroups::Rig_Launcher:
            case EVEDB::invGroups::Rig_Mining:
            case EVEDB::invGroups::Rig_Projectile_Weapon:
            case EVEDB::invGroups::Rig_Security_Transponder:
            case EVEDB::invGroups::Rig_Shield:
            case EVEDB::invGroups::Damage_Control: {    // this is not a rig, but it is NOT nerfed.  easiest way to make this check
                isRig = true;
            } break;
        }
        switch (cur.second.srcRef->categoryID()) {
            case EVEDB::invCategories::Charge: {
                charge = true;
            } break;
            case EVEDB::invCategories::Subsystem: {
                subSys = true;
            } break;
        }
        InventoryItemRef srcItemRef = cur.second.srcRef;
        std::vector<InventoryItemRef> itemRefVec;
        // affected target depends on source.  get source and target(s) here.
        switch (cur.second.fxSrc) {
            case dgmSrcGroup: {     // not a source per se, but defines effect's target selection requirements
                // this is to apply modifiers to ship's modules of groupID defined in 'grpID'
                std::vector<InventoryItemRef> moduleList;
                pShip->GetModuleManager()->GetModuleListOfRefsAsc(&moduleList);
                for (auto mod : moduleList)
                    if (mod->groupID() == cur.second.grpID)
                        itemRefVec.push_back(mod);
            } break;
            case dgmSrcSkill: {    // source of this effect is skill, implant, or booster
                if (cur.second.typeID == EVEDB::invTypes::typeInvalid) {    //invalid
                    _log(EFFECTS__WARNING, "FxProc::ApplyEffects(): typeID is invalid");
                    continue;  // make error here
                }
                switch (cur.second.targLoc) {
                    //  apply the modifier to ....
                    case dgmTargLocSelf: {
                        // ....item itself
                        itemRefVec.push_back(cur.second.srcRef);
                    } break;
                    case dgmTargLocShip:  {
                        if (cur.second.typeID) {
                            // .....ship's modules that require skillID defined in "typeID"
                            pShip->GetModuleManager()->GetModuleListByReqSkill(cur.second.typeID, &itemRefVec);
                        } else {
                            // ..... ship that require skill in 'srcRef'
                            if (pShip->HasReqSkill(cur.second.srcRef->typeID()))
                                itemRefVec.push_back(static_cast<InventoryItemRef>(pShip));
                        }
                    } break;
                    case dgmTargLocChar: {
                        // ....char skills that require skill in 'srcRef' or defined in 'typeID'
                        uint16 skillID = cur.second.srcRef->typeID();
                        if (cur.second.typeID)
                            skillID = cur.second.typeID;
                        std::vector<InventoryItemRef> allSkills;
                        pChar->GetSkillsList(allSkills);
                        for (auto curSkill : allSkills)
                            if (curSkill->HasReqSkill(skillID))
                                itemRefVec.push_back(curSkill);
                    } break;
                    case dgmTargLocOther: {
                        // ....ship from 'core' pilot skills (electronics, mechanics, etc)
                        itemRefVec.push_back(static_cast<InventoryItemRef>(pShip));
                    } break;
                    case dgmTargLocCharge: {
                        // ....charges
                        // will need more testing to verify this.
                        std::map<EVEItemFlags, InventoryItemRef> charges;
                        pShip->GetModuleManager()->GetLoadedCharges(charges);
                        for (auto mod : charges)
                            if (mod.second->HasReqSkill(cur.second.typeID))
                                itemRefVec.push_back(mod.second);
                    } break;
                    case dgmTargLocTarget: {
                        // ...current target (focused, volatile...removed on 'invalid target')
                        itemRefVec.push_back(pShip->GetTargetRef());
                    } break;
                    case dgmTargLocInvalid: {   // null
                        _log(EFFECTS__WARNING, "FxProc::ApplyEffects(): target location invalid.");
                        continue;
                    } break;
                    default: {
                        _log(EFFECTS__ERROR, "FxProc::ApplyEffects(): target undefined.");
                    } break;
                }
            } break;
            case dgmSrcSelf: {  // source is module or charge
                //  apply the modifier to ....
                switch (cur.second.targLoc) {
                    case dgmTargLocShip:  {
                        // ....the ship the calling item is located in/on
                        itemRefVec.push_back(static_cast<InventoryItemRef>(pShip));
                    } break;
                    case dgmTargLocSelf: {
                        // ....item itself
                        itemRefVec.push_back(cur.second.srcRef);
                    } break;
                    case dgmTargLocCharge: {
                        // ....charge on src item
                        itemRefVec.push_back(pShip->GetModuleManager()->GetLoadedChargeOnModule(cur.second.srcRef->flag()));
                    } break;
                    case dgmTargLocOther: {
                        // ....module containing the src item (charge)
                        itemRefVec.push_back(pShip->GetModuleManager()->GetModule(cur.second.srcRef->flag())->GetSelf());
                    } break;
                    case dgmTargLocTarget: {
                        // ...current target (focused, volatile...removed on 'invalid target')
                        itemRefVec.push_back(pShip->GetTargetRef());
                    } break;
                    default: {
                        _log(EFFECTS__ERROR, "FxProc::ApplyEffects(): target undefined - %s.", GetSourceName(cur.second.targLoc).c_str());
                    } break;
                }
            } break;
            case dgmSrcShip: {      // source is a subsystem
                ;   // not sure how to do this on yet.  t3 ships arent implemented (actually blocked)
                _log(EFFECTS__DEBUG, "FxProc::ApplyEffects(): calling ship target.");
            } break;
            case dgmSrcGang: {      // source is a gang leader skill
                ;   //dgmTargLocSelf is ship of gang member to apply leader's skill bonuses to
                _log(EFFECTS__DEBUG, "FxProc::ApplyEffects(): calling gang target.");
            } break;
            case dgmSrcInvalid: {
                _log(EFFECTS__ERROR, "FxProc::ApplyEffects(): source location invalid.");
                continue;
            } break;
            // these are not used (not coded)
            case dgmSrcTarget:
            case dgmSrcOwner: {
                _log(EFFECTS__ERROR, "FxProc::ApplyEffects(): source location not coded.");
                continue;
            } break;
        }

        if (itemRefVec.empty()) {
            //_log(EFFECTS__TRACE, "FxProc::ApplyEffects(): target item vector empty.");
            continue;
        }

        // get srcAttr
        EvilNumber srcValue = srcItemRef->GetAttribute(cur.second.srcAttr);
        // check for inf/nan and then reset?  this will fuck up all previous fx processing on this value.
        if (srcValue.isNaN() or srcValue.isInf()) {
            srcValue = srcItemRef->GetDefaultAttribute(cur.second.srcAttr);
            _log(EFFECTS__ERROR, "FxProc::ApplyEffects(): srcValue isInf or isNaN.  Data: %s(%u) - src(%s:%u) set to %.3f.", \
            srcItemRef->itemName().c_str(), srcItemRef->itemID(), GetSourceName(cur.second.fxSrc).c_str(), cur.second.srcAttr, srcValue.get_float());
        }

        // set target attr to modified value
        EvilNumber targValue = 0;
        int8 opID = cur.first;
        for (auto item : itemRefVec) {
            if (item.get() == nullptr)  // not sure why i need this, but have seen nulls in the vector (segfaults)
                continue;
            // get targAttr
            targValue = item->GetAttribute(cur.second.targAttr);
            // check for inf/nan and then reset?  this will fuck up all previous fx processing on this value.
            if (targValue.isNaN() or targValue.isInf()) {
                targValue = item->GetDefaultAttribute(cur.second.targAttr);
                _log(EFFECTS__ERROR, "FxProc::ApplyEffects(): targValue isInf or isNaN.  Data: %s(%u) - src(%s:%u) %.3f <%s> targ(%s:%u) set targ to %.3f.", \
                srcItemRef->itemName().c_str(), srcItemRef->itemID(), GetSourceName(cur.second.fxSrc).c_str(), cur.second.srcAttr, srcValue.get_float(), \
                    GetMathMethodName(opID).c_str(), GetTargLocName(cur.second.targLoc).c_str(), cur.second.targAttr, targValue.get_float());
            }

            switch (opID) {
                case dgmMathPreMul:
                case dgmMathPostMul:
                case dgmMathPreDiv:
                case dgmMathPostDiv:{
                    if (targValue == 0)
                        targValue = 1;
                } break;
            }

            // send data to calculator
            EvilNumber newValue = CalculateAttributeValue(targValue, srcValue, opID);
            // avoid creating 0-value attributes on items
            if (newValue == 0)
                continue;
            // set new calculated value for target attribute
            _log(EFFECTS__MESSAGE, "FxProc::ApplyEffects(%i): %s(%u) - src(%s:%u) %.3f <%s> targ(%s:%u) set targ from %.3f to %.3f.", cur.first, srcItemRef->itemName().c_str(), \
            srcItemRef->itemID(), GetSourceName(cur.second.fxSrc).c_str(), cur.second.srcAttr, srcValue.get_float(), GetMathMethodName(opID).c_str(), \
                GetTargLocName(cur.second.targLoc).c_str(), cur.second.targAttr, targValue.get_float(), newValue.get_float());

            // update is used to send attrib changes to client when changing module states while in space, but NOT for pilot login. (client acts funky)
            item->SetAttribute(cur.second.targAttr, newValue, update);
        }
    }
    /*  not used
    if (action)
        sFxAct.DoAction(action, pShip->GetPilot()->GetShipSE());   // this MUST be called AFTER all active effects are applied, as it uses those modified values
    */
}

EvilNumber FxProc::CalculateAttributeValue(EvilNumber val1/*targ*/, EvilNumber val2/*src*/, int8 method)
{
    if (val2 == 0)
        return val1;
    using namespace Effects;
    switch (method) {
        case dgmMathSkillCheck:
        case dgmMathPreAssignment:
        case dgmMathPostAssignment:
            return val2;
        case dgmMathPreMul:
        case dgmMathPostMul:
            return val1 * val2;
        case dgmMathPreDiv:
        case dgmMathPostDiv:
            return val1 / val2;
        case dgmMathModAdd:
            return val1 + val2;
        case dgmMathModSub:
            return val1 - val2;
        case dgmMathPostPercent:
            return val1 * (1 + (val2 / 100));
        case dgmMathRevPostPercent:
            return val1 / (1 + (val2 / 100));
        case dgmMathInvalid:
            _log(EFFECTS__WARNING, "FxProc::CalculateNewAttributeValue() - Invalid Association used");
            return val1;
    }
    _log(EFFECTS__ERROR, "FxProc::CalculateNewAttributeValue() - Unknown Association used: %i", (int8)method);
    return 0;
}

int8 FxProc::GetAssociationEnum(const std::string& association)
{   // opID 21
    using namespace Effects;
    if (association == "PreAssignment")
        return dgmMathPreAssignment;
    else if (association == "PreDiv")
        return dgmMathPreDiv;
    else if (association == "PreMul")
        return dgmMathPreMul;
    else if (association == "ModAdd")
        return dgmMathModAdd;
    else if (association == "ModSub")
        return dgmMathModSub;
    else if (association == "PostPercent")
        return dgmMathPostPercent;
    else if (association == "PostMul")
        return dgmMathPostMul;
    else if (association == "PostDiv")
        return dgmMathPostDiv;
    else if (association == "PostAssignment")
        return dgmMathPostAssignment;
    else if (association == "SkillCheck")
        return dgmMathSkillCheck;
    else if (association == "AddRate")
        return dgmMathAddRate;
    else if (association == "SubRate")
        return dgmMathSubRate;
    else
        return dgmMathInvalid;  //throw std::bad_typeid();
}

int8 FxProc::GetEnvironmentEnum(const std::string& env)
{   // opID 24
    using namespace Effects;
    if (env == "Self")
        return dgmTargLocSelf;
    else if (env == "Char")
        return dgmTargLocChar;
    else if (env == "Ship")
        return dgmTargLocShip;
    else if (env == "Target")
        return dgmTargLocTarget;
    else if (env == "Area")
        return dgmTargLocArea;
    else if (env == "Other")
        return dgmTargLocOther;
    else if (env == "Charge")
        return dgmTargLocCharge;
    else
        return dgmTargLocInvalid;  //throw std::bad_typeid();
}

std::string FxProc::GetMathMethodName(int8 id)
{
    using namespace Effects;
    switch (id) {
        case dgmMathPreAssignment:  return "PreAssignment";
        case dgmMathPreDiv:         return "PreDiv";
        case dgmMathPreMul:         return "PreMul";
        case dgmMathModAdd:         return "ModAdd";
        case dgmMathModSub:         return "ModSub";
        case dgmMathPostPercent:    return "PostPercent";
        case dgmMathRevPostPercent: return "RevPostPercent";
        case dgmMathPostMul:        return "PostMul";
        case dgmMathPostDiv:        return "PostDiv";
        case dgmMathPostAssignment: return "PostAssignment";
        case dgmMathSkillCheck:     return "SkillCheck";
        case dgmMathAddRate:        return "AddRate";
        case dgmMathSubRate:        return "SubRate";
        case dgmMathInvalid:
        default:                    return "Invalid";
    }
}

std::string FxProc::GetSourceName(int8 id)
{
    using namespace Effects;
    switch (id) {
        case dgmSrcSelf:            return "Self";
        case dgmSrcSkill:           return "Skill";
        case dgmSrcShip:            return "Ship";
        case dgmSrcOwner:           return "Owner";
        case dgmSrcGang:            return "Gang";
        case dgmSrcGroup:           return "Group";
        case dgmSrcTarget:          return "Target";
        case dgmSrcInvalid:
        default:                    return "Invalid";
    }
}

std::string FxProc::GetTargLocName(int8 id)
{
    using namespace Effects;
    switch (id) {
        case dgmTargLocSelf:        return "Self";
        case dgmTargLocChar:        return "Char";
        case dgmTargLocShip:        return "Ship";
        case dgmTargLocTarget:      return "Target";
        case dgmTargLocArea:        return "Area";
        case dgmTargLocOther:       return "Other";
        case dgmTargLocCharge:      return "Charge";
        case dgmTargLocInvalid:
        default:                    return "Invalid";
    }
}

std::string FxProc::GetStateName(int8 id)
{
    using namespace Effects;
    switch (id) {
        case dgmStatePassive:       return "Passive";
        case dgmStateActive:        return "Active";
        case dgmStateTarget:        return "Target";
        case dgmStateArea:          return "Area";
        case dgmStateOnline:        return "Online";
        case dgmStateOverloaded:    return "Overload";
        case dgmStateDungeon:       return "Dungeon";
        case dgmStateSystem:        return "System";
        case dgmStateInvalid:
        default:                    return "Invalid";
    }

}
