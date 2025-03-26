# Interrupt test
Compile and run the interrupt test:
~~~~~shell
make clean all run
~~~~~

The expected output is the following:
```
Ticks=334
HELLO 1 (if =1, ISR has worked)!
Start=2
End=336 vs set=336
```

## Available tests
You can change the target tests (defualt `test1.c`) by modifying the `Makefile` appropriately:
 - `test1.c`: simple timer + interrupt test with wait-for-interrupt after 10 milliseconds
 - `test2.c`: loop of 64 intervals of 10ms each with timer+WFI; timer is re-armed outside of the IRQ handler
 - `test3.c`: same as `test2.c` but with GPIO toggling
 - `test4.c`: same as `test3.c` but timer re-arming and GPIO toggling are performed *inside* the IRQ handler
 - `test5.c`: simple timer + interrupt test without WFI (emulates asynchronous interrupt)
 - `uart.c`: bit-banged UART

## What to check
### Disassembling the binary
To disassemble the binary, launch
```
make dis > disassembly.S
```
Then, open `disassembly.S` in the editor.
As the tab size used internally is 8, it is recommended to set the editor tab size to 8 as well to read this file (*CTRL+SHIFT+P* > *Change tab display size*).
The most interesting piece of code is `timer_handler`, which should look like this for `test1.c`:
```
1c008706 <timer_handler>:
1c008706:	1141                addi    sp,sp,-16
1c008708:	c43e                sw	    a5,8(sp)
1c00870a:	1c0017b7            lui     a5,0x1c001
1c00870e:	26078793            addi    a5,a5,608 # 1c001260 <_edata>
1c008712:	c63a                sw      a4,12(sp)
1c008714:	4398                lw      a4,0(a5)
1c008716:	0705                addi    a4,a4,1
1c008718:	c398                sw      a4,0(a5)
1c00871a:	4732                lw      a4,12(sp)
1c00871c:	47a2                lw      a5,8(sp)
1c00871e:	0141                addi    sp,sp,16
1c008720:	30200073            mret
```
Let's examine this step by step.
First, we create a stack frame for the handler function:
```
1c008706:	1141                	addi	sp,sp,-16
```
Then, we save the current value of a register that is used inside the IRQ handler (namely `a5`), which is then used to host the static address `0x1c001260` into `a5`: this is the address of the global variable `global`:
```
1c008708:	c43e                	sw	a5,8(sp)
1c00870a:	1c0017b7          	lui	a5,0x1c001
1c00870e:	26078793          	addi	a5,a5,608 # 1c001260 <_edata>
```
Then, we save `a4` in the stack and then we use it as temporary register to load `global`, increment it by 1, and then store it back to memory:
```
1c008712:	c63a                	sw	a4,12(sp)
1c008714:	4398                	lw	a4,0(a5)
1c008716:	0705                	addi	a4,a4,1
1c008718:	c398                	sw	a4,0(a5)
```
As cleanup, we reload the original values of `a4`, `a5` and destroy the stack frame:
```
1c00871a:	4732                	lw	a4,12(sp)
1c00871c:	47a2                	lw	a5,8(sp)
1c00871e:	0141                	addi	sp,sp,16
```
Finally, we return with `mret`:
```
1c008720:	30200073          	mret
```

### Tracing execution
To run with instruction tracer,
```
make run runner_args="--trace=insn" > trace.log
```
and open `trace.log` in the editor.

### Visual waveforms (GTKWave)
The following assumes GTKWave (https://gtkwave.github.io/gtkwave/) is installed from the instructions available online.
For an alternative, see Surfer below.
To save waveforms for visualization, run
```
make run runner_args="--vcd --event=.*"
```
Then you can run
~~~~~shell
gtkwave BUILD/PULP/GCC_RISCV/all.vcd
~~~~~
Inside GTKWave, you can add relevant waveforms through the hierarchy navigator:
![example of GTKWave hierarchy](.image/image.png)
Most interesting signals are located within `chip > soc > fc` (in particular `irq_enable`, `irq_enter`, `irq_exit`, `busy`, `pc`) and within `chip > soc > gpio` (in particular `padout`).

### Visual waveforms (Surfer)
To install Surfer as a VSCode extension, open the extensions marketplace (`CTRL+SHIFT+P` on Linux/Windows, `CMD+SHIFT+P` on MacOS), then type `Extensions: Install extensions` and search for **surfer** from *surfer-project*.
For an alternative, see GTKWave above. *Notice that due to GVSoC limitations, support is currently more limited.*
To save waveforms for visualization, run
```
make run runner_args="--vcd --event=/chip/soc/fc/pc --event=/chip/soc/fc/irq_enable --event=/chip/soc/gpio/padout --event-format=vcd"
```
You can open the generated VCD directly from the VSCode interface, finding it in `BUILD/PULP/GCC_RISCV/all.vcd`.

### (Printable) ASCII character table
Decimal | Character | Hexadecimal | Binary
--------|-----------|-------------|---------
29      | ¡         | 1D          | 00011101
30      | ¢         | 1E          | 00011110
31      | ¤         | 1F          | 00011111
32      |           | 20          | 00100000
33      | !         | 21          | 00100001
34      | "         | 22          | 00100010
35      | #         | 23          | 00100011
36      | $         | 24          | 00100100
37      | %         | 25          | 00100101
38      | &         | 26          | 00100110
39      | '         | 27          | 00100111
40      | (         | 28          | 00101000
41      | )         | 29          | 00101001
42      | *         | 2A          | 00101010
43      | +         | 2B          | 00101011
44      | ,         | 2C          | 00101100
45      | -         | 2D          | 00101101
46      | .         | 2E          | 00101110
47      | /         | 2F          | 00101111
48      | 0         | 30          | 00110000
49      | 1         | 31          | 00110001
50      | 2         | 32          | 00110010
51      | 3         | 33          | 00110011
52      | 4         | 34          | 00110100
53      | 5         | 35          | 00110101
54      | 6         | 36          | 00110110
55      | 7         | 37          | 00110111
56      | 8         | 38          | 00111000
57      | 9         | 39          | 00111001
58      | :         | 3A          | 00111010
59      | ;         | 3B          | 00111011
60      | <         | 3C          | 00111100
61      | =         | 3D          | 00111101
62      | >         | 3E          | 00111110
63      | ?         | 3F          | 00111111
64      | @         | 40          | 01000000
65      | A         | 41          | 01000001
66      | B         | 42          | 01000010
67      | C         | 43          | 01000011
68      | D         | 44          | 01000100
69      | E         | 45          | 01000101
70      | F         | 46          | 01000110
71      | G         | 47          | 01000111
72      | H         | 48          | 01001000
73      | I         | 49          | 01001001
74      | J         | 4A          | 01001010
75      | K         | 4B          | 01001011
76      | L         | 4C          | 01001100
77      | M         | 4D          | 01001101
78      | N         | 4E          | 01001110
79      | O         | 4F          | 01001111
80      | P         | 50          | 01010000
81      | Q         | 51          | 01010001
82      | R         | 52          | 01010010
83      | S         | 53          | 01010011
84      | T         | 54          | 01010100
85      | U         | 55          | 01010101
86      | V         | 56          | 01010110
87      | W         | 57          | 01010111
88      | X         | 58          | 01011000
89      | Y         | 59          | 01011001
90      | Z         | 5A          | 01011010
91      | [         | 5B          | 01011011
92      | \         | 5C          | 01011100
93      | ]         | 5D          | 01011101
94      | ^         | 5E          | 01011110
95      | _         | 5F          | 01011111
96      | `         | 60          | 01100000
97      | a         | 61          | 01100001
98      | b         | 62          | 01100010
99      | c         | 63          | 01100011
100     | d         | 64          | 01100100
101     | e         | 65          | 01100101
102     | f         | 66          | 01100110
103     | g         | 67          | 01100111
104     | h         | 68          | 01101000
105     | i         | 69          | 01101001
106     | j         | 6A          | 01101010
107     | k         | 6B          | 01101011
108     | l         | 6C          | 01101100
109     | m         | 6D          | 01101101
110     | n         | 6E          | 01101110
111     | o         | 6F          | 01101111
112     | p         | 70          | 01110000
113     | q         | 71          | 01110001
114     | r         | 72          | 01110010
115     | s         | 73          | 01110011
116     | t         | 74          | 01110100
117     | u         | 75          | 01110101
118     | v         | 76          | 01110110
119     | w         | 77          | 01110111
120     | x         | 78          | 01111000
121     | y         | 79          | 01111001
122     | z         | 7A          | 01111010
123     | {         | 7B          | 01111011
124     | |         | 7C          | 01111100
125     | }         | 7D          | 01111101
126     | ~         | 7E          | 01111110
