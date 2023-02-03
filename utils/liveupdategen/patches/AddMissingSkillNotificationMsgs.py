#@liveupdate("globalClassMethod", "svc.charactersheet::CharacterSheet", "ShowMySkillHistory")
def ShowMySkillHistory(self):
    wnd = self.GetWnd()
    if not wnd:
        return

    def GetPts(lvl):
        return skillUtil.GetSPForLevelRaw(stc, lvl)

    wnd.sr.nav.DeselectAll()
    wnd.sr.scroll.sr.id = 'charsheet_skillhistory'
    wnd.sr.scroll.state = uiconst.UI_PICKCHILDREN
    rs = sm.GetService('skills').GetSkillHistory()
    scrolllist = []
    actions = {24: localization.GetByLabel('UI/RedeemWindow/RedeamMessages/GIFTFROMCCP'),
        33: localization.GetByLabel('UI/PeopleAndPlaces/Creator'),
        34: localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/SkillTabs/SkillClonePenalty'),
        35: localization.GetByLabel('UI/Common/Updated'),
        36: localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/SkillTabs/SkillTrainingStarted'),
        37: localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/SkillTabs/SkillTrainingComplete'),
        38: localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/SkillTabs/SkillTrainingCanceled'),
        39: localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/SkillTabs/GMGiveSkill'),
        53: localization.GetByLabel('UI/Generic/SkillTrainingComplete'),
        56: localization.GetByLabel('UI/SkillQueue/InjectSkill'),
        177: localization.GetByLabel('UI/PI/Common/Remove'),
        260: localization.GetByLabel('UI/Inflight/Brackets/TargetLocked'),
        307: localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/SkillTabs/SkillPointsApplied')}
    for r in rs:
        skill = sm.GetService('skills').HasSkill(r.skillTypeID)
        if skill:
            stc = skill.skillTimeConstant
            levels = [0,
                GetPts(1),
                GetPts(2),
                GetPts(3),
                GetPts(4),
                GetPts(5)]
            level = 5
            spNext = levels[5]
            for i in range(len(levels)):
                if levels[i] > r.absolutePoints:
                    level = i - 1
                    spNext = levels[i]
                    break

            data = util.KeyVal()
            data.label = util.FmtDate(r.logDate, 'ls') + '<t>'
            data.label += cfg.invtypes.Get(r.skillTypeID).name + '<t>'
            data.label += actions.get(r.eventTypeID, localization.GetByLabel('UI/Generic/Unknown')) + '<t>'
            data.label += localizationUtil.FormatNumeric(level)
            data.Set('sort_%s' % localization.GetByLabel('UI/Common/Date'), r.logDate)
            data.id = r.skillTypeID
            data.GetMenu = self.GetItemMenu
            data.MenuFunction = self.GetItemMenu
            data.OnDblClick = (self.DblClickShowInfo, data)
            addItem = listentry.Get('Generic', data=data)
            scrolllist.append(addItem)

    wnd.sr.scroll.Load(contentList=scrolllist, headers=[localization.GetByLabel('UI/Common/Date'),
        localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/SkillTabs/Skill'),
        localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/SkillTabs/Action'),
        localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/SkillTabs/Level')],
        noContentHint=localization.GetByLabel('UI/CharacterSheet/CharacterSheetWindow/SkillTabs/NoRecordsFound'),
        reversesort=True)
