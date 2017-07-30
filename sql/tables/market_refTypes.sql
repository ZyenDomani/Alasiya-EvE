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
-- Table structure for table `market_refTypes`
--

DROP TABLE IF EXISTS `market_refTypes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `market_refTypes` (
  `entryTypeID` int(10) unsigned NOT NULL DEFAULT '0',
  `entryTypeName` varchar(100) NOT NULL DEFAULT '',
  `description` mediumtext NOT NULL,
  PRIMARY KEY (`entryTypeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `market_refTypes`
--

LOCK TABLES `market_refTypes` WRITE;
/*!40000 ALTER TABLE `market_refTypes` DISABLE KEYS */;
INSERT INTO `market_refTypes` VALUES (0,'Undefined',''),(1,'Player Trading',''),(2,'Market Transaction',''),(3,'GM Cash Transfer',''),(4,'ATM Withdraw',''),(5,'ATM Deposit',''),(6,'Backward Compatible',''),(7,'Mission Reward',''),(8,'Clone Activation',''),(9,'Inheritance',''),(10,'Player Donation','Player gave cash to another owner'),(11,'Corporation Payment','CEO or Accountant transferred cash  from corp. account'),(12,'Docking Fee',''),(13,'Office Rental Fee',''),(14,'Factory Slot Rental Fee',''),(15,'Repair Bill',''),(16,'Bounty','Player gave cash to someone\'s  bounty pool'),(17,'Bounty Prize','Player got bounty prize for killing someone'),(18,'Agents_temporary','TEMP'),(19,'Insurance',''),(20,'Mission Expiration',''),(21,'Mission Completion',''),(22,'Shares',''),(23,'Courier Mission Escrow',''),(24,'Mission Cost',''),(25,'Agent Miscellaneous','Agent paid you'),(26,'Miscellaneous Payment To Agent','You paid agent'),(27,'Agent Location Services','You paid agent to locate somebody'),(28,'Agent Donation','You donated/bribed the agent'),(29,'Agent Security Services','You paid agent to clean your rep'),(30,'Agent Mission Collateral Paid','You gave agent collateral for a mission'),(31,'Agent Mission Collateral Refunded','The agent returned collateral to you'),(32,'Agents_preward','The agent gave you this when you accepted the mission'),(33,'Agent Mission Reward','The agent gave you this as a reward'),(34,'Agent Mission Time Bonus Reward','The agent gave you this as a special reward for fast mission completion'),(35,'CSPA','CONCORD Spam Prevention Act'),(36,'CSPAOfflineRefund','Refunded CSPA charge because the other party was not online'),(37,'Corporation Account Withdrawal','Withdrawal from corporation account'),(38,'Corporation Dividend Payment',''),(39,'Corporation Registration Fee',''),(40,'Corporation Logo Change Cost',''),(41,'Release Of Impounded Property','Charge for the receipt of goods from a corporation hangar that is no longer rented'),(42,'Market Escrow',''),(43,'Agent Services Rendered','For miscellaneous services rendered by the agent'),(44,'Market Fine Paid',''),(45,'Corporation Liquidation','Funds from the liquidation of a corporation to a shareholder'),(46,'Broker fee',''),(47,'Corporation Bulk Payment','A payment from a corporation'),(48,'Alliance Registration Fee',''),(49,'War Fee',''),(50,'Alliance Maintainance Fee',''),(51,'Contraband Fine',''),(52,'Clone Transfer',''),(53,'Acceleration Gate Fee',''),(54,'Transaction Tax','Sales tax paid to the SCC for any transaction'),(55,'Jump Clone Installation Fee',''),(56,'Manufacturing','Installation and runtime cost for a manufacturing job'),(57,'Researching Technology','Installation and runtime cost for a technological research job'),(58,'Researching Time Productivity','Installation and runtime cost for a time productivity research job'),(59,'Researching Material Productivity','Installation and runtime cost for a material productivity research job'),(60,'Copying','Installation and runtime cost for a blueprint copying job'),(61,'Duplicating','Installation and runtime cost for an item duplication job'),(62,'Reverse Engineering','Installation and runtime cost for a reverse engineering job');
/*!40000 ALTER TABLE `market_refTypes` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2017-07-29 20:02:51
