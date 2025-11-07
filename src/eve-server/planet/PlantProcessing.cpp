
/*
rewrite plant process method

are we looping or still running delta/runs?

idea...loop thru pLevel from lo to hi for proper item processing
    use var for m_pLevel
    must loop plants to allow for silo feeding multiple plants with same matl
      (first plant takes all material)

    this has all process pins, mapped by plevel
m_plantMap.equal_range(curCycle);

get main plant pin
    do misc data checks
    do time checks
    do matl checks

dest route -> endpoint will be *this plant
    find source and check for contents

check all inputs
verify plant can run this loop
process plant
src route -> srcpoint will be *this plant
    can endpoint accept products?
    add products to endpoint
*/

uint32 CanAccept(pin, typeID, qty) {
    // returns remainingSpace




}

    def CanAccept(self, typeID, quantity):  -plant
        if typeID not in self.demands:
            return 0
        if quantity < 0:
            quantity = self.demands[typeID]
        remainingSpace = self.demands[typeID]
        if typeID in self.contents:
            remainingSpace = self.demands[typeID] - self.contents[typeID]
        if remainingSpace < quantity:
            return remainingSpace
        return quantity

    def CanAccept(self, typeID, quantity):  -basePin
        if self.activityState < STATE_IDLE:
            return 0
        if self.GetCapacity() is not None:
            newTypeObj = cfg.invtypes.Get(typeID)
            newVolume = newTypeObj.volume * quantity
            capacityRemaining = max(0, self.GetCapacity() - self.capacityUsed)
            if newVolume > capacityRemaining or quantity == -1:
                return int(capacityRemaining / newTypeObj.volume)
            else:
                return quantity
        else:
            return max(0, quantity)



