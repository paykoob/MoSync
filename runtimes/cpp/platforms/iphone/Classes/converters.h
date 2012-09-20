/* Copyright (C) 2010 MoSync AB

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

#include <helpers/maapi_defs.h>
#include "mstypeinfo.h"

template <typename T>
inline int convertRet(T type);

template <>
inline int convertRet<void*>(void* type) {
	return (int)((byte*)type - (int)mem_ds);
}

template <typename T>
inline T convertSingleArg(int arg) {
	return (T)arg;
}

inline double convertDoubleArg(int arg1, int arg2) {
	MA_DV dv;
	dv.hi = arg1;
	dv.lo = arg2;
	return dv.d;
}


template <typename T>
inline T convertPointerArg(int arg) {
	// validate
	return (T) ((byte*)mem_ds+arg);
}

#define FLOATS_DIDF(i0, i1, frd) { FREG temp; temp.i[0] = i0; temp.i[1] = i1; frd = (double)(signed long long)temp.ll; }
#define FLOATU_DIDF(i0, i1, frd) { FREG temp; temp.i[0] = i0; temp.i[1] = i1; frd = (double)(unsigned long long)temp.ll; }

#define MOV_SFSI(i0, frs) { MA_FV fv; fv.f = (float)(frs); i0 = fv.i; }
#define MOD_DFDI(i0, i1, frs) { FREG temp; temp.d = frs; i0 = temp.i[0]; i1 = temp.i[1]; }

#define MOV_SISF(i0, frd) { MA_FV fv; fv.i = i0; frd = fv.f; }
#define MOV_DIDF(i0, i1, frd) { FREG temp; temp.i[0] = i0; temp.i[1] = i1; frd = temp.d; }

#define FIXS_DFDI(i0, i1, frs) { FREG temp; temp.ll = (long long)frs; i0 = temp.i[0]; i1 = temp.i[1]; }
#define FIXU_DFDI(i0, i1, frs) { FREG temp; temp.ll = (unsigned long long)frs; i0 = temp.i[0]; i1 = temp.i[1]; }

#define WFLOAT(addr, frs) { MA_FV fv; fv.f = (float)(frs); WINT(addr, fv.i); }
#define WDOUBLE(addr, frs) { FREG temp; temp.d = frs; WINT(addr, temp.i[0]); WINT(addr + 4, temp.i[1]); }

#define MOV_DI(i0, i1, lo) { FREG temp; temp.ll = lo; i0 = temp.i[0]; i1 = temp.i[1]; }

inline long long RETURN_DI(int i0, int i1) {
	FREG temp; temp.i[0] = i0; temp.i[1] = i1; return temp.ll;
}
inline __complex__ double RETURN_CF(double f8, double f9) {
	__complex__ double cd; __real__ cd = f8; __imag__ cd = f9; return cd;
}
inline __complex__ int RETURN_CI(int i0, int i1) {
	__complex__ int ci; __real__ ci = i0; __imag__ ci = i1; return ci;
}
