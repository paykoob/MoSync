#include <ma.h>

typedef int ptrdiff_t;

#define offsetof(type, member) __builtin_offsetof (type, member)
