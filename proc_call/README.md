# Procedure call test
Compile and run the minimal test:
~~~~~shell
make clean all run
~~~~~
Notice that this is compiled with `-O0` (no compiler optimization), otherwise it will be impossible to show a procedure call in such a simple example!

It will do... absolutely nothing. That's normal! But let's still look what happens under the hood.

## Looking inside the code: disassemble the binary
To disassemble the binary, launch
```
make dis > disassembly.S
```
Then, open `disassembly.S` in the editor.

## Launching GDB (the debugger)
To start GDB, open two terminals. In one launch
```
make all run runner_args=--gdbserver
```
This will run the GVSOC virtual platform with a GDB server. The output will be
```
GDB server listening on port 12345
```
In the second terminal, run
```
riscv32-unknown-elf-gdb BUILD/PULP/GCC_RISCV/test/test
```
Inside GDB, you need to connect to the GDB server. You can do that with the command `target remote:12345` in the GDB shell:
```
(gdb) target remote:12345
```
If everything works correctly, you will be connected to the GDB server.
```
Remote debugging using :12345
0x00000000 in ?? ()
```
The `0x00000000 in ?? ()` means you are currently stopped before entering the actual program.

## Follow the procedure call
Try to follow how the program evolves instruction by instruction.
Break at the start of the `main`:
```
(gdb) break main
(gdb) continue
(gdb) display/4i $pc
```
From this point on, you can step instruction by instruction with
```
(gdb) si
```
What happens? First, we execute
```
int x = 5;
```
which is compiled into
```
li a5, 5       # load immediate 5 into reg a5 (caller-saved)
sw a5, -20(s0) # push a5 to the stack (s0 is the frame pointer)
```
Then, we call `square(x)` -- the assembly is pretty silly because it is compiled with `-O0`:
```
lw a0, -20(s0)          # load back argument of function from stack
jal 0x1c00809c <square> # jump and link to square
```
You can check the status of registers with
```
info registers
```
In particular, register `ra` (return address) will change after the function call!
When you are inside `main`, it is:
```
(gdb) info register
ra             0x1c00912a       469799210
```
When you are inside `square`, instead, it is:
```
(gdb) info register
ra             0x1c0080ce       469795022
```
which is, in fact, the instruction immediately after the `jal`.
You can check in the disassembly what the `ra` points to in the `main` function...
After the `jal`, we enter the "prologue" of the `square` function:
```
addi sp, sp, -32 # make space on the stack and move the stack pointer
sw   s0, 28(sp)  # save previous frame pointer to the stack
addi s0, sp, 32  # set new frame pointer at top of stack frame
sw   a0, -20, s0 # save argument in the stack (useless!)
```
Then it continues with the actual body up to the point where the `square` function returns:
```
mv   a0, a5     # save return value in a0
lw   s0, 28(sp) # recover main's frame pointer from stack
addi sp, sp, 32 # remove current frame from the stack
ret             # return (actually, jalr ra, x0, 0)
```