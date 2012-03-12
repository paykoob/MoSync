/* run front end support for arm
   Copyright (C) 1995-1997, 2000-2002, 2007-2012 Free Software
   Foundation, Inc.

   This file is part of ARM SIM.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* This file provides the interface between the simulator and
   run.c and gdb (when the simulator is linked with gdb).
   All simulator interaction should go through this file.  */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <bfd.h>
#include <signal.h>

#include "armdefs.h"
#include "armemu.h"
#include "dbg_rdi.h"
#include "ansidecl.h"

int stop_simulator;

void
ARMul_ConsolePrint VPARAMS ((ARMul_State * state,
			     const char * format,
			     ...))
{
  va_list ap;

  if (state->verbose)
    {
      va_start (ap, format);
      vprintf (format, ap);
      va_end (ap);
    }
}

ARMword
ARMul_Debug (ARMul_State * state ATTRIBUTE_UNUSED, ARMword pc ATTRIBUTE_UNUSED, ARMword instr ATTRIBUTE_UNUSED)
{
  return 0;
}

// invoke syscall
unsigned ARMul_OSHandleSWI (ARMul_State * state, ARMword number) {
	return state->swiHandler(state, number, state->user);
}

void __declspec(dllexport) ARMul_SetSWIhandler (ARMul_State* state, ARMul_SWIhandler* h, void* user);
void __declspec(dllexport) ARMul_SetSWIhandler (ARMul_State* state, ARMul_SWIhandler* h, void* user) {
	state->swiHandler = h;
	state->user = user;
}

__declspec(dllexport) ARMword* ARMul_GetRegs (ARMul_State * state);
__declspec(dllexport) ARMword* ARMul_GetRegs (ARMul_State * state) {
	return state->Reg;
}

/* The emulator calls this routine when an Exception occurs.  The second
   parameter is the address of the relevant exception vector.  Returning
   FALSE from this routine causes the trap to be taken, TRUE causes it to
   be ignored (so set state->Emulate to FALSE!).  */

unsigned
ARMul_OSException (ARMul_State * state  ATTRIBUTE_UNUSED,
		   ARMword       vector ATTRIBUTE_UNUSED,
		   ARMword       pc     ATTRIBUTE_UNUSED)
{
  return FALSE;
}
