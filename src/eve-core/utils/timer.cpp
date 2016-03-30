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
    Updates:    Allan
*/

#include "eve-core.h"

#include "utils/timer.h"
#include <ctime>

static uint32 currentTime = 0;
static uint32 currentSeconds = 0;
static uint32 lastTime = 0;

Timer::Timer(uint32 time, bool useAcurateTiming /*false*/) {
    m_timerTime = time;
    m_startTime = currentTime;
    m_setAtTrigger = m_timerTime;
    m_useAcurateTiming = useAcurateTiming;

	if (m_timerTime)
        m_enabled = true;
    else
        m_enabled = false;
}

Timer::Timer(uint32 startAt, uint32 time, bool useAcurateTiming /*false*/) {
    m_timerTime = time;
    m_startTime = startAt;
    m_setAtTrigger = m_timerTime;
    m_useAcurateTiming = useAcurateTiming;

    if (m_timerTime)
        m_enabled = true;
    else
        m_enabled = false;
}

/* This function checks if the timer triggered */
bool Timer::Check(bool reset /*true*/)
{
    if (!this) {
        printf( "Null timer during ->Check()!?\n" );
        return true;
    }
    if (m_enabled && (currentTime - m_startTime > m_timerTime)) {
        if (reset) {
            if (m_useAcurateTiming)
                m_startTime += m_timerTime; /* set start time to end of last timer */
            else
                m_startTime = currentTime; /* set start time to now */
            m_timerTime = m_setAtTrigger;
        }
        return true;
    }

    return false;
}

/* This function sets the timer and starts it */
void Timer::Start(uint32 setTimerTime, bool changeResetTimer /*true*/) {
    m_startTime = currentTime;
    m_enabled = true;
    if (setTimerTime) {
        m_timerTime = setTimerTime;
        if (changeResetTimer)
            m_setAtTrigger = setTimerTime;
    }
}

/* This updates the timer without restarting it */
void Timer::SetTimer(uint32 setTimerTime) {
    /* If we were disabled before => restart the timer */
    if (!m_enabled) {
        m_startTime = currentTime;
        m_enabled = true;
    }
    if (setTimerTime) {
        m_timerTime = setTimerTime;
        m_setAtTrigger = setTimerTime;
    }
}

uint32 Timer::GetRemainingTime() const {
    if (m_enabled)
        if ((currentTime - m_startTime) < m_timerTime)
            return (m_startTime + m_timerTime - currentTime);

    return 0;
}

void Timer::SetAtTrigger(uint32 setAtTrigger, bool enableIfDisabled) {
    m_setAtTrigger = setAtTrigger;
    if (!Enabled() && enableIfDisabled)
        Enable();
}

void Timer::Trigger() {
    m_enabled = true;
    m_timerTime = m_setAtTrigger;
    m_startTime = (currentTime - m_timerTime - 1);
}

const uint32 Timer::GetCurrentTime() {
    return currentTime;
}

//just to keep all time related crap in one place... not really related to timers.
const uint32 Timer::GetTimeSeconds() {
    return currentSeconds;
}

const uint32 Timer::SetCurrentTime()
{
    const uint32 tickCount = ::GetTickCount();

    if( lastTime == 0 )
        currentTime = 0;
    else
        currentTime += (tickCount - lastTime);

    lastTime = tickCount;
    currentSeconds = (tickCount / 1000);

    return currentTime;
}
