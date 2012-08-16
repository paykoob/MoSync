#include "elfStabSld.h"
#include "../../runtimes/cpp/core/gen-opcodes.h"

#include <sstream>
#include <fstream>

static void streamFunctionPrototype(ostream& os, const Function& f);
static void streamFunctionContents(const DebuggingData& data, const Array0<byte>& bytes,
	ostream& os, const Function& f, CallRegs& cr, const Array0<Elf32_Rela>& textRela);
static void parseFunctionInfo(Function& f);
static bool readSegments(const DebuggingData& data, Array0<byte>& bytes);
static void setCallRegDataRefs(const DebuggingData& data, CallRegs& cr);
static void parseStabParams(const char* comma, unsigned& intParams, unsigned& floatParams);
static void streamFunctionPrototypeParams(ostream& os, const CallInfo& ci, bool first = true);
static void streamCallRegPrototype(ostream& os, const CallInfo& ci);

// warning: must match enum ReturnType!
static const char* returnTypeStrings[] = {
	"void",
	"int",
	"double",
	"int64_t",
};

void writeCpp(const DebuggingData& data, const char* cppName) {
	Array0<byte> bytes;
	DEBUG_ASSERT(readSegments(data, bytes));

	// gotta do this first; the others rely on it.
	for(set<Function>::iterator i = functions.begin(); i != functions.end(); ++i) {
		Function& f((Function&)*i);
		parseFunctionInfo(f);
	}

	CallRegs cr;
	setCallRegDataRefs(data, cr);

	ofstream file(cppName);

	file <<
"//****************************************\n"
"//          Generated Cpp code\n"
"//****************************************\n"
"\n"
"#include \"mstypeinfo.h\"\n"
"\n"
"// Prototypes\n"
"\n";

	ostringstream oss;

	// normal function declarations
	for(set<Function>::iterator i = functions.begin(); i != functions.end(); ++i) {
		Function& f((Function&)*i);
		streamFunctionPrototype(oss, f);
		oss << ";\n";
	}

	oss <<
"\n"
"// Definitions\n";

	// normal function definitions.
	// this also populates the callReg map.
	for(set<Function>::iterator i=functions.begin(); i!=functions.end(); ++i) {
		const Function& f(*i);
		oss << '\n';
		streamFunctionPrototype(oss, f);
		oss << " {\n";
		streamFunctionContents(data, bytes, oss, f, cr, data.textRela);
		oss << "}\n";
	}

	// callReg declarations
	for(FunctionPointerMap::const_iterator itr = gFunctionPointerMap.begin();
		itr != gFunctionPointerMap.end(); ++itr)
	{
		const CallInfo& ci(itr->first);
		streamCallRegPrototype(file, ci);
		file << ";\n";
	}

	file << oss.str();

	// CallReg definitions
	for(FunctionPointerMap::const_iterator itr = gFunctionPointerMap.begin();
		itr != gFunctionPointerMap.end(); ++itr)
	{
		const CallInfo& ci(itr->first);
		const FunctionPointerSet& fps(itr->second);
		file << '\n';
		streamCallRegPrototype(file, ci);
		file << " {\n"
		"\tswitch(pointer) {\n";
		for(FunctionPointerSet::const_iterator jtr = fps.begin(); jtr != fps.end(); ++jtr) {
			file << "\tcase " << *jtr << ": ";
			if(ci.returnType != eVoid)
				file << "return ";
			Function dummy;
			dummy.start = *jtr;
			set<Function>::const_iterator ftr = functions.find(dummy);
			DEBUG_ASSERT(ftr != functions.end());
			const Function& cf(*ftr);
			streamFunctionCall(file, cf);
			if(ci.returnType == eVoid)
				file << "; return";
			file << ";\n";
		}
		file << "\tdefault: maPanic(pointer, \"Invalid callReg\");\n"
		"\t}\n"
		"}\n";
	}
}

void writeCs(const DebuggingData& data, const char* csName) {
	printf("Not implemented!\n");
	DEBIG_PHAT_ERROR;
}

static void streamCallRegPrototype(ostream& os, const CallInfo& ci) {
	os << "static " << returnTypeStrings[ci.returnType] << " ";
	streamCallRegName(os, ci);
	os << "(int pointer";
	streamFunctionPrototypeParams(os, ci, false);
}

bool readSegments(const DebuggingData& data, Array0<byte>& bytes)
{
	//Read Program Table
	//LOG("%i segments, offset %X:\n", data.e_phnum, data.e_phoff);

	// first, find the size of the data.
	unsigned size = 0;
	Stream& file(data.elfFile);
	const Elf32_Ehdr& ehdr(data.ehdr);
	for(unsigned i=0; i<ehdr.e_phnum; i++) {
		Elf32_Phdr phdr;
		TEST(file.seek(Seek::Start, ehdr.e_phoff + i * sizeof(Elf32_Phdr)));
		TEST(file.readObject(phdr));
#if 1
		LOG("Type: 0x%X, Offset: 0x%X, VAddress: %08X, PAddress: %08X, Filesize: 0x%X",
			phdr.p_type, phdr.p_offset, phdr.p_vaddr, (phdr.p_paddr), phdr.p_filesz);
		LOG(", Memsize: 0x%X, Flags: %08X, Align: %i\n",
			(phdr.p_memsz), phdr.p_flags, (phdr.p_align));
#endif
		if(phdr.p_type == PT_NULL)
			continue;
		else if(phdr.p_type == PT_LOAD) {
#if 1
			bool text = (phdr.p_flags & PF_X);
			LOG("%s segment: 0x%x, 0x%x bytes\n",
				text ? "text" : "data",
				phdr.p_vaddr, phdr.p_filesz);
#endif
			size = MAX(size, phdr.p_vaddr + phdr.p_filesz);
		} else {
			DEBIG_PHAT_ERROR;
		}
	}

	// then we can allocate the buffer and read the data.
	bytes.resize(size);

	for(unsigned i=0; i<ehdr.e_phnum; i++) {
		Elf32_Phdr phdr;
		TEST(file.seek(Seek::Start, ehdr.e_phoff + i * sizeof(Elf32_Phdr)));
		TEST(file.readObject(phdr));
		if(phdr.p_type == PT_NULL)
			continue;
		else if(phdr.p_type == PT_LOAD) {
			TEST(file.seek(Seek::Start, phdr.p_offset));
			void* dst = bytes + phdr.p_vaddr;
			LOG("Reading 0x%x bytes to %p...\n", phdr.p_filesz, dst);
			TEST(file.read(dst, phdr.p_filesz));
		} else {
			DEBIG_PHAT_ERROR;
		}
	}
	return true;
}

#if 0
typedef struct
{
  Elf32_Addr	r_offset;		/* Address */
  Elf32_Word	r_info;			/* Relocation type and symbol index */
  Elf32_Sword	r_addend;		/* Addend */
} Elf32_Rela;

typedef struct
{
  Elf32_Word	st_name;		/* Symbol name (string tbl index) */
  Elf32_Addr	st_value;		/* Symbol value */
  Elf32_Word	st_size;		/* Symbol size */
  unsigned char	st_info;		/* Symbol type and binding */
  unsigned char	st_other;		/* Symbol visibility */
  Elf32_Section	st_shndx;		/* Section index */
} Elf32_Sym;
#endif

void setCallRegDataRef(const Array0<Elf32_Sym>& symbols, const Elf32_Rela& r, CallRegs& cr) {
	const Elf32_Sym& sym(symbols[ELF32_R_SYM(r.r_info)]);
	//printf("stt: %i, val: %i, section %i\n", ELF32_ST_TYPE(sym.st_info), sym.st_value, sym.st_shndx);
	fflush(stdout);
	// assuming here that section 1 is .text
	if(ELF32_ST_TYPE(sym.st_info) == STT_FUNC ||
		(ELF32_ST_TYPE(sym.st_info) == STT_NOTYPE && sym.st_shndx == 1))
	{
		Function dummy;
		dummy.start = sym.st_value;
		set<Function>::const_iterator itr = functions.find(dummy);
		DEBUG_ASSERT(itr != functions.end());
		const Function& f(*itr);
		printf("Found function pointer to: %s, type %i\n", f.name, f.ci.returnType);
		fflush(stdout);
		gFunctionPointerMap[f.ci].insert(sym.st_value);
	}
}

static void setCallRegDataRefs(const DebuggingData& data, const Array0<Elf32_Rela>& rela, CallRegs& cr) {
	for(size_t i=0; i<rela.size(); i++) {
		const Elf32_Rela& r(rela[i]);
		setCallRegDataRef(data.symbols, r, cr);
	}
}

static void setCallRegDataRefs(const DebuggingData& data, CallRegs& cr)
{
	// We search the relocations in the data segment for references to the code segment,
	// which means function pointers.
	// Together with immediates that will be extracted from the code section,
	// this will be the list of functions that can be called by register.
	printf("rodata: %" PRIuPTR " relas\n", data.rodataRela.size());
	setCallRegDataRefs(data, data.rodataRela, cr);
	printf("data: %" PRIuPTR " relas\n", data.dataRela.size());
	setCallRegDataRefs(data, data.dataRela, cr);
	printf("Total callRegs: %" PRIuPTR "\n", gFunctionPointerMap.size());
	fflush(stdout);
}

typedef const char* (*getRegNameFunc)(size_t);
static void streamRegisterDeclarations(ostream& os, const char* type, unsigned regUsage, size_t nRegs,
	size_t start, size_t paramLow, size_t paramHi, unsigned nParams, getRegNameFunc grn)
{
	if(regUsage != 0) {
		bool first = true;
		for(size_t i=start; i<nRegs; i++) {
			if((regUsage & (1 << i)) == 0)
				continue;
			if(i >= paramLow && i <= paramHi && (i - paramLow) < nParams)
				continue;
			if(first) {
				first = false;
				os << type;
			} else
				os << ", ";
			os << grn(i);
		}
		if(!first)
			os << ";\n";
	}
}

static void streamFunctionContents(const DebuggingData& data, const Array0<byte>& bytes,
	ostream& os, const Function& f, CallRegs& cr, const Array0<Elf32_Rela>& textRela)
{
	// output instructions
	ostringstream oss;
	SIData pid = { oss, cr, data.elfFile, bytes, textRela, data.symbols, data.textRelocMap, {0,0} };
	streamFunctionInstructions(pid, f);

	// declare registers
	streamRegisterDeclarations(os, "int ", pid.regUsage.i, nIntRegs, REG_fp, REG_p0, REG_p3, f.ci.intParams, getIntRegName);
	streamRegisterDeclarations(os, "double ", pid.regUsage.f, nFloatRegs, 0, 8, 15, f.ci.floatParams, getFloatRegName);

	os << oss.str();
}

static void streamFunctionPrototypeParams(ostream& os, const CallInfo& ci, bool first) {
	if(first)
		os << '(';
	for(unsigned j=0; j<ci.intParams; j++) {
		if(first)
			first = false;
		else
			os << ", ";
		os << "int p" << j;
	}
	for(unsigned j=0; j<ci.floatParams; j++) {
		if(first)
			first = false;
		else
			os << ", ";
		os << "double f" << (8+j);
	}
	os << ')';
}

static void streamFunctionPrototype(ostream& os, const Function& f) {
	if(strcmp(f.name, "crt0_startup") != 0)
		os << "static ";
	os << returnTypeStrings[f.ci.returnType] << " ";
	streamFunctionName(os, f.name);
	streamFunctionPrototypeParams(os, f.ci);
}

static bool isctype(int c) {
	return isalnum(c) || c == '_';
}

void streamFunctionName(ostream& os, const char* name) {
	const char* ptr = name;
	while(*ptr) {
		if(isctype(*ptr))
			os << *ptr;
		else
			os << '_';
		ptr++;
	}
}

static void parseFunctionInfo(Function& f) {
	DEBUG_ASSERT(f.info);

	const char* type = f.info;
	const char* comma = strchr(type, ',');
	DEBUG_ASSERT(comma);
	int tlen = comma - type;
	if(strncmp(type, "void", tlen) == 0)
		f.ci.returnType = eVoid;
	else if(strncmp(type, "int", tlen) == 0)
		f.ci.returnType = eInt;
	else if(strncmp(type, "double", tlen) == 0)
		f.ci.returnType = eFloat;
	else if(strncmp(type, "float", tlen) == 0)
		f.ci.returnType = eFloat;
	else if(strncmp(type, "long", tlen) == 0)
		f.ci.returnType = eLong;
	else
		DEBIG_PHAT_ERROR;

	parseStabParams(comma, f.ci.intParams, f.ci.floatParams);
}

static void parseStabParams(const char* comma, unsigned& intParams, unsigned& floatParams) {
	const char* ip = comma + 1;
	comma = strchr(ip, ',');
	DEBUG_ASSERT(comma);
	int ilen = comma - ip;
	DEBUG_ASSERT(ilen == 1);
	DEBUG_ASSERT(isdigit(*ip));
	intParams = *ip - '0';

	const char* fp = comma + 1;
	int flen = strlen(fp);
	DEBUG_ASSERT(flen == 1);
	DEBUG_ASSERT(isdigit(*fp));
	floatParams = *fp - '0';
}

struct ReturnTypeMapping {
	const char* mode;
	ReturnType type;
};

// returns comma
static const char* parseReturnType(const char* stab, ReturnType& type) {
	static const ReturnTypeMapping mappings[] = {
		{ "VOID", eVoid },
		{ "SI", eInt },
		{ "DI", eLong },
		{ "SF", eFloat },
		{ "DF", eFloat },
	};

	const char* comma = strchr(stab, ',');
	DEBUG_ASSERT(comma);
	int len = comma - stab;
	for(size_t i=0; i<ARRAY_SIZE(mappings); i++) {
		const ReturnTypeMapping& m(mappings[i]);
		if(strncmp(m.mode, stab, len) == 0) {
			type = m.type;
			return comma;
		}
	}
	DEBIG_PHAT_ERROR;
}

CallInfo parseCallInfoStab(const char* stab) {
	CallInfo ci;
	const char* comma = parseReturnType(stab, ci.returnType);
	parseStabParams(comma, ci.intParams, ci.floatParams);
	return ci;
}



#if 0

static bool strbeg(const char* a, const char* b) {
	return strncmp(a, b, strlen(b)) == 0;
}

if(strend(str, '\\'))
	//combine with next line

#endif
