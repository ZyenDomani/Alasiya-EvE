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
-- Table structure for table `roidRatClassGroup`
--

DROP TABLE IF EXISTS `roidRatClassGroup`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `roidRatClassGroup` (
  `shipClass` int(11) NOT NULL,
  `groupID` int(11) NOT NULL,
  `factionID` int(11) NOT NULL,
  `groupName` varchar(50) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`groupID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `roidRatClassGroup`
--

LOCK TABLES `roidRatClassGroup` WRITE;
/*!40000 ALTER TABLE `roidRatClassGroup` DISABLE KEYS */;
INSERT INTO `roidRatClassGroup` VALUES (1,550,500011,'Asteroid Angel Cartel Frigate'),(2,575,500011,'Asteroid Angel Cartel Destroyer'),(3,551,500011,'Asteroid Angel Cartel Cruiser'),(4,576,500011,'Asteroid Angel Cartel BattleCruiser'),(5,552,500011,'Asteroid Angel Cartel Battleship'),(6,554,500011,'Asteroid Angel Cartel Hauler'),(7,553,500011,'Asteroid Angel Cartel Officer'),(8,789,500011,'Asteroid Angel Cartel Commander Frigate'),(9,794,500011,'Asteroid Angel Cartel Commander Destroyer'),(10,790,500011,'Asteroid Angel Cartel Commander Cruiser'),(11,793,500011,'Asteroid Angel Cartel Commander BattleCruiser'),(12,848,500011,'Asteroid Angel Cartel Commander Battleship'),(1,557,500012,'Asteroid Blood Raiders Frigate'),(2,577,500012,'Asteroid Blood Raiders Destroyer'),(3,555,500012,'Asteroid Blood Raiders Cruiser'),(4,578,500012,'Asteroid Blood Raiders BattleCruiser'),(5,556,500012,'Asteroid Blood Raiders Battleship'),(6,558,500012,'Asteroid Blood Raiders Hauler'),(7,559,500012,'Asteroid Blood Raiders Officer'),(8,792,500012,'Asteroid Blood Raiders Commander Frigate'),(9,796,500012,'Asteroid Blood Raiders Commander Destroyer'),(10,791,500012,'Asteroid Blood Raiders Commander Cruiser'),(11,795,500012,'Asteroid Blood Raiders Commander BattleCruiser'),(12,849,500012,'Asteroid Blood Raiders Commander Battleship'),(1,562,500010,'Asteroid Guristas Frigate'),(2,579,500010,'Asteroid Guristas Destroyer'),(3,561,500010,'Asteroid Guristas Cruiser'),(4,580,500010,'Asteroid Guristas BattleCruiser'),(5,560,500010,'Asteroid Guristas Battleship'),(6,563,500010,'Asteroid Guristas Hauler'),(7,564,500010,'Asteroid Guristas Officer'),(8,800,500010,'Asteroid Guristas Commander Frigate'),(9,799,500010,'Asteroid Guristas Commander Destroyer'),(10,798,500010,'Asteroid Guristas Commander Cruiser'),(11,797,500010,'Asteroid Guristas Commander BattleCruiser'),(12,850,500010,'Asteroid Guristas Commander Battleship'),(1,567,500019,'Asteroid Sansha\'s Nation Frigate'),(2,581,500019,'Asteroid Sansha\'s Nation Destroyer'),(3,566,500019,'Asteroid Sansha\'s Nation Cruiser'),(4,582,500019,'Asteroid Sansha\'s Nation BattleCruiser'),(5,565,500019,'Asteroid Sansha\'s Nation Battleship'),(6,568,500019,'Asteroid Sansha\'s Nation Hauler'),(7,569,500019,'Asteroid Sansha\'s Nation Officer'),(8,810,500019,'Asteroid Sansha\'s Nation Commander Frigate'),(9,809,500019,'Asteroid Sansha\'s Nation Commander Destroyer'),(10,808,500019,'Asteroid Sansha\'s Nation Commander Cruiser'),(11,807,500019,'Asteroid Sansha\'s Nation Commander BattleCruiser'),(12,851,500019,'Asteroid Sansha\'s Nation Commander Battleship'),(1,572,500020,'Asteroid Serpentis Frigate'),(2,583,500020,'Asteroid Serpentis Destroyer'),(3,571,500020,'Asteroid Serpentis Cruiser'),(4,584,500020,'Asteroid Serpentis BattleCruiser'),(5,570,500020,'Asteroid Serpentis Battleship'),(6,573,500020,'Asteroid Serpentis Hauler'),(7,574,500020,'Asteroid Serpentis Officer'),(8,814,500020,'Asteroid Serpentis Commander Frigate'),(9,813,500020,'Asteroid Serpentis Commander Destroyer'),(10,812,500020,'Asteroid Serpentis Commander Cruiser'),(11,811,500020,'Asteroid Serpentis Commander BattleCruiser'),(12,852,500020,'Asteroid Serpentis Commander Battleship'),(1,759,500022,'Asteroid Rogue Drone Frigate'),(2,758,500022,'Asteroid Rogue Drone Destroyer'),(3,757,500022,'Asteroid Rogue Drone Cruiser'),(4,755,500022,'Asteroid Rogue Drone BattleCruiser'),(5,756,500022,'Asteroid Rogue Drone Battleship'),(6,760,500022,'Asteroid Rogue Drone Hauler'),(7,761,500022,'Asteroid Rogue Drone Swarm'),(8,847,500022,'Asteroid Rogue Drone Commander Frigate'),(9,846,500022,'Asteroid Rogue Drone Commander Destroyer'),(10,845,500022,'Asteroid Rogue Drone Commander Cruiser'),(11,843,500022,'Asteroid Rogue Drone Commander BattleCruiser'),(12,844,500022,'Asteroid Rogue Drone Commander Battleship');
/*!40000 ALTER TABLE `roidRatClassGroup` ENABLE KEYS */;
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
