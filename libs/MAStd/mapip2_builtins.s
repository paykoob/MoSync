.text
.global _sqrt
_sqrt:
	fsqrt f8,f8
	ret
.global _sin
_sin:
	fsin f8,f8
	ret
.global _cos
_cos:
	fcos f8,f8
	ret
.global _exp
_exp:
	fexp f8,f8
	ret
.global _log
_log:
	flog f8,f8
	ret
.global _pow
_pow:
	fpow f8,f9
	ret
.global _atan2
_atan2:
	fatan2 f8,f9
	ret

.global _sinf
.set _sinf,_sin

.global _cosf
.set _cosf,_cos

.global _tanf
.set _tanf,_tan

.global _tanl
.set _tanl,_tan

.global _atan2f
.set _atan2f,_atan2

.global _cargl
.set _cargl,_carg

.global _cargf
.set _cargf,_carg
