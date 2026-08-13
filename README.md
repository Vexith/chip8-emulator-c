# chip8-emulator-c
#  CHIP-8 Emulator in C

A fast, lightweight CHIP-8 emulator built from scratch in C with Visual Studio. Features a custom ASCII block terminal renderer running smoothly.
> Built as part of my **Hack Club** / Star Club devlog!

---

## 📸 Demo

![CHIP-8 ASCII Demo](demo.gif)


---

## ✨ Features

- **Core Hardware Emulation:**
  - 4KB RAM with fontset loaded at `0x000`
  - Entry point set at `0x200` for ROM binaries
  - 16 General Purpose 8-bit registers (`V0` - `VF`)
  - 16-bit Index Register (`I`) and Program Counter (`PC`)
- **Implemented Opcodes:**
  - `00E0`: Clear screen
  - `1NNN`: Jump to address `NNN`
  - `6XNN`: Set `Vx` to `NN`
  - `7XNN`: Add `NN` to `Vx`
  - `ANNN`: Set `I` to address `NNN`
  - `DXYN`: Draw 8xN sprite at position `(Vx, Vy)`
## 📋 To-Do List

### 🟢 Completed
- [x] Initialize 4KB RAM, registers (`V0`-`VF`), Index register (`I`), and Program Counter (`0x200`)
- [x] Binary ROM loader (`.ch8`)
- [x] Core Fetch-Decode-Execute CPU cycle
- [x] Key opcodes implemented (`00E0`, `1NNN`, `6XNN`, `7XNN`, `ANNN`)
- [x] Sprite rendering (`DXYN`) in ASCII terminal

### 🟡 Next Steps
- [ ] Implement all remaining CHIP-8 opcodes (35 total)
- [ ] Add 16-key hex keypad input handling
- [ ] Implement 60 Hz Delay and Sound timers
- [ ] Migrate terminal display to a native GUI window (SDL2 or Raylib)
---

## 🚀 How to Run

1. Clone this repository:
   ```bash
   git clone https://github.com/Vexith/chip8-emulator-c.git
