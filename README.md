<div align="center">
  <img src=".github/assets/logo.svg" width="256"  alt="Logo"/>
  <p>Nadir, an assembler from rock bottom to one of a kind.</p>
</div>

---

# Nadir Assembler

Nadir is a small, customizable assembler for generating binary files:

```asm
constant Register {
  A = 0;
  B = @add(Register.A, 1);
}

binary $100 {
  load(Register.A, $2A);
  load(Register.B, $10);
  halt();
}

procedure load(u8, u8) {
  $10;
  @arg(0);
  @arg(1);
}

procedure halt() { $FF; }
```

## Installation

You can download the latest pre-compiled binary for your system from
the [releases page](https://github.com/mikuwithbeer/Nadir/releases/latest).

## Documentation

> [!NOTE]
> This section is currently being expanded and refined.

Documentation and architectural details are maintained inside the `doc/` directory. Check out
the [README.md](doc/README.md) file for one-file documentation.

## Building From Source

> [!NOTE]
> This project aggressively targets modern toolchains.

### Prerequisites

To compile from source, make sure your environment meets the following requirements:

| Requirement | Version                                                     | 
|-------------|-------------------------------------------------------------|
| CMake       | **4.0** or newer                                            |
| C Compiler  | **C23** compatible (The latest LLVM toolchain has priority) |

### Steps

1. Clone the Repository:
   ```bash
   git clone https://github.com/mikuwithbeer/Nadir.git
   cd Nadir
   ```

2. Configure the Build:
   ```bash
   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
   ```

3. Compile the Project:
   ```bash
   cmake --build build --config Release
   ```

   Upon a successful build, the compiled binary will be located inside the `build/` directory.

## License

This project is licensed under the **Blue Oak Model License 1.0.0**.
