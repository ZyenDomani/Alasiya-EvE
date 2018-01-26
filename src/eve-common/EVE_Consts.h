/* Alasiya EvE Constants
 *   this file is a common location for all static-type defined data
 */


/*
 *  misc static consts
 */

static const char alphaList[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char numList[] = "0123456789";
static const char hexList[] = "0123456789ABCDEF";

/*  these are based on client settings of damage notification.
 * msg packets are
 *   " "  - to others
 *   "R"  - received
 *   "RD  - received with details
 */

/*{'messageKey': 'AttackHit1', 'dataID': 17885829, 'suppressable': False, 'bodyID': 260383, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 114}
 * {'messageKey': 'AttackHit1Banked', 'dataID': 17878336, 'suppressable': False, 'bodyID': 257589, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2641}
 * {'messageKey': 'AttackHit1Banked_Simple', 'dataID': 17878333, 'suppressable': False, 'bodyID': 257588, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2652}
 * {'messageKey': 'AttackHit1R', 'dataID': 17885835, 'suppressable': False, 'bodyID': 260385, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 115}
 * {'messageKey': 'AttackHit1RD', 'dataID': 17885838, 'suppressable': False, 'bodyID': 260386, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 116}
 * {'messageKey': 'AttackHit1RD_Simple', 'dataID': 17879270, 'suppressable': False, 'bodyID': 257946, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2352}
 * {'messageKey': 'AttackHit1R_Simple', 'dataID': 17879273, 'suppressable': False, 'bodyID': 257947, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2367}
 * {'messageKey': 'AttackHit1_Simple', 'dataID': 17879276, 'suppressable': False, 'bodyID': 257948, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2361}
 * {'messageKey': 'AttackHit2', 'dataID': 17885841, 'suppressable': False, 'bodyID': 260387, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 117}
 * {'messageKey': 'AttackHit2Banked', 'dataID': 17878339, 'suppressable': False, 'bodyID': 257590, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2642}
 * {'messageKey': 'AttackHit2Banked_Simple', 'dataID': 17878330, 'suppressable': False, 'bodyID': 257587, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2653}
 * {'messageKey': 'AttackHit2R', 'dataID': 17885844, 'suppressable': False, 'bodyID': 260388, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 118}
 * {'messageKey': 'AttackHit2RD', 'dataID': 17885850, 'suppressable': False, 'bodyID': 260390, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 119}
 * {'messageKey': 'AttackHit2RD_Simple', 'dataID': 17879279, 'suppressable': False, 'bodyID': 257949, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2353}
 * {'messageKey': 'AttackHit2R_Simple', 'dataID': 17879282, 'suppressable': False, 'bodyID': 257950, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2368}
 * {'messageKey': 'AttackHit2_Simple', 'dataID': 17879285, 'suppressable': False, 'bodyID': 257951, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2362}
 * {'messageKey': 'AttackHit3', 'dataID': 17885853, 'suppressable': False, 'bodyID': 260391, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 120}
 * {'messageKey': 'AttackHit3Banked', 'dataID': 17878342, 'suppressable': False, 'bodyID': 257591, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2644}
 * {'messageKey': 'AttackHit3Banked_Simple', 'dataID': 17878327, 'suppressable': False, 'bodyID': 257586, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2654}
 * {'messageKey': 'AttackHit3R', 'dataID': 17885856, 'suppressable': False, 'bodyID': 260392, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 121}
 * {'messageKey': 'AttackHit3RD', 'dataID': 17885859, 'suppressable': False, 'bodyID': 260393, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 122}
 * {'messageKey': 'AttackHit3RD_Simple', 'dataID': 17879288, 'suppressable': False, 'bodyID': 257952, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2354}
 * {'messageKey': 'AttackHit3R_Simple', 'dataID': 17879291, 'suppressable': False, 'bodyID': 257953, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2369}
 * {'messageKey': 'AttackHit3_Simple', 'dataID': 17879294, 'suppressable': False, 'bodyID': 257954, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2363}
 * {'messageKey': 'AttackHit4', 'dataID': 17885862, 'suppressable': False, 'bodyID': 260394, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 123}
 * {'messageKey': 'AttackHit4Banked', 'dataID': 17878345, 'suppressable': False, 'bodyID': 257592, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2645}
 * {'messageKey': 'AttackHit4Banked_Simple', 'dataID': 17878318, 'suppressable': False, 'bodyID': 257583, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2655}
 * {'messageKey': 'AttackHit4R', 'dataID': 17885865, 'suppressable': False, 'bodyID': 260395, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 124}
 * {'messageKey': 'AttackHit4RD', 'dataID': 17885868, 'suppressable': False, 'bodyID': 260396, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 125}
 * {'messageKey': 'AttackHit4RD_Simple', 'dataID': 17879297, 'suppressable': False, 'bodyID': 257955, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2355}
 * {'messageKey': 'AttackHit4R_Simple', 'dataID': 17879300, 'suppressable': False, 'bodyID': 257956, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2370}
 * {'messageKey': 'AttackHit4_Simple', 'dataID': 17879303, 'suppressable': False, 'bodyID': 257957, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2364}
 * {'messageKey': 'AttackHit5', 'dataID': 17885871, 'suppressable': False, 'bodyID': 260397, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 126}
 * {'messageKey': 'AttackHit5Banked', 'dataID': 17878348, 'suppressable': False, 'bodyID': 257593, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2646}
 * {'messageKey': 'AttackHit5Banked_Simple', 'dataID': 17878324, 'suppressable': False, 'bodyID': 257585, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2657}
 * {'messageKey': 'AttackHit5R', 'dataID': 17885874, 'suppressable': False, 'bodyID': 260398, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 127}
 * {'messageKey': 'AttackHit5RD', 'dataID': 17885877, 'suppressable': False, 'bodyID': 260399, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 128}
 * {'messageKey': 'AttackHit5RD_Simple', 'dataID': 17879306, 'suppressable': False, 'bodyID': 257958, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2356}
 * {'messageKey': 'AttackHit5R_Simple', 'dataID': 17879309, 'suppressable': False, 'bodyID': 257959, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2371}
 * {'messageKey': 'AttackHit5_Simple', 'dataID': 17879312, 'suppressable': False, 'bodyID': 257960, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2365}
 * {'messageKey': 'AttackHit6', 'dataID': 17885880, 'suppressable': False, 'bodyID': 260400, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 129}
 * {'messageKey': 'AttackHit6Banked', 'dataID': 17878351, 'suppressable': False, 'bodyID': 257594, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2647}
 * {'messageKey': 'AttackHit6Banked_Simple', 'dataID': 17878321, 'suppressable': False, 'bodyID': 257584, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2658}
 * {'messageKey': 'AttackHit6R', 'dataID': 17885883, 'suppressable': False, 'bodyID': 260401, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 130}
 * {'messageKey': 'AttackHit6RD', 'dataID': 17885886, 'suppressable': False, 'bodyID': 260402, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 131}
 * {'messageKey': 'AttackHit6RD_Simple', 'dataID': 17879315, 'suppressable': False, 'bodyID': 257961, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2357}
 * {'messageKey': 'AttackHit6R_Simple', 'dataID': 17879318, 'suppressable': False, 'bodyID': 257962, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2372}
 * {'messageKey': 'AttackHit6_Simple', 'dataID': 17879321, 'suppressable': False, 'bodyID': 257963, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2366}
 * {'messageKey': 'AttackMiss1', 'dataID': 17885907, 'suppressable': False, 'bodyID': 260410, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 135}
 * {'messageKey': 'AttackMiss1Banked', 'dataID': 17878555, 'suppressable': False, 'bodyID': 257671, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2648}
 * {'messageKey': 'AttackMiss1Banked_Simple', 'dataID': 17878563, 'suppressable': False, 'bodyID': 257674, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2660}
 * {'messageKey': 'AttackMiss1R', 'dataID': 17885910, 'suppressable': False, 'bodyID': 260411, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 136}
 * {'messageKey': 'AttackMiss1RD', 'dataID': 17885913, 'suppressable': False, 'bodyID': 260412, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 137}
 * {'messageKey': 'AttackMiss1RD_Simple', 'dataID': 17879536, 'suppressable': False, 'bodyID': 258044, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2358}
 * {'messageKey': 'AttackMiss1R_Simple', 'dataID': 17879539, 'suppressable': False, 'bodyID': 258045, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2373}
 * {'messageKey': 'AttackMiss1_Simple', 'dataID': 17879474, 'suppressable': False, 'bodyID': 258022, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2376}
 * {'messageKey': 'AttackMiss2', 'dataID': 17885916, 'suppressable': False, 'bodyID': 260413, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 138}
 * {'messageKey': 'AttackMiss2Banked', 'dataID': 17878571, 'suppressable': False, 'bodyID': 257677, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2649}
 * {'messageKey': 'AttackMiss2Banked_Simple', 'dataID': 17878383, 'suppressable': False, 'bodyID': 257606, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2661}
 * {'messageKey': 'AttackMiss2R', 'dataID': 17885919, 'suppressable': False, 'bodyID': 260414, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 139}
 * {'messageKey': 'AttackMiss2RD', 'dataID': 17885927, 'suppressable': False, 'bodyID': 260417, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 140}
 * {'messageKey': 'AttackMiss2RD_Simple', 'dataID': 17879542, 'suppressable': False, 'bodyID': 258046, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2359}
 * {'messageKey': 'AttackMiss2R_Simple', 'dataID': 17879554, 'suppressable': False, 'bodyID': 258050, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2374}
 * {'messageKey': 'AttackMiss2_Simple', 'dataID': 17879551, 'suppressable': False, 'bodyID': 258049, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2377}
 * {'messageKey': 'AttackMiss3', 'dataID': 17885930, 'suppressable': False, 'bodyID': 260418, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 141}
 * {'messageKey': 'AttackMiss3Banked', 'dataID': 17878574, 'suppressable': False, 'bodyID': 257678, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2650}
 * {'messageKey': 'AttackMiss3Banked_Simple', 'dataID': 17878482, 'suppressable': False, 'bodyID': 257642, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2662}
 * {'messageKey': 'AttackMiss3R', 'dataID': 17885933, 'suppressable': False, 'bodyID': 260419, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 142}
 * {'messageKey': 'AttackMiss3RD', 'dataID': 17885936, 'suppressable': False, 'bodyID': 260420, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 143}
 * {'messageKey': 'AttackMiss3RD_Simple', 'dataID': 17879557, 'suppressable': False, 'bodyID': 258051, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2360}
 * {'messageKey': 'AttackMiss3R_Simple', 'dataID': 17879548, 'suppressable': False, 'bodyID': 258048, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2375}
 * {'messageKey': 'AttackMiss3_Simple', 'dataID': 17879545, 'suppressable': False, 'bodyID': 258047, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2378}
 *
 * {'FullPath': u'UI/Messages', 'messageID': 258044, 'label': u'AttackMiss1RD_SimpleBody'}(u'{owner} ({[item]weapon.name}) misses you', None, {u'{owner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'owner'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258045, 'label': u'AttackMiss1R_SimpleBody'}(u'{source} misses you', None, {u'{source}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'source'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258046, 'label': u'AttackMiss2RD_SimpleBody'}(u'{owner} ({[item]weapon.name}) misses you', None, {u'{owner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'owner'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258047, 'label': u'AttackMiss3_SimpleBody'}(u'{[item]weapon.name} misses {target}', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258048, 'label': u'AttackMiss3R_SimpleBody'}(u'{source} misses you', None, {u'{source}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'source'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258049, 'label': u'AttackMiss2_SimpleBody'}(u'{[item]weapon.name} misses {target}', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258050, 'label': u'AttackMiss2R_SimpleBody'}(u'{source} misses you', None, {u'{source}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'source'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258051, 'label': u'AttackMiss3RD_SimpleBody'}(u'{owner} ({[item]weapon.name}) misses you', None, {u'{owner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'owner'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257584, 'label': u'AttackHit6Banked_SimpleBody'}(u'<color=0xff99bb00>Group of {[item]weapon.name} strikes {target} for <b>{[numeric]damage, decimalPlaces=0}</b> damage', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[numeric]damage, decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'damage'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257585, 'label': u'AttackHit5Banked_SimpleBody'}(u'<color=0xff99bb00>Group of {[item]weapon.name} strikes {target} for <b>{[numeric]damage, decimalPlaces=0}</b> damage', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[numeric]damage, decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'damage'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257586, 'label': u'AttackHit3Banked_SimpleBody'}(u'<color=0xff99bb00>Group of {[item]weapon.name} strikes {target} for <b>{[numeric]damage, decimalPlaces=0}</b> damage', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[numeric]damage, decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'damage'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257587, 'label': u'AttackHit2Banked_SimpleBody'}(u'<color=0xff66bb00>Group of {[item]weapon.name} strikes {target} for <b>{[numeric]damage, decimalPlaces=0}</b> damage', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[numeric]damage, decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'damage'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257588, 'label': u'AttackHit1Banked_SimpleBody'}(u'<color=0xff99bb00>Group of {[item]weapon.name} strikes {target} for <b>{[numeric]damage, decimalPlaces=0}</b> damage', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[numeric]damage, decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'damage'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257589, 'label': u'AttackHit1BankedBody'}(u'<color=0xffbbbb00>Your group of {[item]weapon.name} barely scratches {target}, causing {[numeric]damage, decimalPlaces=1} damage.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257590, 'label': u'AttackHit2BankedBody'}(u'<color=0xffbbbb00>Your group of {[item]weapon.name} lightly hits {target}, doing {[numeric]damage, decimalPlaces=1} damage.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257591, 'label': u'AttackHit3BankedBody'}(u'<color=0xffbbbb00>Your group of {[item]weapon.name} hits {target}, doing {[numeric]damage, decimalPlaces=1} damage.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257592, 'label': u'AttackHit4BankedBody'}(u'<color=0xffbbbb00>Your group of {[item]weapon.name} is well aimed at {target}, inflicting {[numeric]damage, decimalPlaces=1} damage.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257593, 'label': u'AttackHit5BankedBody'}(u'<color=0xffbbbb00>Your group of {[item]weapon.name} places an excellent hit on {target}, inflicting {[numeric]damage, decimalPlaces=1} damage.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257594, 'label': u'AttackHit6BankedBody'}(u'<color=0xffbbbb00>Your group of {[item]weapon.name} perfectly strikes {target}, wrecking for {[numeric]damage, decimalPlaces=1} damage.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}, u'{[numeric]damage, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'damage'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257606, 'label': u'AttackMiss2Banked_SimpleBody'}(u'Group of {[item]weapon.name} misses {target}', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257642, 'label': u'AttackMiss3Banked_SimpleBody'}(u'Group of {[item]weapon.name} miss {target}.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257671, 'label': u'AttackMiss1BankedBody'}(u'Your group of {[item]weapon.name} misses {target} completely.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257674, 'label': u'AttackMiss1Banked_SimpleBody'}(u'Group of {[item]weapon.name} miss {target}', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257677, 'label': u'AttackMiss2BankedBody'}(u'Your group of {[item]weapon.name} barely misses {target}.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257678, 'label': u'AttackMiss3BankedBody'}(u'Your group of {[item]weapon.name} glances off {target}, causing no real damage.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257947, 'label': u'AttackHit1R_SimpleBody'}(u'<color=0xffbb6600>{source} hits you for <b>{[numeric]damage,decimalPlaces=0}</b> damage\r\n\r\n', None, {u'{source}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'source'}, u'{[numeric]damage,decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'damage'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257948, 'label': u'AttackHit1_SimpleBody'}(u'<color=0xff99bb00>{[item]weapon.name} hits {target} for <b>{[numeric]damage,decimalPlaces=0}</b> damage', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[numeric]damage,decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'damage'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257949, 'label': u'AttackHit2RD_SimpleBody'}(u'<color=0xffbb6600>{owner} ({[item]weapon.name}) hits you for <b>{[numeric]damage,decimalPlaces=0}</b> damage', None, {u'{owner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'owner'}, u'{[numeric]damage,decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'damage'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257950, 'label': u'AttackHit2R_SimpleBody'}(u'<color=0xffbb6600>{source} hits you for <b>{[numeric]damage,decimalPlaces=0}</b> damage', None, {u'{source}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'source'}, u'{[numeric]damage,decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'damage'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257951, 'label': u'AttackHit2_SimpleBody'}(u'<color=0xff66bb00>{[item]weapon.name} hits {target} for <b>{[numeric]damage,decimalPlaces=0}</b> damage', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[numeric]damage,decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'damage'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257952, 'label': u'AttackHit3RD_SimpleBody'}(u'<color=0xffbb6600>{owner} ({[item]weapon.name}) hits you for <b>{[numeric]damage,decimalPlaces=0}</b> damage', None, {u'{owner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'owner'}, u'{[numeric]damage,decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'damage'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257953, 'label': u'AttackHit3R_SimpleBody'}(u'<color=0xffbb6600>{source} hits you for <b>{[numeric]damage,decimalPlaces=0}</b> damage', None, {u'{source}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'source'}, u'{[numeric]damage,decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'damage'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257954, 'label': u'AttackHit3_SimpleBody'}(u'<color=0xff99bb00>{[item]weapon.name} hits {target} for <b>{[numeric]damage,decimalPlaces=0}</b> damage', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[numeric]damage,decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'damage'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257955, 'label': u'AttackHit4RD_SimpleBody'}(u'<color=0xffbb6600>{owner} ({[item]weapon.name}) hits you for <b>{[numeric]damage,decimalPlaces=0}</b> damage', None, {u'{owner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'owner'}, u'{[numeric]damage,decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'damage'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257956, 'label': u'AttackHit4R_SimpleBody'}(u'<color=0xffbb6600>{source} hits you for <b>{[numeric]damage,decimalPlaces=0}</b> damage', None, {u'{source}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'source'}, u'{[numeric]damage,decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'damage'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257958, 'label': u'AttackHit5RD_SimpleBody'}(u'<color=0xffbb6600>{owner} ({[item]weapon.name}) hits you for <b>{[numeric]damage,decimalPlaces=0}</b> damage', None, {u'{owner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'owner'}, u'{[numeric]damage,decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'damage'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257959, 'label': u'AttackHit5R_SimpleBody'}(u'<color=0xffbb6600>{source} hits you for <b>{[numeric]damage,decimalPlaces=0}</b> damage', None, {u'{source}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'source'}, u'{[numeric]damage,decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'damage'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257960, 'label': u'AttackHit5_SimpleBody'}(u'<color=0xff99bb00>{[item]weapon.name} hits {target} for <b>{[numeric]damage,decimalPlaces=0}</b> damage', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[numeric]damage,decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'damage'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257961, 'label': u'AttackHit6RD_SimpleBody'}(u'<color=0xffbb6600>{owner} ({[item]weapon.name}) hits you for <b>{[numeric]damage,decimalPlaces=0}</b> damage (Wrecking!)', None, {u'{owner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'owner'}, u'{[numeric]damage,decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'damage'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * {'FullPath': u'UI/Messages', 'messageID': 257962, 'label': u'AttackHit6R_SimpleBody'}(u'<color=0xffbb6600>{source} hits you for <b>{[numeric]damage,decimalPlaces=0}</b> damage (Wrecking!)', None, {u'{source}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'source'}, u'{[numeric]damage,decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'damage'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258022, 'label': u'AttackMiss1_SimpleBody'}(u'{[item]weapon.name} misses {target}', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]weapon.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'weapon'}})
 * 
 */

static const char* DamageMessageIDs_Self[7] = {
    "AttackMiss1R",   //miss
    //AttackMiss1Banked  ?not sure here....'banked' means 'group weapons'  also in ship.modules.GetTurretSets in MakeSlimItem()
    "AttackHit1R",    //barely scratches
    "AttackHit2R",    //lightly hits
    "AttackHit3R",    //hits
    "AttackHit4R",    //aims well at you
    "AttackHit5R",    //places an excellent hit
    "AttackHit6R"     //strikes you perfectly, wrecking
};

static const char* DamageMessageIDs_SelfNamed[7] = {
    "AttackMiss1RD",   //miss
    "AttackHit1RD",    //barely scratches
    "AttackHit2RD",    //lightly hits
    "AttackHit3RD",    //hits
    "AttackHit4RD",    //aims well at you
    "AttackHit5RD",    //places an excellent hit
    "AttackHit6RD"     //strikes you perfectly, wrecking
};

static const char* DamageMessageIDs_Other[7] = {
    "AttackMiss1",   //miss
    "AttackHit1",    //barely scratches
    "AttackHit2",    //lightly hits
    "AttackHit3",    //hits
    "AttackHit4",    //aims well
    "AttackHit5",    //places an excellent hit
    "AttackHit6"     //strikes perfectly, wrecking
};

static const uint16 SHIP_PROCESS_TICK_MS = 5000;    // 5s

static const GPoint NULL_ORIGIN(0,0,0);  // common place for a zero-value gpoint
static const GVector NULL_ORIGIN_V(0,0,0);

const int32 ITEM_DB_SAVE_TIMER_EXPIRY(10);

static const float TIC_DURATION_IN_SECONDS(1000);

static const uint32 minWarpDistance(100000);    // 100km

static const float onlineModInSpace(0.75);     // onling modules while NOT docked or using fitting services will take 75% of capacitor capacity.

//   based on client code...
static const int64 ONE_LIGHTYEAR(9460000000000000);  // in meters
static const int64 ONE_AU_IN_METERS(149597870700);     // 1 astronomical unit in meters, per EVElopedia: http://wiki.eveonline.com/en/wiki/Astronomical_Unit
static const int64 STATION_HANGAR_MAX_CAPACITY(9000000000000000);  //per client
static const double MAX_MARKET_PRICE(9223372036854);  //max int64/1000000  (9223372036854775807/1000000)
static const int32 INCAPACITATION_DISTANCE(250000);    // drone to ship max distance.  after this, drone goes Offline and is considered 'lost'

// Cosmic Managers constants here  *not used yet*
static const uint32 ASTEROID_GROWTH_INTERVAL_MS(3600000);  /* this is grow check in ms (1d) */

// gravitational constant
static const double Gc(6.6725985e-11);     //per client (changed from original 6.673e-11)

/*  misc data
 * radius constants
 * moon    =  1737km
 * mars    =  3390km
 * earth   =  6371km
 * jupiter = 69911km
 *
 * gravity constants
 * moon    =  1.622 m/s^2
 * mars    =  3.711 m/s^2
 * earth   =  9.807 m/s^2
 * jupiter = 24.790 m/s^2
 */

/* ship agility by class
 * class          agility
 * Capsule          .06
 * Shuttle          1.6
 * Rookie           5
 * Frigates         3 - 6 (adv. 3 - 4)
 * Destroyers       4 - 5
 * Cruisers         4 - 8
 * T3 Cruiser       2.4 - 2.8
 * HAC              5 - 7
 * Battlecruisers   6 - 9
 * Battleships      8 - 14
 * Industrials      8 - 12
 * Marauder         ~12
 * Orca             40
 * Freighters       ~60
 * Supercarrier     ~60
 * Command          ~9
 * Transport        5 or 19
 * Barges           10 - 18
 * Dreadnought      ~55
 * Zephyr           5
 */