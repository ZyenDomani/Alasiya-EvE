-- MySQL dump 10.15  Distrib 10.0.24-MariaDB, for Linux (x86_64)
--
-- Host: localhost    Database: alasiya-new
-- ------------------------------------------------------
-- Server version	10.0.24-MariaDB

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
-- Table structure for table `posStructureData`
--

DROP TABLE IF EXISTS `posStructureData`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `posStructureData` (
  `itemID` int(10) NOT NULL DEFAULT '0',
  `towerID` int(10) NOT NULL DEFAULT '0',
  `planetID` int(10) NOT NULL DEFAULT '0',
  `harmonic` int(10) NOT NULL DEFAULT '0',
  `password` varchar(50) NOT NULL DEFAULT '''''',
  `standingOwnerID` int(10) NOT NULL DEFAULT '0',
  `state` tinyint(1) NOT NULL DEFAULT '-1',
  `standing` double NOT NULL DEFAULT '0',
  `status` double NOT NULL DEFAULT '0',
  `timestamp` bigint(20) NOT NULL DEFAULT '0',
  `rotationX` double NOT NULL DEFAULT '0',
  `rotationY` double NOT NULL DEFAULT '0',
  `rotationZ` double NOT NULL DEFAULT '0',
  `statusDrop` tinyint(1) NOT NULL DEFAULT '0',
  `corpWar` tinyint(1) NOT NULL DEFAULT '0',
  `showInCalendar` tinyint(1) NOT NULL DEFAULT '0',
  `sendFuelNotifications` tinyint(1) NOT NULL DEFAULT '0',
  `allowCorp` tinyint(1) NOT NULL DEFAULT '0',
  `allowAlliance` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`itemID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='POS - Structure Data';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `posStructureData`
--

LOCK TABLES `posStructureData` WRITE;
/*!40000 ALTER TABLE `posStructureData` DISABLE KEYS */;
/*!40000 ALTER TABLE `posStructureData` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2017-08-01 18:36:49
