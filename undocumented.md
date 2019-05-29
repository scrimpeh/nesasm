### scrm's list of weird undocumented nesasm features:

Stuff I found in nesasm by digging through the source code. Most of these are pretty obtuse and half-baked. I might extend support for them later. The main purpose of this list is to document all of this stuff somewhere, and so I don't forget about it later.

I'm not getting into any of the PCE stuff here. If you want to find out about that, feel free to dig in yourself.

It might seem overbearing to list all of the tiniest details, but I don't remember ever reading about these features, so they should be noted somewhere.

This list is not complete. I'll add other stuff as I find it.

#### Common Stuff

People should know this already, but if they don't, here it is.

* `*` returns the current location of the PC.
* `lda <value` uses zero page addressing instead of absolute addressing.

#### Addressing Modes

* Indexed post-increment: `lda value,x++`, `lda value,y++`. Expands to `lda value` followed by `inx` or `iny` respectively. Why there's no post-decrement or pre-increment/decrement functions, I don't know.
* Tagged indirect addressing. `lda [ptr].offset`. Expands to `ldy #offset` followed by `lda [ptr],y`. 
* Low and high addressing. `lda.L value` and `lda.H value`. Expand to `lda value` and `lda value+1` respectively.

#### Operators

* `<>` is equivalent to `!=`

#### Directives

* `call` - Note that `call` may not be preceded by a `.`. Used as `call proc`. More or less inserts a `jsr proc` into the code.
* `.mac` - Short for macro
* `.opt` - Allows setting assembler options in code. Syntax is `.opt e+, d-, ...`. Use + to enable flags and - to disable flags. Recognized options are
	* `l` - List assembler source. Equivalent to `.list` and `.nolist`.
	* `m` - Expand macros in listing. Equivalent to `.mlist` and `.nomlist`.
	* `w` - Enables assembler warnings. The only warnings implemented are bank overruns with included binaries.
	* `o` - Optimize. Doesn't do anything.
* `.proc`, `.endp`, `.procgroup`, `.endprocgroup`-  I still don't know how they are intended to be used. Ignore them.
* `.bank` can take an optional string parameter as name, i.e. `.bank 2, "Graphics"`. This name is not functional in any way I can tell, but it is displayed in the segment usage output. 


#### Reserved Symbols

* MagicKit reserves `MAGICKIT`as `1`.
* `_bss_end` is the largest address in BSS storage
* `_bank_base` is relevant for PCE development. It is always `0` in NESASM.
* `_nb_bank` is defined as the current number of 8KB banks used for the ROM.
* `_call_bank` is also defined as the last bank in the ROM.

#### Other

* `*` as the first character of the line turns the line into a comment
* Macro arguments can be enclosed in braces (`{...}`). Allows giving an argument containing a comma (`,`) without having it count as two arguments. Note that NESASM automatically turns indexed arguments (`value, x`) into a single argument instead of two, even without braces.
* Inbuilt functions such as `HIGH` and `LOW` are case-insensitive, unlike regular functions.
