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
-- Table structure for table `chrPortraitData`
--

DROP TABLE IF EXISTS `chrPortraitData`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `chrPortraitData` (
  `charID` int(10) NOT NULL DEFAULT '0',
  `backgroundID` int(10) NOT NULL DEFAULT '0',
  `lightID` int(10) NOT NULL DEFAULT '0',
  `lightColorID` int(10) NOT NULL DEFAULT '0',
  `cameraX` float NOT NULL DEFAULT '0',
  `cameraY` float NOT NULL DEFAULT '0',
  `cameraZ` float NOT NULL DEFAULT '0',
  `cameraPoiX` float NOT NULL DEFAULT '0',
  `cameraPoiY` float NOT NULL DEFAULT '0',
  `cameraPoiZ` float NOT NULL DEFAULT '0',
  `headLookTargetX` float NOT NULL DEFAULT '0',
  `headLookTargetY` float NOT NULL DEFAULT '0',
  `headLookTargetZ` float NOT NULL DEFAULT '0',
  `lightIntensity` float NOT NULL DEFAULT '0',
  `headTilt` float NOT NULL DEFAULT '0',
  `orientChar` float NOT NULL DEFAULT '0',
  `browLeftCurl` float NOT NULL DEFAULT '0',
  `browLeftTighten` float NOT NULL DEFAULT '0',
  `browLeftUpDown` float NOT NULL DEFAULT '0',
  `browRightCurl` float NOT NULL DEFAULT '0',
  `browRightTighten` float NOT NULL DEFAULT '0',
  `browRightUpDown` float NOT NULL DEFAULT '0',
  `eyeClose` float NOT NULL DEFAULT '0',
  `eyesLookVertical` float NOT NULL DEFAULT '0',
  `eyesLookHorizontal` float NOT NULL DEFAULT '0',
  `squintLeft` float NOT NULL DEFAULT '0',
  `squintRight` float NOT NULL DEFAULT '0',
  `jawSideways` float NOT NULL DEFAULT '0',
  `jawUp` float NOT NULL DEFAULT '0',
  `puckerLips` float NOT NULL DEFAULT '0',
  `frownLeft` float NOT NULL DEFAULT '0',
  `frownRight` float NOT NULL DEFAULT '0',
  `smileLeft` float NOT NULL DEFAULT '0',
  `smileRight` float NOT NULL DEFAULT '0',
  `cameraFieldOfView` float NOT NULL DEFAULT '0',
  `portraitPoseNumber` float NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='Portrait Data for Characters';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `chrPortraitData`
--

LOCK TABLES `chrPortraitData` WRITE;
/*!40000 ALTER TABLE `chrPortraitData` DISABLE KEYS */;
INSERT INTO `chrPortraitData` VALUES (90000000,61,10866,10866,0.031108,1.72759,1.56851,0.031108,1.72759,0.06851,0,1.5,1,0.5,0,0.5,0.5,0,0.5,0.5,0,0.5,0,0.5,0.5,0,0,0.5,0.5,0,0,0,0,0,0.3,2),(90000001,24,10866,10866,0.039981,1.72748,1.56591,0.039981,1.72748,0.065912,0,1.5,1,0.5,0,0.5,0.5,0,0.5,0.5,0,0.5,0,0.5,0.5,0,0,0.5,0.5,0,0,0,0,0,0.3,2),(90000002,33,10866,10866,0.62009,1.79331,1.44909,0.008935,1.54693,0.101575,0.1215,1.5,0.5,0.5,0.27,0.5,0.5,0,0.5,0.5,0,0.5,0,0.425,0.5585,0,0,0.5,0.5,0,0,0,0,0,0.3,1),(90000003,58,10743,10752,0.278806,1.70833,1.56117,-0.025019,1.55858,0.09992,-0.0405,1.497,0.5,0.743363,-0.105,0.5,0.5,0,0.5,0.5,0,0.5,0,0.4325,0.671,0,0,0.5,0.5,0,0,0,0,0,0.3,2),(90000004,19,10866,10866,-0.030979,1.64192,1.20369,-0.030979,1.64192,0.103692,-0.1035,1.493,0.5,0.5,0,0.5,0.5,0,0.5,0.5,0,0.5,0,0.5,0.5,0,0,0.5,0.5,0,0,0,0,0,0.3,5),(90000005,60,10866,10866,0.030494,1.72419,1.63741,0.030494,1.72419,0.137409,0,1.5,1,0.5,0,0.5,0.5,0,0.5,0.5,0,0.5,0,0.5,0.5,0,0,0.5,0.5,0,0,0,0,0,0.3,5),(90000006,45,10866,10866,0.020877,1.7377,1.6197,0.020877,1.7377,0.119699,0,1.5,1,0.5,0,0.5,0.5,0,0.5,0.5,0,0.5,0,0.5,0.5,0,0,0.5,0.5,0,0,0,0,0,0.3,0),(90000008,67,10866,10866,0.020906,1.73769,1.61968,0.020906,1.73769,0.119677,0,1.5,1,0.5,0,0.5,0.5,0,0.5,0.5,0,0.5,0,0.5,0.5,0,0,0.5,0.5,0,0,0,0,0,0.3,0);
/*!40000 ALTER TABLE `chrPortraitData` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2026-08-01  9:55:14
