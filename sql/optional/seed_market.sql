-- seeds specified region with skills and ships
-- regionID, 02: The Forge - 01:derelik - 30:heimatar - 16:lonetrek - 42:metropolis - 43:domain - 32:Sinq Laison
set @regionid=10000032;
set @saturation=1.0; -- 80% of stations are filled with orders
-- change "categoryID in (16,6)" to categories you want to be seeded

use EVE_Crucible;

-- select stations to fill
create temporary table if not exists tStations (stationId int, solarSystemID int, regionID int);
truncate table tStations;
select round(count(stationID)*@saturation) into @lim from staStations where regionID=@regionid ;
set @i=0;
insert into tStations
  select stationID,solarSystemID,regionID from staStations where (@i:=@i+1)<=@lim AND regionID=@regionid order by rand();

-- actual seeding
INSERT INTO market_orders (typeID, charID, regionID, stationID, bid, price, volEntered, volRemaining, issued, orderState, minVolume, contraband, accountID, duration, isCorp, solarSystemID, escrow, jumps)
  SELECT typeID,1 as charID, regionID, stationID, 0 as bid, IF(basePrice=0, 1000, basePrice/100) as price, 550 as volEntered, 550 as volRemaining, 130565976636875000 as issued,1 as orderState, 1 as minVolume,0 as contraband, 0 as accountID, 18250 as duration,0 as isCorp, solarSystemID, 0 as escrow, 15 as jumps
  FROM tStations, invTypes inner join invGroups on invTypes.groupID=invGroups.groupID
  WHERE invTypes.published = 1 and categoryID IN (4, 5, 6, 7, 8, 9, 16, 17, 18, 22, 23, 24, 32, 34, 35, 39, 40, 41, 42, 43, 46);

groupid 450 - 462  catid    ores
groupid 34 - 40  catid   minerals

categoryID  categoryName
4   Material
5   Accessories
6   Ship
7   Module
8   Charge
9   Blueprint
16  Skill
17  Commodity
18  Drone
22  Deployable
23  Structure
24  Reaction
32  Subsystem
34  Ancient Relics
35  Decryptors
39  Infrastructure Upgrades
40  Sovereignty Structures
41  Planetary Interaction
42  Planetary Resources
43  Planetary Commodities
46  Orbitals
