#ifndef VMCOREINT_H
#define VMCOREINT_H

#include "Core.h"

#ifdef GDB_DEBUG
#define UPDATE_IP
/*
#include "debugger.h"
*/
#include "GdbStub.h"
#include "GdbCommon.h"
#endif

#include <base/Syscall.h>

#include <base/base_errors.h>
using namespace MoSyncError;

namespace Core {

class VMCoreInt : public VMCore {
protected:
	Syscall& mSyscall;

public:
	VMCoreInt(Syscall& aSyscall) : mSyscall(aSyscall)
	{
		mem_ds = NULL;
	}
	virtual ~VMCoreInt() {}

	virtual bool LoadVM(Stream& stream) = 0;
	virtual bool LoadVMApp(Stream& stream, const char* combfile) = 0;
	virtual int GetIp(void) const = 0;
	virtual void SetIp(int) = 0;
	virtual int GetValidatedStackValue(int offset) = 0;
	virtual void InvokeSysCall(int syscall_id) = 0;
	virtual void Run2() = 0;

	int VM_Yield;
	void* customEventPointer;

#ifdef FAKE_CALL_STACK
	int* fakeCallStack;
	int fakeCallStackDepth;	//measured in ints
	int fakeCallStackCapacity;	//measured in ints
#endif


#ifdef _android
	virtual bool LoadVMApp(int modFd, int resFd) = 0;
	bool LoadVMAppBase(int modFd, int resFd) {
		FileStream mod(modFd);

		if(!LoadVM(mod))
			return false;

		//-2 means that the mosync application does not need any resources.
		if(-2 != resFd)
		{
			FileStream res(resFd);

			if(!mSyscall.loadResources(res, "resources"))
				return false;
		}
#else	//_android
	virtual bool LoadVMApp(const char* modfile, const char* resfile) = 0;
	bool LoadVMAppBase(const char* modfile, const char* resfile) {
		FileStream mod(modfile);
		if(!LoadVM(mod))
			return false;

		FileStream res(resfile);
		if(!mSyscall.loadResources(res, resfile))
			return false;
#endif	//_android

#ifdef GDB_DEBUG
		if(mGdbOn) {
			if(!mGdbStub) {
				mGdbStub = GdbStub::create(this, this->getCpuType());
				mGdbStub->setupDebugConnection();
			}
			mGdbStub->waitForRemote();
		}
		mGdbSignal = eNone;
#endif
		return true;
	}

#ifdef GDB_DEBUG
	virtual GdbStub::CpuType getCpuType() const = 0;
#endif

	// memory support
#define RAW_MEMREF(type, addr) (*(type*)(((char*)mem_ds) + (addr)))
#define MEMREF(type, addr) RAW_MEMREF(type, \
	(addr) & DATA_SEGMENT_MASK & ~(sizeof(type) - 1))

#ifdef MEMORY_PROTECTION
#define SET_PROTECTION(x) (protectionSet[(x)>>3]|=(1<<((x)&0x7)))
#define RESET_PROTECTION(x) (protectionSet[(x)>>3]&=~(1<<((x)&0x7)))
#define GET_PROTECTION(x) (protectionSet[(x)>>3]&(1<<((x)&0x7)))

	void checkProtection(uint address, uint size) const {
		if(protectionEnabled) {
		for(uint i = address; i < address+size; i++)
			if(GET_PROTECTION(i)) {
				BIG_PHAT_ERROR(ERR_MEMORY_PROTECTED);
			}
		}
	}
#endif	//MEMORY_PROTECTION

#ifdef MEMORY_DEBUG
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
			LOG("Memory reference validation failed. Size %"PFZT", address 0x%x\n",
				sizeof(T), address);
			if((address & (sizeof(T) - 1)) != 0) {
				BIG_PHAT_ERROR(ERR_MEMORY_ALIGNMENT);
			} else if(address >= DATA_SEGMENT_SIZE || (address+sizeof(T)) > DATA_SEGMENT_SIZE) {
				BIG_PHAT_ERROR(ERR_MEMORY_OOB);
			} else {
				BIG_PHAT_ERROR(ERR_MEMORY_NULL);
			}
		}

#ifdef MEMORY_PROTECTION
		checkProtection(address, sizeof(T));
#endif

		return MEMREF(T, address);
	}
#else
#define MEM(type, addr, write) MEMREF(type, addr)
#endif	//_DEBUG

	//****************************************
	//Memory validation
	//****************************************
#define PTR2ADDRESS(ptr) ((unsigned)((char*)ptr - (char*)mem_ds))
	void ValidateMemStringAddress(unsigned address) const {
#ifdef MEMORY_PROTECTION
		int initialAddr = address;
#endif
		do {
			if(address >= DATA_SEGMENT_SIZE)
				BIG_PHAT_ERROR(ERR_MEMORY_OOB);
		} while(RAW_MEMREF(char, address++) != 0);
#ifdef MEMORY_PROTECTION
		checkProtection(initialAddr, address-initialAddr);
#endif
	}
	void ValidateMemWStringAddress(unsigned address) const {
#ifdef MEMORY_PROTECTION
		int initialAddr = address;
#endif
		address -= 2;
		do {
			address += 2;
			if(address >= DATA_SEGMENT_SIZE)
				BIG_PHAT_ERROR(ERR_MEMORY_OOB);
		} while(RAW_MEMREF(short, address) != 0);
#ifdef MEMORY_PROTECTION
		checkProtection(initialAddr, address-initialAddr);
#endif
	}
	int ValidatedStrLen(const char* ptr) const {
		unsigned address = PTR2ADDRESS(ptr);
		do {
			if(address >= DATA_SEGMENT_SIZE)
				BIG_PHAT_ERROR(ERR_MEMORY_OOB);
		} while(RAW_MEMREF(char, address++) != 0);

#ifdef MEMORY_PROTECTION
		checkProtection(PTR2ADDRESS(ptr), address-PTR2ADDRESS(ptr));
#endif
		return address - PTR2ADDRESS(ptr) - 1;
	}
	void ValidateMemRange(const void* ptr, unsigned int size) const {
		unsigned address = PTR2ADDRESS(ptr);
		if(address >= DATA_SEGMENT_SIZE || (address+size) >= DATA_SEGMENT_SIZE ||
			size > DATA_SEGMENT_SIZE)
			BIG_PHAT_ERROR(ERR_MEMORY_OOB);
#ifdef MEMORY_PROTECTION
		checkProtection(address, size);
#endif
	}
	void* GetValidatedMemRange(int address, int size) {
		if(address == 0) return NULL;
		if(uint(address) >= DATA_SEGMENT_SIZE || uint(address+size) >= DATA_SEGMENT_SIZE ||
			uint(size) > DATA_SEGMENT_SIZE)
			BIG_PHAT_ERROR(ERR_MEMORY_OOB);
#ifdef MEMORY_PROTECTION
		checkProtection(address, size);
#endif
		return ((char*)mem_ds) + address;
	}

	int TranslateNativePointerToMoSyncPointer(void *nativePointer) {
		if(nativePointer == NULL)
			return 0;
		else
			return (int)PTR2ADDRESS(nativePointer);
	}

	const char* GetValidatedStr(int a) const {
		unsigned address = a;
		do {
			if(address >= DATA_SEGMENT_SIZE)
				BIG_PHAT_ERROR(ERR_MEMORY_OOB);
		} while(RAW_MEMREF(char, address++) != 0);
#ifdef MEMORY_PROTECTION
		checkProtection(a, address-a);
#endif
		return ((char*)mem_ds) + a;
	}

	const wchar* GetValidatedWStr(int a) const {
		unsigned address = a - sizeof(wchar);
		MYASSERT((address & (sizeof(wchar)-1)) == 0, ERR_MEMORY_ALIGNMENT);
		do {
			address += sizeof(wchar);
			if(address >= DATA_SEGMENT_SIZE)
				BIG_PHAT_ERROR(ERR_MEMORY_OOB);
		} while(RAW_MEMREF(wchar, address) != 0);
#ifdef MEMORY_PROTECTION
		checkProtection(a, address-a);
#endif
		return (wchar*)(((char*)mem_ds) + a);
	}

#ifdef MEMORY_PROTECTION
	void protectMemory(uint start, uint length) {
		char *ptr = ((char*)mem_ds)+start;
		ValidateMemRange(ptr, length);
		//memset(&protectionSet[start], 1, length);
		for(uint i = start; i < start+length; i++)
			SET_PROTECTION(i);
	}

	void unprotectMemory(uint start, uint length) {
		char *ptr = ((char*)mem_ds)+start;
		ValidateMemRange(ptr, length);
		//memset(&protectionSet[start], 0, length);
		for(uint i = start; i < start+length; i++)
			RESET_PROTECTION(i);
	}

	void setMemoryProtection(int enable) {
		this->protectionEnabled = enable;
	}

	int getMemoryProtection() {
		return this->protectionEnabled;
	}
#endif
};

}	//namespace Core

#endif	//VMCOREINT_H
