#include <ma.h>
#include <stdlib.h>

void abort(void) {
	maExit(420);
}

#ifdef MAPIP
void exit(int code) {
	maExit(code);
}
#endif

int __main(void)
{
	return 0;
}

int resource_selector()
{
	return 0;
}

int main(void);

int MAMain(void) {
	return main();
}
