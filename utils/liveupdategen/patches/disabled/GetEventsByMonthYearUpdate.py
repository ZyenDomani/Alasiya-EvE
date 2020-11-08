#@liveupdate("globalClassMethod", "svc.eveCalendar::EveCalendarSvc", "GetEventsByMonthYear")
def GetEventsByMonthYear(self, month, year):
    eventList = self.events.get((month, year))
    if eventList is None:
        eventList = []
        dbRowList = self.GetCalendarProxy().GetEventList(month, year)
        for dbRows in dbRowList:
            if dbRows is not None:
                eventList.extend([ util.KeyVal(x) for x in dbRows ])

        self.events[month, year] = eventList
    return eventList
