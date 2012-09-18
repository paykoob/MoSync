#ifndef __STDLIB_H__
#define __STDLIB_H__

#include <mastdlib.h>
#include <conprint.h>
#include <maheap.h>

#ifdef __cplusplus
extern "C" {
#endif
void exit(int);
void abort(void);
#ifdef __cplusplus
}
#endif

#define EXIT_SUCCESS 0

#endif	//__STDLIB_H__
