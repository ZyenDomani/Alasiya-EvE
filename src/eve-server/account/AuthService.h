/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2023 Alasiya-EvE (by Allan)
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
    Author:     Zhur
*/


#ifndef __AUTH_H_INCL__
#define __AUTH_H_INCL__

#include "PyService.h"

class AuthService : public PyService {
public:
    AuthService(PyServiceMgr *mgr);
    virtual ~AuthService();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(Ping);
    PyCallable_DECL_CALL(GetPostAuthenticationMessage);
    PyCallable_DECL_CALL(AmUnderage);
    PyCallable_DECL_CALL(AccruedTime);
    PyCallable_DECL_CALL(SetLanguageID);
};

#endif

/*all clients up to Odyssey 1.1

-----BEGIN PUBLIC KEY-----
MFwwDQYJKoZIhvcNAQEBBQADSwAwSAJBAMZsTz93LplFdBeFY8uJDJ9vYTNCGje9
9TdGVyN892pnSuXlq2x0B00Tr0XWix5wyTLNp1P7mh6YELPO86fgvVkCAwEAAQ==
-----END PUBLIC KEY-----

-----BEGIN RSA PRIVATE KEY-----
MIIBOwIBAAJBAMZsTz93LplFdBeFY8uJDJ9vYTNCGje99TdGVyN892pnSuXlq2x0
B00Tr0XWix5wyTLNp1P7mh6YELPO86fgvVkCAwEAAQJAUMM3AlsNUX9ugEBf3TFc
POzFwGpQZ43e6G+t+hjcT6ch4Nomk/b5uiKekHk792MzV0FXYe/1KLmtv0UI7dH9
XQIhAOJUwW9Z4f9y02UvUU/L6M5T0hXS2bznAF4XkOFdqsEXAiEA4G8DqvvlohNF
73GC8/jpq6JBg8miZVWfhjmFLHjOGw8CIQDJYhKvinFtgvUXvk+CSfQ+yhRPOMpm
q8AG+L7/2AEcRwIhAIraxFcuP/WVnQg2n4GYd+HTolsKDipJ3keqIMXR/BprAiBp
rEPcieCoUI6HZWAyn4YMSGtdKTDFy3NGnZRSapjEeA==
-----END RSA PRIVATE KEY-----

*/