#ifndef _STDDEF_H_
#define _STDDEF_H_

#include <ma.h>
#include <stdint.h>

#define offsetof(type, member) __builtin_offsetof (type, member)

typedef int ptrdiff_t;

#endif	//_STDDEF_H_
