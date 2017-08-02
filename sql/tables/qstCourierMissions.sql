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
-- Table structure for table `qstCourierMissions`
--

DROP TABLE IF EXISTS `qstCourierMissions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `qstCourierMissions` (
  `missionID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `kind` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `state` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `availabilityID` int(10) unsigned DEFAULT NULL,
  `inOrder` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `description` text NOT NULL,
  `issuerID` int(10) unsigned DEFAULT NULL,
  `assigneeID` int(10) unsigned DEFAULT NULL,
  `acceptFee` double NOT NULL DEFAULT '0',
  `acceptorID` int(10) unsigned DEFAULT NULL,
  `dateIssued` int(10) unsigned NOT NULL DEFAULT '0',
  `dateExpires` int(10) unsigned NOT NULL DEFAULT '0',
  `dateAccepted` int(10) unsigned NOT NULL DEFAULT '0',
  `dateCompleted` int(10) unsigned NOT NULL DEFAULT '0',
  `totalReward` double NOT NULL DEFAULT '0',
  `tracking` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `pickupStationID` int(10) unsigned DEFAULT NULL,
  `craterID` int(10) unsigned DEFAULT NULL,
  `dropStationID` int(10) unsigned DEFAULT NULL,
  `volume` double NOT NULL DEFAULT '0',
  `pickupSolarSystemID` int(10) unsigned DEFAULT NULL,
  `pickupRegionID` int(10) unsigned DEFAULT NULL,
  `dropSolarSystemID` int(10) unsigned DEFAULT NULL,
  `dropRegionID` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`missionID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `qstCourierMissions`
--

LOCK TABLES `qstCourierMissions` WRITE;
/*!40000 ALTER TABLE `qstCourierMissions` DISABLE KEYS */;
/*!40000 ALTER TABLE `qstCourierMissions` ENABLE KEYS */;
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
