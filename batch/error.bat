gcc -c ..\src\lexer.c ..\src\parser.c ..\src\ast.c ..\src\symbol_table.c ..\src\semantic.c ..\src\tac.c ..\src\gen.c ..\src\main.c -I..\include
gcc -o bakscript.exe lexer.o parser.o ast.o symbol_table.o semantic.o tac.o gen.o main.o
.\bakscript.exe ..\tests\error\semantic_errors.bak
