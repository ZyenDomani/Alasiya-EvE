-- MySQL dump 10.15  Distrib 10.0.36-MariaDB, for Linux (x86_64)
--
-- Host: localhost    Database: EvE_AlasiyaDev
-- ------------------------------------------------------
-- Server version	10.0.36-MariaDB

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `qstCourier`
--

DROP TABLE IF EXISTS `qstCourier`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `qstCourier` (
  `id` int(5) NOT NULL DEFAULT '0',
  `briefingID` int(5) NOT NULL DEFAULT '0',
  `name` text,
  `level` tinyint(1) NOT NULL DEFAULT '0',
  `typeID` tinyint(1) NOT NULL DEFAULT '0',
  `important` tinyint(1) NOT NULL DEFAULT '0',
  `storyline` tinyint(1) NOT NULL DEFAULT '0',
  `raceID` tinyint(2) NOT NULL DEFAULT '0',
  `itemTypeID` int(6) NOT NULL DEFAULT '0',
  `itemQty` int(10) NOT NULL DEFAULT '0',
  `rewardItemID` int(11) NOT NULL DEFAULT '0',
  `rewardItemQty` int(11) NOT NULL DEFAULT '0',
  `bonusTime` int(5) NOT NULL DEFAULT '0',
  `collateral` int(7) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=Aria DEFAULT CHARSET=utf8 PAGE_CHECKSUM=1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `qstCourier`
--

LOCK TABLES `qstCourier` WRITE;
/*!40000 ALTER TABLE `qstCourier` DISABLE KEYS */;
INSERT INTO `qstCourier` VALUES (54833,129523,'Need A Lift  ***special pickup location***',0,3,1,0,1,0,0,0,0,0,0),(55005,130400,'Transaction Data Delivery',1,3,1,1,0,11702,25,0,1,20,10000),(55006,130404,'Transaction Data Delivery',2,3,1,1,8,11602,150,0,1,30,250000),(55007,130408,'Transaction Data Delivery',2,3,1,1,1,11604,150,0,0,30,250000),(55008,130412,'Transaction Data Delivery',2,3,1,1,4,11606,150,0,0,30,250000),(55009,130416,'Transaction Data Delivery',2,3,1,1,6,11608,150,0,0,30,250000),(55010,130420,'Transaction Data Delivery',2,3,1,1,2,11607,150,0,0,30,250000),(55011,130424,'Transaction Data Delivery',4,3,1,1,0,11702,750,0,0,45,750000),(55012,130428,'Transaction Data Delivery',5,3,1,1,0,11702,1500,0,0,60,3750000),(55184,130956,'A Special Breed',1,3,0,0,4,0,0,0,0,0,0),(55186,143439,'A Humble Gift',1,3,0,0,0,2627,4,0,0,0,0),(55187,130960,'Miracle Drug',1,3,0,0,0,2660,5,0,0,30,0),(55188,130962,'Bio-Engineering Pays',1,3,0,0,0,2668,3,0,0,0,0),(55189,130964,'Army Recruits',1,3,0,0,0,2662,5,0,0,0,0),(55190,130966,'Supplies For The Needy',2,3,0,0,0,2632,12,0,0,30,0),(55191,130968,'Breakdown',1,3,0,0,0,2640,4,0,0,0,0),(55192,130970,'Party Goods',1,3,0,0,0,2646,3,0,0,20,0),(55193,130972,'Cool Cat',1,3,0,0,0,2598,3,0,0,0,0),(55194,130974,'Save The Children',1,3,0,0,0,2596,3,0,0,0,0),(55205,130997,'Broken Reactor',1,3,0,0,0,3687,75,0,0,20,0),(55206,130999,'Homeless People Everywhere',3,3,0,0,0,26783,30,0,0,0,0),(55207,131000,'High Command',1,3,0,0,0,3814,100,0,0,0,0),(55208,131001,'Dirt',1,3,0,0,0,2645,5,0,0,30,0),(55320,131498,'Move The Goods',1,3,0,0,0,16044,20,0,0,20,0),(55321,131502,'Move The Goods',2,3,0,0,0,16042,20,0,0,0,0),(55322,131504,'Move The Goods',3,3,0,0,0,16045,20,0,0,0,0),(55325,131525,'A Special Delivery',1,3,1,1,0,16044,5,0,0,20,0),(55326,131529,'A Special Delivery',2,3,1,1,0,16042,30,0,0,30,0),(55327,131534,'A Special Delivery',3,3,1,1,0,16045,75,0,0,30,0),(55328,131539,'A Special Delivery',4,3,1,1,0,16043,40,0,0,45,0),(55329,131544,'A Special Delivery',5,3,1,1,0,16041,40,0,0,45,0),(55472,132243,'A Special Delivery',2,3,1,1,0,0,10,0,0,20,0),(55473,132247,'A Special Delivery',4,3,1,1,0,0,20,0,0,30,0),(56901,134918,'Slave Shipment',1,3,0,0,4,2665,4,0,0,0,0),(56903,134937,'Send in the Marines',1,3,0,0,0,2670,3,0,0,30,0),(57114,143437,'Medical Delivery',1,3,0,0,1,2653,3,0,0,0,0),(57117,145160,'Wee Bug Problem',2,3,0,0,0,2631,7,0,0,0,0),(57824,143755,'Beefing Up',1,3,0,0,0,21734,2,0,0,0,0),(57825,143443,'Future Leaders',1,3,0,0,0,12243,2,0,0,0,0),(57826,144416,'Giving Shelter',2,3,0,0,0,2674,7,0,0,0,0),(57827,143459,'Hunger Strikes',1,3,0,0,0,2610,2,0,0,20,0),(57828,143441,'Incriminating Evidence',2,3,0,0,0,2654,5,0,0,0,0),(57829,143450,'Marriage',2,3,0,0,0,17765,5,0,0,0,0),(57830,143452,'Rat Problem',2,3,0,0,0,26775,2,0,0,0,0),(57831,143449,'Stolen Arms',1,3,0,0,0,21044,10,0,0,20,0),(57832,143416,'Under Construction',2,3,0,0,0,26791,6,0,0,0,0),(57833,143442,'Air',1,3,0,0,0,2632,3,0,0,0,0),(57834,143659,'Arms Dealer',1,3,0,0,0,2644,3,0,0,0,0),(57835,143440,'Cigarettes',1,3,0,0,0,2618,3,0,0,0,0),(57836,143447,'Ridiculous Oil Prices',0,3,0,0,0,0,0,0,0,0,0),(57837,143446,'Shipping Problems',1,3,0,0,0,26778,3,0,0,0,0),(57838,144582,'Tightening Security',1,3,0,0,0,2670,3,0,0,0,0),(57839,145156,'Tourists *broke*',1,3,0,0,0,0,40,3719,0,0,0),(57840,131015,'A Bit of Terraforming',2,3,0,0,0,2608,5,0,0,0,0),(57841,0,'A Feast Fit for a King',1,3,0,0,0,0,0,0,0,0,0),(57842,0,'A Feast Fit for a King',2,3,0,0,0,0,0,0,0,0,0),(57843,0,'A Total Mess',1,3,0,0,0,0,0,0,0,0,0),(57844,143756,'Beefing Up',2,3,0,0,0,21734,5,0,0,0,0),(57847,130969,'Breakdown',2,3,0,0,0,2639,6,0,0,0,0),(57850,130983,'Equipped For The Job',2,3,0,0,0,26781,5,0,0,0,0),(57851,130975,'Fertile Ground',2,3,0,0,0,2608,4,0,0,0,0),(57852,0,'From Bad To Worse',0,3,0,0,0,0,0,0,0,0,0),(57853,143444,'Future Leaders',2,3,0,0,0,2675,2,0,0,0,0),(57854,144970,'Garbage Man',4,3,0,0,0,2615,60,0,0,0,0),(57855,144417,'Giving Shelter',4,3,0,0,0,2728,9,0,0,0,0),(57856,130977,'Good Harvest',2,3,0,0,0,26785,7,0,0,0,0),(57858,130987,'Human Problem',1,3,0,0,0,12110,5,0,0,20,0),(57859,144413,'Hunger Strikes',2,3,0,0,0,2715,7,0,0,0,0),(57860,0,'Important Launch',3,3,0,0,0,0,0,0,0,0,0),(57862,143451,'Incriminating Evidence',4,3,0,0,0,2723,9,0,0,0,0),(57863,0,'Keeping The Grunts Happy',0,3,0,0,0,0,0,0,0,0,0),(57864,143454,'Marriage',3,3,0,0,0,2666,1,0,0,0,0),(57865,131013,'Miner Problems',1,3,0,0,0,21466,15,0,0,20,0),(57866,131506,'Move The Goods',4,3,0,0,0,16043,20,0,0,0,0),(57867,131012,'My Little Girl',1,3,0,0,0,23547,150,0,0,0,0),(57868,131014,'Our Boys Are Getting Bored',1,3,0,0,0,3647,25,0,0,20,0),(57869,131030,'Our Boys Are Getting Bored',2,3,0,0,0,3647,140,0,0,30,0),(57870,143455,'Rat Problem',3,3,0,0,0,2726,3,0,0,0,0),(57871,130979,'Silly Miners',2,3,0,0,0,2602,4,0,0,0,0),(57872,145178,'Stolen Arms',2,3,0,0,0,21044,30,0,0,0,0),(57873,144570,'Stress Reliever',2,3,0,0,0,26790,5,0,0,0,0),(57874,131010,'Take This Away',1,3,0,0,0,2615,3,0,0,20,0),(57877,130981,'Troublemakers',2,3,0,0,0,3647,150,0,0,0,0),(57878,143418,'Under Construction',5,3,0,0,0,2710,10,0,0,0,0),(57880,130984,'Equipped For The Job',4,3,0,0,0,2722,9,0,0,0,0),(57881,130976,'Fertile Ground',4,3,0,0,0,2714,9,0,0,0,0),(57882,130978,'Good Harvest',4,3,0,0,0,2716,7,0,0,0,0),(57883,130990,'Human Problem',2,3,0,0,0,26783,3,0,0,0,0),(57884,130980,'Silly Miners',4,3,0,0,0,2712,8,0,0,0,0),(57885,144571,'Stress Reliever',4,3,0,0,0,2684,8,0,0,0,0),(57886,130982,'Troublemakers',4,3,0,0,0,3647,400,0,0,0,0),(57887,131027,'Miner Problems',2,3,0,0,0,21466,95,0,0,0,0),(57888,131026,'My Little Girl',2,3,0,0,0,23547,3500,0,0,0,0),(57889,131025,'Take This Away',2,3,0,0,0,2717,3,0,0,0,0),(57890,131035,'A Bit of Terraforming',4,3,0,0,0,2714,7,0,0,0,0),(57891,0,'A Feast Fit for a King',3,3,0,0,0,0,0,0,0,0,0),(57892,0,'A Total Mess',2,3,0,0,0,0,0,0,0,0,0),(57894,0,'Important Launch',4,3,0,0,0,0,0,0,0,0,0),(57896,0,'Keeping The Grunts Happy',0,3,0,0,0,0,0,0,0,0,0),(57897,131047,'Our Boys Are Getting Bored',3,3,0,0,0,3647,350,0,0,0,0),(57899,0,'A Feast Fit for a King',4,3,0,0,0,0,0,0,0,0,0),(57901,131031,'Breakdown',3,3,0,0,0,2624,15,0,0,0,0),(57904,0,'From Bad To Worse',0,3,0,0,0,0,0,0,0,0,0),(57905,144970,'Garbage Man',5,3,0,0,0,2717,10,0,0,0,0),(57907,131048,'Our Boys Are Getting Bored',4,3,0,0,0,3647,650,0,0,0,0),(57910,136051,'A Humble Gift',3,3,0,0,0,2688,3,0,0,0,0),(57911,130959,'A Special Breed',3,3,0,0,4,0,0,0,0,0,0),(57912,130965,'Army Recruits',3,3,0,0,0,2704,3,0,0,0,0),(57913,130963,'Bio-Engineering Pays',3,3,0,0,0,2701,4,0,0,0,0),(57914,131034,'Breakdown',4,3,0,0,0,2687,3,0,0,0,0),(57915,130973,'Cool Cat',3,3,0,0,0,2616,4,0,0,0,0),(57916,130971,'Party Goods',3,3,0,0,0,2699,3,0,0,40,0),(57917,143412,'Save The Children',3,3,0,0,0,2691,4,0,0,0,0),(57918,130967,'Supplies For The Needy',3,3,0,0,0,2689,25,0,0,0,0),(57919,131016,'Broken Reactor',2,3,0,0,0,3687,150,0,0,0,0),(57920,131022,'Dirt',3,3,0,0,0,2695,7,0,0,0,0),(57921,131020,'High Command',2,3,0,0,0,3814,200,0,0,0,0),(57922,131018,'Homeless People Everywhere',4,3,0,0,0,2702,30,0,0,0,0),(57985,143460,'Air',3,3,0,0,0,2689,4,0,0,0,0),(57986,143660,'Arms Dealer',3,3,0,0,0,2694,3,0,0,0,0),(57987,144575,'Cigarettes',3,3,0,0,0,2684,4,0,0,0,0),(57990,130961,'Miracle Drug',3,3,0,0,0,2682,4,0,0,0,0),(57991,143453,'Ridiculous Oil Prices',0,3,0,0,0,0,0,0,0,0,0),(57992,143415,'Salve for Sis',1,3,0,0,0,26904,1,0,0,0,0),(57993,143448,'Shipping Problems',3,3,0,0,0,2685,3,0,0,0,0),(57995,144583,'Tightening Security',3,3,0,0,0,2704,7,0,0,0,0),(57996,145157,'Tourists *broke*',2,3,0,0,0,0,20,2678,0,0,0),(57997,130741,'Corporate Documents',2,3,0,0,0,2599,10,0,0,0,0),(58057,143795,'A Piece of History',1,3,1,1,4,2215,10,0,0,0,0),(58061,0,'Their Secret Defense',0,3,0,0,0,0,0,0,0,0,0),(58062,143840,'A Cargo With Attitude',3,3,0,1,0,0,0,0,0,0,0),(58079,143998,'Ditanium',1,3,1,1,4,2972,10,0,0,0,0),(58083,144034,'Hunting Black Dog',3,3,1,1,0,2984,6,0,0,0,0),(58084,144429,'Operation Doorstop',2,3,1,1,0,2898,3,0,0,0,0),(58090,0,'Their Secret Defense',0,3,0,0,0,0,0,0,0,0,0),(58091,144529,'The State of the Empire',2,3,1,1,4,3013,40,0,0,0,0),(58107,144234,'A Cargo With Attitude',4,3,0,1,0,0,0,0,0,0,0),(58114,144297,'A Piece of History',3,3,1,1,4,2216,10,0,0,0,0),(58118,144342,'Hunting Black Dog',4,3,1,1,0,2984,25,0,0,0,0),(58120,144431,'Operation Doorstop',4,3,1,1,0,2990,10,0,0,0,0),(58123,144533,'The State of the Empire',4,3,1,1,4,3014,80,0,0,0,0),(58350,144306,'Ditanium',4,3,1,1,4,2973,15,0,0,0,0),(58354,144459,'Materials For War Preparation',1,3,0,1,0,1230,1000,0,1,10,0),(58355,144461,'Materials For War Preparation',2,3,0,1,0,1228,1500,0,1,15,0),(58356,144550,'Materials For War Preparation',3,3,0,1,0,1227,10000,0,1,20,0),(58357,144553,'Materials For War Preparation',4,3,0,1,0,20,8000,0,1,30,0),(58358,144556,'Materials For War Preparation',5,3,0,1,0,1231,4000,0,1,45,0),(58089,144084,'The Governor’s Ball',2,3,1,1,4,3006,10,0,0,23,100000),(58124,144381,'The Governor’s Ball',4,3,1,1,4,3008,25,0,0,18,150000),(55202,130994,'Reactor Meltdown',1,3,0,0,0,0,15,0,0,20,0),(57204,131009,'Coolant Run',2,3,0,0,0,9832,45,0,0,30,0),(76965,145157,'Tourists *broke*',4,3,0,0,0,0,20,2708,0,0,0);
/*!40000 ALTER TABLE `qstCourier` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2026-08-01  9:55:37
