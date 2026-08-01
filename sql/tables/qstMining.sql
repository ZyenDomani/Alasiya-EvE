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
-- Table structure for table `qstMining`
--

DROP TABLE IF EXISTS `qstMining`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `qstMining` (
  `id` int(5) NOT NULL DEFAULT '0',
  `briefingID` int(5) NOT NULL DEFAULT '0',
  `name` text,
  `level` tinyint(1) NOT NULL DEFAULT '0',
  `typeID` tinyint(1) NOT NULL DEFAULT '0',
  `itemTypeID` int(6) NOT NULL DEFAULT '0',
  `itemQty` int(10) NOT NULL DEFAULT '0',
  `rewardItemID` int(11) NOT NULL DEFAULT '0',
  `rewardItemQty` int(11) NOT NULL DEFAULT '0',
  `bonusTime` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=Aria DEFAULT CHARSET=utf8 PAGE_CHECKSUM=1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `qstMining`
--

LOCK TABLES `qstMining` WRITE;
/*!40000 ALTER TABLE `qstMining` DISABLE KEYS */;
INSERT INTO `qstMining` VALUES (56752,135338,'Like Drones to a Cloud',1,5,28630,20,0,0,0),(56871,134495,'Starting Simple',1,5,28617,20000,0,0,0),(56873,134531,'Asteroid Catastrophe',1,5,28618,3600,0,0,30),(56874,134551,'Burnt Traces',1,5,28618,3600,0,0,30),(56875,134561,'Down and Dirty',2,5,28618,7500,0,0,0),(56878,134624,'Mercium Belt',1,5,28619,1000,0,0,0),(56879,134624,'Mercium Belt',3,5,28619,10000,0,0,0),(56880,134624,'Mercium Belt',4,5,28619,60000,0,0,0),(56881,134652,'Unknown Events',2,5,28620,5000,0,0,0),(56883,134630,'Mercium Experiments',1,5,28619,480,0,0,30),(56913,135098,'Persistent Pests',3,5,28617,40000,0,0,0),(56915,135124,'Drone Distribution',3,5,28617,40000,0,0,0),(56916,0,'Coming \'Round the Mountain',3,5,0,4500,0,0,0),(56919,135182,'Beware They Live',3,5,28619,15000,0,0,0),(56920,135191,'Pile of Pithix',3,5,28621,9000,0,0,0),(56923,135232,'Ice Installation',4,5,28628,20,0,0,0),(56924,135247,'Cheap Chills',4,5,28627,20,0,0,0),(56925,144468,'Like Drones to a Cloud',2,5,28630,150,0,0,0),(56926,144501,'Like Drones to a Cloud',3,5,28630,300,0,0,0),(56927,135274,'Mother Lode',4,5,28625,2800,0,0,0),(56928,135290,'Geodite and Gemology',4,5,28624,2800,0,0,0),(56929,135296,'Feeding the Giant',4,5,28623,5600,0,0,0),(56930,135325,'Arisite Envy',4,5,28622,9000,0,0,0),(56931,135325,'Not Gneiss at All',4,5,0,45000,0,0,0),(56934,135381,'Gas Injections',4,5,28629,425,0,0,0),(57062,139242,'Data Mining',2,2,1228,2000,0,0,0),(57063,139245,'Data Mining',2,2,1228,2000,0,0,0),(57095,144657,'Like Drones to a Cloud',4,5,28630,425,0,0,0),(57174,140253,'Stay Frosty',3,5,28627,10,0,0,0),(57288,144472,'Claimjumpers',2,5,28617,18000,0,0,0),(58553,134624,'Mercium Belt',2,5,28619,6000,0,0,0),(58555,144663,'Like Drones to a Cloud',5,5,28630,600,0,0,0),(56917,140163,'A Better World',3,5,28618,20000,0,0,0);
/*!40000 ALTER TABLE `qstMining` ENABLE KEYS */;
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
