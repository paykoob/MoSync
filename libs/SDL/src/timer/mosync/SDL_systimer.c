/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2009 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"
#include <ma.h>

#if defined(SDL_TIMER_DUMMY) || defined(SDL_TIMERS_DISABLED)

#include "SDL_events.h"
#include "SDL_timer.h"
#include "../SDL_timer_c.h"

int sStartTicks = 0;
void SDL_StartTicks(void)
{
	sStartTicks = maGetMilliSecondCount();
}

Uint32 SDL_GetTicks (void)
{
	return maGetMilliSecondCount()-sStartTicks;
}

void SDL_Delay (Uint32 ms)
{
	Uint32 startTime = SDL_GetTicks();
	int timeLeft =  ms;
	while(timeLeft>0) {
		maWait(timeLeft);
		SDL_PumpEvents();
		timeLeft = ms - (SDL_GetTicks()-startTime);
	}
}

/* This is only called if the event thread is not running */
int SDL_SYS_TimerInit(void)
{
	return 0;
}

void SDL_SYS_TimerQuit(void)
{
}

int SDL_SYS_StartTimer(void)
{
	SDL_SetError("Internal logic error: threaded timer in use");
	return(-1);
}

void SDL_SYS_StopTimer(void)
{
	return;
}

#endif /* SDL_TIMER_DUMMY || SDL_TIMERS_DISABLED */
