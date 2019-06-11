	; Test assertions about aliases and keyboard overrides
	; The test is successful if the assembler doesn't exit with an error

	.ifndef assert
assert .macro
	.if !(\1)
		.fail Assert failed: \1
	.endif
	.endm
	.endif
	
	.ifndef assert_pass1
assert_pass1 .macro
	.ifndef ASSERT\@
ASSERT\@ = 1
	.if !(\1)
		.fail (PASS 1) Assert failed: \1
	.endif
	.endif
	.endm
	.endif
	
	.ifndef assert_pass2
assert_pass2 .macro
	.ifdef ASSERT\@
	.if !(\1)
		.fail (PASS 2) Assert failed: \1
	.endif
	.else
ASSERT\@ = 2
	.endif
	.endm
	.endif
	
	; Alias reserved keyword
	assert_pass1 (!DEFINED(MAGICKIT_ALIAS))
	assert_pass2 (MAGICKIT_ALIAS = MAGICKIT)
MAGICKIT_ALIAS          .alias MAGICKIT
	assert (MAGICKIT_ALIAS = MAGICKIT)
	
	; Alias reserved function
HIGH_ALIAS				.alias HIGH
	assert (HIGH_ALIAS($12AB) = $12)
	assert (HIGH_ALIAS($1337) = HIGH($1337))

	; Alias overridable function, also test nested aliases
POW_ALIAS               .alias POW
POW_ALIAS_NESTED		.alias POW_ALIAS
	assert (POW_ALIAS(7, 5) = 16807)
	assert (POW_ALIAS(3, 4) = POW_ALIAS_NESTED(3, 4))	
POW     .func \1 + \2
	assert (POW(7, 5) = 12)
	assert (POW_ALIAS(9, 3) = 729)
	assert (POW_ALIAS(2, 9) = POW_ALIAS_NESTED(2, 9))	

	; Alias directives
db_alias				.alias .db
data0     db_alias        $00, $01
data1    .db_alias        $00, $01, $02, $03
	assert (SIZEOF(data0) = 2 & SIZEOF(data1) = 4)
	assert (data0 + 2 = data1)

db_alias2				.alias  db
data2    db_alias2       $00, $01
data3   .db_alias2       $00, $01
	assert (SIZEOF(data2) = 2 & SIZEOF(data3) = 2)

db_alias_nested			.alias db_alias
data4    db_alias_nested  $00, $01
data5   .db_alias_nested  $02, $03
	assert (SIZEOF(data4) = 2 & SIZEOF(data5) = 2)

db_alias_nested2		.alias .db_alias
data6    db_alias_nested2 $00, $01, $02
data7   .db_alias_nested2 $00
	assert (SIZEOF(data6) = 3 & SIZEOF(data7) = 1)
	
org_alias				.alias org
	.org_alias $2468
	assert (* = $2468)
	org_alias $8642
	assert (* = $8642)

	; Alias instructions
sti                     .alias cli
	sti
	cli
ld						.alias lda
	ld $00
	ld <$00
	ld.L <$00
	ld.H <$00
	ld data0

	; Alias overridable keywords
	assert_pass1 (NESASM_S = 1)
	assert_pass2 (NESASM_S = 10)
	assert_pass1 (!DEFINED(alias_nesasm))
	assert_pass2 (alias_nesasm = 1)
	assert_pass1 (!DEFINED(alias_nesasm_new_nested))
	assert_pass2 (alias_nesasm_new_nested = 10)

alias_nesasm            .alias NESASM_S
alias_nesasm_nested     .alias alias_nesasm
NESASM_S 	            .equ   10
alias_nesasm_new        .alias NESASM_S
alias_nesasm_new_nested .alias alias_nesasm_new

	assert (NESASM_S = 10)
	assert (alias_nesasm = 1)
	assert (alias_nesasm_new_nested = 10)
	
	; Alias user labels
	assert_pass1 (!DEFINED(alias_ts))
	assert_pass2 (alias_ts = 15)
	
TEST_SYMBOL				.equ   15
alias_ts			    .alias TEST_SYMBOL
	lda TEST_SYMBOL
	lda alias_ts
	
	assert (TEST_SYMBOL = 15)
	assert (alias_ts = 15)
	
	; Alias functions
TEST_FUNC               .func  \1 * 2
alias_func			    .alias TEST_FUNC
alias_func_nested	    .alias alias_func
	assert (TEST_FUNC(10) = 20)
	assert (alias_func(20) = 40)
	assert (alias_func_nested(30) = 60)

	; Alias macros
TEST_MACRO				.macro
	assert ((\1 + \2) = 10)
	.endm
alias_macro				.alias TEST_MACRO
alias_macro_nested		.alias alias_macro	
	TEST_MACRO 5, 5
	alias_macro 10, 0
	alias_macro_nested 1, 9

	; Custom macro directive 
MAKEMAC	.alias .macro
ENDMAC  .alias .endm
TEST_MAC1 MAKEMAC 
	assert ((\1 + \2) = 10)
	ENDMAC
	
    MAKEMAC TEST_MAC2
	assert ((\1 + \2) = 10)
	ENDMAC
	
	TEST_MAC1 10, 0
	TEST_MAC2 0, 10
	
	