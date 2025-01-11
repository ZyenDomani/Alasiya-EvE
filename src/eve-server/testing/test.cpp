
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
#include "StaticDataMgr.h"
#include "system/SystemEntity.h"
#include "testing/test.h"

void testing::posTest(Client* pClient) {
    SystemEntity* mySE(pClient->GetShipSE());

    sLog.Warning("\ttesting","Test competed");
}

void testing::CharAttribTest() {
    /* this is for testing and setting default character attributes
     * it will need names, Base, Ancestry.Bonus, BL.Bonus
     */

    DBQueryResult res, res2, res3, res4;
    DBResultRow row, row2, row3, row4;

    // ancestryID is unique.  get it first.  (42)
    sDatabase.RunQuery(res,
    "SELECT ancestryID, ancestryName, bloodlineID, perception, willpower, charisma, memory, intelligence "
    " FROM chrAncestries");

    while (res.GetRow(row)) {
        attrTestData data = attrTestData();
        data.ancestryID = row.GetUInt8(0);   // unique
        data.ancestryName = row.GetText(1);
        data.bloodlineID = row.GetUInt8(2);
        data.aPerception = row.GetUInt8(3);
        data.aWillpower = row.GetUInt8(4);
        data.aCharisma = row.GetUInt8(5);
        data.aMemory = row.GetUInt8(6);
        data.aIntelligence = row.GetUInt8(7);

        // bloodline is based on ancestry  (14)
        sDatabase.RunQuery(res2,
        "SELECT bloodlineName, raceID, perception, willpower, charisma, memory, intelligence"
        " FROM chrBloodlines"
        " WHERE bloodlineID = %u", row.GetUInt8(2));
        res2.GetRow(row2);

        data.bloodlineName = row2.GetText(0);
        data.raceID = row2.GetUInt8(1);
        data.bPerception = row2.GetUInt8(2);
        data.bWillpower = row2.GetUInt8(3);
        data.bCharisma = row2.GetUInt8(4);
        data.bMemory = row2.GetUInt8(5);
        data.bIntelligence = row2.GetUInt8(6);

        sDatabase.RunQuery(res3, "SELECT typeID FROM bloodlineTypes WHERE bloodlineID = %u", row.GetUInt8(2));
        res3.GetRow(row3);

        const ItemType *iType = sItemFactory.GetType(row3.GetUInt(0));

        data.typeName = iType->name();
        data.cPerception = iType->GetAttribute(AttrPerception).get_uint32();
        data.cWillpower = iType->GetAttribute(AttrWillpower).get_uint32();
        data.cCharisma = iType->GetAttribute(AttrCharisma).get_uint32();
        data.cMemory = iType->GetAttribute(AttrMemory).get_uint32();
        data.cIntelligence = iType->GetAttribute(AttrIntelligence).get_uint32();

        data.tPerception = data.aPerception + data.bPerception + data.cPerception;
        data.tWillpower = data.aWillpower + data.bWillpower + data.cWillpower;
        data.tCharisma = data.aCharisma + data.bCharisma + data.cCharisma;
        data.tMemory = data.aMemory + data.bMemory + data.cMemory;
        data.tIntelligence = data.aIntelligence + data.bIntelligence + data.cIntelligence;

        m_attribTest[row.GetUInt8(0)] = std::move(data);
    }

    sLog.Green("Character Base Attributes", "%u Ancestry ID's registered.", m_attribTest.size());
    for (auto& cur : m_attribTest) {
        sLog.Blue("Character ", "%s", sDataMgr.GetRaceName(cur.second.raceID));
        sLog.Yellow("        ", "Per: %u, Wil: %u, Cha: %u, Mem: %u, Int: %u", \
                cur.second.tPerception, cur.second.tWillpower, cur.second.tCharisma, \
                cur.second.tMemory, cur.second.tIntelligence);


        sLog.Blue("          ", "Ancestry: %s", cur.second.ancestryName.c_str());
        sLog.Yellow("        ", "Per: %u, Wil: %u, Cha: %u, Mem: %u, Int: %u", \
                cur.second.aPerception, cur.second.aWillpower, cur.second.aCharisma, \
                cur.second.aMemory, cur.second.aIntelligence);


        sLog.Blue("          ", "Bloodline: %s", cur.second.bloodlineName.c_str());
        sLog.Yellow("        ", "Per: %u, Wil: %u, Cha: %u, Mem: %u, Int: %u", \
                cur.second.bPerception, cur.second.bWillpower, cur.second.bCharisma, \
                cur.second.bMemory, cur.second.bIntelligence);
    }
}
