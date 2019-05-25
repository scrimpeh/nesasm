## NESASM-S (Working Title)

This is a fork of NESASM 3.1, intended to provide useful features to the assembler while maintaining maximum compatibility to existing NESASM projects. This fork is intended to serve as a simple drop-in replacement to the existing assembler.

Note that while the PCE-Engine assembler is still included in the source, it is untested and not updated. Compile and use PCEAS at your own risk.

The repository has been forked from [camsaul/nesasm](https://github.com/camsaul/nesasm). I've added a CMake build for platform-independent building. See the original repository for further details, resources and credits.

#### Changes to NESASM 3.1
* The Program Counter (`*`) is now a symbol instead of a value. It may be used as an operand in the `BANK` and `PAGE` functions. `BANK(*)` returns the current bank of the assembler.
* `PAGE` now allows values in addition to symbols. E.g. `PAGE($C000) ; -> 6`

More to be added!

#### Further Reading and Links

* [`usage.txt`](https://raw.githubusercontent.com/scrimpeh/nesasm/master/usage.txt)
* [Bunnyboy's source](http://www.nespowerpak.com/nesasm/)
* [Original MagicKit assembler](http://www.magicengine.com/mkit/)
* [camsaul/nesasm Readme](https://github.com/camsaul/nesasm) (Contains lots of useful links!)
