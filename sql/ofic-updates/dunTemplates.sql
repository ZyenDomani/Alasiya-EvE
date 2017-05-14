/*  dungeon template system data
 *  sql by deslona
 *  updates by allan
 */

--  dungeon template structure

DROP TABLE IF EXISTS dunTemplates;
CREATE TABLE dunTemplates (
    dunTemplateID INT NOT NULL,
    dunTemplateName VARCHAR(85) NOT NULL,
    dunEntryID INT NOT NULL DEFAULT 0,
    dunTypeID INT NOT NULL DEFAULT 0,
    dunSpawnType INT NOT NULL DEFAULT 0,
    dunRoomID INT NOT NULL DEFAULT 0,
    dunRooms INT NOT NULL DEFAULT 0,
    dunRoomTypeID INT NOT NULL DEFAULT 0,
    dunRoomCategoryID INT NOT NULL DEFAULT 0,
KEY dunTemplateID (dunTemplateID)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

DROP TABLE IF EXISTS dunEntryData;
CREATE TABLE dunEntryData (
   dunEntryID INT NOT NULL,
   dunEntryName VARCHAR(85) NOT NULL,
   xpos INT NOT NULL DEFAULT 0,
   ypos INT NOT NULL DEFAULT 0,
   zpos INT NOT NULL DEFAULT 0,
KEY dunEntryID (dunEntryID)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

DROP TABLE IF EXISTS dunRoomInfo;
CREATE TABLE dunRoomInfo (
   dunRoomID INT NOT NULL,
   dunRoomName VARCHAR(85) NOT NULL,
   dunRoomType INT NOT NULL DEFAULT 0,
   dunRoomCategory INT NOT NULL DEFAULT 0,
   dunRoomSpawnID INT NOT NULL DEFAULT 0,
   dunRoomSpawnType INT NOT NULL DEFAULT 0,
KEY dunRoomID (dunRoomID)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

DROP TABLE IF EXISTS dunRoomData;
CREATE TABLE dunRoomData (
   dunRoomID INT NOT NULL DEFAULT 0,
   dunGroupID INT NOT NULL DEFAULT 0,
   xpos INT NOT NULL DEFAULT 0,
   ypos INT NOT NULL DEFAULT 0,
   zpos INT NOT NULL DEFAULT 0,
KEY dunRoomID (dunRoomID)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

DROP TABLE IF EXISTS dunGroupInfo;
CREATE TABLE dunGroupInfo (
   dunGroupID INT NOT NULL,
   dunGroupName VARCHAR(85) NOT NULL,
KEY dunGroupID (dunGroupID)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

DROP TABLE IF EXISTS dunGroupData;
CREATE TABLE dunGroupData (
   dunGroupID INT NOT NULL,
   itemTypeID INT NOT NULL,
   xpos INT NOT NULL DEFAULT 0,
   ypos INT NOT NULL DEFAULT 0,
   zpos INT NOT NULL DEFAULT 0,
KEY dunGroupID (dunGroupID)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

DROP TABLE IF EXISTS dunRoomSpawnInfo;
CREATE TABLE dunRoomSpawnInfo (
   dunRoomSpawnID INT NOT NULL,
   dunRoomSpawnName VARCHAR(85) NOT NULL,
   dunRoomSpawnType INT NOT NULL,
   xpos INT NOT NULL DEFAULT 0,
   ypos INT NOT NULL DEFAULT 0,
   zpos INT NOT NULL DEFAULT 0,
KEY dunRoomSpawnID (dunRoomSpawnID)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

DROP TABLE IF EXISTS dunRoomSpawnData;
CREATE TABLE dunRoomSpawnData (
   dunRoomSpawnID INT NOT NULL,
   dunRoomSpawnName VARCHAR(85) NOT NULL,
KEY dunRoomSpawnID (dunRoomSpawnID)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

DROP TABLE IF EXISTS dunSpawnType;
CREATE TABLE dunSpawnType (
   dunSpawnTypeID INT NOT NULL,
   dunSpawnName VARCHAR(85) NOT NULL,
KEY dunSpawnTypeID (dunSpawnTypeID)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

DROP TABLE IF EXISTS dunActive;
CREATE TABLE IF NOT EXISTS dunActive (
   systemID INT NOT NULL,
   dungeonID INT NOT NULL,
   dunTemplateID INT NOT NULL,
   dunExpiryTime BIGINT(20) NOT NULL DEFAULT 0,
   state INT NOT NULL DEFAULT 0,
   xpos FLOAT NOT NULL DEFAULT 0,
   ypos FLOAT NOT NULL DEFAULT 0,
   zpos FLOAT NOT NULL DEFAULT 0,
  KEY systemID (systemID)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;


--  dungeon template data

-- All asteroid anomalies
INSERT INTO dunTemplates  VALUES
('1', 'Small Asteroid Cluster', '0', '2', '0', '1', '0', '0', '0'),
('2', 'Small Omber Deposit', '0', '2', '0', '2', '0', '0', '0'),
('3', 'Small Kernite and Omber Deposit', '0', '2', '0', '3', '0', '0', '0'),
('4', 'Small Jaspet, Kernite and Omber Deposit', '0', '2', '0', '4', '0', '0', '0'),
('5', 'Small Hemorphite, Jaspet and Kernite Deposit', '0', '2', '0', '5', '0', '0', '0'),
('6', 'Small Hedbergite, Hemorphite and Jaspet Deposit', '0', '2', '0', '6', '0', '0', '0'),
('7', 'Small Hedbergite and Hemorphite Deposit', '0', '2', '0', '7', '0', '0', '0'),
('8', 'Small Gneiss Deposit', '0', '2', '0', '8', '0', '0', '0'),
('9', 'Small Dark Ochre and Gneiss Deposit', '0', '2', '0', '9', '0', '0', '0'),
('10', 'Small Crokite, Dark Ochre and Gneiss Deposit', '0', '2', '0', '10', '0', '0', '0'),
('11', 'Small Spodumain, Crokite and Dark Ochre Deposit', '0', '2', '0', '11', '0', '0', '0'),
('12', 'Small Bistot Deposit', '0', '2', '0', '12', '0', '0', '0'),
('13', 'Small Arkanor and Bistot Deposit', '0', '2', '0', '13', '0', '0', '0'),
('14', 'Small Mercoxit, Arkonor and Bistot Deposit', '0', '2', '0', '14', '0', '0', '0'),
('15', 'Moderate Asteroid Cluster', '0', '2', '0', '15', '0', '0', '0'),
('16', 'Average Omber Deposit', '0', '2', '0', '16', '0', '0', '0'),
('17', 'Average Kernite and Omber Deposit', '0', '2', '0', '17', '0', '0', '0'),
('18', 'Average Jaspet, Kernite and Omber Deposit', '0', '2', '0', '18', '0', '0', '0'),
('19', 'Average Hemorphite, Jaspet and Kernite Deposit', '0', '2', '0', '19', '0', '0', '0'),
('20', 'Average Hedbergite, Hemorphite and Jaspet Deposit', '0', '2', '0', '20', '0', '0', '0'),
('21', 'Average Hedbergite and Hemorphite Deposit', '0', '2', '0', '21', '0', '0', '0'),
('22', 'Average Gneiss Deposit', '0', '2', '0', '22', '0', '0', '0'),
('23', 'Average Dark Ochre and Gneiss Deposit', '0', '2', '0', '23', '0', '0', '0'),
('24', 'Average Crokite, Dark Ochre and Gneiss Deposit', '0', '2', '0', '24', '0', '0', '0'),
('25', 'Average Spodumain, Crokite and Dark Ochre Deposit', '0', '2', '0', '25', '0', '0', '0'),
('26', 'Average Bistot Deposit', '0', '2', '0', '26', '0', '0', '0'),
('27', 'Average Arkanor and Bistot Deposit', '0', '2', '0', '27', '0', '0', '0'),
('28', 'Average Mercoxit, Arkonor and Bistot Deposit', '0', '2', '0', '28', '0', '0', '0'),
('29', 'Large Asteroid Cluster', '0', '2', '0', '29', '0', '0', '0'),
('30', 'Large Omber Deposit', '0', '2', '0', '30', '0', '0', '0'),
('31', 'Large Kernite and Omber Deposit', '0', '2', '0', '31', '0', '0', '0'),
('32', 'Large Jaspet, Kernite and Omber Deposit', '0', '2', '0', '32', '0', '0', '0'),
('33', 'Large Hemorphite, Jaspet and Kernite Deposit', '0', '2', '0', '33', '0', '0', '0'),
('34', 'Large Hedbergite, Hemorphite and Jaspet Deposit', '0', '2', '0', '34', '0', '0', '0'),
('35', 'Large Hedbergite and Hemorphite Deposit', '0', '2', '0', '35', '0', '0', '0'),
('36', 'Large Gneiss Deposit', '0', '2', '0', '36', '0', '0', '0'),
('37', 'Large Dark Ochre and Gneiss Deposit', '0', '2', '0', '37', '0', '0', '0'),
('38', 'Large Crokite, Dark Ochre and Gneiss Deposit', '0', '2', '0', '38', '0', '0', '0'),
('39', 'Large Spodumain, Crokite and Dark Ochre Deposit', '0', '2', '0', '39', '0', '0', '0'),
('40', 'Large Bistot Deposit', '0', '2', '0', '40', '0', '0', '0'),
('41', 'Large Arkanor and Bistot Deposit', '0', '2', '0', '41', '0', '0', '0'),
('42', 'Large Mercoxit, Arkonor and Bistot Deposit', '0', '2', '0', '42', '0', '0', '0'),
(1001,"test dungeon",0,8,0,1001,0,0,0),
(1002,"test data center",0,8,0,1002,0,0,0),
(1003,"test dungeon 2",0,8,0,1003,0,0,0);

-- All asteroid anomaly groups (points to nowhere for most of them as yet)
INSERT INTO dunRoomData  VALUES
('1','1','0','0','0'),
('2','1','0','0','0'),
('3','3','0','0','0'),
('4','1','0','0','0'),
('5','1','0','0','0'),
('6','1','0','0','0'),
('7','1','0','0','0'),
('8','1','0','0','0'),
('9','1','0','0','0'),
('10','1','0','0','0'),
('11','1','0','0','0'),
('12','1','0','0','0'),
('13','1','0','0','0'),
('14','1','0','0','0'),
('15','1','0','0','0'),
('16','1','0','0','0'),
('17','1','0','0','0'),
('18','1','0','0','0'),
('19','1','0','0','0'),
('20','1','0','0','0'),
('21','1','0','0','0'),
('22','1','0','0','0'),
('23','1','0','0','0'),
('24','1','0','0','0'),
('25','1','0','0','0'),
('26','1','0','0','0'),
('27','1','0','0','0'),
('28','1','0','0','0'),
('29','1','0','0','0'),
('30','1','0','0','0'),
('31','1','0','0','0'),
('32','1','0','0','0'),
('33','1','0','0','0'),
('34','1','0','0','0'),
('35','1','0','0','0'),
('36','1','0','0','0'),
('37','1','0','0','0'),
('38','1','0','0','0'),
('39','1','0','0','0'),
('40','1','0','0','0'),
('41','1','0','0','0'),
('42','1','0','0','0'),
(1001,1001,0,0,0),
(1001,1002,15000,200,15000),
(1001,1003,20000,1000,20000),
(1001,1006,0,0,0),
(1001,1007,0,0,0),
(1003,1001,100,0,100),
(1003,1002,15000,200,15000),
(1003,1003,20000,1000,20000),
(1003,1006,100,0,0),
(1003,1007,1000,0,0);

-- Asteroids for small omber anomaly, this needs many MANY more entries for the other 41 sites
INSERT INTO dunGroupData  VALUES
('1','25','1000','0','0'), -- corpse
('3','17868','9000','7000','10000'),
('3','17868','8000','11000','12000'),
('3','17868','10000','10000','13000'),
('3','17868','12000','15000','18000'),
('3','1227','4000','8000','8000'),
('3','1227','10000','9000','11000'),
('3','1227','11000','10000','9000'),
('3','1227','7000','9000','14000'),
('3','1227','10000','9000','12000'),
('3','1227','15000','8000','7000'),
('3','1227','11000','7000','13000'),
('3','1227','10000','9000','15000'),
('3','1227','13000','11000','11000'),
('3','1227','9000','10000','15000'),
('3','1227','15000','8000','16000'),
('3','17867','9000','6000','6000'),
('3','17867','4000','7000','9000'),
('3','17867','11000','9000','8000'),
('3','17867','8000','8000','13000'),
('3','17867','12000','10000','9000'),
('3','17867','14000','8000','14000'),
('3','17867','16000','12000','15000'),
(1001,1194,10000,1200,5000),
(1001,1194,5000,1200,10000),
(1001,1194,0,-1200,10000),
(1001,1194,10000,-1200,10000),
(1002,2372,0,200,0),
(1002,2372,1000,200,1000),
(1002,2372,0,200,1000),
(1002,2372,1000,200,0),
(1003,16748,0,0,0),
(1006,26573,2000,0,2000),
(1006,26574,2500,0,4000),
(1006,26574,4000,0,6000),
(1007,1230,15000,200,15000),
(1007,1230,18000,100,20000),
(1007,1230,20000,150,18000);


INSERT INTO dunGroupInfo  VALUES
('1','CorpseTEST'),
('2','SmallAsteroidCluster'),
('3','SmallOmberDeposit'),
('4','SmallKerniteandOmberDeposit'),
('5','SmallJaspet,KerniteandOmberDeposit'),
('6','SmallHemorphite,JaspetandKerniteDeposit'),
('7','SmallHedbergite,HemorphiteandJaspetDeposit'),
('8','SmallHedbergiteandHemorphiteDeposit'),
('9','SmallGneissDeposit'),
('10','SmallDarkOchreandGneissDeposit'),
('11','SmallCrokite,DarkOchreandGneissDeposit'),
('12','SmallSpodumain,CrokiteandDarkOchreDeposit'),
('13','SmallBistotDeposit'),
('14','SmallArkanorandBistotDeposit'),
('15','SmallMercoxit,ArkonorandBistotDeposit'),
('16','ModerateAsteroidCluster'),
('17','AverageOmberDeposit'),
('18','AverageKerniteandOmberDeposit'),
('19','AverageJaspet,KerniteandOmberDeposit'),
('20','AverageHemorphite,JaspetandKerniteDeposit'),
('21','AverageHedbergite,HemorphiteandJaspetDeposit'),
('22','AverageHedbergiteandHemorphiteDeposit'),
('23','AverageGneissDeposit'),
('24','AverageDarkOchreandGneissDeposit'),
('25','AverageCrokite,DarkOchreandGneissDeposit'),
('26','AverageSpodumain,CrokiteandDarkOchreDeposit'),
('27','AverageBistotDeposit'),
('28','AverageArkanorandBistotDeposit'),
('29','AverageMercoxit,ArkonorandBistotDeposit'),
('30','LargeAsteroidCluster'),
('31','LargeOmberDeposit'),
('32','LargeKerniteandOmberDeposit'),
('33','LargeJaspet,KerniteandOmberDeposit'),
('34','LargeHemorphite,JaspetandKerniteDeposit'),
('35','LargeHedbergite,HemorphiteandJaspetDeposit'),
('36','LargeHedbergiteandHemorphiteDeposit'),
('37','LargeGneissDeposit'),
('38','LargeDarkOchreandGneissDeposit'),
('39','LargeCrokite,DarkOchreandGneissDeposit'),
('40','LargeSpodumain,CrokiteandDarkOchreDeposit'),
('41','LargeBistotDeposit'),
('42','LargeArkanorandBistotDeposit'),
('43','LargeMercoxit,ArkonorandBistotDeposit');
