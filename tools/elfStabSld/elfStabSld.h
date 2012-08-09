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

#define Log printf
#define DUMP_STABS 0

#define HAVE_EMPTY_NFUN 1

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

struct Parameter {
	const char* stab;
	int regnum;
};

enum ReturnType {
	eVoid,
	eInt,
	eFloat,
	eLong,
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
	ReturnType returnType;
	int intParams, floatParams;
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

struct DebuggingData {
	DebuggingData(Stream& ef) : elfFile(ef) {}
	Stream& elfFile;
	Array0<Stab> stabs;
	// todo: string tables, symbol table.
	// this is the .stabstr section
	Array0<char> stabstr;
	bfd_vma textSectionEndAddress;
	unsigned e_phnum, e_phoff;
};

extern vector<File> files;
extern vector<SLD> slds;
extern set<Function> functions;
extern FILE* gOutputFile;

void writeCpp(const DebuggingData& data, const char*);
void writeCs(const DebuggingData& data, const char*) __attribute__((noreturn));

struct SIData {
	// input
	ostream& stream;
	Stream& elfFile;
	const Array0<byte>& bytes;

	// output
	struct RegUsage {
		uint32_t i;
		uint16_t f;
	} regUsage;
};

void streamFunctionInstructions(SIData& data, const Function& f);
void streamFunctionName(ostream& os, const char* name);
