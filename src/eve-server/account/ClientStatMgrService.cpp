/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
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
    Author:        Captnoord
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "account/ClientStatMgrService.h"

PyCallable_Make_InnerDispatcher(ClientStatsMgr)

ClientStatsMgr::ClientStatsMgr(PyServiceMgr *mgr)
: PyService(mgr, "clientStatsMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(ClientStatsMgr, SubmitStats);
}

ClientStatsMgr::~ClientStatsMgr()
{
    delete m_dispatch;
}

PyResult ClientStatsMgr::Handle_SubmitStats( PyCallArgs& call )
{
    sLog.Warning( "ClientStatsMgr", "Called SubmitStats stub." );
    //call.Dump(CLIENT__CALL_DUMP);

    return PyStatic.NewNone();
}

/*)
 * 18:23:27 [Service] clientStatsMgr::SubmitStats()
 * 18:23:27 W ClientStatsMgr: Called SubmitStats stub.
 * 18:23:27 [ClientCallDump]   Call Arguments:
 * 18:23:27 [ClientCallDump]      Tuple: 1 elements
 * 18:23:27 [ClientCallDump]       [ 0]  Tuple: 2 elements
 * 18:23:27 [ClientCallDump]       [ 0]   [ 0]  Tuple: 5 elements
 * 18:23:27 [ClientCallDump]       [ 0]   [ 0]   [ 0]    Integer: 2
 * 18:23:27 [ClientCallDump]       [ 0]   [ 0]   [ 1]    Integer: 1745
 * 18:23:27 [ClientCallDump]       [ 0]   [ 0]   [ 2]    Integer: 360229
 * 18:23:27 [ClientCallDump]       [ 0]   [ 0]   [ 3]    Integer: 1
 * 18:23:27 [ClientCallDump]       [ 0]   [ 0]   [ 4]    Integer: 2
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]  Dictionary: 8 entries
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 0]   Key:    Integer: 512
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 0] Value:  Dictionary: 3 entries
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 0] Value:   [ 0]   Key:    Integer: 5
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 0] Value:   [ 0] Value:    Integer: 345205
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 0] Value:   [ 1]   Key:    Integer: 2
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 0] Value:   [ 1] Value:       Long: 8742031250
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 0] Value:   [ 2]   Key:    Integer: 0
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 0] Value:   [ 2] Value:    Integer: 755843072
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 1]   Key:    Integer: 64
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 1] Value:  Dictionary: 4 entries
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 1] Value:   [ 0]   Key:    Integer: 5
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 1] Value:   [ 0] Value:    Integer: 10600
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 1] Value:   [ 1]   Key:    Integer: 2
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 1] Value:   [ 1] Value:       Long: 2568906250
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 1] Value:   [ 2]   Key:    Integer: 1
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 1] Value:   [ 2] Value:    Integer: 5926728
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 1] Value:   [ 3]   Key:    Integer: 0
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 1] Value:   [ 3] Value:    Integer: 700846080
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 2]   Key:    Integer: 8
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 2] Value:  Dictionary: 4 entries
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 2] Value:   [ 0]   Key:    Integer: 5
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 2] Value:   [ 0] Value:    Integer: 2801
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 2] Value:   [ 1]   Key:    Integer: 2
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 2] Value:   [ 1] Value:       Long: 2465312500
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 2] Value:   [ 2]   Key:    Integer: 1
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 2] Value:   [ 2] Value:    Integer: -1
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 2] Value:   [ 3]   Key:    Integer: 0
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 2] Value:   [ 3] Value:    Integer: 354742272
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 3]   Key:    Integer: 4
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 3] Value:  Dictionary: 4 entries
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 3] Value:   [ 0]   Key:    Integer: 5
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 3] Value:   [ 0] Value:    Integer: 194170
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 3] Value:   [ 1]   Key:    Integer: 2
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 3] Value:   [ 1] Value:       Long: 2462031250
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 3] Value:   [ 2]   Key:    Integer: 1
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 3] Value:   [ 2] Value:    Integer: -1
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 3] Value:   [ 3]   Key:    Integer: 0
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 3] Value:   [ 3] Value:    Integer: 354742272
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 4]   Key:    Integer: 2
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 4] Value:  Dictionary: 4 entries
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 4] Value:   [ 0]   Key:    Integer: 5
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 4] Value:   [ 0] Value:    Integer: 2
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 4] Value:   [ 1]   Key:    Integer: 2
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 4] Value:   [ 1] Value:    Integer: 296093750
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 4] Value:   [ 2]   Key:    Integer: 1
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 4] Value:   [ 2] Value:    Integer: -1
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 4] Value:   [ 3]   Key:    Integer: 0
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 4] Value:   [ 3] Value:    Integer: 215875584
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 5]   Key:    Integer: 1024
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 5] Value:  Dictionary: 3 entries
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 5] Value:   [ 0]   Key:    Integer: 5
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 5] Value:   [ 0] Value:    Integer: 109
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 5] Value:   [ 1]   Key:    Integer: 2
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 5] Value:   [ 1] Value:       Long: 8742812500
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 5] Value:   [ 2]   Key:    Integer: 0
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 5] Value:   [ 2] Value:    Integer: 755843072
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 6]   Key:    Integer: 1
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 6] Value:  Dictionary: 4 entries
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 6] Value:   [ 0]   Key:    Integer: 5
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 6] Value:   [ 0] Value:    Integer: 14
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 6] Value:   [ 1]   Key:    Integer: 2
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 6] Value:   [ 1] Value:    Integer: 55312500
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 6] Value:   [ 2]   Key:    Integer: 1
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 6] Value:   [ 2] Value:    Integer: -1
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 6] Value:   [ 3]   Key:    Integer: 0
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 6] Value:   [ 3] Value:    Integer: 65232896
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 7]   Key:    Integer: 128
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 7] Value:  Dictionary: 4 entries
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 7] Value:   [ 0]   Key:    Integer: 5
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 7] Value:   [ 0] Value:    Integer: 6909
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 7] Value:   [ 1]   Key:    Integer: 2
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 7] Value:   [ 1] Value:       Long: 2648281250
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 7] Value:   [ 2]   Key:    Integer: 1
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 7] Value:   [ 2] Value:    Integer: 9237748
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 7] Value:   [ 3]   Key:    Integer: 0
 * 18:23:27 [ClientCallDump]       [ 0]   [ 1]   [ 7] Value:   [ 3] Value:    Integer: 764661760
 * 18:23:27 [ClientCallDump]  Named Arguments:
 * 18:23:27 [ClientCallDump]   machoVersion
 * 18:23:27 [ClientCallDump]        Integer: 1
 */

/**
    def SendContentsToServer(self, contents = None):
        try:
            if not sm.services['machoNet'].IsConnected():
                return
        except:
            sys.exc_clear()
            return

        if contents is None:
            contents = self.prevContents
        if contents[0] != self.version:
            contents = {}
        else:
            contents = contents[1]
        build = boot.GetValue('build', None)
        contentType = CONTENT_TYPE_PREMIUM
        operatingSystem = PLATFORM_WINDOWS
        if blue.win32.IsTransgaming():
            operatingSystem = PLATFORM_MACOS
        blendedContents = self.entries
        blendedStateMask = self.stateMask
        self.entries = dict()
        self.stateMask = 0
        if contents.has_key(STATE_DISCONNECT):
            blendedContents[STATE_DISCONNECT] = contents[STATE_DISCONNECT]
            blendedStateMask += STATE_DISCONNECT
        if contents.has_key(STATE_GAMESHUTDOWN):
            blendedContents[STATE_GAMESHUTDOWN] = contents[STATE_GAMESHUTDOWN]
            blendedStateMask += STATE_GAMESHUTDOWN
        header = (self.version,
         blendedStateMask,
         build,
         operatingSystem,
         contentType)
        data = (header, blendedContents)
        try:
            uthread.Lock(self, 'sendContents')
            sm.RemoteSvc('clientStatsMgr').SubmitStats(data)
            if hasattr(self, 'prevContents'):
                delattr(self, 'prevContents')
            return True
        finally:
            uthread.UnLock(self, 'sendContents')
*/
