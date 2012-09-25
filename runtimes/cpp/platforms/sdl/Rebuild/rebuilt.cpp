#include "rebuilt.h"
#include "Syscall.h"
#include "sdl_syscall.h"

using namespace Base;

int sp;
unsigned char* mem_ds;
unsigned char* customEventPointer;
static unsigned DATA_SEGMENT_SIZE;
static unsigned DATA_SEGMENT_MASK;
static unsigned STACK_TOP;
static unsigned STACK_BOTTOM;

void runRebuiltCode(const char* dataSectionFile) {
	DATA_SEGMENT_SIZE = 64*1024*1024;
	DATA_SEGMENT_MASK = DATA_SEGMENT_SIZE - 1;
	mem_ds = new unsigned char[DATA_SEGMENT_SIZE];
	memset(mem_ds, 0, DATA_SEGMENT_SIZE);	// bss. todo: zero only the bss section.

	// read data section from file
	//memset(mem_ds, 0, mallocSize);
	FileStream file(dataSectionFile);
	bool res;
	int size;
	res = file.length(size);
	DEBUG_ASSERT(res);
	res = file.read(mem_ds, size);
	DEBUG_ASSERT(res);

	int maxCustomEventSize = getMaxCustomEventSize();
	customEventPointer = mem_ds + (DATA_SEGMENT_SIZE - maxCustomEventSize);

	STACK_TOP = DATA_SEGMENT_SIZE - maxCustomEventSize;
	STACK_BOTTOM = STACK_TOP - 1024*1024;
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

namespace Base {

#define RAW_MEMREF(type, addr) (*(type*)(((char*)mem_ds) + (addr)))
#define MEMREF(type, addr) RAW_MEMREF(type, \
	(addr) & DATA_SEGMENT_MASK & ~(sizeof(type) - 1))

#ifdef MEMORY_DEBUG
	template<class T, bool write> T& getValidatedMemRefBase(uint address);

#define MEM(type, addr, write) getValidatedMemRef##write<type>(addr)
	template<class T> const T& getValidatedMemRefREAD(uint address) {
		return getValidatedMemRefBase<T, 0>(address);
	}
	template<class T> T& getValidatedMemRefWRITE(uint address) {
		return getValidatedMemRefBase<T, 1>(address);
	}

	template<class T, bool write> T& getValidatedMemRefBase(uint address) {
		DEBUG_ASSERT(isPowerOf2(sizeof(T)));
		if(address >= DATA_SEGMENT_SIZE || (address+sizeof(T)) > DATA_SEGMENT_SIZE ||
			((address & (sizeof(T) - 1)) != 0) || //alignment check
			(address < 4))	//NULL pointer check
		{
			LOG("Memory reference validation failed. Size %" PFZT ", address 0x%x\n",
				sizeof(T), address);
			if((address & (sizeof(T) - 1)) != 0) {
				BIG_PHAT_ERROR(ERR_MEMORY_ALIGNMENT);
			} else if(address >= DATA_SEGMENT_SIZE || (address+sizeof(T)) > DATA_SEGMENT_SIZE) {
				BIG_PHAT_ERROR(ERR_MEMORY_OOB);
			} else {
				BIG_PHAT_ERROR(ERR_MEMORY_NULL);
			}
		}

		return MEMREF(T, address);
	}
#else
#define MEM(type, addr, write) MEMREF(type, addr)
#endif	//_DEBUG

	//****************************************
	//Memory validation
	//****************************************
#define PTR2ADDRESS(ptr) ((unsigned)((char*)ptr - (char*)mem_ds))
#if 0
	void ValidateMemStringAddress(unsigned address) {
		do {
			if(address >= DATA_SEGMENT_SIZE)
				BIG_PHAT_ERROR(ERR_MEMORY_OOB);
		} while(RAW_MEMREF(char, address++) != 0);
	}
	void ValidateMemWStringAddress(unsigned address) {
		address -= 2;
		do {
			address += 2;
			if(address >= DATA_SEGMENT_SIZE)
				BIG_PHAT_ERROR(ERR_MEMORY_OOB);
		} while(RAW_MEMREF(short, address) != 0);
	}
#endif
	int Syscall::ValidatedStrLen(const char* ptr) {
		unsigned address = PTR2ADDRESS(ptr);
		do {
			if(address >= DATA_SEGMENT_SIZE)
				BIG_PHAT_ERROR(ERR_MEMORY_OOB);
		} while(RAW_MEMREF(char, address++) != 0);

		return address - PTR2ADDRESS(ptr) - 1;
	}
	void Syscall::ValidateMemRange(const void* ptr, int size) {
		unsigned address = PTR2ADDRESS(ptr);
		if(address >= DATA_SEGMENT_SIZE || (address+size) >= DATA_SEGMENT_SIZE ||
			(unsigned)size > DATA_SEGMENT_SIZE)
			BIG_PHAT_ERROR(ERR_MEMORY_OOB);
	}
	void* Syscall::GetValidatedMemRange(int address, int size) {
		if(address == 0) return NULL;
		if(uint(address) >= DATA_SEGMENT_SIZE || uint(address+size) >= DATA_SEGMENT_SIZE ||
			uint(size) > DATA_SEGMENT_SIZE)
		{
			LOG("GetValidatedMemRange(address 0x%x, size 0x%x)\n", address, size);
			BIG_PHAT_ERROR(ERR_MEMORY_OOB);
		}
		return ((char*)mem_ds) + address;
	}

	int Syscall::GetValidatedStackValue(int offset, va_list argptr) {
		int address = sp + offset;
		if(((address&0x03)!=0) || uint(address)<STACK_BOTTOM || uint(address)>STACK_TOP)
			BIG_PHAT_ERROR(ERR_STACK_OOB);
		return *(int*)(mem_ds + address);
	}

	int Syscall::TranslateNativePointerToMoSyncPointer(void *nativePointer) {
	    if(nativePointer == NULL)
	        return 0;
	    else
	        return (int)PTR2ADDRESS(nativePointer);
	}

	void* Base::Syscall::GetCustomEventPointer() {
		return mem_ds + STACK_TOP;
	}

	const char* Syscall::GetValidatedStr(int a) {
		unsigned address = a;
		do {
			if(address >= DATA_SEGMENT_SIZE)
				BIG_PHAT_ERROR(ERR_MEMORY_OOB);
		} while(RAW_MEMREF(char, address++) != 0);
		return ((char*)mem_ds) + a;
	}

	const wchar* Syscall::GetValidatedWStr(int a) {
		unsigned address = a - sizeof(wchar);
		MYASSERT((address & (sizeof(wchar)-1)) == 0, ERR_MEMORY_ALIGNMENT);
		do {
			address += sizeof(wchar);
			if(address >= DATA_SEGMENT_SIZE)
				BIG_PHAT_ERROR(ERR_MEMORY_OOB);
		} while(RAW_MEMREF(wchar, address) != 0);
		return (wchar*)(((char*)mem_ds) + a);
	}
}
