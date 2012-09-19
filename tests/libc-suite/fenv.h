#define FE_ALL_EXCEPT 0
#define FE_OVERFLOW 0
#define FE_UNDERFLOW 0
#define FE_INEXACT 0

int feclearexcept(int excepts);
int fetestexcept (int excepts);
