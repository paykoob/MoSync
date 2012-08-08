#include "elfStabSld.h"

static void printFunctionName(FILE* file, const char* name);
static void printFunctionPrototype(FILE* file, const Function& f);
static void printFunctionContents(FILE* file, const Function& f);
static void parseFunctionInfo(Function& f);

void writeCpp(const DebuggingData& data, const char* cppName) {
	FILE* file = fopen(cppName, "w");

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

	for(set<Function>::iterator i=functions.begin(); i!=functions.end(); ++i) {
		const Function& f(*i);
		fputc('\n', file);
		printFunctionPrototype(file, f);
		fputs(" {\n", file);
		printFunctionContents(file, f);
		fputs("}\n", file);
	}

	fclose(file);
}

void writeCs(const DebuggingData& data, const char* csName) {
	printf("Not implemented!\n");
	exit(1);
}


static void printFunctionContents(FILE* file, const Function& f) {
	// todo: declare used registers (except parameter regs, which are already declared)

	// output instructions
	//printInstructions(file, f.start, f.end);
}

static void printFunctionPrototype(FILE* file, const Function& f) {
	fprintf(file, "static %s ", f.returnType);
	printFunctionName(file, f.name);
	fputc('(', file);
	bool first = true;
	for(int j=0; j<f.intParams; j++) {
		if(first)
			first = false;
		else
			fputs(", ", file);
		fprintf(file, "int p%i", j);
	}
	for(int j=0; j<f.floatParams; j++) {
		if(first)
			first = false;
		else
			fputs(", ", file);
		fprintf(file, "double f%i", 8+j);
	}
	fputc(')', file);
}

static void printFunctionName(FILE* file, const char* name) {
	const char* ptr = name;
	while(*ptr) {
		if(*ptr == ':')
			fputc('_', file);
		else
			fputc(*ptr, file);
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
		f.returnType = "void";
	else if(strncmp(type, "int", tlen) == 0)
		f.returnType = "int";
	else if(strncmp(type, "double", tlen) == 0)
		f.returnType = "double";
	else if(strncmp(type, "float", tlen) == 0)
		f.returnType = "double";
	else if(strncmp(type, "long", tlen) == 0)
		f.returnType = "int64_t";
	else
		DEBIG_PHAT_ERROR;

	const char* ip = comma + 1;
	comma = strchr(ip, ',');
	DEBUG_ASSERT(comma);
	int ilen = comma - ip;
	DEBUG_ASSERT(ilen == 1);
	DEBUG_ASSERT(isdigit(*ip));
	f.intParams = *ip - '0';

	const char* fp = comma + 1;
	int flen = strlen(fp);
	DEBUG_ASSERT(flen == 1);
	DEBUG_ASSERT(isdigit(*fp));
	f.floatParams = *fp - '0';
}


#if 0

static bool strbeg(const char* a, const char* b) {
	return strncmp(a, b, strlen(b)) == 0;
}

if(strend(str, '\\'))
	//combine with next line

#endif
