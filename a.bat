bison -d -oparser.cpp parser.y 
flex -otokens.cpp tokens.l
rename parser.hpp parser.cpp.h
g++ parser.cpp tokens.cpp node.cpp code.cpp main.cpp
a.exe test.cpp
nasm -f elf -F stabs output.asm -ooutput.o 
gcc -ooutput.exe output.o
output.exe
pause
