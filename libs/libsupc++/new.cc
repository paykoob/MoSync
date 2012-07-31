#include <new>

void *operator new(std::size_t s, const std::nothrow_t&) { return operator new(s); }
void *operator new[](std::size_t s, const std::nothrow_t&) { return operator new(s); }
void operator delete(void *p, const std::nothrow_t&) { operator delete(p); }
void operator delete[](void *p, const std::nothrow_t&) { operator delete(p); }
