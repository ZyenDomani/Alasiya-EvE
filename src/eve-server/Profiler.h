/**
 * @name Profiler.h
 *   lightweight profiling code specifically for timing sections of running EvEmu application
 *   this code is very basic, and very specific.
 * @Author:         Allan
 * @date:   13 April 2015
 * @update: Allan 17Nov26
 */


#pragma once

#include "eve-common.h"

class Profile
: public Singleton<Profile>
{
public:
    struct Type {
        enum {          // implemented?  (* = yes)
            destiny     = 1,    //*
            map         = 2,    //
            client      = 3,    //*
            npc         = 4,    //*
            bubbles     = 5,    //*
            items       = 6,    //*
            modules     = 7,    //*
            functions   = 8,    //
            db          = 9,    //*
            ship        = 10,   //*
            targets     = 11,   //*
            server      = 12,   //
            missile     = 13,   //*
            system      = 14,   //*
            entityS     = 15,   //*
            loot        = 16,   //*
            salvage     = 17,   //*
            spawn       = 18,   //*
            collision   = 19,   //*
            drone       = 20,   //*
            itemload    = 21,   //*
            concord     = 22,   //
            colony      = 23,   //*
            damage      = 24,   //*
            parseFX     = 25,   //*
            applyFX     = 26,   //*
            onTarg      = 27,   //
            clientCall  = 28,   //*
            civilians   = 29,   //
            entityP     = 30,    //*
            Max         = 31
        };
    };

    struct Stat {
        double total = 0.0;
        double hi    = 0.0;
        double lo    = 0.0;
        uint32 count = 0;
        uint32 over  = 0;

        void Reset() {
            total = 0.0;
            hi    = 0.0;
            lo    = 0.0;
            count = 0;
            over  = 0;
        }
    };

    inline void AddTime(uint8 key, double value) {
        if (key >= Profile::Type::Max)
            return;
        Stat& s = m_stats[key];
        if (value > 1000000.0) {
            ++s.over;
            return;
        }
        if (s.count == 0) {
            s.hi = value;
            s.lo = value;
        } else {
            if (value > s.hi) s.hi = value;
            if (value < s.lo) s.lo = value;
        }
        s.total += value;
        ++s.count;
    }

    void Initialize();
    
    void ClearAll();

    void PrintProfile();
    void PrintStartUpData();
    void PrintLine(const char* label, uint8 key);

    void GetSize(size_t cSize, std::string& ret);

protected:
    const char* GetKeyName(uint8 key) const;

private:
    Stat m_stats[Profile::Type::Max]; // 31 * 32 bytes = 992 bytes total (fits in 16 cache lines!)
};

#define sProfiler \
( Profile::get() )

