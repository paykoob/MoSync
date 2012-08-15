#include "elfStabSld.h"
#include "../../runtimes/cpp/core/gen-opcodes.h"

#include <sstream>

static void printFunctionName(FILE* file, const char* name);
static void printFunctionPrototype(FILE* file, const Function& f);
static void printFunctionContents(const DebuggingData& data, const Array0<byte>& bytes,
	FILE* file, const Function& f, CallRegs& cr, const Array0<Elf32_Rela>& textRela);
static void parseFunctionInfo(Function& f);
static bool readSegments(const DebuggingData& data, Array0<byte>& bytes);
static void setCallRegDataRefs(const DebuggingData& data, CallRegs& cr);
static void parseStabParams(const char* comma, unsigned& intParams, unsigned& floatParams);

void writeCpp(const DebuggingData& data, const char* cppName) {
	FILE* file = gOutputFile = fopen(cppName, "w");

	fputs(
"//****************************************\n"
"//          Generated Cpp code\n"
"//****************************************\n"
"\n"
"#include \"mstypeinfo.h\"\n"
"\n"
"// Prototypes\n"
"\n"
		,file);

	for(set<Function>::iterator i=functions.begin(); i!=functions.end(); ++i) {
		Function& f((Function&)*i);
		parseFunctionInfo(f);
		printFunctionPrototype(file, f);
		fputs(";\n", file);
	}

	fputs(
"\n"
"// Definitions\n"
		, file);

	Array0<byte> bytes;
	DEBUG_ASSERT(readSegments(data, bytes));

	CallRegs cr;
	setCallRegDataRefs(data, cr);

	for(set<Function>::iterator i=functions.begin(); i!=functions.end(); ++i) {
		const Function& f(*i);
		fputc('\n', file);
		printFunctionPrototype(file, f);
		fputs(" {\n", file);
		printFunctionContents(data, bytes, file, f, cr, data.textRela);
		fputs("}\n", file);
	}

	// CallReg
	fputc('\n', file);

	fclose(file);
}

void writeCs(const DebuggingData& data, const char* csName) {
	printf("Not implemented!\n");
	DEBIG_PHAT_ERROR;
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

static void setCallRegDataRefs(const DebuggingData& data, CallRegs& cr)
{
}

typedef const char* (*getRegNameFunc)(size_t);
static void printRegisterDeclarations(FILE* file, const char* type, unsigned regUsage, size_t nRegs,
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
				fputs(type, file);
			} else
				fputs(", ", file);
			fputs(grn(i), file);
		}
		if(!first)
			fputs(";\n", file);
	}
}

static void printFunctionContents(const DebuggingData& data, const Array0<byte>& bytes,
	FILE* file, const Function& f, CallRegs& cr, const Array0<Elf32_Rela>& textRela)
{
	// output instructions
	ostringstream os;
	SIData pid = { os, cr, data.elfFile, bytes, textRela, {0,0} };
	streamFunctionInstructions(pid, f);

	// declare registers
	printRegisterDeclarations(file, "int ", pid.regUsage.i, nIntRegs, REG_fp, REG_p0, REG_p3, f.ci.intParams, getIntRegName);
	printRegisterDeclarations(file, "double ", pid.regUsage.f, nFloatRegs, 0, 8, 15, f.ci.floatParams, getFloatRegName);

	fputs(os.str().c_str(), file);
}

// warning: must match enum ReturnType!
static const char* returnTypeStrings[] = {
	"void",
	"int",
	"double",
	"int64_t",
};

static void printFunctionPrototype(FILE* file, const Function& f) {
	fprintf(file, "static %s ", returnTypeStrings[f.ci.returnType]);
	printFunctionName(file, f.name);
	fputc('(', file);
	bool first = true;
	for(unsigned j=0; j<f.ci.intParams; j++) {
		if(first)
			first = false;
		else
			fputs(", ", file);
		fprintf(file, "int p%i", j);
	}
	for(unsigned j=0; j<f.ci.floatParams; j++) {
		if(first)
			first = false;
		else
			fputs(", ", file);
		fprintf(file, "double f%i", 8+j);
	}
	fputc(')', file);
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

static void printFunctionName(FILE* file, const char* name) {
	const char* ptr = name;
	while(*ptr) {
		if(isctype(*ptr))
			fputc(*ptr, file);
		else
			fputc('_', file);
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
