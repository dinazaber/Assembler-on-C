This project was created on Ubuntu 24.04
Compiled according to ANSI C standards.
The project contains some test files in 'test' folder with output files.
This project simulates assembler, it gets .as files as input, validates their syntax with exact errors found(if found) and if
no errors were found, outputs this files for each assembly file given:
1. .am file that contains the code after macro parsing, the program continues to run on this file
2. .ob file that contains full machine code of the program represented in hexadecimal base and in little endian.
3. .entry file that contains names of all the entry labels used with their addresses.
4. .extern file that contains names of all the extern labels used with their addresses.
All the output files are stored in the folder of source file.
This project doesn't run the assembly code that was given.

**Specifications**:
* Supported commands: add, sub, and, or, nor, move, mvh, mvlo, addi, subi, andi, ori, nori, bne, beq, blt, bgt, lb, sb, lw, sw, lh, sh, jmp, la, call, hlt
* Supported directives: .db, .dw, .dh, .entry, .extern, .asciz
* Supported registers: $0 - $31
* Memory size: 2^25 bytes
* Label syntax: Up to 31 symbols, begins with an English letter, contains only letters and numbers, must be unique, ':' right after label's name
* Macro syntax: Begins with 'mcro' word and right after comes macro's name with the same syntax as for the label but without ':' ending, ends with 'mcroend'
* No float or double numbers support
* Maximum length of each line in source file is 80 symbols
* Supports comments in source file

**How it works**:

The program does this steps for each input file:
  1. Parses all the macros with syntax checks. If any error was found, stops and goes to next input file. Otherwise, outputs .am file with parsed macros
  2. The program reads the output .am file line after line, for each line it checks the syntax and prints the first error found.
  In addition, depending on the type of the line it does this if no error was found:
  * Code line: Increments IC(instruction count) by 4
  * Data directive: Increments DC(data count) by the amount of bytes that were stored
  * .entry/.extern and labels: Stores label's name and address in the symbol table
  3. After reading a line, it stores early machine code of that line
  4. After all the file was readen, the program reads the symbol table and according to it, fills the missing machine code parts
  5. Builds .entry and .extern files according to the symbol table
