#@liveupdate("globalClassMethod", "svc.eveCalendar::EveCalendarSvc", "GetEventFlag")
def GetEventFlag(self, ownerID, autoEventType = None):
    if autoEventType is not None:
        return const.calendarTagAutomated
    if ownerID == session.corpid:
        if autoEventType is None:
            return const.calendarTagCorp
        return const.calendarTagAutomated
    elif ownerID == session.allianceid:
        return const.calendarTagAlliance
    elif ownerID == const.ownerSystem:
        return const.calendarTagCCP
    else:
        return const.calendarTagPersonal
