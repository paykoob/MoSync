#include "rebuilt.h"
#include "Syscall.h"
#include "sdl_syscall.h"

using namespace Base;

int sp;
unsigned char* mem_ds;
unsigned char* customEventPointer;

void runRebuiltCode() {
	int DATA_SEGMENT_SIZE = 64*1024*1024;
	mem_ds = new unsigned char[DATA_SEGMENT_SIZE];

	// read data section from file
	//memset(mem_ds, 0, mallocSize);
	FileStream file("data_section.bin");
	bool res;
	int size;
	res = file.length(size);
	DEBUG_ASSERT(res);
	res = file.read(mem_ds, size);
	DEBUG_ASSERT(res);

	int maxCustomEventSize = getMaxCustomEventSize();
	customEventPointer = mem_ds + (DATA_SEGMENT_SIZE - maxCustomEventSize);

	int STACK_TOP = DATA_SEGMENT_SIZE - maxCustomEventSize;
	int STACK_BOTTOM = STACK_TOP - 1024*1024;
	DUMPHEX(STACK_TOP);
	DUMPHEX(STACK_BOTTOM);

	// set the initial registers
	// sp: top of stack
	sp = STACK_TOP - 16;

#if 0
	// p0: memory size
	// p1: stack size
	// p2: heap size
	// p3: ctor chain
	int p0 = DATA_SEGMENT_SIZE;
	int p1 = 1024*1024;
	int p2 = 32*(1024*1024);

	// some programs have no ctor section.
	int p3 = 0;
#endif

	entryPoint();
}
