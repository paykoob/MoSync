#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <elf.h>
#include "FileStream.h"
#include <stdint.h>
#include "helpers/array.h"
#include "build/stabdefs.h"
#include <vector>
#include <set>
#include <ostream>
#include <unordered_map>
#include <map>

#define Log printf
#define DUMP_STABS 0

#define HAVE_EMPTY_NFUN 1

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

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

enum ReturnType {
	eVoid,
	eInt,
	eFloat,
	eLong,
};

struct CallInfo {
	ReturnType returnType;
	unsigned intParams, floatParams;

	bool operator<(const CallInfo& o) const {
		if(returnType != o.returnType)
			return returnType < o.returnType;
		if(intParams != o.intParams)
			return intParams < o.intParams;
		return floatParams < o.floatParams;
	}
};

struct Function {
	// filled by parser
	const char* name;
	unsigned start;
#if HAVE_EMPTY_NFUN
	unsigned end;
#endif
	unsigned scope;
	const char* info;

	// filled by writer
	CallInfo ci;
	bool operator<(const Function& o) const { return start < o.start; }
};

struct Variable {
	const char* name;
	unsigned scope, address;
};

struct File {
	const char* name;
};

template<class T> class Array0 : public Array<T> {
public:
	Array0() : Array<T>(0) {}
};

// reloc address => rela index
typedef unordered_map<unsigned, unsigned> RelocMap;

struct DebuggingData {
	DebuggingData(Stream& ef) : elfFile(ef) {}
	Stream& elfFile;
	Elf32_Ehdr ehdr;
	Array0<Stab> stabs;
	// this is the .stabstr section
	Array0<char> stabstr;
	bfd_vma textSectionEndAddress;
	// these are valid only in cOutput mode.
	Array0<Elf32_Rela> textRela, rodataRela, dataRela;
	Array0<Elf32_Sym> symbols;
	Array0<char> strtab;	// this one is used by symbols.
	RelocMap textRelocMap;
};

// stores the set of addresses to functions that may be called by register.
// hopefully, this set should remain small.
typedef set<unsigned> CallRegs;

// key: return address
typedef unordered_map<unsigned, CallInfo> CallMap;

// function address, index to functions
typedef set<unsigned> FunctionPointerSet;

typedef map<CallInfo, FunctionPointerSet> FunctionPointerMap;

extern vector<File> files;
extern vector<SLD> slds;
extern set<Function> functions;
extern CallMap gCallMap;
extern FunctionPointerMap gFunctionPointerMap;
extern FILE* gOutputFile;

void writeCpp(const DebuggingData& data, const char*);
void writeCs(const DebuggingData& data, const char*) __attribute__((noreturn));

struct SIData {
	// input
	ostream& stream;
	CallRegs& cr;
	Stream& elfFile;
	const Array0<byte>& textBytes, & dataBytes;
	const Array0<Elf32_Rela>& textRela;
	const Array0<Elf32_Sym>& symbols;
	const Array0<char>& strtab;
	const RelocMap& textRelocMap;

	// output
	struct RegUsage {
		uint32_t i;
		uint16_t f;
	} regUsage;
};

void streamFunctionInstructions(SIData& data, const Function& f);
void streamFunctionName(ostream& os, const char* name);
void streamCallRegName(ostream& os, const CallInfo& ci);
void streamFunctionCall(ostream& os, const Function& cf);

CallInfo parseCallInfoStab(const char* stab);

void setCallRegDataRef(const Array0<Elf32_Sym>& symbols, const Array0<char>& strtab, const Elf32_Rela& r, CallRegs& cr);

const char* getIntRegName(size_t r);
const char* getFloatRegName(size_t r);
extern const size_t nIntRegs, nFloatRegs;
