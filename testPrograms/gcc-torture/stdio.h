#include <mavsprintf.h>

#define stdout 1
#define stderr 2
#define fprintf(fd, args...) lprintfln(args)
#define vfprintf(fd, fmt, args) vlprintfln(fmt, args)

typedef struct FILE FILE;
