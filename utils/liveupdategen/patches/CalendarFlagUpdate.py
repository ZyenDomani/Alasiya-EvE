#@liveupdate("globalClassMethod", "svc.eveCalendar::GetEventFlag", "GetEventFlag")
def GetEventFlag(self, ownerID, autoEventType = None):
    if autoEventType is not None:
        return const.calendarTagAutomated
    if ownerID == session.corpid:
        return const.calendarTagCorp
    elif ownerID == session.allianceid:
        return const.calendarTagAlliance
    elif ownerID == const.ownerSystem:
        return const.calendarTagCCP
    else:
        return const.calendarTagPersonal
