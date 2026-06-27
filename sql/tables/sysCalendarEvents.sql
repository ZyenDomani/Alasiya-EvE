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
-- Table structure for table `sysCalendarEvents`
--

DROP TABLE IF EXISTS `sysCalendarEvents`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sysCalendarEvents` (
  `eventID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `ownerID` int(10) NOT NULL DEFAULT '0',
  `creatorID` int(10) NOT NULL DEFAULT '0',
  `month` tinyint(2) NOT NULL DEFAULT '0',
  `year` smallint(4) NOT NULL DEFAULT '0',
  `eventDateTime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `eventDuration` smallint(4) unsigned DEFAULT NULL,
  `dateModified` bigint(20) DEFAULT NULL,
  `importance` tinyint(1) NOT NULL DEFAULT '0',
  `isDeleted` tinyint(1) NOT NULL DEFAULT '0',
  `flag` tinyint(2) unsigned NOT NULL DEFAULT '1',
  `autoEventType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `eventTitle` varchar(80) NOT NULL,
  `eventText` varchar(500) NOT NULL,
  UNIQUE KEY `eventID` (`eventID`)
) ENGINE=InnoDB AUTO_INCREMENT=20 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sysCalendarEvents`
--

LOCK TABLES `sysCalendarEvents` WRITE;
/*!40000 ALTER TABLE `sysCalendarEvents` DISABLE KEYS */;
INSERT INTO `sysCalendarEvents` VALUES (1,98000001,90000000,11,2020,132487463944203632,120,NULL,1,0,2,0,'event test','this is a code and db test event '),(2,1,1000160,11,2020,132487404000000000,240,NULL,1,0,8,0,'yet another test','testing system event here.  db edit'),(3,90000000,90000000,11,2020,132494832000000000,60,NULL,0,0,1,0,'personal test','personal event test'),(4,98000001,1000160,11,2020,132493881115543104,NULL,NULL,1,0,16,2,'Manufacturing in Zemalu IX - Moon 2 - Thukker Mix Factory.','Completion of Manufacturing job in Zemalu IX - Moon 2 - Thukker Mix Factory.'),(5,90000000,1000160,11,2020,132491781743622832,NULL,NULL,0,0,16,2,'Manufacturing in Zemalu IX - Moon 2 - Thukker Mix Factory.','Completion of Manufacturing job in Zemalu IX - Moon 2 - Thukker Mix Factory.'),(6,90000000,1000160,11,2020,132491824394099216,NULL,NULL,0,0,16,2,'Manufacturing in Zemalu IX - Moon 2 - Thukker Mix Factory.','Completion of Manufacturing job in Zemalu IX - Moon 2 - Thukker Mix Factory.'),(7,98000001,1000160,11,2020,132493605235093008,NULL,NULL,1,0,16,2,'Research in Zemalu IX - Moon 2 - Thukker Mix Factory.','Completion of Research job in Zemalu IX - Moon 2 - Thukker Mix Factory.'),(8,90000000,1000160,11,2020,132491873795865008,NULL,NULL,0,0,16,2,'Manufacturing in Zemalu IX - Moon 2 - Thukker Mix Factory.','Completion of Manufacturing job in Zemalu IX - Moon 2 - Thukker Mix Factory.'),(9,90000000,1000160,11,2020,132491873795865008,NULL,NULL,0,0,16,2,'Copying Job','Completion of Copying job in Zemalu IX - Moon 2 - Thukker Mix Factory.<br><br>This job is for 10 runs of Titanium Sabot S'),(10,90000000,1000160,11,2020,132497871943428960,NULL,NULL,0,0,16,2,'Copying Job','Your <color=white>Personal</color> Copying job in Zemalu IX - Moon 2 - Thukker Mix Factory is scheduled to complete at the time noted.<br>\nThis job is for <color=yellow>5</color> runs of <color=green>Nuclear S</color> <color=cyan>Projectile Ammo</color><br>\n<br><br>And a good time shall be had by all'),(11,90000000,1000160,11,2020,132497871953428960,NULL,NULL,0,0,16,2,'Copying Job 2','Your <color=white>Personal</color> Manufacturing job in Zemalu IX - Moon 2 - Thukker Mix Factory is scheduled to complete at the time noted.<br>\nThis job is for <color=red>5</color> runs of <color=yellow>Projectile Ammo</color>,<br> producing <color=green>500</color> units of <color=green>Nuclear S</color> \n<br><br>And a good time shall be had by all!'),(12,90000000,1000160,11,2020,132498806913411328,NULL,NULL,0,0,16,2,'Copying Job','Your <color=white>Personal</color>Copying job in Zemalu IX - Moon 2 - Thukker Mix Factory is scheduled to complete at the time noted.<br><br><color=green>3</color> Copies of the <color=yellow>125mm Gatling AutoCannon I Blueprint</color> BPO will be made.'),(13,90000000,1000160,11,2020,132500381321387904,NULL,NULL,0,0,16,2,'Research ME Job','Your <color=white>Personal</color>Research ME job in Zemalu IX - Moon 2 - Thukker Mix Factory is scheduled to complete at the time noted.<br><br>Upon completion, this job will increase the Material Efficiency of the <color=yellow>Carbonized Lead S Blueprint</color> BPO from <color=red>0 </color> to <color=green>3</color>.'),(14,90000000,0,11,2020,132500336781375504,NULL,NULL,0,0,16,2,'Personal Research PE Job','Your Research PE job in Unknown Location is scheduled to complete at the time noted.<br><br>Upon completion, this job will increase the Production Efficiency of the <color=yellow>Fusion S Blueprint</color> from <color=red>0</color> to <color=green>3</color>.<br><br><br>And a good time shall be had by all!'),(15,90000000,0,11,2020,132498780273633328,NULL,NULL,0,0,16,2,'Personal Manufacturing Job','Your Manufacturing job in Unknown Location is scheduled to complete at the time noted.<br><br>This job is for <color=red>10</color> runs of <color=yellow>Projectile Ammo</color>,<br>producing <color=green>1000</color> units of <color=green>Nuclear S</color>.'),(16,90000000,1000160,11,2020,132498880841965440,NULL,NULL,0,0,16,2,'Personal Manufacturing Job','Your Manufacturing job in Zemalu IX - Moon 2 - Thukker Mix Factory is scheduled to complete at the time noted.<br><br>This job is for <color=red>10</color> runs of <color=yellow>Projectile Ammo</color>,<br>producing <color=green>1000</color> units of <color=green>Nuclear S</color>.<br><br><br>And a good time shall be had by all!'),(17,90000000,1000160,11,2020,132499674510723088,NULL,NULL,0,0,16,2,'Personal Copying Job','Your Copying job in Zemalu IX - Moon 2 - Thukker Mix Factory is scheduled to complete at the time noted.<br><br><color=green>1</color> Copies of the <color=yellow>Depleted Uranium S Blueprint</color> will be made.<br><br><br>And a good time shall be had by all!'),(18,90000000,1000160,11,2020,132499777882437312,NULL,NULL,0,0,16,2,'Personal Manufacturing Job','Your Manufacturing job in Zemalu IX - Moon 2 - Thukker Mix Factory is scheduled to complete at the time noted.<br><br>This job is for <color=red>10</color> runs of <color=yellow>Projectile Weapon</color>,<br>producing <color=green>10</color> units of <color=green>125mm Gatling AutoCannon I</color>.<br><br><br>And a good time shall be had by all!'),(19,90000000,1000160,2,2025,133833533528824480,NULL,NULL,0,0,16,2,'Personal Research ME Job','Your Research ME job in Zemalu IX - Moon 2 - Thukker Mix Factory is scheduled to complete at the time noted.<br><br>Upon completion, this job will increase the Material Efficiency of the <color=yellow>Depleted Uranium S Blueprint</color> from <color=red>0</color> to <color=green>10</color>.');
/*!40000 ALTER TABLE `sysCalendarEvents` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2026-06-27  0:42:49
