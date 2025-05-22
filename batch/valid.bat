gcc -c ..\include\ast.h ..\include\lexer.h ..\include\parser.h ..\include\symbol_table.h ..\include\semantic.h ..\include\tac.h ..\include\gen.h ..\src\lexer.c ..\src\parser.c ..\src\ast.c ..\src\symbol_table.c ..\src\semantic.c ..\src\tac.c ..\src\gen.c ..\src\main.c
gcc -o bakscript.exe lexer.o parser.o ast.o symbol_table.o semantic.o tac.o gen.o main.o
.\bakscript.exe ..\tests\main\test.bak