/**
 * @name Threading.cpp
 *   lightweight threading code specifically for EvEmu concurrency
 *   this code is very basic, and very specific.
 * @Author:         Allan
 * @date:   05 March 2016
 */

/** @todo the entire concurrency idea needs work throughout evemu.  currently, it uses posix threads, boost, and shared_pointer.
 * they all need updating
 */

#include "Threading.h"


Threading::Threading()
{
    m_threads.clear();
}

Threading::~Threading()
{
    m_threads.clear();
}

void Threading::CreateThread() {
    /*
    pthread_t thread;
    pthread_create( &thread, NULL, classType, classPtr );
    sLog.Log( "        Threading", "Starting %s with thread ID 0x%X", classType, classPtr );
    m_threads.push_back(thread);*/
}

void Threading::AddThread(pthread_t thread)
{
    m_threads.push_back(thread);
    sLog.Log( "        Threading", "Added thread ID 0x%X", thread );
}

void Threading::ListThreads() {
    /* need to make vector of <class name, pointer> and can use here to id the thread */
    for (auto cur : m_threads)
        sLog.Log( "                 ", "Thread ID 0x%X", cur );
}

void Threading::EndThreads() {
    for (auto cur : m_threads) {
        sLog.Log( "        Threading", "Joining thread with ID 0x%X", cur );
        pthread_join(cur, nullptr);
        //pthread_exit(NULL);
    }

    }
