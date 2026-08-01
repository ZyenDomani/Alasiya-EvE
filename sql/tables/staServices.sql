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
-- Table structure for table `staServices`
--

DROP TABLE IF EXISTS `staServices`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `staServices` (
  `serviceID` int(11) NOT NULL,
  `serviceName` varchar(100) DEFAULT NULL,
  `description` varchar(1000) DEFAULT NULL,
  `serviceNameID` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`serviceID`)
) ENGINE=Aria DEFAULT CHARSET=utf8 PAGE_CHECKSUM=1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `staServices`
--

LOCK TABLES `staServices` WRITE;
/*!40000 ALTER TABLE `staServices` DISABLE KEYS */;
INSERT INTO `staServices` VALUES (1,'Bounty Missions',NULL,61496),(2,'Assassination Missions',NULL,61497),(4,'Courier Missions',NULL,61498),(8,'Interbus',NULL,61499),(16,'Reprocessing Plant',NULL,61500),(32,'Refinery',NULL,61501),(64,'Market',NULL,61502),(128,'Black Market',NULL,61503),(256,'Stock Exchange',NULL,61504),(512,'Cloning',NULL,61505),(1024,'Surgery',NULL,61506),(2048,'DNA Therapy',NULL,61507),(4096,'Repair Facilities',NULL,61508),(8192,'Factory',NULL,61509),(16384,'Laboratory',NULL,61510),(32768,'Gambling',NULL,61511),(65536,'Fitting',NULL,61512),(131072,'Paintshop',NULL,61513),(262144,'News',NULL,61514),(524288,'Storage',NULL,61515),(1048576,'Insurance','Used to buy insurance for ships.',61516),(2097152,'Docking',NULL,61517),(4194304,'Office Rental',NULL,61518),(8388608,'Jump Clone Facility',NULL,61519),(16777216,'Loyalty Point Store',NULL,61520),(33554432,'Navy Offices',NULL,61521);
/*!40000 ALTER TABLE `staServices` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2026-08-01  9:55:39
