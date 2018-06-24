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


#include "eve-server.h"

#include "StaticDataMgr.h"
#include "agents/Agent.h"
#include "agents/AgentDB.h"

Agent::Agent(uint32 id)
: m_agentID(id),
m_locationID(0)
{
}

Agent::~Agent() {
    /*
    std::map<uint32, AgentActions *>::iterator cur = m_actions.begin();
    for(; cur != m_actions.end(); cur++) {
        delete cur->second;
    }
    */
}

bool Agent::Load(AgentDB *from) {
    bool ret = from->LoadAgentLocation(m_agentID, m_locationID, m_locationType);
    if (IsStation(m_locationID))
        m_solarSystemID = sDataMgr.GetStationSystem(m_locationID);
    else
        m_solarSystemID = m_locationID;
    return ret;
}


PyRep* Agent::GetLocation() {
    PyDict *res = new PyDict();
    res->SetItemString("typeID", new PyInt(m_locationType) );
    res->SetItemString("locationID", new PyInt(m_locationID) );
    res->SetItemString("solarsystemID", new PyInt(m_solarSystemID) );
    return res;
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

uint32 Agent::GetLoyaltyPoints(Client *who) {
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
void Agent::DoAction(
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
    choices[2] = "I want work, do you have anything?";
    //locate char = 15
    choices[15] = "I need to find somebody.  Can you help me?";
}
