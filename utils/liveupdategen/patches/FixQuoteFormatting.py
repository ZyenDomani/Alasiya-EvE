#@liveupdate("globalClassMethod", "xtriui.QuotePanel::QuotePanel", "LoadQuote")
def LoadQuote(self, *args):
    uix.Flush(self.sr.quoteDetails)
    self.sr.quoteScroll.Load(contentList=[], headers=[])
    self.sr.quoteScroll.ShowHint('Fetching quote...')
    nameLabel = localization.GetByLabel('UI/Common/Name')
    requiredLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/Required')
    missingLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/Missing')
    dmgJobLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/DamagePerJob')
    wasteLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/Waste')
    scrolllist = []
    bom = []
    headers = [nameLabel,
                   requiredLabel,
                   missingLabel,
                   dmgJobLabel,
                   wasteLabel]
    self.sr.quoteScroll.sr.fixedColumns = {nameLabel: 224,
                                           requiredLabel: 70,
                                           missingLabel: 70,
                                           dmgJobLabel: 60,
                                           wasteLabel: 60}
    if self.quote.bom:
        if len(self.quote.bom.rawMaterials):
            rawMaterialLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/RawMaterial')
            bom.append((rawMaterialLabel, self.quote.bom.rawMaterials))
        if len(self.quote.bom.extras):
            extraMaterialLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/ExtraMaterial')
            bom.append((extraMaterialLabel, self.quote.bom.extras))
        if len(self.quote.bom.skills):
            skillLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/Skill')
            bom.append((skillLabel, self.quote.bom.skills))
        for label, content in bom:
            id = ('BOM', label)
            uicore.registry.SetListGroupOpenState(id, 1)
            data = {'GetSubContent': self.GetBOMSubContent,
                    'label': label,
                    'groupItems': content,
                    'wasteMaterials': self.quote.bom.wasteMaterials,
                    'missingMaterials': self.quote.missingMaterials,
                    'id': id,
                    'tabs': [],
                    'showlen': False,
                    'posttext': self.GetMissingMaterialString(content, self.quote.missingMaterials),
                    'state': 'locked',
                    'showicon': 'hide',
                    'hideExpander': True,
                    'hideExpanderLine': True,
                    'disableToggle': True,
                    'BlockOpenWindow': True}
            scrolllist.append(listentry.Get('Group', data))

    self.sr.quoteScroll.Load(contentList=scrolllist, headers=headers)
    if scrolllist:
        self.sr.quoteScroll.ShowHint()
    else:
        noMaterialsRequiredLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/NoMaterialsRequired')
        self.sr.quoteScroll.ShowHint(noMaterialsRequiredLabel)
    uix.Flush(self.sr.quoteDetails)
    productionStartTimeLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/ProductionStartTime')
    productionTimeLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/ProductionTime')
    totalCostLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/TotalCost')
    installCostLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/InstallCost')
    usageCostLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/UsageCost')
    walletDivisionLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/WalletDivision')
    mtrlMultiAsmblyLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/MaterialMultiplierAssemblyItem')
    mtrlMultiSkillLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/MaterialMultiplierSkill')
    timeMultiAsmblyLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/TimeMultiplierAssemblyItem')
    timeMultiSkillLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/TimeMultiplierSkill')
    d = [(productionStartTimeLabel, 'maxJobStartTime'),
        (productionTimeLabel, 'productionTime'),
        (totalCostLabel, 'cost'),
        (installCostLabel, 'installCost'),
        (usageCostLabel, 'usageCost'),
        (walletDivisionLabel, 'accountKey'),
        (mtrlMultiAsmblyLabel, 'materialMultiplier'),
        (mtrlMultiSkillLabel, 'charMaterialMultiplier'),
        (timeMultiAsmblyLabel, 'timeMultiplier'),
        (timeMultiSkillLabel, 'charTimeMultiplier')]
    i = 0
    j = -1
    left = 0
    tabs = [130, 300]
    top = 4
    maxHeight = 0
    for header, key in d:
        if j == -1:
            top = 4
        if key not in self.quote.header:
            continue
        value = getattr(self.quote, key, '')
        if key in ('charTimeMultiplier','materialMultiplier'):
            value = localizationUtil.FormatNumeric(value, decimalPlaces=1)
        if key in ('cost', 'installCost', 'usageCost'):
            value = util.FmtISK(value)
        if key == 'accountKey':
            value = sm.GetService('corp').GetCorpAccountName(value)
        if key == 'productionTime':
            value = util.FmtDate(long(float(value) * 10000000L))
        if key == 'maxJobStartTime':
            if value is not None:
                value = value - blue.os.GetWallclockTime()
                if value < 0:
                    nowLabel = localization.GetByLabel('UI/Common/Now')
                    value = '<color=0xff00FF00>%s<color=0xffffffff>' % nowLabel
                else:
                    value = int(value / 600000000L) * 10000000L * 60
                    value = '<color=0xffFF0000>%s<color=0xffffffff>' % util.FmtDate(value)
        t = uicls.EveLabelMedium(text='%s<t><right>%s' % (header, value), parent=self.sr.quoteDetails, left=left, top=top, maxLines=None, tabs=tabs)
        t.hint = header
        top = top + t.height
        maxHeight = max(top, maxHeight)
        i += 1
        j += 1
        if i == 5:
            j = -1
            left += 300
            tabs = [220, 280]

    uicls.Line(parent=self.sr.quoteDetails, align=uiconst.RELATIVE, top=4, width=1, height=maxHeight, left=300)
    top = maxHeight + 8
    if len(self.quote.missingMaterials) > 0:
        materialOrSkillMissingLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/MaterialOrSkillMissing')
        t = uicls.EveLabelMedium(text='<color=0xffff0000>%s' % materialOrSkillMissingLabel, parent=self.sr.quoteDetails, left=6, top=top, maxLines=None)

    submit = uicls.Button(parent=self.sr.quoteDetails, label=self.submitHeader, func=self.Submit, args=None, pos=(0,
                                                                                                                top,
                                                                                                                0,
                                                                                                                0), align=uiconst.TOPRIGHT)
    refreshLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/RefreshCommand')
    refreshBtn = uicls.Button(parent=self.sr.quoteDetails, label=refreshLabel, func=self.Refresh, args=None, pos=(submit.width + 4,
                                                                                                                top,
                                                                                                                0,
                                                                                                                0), align=uiconst.TOPRIGHT)
    top = submit.height + top
    self.sr.quoteDetails.height = top + 6
    uicore.registry.SetFocus(submit)
