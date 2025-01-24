# Minimal test
Compile and run the minimal test:
~~~~~shell
make clean all run
~~~~~

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

## One instruction at a time from the `_start`!
Try to follow how the program evolves instruction by instruction.
You can find a more complete GDB cheat sheet below, but for this experiment you can simply follow the instructions here.
The first instruction is at address `0x1c008080`, so break there:
```
(gdb) b 0x1c008080
Breakpoint 1 at 0x1c008080: file /app/pulp-sdk/rtos/pulpos/common/kernel/crt0.S, line 133.
(gdb) c
(gdb) display/4i $pc
1: x/4i $pc
=> 0x1c008080 <_start>: auipc   a0,0x0
   0x1c008084 <_start+4>:       addi    a0,a0,1304
   0x1c008088 <_start+8>:       jr      a0
   0x1c00808c <pos_illegal_instr>:      j       0x1c00808c <pos_illegal_instr>
```
Now, proceed one instruction at a time using `si`. 
For example, at the first step,
```
(gdb) si
0x1c008084      133         la  a0, pos_init_entry
1: x/4i $pc
=> 0x1c008084 <_start+4>:       addi    a0,a0,1304
   0x1c008088 <_start+8>:       jr      a0
   0x1c00808c <pos_illegal_instr>:      j       0x1c00808c <pos_illegal_instr>
   0x1c008090 <pos_no_irq_handler>:     mret
```
Try to follow the control flow of the code through the `disassembly.S`.

## GDB cheat sheet

#### Breakpoints
To add a breakpoint, use 
```break <breakpoint>```
or simply
```b <breakpoint>```
You can set a breakpoint:
- on a symbol name (e.g., a function), e.g., `b main`
- on a specific instruction address with `b *0x1c008080`
- on a source code line, e.g., `b test.c:24`

#### Continuing
To continue execution up to the next breakpoint, use
```continue```
or simply
```c```

#### Stepping
To step execution, you can either:
- step to the next line of code, including procedure calls:
```step```
or simply
```s```
- step to the next line of code, *excluding* procedure calls:
```next```
or simply
```n```
- step to the next instruction:
```stepi```
or simply
```si```

#### Printing variables/memory locations
To print the value of a variable, use:
```print varname```
or simply
```p varname```
If you want to print the content of a specific memory address, use:
```p *0x1c008080```

#### Print the content of RISC-V registers
To print the content of RISC-V registers, use:
```info registers```

#### Print function call backtrace
To print the backtrace of the levels of the stack, use
```backtrace```
or simply
```bt```

### Print next assembly instructions
To set up GDB to print the next few assembly instructions (e.g., the current one + the next three), use
```display/4i $pc```

