#include "elfStabSld.h"

static bool readStabs(Stream& elfFile, DebuggingData& data, bool cOutput);
#if DUMP_STABS
static void dumpStabs(const DebuggingData& data);
#endif
static void writeSld(const DebuggingData& data, const char* sldName);
static void parseStabs(const DebuggingData& data, bool cOutput);

static const char* s_outputName;

vector<File> files;
vector<SLD> slds;
set<Function> functions;
CallMap gCallMap;
FunctionPointerMap gFunctionPointerMap;
FILE* gOutputFile = NULL;

#ifdef main
#undef main
#endif

enum Mode {
	eSLD,
	eCPP,
	eCS,
};

#ifdef main
#undef main
#endif

int main(int argc, const char** argv) {
	if(argc < 3) {
		printf("Usage: elfStabSld [options] <input> <output>\n");
		printf("\n");
		printf("Reads the .stab section of an elf file and outputs a text file, depending on options:\n");
		printf(" Default: An sld file suitable for MoRE.\n");
		printf(" -cpp\tC++ code suitable for the iOS runtime.\n");
		printf(" -cs\tC# code suitable for the Windows Phone runtime.\n");
		return 1;
	}

	Mode mode = eSLD;
	bool cOutput = false;

	// parse options
	int i = 1;
	while(argv[i][0] == '-') {
		const char* a = argv[i];
		if(strcmp(a, "-cpp") == 0) {
			mode = eCPP;
			cOutput = true;
		} else if(strcmp(a, "-cs") == 0) {
			mode = eCS;
			cOutput = true;
		} else {
			printf("Unrecognized option: %s\n", a);
			exit(1);
		}
		i++;
	}

	const char* elfName = argv[i];
	s_outputName = argv[i+1];

	FileStream file(elfName);
	DebuggingData data(file);
	if(!readStabs(file, data, cOutput)) {
		printf("Could not read stabs!\n");
		return 1;
	}
#if DUMP_STABS
	dumpStabs(data);
#endif

	parseStabs(data, cOutput);

	switch(mode) {
	case eSLD:
		writeSld(data, s_outputName);
		break;
	case eCPP:
		writeCpp(data, s_outputName);
		break;
	case eCS:
		writeCs(data, s_outputName);
		break;
	}

	return 0;
}

static void parseStabs(const DebuggingData& data, bool cOutput) {
	//printf("data.stringSize: %" PRIuPTR "\n", data.stabstr.size());
	size_t strOffset = 0;
	size_t fileNum = 0;
	Function f;
	f.name = NULL;
	f.info = "";
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
					File file;
					file.name = filename;
					files.push_back(file);
				}
			}
			else
			// source line
			if(s.n_type == N_SLINE) {
				//printf("0x%" PRIxPTR ": strx: 0x%08x type: 0x%02x (%s) other: 0x%02x desc: 0x%04x value: 0x%x\n",
					//i, s.n_strx, s.n_type, stabName(s.n_type), s.n_other, s.n_desc, s.n_value);
				SLD sld = { (size_t)s.n_value, s.n_desc, fileNum };
				slds.push_back(sld);
			}
			else
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
#if HAVE_EMPTY_NFUN
				if(*name) {
					DEBUG_ASSERT(!f.name);
					f.name = name;
					f.start = address;
					f.info = NULL;
				} else {
					DEBUG_ASSERT(f.info != NULL);
					f.end = f.start + address - 1;
					functions.insert(f);
					f.name = NULL;
				}
#else
				f.name = name;
				f.start = address;
				f.scope = fileNum;
				functions.insert(f);
#endif
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
			else
#if 0
			// variable
			if(s.n_type == N_ROSYM || s.n_type == N_STSYM || s.n_type == N_LCSYM || s.n_type == N_GSYM) {
				//printf("0x%" PRIxPTR ": strx: 0x%08x type: 0x%02x (%s) other: 0x%02x desc: 0x%04x value: 0x%x\n",
					//i, s.n_strx, s.n_type, stabName(s.n_type), s.n_other, s.n_desc, s.n_value);
				const char* name = stabstr + s.n_strx;
				unsigned address = s.n_value;
				//Variable v = { name, fileNum, address };
				//printf("%s %" PRIuPTR " 0x%x\n", name, fileNum, address);
			}
			else
#endif
			// MoSync custom stab: function return & parameter info.
			if(s.n_type == N_MOSYNC) {
				//printf("0x%" PRIxPTR ": strx: 0x%08x type: 0x%02x (%s) other: 0x%02x desc: 0x%04x value: 0x%x\n",
					//i, s.n_strx, s.n_type, stabName(s.n_type), s.n_other, s.n_desc, s.n_value);
				const char* stab = stabstr + s.n_strx;
				switch(s.n_other) {
				case 0:	// function info
#if 0
				if(f.info != NULL) {
						// weak functions (like inline class methods) may have duplicate implementations.
						// the code is removed, but the stabs are not.
						// ensure that the info is identical before accepting a dupe.
						if(strcmp(f.info, stab) != 0) {
							printf("Duplicate info mismatch. Old: %s  New: %s\n", f.info, stab);
#if 1
						printf("0x%" PRIxPTR ": strx: 0x%08x type: 0x%02x (%s) other: 0x%02x desc: 0x%04x value: 0x%x\n",
							i, s.n_strx, s.n_type, stabName(s.n_type), s.n_other, s.n_desc, s.n_value);
						printf("info: %s\n", stab);
						printf("old function: %s\n", f.name);
						printf("old info: %s\n", f.info);
#endif
							DEBIG_PHAT_ERROR;
						}
					}
#else
					DEBUG_ASSERT(f.info == NULL);
					DEBUG_ASSERT(f.name != NULL);
#endif
					f.info = stab;
					//printf("info: %s\n", f.info);
					break;
				case 1:	// call info
					if(cOutput) {
						pair<CallMap::iterator, bool> res = gCallMap.insert(pair<unsigned, CallInfo>(s.n_value, parseCallInfoStab(stab)));
						DEBUG_ASSERT(res.second);
					}
					break;
				default:
					DEBIG_PHAT_ERROR;
				}
			}
#if 0
			else
			{
				// print all unhandled stabs
				printf("0x%" PRIxPTR ": strx: 0x%08x type: 0x%02x (%s) other: 0x%02x desc: 0x%04x value: 0x%x\n",
					i, s.n_strx, s.n_type, stabName(s.n_type), s.n_other, s.n_desc, s.n_value);
			}
#endif
		}
		fileNum++;
		strOffset += strTabFragSize;
	}
	if(cOutput) {
		printf("Found %" PRIuPTR " function calls.\n", gCallMap.size());
	}
	DEBUG_ASSERT(!f.name);
}

static void writeSld(const DebuggingData& data, const char* sldName) {
	// open file
	FILE* file = fopen(sldName, "w");

	// dummy
	vector<Variable> variables;

	// output Files
	fputs("Files\n", file);
	for(size_t i=0; i<files.size(); i++) {
		fprintf(file, "%" PRIuPTR ":%" PRIuPTR ":%s\n", i, i, files[i].name);
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
#if HAVE_EMPTY_NFUN	// empty N_FUNs marks the length of a function.
		set<Function>::const_iterator i=functions.begin();
		for(; i!=functions.end(); ++i) {
			const Function& fi(*i);
			fprintf(file, "%s %x,%x\n",
				fi.name, fi.start, fi.end);
		}
#else
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
#endif
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

static bool readStabs(Stream& file, DebuggingData& data, bool readCOutputData) {
	Elf32_Ehdr& ehdr(data.ehdr);
	TEST(file.isOpen());
	TEST(file.readObject(ehdr));

#define EIMAG_CHECK(nr) (ehdr.e_ident[EI_MAG##nr] == ELFMAG##nr)

	if(!(EIMAG_CHECK(0) && EIMAG_CHECK(1) && EIMAG_CHECK(2) && EIMAG_CHECK(3))) {
		DEBIG_PHAT_ERROR;
	}

#define INVALID_INCOMPAT_CHECK(data, invalid_value, compatible_value) \
if(data == invalid_value) { LOG("%s = %s\n", #data, #invalid_value); DEBIG_PHAT_ERROR; }\
if(data != compatible_value) { LOG("%s = 0x%X\n", #data, (uint32_t)data);\
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
#if 1
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
			if(readCOutputData) {
				Array0<Elf32_Rela>* relaP = NULL;
				if(strcmp(name, ".rela.text") == 0)
					relaP = &data.textRela;
				else if(strcmp(name, ".rela.rodata") == 0)
					relaP = &data.rodataRela;
				else if(strcmp(name, ".rela.data") == 0)
					relaP = &data.dataRela;
				if(relaP) {
					Array0<Elf32_Rela>& rela(*relaP);
					DEBUG_ASSERT(shdr.sh_size % sizeof(Elf32_Rela) == 0);
					rela.resize(shdr.sh_size / sizeof(Elf32_Rela));
					TEST(file.seek(Seek::Start, shdr.sh_offset));
					TEST(file.read(rela, shdr.sh_size));
					printf("%s: %" PRIuPTR " entries.\n", name, rela.size());

					if(relaP == &data.textRela) {
						for(unsigned j=0; j<rela.size(); j++) {
							const Elf32_Rela& r(rela[j]);
							pair<RelocMap::iterator, bool> res = data.textRelocMap.insert(
								pair<unsigned, unsigned>(r.r_offset, j));
							DEBUG_ASSERT(res.second);
						}
					}
				}
			}
			// to use the relocation tables, we'll need the symbol table.
			if(readCOutputData && strcmp(name, ".symtab") == 0) {
				DEBUG_ASSERT(shdr.sh_size % sizeof(Elf32_Sym) == 0);
				data.symbols.resize(shdr.sh_size / sizeof(Elf32_Sym));
				TEST(file.seek(Seek::Start, shdr.sh_offset));
				TEST(file.read(data.symbols, shdr.sh_size));
				printf("%s: %" PRIuPTR " symbols.\n", name, data.symbols.size());
			}
			if(readCOutputData && strcmp(name, ".strtab") == 0) {
				data.strtab.resize(shdr.sh_size);
				TEST(file.seek(Seek::Start, shdr.sh_offset));
				TEST(file.read(data.strtab, shdr.sh_size));
			}
		}
	}
	return true;
}

void MoSyncErrorExit(int code) {
	printf("MoSyncErrorExit(%i)\n", code);
	printf("remove(%s)\n", s_outputName);
	if(gOutputFile)
		fclose(gOutputFile);
	gOutputFile = NULL;
	//remove(s_outputName);
	exit(code);
}
