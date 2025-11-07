
// client python methods to get ideas on how to improve colony plant processing...


    for pin in self.colonyData.pins.itervalues():
        if pin.CanRun(simEndTime):
            self.SchedulePin(pin)


    def SchedulePin(self, pin):
        nextRunTime = pin.GetNextRunTime()
        for evalTime, evalPinID in self.simQueue:
            if evalPinID == pin.id:
                if nextRunTime is None or nextRunTime < evalTime:
                    self.LogInfo('SchedulePin::Rescheduling pin', pin.id, 'from', evalTime, 'to', self.currentSimTime)
                    self.simQueue.remove((evalTime, evalPinID))
                else:
                    return

        if nextRunTime is None or nextRunTime < self.currentSimTime:
            self.AddTimer(pin.id, self.currentSimTime)
        else:
            self.AddTimer(pin.id, nextRunTime)


    def RunSimulation(self, runSimUntil = None, beNice = True):
        if self.colonyData is None:
            raise RuntimeError('Attempting to run simulation on a colony without attached colonyData')
        with self.planetBroker.LockedService(self.ownerID):
            if self.currentSimTime is None:
                self.RecalculateCurrentSimTime()
            if self.currentSimTime is None:
                raise RuntimeError('CurrentSimTime is none for character', self.ownerID, '. This is a fatal error that can cause infinite looping.')
            simEndTime = runSimUntil if runSimUntil is not None else blue.os.GetWallclockTime()
            with bluepy.Timer('BaseColony::PrimeSimulation'):
                self.PrimeSimulation(simEndTime)
            with bluepy.Timer('BaseColony::RunSimulation'):
                while len(self.simQueue) > 0:
                    simTime, simPinID = heapq.heappop(self.simQueue)
                    if simTime > simEndTime:
                        break
                    self.currentSimTime = simTime
                    simPin = self.GetPin(simPinID)
                    if simPin is None:
                        log.LogTraceback('Unable to find scheduled pin! This should never happen!')
                        continue
                    with bluepy.TimerPush('EvaluatePin::' + simPin.__guid__):
                        if not simPin.CanRun(self.currentSimTime):
                            continue
                        self.EvaluatePin(simPin)
                    if beNice:
                        blue.pyos.BeNice()

                self.currentSimTime = simEndTime
            return simEndTime

    def Run(self, runTime):
        products = {}
        if self.IsActive():
            products = self.products.copy()
        canConsume = True
        for demandTypeID, demandQty in self.demands.iteritems():
            if demandTypeID not in self.contents:
                canConsume = False
                break
            if demandQty > self.contents[demandTypeID]:
                canConsume = False
                break

        if canConsume:
            for demandTypeID, demandQty in self.demands.iteritems():
                self.RemoveCommodity(demandTypeID, demandQty)

            self.SetState(planet.STATE_ACTIVE)
        else:
            self.SetState(planet.STATE_IDLE)
        self.receivedInputsLastCycle = self.hasReceivedInputs
        self.hasReceivedInputs = False
        self.lastRunTime = runTime
        return products


    def IsActive(self):
        return self.activityState > STATE_IDLE

    def IsActive(self):  --ecus
        return self.programType is not None and self.activityState > planet.STATE_IDLE


    def SetContents(self, newContents):
        self.contents = dict(newContents)
        self.capacityUsed = 0
        for typeID, qty in self.contents.iteritems():
            self.capacityUsed += qty * cfg.invtypes.Get(typeID).volume

    def HasEnoughInputs(self):
        for demandTypeID, demandQty in self.demands.iteritems():
            if demandTypeID not in self.contents:
                return False
            if demandQty > self.contents[demandTypeID]:
                return False

        return True

    def CanActivate(self):
        if self.activityState < planet.STATE_IDLE:
            return False
        if self.schematicID is None:
            return False
        if self.IsActive():
            return True
        if self.hasReceivedInputs or self.receivedInputsLastCycle:
            return True
        if not self.HasEnoughInputs():
            return False
        return True

    def GetNextRunTime(self):
        if not self.IsActive() and self.HasEnoughInputs():
            return None
        else:
            return planet.BasePin.GetNextRunTime(self)

    def CanRun(self, runTime = None):
        if not self.IsActive() and not self.CanActivate():
            return False
        rt = runTime
        if runTime is None:
            rt = blue.os.GetWallclockTime()
        nextRunTime = self.GetNextRunTime()
        if nextRunTime is None or nextRunTime <= rt:
            return True
        return False

    def AddCommodity(self, typeID, quantity):
        qtyAdded = self._AddCommodity(typeID, quantity)
        if qtyAdded > 0:
            self.hasReceivedInputs = True
        return qtyAdded

    def _AddCommodity(self, typeID, quantity):
        quantityToAdd = self.CanAccept(typeID, quantity)
        if quantityToAdd < 1:
            return 0
        if self.GetCapacity() is not None:
            newTypeObj = cfg.invtypes.Get(typeID)
            self.capacityUsed += quantityToAdd * newTypeObj.volume
        if typeID not in self.contents:
            self.contents[typeID] = quantityToAdd
        else:
            self.contents[typeID] += quantityToAdd
        return quantityToAdd

    def CanRemove(self, typeID, quantity):
        if self.activityState < STATE_IDLE:
            return 0
        if typeID not in self.contents:
            return 0
        return min(self.contents[typeID], quantity)

    def RemoveCommodity(self, typeID, quantity):
        return self._RemoveCommodity(typeID, quantity)

    def _RemoveCommodity(self, typeID, quantity):
        if typeID not in self.contents:
            return 0
        qtyRemoved = 0
        if self.contents[typeID] <= quantity:
            qtyRemoved = self.contents[typeID]
            del self.contents[typeID]
        else:
            qtyRemoved = quantity
            self.contents[typeID] -= qtyRemoved
        if self.GetCapacity() is not None:
            self.capacityUsed = max(0, self.capacityUsed - cfg.invtypes.Get(typeID).volume * qtyRemoved)
        return qtyRemoved

    def IsConsumer(self):  -process pins
        return True

    def SetSchematic(self, schematic):  -process pins
        self.demands = {}
        self.products = {}
        for commodity in cfg.schematicstypemap.get(schematic.schematicID, []):
            if commodity.isInput:
                self.demands[commodity.typeID] = commodity.quantity
            else:
                self.products[commodity.typeID] = commodity.quantity

        self.schematicID = schematic.schematicID
        self.cycleTime = schematic.cycleTime * SEC
        newContents = {}
        for commodityID, quantity in self.contents.iteritems():
            if commodityID in self.demands:
                newContents[commodityID] = quantity if quantity < self.demands[commodityID] else self.demands[commodityID]

        self.contents = newContents

    def TransferCommodities(self, sourcePinID, destPinID, typeID, qty, commodities, maxAmount = None):
        if self.colonyData is None:
            raise RuntimeError('Unable to execute route - no colony data')
        sourcePin = self.GetPin(sourcePinID)
        if not sourcePin:
            raise RuntimeError('Unable to find pin', sourcePinID)
        commodsToPush = {}
        if typeID not in commodities:
            return (0, 0)
        amtToMove = min(commodities[typeID], qty)
        if maxAmount is not None:
            amtToMove = min(maxAmount, amtToMove)
        if amtToMove <= 0:
            return (0, 0)
        destPin = self.GetPin(destPinID)
        if not destPin:
            raise RuntimeError('Unable to find pin', destPinID)
        amtMoved = destPin.AddCommodity(typeID, amtToMove)
        if sourcePin.IsStorage():
            sourcePin.RemoveCommodity(typeID, amtMoved)
        return (typeID, amtMoved)

    def RouteCommodityOutput(self, sourcePin, commodities):
        if self.colonyData is None:
            raise RuntimeError('No colony data attached - cannot route commodity output')
        pinsReceivingCommodities = {}
        done = False
        for isStorageRoutes, listOfRoutes in enumerate(self.colonyData.GetSortedRoutesForPin(sourcePin.id, commodities)):
            if done:
                break
            while listOfRoutes:
                dummy, (destID, commodityTypeID, qty) = heapq.heappop(listOfRoutes)
                maxAmount = None
                if isStorageRoutes:
                    maxAmount = math.ceil(float(commodities.get(commodityTypeID, 0)) / (len(listOfRoutes) + 1))
                typeID, qty = self.TransferCommodities(sourcePin.id, destID, commodityTypeID, qty, commodities, maxAmount=maxAmount)
                if typeID in commodities:
                    commodities[typeID] -= qty
                    if commodities[typeID] <= 0:
                        del commodities[typeID]
                if qty > 0:
                    if destID not in pinsReceivingCommodities:
                        pinsReceivingCommodities[destID] = {}
                    if typeID not in pinsReceivingCommodities[destID]:
                        pinsReceivingCommodities[destID][typeID] = 0
                    pinsReceivingCommodities[destID][typeID] += qty
                if len(commodities) <= 0:
                    done = True
                    break

        for receivingPinID, commodsAdded in pinsReceivingCommodities.iteritems():
            receivingPin = self.GetPin(receivingPinID)
            if receivingPin.IsConsumer():
                self.SchedulePin(receivingPin)
            if not sourcePin.IsStorage() and receivingPin.IsStorage():
                self.LogInfo('RouteCommodityOutput :: Redistributing added commods', commodsAdded, 'from', sourcePin.id, 'via', receivingPin.id)
                self.RouteCommodityOutput(receivingPin, commodsAdded)

    def RouteCommodityInput(self, destinationPin):
        if self.colonyData is None:
            raise RuntimeError('No colony data attached - cannot route commodity input')
        routesToEvaluate = self.colonyData.GetDestinationRoutesForPin(destinationPin.id)
        for routeID in routesToEvaluate:
            route = self.colonyData.GetRoute(routeID)
            sourcePinID = route.GetSourcePinID()
            sourcePin = self.GetPin(sourcePinID)
            if sourcePin is None:
                self.LogWarn('Route', routeID, 'has nonexistent source pin', sourcePinID)
                continue
            if not sourcePin.IsStorage():
                continue
            storedCommods = sourcePin.GetContents()
            if len(storedCommods) < 1:
                continue
            self.LogInfo('RouteCommodityInput :: Routing', storedCommods, 'from', route.GetSourcePinID(), 'to', route.GetDestinationPinID())
            self.ExecuteRoute(routeID, storedCommods)

    def EvaluatePin(self, pin):
        self.LogInfo('EvaluatePin ::', pin.id, '(', pin.typeID, ') at', self.currentSimTime)
        if not pin.CanActivate() and not pin.IsActive():
            return
        self.LogInfo('EvaluatePin ::', pin.id, '(', pin.typeID, ') Running pin')
        with bluepy.TimerPush('Run'):
            commods = pin.Run(self.currentSimTime)
        if pin.IsConsumer():
            self.LogInfo('EvaluatePin :: Consumer detected, routing inputs', pin.id)
            with bluepy.TimerPush('RouteCommodityInput'):
                self.RouteCommodityInput(pin)
        if pin.IsActive() or pin.CanActivate():
            self.LogInfo('EvaluatePin ::', pin.id, 'Scheduling pin')
            self.SchedulePin(pin)
        if len(commods) == 0:
            return
        self.LogInfo('EvaluatePin ::', pin.id, 'Routing outputs')
        with bluepy.TimerPush('RouteCommodityOutput'):
            self.RouteCommodityOutput(pin, commods)
        self.LogInfo('EvaluatePin ::', pin.id, 'Done')

    def StimulateIdlePin(self, pin):
        if pin is None:
            raise RuntimeError('Cannot stimulate a None pin')
        if pin.IsStorage():
            self.RouteCommodityOutput(pin, pin.GetContents())
        if pin.IsConsumer():
            pin.hasReceivedInputs = True
        if not pin.IsActive() and pin.CanActivate():
            self.SchedulePin(pin)
