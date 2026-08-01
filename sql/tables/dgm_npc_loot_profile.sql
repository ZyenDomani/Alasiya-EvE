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
-- Table structure for table `dgm_npc_loot_profile`
--

DROP TABLE IF EXISTS `dgm_npc_loot_profile`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dgm_npc_loot_profile` (
  `profile_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `profile_name` varchar(64) NOT NULL,
  `ship_class` int(11) NOT NULL DEFAULT '0',
  `source_group_id` int(10) unsigned NOT NULL,
  `is_advanced` tinyint(1) DEFAULT '0',
  PRIMARY KEY (`profile_id`),
  KEY `idx_lookup` (`source_group_id`,`is_advanced`)
) ENGINE=Aria AUTO_INCREMENT=126 DEFAULT CHARSET=utf8mb4 PAGE_CHECKSUM=1 ROW_FORMAT=PAGE TRANSACTIONAL=1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `dgm_npc_loot_profile`
--

LOCK TABLES `dgm_npc_loot_profile` WRITE;
/*!40000 ALTER TABLE `dgm_npc_loot_profile` DISABLE KEYS */;
INSERT INTO `dgm_npc_loot_profile` VALUES (1,'Asteroid Angel Cartel Frigate',1,550,0),(2,'Asteroid Angel Cartel Advanced Frigate',2,550,1),(3,'Asteroid Angel Cartel Destroyer',3,575,0),(4,'Asteroid Angel Cartel Cruiser',4,551,0),(5,'Asteroid Angel Cartel Advanced Cruiser',5,551,1),(6,'Asteroid Angel Cartel BattleCruiser',6,576,0),(7,'Asteroid Angel Cartel Battleship',7,552,0),(8,'Asteroid Angel Cartel Hauler',8,554,0),(9,'Asteroid Angel Cartel Officer',9,553,0),(10,'Asteroid Angel Cartel Commander Battleship',14,851,1),(11,'Asteroid Angel Cartel Commander Cruiser',12,850,1),(12,'Asteroid Angel Cartel Commander Frigate',10,848,1),(13,'Deadspace Rogue Drone Swarm',22,806,0),(14,'Deadspace Rogue Drone Battleship',21,802,0),(15,'Deadspace Angel Cartel Frigate',15,597,0),(16,'Deadspace Angel Cartel Advanced Frigate',16,597,1),(17,'Deadspace Angel Cartel Destroyer',17,596,0),(18,'Deadspace Angel Cartel Cruiser',18,595,0),(19,'Deadspace Angel Cartel Advanced Cruiser',19,595,1),(20,'Deadspace Angel Cartel BattleCruiser',20,594,0),(21,'Deadspace Angel Cartel Battleship',21,593,0),(22,'Asteroid Blood Raiders Frigate',1,557,0),(23,'Asteroid Blood Raiders Advanced Frigate',2,557,1),(24,'Asteroid Blood Raiders Destroyer',3,577,0),(25,'Asteroid Blood Raiders Cruiser',4,555,0),(26,'Asteroid Blood Raiders Advanced Cruiser',5,555,1),(27,'Asteroid Blood Raiders BattleCruiser',6,578,0),(28,'Asteroid Blood Raiders Battleship',7,556,0),(29,'Asteroid Blood Raiders Hauler',8,558,0),(30,'Asteroid Blood Raiders Officer',9,559,0),(31,'Deadspace Rogue Drone BattleCruiser',20,801,0),(32,'Deadspace Rogue Drone Advanced Cruiser',19,803,1),(33,'Deadspace Rogue Drone Cruiser',18,803,0),(34,'Deadspace Rogue Drone Destroyer',17,804,0),(35,'Deadspace Rogue Drone Advanced Frigate',16,805,1),(36,'Deadspace Blood Raiders Frigate',15,606,0),(37,'Deadspace Blood Raiders Advanced Frigate',16,606,1),(38,'Deadspace Blood Raiders Destroyer',17,605,0),(39,'Deadspace Blood Raiders Cruiser',18,604,0),(40,'Deadspace Blood Raiders Advanced Cruiser',19,604,1),(41,'Deadspace Blood Raiders BattleCruiser',20,602,0),(42,'Deadspace Blood Raiders Battleship',21,603,0),(43,'Asteroid Guristas Frigate',1,562,0),(44,'Asteroid Guristas Advanced Frigate',2,562,1),(45,'Asteroid Guristas Destroyer',3,579,0),(46,'Asteroid Guristas Cruiser',4,561,0),(47,'Asteroid Guristas Advanced Cruiser',5,561,1),(48,'Asteroid Guristas BattleCruiser',6,580,0),(49,'Asteroid Guristas Battleship',7,560,0),(50,'Asteroid Guristas Hauler',8,563,0),(51,'Asteroid Guristas Officer',9,564,0),(52,'Deadspace Rogue Drone Frigate',15,805,0),(53,'Asteroid Rogue Drone Commander Battleship',14,844,1),(54,'Asteroid Rogue Drone Commander BattleCruiser',13,843,1),(55,'Asteroid Rogue Drone Commander Cruiser',12,845,1),(56,'Asteroid Rogue Drone Commander Destroyer',11,846,1),(57,'Deadspace Guristas Frigate',15,615,0),(58,'Deadspace Guristas Advanced Frigate',16,615,1),(59,'Deadspace Guristas Destroyer',17,614,0),(60,'Deadspace Guristas Cruiser',18,613,0),(61,'Deadspace Guristas Advanced Cruiser',19,613,1),(62,'Deadspace Guristas BattleCruiser',20,611,0),(63,'Deadspace Guristas Battleship',21,612,0),(64,'Asteroid Sansha\'s Nation Frigate',1,567,0),(65,'Asteroid Sansha\'s Nation Advanced Frigate',2,567,1),(66,'Asteroid Sansha\'s Nation Destroyer',3,581,0),(67,'Asteroid Sansha\'s Nation Cruiser',4,566,0),(68,'Asteroid Sansha\'s Nation Advanced Cruiser',5,566,1),(69,'Asteroid Sansha\'s Nation BattleCruiser',6,582,0),(70,'Asteroid Sansha\'s Nation Battleship',7,565,0),(71,'Asteroid Sansha\'s Nation Hauler',8,568,0),(72,'Asteroid Sansha\'s Nation Officer',9,569,0),(73,'Asteroid Rogue Drone Commander Frigate',10,847,1),(74,'Asteroid Rogue Drone Swarm',9,761,0),(75,'Asteroid Rogue Drone Hauler',8,760,0),(76,'Asteroid Rogue Drone Battleship',7,756,0),(77,'Asteroid Rogue Drone BattleCruiser',6,755,0),(78,'Deadspace Sansha\'s Nation Frigate',15,624,0),(79,'Deadspace Sansha\'s Nation Advanced Frigate',16,624,1),(80,'Deadspace Sansha\'s Nation Destroyer',17,623,0),(81,'Deadspace Sansha\'s Nation Cruiser',18,622,0),(82,'Deadspace Sansha\'s Nation Advanced Cruiser',19,622,1),(83,'Deadspace Sansha\'s Nation BattleCruiser',20,620,0),(84,'Deadspace Sansha\'s Nation Battleship',21,621,0),(85,'Asteroid Serpentis Frigate',1,572,0),(86,'Asteroid Serpentis Advanced Frigate',2,572,1),(87,'Asteroid Serpentis Destroyer',3,583,0),(88,'Asteroid Serpentis Cruiser',4,571,0),(89,'Asteroid Serpentis Advanced Cruiser',5,571,1),(90,'Asteroid Serpentis BattleCruiser',6,584,0),(91,'Asteroid Serpentis Battleship',7,570,0),(92,'Asteroid Serpentis Hauler',8,573,0),(93,'Asteroid Serpentis Officer',9,574,0),(94,'Asteroid Rogue Drone Advanced Cruiser',5,757,1),(95,'Asteroid Rogue Drone Cruiser',4,757,0),(96,'Asteroid Rogue Drone Destroyer',3,758,0),(97,'Asteroid Rogue Drone Advanced Frigate',2,759,1),(98,'Asteroid Rogue Drone Frigate',1,759,0),(99,'Deadspace Serpentis Frigate',15,633,0),(100,'Deadspace Serpentis Advanced Frigate',16,633,1),(101,'Deadspace Serpentis Destroyer',17,632,0),(102,'Deadspace Serpentis Cruiser',18,631,0),(103,'Deadspace Serpentis Advanced Cruiser',19,631,1),(104,'Deadspace Serpentis BattleCruiser',20,629,0),(105,'Deadspace Serpentis Battleship',21,630,0),(106,'Asteroid Guristas Commander Frigate',10,853,1),(107,'Asteroid Guristas Commander Cruiser',12,855,1),(108,'Asteroid Guristas Commander Battleship',14,856,1),(109,'Asteroid Blood Raiders Commander Frigate',10,858,1),(110,'Asteroid Blood Raiders Commander Cruiser',12,860,1),(111,'Asteroid Blood Raiders Commander Battleship',14,861,1),(112,'Asteroid Sanshas Nation Commander Frigate',10,863,1),(113,'Asteroid Sanshas Nation Commander Cruiser',12,865,1),(114,'Asteroid Sanshas Nation Commander Battleship',14,866,1),(115,'Asteroid Serpentis Commander Frigate',10,868,1),(116,'Asteroid Serpentis Commander Cruiser',12,870,1),(117,'Asteroid Serpentis Commander Battleship',14,871,1),(118,'Capital Angel Cartel Dreadnought',30,553,1),(119,'Capital Guristas Dreadnought',30,554,1),(120,'Capital Blood Raiders Dreadnought',30,551,1),(121,'Capital Sanshas Nation Dreadnought',30,552,1),(122,'Capital Serpentis Dreadnought',30,550,1),(123,'Sleeper Wormhole Frigate',40,959,0),(124,'Sleeper Wormhole Cruiser',41,960,0),(125,'Sleeper Wormhole Battleship',42,961,0);
/*!40000 ALTER TABLE `dgm_npc_loot_profile` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2026-08-01  9:55:17
