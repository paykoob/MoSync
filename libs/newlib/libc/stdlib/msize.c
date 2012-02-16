#ifndef MALLOC_PROVIDED
/* msize.c -- a wrapper for malloc_usable_size.  */

#include <_ansi.h>
#include <reent.h>
#include <stdlib.h>
#include <malloc.h>

#if !defined(_REENT_ONLY) && !defined(NO_REENT_MALLOC)

size_t
_DEFUN (malloc_usable_size, (ptr),
	_PTR ptr)
{
  return _malloc_usable_size_r (_REENT, ptr);
}

#endif
#endif
