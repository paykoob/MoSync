
.text
.global _getStackTop
_getStackTop:

	ld r0, fp
	sub r0, #8
	ret
