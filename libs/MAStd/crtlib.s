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

//****************************************
//			Start up code
//****************************************

	.text
	.align 4
	.global crt0_startup

	// sp: top of stack
	// i0: memory size
	// i1: stack size
	// i2: heap size
crt0_startup:

	ld	[&__memtop],p0		// Save top of memory

	sub sp, #16			// move stack down memory 16 bytes
	ld	[&__stacktop],sp	// Save top of memory
	ld	p0,sp

	sub p0,p1			// make i0 into heap_top
	sub p0,p2			// i0 is now start of heap
	ld  p1,p2			// make i1 into heap_size

	call &_override_heap_init_crt0

	call &_resource_selector

//	ld	p0,__global_ctor_chain 		//constructor chain
//	call &_crt_tor_chain

	call &_MAMain

crt_exit:
	ld	[sp,0], r0		// save return value

//	ld	p0,__global_dtor_chain			// destructor chain
//	call &_crt_tor_chain

	ld	p0, [sp,0]
	call &_maExit
	ret
