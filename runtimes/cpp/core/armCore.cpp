
#define SYSCALL_DEBUGGING_MODE

#include "config_platform.h"
#include "helpers/log.h"
#include "armCore.h"
#include "VMCoreInt.h"
#include "elf.h"
#include <vector>
#include <arm-core/arm_ext.h>

using namespace Core;
using namespace std;

class ArmCore : public VMCoreInt {
public:
	ArmCore(Syscall& aSyscall) : VMCoreInt(aSyscall) {}

	virtual ~ArmCore() {}

#ifdef GDB_DEBUG
	virtual GdbStub::CpuType getCpuType() const { return GdbStub::Arm; }
#endif

#ifdef _android
	bool LoadVMApp(int modFd, int resFd) {
		return LoadVMAppBase(modFd, resFd);
	}
#else
	bool LoadVMApp(const char* modfile, const char* resfile) {
		return LoadVMAppBase(modfile, resfile);
	}
#endif
	bool LoadVMApp(Stream& stream, const char* combfile) {
		LOG("LoadVMApp...\n");
		if(!LoadVM(stream))
			return false;
		if(!mSyscall.loadResources(stream, combfile))
			return false;
		return true;
	}

	bool LoadVM(Stream& stream);

	int GetIp(void) const {
		if(mArmState)
			return ARMul_GetPC(mArmState);
		else
			return 0;
	}
	void SetIp(int) {
		DEBIG_PHAT_ERROR;
	}
	int GetValidatedStackValue(int offset) {
		int address = regs[13] + offset;
		if(((address&0x03)!=0) || uint(address)<STACK_BOTTOM || uint(address)>STACK_TOP)
			BIG_PHAT_ERROR(ERR_STACK_OOB);
		address>>=2;
		return mem_ds[address];
	}
	void InvokeSysCall(int syscall_id) {
		DEBIG_PHAT_ERROR;
	}
	void Run2() {
		DEBUG_ASSERT(mPC >= 0x8000);
		ARMul_SetPC(mArmState, mPC);
		ARMword oldPC = mPC;
		if(mGdbSignal == eStep) {
			LOG("step\n");
			mPC = ARMul_DoInstr(mArmState);
			ARMul_SetPC(mArmState, mPC);
			waitForRemote(mGdbSignal);
			LOG("PC: 0x%08x -> 0x%08x\n", oldPC, mPC);
		} else {
#if 1
		if(!mGdbOn) {
			mPC = ARMul_DoProg(mArmState);
			LOG("PC: 0x%08x -> 0x%08x\n", oldPC, mPC);
		} else
#endif
		{
			if(mem_ds[mPC >> 2] == (int)0xe7ffdefe) {	// breakpoint
				LOG("Breakpoint hit at 0x%08x\n", mPC);
				mGdbSignal = eBreakpoint;
				waitForRemote(mGdbSignal);
			} else {
				mPC = ARMul_DoInstr(mArmState);
				ARMul_SetPC(mArmState, mPC);
			}
		}
		}
	}


	bool getCustomReg(int regNum, int& val) {
		switch(regNum) {
		case 0x19:	//cpsr
			val = ARMul_GetCPSR(mArmState);
			break;
		default:
			return false;
		}
		return true;
	}
private:
	ARMul_State* mArmState;
	ARMword mPC;
	ARMword* mArmRegs;

//******************************************************************************
// syscall invoker
//******************************************************************************

#define REG(num) mArmRegs[num]
#define REG_r14 0
#define REG_i0 0
#define REG_i1 1
#define REG_i2 2
#define REG_i3 3

#define _MOSYNC_SYSCALL_ARGUMENTS_H_
#define SAFUNC
#include "syscall_arguments.h"
#undef _SYSCALL_CONVERT_double
#define _SYSCALL_CONVERT_double(a) _convert_double((int*)&a);
#undef _SYSCALL_CONVERT_float
#define _SYSCALL_CONVERT_float(a) _convert_float((int*)&a);

	void swiHandler(ARMword number) {
		switch(number) {
#include "invoke_syscall_cpp.h"
		default:
			LOG("SWI %i\n", number);
			DEBIG_PHAT_ERROR;
		}
		return;
	}
	friend unsigned swiHandler(ARMul_State * state, ARMword number, void* user);
};

VMCore* Core::CreateArmCore(Syscall& aSyscall) {
	return new ArmCore(aSyscall);
}

unsigned swiHandler(ARMul_State * state, ARMword number, void* user) {
	ArmCore* core = (ArmCore*)user;
	core->swiHandler(number);
	return 1;
}

static unsigned memErrHandler(ARMul_State * state, ARMword number, void* user) {
	LOG("Invalid memory access: 0x%x\n", number);
	BIG_PHAT_ERROR(ERR_MEMORY_OOB);
	return 0;
}

//******************************************************************************
// ELF loader
//******************************************************************************

static WORD swaph(WORD w) {
	return w;
}

static DWORD swapw(DWORD w) {
	return w;
}

bool ArmCore::LoadVM(Stream& file) {
	Elf32_Ehdr ehdr;

	// hard-coded size for now
	// ARM has only one memory segment; data and code are one.
	DATA_SEGMENT_SIZE = 64*1024*1024;
	mem_ds = new int[DATA_SEGMENT_SIZE / sizeof(int)];

	ARMul_EmulateInit();
	mArmState = ARMul_NewState();
	ARMul_CoProInit(mArmState);
	ARMul_SetSWIhandler(mArmState, (ARMul_SWIhandler*)::swiHandler, this);
	ARMul_SetMemErrHandler(mArmState, (ARMul_SWIhandler*)::memErrHandler);
	mArmRegs = ARMul_GetRegs(mArmState);
	regs = (int*)mArmRegs;

	ARMul_MemoryInit2(mArmState, mem_ds, DATA_SEGMENT_SIZE);

	// set the stack pointer
	mArmRegs[13] = DATA_SEGMENT_SIZE - 1024;
	STACK_TOP = mArmRegs[13];
	STACK_BOTTOM = 0;

	// fill registers with debug markers
	for(int i=0; i<13; i++) {
		regs[i] = i;
	}

#ifdef MEMORY_PROTECTION
	protectionSet = new byte[(DATA_SEGMENT_SIZE+7)>>3];
	ZEROMEM(protectionSet, (DATA_SEGMENT_SIZE+7)>>3);
#endif

	TEST(file.isOpen());
	TEST(file.readObject(ehdr));

#define EIMAG_CHECK(nr) (ehdr.e_ident[EI_MAG##nr] == ELFMAG##nr)

	if(!(EIMAG_CHECK(0) && EIMAG_CHECK(1) && EIMAG_CHECK(2) && EIMAG_CHECK(3))) {
		//FAIL(UE_INVALID_ELF);
		DEBIG_PHAT_ERROR;
	}

#define ELF_SWAPH(data) ((ehdr.e_ident[EI_DATA] == ELFDATA2MSB) ? swaph(data) : data)
#define ELF_SWAPW(data) ((ehdr.e_ident[EI_DATA] == ELFDATA2MSB) ? swapw(data) : data)

#define ELF_SWAP16(data) data = ELF_SWAPH(data)
#define ELF_SWAP32(data) data = ELF_SWAPW(data)

#define INVALID_INCOMPAT_CHECK(data, invalid_value, compatible_value) \
	if(data == invalid_value) { DEBIG_PHAT_ERROR; }\
	if(data != compatible_value) { LOG("%s = %X\n", #data, data);\
	DEBIG_PHAT_ERROR; }

	/*if(ehdr.e_ident[EI_CLASS] == ELFCLASSNONE)
	return InvalidELF;
	if(ehdr.e_ident[EI_CLASS] != ELFCLASS32)
	return IncompatibleELF;*/
	INVALID_INCOMPAT_CHECK(ehdr.e_ident[EI_CLASS], ELFCLASSNONE, ELFCLASS32);
	//INVALID_INCOMPAT_CHECK(ehdr.e_ident[EI_DATA], ELFDATANONE, ELFDATA2MSB);
	if(ehdr.e_ident[EI_DATA] != ELFDATA2MSB && ehdr.e_ident[EI_DATA] != ELFDATA2LSB) {
		DEBIG_PHAT_ERROR;
	}

	INVALID_INCOMPAT_CHECK(ELF_SWAPH(ehdr.e_type), ET_NONE, ET_EXEC);
	INVALID_INCOMPAT_CHECK(ELF_SWAPH(ehdr.e_machine), EM_NONE, EM_ARM);
	INVALID_INCOMPAT_CHECK(ELF_SWAPW(ehdr.e_version), EV_NONE, EV_CURRENT);

	if(ELF_SWAPH(ehdr.e_ehsize) != sizeof(Elf32_Ehdr)) {
		DEBIG_PHAT_ERROR;
	}
	if(ELF_SWAPH(ehdr.e_shentsize) != sizeof(Elf32_Shdr)) {
		DEBIG_PHAT_ERROR;
	}
	if(ELF_SWAPH(ehdr.e_phentsize) != sizeof(Elf32_Phdr)) {
		DEBIG_PHAT_ERROR;
	}

	// entry point
	mPC = ELF_SWAPW(ehdr.e_entry);
	ARMul_SetPC(mArmState, mPC);
	mArmRegs[14] = mPC;	//Link Register
	LOG("Entry point: 0x%x\n", mPC);

	{ //Read Section Table
		ELF_SWAP16(ehdr.e_shstrndx);
		ELF_SWAP16(ehdr.e_shnum);
		ELF_SWAP32(ehdr.e_shoff);

		char* strings = NULL;
		if(ehdr.e_shstrndx != 0) {
			Elf32_Shdr shdr;
			TEST(file.seek(Seek::Start, ehdr.e_shoff + ehdr.e_shstrndx * sizeof(Elf32_Shdr)));
			TEST(file.readObject(shdr));
			TEST(file.seek(Seek::Start, ELF_SWAPW(shdr.sh_offset)));
			ELF_SWAP32(shdr.sh_size);
			strings = new char[shdr.sh_size];
			TEST(file.read(strings, shdr.sh_size));
		}

		LOG("%i sections, offset %X:\n", ehdr.e_shnum, ehdr.e_shoff);
		for(int i=0; i<ehdr.e_shnum; i++) {
			Elf32_Shdr shdr;
			TEST(file.seek(Seek::Start, ehdr.e_shoff + i * sizeof(Elf32_Shdr)));
			TEST(file.readObject(shdr));
			ELF_SWAP32(shdr.sh_name);
			ELF_SWAP32(shdr.sh_addr);
			ELF_SWAP32(shdr.sh_size);
			if(ehdr.e_shstrndx == 0 && shdr.sh_name != 0) {
				DEBIG_PHAT_ERROR;
			}
			LOG("Name: %s(%i), Type: 0x%X, Flags: %08X, Address: %08X, Offset: 0x%X",
				(shdr.sh_name == 0) ? "" : (&strings[shdr.sh_name]), shdr.sh_name,
				ELF_SWAPW(shdr.sh_type), ELF_SWAPW(shdr.sh_flags),
				shdr.sh_addr, ELF_SWAPW(shdr.sh_offset));
			LOG(", Size: 0x%X, Link: 0x%X, Info: 0x%X, Addralign: %i, Entsize: %i\n",
				shdr.sh_size, ELF_SWAPW(shdr.sh_link), ELF_SWAPW(shdr.sh_info),
				ELF_SWAPW(shdr.sh_addralign), ELF_SWAPW(shdr.sh_entsize));

			//if(shdr.sh_name != 0) if(strcmp(&strings[shdr.sh_name], ".sbss") == 0 ||
			//strcmp(&strings[shdr.sh_name], ".bss") == 0)
			//memset(m.getp_translated(shdr.sh_addr, shdr.sh_size), 0, shdr.sh_size);
			//no need to zero bss, memeory's already zeroed.
		}
	}

	//DWORD ArenaLo = 0;

	{ //Read Program Table
		ELF_SWAP16(ehdr.e_phnum);
		ELF_SWAP32(ehdr.e_phoff);

		LOG("%i segments, offset %X:\n", ehdr.e_phnum, ehdr.e_phoff);
		for(int i=0; i<ehdr.e_phnum; i++) {
			Elf32_Phdr phdr;
			TEST(file.seek(Seek::Start, ehdr.e_phoff + i * sizeof(Elf32_Phdr)));
			TEST(file.readObject(phdr));
			ELF_SWAP32(phdr.p_type);
			ELF_SWAP32(phdr.p_offset);
			ELF_SWAP32(phdr.p_vaddr);
			ELF_SWAP32(phdr.p_filesz);
			ELF_SWAP32(phdr.p_flags);
			LOG("Type: 0x%X, Offset: 0x%X, VAddress: %08X, PAddress: %08X, Filesize: 0x%X",
				phdr.p_type, phdr.p_offset, phdr.p_vaddr, ELF_SWAPW(phdr.p_paddr), phdr.p_filesz);
			LOG(", Memsize: 0x%X, Flags: %08X, Align: %i\n",
				ELF_SWAPW(phdr.p_memsz), phdr.p_flags, ELF_SWAPW(phdr.p_align));
			if(phdr.p_type == PT_NULL)
				continue;
			else if(phdr.p_type == PT_LOAD) {
				LOG("%s section: 0x%x, 0x%x bytes\n",
					(phdr.p_flags & PF_X) ? "text" : "data",
					phdr.p_vaddr, phdr.p_filesz);
#if 0
				if(ArenaLo < phdr.p_vaddr + phdr.p_filesz)
					ArenaLo = phdr.p_vaddr + phdr.p_filesz;
#endif
				TEST(file.seek(Seek::Start, phdr.p_offset));
				TEST(file.read(this->GetValidatedMemRange(phdr.p_vaddr, phdr.p_filesz), phdr.p_filesz));
			} else {
				//FAIL(UE_INCOMPATIBLE_ELF);
			}

			/*else if(phdr.p_type == PT_NOTE) {
			Elf32_Nhdr nhdr;
			TEST(file.seek(phdr.p_offset));
			TEST(file.read(&nhdr, sizeof(nhdr)));
			ELF_SWAP32(nhdr.n_namesz);
			ELF_SWAP32(nhdr.n_descsz);
			ELF_SWAP32(nhdr.n_type);
			Container<char> name(nhdr.n_namesz), desc(nhdr.n_descsz);

			LOG("Note. Name size: %i, Desc size: %i, Type: %i, Name: %s, Desc: %s\n",

			}*/

		}	//if
	}	//for

	return true;
}
