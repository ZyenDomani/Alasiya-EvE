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
-- Table structure for table `dunTemplates`
--

DROP TABLE IF EXISTS `dunTemplates`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dunTemplates` (
  `dunTemplateID` int(11) NOT NULL,
  `dunTemplateName` varchar(85) COLLATE utf8_bin NOT NULL,
  `dunEntryID` int(11) NOT NULL DEFAULT '0',
  `dunTypeID` int(11) NOT NULL DEFAULT '0',
  `dunSpawnType` int(11) NOT NULL DEFAULT '0',
  `dunRoomID` int(11) NOT NULL DEFAULT '0',
  `dunRooms` int(11) NOT NULL DEFAULT '0',
  `dunRoomTypeID` int(11) NOT NULL DEFAULT '0',
  `dunRoomCategoryID` int(11) NOT NULL DEFAULT '0',
  KEY `dunTemplateID` (`dunTemplateID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `dunTemplates`
--

LOCK TABLES `dunTemplates` WRITE;
/*!40000 ALTER TABLE `dunTemplates` DISABLE KEYS */;
INSERT INTO `dunTemplates` VALUES (1,'Small Asteroid Cluster',0,2,0,1,0,0,0),(2,'Small Omber Deposit',0,2,0,2,0,0,0),(3,'Small Kernite and Omber Deposit',0,2,0,3,0,0,0),(4,'Small Jaspet, Kernite and Omber Deposit',0,2,0,4,0,0,0),(5,'Small Hemorphite, Jaspet and Kernite Deposit',0,2,0,5,0,0,0),(6,'Small Hedbergite, Hemorphite and Jaspet Deposit',0,2,0,6,0,0,0),(7,'Small Hedbergite and Hemorphite Deposit',0,2,0,7,0,0,0),(8,'Small Gneiss Deposit',0,2,0,8,0,0,0),(9,'Small Dark Ochre and Gneiss Deposit',0,2,0,9,0,0,0),(10,'Small Crokite, Dark Ochre and Gneiss Deposit',0,2,0,10,0,0,0),(11,'Small Spodumain, Crokite and Dark Ochre Deposit',0,2,0,11,0,0,0),(12,'Small Bistot Deposit',0,2,0,12,0,0,0),(13,'Small Arkanor and Bistot Deposit',0,2,0,13,0,0,0),(14,'Small Mercoxit, Arkonor and Bistot Deposit',0,2,0,14,0,0,0),(15,'Moderate Asteroid Cluster',0,2,0,15,0,0,0),(16,'Average Omber Deposit',0,2,0,16,0,0,0),(17,'Average Kernite and Omber Deposit',0,2,0,17,0,0,0),(18,'Average Jaspet, Kernite and Omber Deposit',0,2,0,18,0,0,0),(19,'Average Hemorphite, Jaspet and Kernite Deposit',0,2,0,19,0,0,0),(20,'Average Hedbergite, Hemorphite and Jaspet Deposit',0,2,0,20,0,0,0),(21,'Average Hedbergite and Hemorphite Deposit',0,2,0,21,0,0,0),(22,'Average Gneiss Deposit',0,2,0,22,0,0,0),(23,'Average Dark Ochre and Gneiss Deposit',0,2,0,23,0,0,0),(24,'Average Crokite, Dark Ochre and Gneiss Deposit',0,2,0,24,0,0,0),(25,'Average Spodumain, Crokite and Dark Ochre Deposit',0,2,0,25,0,0,0),(26,'Average Bistot Deposit',0,2,0,26,0,0,0),(27,'Average Arkanor and Bistot Deposit',0,2,0,27,0,0,0),(28,'Average Mercoxit, Arkonor and Bistot Deposit',0,2,0,28,0,0,0),(29,'Large Asteroid Cluster',0,2,0,29,0,0,0),(30,'Large Omber Deposit',0,2,0,30,0,0,0),(31,'Large Kernite and Omber Deposit',0,2,0,31,0,0,0),(32,'Large Jaspet, Kernite and Omber Deposit',0,2,0,32,0,0,0),(33,'Large Hemorphite, Jaspet and Kernite Deposit',0,2,0,33,0,0,0),(34,'Large Hedbergite, Hemorphite and Jaspet Deposit',0,2,0,34,0,0,0),(35,'Large Hedbergite and Hemorphite Deposit',0,2,0,35,0,0,0),(36,'Large Gneiss Deposit',0,2,0,36,0,0,0),(37,'Large Dark Ochre and Gneiss Deposit',0,2,0,37,0,0,0),(38,'Large Crokite, Dark Ochre and Gneiss Deposit',0,2,0,38,0,0,0),(39,'Large Spodumain, Crokite and Dark Ochre Deposit',0,2,0,39,0,0,0),(40,'Large Bistot Deposit',0,2,0,40,0,0,0),(41,'Large Arkanor and Bistot Deposit',0,2,0,41,0,0,0),(42,'Large Mercoxit, Arkonor and Bistot Deposit',0,2,0,42,0,0,0),(1001,'test dungeon',0,8,0,1001,0,0,0),(1002,'test data center',0,8,0,1002,0,0,0),(1003,'test dungeon 2',0,8,0,1003,0,0,0);
/*!40000 ALTER TABLE `dunTemplates` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2017-05-14 22:47:41
