#@liveupdate("globalClassMethod", "form.attributeRespecWindow::AttributeRespecWindow", "Load")
def Load(self, *args):
    if not eve.session.charid or not self or self.destroyed:
        return
    dogmaLM = self.godma.GetDogmaLM()
    attrDict = dogmaLM.GetCharacterBaseAttributes()
    unspentPts = 300
    for x in xrange(0, 5):
        attr = self.attributes[x]
        if attr in attrDict:
            attrValue = attrDict[attr]
            if attrValue > 65:
                attrValue = 65
            if attrValue < 45:
                attrValue = 45
            self.currentAttributes[attr] = attrValue
            self.respecBar[x].SetValue(attrValue - 45)
            unspentPts -= attrValue
        modifiers = self.skillHandler.GetCharacterAttributeModifiers(attr)
        implantBonus = 0
        for itemID, typeID, operation, value in modifiers:
            categoryID = cfg.invtypes.Get(typeID).categoryID
            if categoryID == const.categoryImplant:
                implantBonus += value

        totalAttributesText = localizationUtil.FormatNumeric(int(self.currentAttributes[attr]) + implantBonus, decimalPlaces=0)
        self.totalLabels[x].text = '<right>%s</right>' % totalAttributesText
        self.implantModifier[x] = implantBonus
        label, icon = self.implantLabels[x]
        if implantBonus == 0:
            icon.SetAlpha(0.5)
            label.text = localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/Attributes/ImplantBonusZero')
            label.SetAlpha(0.5)
        else:
            label.text = localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/Attributes/ImplantBonus', implantBonus=int(implantBonus))

    if not self.readOnly:
        self.unspentPts = unspentPts
        self.sr.unassignedBar.SetValue(unspentPts)
        unspentPtsText = localizationUtil.FormatNumeric(self.unspentPts, decimalPlaces=0)
        self.availableLabel.text = '<right>%s</right>' % unspentPtsText
        if self.unspentPts <= 0:
            self.sr.saveWarningText.state = uiconst.UI_HIDDEN
        else:
            self.sr.saveWarningText.state = uiconst.UI_DISABLED


