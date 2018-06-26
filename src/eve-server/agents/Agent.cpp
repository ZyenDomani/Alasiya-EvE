/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Zhur
*/

/** @todo  this needs updating.....  */


/*
 * # Agent Logging:
 * AGENT__ERROR
 * AGENT__WARNING
 * AGENT__MESSAGE
 * AGENT__DEBUG
 * AGENT__INFO
 * AGENT__TRACE
 * AGENT__DUMP
 * AGENT__RSPDUMP
 */

#include "eve-server.h"

#include "StaticDataMgr.h"
#include "agents/Agent.h"
#include "agents/AgentDB.h"
#include "../../eve-common/EVE_Agent.h"

CorpAgent::CorpAgent(uint32 id)
: m_agentID(id),
m_locationID(0)
{
    _log(AGENT__TRACE, "CorpAgent created for AgentID %u", id);
}

CorpAgent::~CorpAgent() {
    /*
    std::map<uint32, AgentActions *>::iterator cur = m_actions.begin();
    for(; cur != m_actions.end(); cur++) {
        delete cur->second;
    }
    */
}

bool CorpAgent::Load(AgentDB *from) {
    from->LoadAgentData(m_agentID, m_data);

    bool ret = from->LoadAgentLocation(m_agentID, m_locationID, m_locationType);

    _log(AGENT__TRACE, "Data Loaded for CorpAgent %u - level: %u, stationID: %u, systemID: %u", m_agentID, m_data.level, m_data.stationID, m_data.solarSystemID);

    return ret;
}


PyDict* CorpAgent::GetLocationWrap() {
    PyDict *res = new PyDict();
    res->SetItemString("typeID", new PyInt(m_locationType) );
    res->SetItemString("locationID", new PyInt(m_locationID) );
    res->SetItemString("solarsystemID", new PyInt(m_data.solarSystemID) );
    return res;

    /* other location data types to put in dict for agents in space
     * locationType
     * coords
     * referringAgentID
     * shipTypeID
     *
     */

    /*
     * def LocationWrapper(location, locationType = None):
     *    if locationType is None and 'locationType' in location:
     *        locationType = location['locationType']
     *    pseudoSecurityRating = cfg.solarsystems.Get(location['solarsystemID']).pseudoSecurity
     *    if pseudoSecurityRating <= 0:
     *        securityKey = '0.0'
     *    else:
     *        securityKey = str(round(pseudoSecurityRating, 1))
     *    secColor = SECURITY_COLORS[securityKey]
     *    secColorAsHtml = '#%02X%02X%02X' % (secColor[0], secColor[1], secColor[2])
     *    secWarning = '<font color=#E3170D>'
     *    secClass = util.SecurityClassFromLevel(pseudoSecurityRating)
     *    standingSvc = sm.GetService('standing')
     *    if secClass <= const.securityClassLowSec:
     *        secWarning += localization.GetByLabel('UI/Agents/LowSecWarning')
     *    elif standingSvc.GetMySecurityRating() <= -5:
     *        secWarning += localization.GetByLabel('UI/Agents/HighSecWarning')
     *    secWarning += '</font>'
     *    if 'coords' in location:
     *        x, y, z = location['coords']
     *        refAgentString = str(location['agentID'])
     *        if 'referringAgentID' in location:
     *            refAgentString += ',' + str(location['referringAgentID'])
     *        infoLinkData = ['showinfo',
     *         location['typeID'],
     *         location['locationID'],
     *         x,
     *         y,
     *         z,
     *         refAgentString,
     *         0,
     *         locationType]
     *    else:
     *        infoLinkData = ['showinfo', location['typeID'], location['locationID']]
     *    spacePigShipType = location.get('shipTypeID', None)
     *    if spacePigShipType is not None:
     *        locationName = localization.GetByLabel('UI/Agents/Items/ItemLocation', typeID=spacePigShipType, locationID=location['locationID'])
     *    else:
     *        locationName = cfg.evelocations.Get(location['locationID']).locationName
     *    return localization.GetByLabel('UI/Agents/LocationWrapper', startFontTag='<font color=%s>' % secColorAsHtml, endFontTag='</font>', securityRating=pseudoSecurityRating, locationName=locationName, linkdata=infoLinkData, securityWarning=secWarning)
     */
}

PyObject* CorpAgent::GetInfoServiceDetails()
{
    PyDict* res = new PyDict();
    res->SetItemString("stationID", new PyInt(m_data.stationID) );
    res->SetItemString("level", new PyInt(m_data.level) );

    //res->SetItemString("services", tuple);
    // 'services' is a tuple of dicts containing data for [research], [locate], and [mission] services this agent offers

    /*  for research agents....
     *  skillTypeID, skillLevel in data.skills:
     * researchData = data.researchData
     * researchData['rpMultiplier']
     * researchData['skillTypeID']
     * researchData['points']  -- current points
     * researchData['pointsPerDay']
     * skillTypeID, blueprintTypeID in data.researchSummary:  -- for predictablePatentNames
     */

    PyTuple* skill1 = new PyTuple(2);
        skill1->SetItem(0, new PyInt(11452)); // Mechanical Engineering
        skill1->SetItem(1, new PyInt(3));
    PyTuple* skill2 = new PyTuple(2);
        skill2->SetItem(0, new PyInt(11453));  //Electronic Engineering
        skill2->SetItem(1, new PyInt(4));
    PyList* skillList = new PyList();
        skillList->AddItem(skill1);
        skillList->AddItem(skill2);
    PyDict* researchData = new PyDict();
        researchData->SetItemString("rpMultiplier", new PyInt(0));
        researchData->SetItemString("skillTypeID", new PyInt(0));
        researchData->SetItemString("points", new PyInt(0));
        researchData->SetItemString("pointsPerDay", new PyInt(0));
    PyTuple* patent1 = new PyTuple(2);
        patent1->SetItem(0, new PyInt(11452));
        patent1->SetItem(1, new PyInt(692));
    PyTuple* patent2 = new PyTuple(2);
        patent2->SetItem(0, new PyInt(11453));
        patent2->SetItem(1, new PyInt(1196));
    PyList* patentList = new PyList();
        patentList->AddItem(patent1);
        patentList->AddItem(patent2);
    PyDict* research = new PyDict();
        research->SetItemString("agentServiceType", new PyString("research"));
        research->SetItemString("skills", skillList);
        research->SetItemString("researchSummary", patentList);
        research->SetItemString("researchData", researchData);

    /* for location agents....
     * data.frequency
     *delayRange, delay, cost in data.delays:       -- (tuple) range (system, const, region, other region), responseTime (in sec), cost
     *   data.callbackID  -- bool for agent locator services being used (locator unavailable)
     *      OR
     *   data.lastUsed  -- blue time?
     */
    PyTuple* sameSystem = new PyTuple(3);
        sameSystem->SetItem(0, new PyInt(0));
        sameSystem->SetItem(1, new PyInt(10));
        sameSystem->SetItem(2, new PyInt(20000));

    PyTuple* sameConst = new PyTuple(3);
        sameConst->SetItem(0, new PyInt(1));
        sameConst->SetItem(1, new PyInt(30));
        sameConst->SetItem(2, new PyInt(200000));

    PyTuple* sameRegion = new PyTuple(3);
        sameRegion->SetItem(0, new PyInt(2));
        sameRegion->SetItem(1, new PyInt(60));
        sameRegion->SetItem(2, new PyInt(2000000));

    PyTuple* otherRegion = new PyTuple(3);
        otherRegion->SetItem(0, new PyInt(3));
        otherRegion->SetItem(1, new PyInt(120));
        otherRegion->SetItem(2, new PyInt(20000000));

    PyTuple* delays = new PyTuple(4);
        delays->SetItem(0, sameSystem);
        delays->SetItem(1, sameConst);
        delays->SetItem(2, sameRegion);
        delays->SetItem(3, otherRegion);
    PyDict* locate = new PyDict();
        locate->SetItemString("agentServiceType", new PyString("locate"));
        locate->SetItemString("delays", delays);
        locate->SetItemString("callbackID", new PyNone());
        locate->SetItemString("lastUsed", new PyInt(0));

    // for mission agents....
    PyDict* mission = new PyDict();
        mission->SetItemString("agentServiceType", new PyString("mission"));
        mission->SetItemString("available", new PyBool(true));

    PyTuple* tuple = new PyTuple(3);
        tuple->SetItem(0, new PyObject("util.KeyVal", research));
        tuple->SetItem(1, new PyObject("util.KeyVal", locate));
        tuple->SetItem(2, new PyObject("util.KeyVal", mission));
    res->SetItemString("services", tuple);

    // not sure when/why this is used yet....
    res->SetItemString("incompatible", new PyString("This Agent is Incompatable with incompatable shit from a compatibility standpoint.") );
    /* can also use locale labelIDs for this using a tuple...
     * tuple* tuple = new PyTuple();
     * tuple->SetItem(0, #labelID );
     * tuple->SetItem(1, #labelText);
     * res->SetItemString("incompatible", tuple);
     */

    if (is_log_enabled(AGENT__RSPDUMP)) {
        sLog.White( "CorpAgent::GetInfoServiceDetails()", "Dump:" );
        res->Dump(AGENT__RSPDUMP, "    ");
    }

    return new PyObject("util.KeyVal", res);

    /*
     * 09:29:06 [SvcCall] Service AgentBound::GetInfoServiceDetails()
     * 09:29:06 [AgentRspDump]      Dictionary: 4 entries
     * 09:29:06 [AgentRspDump]       [ 0]   Key:     String: 'services'
     * 09:29:06 [AgentRspDump]       [ 0] Value:  Tuple: 3 elements
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0] Object:
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Type:     String: 'util.KeyVal'
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:  Dictionary: 4 entries
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 0]   Key:     String: 'researchData'
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 0] Value:  Dictionary: 4 entries
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 0] Value:   [ 0]   Key:     String: 'pointsPerDay'
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 0] Value:   [ 0] Value:    Integer: 0
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 0] Value:   [ 1]   Key:     String: 'points'
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 0] Value:   [ 1] Value:    Integer: 0
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 0] Value:   [ 2]   Key:     String: 'skillTypeID'
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 0] Value:   [ 2] Value:    Integer: 0
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 0] Value:   [ 3]   Key:     String: 'rpMultiplier'
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 0] Value:   [ 3] Value:    Integer: 0
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 1]   Key:     String: 'researchSummary'
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 1] Value:   List: 2 elements
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 1] Value:   [ 0]  Tuple: 2 elements
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 1] Value:   [ 0]   [ 0]    Integer: 11452
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 1] Value:   [ 0]   [ 1]    Integer: 692
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 1] Value:   [ 1]  Tuple: 2 elements
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 1] Value:   [ 1]   [ 0]    Integer: 11453
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 1] Value:   [ 1]   [ 1]    Integer: 1196
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 2]   Key:     String: 'skills'
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 2] Value:   List: 2 elements
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 2] Value:   [ 0]  Tuple: 2 elements
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 2] Value:   [ 0]   [ 0]    Integer: 11452
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 2] Value:   [ 0]   [ 1]    Integer: 3
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 2] Value:   [ 1]  Tuple: 2 elements
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 2] Value:   [ 1]   [ 0]    Integer: 11453
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 2] Value:   [ 1]   [ 1]    Integer: 4
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 3]   Key:     String: 'agentServiceType'
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 0]   Args:   [ 3] Value:     String: 'research'
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1] Object:
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Type:     String: 'util.KeyVal'
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:  Dictionary: 4 entries
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 0]   Key:     String: 'lastUsed'
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 0] Value:    Integer: 0
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 1]   Key:     String: 'callbackID'
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 1] Value:       None
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 2]   Key:     String: 'delays'
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 2] Value:  Tuple: 4 elements
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 2] Value:   [ 0]  Tuple: 3 elements
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 2] Value:   [ 0]   [ 0]    Integer: 0
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 2] Value:   [ 0]   [ 1]    Integer: 10
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 2] Value:   [ 0]   [ 2]    Integer: 20000
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 2] Value:   [ 1]  Tuple: 3 elements
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 2] Value:   [ 1]   [ 0]    Integer: 1
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 2] Value:   [ 1]   [ 1]    Integer: 30
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 2] Value:   [ 1]   [ 2]    Integer: 200000
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 2] Value:   [ 2]  Tuple: 3 elements
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 2] Value:   [ 2]   [ 0]    Integer: 2
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 2] Value:   [ 2]   [ 1]    Integer: 60
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 2] Value:   [ 2]   [ 2]    Integer: 2000000
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 2] Value:   [ 3]  Tuple: 3 elements
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 2] Value:   [ 3]   [ 0]    Integer: 3
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 2] Value:   [ 3]   [ 1]    Integer: 120
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 2] Value:   [ 3]   [ 2]    Integer: 20000000
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 3]   Key:     String: 'agentServiceType'
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 1]   Args:   [ 3] Value:     String: 'locate'
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 2] Object:
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 2]   Type:     String: 'util.KeyVal'
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 2]   Args:  Dictionary: 2 entries
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 2]   Args:   [ 0]   Key:     String: 'available'
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 2]   Args:   [ 0] Value:    Boolean: true
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 2]   Args:   [ 1]   Key:     String: 'agentServiceType'
     * 09:29:06 [AgentRspDump]       [ 0] Value:   [ 2]   Args:   [ 1] Value:     String: 'mission'
     * 09:29:06 [AgentRspDump]       [ 1]   Key:     String: 'level'
     * 09:29:06 [AgentRspDump]       [ 1] Value:    Integer: 2
     * 09:29:06 [AgentRspDump]       [ 2]   Key:     String: 'incompatible'
     * 09:29:06 [AgentRspDump]       [ 2] Value:     String: 'This Agent is Incompatable with incompatable shit from a compatibility standpoint.'
     * 09:29:06 [AgentRspDump]       [ 3]   Key:     String: 'stationID'
     * 09:29:06 [AgentRspDump]       [ 3] Value:    Integer: 60005728
     * 09:29:06 [SvcCall] Service stationSvc::GetStation()
     * 09:29:06 [SvcCall] Service alert::BeanCount()
     * 09:29:06 [SvcCall] Service alert::SendClientStackTraceAlert()
     * EXCEPTION #5 logged at  06/26/2018 9:29:06 Unhandled exception in <TaskletExt object at 48efcc90, abps=1001, ctxt=None>
     * Caught at:
     * /common/lib/bluepy.py(98) CallWrapper
     * Thrown at:
     * /common/lib/bluepy.py(86) CallWrapper
     * /client/script/ui/services/infosvc.py(3231) LoadData
     * /client/script/ui/services/infosvc.py(3658) _LoadInfoWindow
     * /client/script/ui/services/infosvc.py(1637) GetWndData
     * /client/script/ui/station/agents/agents.py(1215) ProcessAgentInfoKeyVal
     * /client/script/ui/station/agents/agents.py(1259) _ProcessResearchServiceInfo
     *        status = [(...)]
     *        skillTypeID = 11452
     *        pattentIDs = 692
     *        skills = u'Electronic Engineering level 4, Mechanical Engineering level 3'
     *        skillList = [u'Electronic Engineering level 4', u'Mechanical Engineering level 3']
     *        researchData = {'points': 0, 'pointsPerDay': 0, 'rpMultiplier': 0, 'skillTypeID': 0}
     *        skillLevel = 4
     *        research = []
     *        header = u'Research Services'
     *        researchStuff = [(...), (...), (...)]
     *        details = [(...)]
     *        predictablePatentNames = []
     *        rpMultiplier = 0
     *        data = <Anonymous KeyVal: {'skills': [(11452, 3), (11453, 4)], 'agentServiceType': 'research', 'researchData': {'pointsPerDay': 0, 'skillTypeID': 0, 'points': 0, 'rpMultiplier': 0}, 'researchSummary': [(11452, 692), (11453, 1196)]}>
     *        self = <svc.Agents instance at 0x48A89CD8>
     * TypeError: 'int' object is not iterable
     *
     *
     */

    /*
     *                details = sm.GetService('agents').GetAgentMoniker(agentID).GetInfoServiceDetails()
     *                stationinfo = sm.RemoteSvc('stationSvc').GetStation(details.stationID):
     *                level = localizationUtil.FormatNumeric(details.level, decimalPlaces=0)
     *                for data in details.services:
     *                    serviceInfo = sm.GetService('agents').ProcessAgentInfoKeyVal(data)
     *                    for entry in serviceInfo:
     *                        wnd.sr.data[C_AGENTINFOTAB]['items'].append(listentry.Get('Header', {'label': entry[0]}))
     *                        for entryDetails in entry[1]:
     *                            wnd.sr.data[C_AGENTINFOTAB]['items'].append(listentry.Get('LabelTextTop', {'line': 1,
     *                            'label': entryDetails[0],
     *                            'text': entryDetails[1]}))
     *
     *                if details.incompatible:
     *                    if type(details.incompatible) is tuple:
     *                        incText = localization.GetByLabel(details.incompatible[0], **details.incompatible[1])
     *                    else:
     *                        incText = details.incompatible
     *                    wnd.sr.data[C_AGENTINFOTAB]['items'].append(listentry.Get('LabelTextTop', {'line': 1,
     *                        'label': localization.GetByLabel('UI/InfoWindow/AgentCompatibility'),
     *                        'text': incText}))
     */

    /*  data.services as follows....
     *
     *    def ProcessAgentInfoKeyVal(self, data):
     *        infoFunc = {'research': self._ProcessResearchServiceInfo,
     *         'locate': self._ProcessLocateServiceInfo,
     *         'mission': self._ProcessMissionServiceInfo}.get(data.agentServiceType, None)
     *        if infoFunc:
     *            return infoFunc(data)
     *        else:
     *            return []
     *
     *    def _ProcessResearchServiceInfo(self, data):
     *        header = localization.GetByLabel('UI/Agents/Research/ResearchServices', session.languageID)
     *        skillList = []
     *        for skillTypeID, skillLevel in data.skills:
     *            skillList.append(localization.GetByLabel('UI/Agents/Research/SkillListing', session.languageID, skillID=skillTypeID, skillLevel=skillLevel))
     *
     *        if not skillList:
     *            skills = localization.GetByLabel('UI/Agents/Research/ErrorNoRelevantResearchSkills', session.languageID)
     *        else:
     *            skillList = localizationUtil.Sort(skillList)
     *            skills = localizationUtil.FormatGenericList(skillList)
     *        details = [(localization.GetByLabel('UI/Agents/Research/RelevantSkills', session.languageID), skills)]
     *        status = []
     *        if data.researchData:
     *            researchData = data.researchData
     *            researchStuff = [(localization.GetByLabel('UI/Agents/Research/ResearchField', session.languageID), cfg.invtypes.Get(researchData['skillTypeID']).name), (localization.GetByLabel('UI/Agents/Research/CurrentStatus', session.languageID), localization.GetByLabel('UI/Agents/Research/CurrentStatusRP', session.languageID, rpAmount=researchData['points'])), (localization.GetByLabel('UI/Agents/Research/ResearchRate', session.languageID), localization.GetByLabel('UI/Agents/Research/ResearchRateRPDay', session.languageID, rpAmount=researchData['pointsPerDay']))]
     *            rpMultiplier = researchData['rpMultiplier']
     *            if rpMultiplier > 1:
     *                researchStuff.append((localization.GetByLabel('UI/Agents/Research/ResearchFieldBonus', session.languageID), localization.GetByLabel('UI/Agents/Research/ResearchFieldBonusRPMultiplier', session.languageID, rpMultiplier=rpMultiplier)))
     *            status = [(localization.GetByLabel('UI/Agents/Research/YourResearch', session.languageID), researchStuff)]
     *        research = []
     *        for skillTypeID, pattentIDs in data.researchSummary:
     *            predictablePatentNames = []
     *            for blueprintTypeID in pattentIDs:
     *                predictablePatentNames.append(cfg.invtypes.Get(blueprintTypeID).name)
     *
     *            if predictablePatentNames:
     *                predictablePatentNames = localizationUtil.Sort(predictablePatentNames)
     *                predictablePatents = localizationUtil.FormatGenericList(predictablePatentNames)
     *            else:
     *                predictablePatents = localization.GetByLabel('UI/Agents/Research/NoPredictablePatents', session.languageID)
     *            research.append((localization.GetByLabel('UI/Agents/Research/ResearchSummary', session.languageID, skillTypeID=skillTypeID), [(localization.GetByLabel('UI/Agents/Research/PredictablePatents', session.languageID), predictablePatents)]))
     *
     *        return [(header, details)] + status + research
     *
     *    def _ProcessLocateServiceInfo(self, data):
     *        header = localization.GetByLabel('UI/Agents/Locator/LocationServices', session.languageID)
     *        if data.frequency:
     *            details = [(localization.GetByLabel('UI/Agents/Locator/MaxFrequency', session.languageID), localization.GetByLabel('UI/Agents/Locator/EveryInterval', session.languageID, interval=data.frequency))]
     *        else:
     *            details = [(localization.GetByLabel('UI/Agents/Locator/MaxFrequency', session.languageID), localization.GetByLabel('UI/Generic/NotAvailableShort', session.languageID))]
     *        for delayRange, delay, cost in data.delays:
     *            rangeText = [localization.GetByLabel('UI/Agents/Locator/SameSolarSystem', session.languageID),
     *             localization.GetByLabel('UI/Agents/Locator/SameConstellation', session.languageID),
     *             localization.GetByLabel('UI/Agents/Locator/SameRegion', session.languageID),
     *             localization.GetByLabel('UI/Agents/Locator/DifferentRegion', session.languageID)][delayRange]
     *            if not delay:
     *                delay = localization.GetByLabel('UI/Agents/Locator/ResultsInstantaneous', session.languageID)
     *            else:
     *                delay = util.FmtTimeInterval(delay * SEC)
     *            details.append((rangeText, localizationUtil.FormatGenericList((util.FmtISK(cost), delay))))
     *
     *        if data.callbackID:
     *            details.append((localization.GetByLabel('UI/Agents/Locator/Availability', session.languageID), localization.GetByLabel('UI/Agents/Locator/NotAvailableInProgress', session.languageID)))
     *        elif data.lastUsed and blue.os.GetWallclockTime() - data.lastUsed < data.frequency:
     *            details.append((localization.GetByLabel('UI/Agents/Locator/AvailableAgain', session.languageID), util.FmtDate(data.lastUsed + data.frequency)))
     *        return [(header, details)]
     *
     *    def _ProcessMissionServiceInfo(self, data):
     *        if data.available:
     *            return [(localization.GetByLabel('UI/Agents/MissionServices', session.languageID), [(localization.GetByLabel('UI/Agents/MissionAvailability', session.languageID), localization.GetByLabel('UI/Agents/MissionAvailabilityStandard', session.languageID))])]
     *        else:
     *            return [(localization.GetByLabel('UI/Agents/MissionServices', session.languageID), [(localization.GetByLabel('UI/Agents/MissionAvailability', session.languageID), localization.GetByLabel('UI/Agents/MissionAvailabilityNone', session.languageID))])]
     */
}

uint32 CorpAgent::GetLoyaltyPoints(Client *who) {
    codelog(AGENT__ERROR, "Unimplemented.");
    return(0);
}

/* It seems as though actionIDs are dynamically assigned at runtime, as they
 * always appear to be sequential, and repeat visits to the same agent will
 * yield different actionIDs for seemingly the same action. No idea if there is
 * any perceived benefit to actually doing that for our case... it could be a
 * mechanism for them to track event ordering (to make sure people do not do
 * missions out of order by only accepting the actions which were actually
 * "allocated" previously.)
 */

        /*
              [PyTuple 2 items]
                [PyInt 592]
                [PyString "[Button]View Mission"]
              [PyTuple 2 items]
                [PyInt 596]
                [PyString "[Button]Request Mission"]
              [PyTuple 2 items]
                [PyInt 597]
                [PyString "[Button]Locate Character"]
              [PyTuple 2 items]
                [PyInt 598]
                [PyString "[Button]Accept"]
              [PyTuple 2 items]
                [PyInt 599]
                [PyString "[Button]Decline"]
              [PyTuple 2 items]
                [PyInt 600]
                [PyString "[Button][CloseOnClick]Delay"]
              [PyTuple 2 items]
                [PyInt 601]
                [PyString "[Button]Complete Mission"]
              [PyTuple 2 items]
                [PyInt 602]
                [PyString "[Button]Quit Mission"]
                */
void CorpAgent::DoAction(
    Client *who, uint32 actionID,
    std::string &say, std::map<uint32, std::string> &choices
) {
    /*
    if(actionID == 0) {
        //default dialog...
        choices[] = "I need something to do.";    //TODO: randomize this like they do on live

    } else {

    }

    std::map<uint32, AgentActions *>::iterator res;
    res = m_actions.find(actionID);
    if(res == m_actions.end()) {
        _log(AGENT__ERROR, "Agent %d: Unable to find action %u for '%s'", m_agentID, actionID, c->GetName());
        say = "Invalid Action";
        return;
    }
    AgentActions *action = res->second;

    say = action->agentSays;
    choices = action->actions;
    loyaltyPoints = action->loyaltyPoints;
    */

    char v[256];
    sprintf(v, "Result of DoAction(%d)", actionID);
    say = v;  //"What do you want? Spit it out, stooge. ";
    //request mission = 2
    choices[Dialog::Button::RequestMission] = "I want work, do you have anything?";
    //locate char = 15
    choices[Dialog::Button::LocateCharacter] = "I need to find somebody.  Can you help me?";
    //start research = 12
    choices[Dialog::Button::StartResearch] = "Start Research";
}
