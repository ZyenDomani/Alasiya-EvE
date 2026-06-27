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
-- Table structure for table `qstMultiPart`
--

DROP TABLE IF EXISTS `qstMultiPart`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `qstMultiPart` (
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
-- Dumping data for table `qstMultiPart`
--

LOCK TABLES `qstMultiPart` WRITE;
/*!40000 ALTER TABLE `qstMultiPart` DISABLE KEYS */;
INSERT INTO `qstMultiPart` VALUES (54412,88359,'Kidnappers Strike - The Flu Outbreak (6 of 10)',3,3,0,0,0,0,2732,0,0,65,0),(56911,0,'Hunting a Heretic - Book Burning (2 of 3)',0,3,0,0,0,0,0,0,0,0,0),(56912,0,'Hunting a Heretic - Crackpot Captured! (3 of 3)',0,3,0,0,0,0,0,0,0,0,0);
/*!40000 ALTER TABLE `qstMultiPart` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2026-06-27  0:42:47
