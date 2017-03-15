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

INSERT INTO srvStatisticDataDescription (dataName, dataDescription)
VALUES
(1, 'Turret Shots Fired', 'Contains the total amount of players turret modules uses, or shots fired in certain period of time.'),
(2, 'Missiles Launched', 'Contains the total amount of players missiles launched (fired) in certain period of time.'),
(3, 'NPC Ships killed', 'Contains the total amount of NPC ships killed in certain period of time.'),
(4, 'Player Ships killed', 'Contains the total amount of player ships killed in certain period of time.'),
(5, 'Bounties Paid', 'Contains the total amount of bounty payouts (in ISK) in certain period of time. Note that the value MUST be INT.'),
(6, 'Bounties Placed', 'Contains the total amount of bounties placed by players in certain period of time. Note that the value MUST be INT.'),
(7, 'Ore Mined', 'Contains the total amount of ore (in m3) mined in certain period of time. Note that value MUST be INT.'),
(8, 'ISK Spent In Market', 'Contains the total amount of ISK spen in the marked in certain period of time. Note that value MUST be INT.'),
(9, 'Player Logins', 'Number of player logins.');
