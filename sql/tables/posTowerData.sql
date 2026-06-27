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
-- Table structure for table `posTowerData`
--

DROP TABLE IF EXISTS `posTowerData`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `posTowerData` (
  `itemID` int(10) NOT NULL DEFAULT '0',
  `harmonic` int(10) NOT NULL DEFAULT '0',
  `password` varchar(50) NOT NULL DEFAULT '''''',
  `status` float NOT NULL DEFAULT '0',
  `standing` float NOT NULL DEFAULT '0',
  `standingOwnerID` int(10) NOT NULL DEFAULT '0',
  `corpWar` tinyint(1) NOT NULL DEFAULT '0',
  `statusDrop` tinyint(1) NOT NULL DEFAULT '0',
  `allyStandings` tinyint(1) NOT NULL DEFAULT '0',
  `showInCalendar` tinyint(1) NOT NULL DEFAULT '0',
  `sendFuelNotifications` tinyint(1) NOT NULL DEFAULT '0',
  `allowCorp` tinyint(1) NOT NULL DEFAULT '0',
  `allowAlliance` tinyint(1) NOT NULL DEFAULT '0',
  `anchor` tinyint(1) NOT NULL DEFAULT '0',
  `unanchor` tinyint(1) NOT NULL DEFAULT '0',
  `online` tinyint(1) NOT NULL DEFAULT '0',
  `offline` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`itemID`),
  KEY `itemID` (`itemID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='POS Tower Data';
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2026-06-27  0:42:47
