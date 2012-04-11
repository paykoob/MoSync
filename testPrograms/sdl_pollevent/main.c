#include <SDL/SDL.h>
#include <mavsprintf.h>
#include <IX_OPENGL_ES.h>

int MAMain(void) {
	SDL_Event e;
	SDL_Init(SDL_INIT_EVERYTHING);
	maOpenGLInitFullscreen(MA_GL_API_GL1);
	while(TRUE) {
		if(SDL_PollEvent(&e))
			break;
	}
	return 0;
}
