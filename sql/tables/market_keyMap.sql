-- MySQL dump 10.15  Distrib 10.0.24-MariaDB, for Linux (x86_64)
--
-- Host: localhost    Database: new
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
-- Table structure for table `market_keyMap`
--

DROP TABLE IF EXISTS `market_keyMap`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `market_keyMap` (
  `keyID` int(10) unsigned NOT NULL DEFAULT '0',
  `keyType` varchar(100) NOT NULL DEFAULT '',
  `keyName` varchar(100) NOT NULL DEFAULT '',
  `description` varchar(100) NOT NULL DEFAULT '',
  PRIMARY KEY (`keyID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `market_keyMap`
--

LOCK TABLES `market_keyMap` WRITE;
/*!40000 ALTER TABLE `market_keyMap` DISABLE KEYS */;
INSERT INTO `market_keyMap` VALUES (1000,'A','cash',''),(1001,'A','cash2',''),(1002,'A','cash3',''),(1003,'A','cash4',''),(1004,'A','cash5',''),(1005,'A','cash6',''),(1006,'A','cash7',''),(1100,'A','property',''),(1200,'A','aurum',''),(1201,'A','aurum2',''),(1202,'A','aurum3',''),(1203,'A','aurum4',''),(1204,'A','aurum5',''),(1205,'A','aurum6',''),(1206,'A','aurum7',''),(1500,'A','escrow',''),(1800,'A','receivables',''),(2000,'L','payables',''),(2010,'L','gold',''),(2900,'L','equity',''),(3000,'R','sales',''),(4000,'C','purchases','');
/*!40000 ALTER TABLE `market_keyMap` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2017-05-14 22:48:04
