	; Test inbuilt functions such as BANK(), PAGE() and so on
	
	.ifndef assert
assert .macro
	.if !(\1)
		.fail Assert failed: \1
	.endif
	.endm
	.endif
	
	; PC should count as defined
	assert DEFINED(*) 
	
	; Test behaviour of Program Counter (*) in BANK and PAGE inbuilts
	.bank 0
	.data
	.org $E800
	assert BANK(*) = 0
	assert PAGE(*) = 7
	
	.ds 20
	assert BANK(*) = 0
	assert PAGE(*) = 7
	
	.bank 1
	.org $C800
	assert BANK(*) = 1
	assert PAGE(*) = 6
	
	.org $2000
	assert BANK(*) = 1
	assert PAGE(*) = 1
	
	.bank 2
	assert * = $0000
	assert BANK(*) = 2
	assert PAGE(*) = 0
	
	.bank 0
	.code
	.org $E800
	assert BANK(*) = 0
	assert PAGE(*) = 7
	
	.bank 1
	.org $C800
	assert BANK(*) = 1
	assert PAGE(*) = 6
	
	.org $2000
	assert BANK(*) = 1
	assert PAGE(*) = 1
	
	.bank 2
	assert * = $0000
	assert BANK(*) = 2
	assert PAGE(*) = 0
	
	.org $0000
	assert BANK(*) = 2
	.incbin "test_padding.bin"		; Include a binary file to push the bank coutner to the next bank
	assert BANK(*) = 3
	
	; Test PAGE in values
	assert PAGE(0) = 0
	assert PAGE($1FFF) = 0
	assert PAGE(1 + $1FFF) = 1
	assert PAGE($FFFF) = 7
	