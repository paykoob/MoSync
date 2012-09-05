#include <ma.h>
#include <stdlib.h>

void abort(void) {
	maExit(420);
}

#ifndef USE_NEWLIB
void _exit(int code);
void _exit(int code) {
	maExit(code);
}

char* getenv(const char* key);
char* getenv(const char* key) {
	return NULL;
}

void fflush(void* f);
void fflush(void* f) {
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

int main(int argc, char** argv, char** envp);

int MAMain(void) {
	return main(0, 0, 0);
}
