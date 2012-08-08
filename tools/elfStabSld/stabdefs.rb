StabDef = Struct.new('StabDef', :type, :name)

#define N_EXT 0x1
#define N_UNDF 0x0
STABS = [
StabDef.new(0x0, 'N_UNDF'),	# Undefined symbol
StabDef.new(0x2, 'N_ABS'),	# File scope absolute symbol
StabDef.new(0x3, 'N_ABS | N_EXT'),	# External absolute symbol
StabDef.new(0x4, 'N_TEXT'),	# File scope text symbol
StabDef.new(0x5, 'N_TEXT | N_EXT'),	# External text symbol
StabDef.new(0x6, 'N_DATA'),	# File scope data symbol
StabDef.new(0x7, 'N_DATA | N_EXT'),	# External data symbol
StabDef.new(0x8, 'N_BSS'),	# File scope BSS symbol
StabDef.new(0x9, 'N_BSS | N_EXT'),	# External BSS symbol
StabDef.new(0x0c, 'N_FN_SEQ'),	# Same as, 'N_FN, for Sequent compilers'),
StabDef.new(0x0a, 'N_INDR'),	# Symbol is indirected to another symbol
StabDef.new(0x12, 'N_COMM'),	# Common--visible after shared library dynamic link
StabDef.new(0x14, 'N_SETA'),
StabDef.new(0x15, 'N_SETA | N_EXT'),	# Absolute set element
StabDef.new(0x16, 'N_SETT'),
StabDef.new(0x17, 'N_SETT | N_EXT'),	# Text segment set element
StabDef.new(0x18, 'N_SETD'),
StabDef.new(0x19, 'N_SETD | N_EXT'),	# Data segment set element
StabDef.new(0x1a, 'N_SETB'),
StabDef.new(0x1b, 'N_SETB | N_EXT'),	# BSS segment set element
StabDef.new(0x1c, 'N_SETV'),
StabDef.new(0x1d, 'N_SETV | N_EXT'),	# Pointer to set vector
StabDef.new(0x1e, 'N_WARNING'),	# Print a warning message during linking
StabDef.new(0x1f, 'N_FN'),	# File name of a .o file


StabDef.new(0x20, 'N_GSYM'),	# Global symbol; see Global Variables.
StabDef.new(0x22, 'N_FNAME'),	# Function name (for BSD Fortran),; see Procedures.
StabDef.new(0x24, 'N_FUN'),	# Function name (see Procedures), or text segment variable (see Statics),.
StabDef.new(0x26, 'N_STSYM'),	# Data segment file-scope variable; see Statics.
StabDef.new(0x28, 'N_LCSYM'),	# BSS segment file-scope variable; see Statics.
StabDef.new(0x2a, 'N_MAIN'),	# Name of main routine; see Main Program.
StabDef.new(0x2c, 'N_ROSYM'),	# Variable in .rodata section; see Statics.
StabDef.new(0x2e, 'N_BNSYM'),	#  Start of a relocatable symbol block; see section Coalesced Symbol Blocks.
StabDef.new(0x30, 'N_PC'),	# Global symbol (for Pascal),; see N_PC.
StabDef.new(0x32, 'N_NSYMS'),	# Number of symbols (according to Ultrix V4.0),; see N_NSYMS.
StabDef.new(0x34, 'N_NOMAP'),	# No DST map; see N_NOMAP.
StabDef.new(0x36, 'N_MAC_DEFINE'),	# Name and body of a #defined macro; see Macro define and undefine.
StabDef.new(0x38, 'N_OBJ'),	# Object file (Solaris2),.
StabDef.new(0x3a, 'N_MAC_UNDEF'),	# Name of an #undefed macro; see Macro define and undefine.
StabDef.new(0x3c, 'N_OPT'),	# Debugger options (Solaris2),.
StabDef.new(0x40, 'N_RSYM'),	# Register variable; see Register Variables.
StabDef.new(0x42, 'N_M2C'),	# Modula-2 compilation unit; see N_M2C.
StabDef.new(0x44, 'N_SLINE'),	# Line number in text segment; see Line Numbers.
StabDef.new(0x46, 'N_DSLINE'),	# Line number in data segment; see Line Numbers.
StabDef.new(0x48, 'N_BSLINE'),	# Line number in bss segment; see Line Numbers.
#StabDef.new(0x48, 'N_BROWS'),	# Sun source code browser, path to .cb file; see N_BROWS.
StabDef.new(0x4a, 'N_DEFD'),	# GNU Modula2 definition module dependency; see N_DEFD.
StabDef.new(0x4c, 'N_FLINE'),	# Function start/body/end line numbers (Solaris2),.
StabDef.new(0x4e, 'N_ENSYM'),	# MacOS X: This tells the end of a relocatable function + debugging info.
StabDef.new(0x50, 'N_EHDECL'),	# GNU C++ exception variable; see N_EHDECL.
#StabDef.new(0x50, 'N_MOD2'),	# Modula2 info "for imc" (according to Ultrix V4.0),; see N_MOD2.
StabDef.new(0x54, 'N_CATCH'),	# GNU C++ catch clause; see N_CATCH.
StabDef.new(0x60, 'N_SSYM'),	# Structure of union element; see N_SSYM.
StabDef.new(0x62, 'N_ENDM'),	# Last stab for module (Solaris2),.
StabDef.new(0x64, 'N_SO'),	# Path and name of source file; see Source Files.
StabDef.new(0x80, 'N_LSYM'),	# Stack variable (see Stack Variables), or type (see Typedefs),.
StabDef.new(0x82, 'N_BINCL'),	# Beginning of an include file (Sun only),; see Include Files.
StabDef.new(0x84, 'N_SOL'),	# Name of include file; see Include Files.
StabDef.new(0xa0, 'N_PSYM'),	# Parameter variable; see Parameters.
StabDef.new(0xa2, 'N_EINCL'),	# End of an include file; see Include Files.
StabDef.new(0xa4, 'N_ENTRY'),	# Alternate entry point; see Alternate Entry Points.
StabDef.new(0xc0, 'N_LBRAC'),	# Beginning of a lexical block; see Block Structure.
StabDef.new(0xc2, 'N_EXCL'),	# Place holder for a deleted include file; see Include Files.
StabDef.new(0xc4, 'N_SCOPE'),	# Modula2 scope information (Sun linker),; see N_SCOPE.
StabDef.new(0xe0, 'N_RBRAC'),	# End of a lexical block; see Block Structure.
StabDef.new(0xe2, 'N_BCOMM'),	# Begin named common block; see Common Blocks.
StabDef.new(0xe4, 'N_ECOMM'),	# End named common block; see Common Blocks.
StabDef.new(0xe8, 'N_ECOML'),	# Member of a common block; see Common Blocks.
StabDef.new(0xea, 'N_WITH'),	# Pascal with statement: type,,0,0,offset (Solaris2),.
StabDef.new(0xf0, 'N_NBTEXT'),	# Gould non-base registers; see Gould.
StabDef.new(0xf2, 'N_NBDATA'),	# Gould non-base registers; see Gould.
StabDef.new(0xf4, 'N_NBBSS'),	# Gould non-base registers; see Gould.
StabDef.new(0xf6, 'N_NBSTS'),	# Gould non-base registers; see Gould.
StabDef.new(0xf8, 'N_NBLCS'),	# Gould non-base registers; see Gould.
StabDef.new(0xfa, 'N_MOSYNC'),	# MoSync custom stabs.
]
