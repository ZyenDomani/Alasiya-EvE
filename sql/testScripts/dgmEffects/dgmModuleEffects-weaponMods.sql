
-- weapon Mods effects data
-- not used yet

INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenalty`, `effectState`, `targetType`, `targetGroup`)
VALUES
-- ID, src, targ, calc, des, rcalc, tgrpID, stack, state, targetType, targetGroup

-- this is a template to add weapon-modifying modules here
-- ship scan strength
(2231, 1030, 208, 5, 'ECCM - Radar Strength', 6, '6', 1, 2, 1, 202);