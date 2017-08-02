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
-- Table structure for table `staOperations`
--

DROP TABLE IF EXISTS `staOperations`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `staOperations` (
  `activityID` tinyint(3) unsigned DEFAULT NULL,
  `operationID` tinyint(3) unsigned NOT NULL,
  `operationName` varchar(100) DEFAULT NULL,
  `description` varchar(1000) DEFAULT NULL,
  `descriptionID` int(3) NOT NULL DEFAULT '0',
  `fringe` tinyint(4) DEFAULT NULL,
  `corridor` tinyint(4) DEFAULT NULL,
  `hub` tinyint(4) DEFAULT NULL,
  `border` tinyint(4) DEFAULT NULL,
  `ratio` tinyint(4) DEFAULT NULL,
  `caldariStationTypeID` int(11) DEFAULT NULL,
  `minmatarStationTypeID` int(11) DEFAULT NULL,
  `amarrStationTypeID` int(11) DEFAULT NULL,
  `gallenteStationTypeID` int(11) DEFAULT NULL,
  `joveStationTypeID` int(11) DEFAULT NULL,
  PRIMARY KEY (`operationID`),
  KEY `activityID` (`activityID`),
  KEY `caldariStationTypeID` (`caldariStationTypeID`),
  KEY `minmatarStationTypeID` (`minmatarStationTypeID`),
  KEY `amarrStationTypeID` (`amarrStationTypeID`),
  KEY `gallenteStationTypeID` (`gallenteStationTypeID`),
  KEY `joveStationTypeID` (`joveStationTypeID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `staOperations`
--

LOCK TABLES `staOperations` WRITE;
/*!40000 ALTER TABLE `staOperations` DISABLE KEYS */;
INSERT INTO `staOperations` VALUES (1,1,'Plantation','Makes livestock and grain that is shipped to food processing units.',0,70,20,10,0,65,1531,2499,1930,3866,3865),(1,2,'Food Packaging','Makes food that is shipped to warehouses.',0,0,20,50,30,20,4024,2497,1928,3866,3865),(1,3,'Warehouse','Stores products and shifts goods to external retailers or production corporations.',0,0,50,0,50,15,1531,2501,1929,3870,3865),(2,4,'Foundry','Mines minerals from asteroid belts/clouds.',0,90,10,0,0,30,1529,2499,1930,3867,3865),(2,5,'Production Plant','Molds finished construction pieces.',0,25,15,50,10,50,1529,2497,1928,3867,3865),(2,6,'Warehouse','Stores product and freights goods to production corps.',0,0,50,0,50,20,1531,2501,1929,3870,3865),(3,7,'Mining Outpost','Mines minerals from asteroid belts/clouds.',0,90,10,0,0,65,4023,2499,1930,3865,3865),(3,8,'Refinery','Refines the minerals, making purified ingots and rock materials.',0,0,10,45,45,15,4024,2497,1928,3867,3865),(3,9,'Mineral Reserve','Stores product and freights goods to production corps.',0,0,50,0,50,20,4023,2501,1929,3865,3865),(4,10,'Reprocessing Facility','Extracts chemicals from asteroid belts/clouds.',0,90,10,0,0,10,4023,2499,1930,3867,3865),(4,11,'Chemical Refinery','Refines the harvested chemicals, making bottled products.',0,0,10,90,0,80,1529,2497,1928,3867,3865),(4,12,'Chemical Storage','Stores products and shifts goods to production corporations.',0,0,50,0,50,10,1531,2501,1929,3870,3865),(5,13,'Testing Facilities','Makes plans and blueprints that are shipped to Assembly Plant.',0,90,10,0,0,20,54,2500,1931,3869,3865),(5,14,'Assembly Plant','Produces weapons and ammunition that are shipped for storage.',0,0,10,60,30,55,1529,2498,1927,3867,3865),(5,15,'Logistic Support','Stores product and freights goods to external retailers.',0,0,50,0,50,25,1530,2501,1929,3868,3865),(6,16,'Biotech Research Center','Harvests resources from a planet.',0,70,30,0,0,20,54,2499,1930,3869,3865),(6,17,'Biotech Production','Produces Biotech products.',0,0,10,45,45,70,1529,2497,1928,3867,3865),(6,18,'Biohazard Containment Facility','Stores and recycles materials classified as biohazards.',0,50,50,0,0,10,54,2501,1929,3869,3865),(7,19,'Research Center','Makes plans and blueprints that are shipped to Factory.',0,70,30,0,0,10,54,2500,1931,3869,3865),(7,20,'Factory','Produces finished hi-tech products.',0,0,10,50,40,80,1529,2497,1928,3867,3865),(7,21,'Warehouse','Stores product and freights goods to external retailers.',0,0,50,0,50,10,1531,2501,1929,3870,3865),(8,22,'Development Studio','Produces entertainment material that is shipped to Publisher.',0,0,10,45,45,80,1531,2496,1931,56,3865),(8,23,'Publisher','Stores and multiplies the entertainment material and ships it to retailers.',0,0,10,80,10,20,3872,2496,1926,57,3865),(9,24,'Shipyard','Produces starships that are shipped for storage in Storage Bay.',0,0,10,90,0,80,1530,2498,1927,3868,3865),(9,25,'Storage Bay','Stores ships before freighting them to retailers.',0,0,50,0,50,20,3872,2502,1929,3870,3865),(10,26,'Storage','Stores cargo for other corps before shipping it to retailers.',0,25,25,25,25,100,1531,2501,1929,3870,3865),(11,27,'Warehouse','Stores products and freights goods to retail center.',0,40,25,10,25,70,1531,2501,1929,3870,3865),(11,28,'Retail Center','Sells goods from warehouses to consumers, and to service stations.',0,0,10,45,45,30,1531,2496,1926,57,3865),(12,29,'Warehouse','Stores products and freights goods to Trading Post.',0,50,25,0,25,50,1531,2501,1929,3870,3865),(12,30,'Trading Post','Sells wares received from production corps.',0,0,15,35,50,50,1531,2502,1932,3868,3865),(13,31,'Bureau Offices','Delivers reports and provides information concerning the Bureau\'s field.',0,0,10,45,45,40,1529,2496,1926,56,3865),(14,32,'Academy','Studies issues within the administration system and produces personnel.',0,50,50,0,0,20,1529,2502,9857,57,3865),(14,33,'Bureau','Delivers reports and publishes rules and manifestos concerning the Bureau\'s field.',0,0,15,40,45,70,1531,2502,1932,56,3865),(14,34,'Treasury','Stores data and kredit sheets for the Bureau to work with.',0,100,0,0,0,10,1530,2498,1927,3868,3865),(17,35,'Depository','Works as a bank.',0,0,10,85,5,70,1529,2502,1932,57,3865),(17,36,'Vault','Stores data and kredit sheets for the Bank to work with.',0,100,0,0,0,30,1530,2501,1927,3868,3865),(18,37,'School','Produces personnel and sells skill packages.',0,0,0,50,50,100,1529,2500,1931,57,3865),(19,38,'Factory',NULL,0,0,10,45,45,80,4024,2497,1928,3867,3865),(19,39,'Warehouse',NULL,0,0,50,0,50,20,1531,2501,1929,3870,3865),(15,40,'Tribunal','Courts where judicial matters are judged',0,0,0,50,50,40,1530,2496,9857,3868,3865),(15,41,'Accounting','Accounting and audit firms',0,30,30,20,20,30,1529,2500,1931,57,3865),(15,42,'Law School',NULL,0,50,30,10,10,30,3871,2502,1929,56,3865),(13,43,'Archives','Stores administrative documents and data.',0,70,30,0,0,30,1529,2500,1931,57,3865),(13,44,'Information Center','Distributes administrative information to the public.',0,0,0,50,50,30,3872,2502,1926,57,3865),(20,45,'Manufacturing',NULL,0,0,0,0,0,0,NULL,NULL,NULL,NULL,NULL),(20,46,'Refining',NULL,0,0,0,0,0,0,NULL,NULL,NULL,NULL,NULL),(20,47,'Cloning',NULL,0,0,0,0,0,0,NULL,NULL,NULL,NULL,NULL),(20,48,'Caldari Research Outpost',NULL,0,0,0,0,0,0,NULL,NULL,NULL,NULL,NULL),(20,49,'Amarr Factory Outpost',NULL,0,0,0,0,0,0,NULL,NULL,NULL,NULL,NULL),(20,50,'Gallente Administrative Outpost',NULL,0,0,0,0,0,0,NULL,NULL,NULL,NULL,NULL),(20,51,'Minmatar Service Outpost',NULL,0,0,0,0,0,0,NULL,NULL,NULL,NULL,NULL);
/*!40000 ALTER TABLE `staOperations` ENABLE KEYS */;
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
