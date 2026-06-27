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
-- Table structure for table `account`
--

DROP TABLE IF EXISTS `account`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `account` (
  `accountID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `clientID` int(10) unsigned NOT NULL DEFAULT '0',
  `accountName` varchar(43) NOT NULL DEFAULT '',
  `password` varchar(43) NOT NULL DEFAULT '',
  `hash` tinyblob,
  `type` tinyint(3) unsigned NOT NULL DEFAULT '23',
  `role` bigint(20) unsigned NOT NULL DEFAULT '0',
  `online` tinyint(1) NOT NULL DEFAULT '0',
  `banned` tinyint(1) NOT NULL DEFAULT '0',
  `logonCount` int(10) unsigned NOT NULL DEFAULT '0',
  `lastLogin` timestamp NULL DEFAULT '0000-00-00 00:00:00' ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`accountID`),
  UNIQUE KEY `accountName` (`accountName`)
) ENGINE=InnoDB AUTO_INCREMENT=9 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `account`
--

LOCK TABLES `account` WRITE;
/*!40000 ALTER TABLE `account` DISABLE KEYS */;
INSERT INTO `account` VALUES (1,2,'allan','','á\ré∆†≠Y¡ƒ„©Vá¥ppsf',23,7131450020691447808,1,0,1669,'2026-06-19 02:59:58'),(2,3,'lee','','ãI<\r§)hÏYÜµêÓß.Âˆx¯•',23,7131450020691447808,0,0,171,'2025-09-27 14:25:15'),(3,4,'groove','','í≥ï÷’Z>⁄¶Çq#’¢ªg˙≠',23,7131450020691447808,0,0,0,'0000-00-00 00:00:00'),(4,5,'artenya','','˛Ô!⁄:∆˚Ze#uÿ—‹Ì≥Cø',23,7131450020691447808,0,0,12,'2020-03-05 01:39:22'),(5,6,'caleya','','Í?HKc~[°≥Npxêr',23,7131450020691447808,0,0,13,'2020-03-05 01:41:38'),(6,7,'ozatomic','','ïÃË^∏<õ‘bç¥áÈJM',23,7131450020691447808,0,0,2,'2020-03-05 06:07:56'),(7,8,'ray','','GÎbæL-)*L˝$Nﬂ›8⁄⁄',23,7131450020691447808,0,0,30,'2020-10-18 22:33:16'),(8,9,'test','','À¥iÍuçVàq‹¶a<[\"È˚`',23,7131450020691447808,0,0,1,'2025-01-31 19:16:42');
/*!40000 ALTER TABLE `account` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2026-06-27  0:42:21
