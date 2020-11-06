#@liveupdate("globalClassMethod", "xtriui.InstallationPanel::InstallationPanel", "GetAssemblyLineDetailsScrolllist")
def GetAssemblyLineDetailsScrolllist(self, installation, onclick = None, ondblclick = None, ongetmenu = None, filterActivity = None, filterAssemblyLineType = None):
    scrolllist = []
    lastRow = []
    lastRowData = util.KeyVal()
    lastLine = None
    numLines = 0
    rows = []
    nextFreeTimeLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/NextFreeTime')
    for line in installation:
        thisRow = [line.assemblyLineTypeID,
         line.costInstall,
         line.costPerHour,
         line.restrictionMask]
        lineType = cfg.ramaltypes.Get(line.assemblyLineTypeID)
        if filterActivity and lineType.activityID != filterActivity:
            continue
        if filterAssemblyLineType and lineType.assemblyLineTypeID != filterAssemblyLineType:
            continue
        if thisRow[3] and self.IsAssemblyLineFilteredOut(thisRow):
            continue
        numLines += 1
        lineGroup = cfg.ramaltypesdetailpergroup.get(line.assemblyLineTypeID, [])
        lineCategory = cfg.ramaltypesdetailpercategory.get(line.assemblyLineTypeID, [])
        expirationTime = line.nextFreeTime
        expirationTime -= blue.os.GetWallclockTime()
        expirationTimeSort = int(expirationTime / MIN) * MIN
        data = util.KeyVal()
        data.confirmOnDblClick = 1
        data.Set('sort_%s' % nextFreeTimeLabel, expirationTimeSort)
        data.listvalue = (line,
         lineType,
         lineGroup,
         lineCategory)
        data.OnClick = onclick
        data.OnDblClick = ondblclick
        data.GetMenu = ongetmenu
        data.assemblyLine = line
        data.numLines = numLines
        if self.showAllAssemblyLines:
            scrolllist.append(listentry.Get('Generic', data=self.AssemblyLineRow(data)))
        else:
            for i in range(len(rows)):
                r = rows[i]
                if r.assemblyLine.assemblyLineTypeID == thisRow[0] and r.assemblyLine.costInstall == thisRow[1] and r.assemblyLine.costPerHour == thisRow[2]:
                    if expirationTimeSort < r.Get('sort_%s' % nextFreeTimeLabel, 0):
                        r.Set('sort_%s' % nextFreeTimeLabel, expirationTimeSort)
                        r.assemblyLine = line
                        r.listvalue = (line,
                         lineType,
                         lineGroup,
                         lineCategory)
                    rows[i].numLines += 1
                    break
            else:
                rows.append(data)

            numLines = 0

    if not self.showAllAssemblyLines and rows:
        for data in rows:
            scrolllist.append(listentry.Get('Generic', data=self.AssemblyLineRow(data)))

    numberHashLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/NumberHash')
    activityLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/Activity')
    installCostLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/InstallCost')
    costPerHour = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/CostPerHour')
    timeMultiplierLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/TimeMultiplier')
    materialMultiplierLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/MaterialMultiplier')
    availabilityLabel = localization.GetByLabel('UI/ScienceAndIndustry/ScienceAndIndustryWindow/Availability')
    tmpHeaders = [numberHashLabel,
     activityLabel,
     nextFreeTimeLabel,
     installCostLabel,
     costPerHour,
     timeMultiplierLabel,
     materialMultiplierLabel,
     availabilityLabel]
    headers = []
    for header in tmpHeaders:
        h = uiutil.ReplaceStringWithTags(header)
        headers.append(h)

    return (scrolllist, headers)
