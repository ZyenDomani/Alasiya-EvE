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
-- Table structure for table `dunGroupInfo`
--

DROP TABLE IF EXISTS `dunGroupInfo`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dunGroupInfo` (
  `dunGroupID` int(11) NOT NULL,
  `dunGroupName` varchar(85) COLLATE utf8_bin NOT NULL,
  KEY `dunGroupID` (`dunGroupID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `dunGroupInfo`
--

LOCK TABLES `dunGroupInfo` WRITE;
/*!40000 ALTER TABLE `dunGroupInfo` DISABLE KEYS */;
INSERT INTO `dunGroupInfo` VALUES (1,'CorpseTEST'),(2,'SmallAsteroidCluster'),(3,'SmallOmberDeposit'),(4,'SmallKerniteandOmberDeposit'),(5,'SmallJaspet,KerniteandOmberDeposit'),(6,'SmallHemorphite,JaspetandKerniteDeposit'),(7,'SmallHedbergite,HemorphiteandJaspetDeposit'),(8,'SmallHedbergiteandHemorphiteDeposit'),(9,'SmallGneissDeposit'),(10,'SmallDarkOchreandGneissDeposit'),(11,'SmallCrokite,DarkOchreandGneissDeposit'),(12,'SmallSpodumain,CrokiteandDarkOchreDeposit'),(13,'SmallBistotDeposit'),(14,'SmallArkanorandBistotDeposit'),(15,'SmallMercoxit,ArkonorandBistotDeposit'),(16,'ModerateAsteroidCluster'),(17,'AverageOmberDeposit'),(18,'AverageKerniteandOmberDeposit'),(19,'AverageJaspet,KerniteandOmberDeposit'),(20,'AverageHemorphite,JaspetandKerniteDeposit'),(21,'AverageHedbergite,HemorphiteandJaspetDeposit'),(22,'AverageHedbergiteandHemorphiteDeposit'),(23,'AverageGneissDeposit'),(24,'AverageDarkOchreandGneissDeposit'),(25,'AverageCrokite,DarkOchreandGneissDeposit'),(26,'AverageSpodumain,CrokiteandDarkOchreDeposit'),(27,'AverageBistotDeposit'),(28,'AverageArkanorandBistotDeposit'),(29,'AverageMercoxit,ArkonorandBistotDeposit'),(30,'LargeAsteroidCluster'),(31,'LargeOmberDeposit'),(32,'LargeKerniteandOmberDeposit'),(33,'LargeJaspet,KerniteandOmberDeposit'),(34,'LargeHemorphite,JaspetandKerniteDeposit'),(35,'LargeHedbergite,HemorphiteandJaspetDeposit'),(36,'LargeHedbergiteandHemorphiteDeposit'),(37,'LargeGneissDeposit'),(38,'LargeDarkOchreandGneissDeposit'),(39,'LargeCrokite,DarkOchreandGneissDeposit'),(40,'LargeSpodumain,CrokiteandDarkOchreDeposit'),(41,'LargeBistotDeposit'),(42,'LargeArkanorandBistotDeposit'),(43,'LargeMercoxit,ArkonorandBistotDeposit');
/*!40000 ALTER TABLE `dunGroupInfo` ENABLE KEYS */;
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
