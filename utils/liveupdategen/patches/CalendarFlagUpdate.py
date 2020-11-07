#@liveupdate("globalClassMethod", "svc.eveCalendar::GetEventFlag", "GetEventFlag")
def GetEventFlag(self, ownerID, autoEventType = None):
    if ownerID == session.corpid:
        if autoEventType is None:
            return const.calendarTagCorp
        return const.calendarTagAutomated
    elif ownerID == session.allianceid:
        return const.calendarTagAlliance
    elif ownerID == const.ownerSystem:
        return const.calendarTagCCP
    elif autoEventType is None:
        return const.calendarTagPersonal
    else:
        return const.calendarTagAutomated
