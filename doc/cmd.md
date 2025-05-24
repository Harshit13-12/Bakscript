### For compiling the phases of compiler :
```powershell
gcc -o bakscript src/*.c -I include
```
```powershell
./bakscript.exe filename/path
```

OR

#### Just go inside the batch folder and run :
`1. compile.md`<br>
`2. asm.md`

### For linking the asm with runtime.c : ( Run in x64 Native Tools Command For VS 2022 )
- cd Foldername : reach the directory where the x86_64.asm is created
- `nasm -f win64 -o x86_64.obj x86_64.asm`
- `cl /c /Fo:runtime.obj .\link\runtime.c`
- `link x86_64.obj runtime.obj /SUBSYSTEM:CONSOLE /ENTRY:_start kernel32.lib`

### Run the code :
```bash
 x86_64.exe
 ```