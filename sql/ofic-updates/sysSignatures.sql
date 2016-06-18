/* anomaly and signature table
 *  this one is used for saving anomalies and signatures
 * and retrieving scanning results
 */



--
-- Table structure for table `sysSignatures`
--
DROP TABLE IF EXISTS `sysSignatures`;
CREATE TABLE IF NOT EXISTS `sysSignatures` (
  `sigID` varchar(7) COLLATE utf8_unicode_ci NOT NULL,
  `dungeonName` varchar(50) COLLATE utf8_unicode_ci NOT NULL,
  `systemID` int(11) NOT NULL,
  `sigItemID` int(11) NOT NULL,
  `typeID` int(11) NOT NULL,
  `scanGroupID` int(11) NOT NULL,
  `groupID` int(10) NOT NULL DEFAULT '0',
  `strengthAttributeID` int(10) NOT NULL DEFAULT '0',
  `x` double NOT NULL,
  `y` double NOT NULL,
  `z` double NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Indexes for table `sysSignatures`
--
ALTER TABLE `sysSignatures`
  ADD PRIMARY KEY (`sigID`),
  ADD UNIQUE KEY `sigID` (`sigID`),
  ADD UNIQUE(`sigID`);