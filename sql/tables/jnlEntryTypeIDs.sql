-- phpMyAdmin SQL Dump
-- version 4.4.15.5
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Dec 09, 2017 at 02:08 AM
-- Server version: 10.0.24-MariaDB
-- PHP Version: 5.6.30

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `alasiya-new`
--

-- --------------------------------------------------------

--
-- Table structure for table `jnlEntryTypeIDs`
--

CREATE TABLE IF NOT EXISTS `jnlEntryTypeIDs` (
  `entryTypeID` int(10)  NOT NULL DEFAULT '0',
  `entryTypeName` varchar(100) NOT NULL DEFAULT '',
  `description` mediumtext NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `jnlEntryTypeIDs`
--

INSERT INTO `jnlEntryTypeIDs` (`entryTypeID`, `entryTypeName`, `description`) VALUES
(0, 'Undefined', ''),
(1, 'Player Trading', ''),
(2, 'Market Transaction', ''),
(3, 'GM Cash Transfer', ''),
(4, 'ATM Withdraw', ''),
(5, 'ATM Deposit', ''),
(6, 'Backward Compatible', ''),
(7, 'Mission Reward', ''),
(8, 'Clone Activation', ''),
(9, 'Inheritance', ''),
(10, 'Player Donation', 'Player gave cash to another owner'),
(11, 'Corporation Payment', 'CEO or Accountant transferred cash  from corp. account'),
(12, 'Docking Fee', ''),
(13, 'Office Rental Fee', ''),
(14, 'Factory Slot Rental Fee', ''),
(15, 'Repair Bill', ''),
(16, 'Bounty', 'Player gave cash to someone''s  bounty pool'),
(17, 'Bounty Prize', 'Player got bounty prize for killing someone'),
(18, 'Agents_temporary', 'TEMP'),
(19, 'Insurance', ''),
(20, 'Mission Expiration', ''),
(21, 'Mission Completion', ''),
(22, 'Shares', ''),
(23, 'Courier Mission Escrow', ''),
(24, 'Mission Cost', ''),
(25, 'Agent Miscellaneous', 'Agent paid you'),
(26, 'Miscellaneous Payment To Agent', 'You paid agent'),
(27, 'Agent Location Services', 'You paid agent to locate somebody'),
(28, 'Agent Donation', 'You donated/bribed the agent'),
(29, 'Agent Security Services', 'You paid agent to clean your rep'),
(30, 'Agent Mission Collateral Paid', 'You gave agent collateral for a mission'),
(31, 'Agent Mission Collateral Refunded', 'The agent returned collateral to you'),
(32, 'Agents_preward', 'The agent gave you this when you accepted the mission'),
(33, 'Agent Mission Reward', 'The agent gave you this as a reward'),
(34, 'Agent Mission Time Bonus Reward', 'The agent gave you this as a special reward for fast mission completion'),
(35, 'CSPA', 'CONCORD Spam Prevention Act'),
(36, 'CSPAOfflineRefund', 'Refunded CSPA charge because the other party was not online'),
(37, 'Corporation Account Withdrawal', 'Withdrawal from corporation account'),
(38, 'Corporation Dividend Payment', ''),
(39, 'Corporation Registration Fee', ''),
(40, 'Corporation Logo Change Cost', ''),
(41, 'Release Of Impounded Property', 'Charge for the receipt of goods from a corporation hangar that is no longer rented'),
(42, 'Market Escrow', ''),
(43, 'Agent Services Rendered', 'For miscellaneous services rendered by the agent'),
(44, 'Market Fine Paid', ''),
(45, 'Corporation Liquidation', 'Funds from the liquidation of a corporation to a shareholder'),
(46, 'Broker fee', ''),
(47, 'Corporation Bulk Payment', 'A payment from a corporation'),
(48, 'Alliance Registration Fee', ''),
(49, 'War Fee', ''),
(50, 'Alliance Maintainance Fee', ''),
(51, 'Contraband Fine', ''),
(52, 'Clone Transfer', ''),
(53, 'Acceleration Gate Fee', ''),
(54, 'Transaction Tax', 'Sales tax paid to the SCC for any transaction'),
(55, 'Jump Clone Installation Fee', ''),
(56, 'Manufacturing', 'Installation and runtime cost for a manufacturing job'),
(57, 'Researching Technology', 'Installation and runtime cost for a technological research job'),
(58, 'Researching Time Productivity', 'Installation and runtime cost for a time productivity research job'),
(59, 'Researching Material Productivity', 'Installation and runtime cost for a material productivity research job'),
(60, 'Copying', 'Installation and runtime cost for a blueprint copying job'),
(61, 'Duplicating', 'Installation and runtime cost for an item duplication job'),
(62, 'Reverse Engineering', 'Installation and runtime cost for a reverse engineering job');

/*  update to add these.  defined in client and server, but this entire set should be sent via bulkdata
            ContractAuctionBid = 63,     // *Contract ID
            ContractAuctionBidRefund = 64,     // *Contract ID
            ContractCollateral = 65,
            ContractRewardRefund = 66,
            ContractAuctionSold = 67,
            ContractReward = 68,
            ContractCollateralRefund = 69,
            ContractCollateralPayout = 70,
            ContractPrice = 71,     // *Contract ID
            ContractBrokersFee = 72,     // *Contract ID
            ContractSalesTax = 73,     // *Contract ID
            ContractDeposit = 74,     // *Contract ID
            ContractDepositSalesTax = 75,
            SecureEVETimeCodeExchange = 76,
            ContractAuctionBidCorp = 77,
            ContractCollateralCorp = 78,
            ContractPriceCorp = 79,     // *Contract ID
            ContractBrokersFeeCorp = 80,     // *Contract ID
            ContractDepositCorp = 81,     // *Contract ID
            ContractDepositRefund = 82,     // *Contract ID
            ContractRewardAdded = 83,
            ContractRewardAddedCorp = 84,
            BountyPrizes = 85,     // * systemID (and type:amt,type:amt,etc  in description (max ~60chars))
            CorporationAdvertisementFee = 86,
            MedalCreation = 87,     // * Character ID of the player creating the medal
            MedalIssuing = 88,     // * Character ID of the player issuing the medal
            AttributeRespecification = 90,
            SovereignityRegistrarFee = 91,
            CorporationTaxNpcBounties = 92,
            CorporationTaxAgentRewards = 93,
            CorporationTaxAgentBonusRewards = 94,
            SovereignityUpkeepAdjustment = 95,
            PlanetaryImportTax = 96,     // * Planet ID
            PlanetaryExportTax = 97,     // * Planet ID
            PlanetaryConstruction = 98,
            RewardManager = 99,
            BountySurcharge = 101,
            ContractReversal = 102,
            CorporationTaxRewards = 103,
            StorePurchase = 106,
            StoreRefund = 107,
            PlexConversion = 108,
            AurumGiveAway = 109,
            MiniGameHouseCut = 110,
            AurumTokenConversion = 111,
            Max = 120,
            /* not sure on these
            ModifyISK = 10001,
            PrimaryMarketplacePurchase = 10002,
            BattleReward = 10003,
            NewCharacterStartingFunds = 10004,
            CorporationAccountWithdrawal = 10005,
            CorporationAccountDeposit = 10006,
            BattleWPWinReward = 10007,
            BattleWPLossReward = 10008,
            BattleWinReward = 10009,
            BattleLossReward = 10010,
            ModifyAUR = 11001,
            RespecPayment = 11002

            */
--
-- Indexes for dumped tables
--

--
-- Indexes for table `jnlEntryTypeIDs`
--
ALTER TABLE `jnlEntryTypeIDs`
  ADD PRIMARY KEY (`entryTypeID`);

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
