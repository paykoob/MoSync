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

#include "ma.h"
#include "madmath.h"

#ifdef MAPIP

#include "math_private.h"

// Functions are intrinsic no prototypes needed
// See http://gcc.gnu.org/onlinedocs/gccint/Soft-float-library-routines.html
// for documentation

static int
_isnan(double x)
{
	int32_t hx,lx;
	EXTRACT_WORDS(hx,lx,x);
	hx &= 0x7fffffff;
	hx |= (u_int32_t)(lx|(-lx))>>31;
	hx = 0x7ff00000 - hx;
	return (int)((u_int32_t)(hx))>>31;
}

int __unorddf2(double a, double b) {
	return _isnan(a) | _isnan(b);
}

int __unordsf2(float a, float b) {
	return _isnan(a) | _isnan(b);
}

float __powisf2(float a, int b) {
	return (float)pow(a, (double)b);
}

double __powidf2(double a, int b) {
	return pow(a, (double)b);
}

#ifndef USE_NEWLIB
double fabs(double d) {
	return d > 0 ? d : -d;
}

double atof(const char* string) {
	return strtod(string, NULL);
}

double fmod(double numerator, double denominator) {
	double res = numerator - floor(numerator/denominator) * denominator;
	if((numerator < 0.0) != (denominator < 0.0))
		res = res - denominator;
	return res;
}
#endif	//USE_NEWLIB

#endif	//MAPIP
