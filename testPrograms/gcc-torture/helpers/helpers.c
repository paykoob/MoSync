#include <ma.h>
#include <stdlib.h>

void abort(void) {
	maExit(420);
}

#ifdef MAPIP
void exit(int code) {
	maExit(code);
}

void _exit(int code);
void _exit(int code) {
	maExit(code);
}
#endif

int __main(void);
int __main(void)
{
	return 0;
}

int resource_selector(void);
int resource_selector(void)
{
	return 0;
}

int main(int argc, char** argv);

int MAMain(void) {
	return main(0, 0);
}

const char* getenv(const char* key);
const char* getenv(const char* key) {
	return NULL;
}
