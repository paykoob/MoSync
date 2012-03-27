/* Copyright (C) 2009 Mobile Sorcery AB

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.
*/

#include "maapi.h"

extern "C" void GCCATTRIB(noreturn) __cxa_pure_virtual();
extern "C" void GCCATTRIB(noreturn) __cxa_pure_virtual() {
	maExit(-42);
}

#ifdef __arm__
extern "C" void ansi_heap_init_crt0(char *start, int length);
static char sHeap[1024*512];

extern "C" int resource_selector();

extern "C" int MAMain();
int main() {
	// todo: put before static constructors.
	ansi_heap_init_crt0(sHeap, sizeof(sHeap));
	resource_selector();
	return MAMain();
}

void exit(int i) {
	maExit(i);
}
#endif
