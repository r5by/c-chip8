# c-chip8 — a tiny Chip-8 emulator in C (SDL3)

`c-chip8` is a clean, testable Chip-8 emulator written in **C11**, using **SDL3** for video and audio.  
It aims to be small, readable, and easy to extend or port. Also fits well for educational purpose.

---

## Features

- **Complete instruction set** (Cowgod-style decoding).
- **Accurate timers**: 60 Hz delay (`DT`) and sound (`ST`) timers driven by wall-clock time.
- **Sound/beeper** via SDL3 audio (soft, non-harsh tone; frequency & volume configurable).
- **Display**: 64×32 monochrome, XOR sprites with wrap-around and collision (VF).
- **Keyboard**: 16-key hex keypad with ergonomic PC mapping.
- **Deterministic core** with small, focused modules and **unit tests** (GoogleTest).

## Configure & build

Both `debug` and `release` are preset, here we use `debug` as example:

```powershell
cmake --preset msvc-ninja-debug-user
cmake --build  --preset build-debug-user
```

Artifacts are placed under:

```powershell
out/build/msvc-ninja-debug-user/
```

> The build copies required SDL3 runtime DLLs next to the executables.

## Run

Example (PONG):

```powershell
.\out\build\msvc-ninja-debug-user\chip8.exe .\ROM\GAMES\PONG.ch8
```

## Keyboard mapping

```mathematica
CHIP-8:                      PC keys:  
1  2  3  C                   1  2  3  4
4  5  6  D                   Q  W  E  R
7  8  9  E                   A  S  D  F
A  0  B  F                   Z  X  C  V

```

## Notes

- Default CPU speed is ~700 Hz (set in main.c). Adjust for ROM feel.

- Beeper defaults to ~330 Hz at gentle volume (beep_init in main.c).

- I've developed & tested it on my Windows PC using `msvc` (`Linux` or `gcc` compiler tool-chain may cause certain issues).

## Tests

GoogleTest is used here; tests are discovered by CTest.

```powershell
cmake --build --preset build-debug-user
ctest --preset test-debug-user --output-on-failure
```

Individual test executables (e.g., test_instr.exe, test_mem.exe) live next to the main binary.

## Troubleshooting

- **No sound**: ensure an audio device is available; `beep_init` logs failures.

- **Missing SDL3 DLLs**: run the executables from the build output folder or ensure DLLs are on `PATH`.

- **Black screen**: some ROMs wait for a key (`Fx0A`). Press any mapped key (e.g., `X` for `0`).

- **Performance**: adjust `CPU_HZ` in `main.c` or window scaler in `config.h`.

## License

`c-chip8` is licensed under a MIT or Apache-2.0 license. See the `LICENSE-MIT`
or `LICENSE-APACHE` file in the root for a copy.

## Acknowledgements

- Instruction reference inspired by the classic [Cowgod’s Chip-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#0.0).

--- 

Enjoy!  PRs and issues are welcome.
