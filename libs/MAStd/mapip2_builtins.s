.text

.stabs "_sqrt",36,0,0,_sqrt
.stabs "float,0,1",250,0,0,0
.global _sqrt
.type _sqrt, @function
_sqrt:
	fsqrt f8,f8
	ret
.size _sqrt, .-_sqrt
Lscope1:
.stabs "",36,0,0,Lscope1-_sqrt

.stabs "_sin",36,0,0,_sin
.stabs "float,0,1",250,0,0,0
.global _sin
.type _sin, @function
_sin:
	fsin f8,f8
	ret
.size _sin, .-_sin
Lscope2:
.stabs "",36,0,0,Lscope2-_sin

.stabs "_cos",36,0,0,_cos
.stabs "float,0,1",250,0,0,0
.global _cos
.type _cos, @function
_cos:
	fcos f8,f8
	ret
.size _cos, .-_cos
Lscope3:
.stabs "",36,0,0,Lscope3-_cos

.stabs "_exp",36,0,0,_exp
.stabs "float,0,1",250,0,0,0
.global _exp
.type _exp, @function
_exp:
	fexp f8,f8
	ret
.size _exp, .-_exp
Lscope4:
.stabs "",36,0,0,Lscope4-_exp

.stabs "_log",36,0,0,_log
.stabs "float,0,1",250,0,0,0
.global _log
.type _log, @function
_log:
	flog f8,f8
	ret
.size _log, .-_log
Lscope5:
.stabs "",36,0,0,Lscope5-_log

.stabs "_pow",36,0,0,_pow
.stabs "float,0,2",250,0,0,0
.global _pow
.type _pow, @function
_pow:
	fpow f8,f9
	ret
.size _pow, .-_pow
Lscope6:
.stabs "",36,0,0,Lscope6-_pow

.stabs "_atan2",36,0,0,_atan2
.stabs "float,0,2",250,0,0,0
.global _atan2
.type _atan2, @function
_atan2:
	fatan2 f8,f9
	ret
.size _atan2, .-_atan2
Lscope7:
.stabs "",36,0,0,Lscope7-_atan2

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

.stabd	78,0,0	// EOF
