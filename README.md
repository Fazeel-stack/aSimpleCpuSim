
# Custom 32-Bit Processor Core & Assembler


A streamlined, hardware-validated 32-bit custom processor core built entirely within **Logisim-Evolution**, paired with a dedicated two-pass C++ assembly-to-machine-code translator.

This repository contains both the hardware design (`DFlipFlop.circ`) and the custom assembler (`main.cpp`) used to program it.

---

## Quick Start (Hardware & Software Simulation)

### 1. Compiling Your Code

1. Write your custom assembly application inside `c.txt` in the root directory.
2. Compile and run the C++ assembler:
   ```bash
   g++ main.cpp -o assembler
   ./assembler
   ```
   The assembler will generate `b.txt`, formatted specifically for Logisim (v3.0 hex words addressed).

### 2. Loading into Hardware

1. Open Logisim-Evolution and load the project file: `DFlipFlop.circ`.
2. Navigate into the `Ccu` (Control Unit) segment sub‑circuit.
3. Right‑click the ROM component, select **Load Image**, and choose the generated `b.txt` file.
4. Tick the master clock to watch the datapath execute your code!

---

## Architectural Overview & Hardware Design

The CPU is a fixed‑length 32‑bit load/store architecture utilizing an automated **Single‑Cycle Datapath**. Every instruction is fetched, decoded, executed, and written back in exactly one tick of the master clock.

### The Multiplexer (Mux) & Control Word Strategy

To avoid hardwired routing conflicts, data junctions use dedicated Multiplexers controlled by a synchronized **Control Word**. The Control Unit reads the **Main Opcode** and outputs a multi‑bit Control Word that acts as a remote control for the CPU:

- **ALU Source Mux (ALUSrc)** – selects whether input B of the ALU receives a variable from Register Port 2 (0) or a constant Immediate value (1).
- **Write‑Back Mux (MemToReg)** – selects whether the Write‑Back port of the Register File receives its data from the ALU output (0) or from the Data RAM (1).
- **PC Source Logic (PCSrc)** – an internal AND Gate evaluates if the Control Word’s Branch line is active **AND** the ALU’s Zero Flag is pulled high. If both match, it flips the PC Mux to execute a jump.

---

## 📐 Instruction “Styles” (Bit Formats)

Instead of forcing every instruction to share a rigid layout, this architecture uses the 3‑bit **Main Opcode** to dynamically change how the instruction packet is sliced and parsed by the hardware.

### 1. R‑Type Format (Register‑to‑Register Math)

**Triggered by:** Main Opcode `000`

**Hardware Routing:** Routes three 3‑bit slices directly to the Register File addresses. The hardware passes the ALU Op bits directly into the ALU to determine the math flavour (add, sub, etc.).

```
┌───────────┬───────────┬───────────┬───────────┬───────────┬───────────────────┐
│  Main Op  │  ALU Op   │  Reg Dest │   Reg A   │   Reg B   │   Zero Padding    │
│   [31:29]  │   [28:26] │   [25:23] │   [22:20] │   [19:17] │      [16:0]       │
└───────────┴───────────┴───────────┴───────────┴───────────┴───────────────────┘
```

### 2. I‑Type Format (Immediate Constants & Memory)

**Triggered by:** Main Opcodes `010`, `011`, `100`, `111`

**Hardware Routing:** Forces the second register read port to shut down. Opens a massive 26‑bit immediate bus directly from the instruction packet into the ALU.

```
┌───────────────────────┬───────────────────────┬───────────────────────────────┐
│     Main Opcode       │   Target Register     │     Immediate Value / Data    │
│       [31:29]         │       [28:26]         │            [25:0]             │
└───────────────────────┴───────────────────────┴───────────────────────────────┘
```

### 3. B/J‑Type Format (Branching & Jumps)

**Triggered by:** Main Opcodes `001`, `101`, `110`

**Hardware Routing:** Interprets the remaining space as a wide 29‑bit target address, funneling it straight into the Program Counter logic to override sequential execution.

```
┌───────────────────────────────────────┬───────────────────────────────────────┐
│              Main Opcode              │            Target Address            │
│                [31:29]                │                [28:0]                 │
└───────────────────────────────────────┴───────────────────────────────────────┘
```

---

## Instruction Set Architecture (ISA) Map

| Mnemonic | Main Opcode (Binary) | ALU Opcode (Binary) | Format Type | Target Hardware Path                                  |
|----------|----------------------|---------------------|-------------|-------------------------------------------------------|
| `add`    | `000`                | `001`               | R‑Type      | Reg File → ALU → Reg File                            |
| `sub`    | `000`                | `000`               | R‑Type      | Reg File → ALU → Reg File                            |
| `and`    | `000`                | `011`               | R‑Type      | Logical Bitwise AND Gates                             |
| `or`     | `000`                | `010`               | R‑Type      | Logical Bitwise OR Gates                              |
| `cmp`    | `000`                | `100`               | R‑Type      | Internal ALU Subtraction (Flags only)                 |
| `shr`    | `000`                | `101`               | R‑Type      | Right‑Shift Barrel Registers                          |
| `sfl`    | `000`                | `110`               | R‑Type      | Left‑Shift Barrel Register                            |
| `xor`    | `000`                | `111`               | R‑Type      | Logical Bitwise XOR Gates                             |
| `bgt`    | `001`                | N/A                 | B‑Type      | Jump if (ALU Sign == 0) and (Zero == 0)              |
| `addi`   | `010`                | N/A                 | I‑Type      | Reg File + Sign‑Extended Imm → Reg File              |
| `load`   | `011`                | N/A                 | I‑Type      | Address Calc → Data RAM Read → Reg File              |
| `store`  | `100`                | N/A                 | I‑Type      | Address Calc → Data RAM Write                        |
| `beq`    | `101`                | N/A                 | B‑Type      | Jump if ALU Zero Flag == 1                           |
| `jmp`    | `110`                | N/A                 | B‑Type      | Unconditional Address Override to PC                  |
| `mov`    | `111`                | N/A                 | I‑Type      | Raw Imm value straight → Reg File                    |

---

## Assembly Syntax & Command Reference

- Every operational line **must** end with a semicolon (`;`).
- The assembler automatically ignores spaces and basic formatting punctuation.
- Labels used for branching loops must be written with a trailing colon (`label_name:`).
- Valid Registers: `r0` through `r7`.

### 1. Register Math & Logic (R‑Type)

**Syntax:** `<operation> <destination_register> <source_A> <source_B>;`

```asm
add r5 r3 r4;   ; Adds r3 + r4, saves to r5.
sub r4 r7 r6;   ; Subtracts r7 - r6, saves to r4.
and r1 r2 r3;   ; Bitwise AND on r2 & r3, saves to r1.
or  r1 r2 r3;   ; Bitwise OR  on r2 | r3, saves to r1.
xor r1 r2 r3;   ; Bitwise XOR on r2 ^ r3, saves to r1.
shr r1 r2 r0;   ; Shifts bits in r2 right, saves to r1.
sfl r1 r2 r0;   ; Shifts bits in r2 left,  saves to r1.
cmp r1 r2 r0;   ; Internal subtraction of r1 - r2 to update ALU Zero/Sign flags (does not overwrite register).
```

### 2. Immediate Constants & Memory (I‑Type)

**Syntax:** `<operation> <target_register> <immediate_value>;`

```asm
mov r2 5;       ; Hardcodes the value 5 directly into r2. (ALUSrc=1, bypasses register reads).
addi r1 10;     ; Adds the constant 10 to the current value of r1 and saves it back to r1.
load r1 32;     ; Reads data from Data RAM at address 32 and copies it into r1. (MemToReg=1).
store r3 64;    ; Takes the value in r3 and commits it to Data RAM at address 64. (MemWrite=1, RegWrite=0).
```

### 3. Execution Control & Jumps (B‑Type)

**Syntax:** `<operation> <label_name>;`

```asm
loop:
mov r3 5;
add r5 r3 r4;
bgt loop;       ; Branch if Greater Than. Jumps to loop: ONLY if ALU evaluation shows operand A > operand B.
beq loop;       ; Branch if Equal. Jumps to loop: ONLY if the ALU Zero Flag from the last operation is 1.
jmp loop;       ; Unconditional jump. Instantly forces the PC to the address of `loop:`.
```

---

## Repository Structure

```
.
├── DFlipFlop.circ   # Logisim-Evolution hardware design file
├── main.cpp          # C++ two‑pass assembler source
├── c.txt             # Your custom assembly input (write your code here)
├── b.txt             # Generated hex machine code (loaded into ROM)
└── README.md         # This file
```

---

## Requirements

- **Logisim-Evolution** – any recent version (recommended 3.x)
- **C++11 compatible compiler** – e.g. `g++`, `clang++`

---

## License

This project is provided for educational and hobbyist use. Feel free to study, modify, and share.





