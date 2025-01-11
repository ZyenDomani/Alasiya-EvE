
 /**
  * @name test.h
  *   Code for testing miscellaneous items in Alasiya EvE
  *
  * @Author:        Allan
  * @date:          19 August 2020
  *
  */


#ifndef EVEMU_TESTING_TEST_H_
#define EVEMU_TESTING_TEST_H_

#include "character/Character.h"


class Client;

struct attrTestData {
    uint8 ancestryID;   // unique
    uint8 bloodlineID;
    uint8 aPerception;
    uint8 aWillpower;
    uint8 aCharisma;
    uint8 aMemory;
    uint8 aIntelligence;

    uint8 raceID;
    uint8 bPerception;
    uint8 bWillpower;
    uint8 bCharisma;
    uint8 bMemory;
    uint8 bIntelligence;

    uint8 cPerception;
    uint8 cWillpower;
    uint8 cCharisma;
    uint8 cMemory;
    uint8 cIntelligence;

    uint8 tPerception;
    uint8 tWillpower;
    uint8 tCharisma;
    uint8 tMemory;
    uint8 tIntelligence;

    std::string ancestryName;
    std::string bloodlineName;
    std::string typeName;
};

class testing {
public:

    static void posTest(Client* pClient);
    void CharAttribTest();


private:
    std::map<uint8, attrTestData> m_attribTest;         //ancestryID, data

};



#endif  // EVEMU_TESTING_TEST_H_