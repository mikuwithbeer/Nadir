![Logo](.github/assets/logo.svg)

> [!WARNING]
> **This project is pre-stable!**
>
> Features, APIs, language syntax, and architecture may change in any release. Backward compatibility is not guaranteed.

Nadir, an assembler from rock bottom to one of a kind.

---

## Installation

You can download the latest pre-compiled binary for your system from
the [releases page](https://github.com/mikuwithbeer/Nadir/releases/latest).

---

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

---

## Documentation

> [!NOTE]
> This section is currently being expanded and refined.

Documentation and architectural details are maintained inside the `doc/` directory.
Check out the [README.md](doc/README.md) file for one-file documentation.

---

## License

This project is licensed under the **Blue Oak Model License 1.0.0**.
