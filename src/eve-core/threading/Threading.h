/**
 * @name Threading.h
 *   lightweight threading code specifically for EvEmu concurrency
 *   this code is very basic, and very specific.
 * @Author:         Allan
 * @date:   05 March 2016
 */


#ifndef EVE_THREADING_H
#define EVE_THREADING_H

#include "../eve-core.h"
#include "log/LogNew.h"
#include "utils/Singleton.h"

class Threading
: public Singleton<Threading>
{
public:
    Threading();
    ~Threading();

    void ListThreads();
    void AddThread(pthread_t thread);
    void EndThreads();

    uint8 Count()                           { return (uint8)(m_threads.size()); }


    void CreateThread();


private:
    std::vector<pthread_t> m_threads;
};

//Singleton
#define sThread \
    ( Threading::get() )

#endif  // EVE_THREADING_H
