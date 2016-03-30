# Server statistic - DB side skeleton

# This query creates the statistics container - simple 3-column table.
CREATE TABLE serverStatistic (
statID INT (5),
statValue INT (20),
timeEntered TIMESTAMP);

# This query creates the body for statistic description.
CREATE TABLE serverStatisticDescription (
statID INT (5),
statName TEXT,
statDescription TEXT);

# And this query fills the desctiption table with data.
INSERT INTO serverStatisticDescription (statID, statName, statDescription)
VALUES
(1, 'CPU Load', 'Contains the snapshot of server CPU load activity in certain period of time. Note that the value MUST be INT.'),
(2, 'Memory Use', 'Contains the snapshot of server memory usage in certain period of time. Note that the value MUST be INT.'),
(3, 'Total Turret Shots Fired', 'Contains the total amount of players turret modules uses, or shots fired in certain period of time.'),
(4, 'Total Missiles Launched', 'Contains the total amount of players missiles launched (fired) in certain period of time.'),
(5, 'Total NPC Ships killed', 'Contains the total amount of NPC ships killed in certain period of time.'),
(6, 'Total Player Ships killed', 'Contains the total amount of player ships killed in certain period of time.'),
(7, 'Total Bounties Paid', 'Contains the total amount of bounty payouts (in ISK) in certain period of time. Note that the value MUST be INT.'),
(8, 'Total Bounties Placed', 'Contains the total amount of bounties placed by players in certain period of time. Note that the value MUST be INT.'),
(9, 'Total Ore Mined', 'Contains the total amount of ore (in m3) mined in certain period of time. Note that value MUST be INT.'),
(10, 'Total ISK Spent In Market', 'Contains the total amount of ISK spen in the marked in certain period of time. Note that value MUST be INT.');
