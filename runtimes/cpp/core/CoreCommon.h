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

// This file is shared by the debugger.

#ifndef _CORE_COMMON_H_
#define _CORE_COMMON_H_

#define DATA_MEMORY_START 0x00000000
#define INSTRUCTION_MEMORY_START 0x40000000
#define ADDRESS_MASK 0x3fffffff

#include "gen-opcodes.h"

typedef struct _MA_HEAD
{
	int Magic;
	int CodeLen;
	int DataLen;
	int DataSize;
	int StackSize;
	int HeapSize;
	int CtorAddress;
	int DtorAddress;
	int BuildID;
	int AppID;
	int EntryPoint;
} MA_HEAD;

#define MA_HEAD_MAGIC 0x5944414d	//MADY

#endif	//_CORE_COMMON_H_
