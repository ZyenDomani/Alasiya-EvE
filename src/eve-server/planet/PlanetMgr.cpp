
 /**
  * @name PlanetMgr.cpp
  *   Specific Class for managing planet resources
  * @Author:         Allan
  * @date:   30 April 2016
  */



#include "eve-server.h"

#include "planet/PlanetMgr.h"


PlanetMgr::PlanetMgr()
{

}

    /* these are for PI */
/*
    AttrHarvesterType = 709,
    AttrHarvesterQuality = 710,
    AttrLogisticalCapacity = 1631,
    AttrPlanetRestriction = 1632,
    AttrPowerLoadPerKm = 1633,
    AttrCPULoadPerKm = 1634,
    AttrCPULoadLevelModifier = 1635,
    AttrPowerLoadLevelModifier = 1636,
    AttrImportTax = 1638,
    AttrExportTax = 1639,
    AttrImportTaxMultiplier = 1640,
    AttrExportTaxMultiplier = 1641,
    AttrPinExtractionQuantity = 1642,
    AttrPinCycleTime = 1643,
    AttrExtractorDepletionRange = 1644,
    AttrExtractorDepletionRate = 1645,
    AttrSpecialCommandCenterHoldCapacity = 1646,
    */

/*
piLaunchOrbitDecayTime = DAY * 5
piCargoInOrbit = 0
piCargoDeployed = 1
piCargoClaimed = 2
piCargoDeleted = 3
*/

/*
planetResourceScanDistance = 1000000000
planetResourceProximityDistant = 0
planetResourceProximityRegion = 1
planetResourceProximityConstellation = 2
planetResourceProximitySystem = 3
planetResourceProximityPlanet = 4
planetResourceProximityLimits = [(2, 6),
 (4, 10),
 (6, 15),
 (10, 20),
 (15, 30)]
planetResourceScanningRanges = [9.0,
 7.0,
 5.0,
 3.0,
 1.0]
planetResourceUpdateTime = 1 * HOUR
planetResourceMaxValue = 1.21
*/