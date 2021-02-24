
 /**
  * @name test.cpp
  *   Code for testing miscellaneous items in Alasiya EvE
  *
  * @Author:        Allan
  * @date:          19 August 2020
  *
  */

#include "eve-server.h"

#include "Client.h"
#include "system/SystemEntity.h"
#include "testing/test.h"

void testing::posTest(Client* pClient) {
    SystemEntity* mySE(pClient->GetShipSE());

    sLog.Warning("\ttesting","Test competed");
}

void testing::SetBasePrice()
{
    /* method to estimate item base price, based on materials to manufacture that item
     * this will fudge cost a bit for material delivery, factory setup, line run cost, and storage of raw and final items
     *
     * eventually, this will query materials prices,
     * then set estimated prices for items based on ME0 BPO
     *
     * mineral prices are queried from db, based on median of buy/sell orders, leveraged with base price in db
     * then, the base price is updated in db and saved for later use
     *
     * ships and modules are loaded and queried from static data for manufacturing materials
     * those materials are queried (as required) for minerals needed
     * once a total mineral value has been calculated, calculate estimated cost based on
     * current mineral values.
     * that cost will be modified for above additions (cost added)
     *
     *
     * NOTES FOR IMPLEMETING THIS SHIT
     *
        //SELECT typeID, materialTypeID, quantity FROM invTypeMaterials
        EvERam::RamMaterials ramMatls = EvERam::RamMaterials();
        ramMatls.quantity       = row.GetInt(2);
        ramMatls.materialTypeID = row.GetInt(1);

        //SELECT typeID, activityID, requiredTypeID, quantity, damagePerJob, extra FROM ramTypeRequirements
        EvERam::RamRequirements ramReq = EvERam::RamRequirements();
        ramReq.activityID       = row.GetInt(1);
        ramReq.requiredTypeID   = row.GetInt(2);
        ramReq.quantity         = row.GetInt(3);
        ramReq.damagePerJob     = row.GetFloat(4);
        ramReq.extra            = row.GetBool(5);

        //SELECT blueprintTypeID, parentBlueprintTypeID, productTypeID, productionTime, techLevel, researchProductivityTime, researchMaterialTime, researchCopyTime,
        //  researchTechTime, productivityModifier, materialModifier, wasteFactor, maxProductionLimit, chanceOfRE, catID FROM invBlueprintTypes
        EvERam::bpTypeData bpTypeData = EvERam::bpTypeData();
        bpTypeData.parentBlueprintTypeID    = row.GetInt(1);
        bpTypeData.productTypeID            = row.GetInt(2);
        bpTypeData.productionTime           = row.GetInt(3);
        bpTypeData.techLevel                = row.GetInt(4);
        bpTypeData.researchProductivityTime = row.GetInt(5);
        bpTypeData.researchMaterialTime     = row.GetInt(6);
        bpTypeData.researchCopyTime         = row.GetInt(7);
        bpTypeData.researchTechTime         = row.GetInt(8);
        bpTypeData.productivityModifier     = row.GetInt(9);
        bpTypeData.materialModifier         = row.GetInt(10);
        bpTypeData.wasteFactor              = row.GetInt(11);
        bpTypeData.maxProductionLimit       = row.GetInt(12);
        bpTypeData.chanceOfRE               = row.GetFloat(13);
        bpTypeData.catID                    = row.GetInt(14);

     */

    // get current mineral prices
    /* catID
     * 4        material
     * 18       4       Mineral
     * groupID 18 is minerals
     *
     * how to get from static data?
     *  pull straight from db?
     * ....neither.  hard-code minerals.
     *    it's not like they change
     */
    std::vector< matlData > data;
    sDataMgr.GetMineralData(data);

    //  get mineral prices here and put into data vector

    
    // get shipIDs
    //  which ones?
    // start with frigates for testing


    // get minerals required for ship
    std::vector< EvERam::RequiredItem > matVec;
    if (0)
        sDataMgr.GetRamRequiredItems(typeID, EvERam::Activity::Manufacturing, matVec);

    /*
        matVec.typeID = it->second.materialTypeID;
        matVec.quantity = it->second.quantity;
    */


    // determine base price of ship based on mineral requirements


    // apply modifier to base price


    // update db for 'new' base price



}
