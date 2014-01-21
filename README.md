
Brainfuck-JIT
==============

Just another brainfuck implementation using DynASM dynamic code generation to speed up execution. It's actually just a project to try out DynASM.

Currently only support x86, but it is not hard to port to x64 (because x86 pass parameter on stack instead of register). It should works on both Windows and Linux, though only tested on Windows.

It should be faster than any brainfuck interpreter out there.

### Optmization exists
 - Grouping of multiple `+`, `-`, `>`, `<` into single instruction
 - `.` print to buffer to save overhead of calling `putchar`

### Optimization possible
 - Linear loop expansion
 - Pattern-matching known tricks
 - Input buffer -- not sure if worthwhile though
