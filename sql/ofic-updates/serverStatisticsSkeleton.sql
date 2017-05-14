/* Server Statistics Data
 *  Data for graphing ingame player activity
 */

CREATE TABLE `srvStatisticData` (
  `timeStamp` bigint(20) NOT NULL DEFAULT '0',
  `period` int(11) NOT NULL DEFAULT '0',
  `1` int(10) unsigned NOT NULL DEFAULT '0',
  `2` int(10) unsigned NOT NULL DEFAULT '0',
  `3` int(10) unsigned NOT NULL DEFAULT '0',
  `4` int(10) unsigned NOT NULL DEFAULT '0',
  `5` int(10) unsigned NOT NULL DEFAULT '0',
  `6` int(10) unsigned NOT NULL DEFAULT '0',
  `7` int(10) unsigned NOT NULL DEFAULT '0',
  `8` int(10) unsigned NOT NULL DEFAULT '0',
  `9` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Current Data for graphing player activity over period of time';

CREATE TABLE `srvStatisticDataHistory` (
  `month` tinyint(2) NOT NULL DEFAULT '1',
  `1` int(10) unsigned NOT NULL DEFAULT '0',
  `2` int(10) unsigned NOT NULL DEFAULT '0',
  `3` int(10) unsigned NOT NULL DEFAULT '0',
  `4` int(10) unsigned NOT NULL DEFAULT '0',
  `5` int(10) unsigned NOT NULL DEFAULT '0',
  `6` int(10) unsigned NOT NULL DEFAULT '0',
  `7` int(10) unsigned NOT NULL DEFAULT '0',
  `8` int(10) unsigned NOT NULL DEFAULT '0',
  `9` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Historical Data for graphing player activity from previous months';


CREATE TABLE `srvStatisticDataDescription` (
  `dataID` tinyint(1) unsigned NOT NULL,
  `dataName` varchar(30) NOT NULL,
  `dataDescription` varchar(200) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Descriptions for DataNames in srvStatisticData table';

INSERT INTO srvStatisticDataDescription (dataID, dataName, dataDescription)
VALUES
(1, 'Turret Shots Fired', 'Shots fired from turrents on player ships.'),
(2, 'Missiles Launched', 'Missiles fired from launchers on player ships.'),
(3, 'NPC Ships killed', 'NPC ships killed.'),
(4, 'Player Ships killed', 'Players popped (not tracking podded).'),
(5, 'Bounties Paid', 'Amount of bounty payouts in ISK.'),
(6, 'Bounties Placed', 'Amount of bounties placed in ISK.'),
(7, 'Ore Mined', 'M3 of ore mined.'),
(8, 'ISK Spent In Market', 'ISK spent in the market, not including broker fees.'),
(9, 'Player Logins', 'Number of player logins.');
