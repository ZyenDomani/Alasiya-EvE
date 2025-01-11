#@liveupdate("globalClassMethod", "form.attributeRespecWindow::AttributeRespecWindow", "ConstructLayout")
def ConstructLayout(self):
    self.sr.topPar = uicls.Container(name='topPar', align=uiconst.TOTOP, parent=self.sr.main, height=64, top=0)
    headingPar = uicls.Container(name='headingPar', align=uiconst.TOBOTTOM, parent=self.sr.topPar, height=14, top=2)
    self.sr.topText = uicls.EveLabelMedium(text=localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/Attributes/CharacterRespecMessage'), parent=self.sr.topPar, left=9, width=485, maxLines=None, state=uiconst.UI_NORMAL, top=4)
    self.sr.topPar.height = 28 + self.sr.topText.textheight
    barColor = (0.5, 0.5, 0.5, 0.75)
    c = uicls.Container(name='', align=uiconst.TOTOP, parent=self.sr.main, height=2, top=0)
    uicls.Line(parent=c, align=uiconst.TOALL, left=10, width=10, height=1, color=barColor)
    self.sr.intPar = uicls.Container(name='intPar', align=uiconst.TOTOP, parent=self.sr.main, height=40)
    self.sr.memPar = uicls.Container(name='memPar', align=uiconst.TOTOP, parent=self.sr.main, height=40)
    self.sr.chaPar = uicls.Container(name='chaPar', align=uiconst.TOTOP, parent=self.sr.main, height=40)
    self.sr.perPar = uicls.Container(name='perPar', align=uiconst.TOTOP, parent=self.sr.main, height=40)
    self.sr.wilPar = uicls.Container(name='wilPar', align=uiconst.TOTOP, parent=self.sr.main, height=40)
    if not self.readOnly:
        c = uicls.Container(name='', align=uiconst.TOTOP, parent=self.sr.main, height=2, top=0)
        uicls.Line(parent=c, align=uiconst.TOALL, left=10, width=10, height=1, color=barColor)
        self.sr.bottomPar = uicls.Container(name='bottomPar', align=uiconst.TOTOP, parent=self.sr.main, top=8, height=65)
    else:
        self.sr.bottomPar = uicls.Container(name='bottomPar', align=uiconst.TOTOP, parent=self.sr.main, top=8, height=5)
    leftMargin = 8
    self.attributePars = [self.sr.intPar,
     self.sr.memPar,
     self.sr.chaPar,
     self.sr.perPar,
     self.sr.wilPar]
    self.sprites = []
    iconsize = 32
    for x in xrange(0, 5):
        icon = uicls.Icon(parent=self.attributePars[x], width=iconsize, height=iconsize, size=iconsize, icon=self.attributeIcons[x], left=leftMargin, align=uiconst.TOLEFT)

    leftMargin += iconsize + 3
    self.totalLabels = []
    maxTextWidth = 10
    attributeLabel = uicls.EveLabelMedium(text=localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/NavScroll/Attributes'), parent=headingPar, maxLines=1, left=leftMargin - iconsize / 2)
    for x in xrange(0, 5):
        label1 = uicls.EveLabelMedium(text=self.attributeLabels[x], parent=self.attributePars[x], maxLines=1, state=uiconst.UI_DISABLED, align=uiconst.CENTERLEFT, left=leftMargin)
        maxTextWidth = max(label1.textwidth, maxTextWidth)

    leftMargin += maxTextWidth + 20
    baseLabel = uicls.EveLabelMedium(text=localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/Attributes/BaseStatPoints'), parent=headingPar, left=leftMargin, maxLines=1)
    dogmaLM = self.godma.GetDogmaLM()
    attrDict = dogmaLM.GetCharacterBaseAttributes()
    for x in xrange(0, 5):
        attr = self.attributes[x]
        attrValue = attrDict[attr]
        minText = localizationUtil.FormatNumeric(attrValue, decimalPlaces=0)
        label2 = uicls.EveLabelMedium(text=minText, parent=self.attributePars[x], width=20, maxLines=1, state=uiconst.UI_DISABLED, left=leftMargin + baseLabel.textwidth / 2 - 10, top=10)
        label2.bold = 1

    leftMargin += baseLabel.textwidth + 15
    implantLabel = uicls.EveLabelMedium(text=localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/Attributes/CharacterImplants'), parent=headingPar, left=leftMargin, maxLines=1)
    self.implantLabels = []
    for x in xrange(0, 5):
        icon = uicls.Icon(parent=self.attributePars[x], width=32, height=32, size=32, icon=util.IconFile(cfg.invtypes.Get(self.implantTypes[x]).iconID), left=leftMargin - 4, align=uiconst.TOPLEFT, ignoreSize=True)
        label2 = uicls.EveLabelMedium(text='', parent=self.attributePars[x], maxLines=1, left=leftMargin + 20, top=10)
        self.implantLabels.append((label2, icon))

    boxWidth = 6
    boxHeight = 12
    boxMargin = 1
    boxSpacing = 1
    barHeight = boxHeight + 2 * boxMargin
    backgroundColor = (0.0, 0.0, 0.0, 0.0)
    leftMargin += implantLabel.textwidth + 10
    leftMargin += (415 - (leftMargin + 150)) / 2
    buttonSize = 22
    if not self.readOnly:
        for x in xrange(0, 5):
            minusText = localization.GetByLabel('UI/Common/Buttons/Minus')
            uicls.Button(parent=self.attributePars[x], label=minusText, fixedwidth=buttonSize, pos=(leftMargin,
             4,
             0,
             0), func=self.DecreaseAttribute, args=(x,))

    leftMargin += buttonSize + 4
    numBoxes = 20
    barWidth = numBoxes * boxSpacing + 2 * boxMargin + numBoxes * boxWidth - 1
    remappableLabel = uicls.EveLabelMedium(text=localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/Attributes/RemappableStat'), parent=headingPar, left=leftMargin, maxLines=1)
    remappableLabel.left = remappableLabel.left - remappableLabel.textwidth / 2 + barWidth / 2
    colorDict = {uicls.ClickableBoxBar.COLOR_UNSELECTED: (0.2, 0.2, 0.2, 1.0),
     uicls.ClickableBoxBar.COLOR_SELECTED: (0.2, 0.8, 0.2, 1.0)}
    self.respecBar = []
    for x in xrange(0, 5):
        bar = uicls.Container(parent=self.attributePars[x], align=uiconst.TOPLEFT, left=leftMargin, top=7, width=barWidth, height=barHeight)
        bar.state = uiconst.UI_PICKCHILDREN
        bar = uicls.ClickableBoxBar(parent=bar, numBoxes=numBoxes, boxWidth=boxWidth, boxHeight=boxHeight, boxMargin=boxMargin, boxSpacing=boxSpacing, backgroundColor=backgroundColor, colorDict=colorDict)
        bar.OnValueChanged = self.OnMemberBoxClick
        bar.OnAttemptBoxClicked = self.ValidateBoxClick
        self.respecBar.append(bar)

    leftMargin += barWidth + 4
    if not self.readOnly:
        for x in xrange(0, 5):
            plusText = localization.GetByLabel('UI/Common/Buttons/Plus')
            uicls.Button(parent=self.attributePars[x], label=plusText, fixedwidth=buttonSize, pos=(leftMargin,
             4,
             0,
             0), func=self.IncreaseAttribute, args=(x,))

    barEnd = leftMargin
    #leftMargin += buttonSize + 20
    leftMargin = 450
    totalLabel = uicls.EveLabelMedium(text=localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/Attributes/StatTotal'), parent=headingPar, left=leftMargin, maxLines=1)
    totalLabelPos = leftMargin + totalLabel.textwidth / 2 - 5
    for x in xrange(0, 5):
        label3 = uicls.EveLabelMedium(name='', parent=self.attributePars[x], width=20, maxLines=1, left=totalLabelPos, top=10)
        label3.bold = 1
        self.totalLabels.append(label3)

    if not self.readOnly:
        textObj = uicls.EveLabelMedium(text=localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/Attributes/UnassignedAttributePoints'), parent=self.sr.bottomPar, left=16)
        numBoxes = 250 - 40 * 5
        barWidth = 0 #numBoxes * boxSpacing + 2 * boxMargin + numBoxes * boxWidth - 1
        colorDict = {uicls.ClickableBoxBar.COLOR_UNSELECTED: (0.2, 0.2, 0.2, 1.0),
         uicls.ClickableBoxBar.COLOR_SELECTED: (0.2, 0.8, 0.2, 1.0)}
        self.sr.unassignedBar = uicls.Container(parent=self.sr.bottomPar, align=uiconst.TOPLEFT, left=barEnd - barWidth + 24, top=0, width=barWidth, height=barHeight)
        self.sr.unassignedBar.state = uiconst.UI_PICKCHILDREN
        self.sr.unassignedBar = uicls.ClickableBoxBar(parent=self.sr.unassignedBar, numBoxes=numBoxes, boxWidth=boxWidth, boxHeight=boxHeight, boxMargin=boxMargin, boxSpacing=boxSpacing, backgroundColor=backgroundColor, colorDict=colorDict, readonly=True, hintFormat='UI/CharacterSheet/CharacterSheetWindow/Attributes/UnassignedPointsHint')
        self.availableLabel = uicls.EveLabelMedium(text='', parent=self.sr.bottomPar, width=20, maxLines=1, left=totalLabelPos)
        self.sr.saveWarningText = uicls.EveLabelMedium(text=localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/Attributes/CannotSaveUnassignedPoints'), parent=self.sr.bottomPar, left=16, top=16, color=(1.0, 0.0, 0.0, 0.9))
        btns = uicls.ButtonGroup(btns=[[localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/Attributes/SaveStatChanges'),
         self.SaveChanges,
         (),
         None], [localization.GetByLabel('UI/Common/Buttons/Cancel'),
         self.CloseByUser,
         (),
         None]], parent=self.sr.main, idx=0)
    heightTotal = self.sr.topParent.height + self.sr.headerParent.height
    for child in self.sr.main.children:
        width, height = child.GetAbsoluteSize()
        heightTotal += height

    self.SetMinSize([self.GetMinWidth(), heightTotal], refresh=1)
