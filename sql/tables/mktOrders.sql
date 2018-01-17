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
-- Table structure for table `mktOrders`
--

DROP TABLE IF EXISTS `mktOrders`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `mktOrders` (
  `orderID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `typeID` int(10) unsigned NOT NULL DEFAULT '0',
  `ownerID` int(10) unsigned NOT NULL DEFAULT '0',
  `regionID` int(10) unsigned NOT NULL DEFAULT '0',
  `stationID` int(10) unsigned NOT NULL DEFAULT '0',
  `orderRange` int(10) unsigned NOT NULL DEFAULT '0',
  `bid` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `price` double NOT NULL DEFAULT '0',
  `volEntered` int(10) unsigned NOT NULL DEFAULT '0',
  `volRemaining` int(10) unsigned NOT NULL DEFAULT '0',
  `issued` bigint(20) unsigned NOT NULL DEFAULT '0',
  `orderState` int(10) unsigned NOT NULL DEFAULT '0',
  `minVolume` int(10) unsigned NOT NULL DEFAULT '0',
  `contraband` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `accountID` int(10) unsigned NOT NULL DEFAULT '0',
  `duration` int(10) unsigned NOT NULL DEFAULT '0',
  `isCorp` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `solarSystemID` int(11) NOT NULL DEFAULT '0',
  `escrow` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `jumps` tinyint(4) NOT NULL DEFAULT '1',
  PRIMARY KEY (`orderID`),
  KEY `typeID` (`typeID`),
  KEY `regionID` (`regionID`),
  KEY `stationID` (`stationID`),
  KEY `solarSystemID` (`solarSystemID`),
  KEY `regionID_2` (`regionID`),
  KEY `typeID_2` (`typeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `mktOrders`
--

LOCK TABLES `mktOrders` WRITE;
/*!40000 ALTER TABLE `mktOrders` DISABLE KEYS */;
/*!40000 ALTER TABLE `mktOrders` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2018-01-16 19:11:58
