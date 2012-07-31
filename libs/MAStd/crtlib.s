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

//****************************************
//			Start up code
//****************************************

	.text
	.global crt0_startup

	// sp: top of stack
	// p0: memory size
	// p1: stack size
	// p2: heap size
	// p3: ctor chain
crt0_startup:

	ld [&___CTOR_LIST__],p3	// save ctor chain

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
	ld	[sp,0], r0		// save return value

	call &_crt_dtor_chain

	ld	p0, [sp,0]	// restore return value
	call &_maExit
