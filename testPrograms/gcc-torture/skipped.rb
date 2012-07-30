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

#bugs below

'builtin-bitops-1.c', #builtin.functions. todo: implement them.
#'930622-2.c', #gcc bug: calls __fixdfdi instead of __fixdfti or __fixtfti
#'20000722-1.c',	# mapip2 bug: caller's stack frame is not proper.
#'20000815-1.c',	# mapip2 bug, unknown. "emit_move_insn QImode -> SImode"?
#'20020227-1.c',	# mapip2 bug, unknown (complex float, should work).

# mapip2 bug, same as 20000722-1: callers that pass structs to callees
# does not properly decrement the stack pointer in the caller's prologue.
#'20050316-1.c',
#'20050316-2.c',
#'20050316-3.c',

#'20070824-1.c',	# mapip2 bug: __builtin_alloca?
'930713-1.c',	# mapip2 bug, segfault due to "emit_move_insn QImode -> SImode"
#'930718-1.c',	# mapip2 bug. struct on stack, 20000722-1.
'931004-3.c',	# mapip2 bug, segfault due to "emit_move_insn HImode -> SImode"
'931004-7.c',	# mapip2 bug, segfault due to "emit_move_insn QImode -> SImode"
'931005-1.c',	# mapip2 bug, segfault due to "emit_move_insn QImode -> SImode"
#'frame-address.c',	# mapip2 bug, __builtin_alloca?
#'pr15296.c',	# mapip2 bug, ridiculously complicated
'pr42154.c',	# mapip2 bug, segfault due to "emit_move_insn QImode -> SImode"
'compile/930503-2.c',	# mapip2 bug, segfault due to "emit_move_insn QImode -> SImode"	MINIMAL TESTCASE!
'compile/limits-exprparen.c',	# gcc bug, segfault due to 27796 stack frames.
'compile/pr44687.c',	# mapip2 bug, segfault due to "emit_move_insn QImode -> SImode"
'compile/structs.c',	# mapip2 bug, segfault due to "emit_move_insn QImode -> SImode"
]

if(!USE_NEWLIB)
	SKIPPED << 'pr34456.c'	# qsort
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
