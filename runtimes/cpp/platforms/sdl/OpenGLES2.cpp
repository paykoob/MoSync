#include <GL/glew.h>
#include "config_platform.h"
#include "OpenGLES2.h"
#include <helpers/CPP_IX_GL2.h>
#include <helpers/log.h>
#include <helpers/helpers.h>
#include <helpers/cpp_defs.h>
#include "Syscall.h"

namespace Base {
	extern Syscall* gSyscall;
}

using namespace Base;

#include "GLFixes.h"

void gles2init() {
	GLenum res = glewInit();
	LOG("glewInit: %s\n", glewGetErrorString(res));
	DEBUG_ASSERT(res == GLEW_OK);
}

int gles2ioctl(int function, int a, int b, int c, va_list argptr) {
	switch(function) {
		maIOCtl_IX_GL2_caselist;
	default:
		return IOCTL_UNAVAILABLE;
	}
};
