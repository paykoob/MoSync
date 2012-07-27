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
'960512-1.c', #complex.type
'complex-1.c', #complex.type
'complex-2.c', #complex.type
'complex-3.c', #complex.type
'complex-4.c', #complex.type
'complex-5.c', #complex.type
'complex-6.c', #complex.type
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
]

if(!USE_NEWLIB)
	SKIPPED << 'pr34456.c'	# qsort
	SKIPPED << '20010226-1.c'	# float.h
end

if(CONFIG == "")
	SKIPPED << 'fprintf-chk-1.c'	#fails.on.purpose
	SKIPPED << 'vfprintf-chk-1.c'	#fails.on.purpose
	#SKIPPED << 'nest-stdar-1.c'	# mapip2 bug, incorrect vararg stack pointer offset in inner function.
	#SKIPPED << 'pr41239.c'	# mapip2 bug, incorrect vararg stack pointer offset in normal function.
else
	SKIPPED << 'pr17377.c'	# mapip2 bug, __builtin_return_address
end
