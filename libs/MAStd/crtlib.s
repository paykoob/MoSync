//********************************************************************************
//								MoSync asm routines
//********************************************************************************

//	.sourcefile 'crtlib.s'

//	.line 1

//****************************************
//				Globals
//****************************************

	.data
	.globl	__memtop
	.align 4
__memtop:
	.long	0

	.globl	__stacktop
__stacktop:
	.long	0

	.globl	___CTOR_LIST__
___CTOR_LIST__:
	.long	0

	.globl	___DTOR_LIST__
___DTOR_LIST__:
	.long	0

//****************************************
//			Start up code
//****************************************

	.text
	.global crt0_startup
	.type crt0_startup, @function

.stabs "crt0_startup",36,0,0,crt0_startup
.stabs "void,5,0",250,0,0,0

	// sp: top of stack
	// p0: memory size
	// p1: stack size
	// p2: heap size
	// p3: ctor chain
	// g0: dtor chain
crt0_startup:

	ld [&___CTOR_LIST__],p3	// save ctor chain
	ld [&___DTOR_LIST__],g0	// save dtor chain

	ld	[&__memtop],p0		// Save top of memory

	sub sp, #16			// move stack down memory 16 bytes
	ld	[&__stacktop],sp	// Save top of stack
	ld	p0,sp

	sub p0,p1			// make p0 into heap_top
	sub p0,p2			// p0 is now start of heap
	ld  p1,p2			// make p1 into heap_size

	call &_override_heap_init_crt0

	call &_resource_selector

	call &_crt_ctor_chain

	call &_MAMain

crt_exit:
	ld	p0, r0		// save return value
	call &_exit

.size crt0_startup, .-crt0_startup
Lscope1:
.stabs "",36,0,0,Lscope1-crt0_startup

.stabd	78,0,0	// EOF
