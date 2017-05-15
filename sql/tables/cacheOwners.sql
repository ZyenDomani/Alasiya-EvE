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
-- Table structure for table `cacheOwners`
--

DROP TABLE IF EXISTS `cacheOwners`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cacheOwners` (
  `ownerID` int(10) unsigned NOT NULL DEFAULT '0',
  `ownerName` varchar(100) NOT NULL DEFAULT '',
  `typeID` int(10) unsigned NOT NULL DEFAULT '0',
  `ownerNameID` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `cacheOwners`
--

LOCK TABLES `cacheOwners` WRITE;
/*!40000 ALTER TABLE `cacheOwners` DISABLE KEYS */;
INSERT INTO `cacheOwners` VALUES (1000001,'Rogue Drone',2,0),(1000002,'CBD Corporation',2,0),(1000003,'Prompt Delivery',2,0),(1000004,'Ytiri',2,0),(1000005,'Hyasyoda Corporation',2,0),(1000006,'Deep Core Mining Inc.',2,0),(1000007,'Poksu Mineral Group',2,0),(1000008,'Minedrill',2,0),(1000009,'Caldari Provisions',2,0),(1000010,'Kaalakiota Corporation',2,0),(1000011,'Wiyrkomi Corporation',2,0),(1000012,'Top Down',2,0),(1000013,'Rapid Assembly',2,0),(1000014,'Perkone',2,0),(1000015,'Caldari Steel',2,0),(1000016,'Zainou',2,0),(1000017,'Nugoeihuvi Corporation',2,0),(1000018,'Echelon Entertainment',2,0),(1000019,'Ishukone Corporation',2,0),(1000020,'Lai Dai Corporation',2,0),(1000021,'Zero-G Research Firm',2,0),(1000022,'Propel Dynamics',2,0),(1000023,'Expert Distribution',2,0),(1000024,'CBD Sell Division',2,0),(1000025,'Sukuuvestaa Corporation',2,0),(1000026,'Caldari Constructions',2,0),(1000027,'Expert Housing',2,0),(1000028,'Caldari Funds Unlimited',2,0),(1000029,'State and Region Bank',2,0),(1000030,'Modern Finances',2,0),(1000031,'Chief Executive Panel',2,0),(1000032,'Mercantile Club',2,0),(1000033,'Caldari Business Tribunal',2,0),(1000034,'House of Records',2,0),(1000035,'Caldari Navy',2,0),(1000036,'Internal Security',2,0),(1000037,'Lai Dai Protection Service',2,0),(1000038,'Ishukone Watch',2,0),(1000039,'Home Guard',2,0),(1000040,'Peace and Order Unit',2,0),(1000041,'Spacelane Patrol',2,0),(1000042,'Wiyrkomi Peace Corps',2,0),(1000043,'Corporate Police Force',2,0),(1000044,'School of Applied Knowledge',2,0),(1000045,'Science and Trade Institute',2,0),(1000046,'Sebiestor tribe',2,0),(1000047,'Krusual tribe',2,0),(1000048,'Vherokior tribe',2,0),(1000049,'Brutor tribe',2,0),(1000050,'Republic Parliament',2,0),(1000051,'Republic Fleet',2,0),(1000052,'Republic Justice Department',2,0),(1000053,'Urban Management',2,0),(1000054,'Republic Security Services',2,0),(1000055,'Minmatar Mining Corporation',2,0),(1000056,'Core Complexion Inc.',2,0),(1000057,'Boundless Creation',2,0),(1000058,'Eifyr and Co.',2,0),(1000059,'Six Kin Development',2,0),(1000060,'Native Freshfood',2,0),(1000061,'Freedom Extension',2,0),(1000062,'The Leisure Group',2,0),(1000063,'Amarr Constructions',2,0),(1000064,'Carthum Conglomerate',2,0),(1000065,'Imperial Armaments',2,0),(1000066,'Viziam',2,0),(1000067,'Zoar and Sons',2,0),(1000068,'Noble Appliances',2,0),(1000069,'Ducia Foundry',2,0),(1000070,'HZO Refinery',2,0),(1000071,'Inherent Implants',2,0),(1000072,'Imperial Shipment',2,0),(1000073,'Amarr Certified News',2,0),(1000074,'Joint Harvesting',2,0),(1000075,'Nurtura',2,0),(1000076,'Further Foodstuffs',2,0),(1000077,'Royal Amarr Institute',2,0),(1000078,'Imperial Chancellor',2,0),(1000079,'Amarr Civil Service',2,0),(1000080,'Ministry of War',2,0),(1000081,'Ministry of Assessment',2,0),(1000082,'Ministry of Internal Order',2,0),(1000083,'Amarr Trade Registry',2,0),(1000084,'Amarr Navy',2,0),(1000085,'Court Chamberlain',2,0),(1000086,'Emperor Family',2,0),(1000087,'Kador Family',2,0),(1000088,'Sarum Family',2,0),(1000089,'Kor-Azor Family',2,0),(1000090,'Ardishapur Family',2,0),(1000091,'Tash-Murkon Family',2,0),(1000092,'Civic Court',2,0),(1000093,'Theology Council',2,0),(1000094,'TransStellar Shipping',2,0),(1000095,'Federal Freight',2,0),(1000096,'Inner Zone Shipping',2,0),(1000097,'Material Acquisition',2,0),(1000098,'Astral Mining Inc.',2,0),(1000099,'Combined Harvest',2,0),(1000100,'Quafe Company',2,0),(1000101,'CreoDron',2,0),(1000102,'Roden Shipyards',2,0),(1000103,'Allotek Industries',2,0),(1000104,'Poteque Pharmaceuticals',2,0),(1000105,'Impetus',2,0),(1000106,'Egonics Inc.',2,0),(1000107,'The Scope',2,0),(1000108,'Chemal Tech',2,0),(1000109,'Duvolle Laboratories',2,0),(1000110,'FedMart',2,0),(1000111,'Aliastra',2,0),(1000112,'Bank of Luminaire',2,0),(1000113,'Pend Insurance',2,0),(1000114,'Garoun Investment Bank',2,0),(1000115,'University of Caille',2,0),(1000116,'President',2,0),(1000117,'Senate',2,0),(1000118,'Supreme Court',2,0),(1000119,'Federal Administration',2,0),(1000120,'Federation Navy',2,0),(1000121,'Federal Intelligence Office',2,0),(1000122,'Federation Customs',2,0),(1000123,'Ammatar Fleet',2,0),(1000124,'Archangels',2,0),(1000125,'CONCORD',2,0),(1000126,'Ammatar Consulate',2,0),(1000127,'Guristas',2,0),(1000128,'Mordu\'s Legion',2,0),(1000129,'Outer Ring Excavations',2,0),(1000130,'Sisters of EVE',2,0),(1000131,'Society of Conscious Thought',2,0),(1000132,'Secure Commerce Commission',2,0),(1000133,'Salvation Angels',2,0),(1000134,'Blood Raiders',2,0),(1000135,'Serpentis Corporation',2,0),(1000136,'Guardian Angels',2,0),(1000137,'DED',2,0),(1000138,'Dominations',2,0),(1000139,'Food Relief',2,0),(1000140,'Genolution',2,0),(1000141,'Guristas Production',2,0),(1000142,'Impro',2,0),(1000143,'Inner Circle',2,0),(1000144,'Intaki Bank',2,0),(1000145,'Intaki Commerce',2,0),(1000146,'Intaki Space Police',2,0),(1000147,'Intaki Syndicate',2,0),(1000148,'InterBus',2,0),(1000149,'Jove Navy',2,0),(1000150,'Jovian directorate',2,0),(1000151,'Khanid Innovation',2,0),(1000152,'Khanid Transport',2,0),(1000153,'Khanid Works',2,0),(1000154,'Nefantar Miner Association',2,0),(1000155,'Prosper',2,0),(1000156,'Royal Khanid Navy',2,0),(1000157,'Serpentis Inquest',2,0),(1000158,'Shapeset',2,0),(1000159,'The Sanctuary',2,0),(1000160,'Thukker Mix',2,0),(1000161,'True Creations',2,0),(1000162,'True Power',2,0),(1000163,'Trust Partners',2,0),(1000164,'X-Sense',2,0),(1000165,'\r\nHedion University',2,0),(1000166,'Imperial Academy',2,0),(1000167,'State War Academy',2,0),(1000168,'Federal Navy Academy',2,0),(1000169,'Center for Advanced Studies',2,0),(1000170,'Republic Military School',2,0),(1000171,'Republic University',2,0),(1000172,'Pator Tech School',2,0),(1000177,'Material Institute',2,0),(1000178,'Academy of Aggressive Behaviour',2,0),(140000000,'allan',1380,0),(140000130,'lee',1379,0);
/*!40000 ALTER TABLE `cacheOwners` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2017-05-14 22:47:37
