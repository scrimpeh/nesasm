	; Test assertions about functions and opeartors, within assembly
	; The test is successful if the assembler doesn't exit with an error

	.ifndef assert
assert .macro
	.if !(\1)
		.fail Assert failed: \1
	.endif
	.endm
	.endif
	
ARG_NONE = 0
ARG_REG = 1
ARG_IMMEDIATE = 2
ARG_ABSOLUTE = 3
ARG_INDIRECT = 4
ARG_STRING = 5
ARG_LABEL = 6

test_label:
a_test_label:
	
	; Test \@ in functions, which returns a unique value every time a function is called
	; Note that \@ is only incremented if a function is actually evaluated, so a nested function
	; call will only increment \@ if the call is required
unique .func \@
const .func 0
id .func \1
nested .func id(\1)
	assert unique() = 0
	assert unique() = 1
	assert id(unique()) = 3					; Nested function call that evaluates unique()
	assert unique() = 4
	assert id(id(unique())) = 7				; Twice nested function call
	assert unique() = 8
	assert unique(unique(), const()) = 9	; inner calls are not evaluated
	assert nested(unique()) = 12			; nested (+1) calls echo (+1) with unique (+1)
	assert unique() + unique() = 27			; 13 + 14
	
	; Test \@ as a function argument to other functions
unique_id .func id(\@)
	assert unique() = 15
	assert unique_id() = 16					; Note that \@ is evaluated early this time
	assert unique_id() = 18
	assert unique() = 20
	
	; Test \# in functions, should behave exactly as with macros
count .func \#
	assert count() = 0
	assert count(,) = 0
	assert count(,,) = 0
	assert count(1) = 1
	assert count(1,) = 1
	assert count(1,1) = 2
	assert count(1,,) = 1
	assert count(,1,,,1,,) = 5
	assert count(count(),count() ) = 2
	assert count(count(count(),count())) = 1
	
	; Test \# as function argument inside of function definitions
x2 .func \1 * 2
count_x2 .func x2(\#)
	assert count_x2() = 0
	assert count_x2(1) = 2
	assert count_x2(1,1) = 4

	; Test equivalent behaviour with macros
mcount0 .macro
	assert \# = 0
	.endm
mcount1 .macro
	assert \# = 1
	.endm
mcount2 .macro
	assert \# = 2
	.endm
mcount3 .macro
	assert \# = 3
	.endm
mcount4 .macro
	assert \# = 4
	.endm
mcount5 .macro
	assert \# = 5
	.endm
	
	mcount0 
	mcount0 ,
	mcount0 ,,
	mcount1 1
	mcount1 1,
	mcount2 1,1
	mcount1 1,,
	mcount5 ,1,,,1,,
	
	; Test arg type counter with \?1. Behaviour should be equivalent to macros
type .func \?1
	assert type() = ARG_NONE
	assert type(,,,) = ARG_NONE
	assert type(,1) = ARG_NONE
	assert type(1) = ARG_ABSOLUTE
	
	assert type(a) = ARG_REG
	assert type(x) = ARG_REG
	assert type(y) = ARG_REG
	assert type("Test") = ARG_STRING
	assert type([$00]) = ARG_INDIRECT
	assert type(#$00) = ARG_IMMEDIATE
	
	assert type(test_label) = ARG_LABEL
	assert type(a_test_label) = ARG_LABEL		; Should not be confused for a register
	
	assert type(type) = ARG_LABEL
	
	assert type(01234) = ARG_ABSOLUTE
	assert type($1234) = ARG_ABSOLUTE
	assert type(%0101) = ARG_ABSOLUTE
	
type9 .func \?9
	assert type9(,,,,,,,,1) = ARG_ABSOLUTE
	assert type9(1,2,3,4,5) = ARG_NONE
	
	; Test using arg type as a function argument itself
typearg1 .func x2(\?1)
typearg2 .func x2(\?2)
	assert typearg1() = ARG_NONE * 2
	assert typearg1(type) = ARG_LABEL * 2
	assert typearg2(,a) = ARG_REG * 2
	
	; Test interaction of macro parameters and function parameters
mcount_func .macro
	assert count(\#) = 1	; The \# here refers to the macro count, so it must always be one
	.endm
	
	mcount_func
	mcount_func 4
	mcount_func 1,1,,1

munique_func .macro
	assert \@ = id(id(id(\@))) 
	assert unique() != unique()		
	assert \@ = \@
	.endm
	
	munique_func


mtype_func .macro
	assert \?1 = type(\1)
	assert \?9 = type(\9)
	assert type(\?1) = ARG_ABSOLUTE
	.endm
	
	mtype_func 1
	
default .func \1 - 0 + ((\?1 = ARG_NONE) * \2 - 0) 
mdefault .macro
default_0_\1 = default(\2)
default_val_\1 = default(\2, 10)
	.endm
	
	mdefault 1
	assert default_0_1 = 0
	assert default_val_1 = 10
	mdefault 2, 5
	assert default_0_2 = 5
	assert default_val_2 = 5
