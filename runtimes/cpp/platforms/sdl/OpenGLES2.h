#ifndef OPENGLES2_H
#define OPENGLES2_H

#include <stdarg.h>

int gles2ioctl(int function, int a, int b, int c, va_list argptr);
void gles2init();

#endif	//OPENGLES2_H
