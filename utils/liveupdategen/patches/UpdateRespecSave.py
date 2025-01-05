#@liveupdate("globalClassMethod", "form.attributeRespecWindow::AttributeRespecWindow", "SaveChanges")
def SaveChanges(self, *args):
    totalAttrs = 0
    newAttributes = {}
    for x in xrange(0, 5):
        newAttributes[self.attributes[x]] = 45 + self.respecBar[x].GetValue()

    for attrValue in newAttributes.itervalues():
        if attrValue < 45:
            raise UserError('RespecAttributesTooLow')
        elif attrValue > 65:
            raise UserError('RespecAttributesTooHigh')
        totalAttrs += attrValue

    if totalAttrs != 300 or self.sr.unassignedBar.GetValue() > 0:
        self.sr.saveWarningText.state = uiconst.UI_DISABLED
        raise UserError('RespecAttributesMisallocated')
    allSame = True
    for attr in self.attributes:
        if int(self.currentAttributes[attr]) != int(newAttributes[attr]):
            allSame = False
            break

    if not allSame:
        respecInfo = sm.GetService('skills').GetRespecInfo()
        freeRespecs = respecInfo['freeRespecs']
        if respecInfo['nextTimedRespec'] is None or respecInfo['nextTimedRespec'] <= blue.os.GetWallclockTime():
            if eve.Message('ConfirmRespec2', {'months': int(3)}, uiconst.YESNO) != uiconst.ID_YES:
                return
        elif freeRespecs > 0:
            if eve.Message('ConfirmRespecFree', {'freerespecs': int(respecInfo['freeRespecs']) - 1}, uiconst.YESNO) != uiconst.ID_YES:
                return
        else:
            raise UserError('RespecTooSoon', {'nextTime': respecInfo['nextTimedRespec']})
        self.skillHandler.RespecCharacter(newAttributes[const.attributeCharisma], newAttributes[const.attributeIntelligence], newAttributes[const.attributeMemory], newAttributes[const.attributePerception], newAttributes[const.attributeWillpower])
    self.CloseByUser()

