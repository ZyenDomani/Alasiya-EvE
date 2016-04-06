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
    Author:     Zhur
    Updates:    Allan
*/

#include "eve-core.h"

#include "network/TCPServer.h"
#include "log/LogNew.h"
#include "log/logsys.h"
#include "threading/Threading.h"

const uint32 TCPSRV_ERRBUF_SIZE = 1024;
const uint32 TCPSRV_LOOP_GRANULARITY = 5;  /* 5ms */

BaseTCPServer::BaseTCPServer()
: mSock( nullptr ),
  mPort( 0 )
{
}

BaseTCPServer::~BaseTCPServer()
{
    // Close socket
    Close();
    // Wait until worker thread terminates
    WaitLoop();
    sThread.RemoveThread(pthread_self());
}

bool BaseTCPServer::IsOpen() const
{
    mMSock.Lock();
    bool ret = (mSock != nullptr);
    mMSock.Unlock();
    return ret;
}

bool BaseTCPServer::Open( uint16 port, char* errbuf )
{
    if (errbuf)
        errbuf[0] = 0;

    // mutex lock
    MutexLock lock( mMSock );

    if (IsOpen()) {
        if (errbuf)
            _log(TCP_SERVER__ERROR, "Open() - Listening socket already open" );
        return false;
    } else {
        mMSock.Unlock();
        WaitLoop();
        mMSock.Lock();
    }

    // Setting up TCP port for new TCP connections
    mSock = new Socket( AF_INET, SOCK_STREAM, 0 );

    // Quag: don't think following is good stuff for TCP, good for UDP
    // Mis: SO_REUSEADDR shouldn't be a problem for tcp - allows you to restart
    //      without waiting for conn's in TIME_WAIT to die
    unsigned int reuse_addr = 1;
    mSock->setopt( SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof( reuse_addr ) );

    // Setup internet address information.
    // This is used with the bind() call
    sockaddr_in address;
    memset( &address, 0, sizeof( address ) );
        address.sin_family = AF_INET;
        address.sin_port = htons( port );
        address.sin_addr.s_addr = htonl( INADDR_ANY );

    if (mSock->bind((sockaddr*)&address, sizeof(address)) < 0) {
        if (errbuf)
            _log(TCP_SERVER__ERROR, "Open()::bind() < 0" );
        SafeDelete( mSock );
        return false;
    }

    unsigned int bufsize = 64 * 1024; // 64kbyte receive buffer, up from default of 8k
    mSock->setopt( SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof( bufsize ) );
    mSock->fcntl( F_SETFL, O_NONBLOCK );
    if (mSock->listen() == SOCKET_ERROR) {
        if (errbuf)
            _log(TCP_SERVER__ERROR, "Open()::listen() failed, Error: %s", strerror( errno ) );
        SafeDelete( mSock );
        return false;
    }

    mPort = port;
    StartLoop();
    return true;
}

void BaseTCPServer::Close()
{
    MutexLock lock(mMSock);
    SafeDelete(mSock);
    mPort = 0;
}

void BaseTCPServer::StartLoop()
{
    pthread_t thread;
    pthread_create( &thread, nullptr, TCPServerLoop, this );
    _log(THREAD__WARNING, "StartLoop() - Created thread ID 0x%X for TCPServerLoop", thread);
    sThread.AddThread(thread);
}

void BaseTCPServer::WaitLoop()
{
    //wait for running loop to stop.
    mMLoopRunning.Lock();
    mMLoopRunning.Unlock();
}

bool BaseTCPServer::Process()
{
    MutexLock lock( mMSock );
    if (IsOpen()) {
        ListenNewConnections();
        return true;
    }
    return false;
}

void BaseTCPServer::ListenNewConnections()
{
    Socket* sock = nullptr;
    sockaddr_in from;
        from.sin_family = AF_INET;
    unsigned int fromlen = sizeof( from );
    MutexLock lock( mMSock );

    while (sock = mSock->accept((sockaddr*)&from, &fromlen)) {
        sock->fcntl( F_SETFL, O_NONBLOCK );
        unsigned int bufsize = 64 * 1024; // 64kbyte receive buffer, up from default of 8k
        sock->setopt( SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof( bufsize ) );
        // New TCP connection, this must consume the socket.
        CreateNewConnection( sock, from.sin_addr.s_addr, ntohs( from.sin_port ) );
    }
}

void* BaseTCPServer::TCPServerLoop( void* arg )
{
    BaseTCPServer* tcps = reinterpret_cast< BaseTCPServer* >( arg );
    assert( tcps != nullptr );

    tcps->TCPServerLoop();

    return nullptr;
}

void BaseTCPServer::TCPServerLoop()
{
    mMLoopRunning.Lock();
    uint32 start = GetTickCount();
    while (Process()) {
        // do the stuff for thread sleeping
        if (TCPSRV_LOOP_GRANULARITY > (GetTickCount() - start))
            Sleep(TCPSRV_LOOP_GRANULARITY);
        start = GetTickCount();
    }
    mMLoopRunning.Unlock();
}
