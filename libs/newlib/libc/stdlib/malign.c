#ifndef MALLOC_PROVIDED
/* malign.c -- a wrapper for memalign_r.  */

#include <_ansi.h>
#include <reent.h>
#include <stdlib.h>
#include <malloc.h>

#if !defined(_REENT_ONLY) && !defined(NO_REENT_MALLOC)

_PTR
_DEFUN (memalign, (align, nbytes),
	size_t align _AND
	size_t nbytes)
{
  return _memalign_r (_REENT, align, nbytes);
}

#endif
#endif
