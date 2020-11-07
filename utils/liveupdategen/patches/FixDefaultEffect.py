#@liveupdate("globalClassMethod", "dogmax.DogmaLocation::CapacitorSimulator", "CapacitorSimulator")
def CapacitorSimulator(self, shipID):
    dogmaItem = self.dogmaItems[shipID]
    capacitorCapacity = self.GetAttributeValue(shipID, const.attributeCapacitorCapacity)
    rechargeTime = self.GetAttributeValue(shipID, const.attributeRechargeRate)
    modules = []
    totalCapNeed = 0
    for moduleID, module in dogmaItem.GetFittedItems().iteritems():
        if not module.IsOnline():
            continue
        try:
            defaultEffectID = self.dogmaStaticMgr.GetDefaultEffect(module.typeID)
        except KeyError:
            defaultEffectID = None
            sys.exc_clear()

        if defaultEffectID is None:
            continue
        if defaultEffectID == 0:
            continue

        defaultEffect = self.dogmaStaticMgr.effects[defaultEffectID]
        durationAttributeID = defaultEffect.durationAttributeID
        dischargeAttributeID = defaultEffect.dischargeAttributeID
        if durationAttributeID is None or dischargeAttributeID is None:
            continue
        duration = self.GetAttributeValue(moduleID, durationAttributeID)
        capNeed = self.GetAttributeValue(moduleID, dischargeAttributeID)
        modules.append([capNeed, long(duration * const.dgmTauConstant), 0])
        totalCapNeed += capNeed / duration

    rechargeRateAverage = capacitorCapacity / rechargeTime
    peakRechargeRate = 2.5 * rechargeRateAverage
    tau = rechargeTime / 5
    TTL = None
    if totalCapNeed > peakRechargeRate:
        TTL = self.RunSimulation(capacitorCapacity, rechargeTime, modules)
        loadBalance = 0
    else:
        c = 2 * capacitorCapacity / tau
        k = totalCapNeed / c
        exponent = (1 - math.sqrt(1 - 4 * k)) / 2
        if exponent == 0:
            loadBalance = 1
        else:
            t = -math.log(exponent) * tau
            loadBalance = (1 - math.exp(-t / tau)) ** 2
    return (peakRechargeRate, totalCapNeed, loadBalance, TTL)
