

-- this file is to add the "this module needs a target" effectID '10' (combat) to moduleID's that need it.

create temporary table if not exists tModules (typeID int);
TRUNCATE tModules;
insert into tModules (SELECT typeID FROM invTypes WHERE groupID IN
(47,48,52,53,54,55,56,65,67,68,71,74,84,88,89,201,202,208,289,290,291,325,379,464,483,506,507,508,509,510,511,512,524,771,1122));

ALTER TABLE `dgmTypeEffects` DROP PRIMARY KEY;

INSERT INTO `dgmTypeEffects` (`typeID`,`effectID`,`isDefault`)
SELECT typeID, 10 AS effectID, 0 AS isDefault FROM tModules;
