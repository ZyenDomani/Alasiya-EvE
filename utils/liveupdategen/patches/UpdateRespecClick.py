#@liveupdate("globalClassMethod", "form.attributeRespecWindow::AttributeRespecWindow", "OnMemberBoxClick")
def OnMemberBoxClick(self, oldValue, newValue):
    if oldValue is None or oldValue == newValue:
        return
    if self.readOnly:
        return
    self.unspentPts -= newValue - oldValue
    self.sr.unassignedBar.SetValue(self.unspentPts)
    unspentPtsText = localizationUtil.FormatNumeric(self.unspentPts, decimalPlaces=0)
    self.availableLabel.text = '<right>%s</right>' % unspentPtsText
    for x in xrange(0, 5):
        totalPts = 45 + self.respecBar[x].GetValue() + self.implantModifier[x]
        totalPtsText = localizationUtil.FormatNumeric(int(totalPts), decimalPlaces=0)
        self.totalLabels[x].text = '<right>%s</right>' % totalPtsText

    if self.unspentPts <= 0:
        self.sr.saveWarningText.state = uiconst.UI_HIDDEN
