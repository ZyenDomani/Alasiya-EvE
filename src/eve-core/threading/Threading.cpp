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
#include "log/LogNew.h"
#include "log/logsys.h"


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
    pthread_create( &thread, nullptr, function, args );
    _log(THREAD__WARNING, "CreateThread() - Creating new thread ID 0x%X to run %s", thread, function );
    m_threads.push_back(thread);*/
}

void Threading::AddThread(pthread_t thread)
{
    m_threads.push_back(thread);
    _log(THREAD__INFO, "AddThread() - Added thread ID 0x%X", thread);
}

void Threading::RemoveThread(pthread_t thread)
{
    for (std::vector<pthread_t>::iterator cur = m_threads.begin(); cur != m_threads.end(); ++cur) {
        if ((*cur) == thread) {
            void* res = nullptr;
            _log(THREAD__INFO, "RemoveThread() - Joining thread ID 0x%X", (*cur) );
            if (pthread_join((*cur), &res))
                _log(THREAD__ERROR, "RemoveThread() - Join returned %s for thread ID 0x%X", (char*)res, (*cur));
            m_threads.erase(cur);
            free(res);
            return;
        }
    }
    _log(THREAD__ERROR, "RemoveThread() - Called Remove on unregistered thread ID 0x%X", thread);
}

void Threading::ListThreads() {
    /* need to make vector of <class name, pointer> to use here to id the thread */
    for (auto cur : m_threads)
        sLog.Log( "                 ", "Thread ID 0x%X", cur );
}

void Threading::EndThreads() {
    if (!m_threads.size()) {
        _log(THREAD__MESSAGE, "EndThreads() - There are no active threads.");
        return;
    }
    _log(THREAD__MESSAGE, "EndThreads() - Joining %u currently active threads.", m_threads.size());
    void* res = nullptr;
    for (std::vector<pthread_t>::iterator cur = m_threads.begin(); cur != m_threads.end(); ++cur) {
        _log(THREAD__TRACE, "EndThreads() - Joining thread ID 0x%X", (*cur));
        if (pthread_join((*cur), &res))
            _log(THREAD__ERROR, "EndThreads() - Join returned %s for thread ID 0x%X", (char*)res, (*cur));
        m_threads.erase(cur);
    }
    free(res);
    m_threads.clear();
}
