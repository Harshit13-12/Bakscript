# 🌀 BakScript Compiler

**BakScript** is a custom, minimalistic programming language designed and implemented from scratch in C. It compiles high-level language constructs into NASM x86-64 assembly, demonstrating core compiler design principles like lexing, parsing, intermediate representation, and code generation.

## ✨ Features

- **Variables**: `num/str x = 5;`
- **Data Types**: `num`, `str`
- **Arithmetic**: `+`, `-`, `*`, `/`
- **Comparisons**: `<`, `>`
- **Print Statement**: `show(x);` or `show ("Hello");`
- **Control Flow**:
  - `when` (if)
  - `otherwise` (else)
- **Loops**:
  - `repeat` (like a `for` loop) , with nested `when-otherwise`

## 🧠 Example: BakScript Program

```baks
num x = 5;
num y = 3;
num z = x + y * 2;
show (z);

str s = "Jatin";
show (s);

when(x > y) {
    show ("x is greater");
} otherwise {
    show ("y is greater or equal");
}

repeat( num i = 0; i < 5; i = i + 1 ){
    show (i);
}
```

## ⚙️ Architecture

- **Lexer (lexer.c)**: Tokenizes input code
- **Parser (parser.c)**: Builds AST nodes for statements and expressions
- **Semantic (semantic.c)**: Type Checking, Scope Checking, Undefined Variable
- **TAC Generator (tac.c)**: Converts AST into intermediate Three-Address Code
- **Code Generator (gen.c)**: Converts TAC into NASM assembly
- **Runtime (runtime.c)**: Provides functions like show_num, show_str, and process_exit

## 🛠️ Build Instructions (Use elf64 in Linux / win64 in Windows)

1. Compile the Compiler :
   ```bash
   gcc -o bakscript src/*.c -I include
   ```
2. Run the Compiler :

   ```bash
   ./bakscript.exe filename/path
   ```

   > This generates x86_64.asm.

3. Assemble and Link :
   ```bash
    nasm -f win64 -o x86_64.obj x86_64.asm
    cl /c /Fo:runtime.obj .\link\runtime.c
    link x86_64.obj runtime.obj /SUBSYSTEM:CONSOLE /ENTRY:_start kernel32.lib
    x86_64.exe
   ```

## 📁 File Structure

```bash
Bakscript
├── batch/          # Batch scripts for building and automation
│ ├── asm.cmd       # Assemble .asm files with NASM
│ ├── compile.cmd   # Compile .c files to .exe
│ ├── error.bat
│ └── valid.bat
├── include/        # Header files
├── link/           # Linking and assembly scripts
├── src/            # Source code (lexer, parser, TAC, gen)
├── tests/          # Test programs in BakScript
├── cmd.txt         # Compilation commands
├── .gitignore      # Git ignore file
└── README.md       # This file
```

## 🔥 Sample Output

> **For:**

```baks
num x = 2 + 3 * 4;
show (x);
```

> The compiler generates:

```x86/64 nasm code
default rel

section .data
  t1: dq 0
  x: dq 0
  t2: dq 0
  t3: dq 0
  t4: dq 0

section .text
global _start
extern show_num
extern process_exit

_start:
  mov rax, 3
  mov [t1], rax
  mov rax, 4
  mov [t2], rax
  mov rax, [t1]
  imul rax, [t2]
  mov [t3], rax
  mov rax, 2
  mov [t4], rax
  mov rax, [t4]
  add rax, [t3]
  mov [x], rax
  mov rcx, [x]
  call show_num

  mov rcx, 0
  call process_exit
```

### **📌 Feature:**

## 🤝 Contributing

> Pull requests are welcome! If you're interested in compiler theory or systems programming, feel free to suggest features or report issues.

## 🧑‍💻 Author

> Jatin Mehra | Pankaj Joshi | Harshit Pandey | Gaurav Karnatak

## 🧠 Inspiration

> This project is built to understand compiler design fundamentals, from tokenizing source code to generating real machine-level assembly
