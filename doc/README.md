# Documentation

Welcome to the official guide for the **Nadir**[^1] assembler! This document covers its syntax, core features, and basic
usage so you can get up and running smoothly.

---

## Introduction

Nadir is a small, customizable assembler for generating binary files.

Unlike most assemblers that are strictly tied to specific architectures (~~like x86, RISC-V, or ARM~~), Nadir hands you
a completely blank canvas. It doesn't come with any built-in instructions or registers. Instead, it gives you a dead
simple language to craft your very own architecture.

---

## Core Ideas

### Platform Agnostic

Nadir evaluates your expressions and writes raw bytes to a file. Because it makes zero assumptions, you get to make all
the rules. Whether you are targeting a custom virtual machine or an obsolete 8-bit CPU, Nadir is flexible enough to
handle it.

### Procedures

In Nadir, standard instructions are just procedures that you define yourself. A procedure simply says what arguments it
needs and exactly how to pack them into bytes. You map a readable name to a bitwise operation, and what you write is
exactly what you get.

### Compile-Time Evaluation

Functions that start with `@` are executed entirely at compile time. They provide a way to perform calculations and
manipulate data before the final binary is written.

You can drop these functions anywhere a value is expected. They are perfect but not limited to:

- Calculating jump offsets
- Shifting bit fields
- Building opcodes

Since all this calculation happens upfront, your final binary only contains the static results.

---

## Syntax and Structure

The syntax is designed to be completely lean and stays out of your way. It simply gives you a consistent way to define
your own rules and pack data together. A full program is just a mix of constants, procedures, and a singular binary
declaration.

### Comments

Comments are the easiest way to leave notes for yourself. Start a line with `#`, and the lexer ignores everything after
it.

> [!TIP]
> You can put comments on their own line or tuck them right next to your code. Future you will thank you!

```asm
# This explains the next step
move(Register.A, 10); # This note sits right next to the code
```

### Numbers

The assembler understands both standard decimal and hexadecimal numbers.

Under the hood, the compiler treats every number as a massive `i128` by default. When it is time to write the output, it
automatically shrinks them down to `u8`.

#### Decimal

Write them exactly as you normally would, with or without a sign:

- `123`
- `-456`
- `+789`

You can also use underscores to separate groups of digits and keep things readable:

- `1_234_567_890` (equivalent to `1234567890`)

#### Hexadecimal

Hexadecimals start with `$` and might include an optional sign. This is usually the best format for raw bytes and memory
addresses:

- `$FF`
- `$+ABC`
- `$-DEF`
- `$0AFed7`

Just like decimals, you can use underscores here too:

- `$AD_CD` (equivalent to `$ADCD`)

### Data Types

Explicit data types enforce strict type safety inside your procedures, making sure you don't accidentally pass a massive
value into a space meant for a single byte.

- **Unsigned:** `u8`, `u16`, `u32`, `u64`
- **Signed:** `i8`, `i16`, `i32`, `i64`

They are also used to force a value to be cast to a specific type when writing to the output file.

### Constants

Hardcoded numbers turn into a mess quickly. If you need a value in more than one place, wrap it in a `constant` block.
This gives your numbers readable names and keeps your code organized.

> [!IMPORTANT]
> You cannot use memory addresses inside a constant block because the final layout of the binary doesn't exist yet!

```asm
# A simple constant block
constant Register1 {
  First = 0;
  Second = 1;
  Last = $200;
}

# This one is a bit more complex, using compile-time evaluation to calculate values
constant Register2 {
  A = 0;
  B = @add(Register.First, 1); # 1
  C = @mul(Register.Last, 2);  # 1024
}

# You can easily expand on a constant defined earlier
constant Register2 {
  Internal = Register1.Second; # 1
}
```

> [!TIP]
> You can expand on a constant block multiple times.

> [!IMPORTANT]
> You can reference a value defined earlier, but you cannot look into the future to reference a value defined later.

### Compile-Time Functions

The compiler includes built-in functions to manipulate data before the binary is written. They always start with an `@`
and they are often known as **comptime**.

Because there is no runtime environment, these functions do all the work while the code is building. They take inputs,
crunch the math, and return a final number before a single byte hits the output file. You can also nest them as deeply
as you need.

#### Utility

| Function                      | Description                                                                 |
|:------------------------------|:----------------------------------------------------------------------------|
| **`@arg(index)`**             | Reads an argument passed into a procedure. *Only usable inside procedures.* |
| **`@cast(value, type)`**      | Forces a value into a specific data type.                                   |
| **`@clamp(value, min, max)`** | Keeps a value safely between a minimum and a maximum.                       |
| **`@max(left, right)`**       | Returns the larger of two values.                                           |
| **`@min(left, right)`**       | Returns the smaller of two values.                                          |

#### Math

| Function                     | Description                                             |
|:-----------------------------|:--------------------------------------------------------|
| **`@abs(value)`**            | Returns the absolute (positive) value.                  |
| **`@neg(value)`**            | Flips positives to negatives, and vice versa.           |
| **`@add(first, second...)`** | Adds two or more values together. **(Variadic!)**       |
| **`@sub(left, right)`**      | Subtracts `right` from `left`.                          |
| **`@mul(left, right)`**      | Multiplies `left` by `right`.                           |
| **`@div(left, right)`**      | Divides `left` by `right`.                              |
| **`@mod(left, right)`**      | Returns the remainder after dividing `left` by `right`. |

#### Bitwise

| Function                                  | Description                                                                                 |
|-------------------------------------------|---------------------------------------------------------------------------------------------|
| **`@or(first, second...)`**               | Performs a bitwise OR operation between given values. **(Variadic!)**                       |
| **`@and(first, second...)`**              | Performs a bitwise AND operation between given values. **(Variadic!)**                      |
| **`@xor(first, second...)`**              | Performs a bitwise XOR operation between given values. **(Variadic!)**                      |
| **`@shl(value, bits)`**                   | Shifts the bits of `value` to the left by the specified number of `bits`.                   |
| **`@shr(value, bits)`**                   | Shifts the bits of `value` to the right by the specified number of `bits`.                  |
| **`@not(value)`**                         | Flips every single bit inside your `value`.                                                 |
| **`@bswap(value, width)`**                | Reverses the byte order of the `value`. `width` must be either `16`, `32`or `64`.           |
| **`@mask(width)`**                        | Generates a right-aligned bitmask of consecutive `1`s of the specified `width`.             |
| **`@insert(base, value, offset, width)`** | Inserts a `value` into a `base` at a specific `offset`, constrained to a specific `width`.  |
| **`@extract(value, offset, width)`**      | Extracts a chunk of bits of a specific `width` from a `value` starting at a given `offset`. |

#### Logic and Comparison

> [!NOTE]
> For all comparison and logic functions, Nadir returns `1` for true and `0` for false.

| Function                               | Description                                                                                        |
|----------------------------------------|----------------------------------------------------------------------------------------------------|
| **`@assert(condition, value)`**        | If `condition` is true, returns `value`. Otherwise, reports an error at compile-time.              |
| **`@if(condition, success, failure)`** | If `condition` is true, returns `success`. Otherwise, returns `failure`.                           |
| **`@eq(left, right)`**                 | Checks whether `left` is exactly equal to `right`.                                                 |
| **`@neq(left, right)`**                | Checks whether `left` is not equal to `right`.                                                     |
| **`@lt(left, right)`**                 | Checks whether `left` is strictly less than `right`.                                               |
| **`@gt(left, right)`**                 | Checks whether `left` is strictly greater than `right`.                                            |
| **`@le(left, right)`**                 | Checks whether `left` is less than or equal to `right`.                                            |
| **`@ge(left, right)`**                 | Checks whether `left` is greater than or equal to `right`.                                         |
| **`@lor(left, right)`**                | Performs a logical OR. Returns `1` if either `left` or `right` is non-zero.                        |
| **`@land(left, right)`**               | Performs a logical AND. Returns `1` if both `left` and `right` are non-zero.                       |
| **`@lnot(value)`**                     | Performs a logical NOT. Instantly turns any non-zero value into a `0`, and flips a `0` into a `1`. |

### Addresses

When building a binary, you will constantly need to navigate around. Maybe you want to loop back to the start of a
routine or read a block of data sitting at the end of your file. Nadir handles this by acting like pins on a map.

#### Store Address

To mark a spot in your code, use the `<` symbol followed by an **UPPERCASE ALPHABETIC** name. Think of it as dropping a
pin
on a map.

```asm
binary 0 {
  <START;
  # ...
  <DATA; $10; $20; $3F;
  # ...
}
```

Writing this does not add any raw bytes to your final file.

> [!IMPORTANT]
> Keep in mind that the address is **only valid inside the binary block**.

#### Load Address

When you actually need to travel to that pin, use the `>` symbol followed by the name.

```asm
binary 0 {
  # ...
  read(>DATA);
  # ...
  jump(>START);
}
```

This hands you the literal memory address of the spot you saved earlier.

> [!WARNING]
> Loading an address is **only allowed inside a procedure call** (exactly where you would normally use it).

The type of the value is up to you. If you need a 16-bit address, just declare the procedure to take a `u16`. If you
need a single byte, use `u8`.

> [!CAUTION]
> Nadir strictly checks the size of the value and will throw an error and halt compilation if it doesn't fit.

### Procedures

This is where you actually build your custom instructions.

When you write a procedure, you declare exactly what data types it needs. This acts as a safety net, stopping you from
accidentally passing a 64-bit address into an instruction that only has room for a single byte.

> [!CAUTION]
> If you pass the wrong size, the assembler will throw an error and stop the build immediately.

```asm
# A simple instruction to clear the screen. 
# It just writes two hardcoded bytes to the file.
procedure clear() {
  $00; $E0;
}

# An instruction to load a value into a specific register.
procedure load_register(u8, u8) {
  @or($60, @and(@arg(0), $0F));
  @arg(1);
}

# A jump instruction that takes a 16-bit memory address.
procedure jump(u16) {
  @or($10, @and(@shr(@arg(0), 8), $0F));
  @and(@arg(0), $FF);
}

```

> [!TIP]
> As you can see, inside the procedure itself, you use `@arg(index)` to grab the value of the argument at that specific
> position.

> [!CAUTION]
> Keep these two strict rules in mind when writing your own procedures:
>
> 1. You **cannot** call a procedure from inside another procedure. This keeps the logic flat and predictable.
> 2. Every statement inside a procedure **must** evaluate to exactly one byte. Since a procedure is basically just a
     blueprint for a sequence of bytes, you have to pack them one by one.

### Binary Block

This is the anchor of your entire program. While procedures act as your instruction templates and constants act as your
references, the binary block is where the actual file gets written. Every program must have **exactly one**.

When you declare the block, you have to hand it a starting memory address, like `$200`.

> [!IMPORTANT]
> This number is incredibly important. It tells the compiler exactly where your program expects to live in memory once
> it runs. If you get the starting address wrong, **every single address inside your program will point to the wrong
place.**

Inside the block, the compiler just moves sequentially. Every time you call a procedure, read a constant, or write a raw
number, the compiler resolves and appends those literal bytes to the output buffer.

```asm
binary $200 {
  clear();
  move(Register.A, $10);

  <LOOP;
  jump(>LOOP);

  # Static data lives right next to the code
  <SPRITE_DATA; $E0; $90; $E0; $90; $E0;
}

```

> [!TIP]
> Because the assembler does not separate executable logic from static data, the binary block holds everything. You can
> write your main execution loop, and then right underneath it, dump the raw hex values for a lookup table or a
> character sprite.

> [!NOTE]
> The location of the binary block is **not** important.

---

## Compiling Your Program

Now it is time to turn that beautiful text into cold, hard machine code.

The compiler runs directly from your terminal:

```
Nadir -i main.asm
```

> [!NOTE]
> If you don't specify an output file, the compiler will create `out.bin` for you.

### Flags and Options

- `-h, --help`: Display help information, including all available flags and options.
- `-v, --version`: Display the current version of the Nadir assembler.
- `-d, --dry-run`: Enable dry run mode. The assembler will parse the input file and report any errors, but it will not
  generate an output file.
- `-i, --input <file>`: Specify the input file. This is required.
- `-o, --output <file>`: Specify the output file. If not provided, defaults to `out.bin`.

---

## Sample Programs

> [!WARNING]
> There are only a few sample programs available at the moment.

Some sample programs are available in the [example](../example) folder.

---

## Under The Hood

Nadir is written entirely in **C23**[^2].

There are no heavy frameworks or hidden dependencies included, just `libc`. Because of this strict diet, the whole
assembler compiles down to a tiny executable that easily fits under **100KB**.

Keeping the design simple also makes it fast. On an **M4 MacBook Air**, Nadir assembles around **three million lines of
code per second**. To achieve that, it keeps most of its data in memory instead of repeatedly reading from or writing to
disk. The trade-off is memory usage grows with the size of your source code.

### Memory Usage

|  Source Size   | Peak Memory |
|:--------------:|------------:|
|  **1k lines**  |       ~1 MB |
| **10k lines**  |       ~7 MB |
| **100k lines** |      ~63 MB |
| **500k lines** |     ~290 MB |
|  **1M lines**  |     ~570 MB |

While it is not fully memory-optimized, you can compile an average project in a fraction of a second with a memory cost
you will barely notice.

---

## Appendix

### Grammar

You can find the reference grammar rules for the Nadir in the [grammar](grammar.pest) file. It uses **Pest**[^3]
structure to define the language.

### References

[^1]: Nadir generally refers to the lowest point; chosen here because this assembler gives you the absolute lowest-level
control possible over your binary output. As a fun linguistic bonus, nadir also translates to "rare" or "scarce" in
Turkish. Perfect fit for an assembler that lets you build your own rare, custom architectures!

[^2]: C23 is the current open standard for the C programming language: https://cppreference.com/c/23.

[^3]: Although Pest is a Rust project, its PEG grammar syntax is clean and expressive. They also provide an online
playground for testing grammars: https://pest.rs/.
