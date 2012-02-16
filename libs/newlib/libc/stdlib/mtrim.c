#ifndef MALLOC_PROVIDED
/* mtrim.c -- a wrapper for malloc_trim.  */

#include <_ansi.h>
#include <reent.h>
#include <stdlib.h>
#include <malloc.h>

#if !defined(_REENT_ONLY) && !defined(NO_REENT_MALLOC)

int
_DEFUN (malloc_trim, (pad),
	size_t pad)
{
  return _malloc_trim_r (_REENT, pad);
}

#endif
#endif
