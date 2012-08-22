.text

.global _getStackTop
.type _getStackTop, @function
.stabs "_getStackTop",36,0,0,_getStackTop
.stabs "int,0,0",250,0,0,0
_getStackTop:

	ld r0, fp
	sub r0, #8
	ret

.size _getStackTop, .-_getStackTop
Lscope1:
.stabs "",36,0,0,Lscope1-_getStackTop

.stabd	78,0,0	// EOF
