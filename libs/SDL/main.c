#include <ma.h>

static const char *argv[] = {"exe", "-width", "240", "-height", "320", "-bpp", "32", NULL};
static int argc = sizeof(argv)/sizeof(char*);

int main(int argc, const char** argv);
int MAMain(void) {
	return main(argc, argv);
}
