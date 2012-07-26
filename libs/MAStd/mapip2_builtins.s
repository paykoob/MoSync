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
