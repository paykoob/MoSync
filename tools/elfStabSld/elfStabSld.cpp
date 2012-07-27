#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "elf.h"
#include "FileStream.h"
#include <stdint.h>
#include "helpers/array.h"
#include "build/stabdefs.h"
#include <vector>
#include <set>

#define Log printf
#define DUMP_STABS 0

using namespace Base;
using namespace std;

typedef int32_t bfd_vma;

struct Stab {
	unsigned int n_strx;         /* index into string table of name */
	unsigned char n_type;         /* type of symbol */
	unsigned char n_other;        /* misc info (usually empty) */
	unsigned short n_desc;        /* description field */
	bfd_vma n_value;              /* value of symbol */
};

struct SLD {
	size_t address, line, filenum;
};

struct Function {
	const char* name;
	unsigned start;
	bool operator<(const Function& o) const { return start < o.start; }
};

struct Variable {
	const char* name;
	unsigned scope, address;
};

template<class T> class Array0 : public Array<T> {
public:
	Array0() : Array<T>(0) {}
};

struct DebuggingData {
	Array0<Stab> stabs;
	// todo: string tables, symbol table.
	// this is the .stabstr section
	Array0<char> stabstr;
	bfd_vma textSectionEndAddress;
};

static bool readStabs(const char* elfName, DebuggingData& data);
#if DUMP_STABS
static void dumpStabs(const DebuggingData& data);
#endif
static void writeSld(const DebuggingData& data, const char* sldName);

static const char* s_sldName;

int main(int argc, const char** argv) {
	if(argc != 3) {
		printf("Usage: elfStabSld <elf> <sld>\n");
		printf("\n");
		printf("Reads the .stab section of an elf file and outputs an sld file suitable for MoRE.\n");
		return 1;
	}
	const char* elfName = argv[1];
	const char* sldName = s_sldName = argv[2];

	DebuggingData data;
	if(!readStabs(elfName, data)) {
		printf("Could not read stabs!\n");
		return 1;
	}
#if DUMP_STABS
	dumpStabs(data);
#endif

	writeSld(data, sldName);

	remove("temp.sld");
	//rename(sldName, "temp.sld");
	return 0;
}

static void writeSld(const DebuggingData& data, const char* sldName) {
	// open file
	FILE* file = fopen(sldName, "w");

	// declare data for later passes
	vector<SLD> slds;
	set<Function> functions;
	vector<Variable> variables;

	// file list
	fputs("Files\n", file);
	//printf("data.stringSize: %" PRIuPTR "\n", data.stabstr.size());
	size_t strOffset = 0;
	size_t fileNum = 0;
	Function f;
	f.name = NULL;
	for(size_t i=0; i<data.stabs.size(); i++) {
		// file-header stab
		size_t stabEnd;
		size_t strTabFragSize;
		//if(s.n_type == N_UNDF)
		{
			const Stab& s(data.stabs[i]);
			DEBUG_ASSERT(s.n_type == N_UNDF);
			//printf("0x%" PRIxPTR ": strx: 0x%08x type: 0x%02x (%s) other: 0x%02x desc: 0x%04x value: 0x%x\n",
				//i, s.n_strx, s.n_type, stabName(s.n_type), s.n_other, s.n_desc, s.n_value);
			DEBUG_ASSERT(s.n_other == 0);
			//strOffset += s.n_strx;
			stabEnd = i + s.n_desc;
			strTabFragSize = s.n_value;
			DEBUG_ASSERT(stabEnd <= data.stabs.size());
			DEBUG_ASSERT(strOffset + strTabFragSize <= data.stabstr.size());
		}
		const char* stabstr = data.stabstr + strOffset;
		for(; i<stabEnd; i++) {
			const Stab& s(data.stabs[i]);
			// source file
			if(s.n_type == N_SO) {
				//printf("0x%" PRIxPTR ": strx: 0x%08x type: 0x%02x (%s) other: 0x%02x desc: 0x%04x value: 0x%x\n",
					//i, s.n_strx, s.n_type, stabName(s.n_type), s.n_other, s.n_desc, s.n_value);
				DEBUG_ASSERT(s.n_strx < strTabFragSize);
				const char* filename = stabstr + s.n_strx;
				//printf("filename?: %s\n", filename);
				if(filename[strlen(filename)-1] != '/') {
					fprintf(file, "%" PRIuPTR ":%" PRIuPTR ":%s\n", fileNum, fileNum, filename);
				}
			}
			// source line
			if(s.n_type == N_SLINE) {
				//printf("0x%" PRIxPTR ": strx: 0x%08x type: 0x%02x (%s) other: 0x%02x desc: 0x%04x value: 0x%x\n",
					//i, s.n_strx, s.n_type, stabName(s.n_type), s.n_other, s.n_desc, s.n_value);
				SLD sld = { s.n_value, s.n_desc, fileNum };
				slds.push_back(sld);
			}
			// function
			if(s.n_type == N_FUN) {
				//printf("0x%" PRIxPTR ": strx: 0x%08x type: 0x%02x (%s) other: 0x%02x desc: 0x%04x value: 0x%x\n",
					//i, s.n_strx, s.n_type, stabName(s.n_type), s.n_other, s.n_desc, s.n_value);
				//DEBUG_ASSERT(s.n_desc < 128);
				const char* name = stabstr + s.n_strx;
				unsigned address = s.n_value;
				//printf("%s 0x%02x 0x%x\n", name, s.n_desc, address);

				// insert function into ordered set
				// we can have duplicate function stabs (inlines or templates).
				// gotta get rid of them.
				f.name = name;
				f.start = address;
				functions.insert(f);
#if 0
				pair<set<Function>::iterator, bool> res = functions.insert(f);
				if(!res.second) {	// duplicate
					if(strcmp(res.first->name, f.name) != 0) {
						printf("Duplicate address 0x%x. Names: %s, %s\n", address, res.first->name, name);
						MoSyncErrorExit(1);
					}
				}
#endif
			}
			// variable
#if 0
			if(s.n_type == N_ROSYM || s.n_type == N_STSYM || s.n_type == N_LCSYM || s.n_type == N_GSYM) {
				printf("0x%" PRIxPTR ": strx: 0x%08x type: 0x%02x (%s) other: 0x%02x desc: 0x%04x value: 0x%x\n",
					i, s.n_strx, s.n_type, stabName(s.n_type), s.n_other, s.n_desc, s.n_value);
				const char* name = stabstr + s.n_strx;
				unsigned address = s.n_value;
				//Variable v = { name, fileNum, address };
				printf("%s %" PRIuPTR " 0x%x\n", name, fileNum, address);
			}
#endif
		}
		fileNum++;
		strOffset += strTabFragSize;
	}

	// output SLD
	fputs("SLD\n", file);
	for(size_t i=0; i<slds.size(); i++) {
		const SLD& sld(slds[i]);
		fprintf(file, "%" PRIxPTR ":%" PRIuPTR ":%" PRIuPTR "\n",
			sld.address, sld.line, sld.filenum);
	}

	// output FUNCTIONS
	fputs("FUNCTIONS\n", file);
	{
		set<Function>::const_iterator i=functions.begin();
		f = *i;
		for(++i; i!=functions.end(); ++i) {
			const Function& fi(*i);
			fprintf(file, "%s %x,%x\n",
				f.name, f.start, fi.start - 1);
			f = fi;
		}
		fprintf(file, "%s %x,%x\n",
			f.name, f.start, data.textSectionEndAddress);
	}

	// output VARIABLES
	fputs("VARIABLES\n", file);
	for(size_t i=0; i<variables.size(); i++) {
		const Variable& v(variables[i]);
		fprintf(file, "%s %i %x\n",
			v.name, v.scope, v.address);
	}
	fputs("\n", file);

	fclose(file);
}

#if DUMP_STABS
static void dumpStabs(const DebuggingData& data) {
	printf("%" PRIuPTR " stabs:\n", data.stabs.size());
	for(size_t i=0; i<data.stabs.size(); i++) {
		const Stab& s(data.stabs[i]);
		printf("%" PRIuPTR ", strx: 0x%08x type: 0x%02x (%s) other: 0x%02x desc: 0x%04x value: 0x%x\n",
			i, s.n_strx, s.n_type, stabName(s.n_type), s.n_other, s.n_desc, s.n_value);
	}
}
#endif

static bool readStabs(const char* elfName, DebuggingData& data) {
	Elf32_Ehdr ehdr;
	FileStream file(elfName);
	TEST(file.isOpen());
	TEST(file.readObject(ehdr));

#define EIMAG_CHECK(nr) (ehdr.e_ident[EI_MAG##nr] == ELFMAG##nr)

	if(!(EIMAG_CHECK(0) && EIMAG_CHECK(1) && EIMAG_CHECK(2) && EIMAG_CHECK(3))) {
		DEBIG_PHAT_ERROR;
	}

#define INVALID_INCOMPAT_CHECK(data, invalid_value, compatible_value) \
if(data == invalid_value) { LOG("%s = %s\n", #data, #invalid_value); DEBIG_PHAT_ERROR; }\
if(data != compatible_value) { LOG("%s = 0x%X\n", #data, data);\
DEBIG_PHAT_ERROR; }

	INVALID_INCOMPAT_CHECK(ehdr.e_ident[EI_CLASS], ELFCLASSNONE, ELFCLASS32);
	//INVALID_INCOMPAT_CHECK(ehdr.e_ident[EI_DATA], ELFDATANONE, ELFDATA2MSB);
	if(ehdr.e_ident[EI_DATA] != ELFDATA2MSB && ehdr.e_ident[EI_DATA] != ELFDATA2LSB) {
		DEBIG_PHAT_ERROR;
	}

#ifndef EM_MAPIP2
#define EM_MAPIP2 0x9127
#endif
	INVALID_INCOMPAT_CHECK((ehdr.e_type), ET_NONE, ET_EXEC);
	INVALID_INCOMPAT_CHECK((ehdr.e_machine), EM_NONE, EM_MAPIP2);
	INVALID_INCOMPAT_CHECK((ehdr.e_version), EV_NONE, EV_CURRENT);

	if((ehdr.e_ehsize) != sizeof(Elf32_Ehdr)) {
		DEBIG_PHAT_ERROR;
	}
	if((ehdr.e_shentsize) != sizeof(Elf32_Shdr)) {
		DEBIG_PHAT_ERROR;
	}
	if((ehdr.e_phentsize) != sizeof(Elf32_Phdr)) {
		DEBIG_PHAT_ERROR;
	}

	// entry point
	//LOG("Entry point: 0x%x\n", ehdr.e_entry);

	{ //Read Section Table
		// this is the ELF string table.
		// there are two other string tables, sections .stabstr and .strtab.
		char *strings = NULL;
		if(ehdr.e_shstrndx != 0) {
			Elf32_Shdr shdr;
			TEST(file.seek(Seek::Start, ehdr.e_shoff + ehdr.e_shstrndx * sizeof(Elf32_Shdr)));
			TEST(file.readObject(shdr));
			TEST(file.seek(Seek::Start, (shdr.sh_offset)));
			strings = new char[shdr.sh_size];
			TEST(file.read(strings, shdr.sh_size));
		}

		//LOG("%i sections, offset %X:\n", ehdr.e_shnum, ehdr.e_shoff);
		for(int i=0; i<ehdr.e_shnum; i++) {
			Elf32_Shdr shdr;
			TEST(file.seek(Seek::Start, ehdr.e_shoff + i * sizeof(Elf32_Shdr)));
			TEST(file.readObject(shdr));
			if(ehdr.e_shstrndx == 0 && shdr.sh_name != 0) {
				DEBIG_PHAT_ERROR;
			}
			const char* name = (shdr.sh_name == 0) ? "" : (&strings[shdr.sh_name]);
#if 0
			LOG("Name: %s(%i), Type: 0x%X, Flags: %08X, Address: %08X, Offset: 0x%X",
				name, shdr.sh_name,
				(shdr.sh_type), (shdr.sh_flags),
				shdr.sh_addr, (shdr.sh_offset));
			LOG(", Size: 0x%X, Link: 0x%X, Info: 0x%X, Addralign: %i, Entsize: %i\n",
				shdr.sh_size, (shdr.sh_link), (shdr.sh_info),
				(shdr.sh_addralign), (shdr.sh_entsize));
#endif

			if(strcmp(name, ".text") == 0) {
				data.textSectionEndAddress = shdr.sh_offset + shdr.sh_size;
			}
			if(strcmp(name, ".stab") == 0) {
				DEBUG_ASSERT(sizeof(Stab) == 12);
				int nStabs = shdr.sh_size / sizeof(Stab);
				DEBUG_ASSERT(sizeof(Stab) * nStabs == shdr.sh_size);
				data.stabs.resize(nStabs);
				TEST(file.seek(Seek::Start, shdr.sh_offset));
				TEST(file.read(data.stabs, shdr.sh_size));
			}
			if(strcmp(name, ".stabstr") == 0) {
				data.stabstr.resize(shdr.sh_size);
				TEST(file.seek(Seek::Start, shdr.sh_offset));
				TEST(file.read(data.stabstr, shdr.sh_size));
			}
		}
	}

#if 0
	//Read Program Table
	LOG("%i segments, offset %X:\n", ehdr.e_phnum, ehdr.e_phoff);
	for(int i=0; i<ehdr.e_phnum; i++) {
		Elf32_Phdr phdr;
		TEST(file.seek(Seek::Start, ehdr.e_phoff + i * sizeof(Elf32_Phdr)));
		TEST(file.readObject(phdr));
		LOG("Type: 0x%X, Offset: 0x%X, VAddress: %08X, PAddress: %08X, Filesize: 0x%X",
			phdr.p_type, phdr.p_offset, phdr.p_vaddr, (phdr.p_paddr), phdr.p_filesz);
		LOG(", Memsize: 0x%X, Flags: %08X, Align: %i\n",
			(phdr.p_memsz), phdr.p_flags, (phdr.p_align));
		if(phdr.p_type == PT_NULL)
			continue;
		else if(phdr.p_type == PT_LOAD) {
			bool text = (phdr.p_flags & PF_X);
			LOG("%s section: 0x%x, 0x%x bytes\n",
				text ? "text" : "data",
				phdr.p_vaddr, phdr.p_filesz);
#if 0
			TEST(file.seek(Seek::Start, phdr.p_offset));
			void* dst = mem_ds;
			if(phdr.p_vaddr != 0)
				dst = this->GetValidatedMemRange(phdr.p_vaddr, phdr.p_filesz);
			LOG("Reading 0x%x bytes to %p...\n", phdr.p_filesz, dst);
			TEST(file.read(dst, phdr.p_filesz));
#endif
		} else {
			DEBIG_PHAT_ERROR;
		}
	}	//for
#endif
	return true;
}

void MoSyncErrorExit(int code) {
	printf("MoSyncErrorExit(%i)\n", code);
	remove(s_sldName);
	exit(code);
}
