#include "elfStabSld.h"

struct CCore {
	unsigned printInstruction(unsigned ip);

	ostream& os;
	const Function& f;
	const byte* bytes;
};

void streamFunctionInstructions(SIData& data, const Function& f) {
	printf("printInstructions(0x%x => 0x%x)\n", f.start, f.end);
	DEBUG_ASSERT(f.end >= f.start);
	CCore core = { data.stream, f, data.bytes };
	unsigned ip = f.start;
	while(ip <= f.end) {
		ip = core.printInstruction(ip);
	}
	DEBUG_ASSERT(ip == f.end + 1);
}

#include "build/gen-regnames.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

static const char* getIntRegName(size_t r) {
	if(r < ARRAY_SIZE(mapip2_register_names)) {
		return mapip2_register_names[r];
	} else {
		DEBIG_PHAT_ERROR;
	}
};

static const char* getFloatRegName(size_t r) {
	if(r < ARRAY_SIZE(mapip2_float_register_names)) {
		return mapip2_float_register_names[r];
	} else {
		DEBIG_PHAT_ERROR;
	}
};

#include "../../runtimes/cpp/core/core_helpers.h"
#include "../../runtimes/cpp/core/gen-opcodes.h"

#define IB ((int)(bytes[ip++]))

#define IMM imm32
#define IMMU "(unsigned)" << imm32
#define REG(r) getIntRegName(r)
#define RS REG(rs)
#define RD REG(rd)
#define RSU "(unsigned)" << RS
#define RDU "(unsigned)" << RD

#define ARITH(rdst, a, oper, b) os << REG(rdst) << " = " << a << " " #oper " " << b << ";"

#define DIVIDE(rdst, a, b) ARITH(rdst, a, /, b)

#define OPC(a) case OP_##a: os << "\t";
#define EOP os << "\n"; break

#define WRITE_REG(rd, src) os << REG(rd) << " = " << src << ";"

#define FREG(r) getFloatRegName(r)
#define FRD FREG(rd)
#define FRS FREG(rs)

#define FRDd FRD << ".d"
#define FRSd FRS << ".d"

class label {
public:
	unsigned mIp;
	label(unsigned ip) : mIp(ip) {}
	template<class charT, class Traits>
  friend std::basic_ostream<charT, Traits>&
		operator<<(std::basic_ostream<charT, Traits >& os, const label& l);
};

template<class charT, class Traits>
std::basic_ostream<charT, Traits>&
	operator<<(std::basic_ostream<charT, Traits >& os, const label& l)
{
	os << "L" << l.mIp;
	return os;
}

static void streamFunctionCall(ostream& os, const Function& cf) {
	streamFunctionName(os, cf.name);
	os << "(";
	bool first = true;
	for(int i=0; i<cf.intParams; i++) {
		if(first)
			first = false;
		else
			os << ", ";
		os << REG(REG_p0 + i);
	}
	for(int i=0; i<cf.floatParams; i++) {
		if(first)
			first = false;
		else
			os << ", ";
		os << FREG(8 + i);
	}
	os << ")";
}

unsigned CCore::printInstruction(unsigned ip) {
	os << "L" << ip << ":\t";

	byte op = IB;
	int imm32;
	byte rs, rd;

	switch (op)
	{
		OPC(ADD)	FETCH_RD_RS	ARITH(rd, RD, +, RS);	EOP;
		OPC(ADDI)	FETCH_RD_CONST	ARITH(rd, RD, +, IMM);	EOP;
		OPC(SUB)	FETCH_RD_RS	ARITH(rd, RD, -, RS);	EOP;
		OPC(SUBI)	FETCH_RD_CONST	ARITH(rd, RD, -, IMM);	EOP;
		OPC(MUL)	FETCH_RD_RS	ARITH(rd, RD, *, RS);	EOP;
		OPC(MULI)	FETCH_RD_CONST	ARITH(rd, RD, *, IMM);	EOP;
		OPC(AND)	FETCH_RD_RS	ARITH(rd, RD, &, RS);	EOP;
		OPC(ANDI)	FETCH_RD_CONST	ARITH(rd, RD, &, IMM);	EOP;
		OPC(OR)	FETCH_RD_RS	ARITH(rd, RD, |, RS);	EOP;
		OPC(ORI)	FETCH_RD_CONST	ARITH(rd, RD, |, IMM);	EOP;
		OPC(XOR)	FETCH_RD_RS	ARITH(rd, RD, ^, RS);	EOP;
		OPC(XORI)	FETCH_RD_CONST	ARITH(rd, RD, ^, IMM);	EOP;
		OPC(DIVU)	FETCH_RD_RS	DIVIDE(rd, RDU, RSU);	EOP;
		OPC(DIVUI)	FETCH_RD_CONST	DIVIDE(rd, RDU, IMMU);	EOP;
		OPC(DIV)	FETCH_RD_RS	DIVIDE(rd, RD, RS);		EOP;
		OPC(DIVI)	FETCH_RD_CONST	DIVIDE(rd, RD, IMM);	EOP;
		OPC(SLL)	FETCH_RD_RS	ARITH(rd, RDU, <<, RSU);	EOP;
		OPC(SLLI)	FETCH_RD_IMM8	ARITH(rd, RDU, <<, IMMU);	EOP;
		OPC(SRA)	FETCH_RD_RS	ARITH(rd, RD, >>, RS);	EOP;
		OPC(SRAI)	FETCH_RD_IMM8	ARITH(rd, RD, >>, IMM);	EOP;
		OPC(SRL)	FETCH_RD_RS	ARITH(rd, RDU, >>, RSU);	EOP;
		OPC(SRLI)	FETCH_RD_IMM8	ARITH(rd, RDU, >>, IMMU);	EOP;

		OPC(NOT)	FETCH_RD_RS	os << RD << " = ~" << RS;	EOP;
		OPC(NEG)	FETCH_RD_RS	os << RD << " = -" << RS;	EOP;

		OPC(PUSH)	FETCH_RD_RS
		{
			byte n = (rs - rd) + 1;
			if(rd < 2 || int(rd) + n > 32 || n == 0) {
				DUMPINT(rd);
				DUMPINT(n);
				DEBIG_PHAT_ERROR; //raise hell
			}

			os << "// push " << RD << ", " << RS << ";";
		}
		EOP;

		OPC(POP) FETCH_RD_RS
		{
			byte n = (rs - rd) + 1;
			if(rd > 31 || int(rs) - n < 1 || n == 0) {
				DUMPINT(rd);
				DUMPINT(rs);
				DUMPINT(n);
				DEBIG_PHAT_ERROR; //raise hell
			}

			os << "// pop " << RD << ", " << RS << ";";
		}
		EOP;

		OPC(FPUSH) FETCH_FRD_FRS
		{
			byte n = (rs - rd) + 1;
			if(int(rd) + n > 15 || n == 0) {
				DUMPINT(rd);
				DUMPINT(n);
				DEBIG_PHAT_ERROR; //raise hell
			}

			os << "// fpush " << FRD << ", " << FRS << ";";
		}
		EOP;

		OPC(FPOP) FETCH_FRD_FRS
		{
			byte n = (rs - rd) + 1;
			if(rd > 15 || n <= 0) {
				DUMPINT(rd);
				DUMPINT(rs);
				DUMPINT(n);
				DEBIG_PHAT_ERROR; //raise hell
			}

			os << "// fpop " << FRD << ", " << FRS << ";";
		}
		EOP;

		OPC(LDB)
		{
			FETCH_RD_RS_CONST
			os << RD << " = RBYTE(" << RS << " + " << IMM << ");";
		}
		EOP;

		OPC(LDH)
		{
			FETCH_RD_RS_CONST
			os << RD << " = RSHORT(" << RS << " + " << IMM << ");";
		}
		EOP;

		OPC(LDW)
		{
			FETCH_RD_RS_CONST
			os << RD << " = RINT(" << RS << " + " << IMM << ");";
		}
		EOP;

		OPC(STB)
		{
			FETCH_RD_RS_CONST
			os << "WBYTE(" << RD << " + " << IMM << ", " << RS << ");";
		}
		EOP;

		OPC(STH)
		{
			FETCH_RD_RS_CONST
			os << "WSHORT(" << RD << " + " << IMM << ", " << RS << ");";
		}
		EOP;

		OPC(STW)
		{
			FETCH_RD_RS_CONST
			os << "WINT(" << RD << " + " << IMM << ", " << RS << ");";
		}
		EOP;

		OPC(LDI)	FETCH_RD_CONST	WRITE_REG(rd, IMM);	EOP;
		OPC(LDR)	FETCH_RD_RS	WRITE_REG(rd, RS);	EOP;

		OPC(LDDR) FETCH_RD_RS WRITE_REG(rd, RS); os << " "; WRITE_REG(rd+1, REG(rs+1)); EOP;
		OPC(LDDI) FETCH_RD_CONST WRITE_REG(rd, IMM); os << " "; FETCH_CONST; WRITE_REG(rd+1, IMM); EOP;

		OPC(FLOATS) FETCH_RD_RS os << FRD << ".d = (double)(signed)" << RS << ";"; EOP;
		OPC(FLOATUNS) FETCH_RD_RS os << FRD << ".d = (double)(unsigned)" << RS << ";"; EOP;

		OPC(FLOATD)
			FETCH_RD_RS;
			os << "{ FREG temp; "
			"temp.i[0] = " << RS << "; "
			"temp.i[1] = " << REG(rs+1) << "; "
			<< FRD << ".d = (double)(signed long long)temp.ll; }";
		EOP;

		OPC(FLOATUND)
			FETCH_RD_RS;
			os << "{ FREG temp; "
			"temp.i[0] = " << RS << "; "
			"temp.i[1] = " << REG(rs+1) << "; "
			<< FRD << ".d = (double)(unsigned long long)temp.ll; }";
		EOP;

		OPC(FSTRS)
			FETCH_RD_RS os << "{ MA_FV fv; ";
			os << "fv.f = (float)" << FRS << ".d; ";
			WRITE_REG(rd, "fv.i");
			os << " }";
		EOP;
		OPC(FSTRD) FETCH_RD_RS WRITE_REG(rd, FRS << ".i[0]"); os << "; "; WRITE_REG(rd+1, FRS << ".i[1]"); EOP;

		OPC(FLDRS) FETCH_RD_RS os << "{ MA_FV fv; fv.i = " << RS << "; " << FRD << ".d = (double)fv.f; }"; EOP;
		OPC(FLDRD) FETCH_RD_RS os << FRD << ".i[0] = " << RS << "; " << FRD << ".i[1] = " << REG(rs+1); EOP;

		OPC(FLDR) FETCH_FRD_FRS os << FRD << ".d = " << FRS << ".d"; EOP;

		OPC(FLDIS) FETCH_RD_CONST os << "{ MA_FV fv; fv.i = " << IMM << "; " << FRD << ".d = (double)fv.f; }"; EOP;
		OPC(FLDID) FETCH_RD_CONST os << FRD << ".i[0] = " << IMM << "; "; FETCH_CONST; os << FRD << ".i[1] = " << IMM << ";"; EOP;

		OPC(FIX_TRUNCS) FETCH_RD_FRS WRITE_REG(rd, "(int)" << FRSd); EOP;
		OPC(FIX_TRUNCD)
			FETCH_RD_FRS;
			os << "{ FREG temp; "
			"temp.ll = (long long)" << FRS << ".d; ";
			WRITE_REG(rd, "temp.i[0]"); os << " ";
			WRITE_REG(rd+1, "temp.i[1]"); os << " }";
		EOP;

		OPC(FIXUN_TRUNCS) FETCH_RD_FRS WRITE_REG(rd, "(unsigned int)" << FRSd); EOP;
		OPC(FIXUN_TRUNCD)
			FETCH_RD_FRS;
			os << "{ FREG temp; "
			"temp.ll = (unsigned long long)" << FRS << ".d; ";
			WRITE_REG(rd, "temp.i[0]"); os << " ";
			WRITE_REG(rd+1, "temp.i[1]"); os << " }";
		EOP;

		OPC(FSTS)
			FETCH_RD_FRS_CONST
			os << "{ MA_FV fv; "
			"fv.f = (float)" << FRS << ".d; "
			"WINT(" << RD << " + " << IMM << ", fv.i; }";
		EOP;

		OPC(FSTD)
			FETCH_RD_FRS_CONST
			os << "WINT(" << RD << " + " << IMM << ", " << FRS << ".i[0]); "
			"WINT(" << RD << " + " << IMM + 4 << ", " << FRS << ".i[1]);";
		EOP;

		OPC(FLDS)
			FETCH_RD_RS_CONST
			os << "{ MA_FV fv; "
			"fv.i = RINT(" << RS << " + " << IMM << "); "
			<< FRD << ".d = (double)fv.f; }";
		EOP;

		OPC(FLDD)
			FETCH_RD_RS_CONST
			os << FRD << ".i[0] = RINT(" << RS << " + " << IMM << "); "
			<< FRD << ".i[0] = RINT(" << RS << " + " << IMM + 4 << ");";
		EOP;

		OPC(FADD) FETCH_FRD_FRS os << FRD << ".d += " << FRS << ".d;"; EOP;
		OPC(FSUB) FETCH_FRD_FRS os << FRD << ".d -= " << FRS << ".d;"; EOP;
		OPC(FMUL) FETCH_FRD_FRS os << FRD << ".d *= " << FRS << ".d;"; EOP;
		OPC(FDIV) FETCH_FRD_FRS os << FRD << ".d /= " << FRS << ".d;"; EOP;

		OPC(FSQRT) FETCH_FRD_FRS os << FRD << ".d = sqrt(" << FRS << ".d);"; EOP;
		OPC(FSIN) FETCH_FRD_FRS os << FRD << ".d = sin(" << FRS << ".d);"; EOP;
		OPC(FCOS) FETCH_FRD_FRS os << FRD << ".d = cos(" << FRS << ".d);"; EOP;
		OPC(FEXP) FETCH_FRD_FRS os << FRD << ".d = exp(" << FRS << ".d);"; EOP;
		OPC(FLOG) FETCH_FRD_FRS os << FRD << ".d = log(" << FRS << ".d);"; EOP;
		OPC(FPOW) FETCH_FRD_FRS os << FRD << ".d = pow(" << FRD << ".d, " << FRS << ".d);"; EOP;
		OPC(FATAN2) FETCH_FRD_FRS os << FRD << ".d = atan2(" << FRD << ".d, " << FRS << ".d);"; EOP;

		OPC(LDD)
			FETCH_RD_RS_CONST
			WRITE_REG(rd, "RINT(" << RS << " + " << IMM);
			WRITE_REG(rd+1, "RINT(" << RS << " + " << IMM + 4);
		EOP;

		OPC(STD)
			FETCH_RD_RS_CONST
			os << "WINT(" << RD << " + " << IMM << ", " << RS << "); "
			"WINT(" << RD << " + " << IMM + 4 << ", " << REG(rs+1) << ");";
		EOP;

		OPC(RET)
			switch(f.returnType) {
			case eVoid: os << "return;"; break;
			case eInt: os << "return r0;"; break;
			case eFloat: os << "return f8;"; break;
			case eLong: os << "{ FREG temp; temp.i[0] = r0; temp.i[1] = r1; return temp.ll; }"; break;
			}
		EOP;

		OPC(CALLR)
			FETCH_RD;
			os << "callReg(" << RD << ");";
		EOP;
		OPC(CALLI)
		{
			FETCH_CONST;
			Function dummy;
			dummy.start = IMM;
			set<Function>::const_iterator itr = functions.find(dummy);
			printf("Searched for function at 0x%x\n", IMM);
			if(itr == functions.end()) {
				os << "Broken function\n";
				return ip;
			}
			DEBUG_ASSERT(itr != functions.end());
			const Function& cf(*itr);
			switch(cf.returnType) {
			case eVoid: break;
			case eInt: os << "r0 = "; break;
			case eFloat: os << "f8 = "; break;
			case eLong: os << "{ FREG temp; temp.ll = "; break;
			}
			streamFunctionCall(os, cf);
			os << ";";
			if(cf.returnType == eLong) os << " }";
		}
		EOP;

#define JCI(oper) os << "if (" << RD << " " #oper " " << RS << ") { goto " << label(IMM) <<"; }"
		OPC(JC_EQ) 	FETCH_RD_RS_CONST	JCI(==);	EOP;
		OPC(JC_NE)	FETCH_RD_RS_CONST	JCI(!=);	EOP;
		OPC(JC_GE)	FETCH_RD_RS_CONST	JCI(>=);	EOP;
		OPC(JC_GT)	FETCH_RD_RS_CONST	JCI(>);	EOP;
		OPC(JC_LE)	FETCH_RD_RS_CONST	JCI(<=);	EOP;
		OPC(JC_LT)	FETCH_RD_RS_CONST	JCI(<);	EOP;

#define JCF(oper) os << "if (" << FRD << ".d " #oper " " << FRS << ".d) { goto " << label(IMM) <<"; }"
		OPC(FJC_EQ) FETCH_FRD_FRS_CONST	JCF(==);	EOP;
		OPC(FJC_NE)	FETCH_FRD_FRS_CONST	JCF(!=);	EOP;
		OPC(FJC_GE)	FETCH_FRD_FRS_CONST	JCF(>=);	EOP;
		OPC(FJC_GT)	FETCH_FRD_FRS_CONST	JCF(>);	EOP;
		OPC(FJC_LE)	FETCH_FRD_FRS_CONST	JCF(<=);	EOP;
		OPC(FJC_LT)	FETCH_FRD_FRS_CONST	JCF(<);	EOP;

#define JCU(oper) os << "if (" << RDU << " " #oper " " << RSU << ") { goto " << label(IMM) << "; }"
		OPC(JC_LTU)	FETCH_RD_RS_CONST	JCU(<);	EOP;
		OPC(JC_GEU)	FETCH_RD_RS_CONST	JCU(>=);	EOP;
		OPC(JC_GTU)	FETCH_RD_RS_CONST	JCU(>);	EOP;
		OPC(JC_LEU)	FETCH_RD_RS_CONST	JCU(<=);	EOP;

		OPC(JPI)	FETCH_CONST	os << "goto " << label(IMM) << ";";	EOP;
		OPC(JPR)	FETCH_RD	DEBIG_PHAT_ERROR;	EOP;

		OPC(XB)	FETCH_RD_RS	os << RD << " = ((" << RS << " & 0x80) == 0) ? (" << RS << " & 0xFF) : (" << RS << " | ~0xFF);"; EOP;
		OPC(XH)	FETCH_RD_RS	os << RD << " = ((" << RS << " & 0x8000) == 0) ? (" << RS << " & 0xFFFF) : (" << RS << " | ~0xFFFF);"; EOP;

		OPC(SYSCALL)
			FETCH_IMM8;
			if(f.returnType != eVoid) {
				os << "return ";
			}
			os << "SYSCALL(";
			streamFunctionCall(os, f);
			os << ");\n";
			// skip return instruction
			DEBUG_ASSERT(IB == OP_RET);
			return ip;
		EOP;
	default:
		printf("Unknown instruction 0x%x @ 0x%x!\n", op, ip);
		DEBIG_PHAT_ERROR;
	}
	return ip;
}
