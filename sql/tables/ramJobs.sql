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
-- Table structure for table `ramJobs`
--

DROP TABLE IF EXISTS `ramJobs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ramJobs` (
  `jobID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `eventID` int(10) unsigned NOT NULL DEFAULT '0',
  `ownerID` int(10) unsigned NOT NULL,
  `installerID` int(10) unsigned NOT NULL,
  `assemblyLineID` int(10) unsigned NOT NULL,
  `installedItemID` int(10) unsigned NOT NULL,
  `installTime` bigint(20) unsigned NOT NULL,
  `beginProductionTime` bigint(20) unsigned NOT NULL,
  `pauseProductionTime` bigint(20) unsigned DEFAULT NULL,
  `endProductionTime` bigint(20) unsigned NOT NULL,
  `runs` smallint(2) NOT NULL DEFAULT '0',
  `outputFlag` smallint(5) unsigned NOT NULL DEFAULT '0',
  `completedStatusID` tinyint(2) unsigned NOT NULL DEFAULT '0',
  `licensedProductionRuns` tinyint(4) DEFAULT NULL,
  PRIMARY KEY (`jobID`),
  KEY `RAMJOBS_ASSEMBLYLINES` (`assemblyLineID`)
) ENGINE=InnoDB AUTO_INCREMENT=25 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2026-06-27  0:42:48
