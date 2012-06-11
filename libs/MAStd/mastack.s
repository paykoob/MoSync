
.text
.align 4
.global _getStackTop
.func _getStackTop

	ld r0, fp
	sub r0, #8
	ret
