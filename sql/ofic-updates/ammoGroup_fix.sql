# Adding a primary ammo types for all torpedo launchers.
INSERT INTO dgmTypeAttributes (typeID, attributeID, valueInt) VALUES
('503', '604','89'),
('2420', '605','89'),
('8001','604','89'),
('8113','604','89'),
('8115','604','89'),
('8117','604','89'),
('13923','604','89'),
('13924','604','89'),
('14524','604','89'),
('14525','604','89'),
('14526','604','89'),
('14527','604','89'),
('14680','604','89'),
('14681','604','89'),
('14682','604','89'),
('14683','604','89'),
('16067','604','89'),
('17490','604','89'),
('20603','604','89'),
('22569','604','89'),
('28513','604','89');

# Updating capital meta-turrets - hybrid
UPDATE dgmTypeAttributes
SET valueInt = 85,
valueFloat = NULL
WHERE attributeID = 604
AND typeID IN (3546, 3550);

# Updating capital meta-turrets - energy
UPDATE dgmTypeAttributes
SET valueInt = 86,
valueFloat = NULL
WHERE attributeID = 604
AND typeID IN (3559, 3561);

# Updating capital meta-turrets - projectiles
UPDATE dgmTypeAttributes
SET valueInt = 83,
valueFloat = NULL
WHERE attributeID = 604
AND typeID IN (3571, 3573);

# Updating and inserting the primary ammo for capital cruise and torp launchers
UPDATE dgmTypeAttributes
SET valueInt = 1019,
valueFloat = NULL
WHERE attributeID = 604
AND typeID = 3563;

INSERT INTO dgmTypeAttributes (typeID, attributeID, valueInt)
VALUES
(32444, 604, 1019);

INSERT INTO dgmTypeAttributes (typeID, attributeID, valueInt)
VALUES
(3565, 604, 476),
(20539, 604, 476);

# Deleting the invalid ammoGroups
DELETE FROM dgmTypeAttributes
WHERE attributeID in (605, 606)
AND typeID in (3563, 3565, 20539, 32444);