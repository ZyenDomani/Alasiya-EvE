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
-- Table structure for table `dunRoomInfo`
--

DROP TABLE IF EXISTS `dunRoomInfo`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dunRoomInfo` (
  `dunRoomID` int(11) NOT NULL,
  `dunRoomName` varchar(150) COLLATE utf8_bin NOT NULL,
  `dunRoomSpawnID` int(11) NOT NULL DEFAULT '0',
  KEY `dunRoomID` (`dunRoomID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `dunRoomInfo`
--

/*!40000 ALTER TABLE `dunRoomInfo` DISABLE KEYS */;
INSERT INTO `dunRoomInfo`(`dunRoomID`, `dunRoomName`, `dunRoomSpawnID`)
VALUES
-- 7xxx mining rooms
-- misc common ore types
(7011,'Small Veldspar Deposit', 0),
(7012,'Small Scordite Deposit', 0),
(7013,'Small Pyroxeres Deposit', 0),
(7014,'Small Plagioclase Deposit', 0),
(7015,'Small Veldspar and Scordite Deposit', 0),

-- 7066 - 7079 ice (rat type and size by region)
(7066,'White Glaze Belt',0),
(7067,'Blue Ice Belt',0),
(7068,'Glacial Mass Belt',0),
(7069,'Glare Crust Belt',0),
(7076,'Enriched White Glaze Belt',0),
(7077,'Thick Blue Ice Belt',0),
(7078,'Smooth Glacial Mass Belt',0),
(7079,'Pristine Glare Crust Belt',0),


(7111,'Small Omber Deposit', 0),
(7112,'Small Kernite and Omber Deposit', 0),
(7113,'Small Jaspet, Kernite and Omber Deposit', 0),
(7211,'Small Hemorphite, Jaspet and Kernite Deposit', 0),
(7212,'Small Hedbergite, Hemorphite and Jaspet Deposit', 0),
(7213,'Small Hedbergite and Hemorphite Deposit', 0),
(7214,'Small Gneiss Deposit', 0),
(7311,'Small Dark Ochre and Gneiss Deposit', 0),
(7312,'Small Crokite, Dark Ochre and Gneiss Deposit', 0),
(7313,'Small Spodumain, Crokite and Dark Ochre Deposit', 0),
(7511,'Small Bistot Deposit', 0),
(7512,'Small Arkanor and Bistot Deposit', 0),
(7513,'Small Mercoxit, Arkonor and Bistot Deposit', 0),

(7121,'Medium Omber Deposit', 0),
(7122,'Medium Kernite and Omber Deposit', 0),
(7123,'Medium Jaspet, Kernite and Omber Deposit', 0),
(7221,'Medium Hemorphite, Jaspet and Kernite Deposit', 0),
(7222,'Medium Hedbergite, Hemorphite and Jaspet Deposit', 0),
(7223,'Medium Hedbergite and Hemorphite Deposit', 0),
(7224,'Medium Gneiss Deposit', 0),
(7321,'Medium Dark Ochre and Gneiss Deposit', 0),
(7322,'Medium Crokite, Dark Ochre and Gneiss Deposit', 0),
(7323,'Medium Spodumain, Crokite and Dark Ochre Deposit', 0),
(7521,'Medium Bistot Deposit', 0),
(7522,'Medium Arkanor and Bistot Deposit', 0),
(7523,'Medium Mercoxit, Arkonor and Bistot Deposit', 0),

(7131,'Large Omber Deposit', 0),
(7132,'Large Kernite and Omber Deposit', 0),
(7133,'Large Jaspet, Kernite and Omber Deposit', 0),
(7231,'Large Hemorphite, Jaspet and Kernite Deposit', 0),
(7232,'Large Hedbergite, Hemorphite and Jaspet Deposit', 0),
(7233,'Large Hedbergite and Hemorphite Deposit', 0),
(7234,'Large Gneiss Deposit', 0),
(7331,'Large Dark Ochre and Gneiss Deposit', 0),
(7332,'Large Crokite, Dark Ochre and Gneiss Deposit', 0),
(7333,'Large Spodumain, Crokite and Dark Ochre Deposit', 0),
(7531,'Large Bistot Deposit', 0),
(7532,'Large Arkanor and Bistot Deposit', 0),
(7533,'Large Mercoxit, Arkonor and Bistot Deposit', 0),

-- ladar sites (gas clouds)
(7010,'Small Nebula - No Rats', 0),
(7110,'Small Nebula - Small Rats', 0),
(7210,'Small Nebula - Smallish Rats', 0),
(7311,'Small Nebula - Medium Rats', 0),
(7511,'Small Nebula - Large Rats', 0),
(7120,'Medium Nebula - No Rats', 0),
(7220,'Medium Nebula - Smallish Rats', 0),
(7320,'Medium Nebula - Medium Rats', 0),
(7520,'Medium Nebula - Large Rats', 0),
(7130,'Large Nebula - Small Rats', 0),
(7230,'Large Nebula - Medium Rats', 0),
(7330,'Large Nebula - Large Rats', 0),
(7530,'Large Nebula - Huge Rats', 0),

-- these grav sites are for sov nullsec with Ore Prospecting Array
(7110,'Small Asteroid Cluster', 0),
(7220,'Moderate Asteroid Cluster', 0),
(7330,'Large Asteroid Cluster', 0),
(7540,'Enormous Asteroid Cluster', 0),
(7750,'Colossal Asteroid Cluster', 0),



/*!40000 ALTER TABLE `dunRoomInfo` ENABLE KEYS */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2017-08-01 18:36:25
