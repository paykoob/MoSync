SKIPPED = [
'20010724-1.c', #platform.specific
'990413-2.c', #platform.specific
'bcp-1.c', #fails.on.purpose
'stdio-opt-1.c', #stdio
'stdio-opt-2.c', #stdio
'stdio-opt-3.c', #stdio
'20020720-1.c', #fails.on.purpose
'20030125-1.c', #fails.on.purpose
'20050121-1.c', #complex.type
#'960512-1.c', #complex.type
#'complex-1.c', #complex.type
#'complex-2.c', #complex.type
#'complex-3.c', #complex.type
#'complex-4.c', #complex.type
'complex-5.c', #complex.type
#'complex-6.c', #complex.type
'ffs-1.c', #builtin.function
'ffs-2.c', #builtin.function
'nestfunc-1.c', #no.trampolines
'nestfunc-2.c', #no.trampolines
'nestfunc-3.c', #no.trampolines
'nestfunc-4.c', #no.nested.functions, please
'nestfunc-5.c', #no.trampolines
'nestfunc-6.c', #todo.qsort
'bitfld-1.c', #fails.on.purpose
'20010122-1.c', #fails.on.purpose
'20000822-1.c', #no.trampolines
'921215-1.c', #no.trampolines
'931002-1.c', #no.trampolines
'eeprof-1.c', #profiler
'941014-1.c', #pipe-tool reports: illegal function pointer manipulation.
'20101011-1.c',	# integer division by zero
'align-3.c',	# illegal alignment
'pr47237.c',	# __builtin_apply() not supported yet.
'compile/20010226-1.c',	# trampolines
'compile/20040323-1.c',	# alias
'compile/20050122-1.c',	# trampolines
'compile/20050122-2.c',	# trampolines
'compile/920520-1.c',	# broken inline assembly
'compile/920521-1.c',	# broken inline assembly
'compile/930506-2.c',	# trampolines
'compile/930623-1.c',	# __builtin_apply
'compile/calls.c',	# broken calls to immediate non-function addresses
'compile/dll.c',	# DLL
'compile/mipscop-1.c',	# MIPS
'compile/mipscop-2.c',	# MIPS
'compile/mipscop-3.c',	# MIPS
'compile/mipscop-4.c',	# MIPS
'compile/nested-1.c',	# trampolines
'compile/nested-2.c',	# trampolines
'compile/pr23237.c',	# section attributes are not supported for this target
'compile/pr27889.c',	# trampolines
'compile/pr30311.c',	# x86
'compile/pr38789.c',	# broken inline assembly
'compile/pr42956.c',	# weird. not valid C, as far as I can tell.
'compile/pr44197.c',	# alias
'unsorted/dump-noaddr.c',	# .x file, too complicated.
#'c-c++-common/asmgoto-2.c',	# fails on purpose
#'c-c++-common/builtin-offsetof.c',	# fails on purpose
#'c-c++-common/fmax-errors.c',	# fails on purpose
'c-c++-common/int128-1.c',	# __int128
'c-c++-common/int128-2.c',	# __int128
'c-c++-common/int128-types-1.c',	# __int128
#'c-c++-common/pr33193.c',	# fails on purpose
#'c-c++-common/pr43690.c',	# fails on purpose
#'c-c++-common/pr51768.c',	# fails on purpose
'c-c++-common/torture/complex-alias-1.c',	# complex builtins
'c-c++-common/torture/complex-sign-mul-minus-one.c',	# complex builtins
'c-c++-common/torture/complex-sign-mul-one.c',	# complex builtins
'c-c++-common/torture/complex-sign-mul.c',	# complex builtins
'g++.old-deja/g++.abi/cxa_vec.C',	# setjmp
'g++.old-deja/g++.abi/ptrflags.C',	# typeinfo
'g++.old-deja/g++.abi/vmihint.C',	# typeinfo
'g++.old-deja/g++.benjamin/15071.C',	# iostream
'g++.old-deja/g++.benjamin/typeid01.C',	# typeinfo
'g++.old-deja/g++.brendan/eh1.C',	# exceptions
'g++.old-deja/g++.jason/conversion10.C',	# old, unfixed gcc bug
'g++.old-deja/g++.jason/dcast2.C',	# dynamic_cast
'g++.old-deja/g++.jason/dcast3.C',	# dynamic_cast
'g++.old-deja/g++.jason/thunk3.C',	# variadric thunk
'g++.old-deja/g++.law/operators27.C',	# exceptions
'g++.old-deja/g++.law/pr25000.C',	# exceptions
'g++.old-deja/g++.law/profile1.C',	# profile mode not supported
'g++.old-deja/g++.martin/new1.C',	# exceptions
'g++.old-deja/g++.mike/dyncast3.C',	# dynamic_cast
'g++.old-deja/g++.mike/dyncast9.C',	# dynamic_cast
'g++.old-deja/g++.mike/p16146.C',	# dynamic_cast
'g++.old-deja/g++.mike/p7912.C',	# exceptions
'g++.old-deja/g++.mike/p9706.C',	# exceptions
'g++.old-deja/g++.oliva/delete3.C',	# exceptions
'g++.old-deja/g++.oliva/new1.C',	# exceptions
'g++.old-deja/g++.other/array1.C',	# exceptions
'g++.old-deja/g++.other/crash15.C',	# exceptions
'g++.old-deja/g++.other/dcast2.C',	# dynamic_cast
'g++.old-deja/g++.other/delete3.C',	# exceptions
'g++.old-deja/g++.other/eh3.C',	# exceptions
'g++.old-deja/g++.other/eh5.C',	# exceptions
'g++.old-deja/g++.other/init7.C',	# exceptions
'g++.old-deja/g++.other/new7.C',	# exceptions
'g++.old-deja/g++.other/singleton.C',	# exceptions
'g++.old-deja/g++.other/syntax1.C',	# -fsyntax-only
'g++.old-deja/g++.other/syntax2.C',	# -fsyntax-only
'g++.old-deja/g++.other/vbase2.C',	# exceptions
'g++.old-deja/g++.pt/const2.C',	# fails on purpose, though it shouldn't. very weird.
'g++.old-deja/g++.pt/crash20.C',	# fails on purpose?
'g++.old-deja/g++.pt/fntry1.C',	# exceptions
'g++.old-deja/g++.pt/instantiate4.C',	# fails on purpose
'g++.old-deja/g++.robertl/eb123.C',	# exceptions
'g++.old-deja/g++.robertl/eb124.C',	# exceptions
'g++.old-deja/g++.robertl/eb130.C',	# requires GNU stl
'g++.old-deja/g++.robertl/eb17.C',	# dynamic_cast
'g++.old-deja/g++.robertl/eb50.C',	# exceptions
'g++.old-deja/g++.robertl/eb88.C',	# exceptions
'gcc.dg/Walways-true-2.c',  # gas says: redefined symbol cannot be used on reloc
'gcc.dg/alias-5.c',	# require-alias
'gcc.dg/alias-6.c',	# require-alias
'gcc.dg/alias-7.c',	# require-alias
'gcc.dg/always_inline.c',	# fails on purpose
'gcc.dg/always_inline2.c',	# fails on purpose
'gcc.dg/always_inline3.c',	# fails on purpose
'gcc.dg/asm-2.c',	# broken inline assembly
'gcc.dg/asm-3.c',	# broken inline assembly
'gcc.dg/asm-4.c',	# broken inline assembly
'gcc.dg/asm-pr24146.c',	# broken inline assembly
'gcc.dg/builtin-apply1.c',	# __builtin_apply
'gcc.dg/builtin-apply2.c',	# __builtin_apply
'gcc.dg/builtin-apply3.c',	# __builtin_apply
'gcc.dg/builtin-apply4.c',	# __builtin_apply
'gcc.dg/builtin-return-1.c',	# __builtin_return
'gcc.dg/builtin-unreachable-1.c',	# broken inline assembly
'gcc.dg/builtins-20.c',	# fails on purpose
'gcc.dg/builtins-57.c',	# fails on purpose
'gcc.dg/builtins-65.c',	# fails on purpose
'gcc.dg/cproj-fails-with-broken-glibc.c',	# fails on purpose

#bugs below

'builtin-bitops-1.c', #builtin.functions. todo: implement them.

# mapip2 bug, segfault due to "emit_move_insn QImode -> SImode"
#'compile/930503-2.c',	# MINIMAL TESTCASE!
'compile/limits-exprparen.c',	# gcc bug, segfault due to 27796 stack frames.
#'g++.old-deja/g++.law/ctors18.C',	# emit_move_insn SImode -> QImode. This one's different.
#'g++.old-deja/g++.law/cvt3.C',	# emit_move_insn SImode -> QImode.
]

SKIPPED_REGEXP = [
%r{g\+\+.old-deja/g\+\+.abi/vtable3..C},	# typeinfo
%r{g\+\+.old-deja/g\+\+.mike/eh.+.C},	# exceptions
%r{g\+\+.old-deja/g\+\+.other/dyncast..C},	# dynamic_cast
/gcc.dg\/c99-tgmath-..c/,	# tgmath.h
]

# if a dejaGnu test includes any of these lines, it is skipped.
SKIP_LINES = [
'#include <iostream>',
'#include <typeinfo>',
'#include <complex>',
'#include <fstream>',
'#include <iomanip>',
'#include <exception>',
'#include <sstream>',
'/* { dg-require-alias "" } */',
'// { dg-require-alias "" }',
'/* { dg-require-ifunc "" } */',
'// { dg-require-weak "" }',
'#include <unwind.h>',
]

SKIPPED_DIRS = [
'g++.old-deja/g++.eh',
]

if(!USE_NEWLIB)
	SKIP_LINES << '#include <string>'
	SKIP_LINES << '#include <list>'
	SKIP_LINES << '#include <map>'
	SKIP_LINES << '#include <vector>'
	SKIP_LINES << '#include <iterator>'
	SKIP_LINES << '#include <utility>'
	SKIP_LINES << '#include <algorithm>'
	SKIPPED << 'pr34456.c'	# qsort
	SKIPPED << 'g++.old-deja/g++.jason/template44.C'	# qsort
	SKIPPED << 'g++.old-deja/g++.martin/bitset1.C'	# bitset
	SKIPPED << 'g++.old-deja/g++.mike/ns15.C'	# qsort
	SKIPPED << 'g++.old-deja/g++.ns/using6.C'	# vector
	SKIPPED << 'g++.old-deja/g++.other/bitfld1.C'	# utility
	SKIPPED << 'g++.old-deja/g++.other/init18.C'	# atexit
	SKIPPED << 'g++.old-deja/g++.other/init19.C'	# atexit
	SKIPPED << 'g++.old-deja/g++.other/init5.C'	# atexit
	SKIPPED << 'gcc.dg/cdce1.c'	# errno.h
	SKIPPED << 'gcc.dg/cdce2.c'	# errno.h
end

if(CONFIG == "")
	SKIPPED << 'fprintf-chk-1.c'	#fails.on.purpose
	SKIPPED << 'vfprintf-chk-1.c'	#fails.on.purpose
	#SKIPPED << 'nest-stdar-1.c'	# mapip2 bug, incorrect vararg stack pointer offset in inner function.
	#SKIPPED << 'pr41239.c'	# mapip2 bug, incorrect vararg stack pointer offset in normal function.
	SKIPPED << 'compile/limits-structnest.c'	# gcc bug, eats 2+ GB RAM in a few seconds. todo: test on Windows.
	SKIPPED << 'compile/pr21840.c'	# illegal call
else
	SKIPPED << 'pr17377.c'	# mapip2 bug, __builtin_return_address
	SKIPPED << 'c-c++-common/Wunused-var-8.c'	# mapip2 bug, find_valid_class(DI, SI)
	SKIPPED << 'c-c++-common/restrict-1.c'	# fails on purpose
end
