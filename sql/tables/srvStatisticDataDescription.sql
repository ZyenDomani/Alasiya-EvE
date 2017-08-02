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
-- Table structure for table `srvStatisticDataDescription`
--

DROP TABLE IF EXISTS `srvStatisticDataDescription`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `srvStatisticDataDescription` (
  `dataID` tinyint(1) unsigned NOT NULL,
  `dataName` varchar(30) NOT NULL,
  `dataDescription` varchar(200) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Descriptions for DataNames in srvStatisticData table';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `srvStatisticDataDescription`
--

LOCK TABLES `srvStatisticDataDescription` WRITE;
/*!40000 ALTER TABLE `srvStatisticDataDescription` DISABLE KEYS */;
INSERT INTO `srvStatisticDataDescription` VALUES (1,'Turret Shots Fired','Shots fired from turrents on player ships.'),(2,'Missiles Launched','Missiles fired from launchers on player ships.'),(3,'NPC Ships killed','NPC ships killed.'),(4,'Player Ships killed','Players popped (not tracking podded).'),(5,'Bounties Paid','Amount of bounty payouts in ISK.'),(6,'Bounties Placed','Amount of bounties placed in ISK.'),(7,'Ore Mined','M3 of ore mined.'),(8,'ISK Spent In Market','ISK spent in the market, not including broker fees.'),(9,'Player Logins','Number of player logins.');
/*!40000 ALTER TABLE `srvStatisticDataDescription` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2017-08-01 18:36:52
