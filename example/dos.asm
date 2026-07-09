# |=================================================================|
# | Hello there! Welcome to a beautifully simple DOS source.        |
# |                                                                 |
# | This file is meant to be a minimal example. Please feel free to |
# | learn from it, tinker with it, and make it your own! :)         |
# |=================================================================|

# |================================================|
# | Constants                                      |
# |================================================|

constant DOS {
  PrintString = $09;
  ExitCode    = $4C00;
}

# |================================================|
# | Entry Point                                    |
# |================================================|

binary $100 {
  # --- Print ---
  mov_dx_imm16(>STRING);
  mov_ah_imm8(DOS.PrintString);
  int_21h();

  # --- Exit ---
  mov_ax_imm16(DOS.ExitCode);
  int_21h();

  <STRING; # "Hello, World!" followed by CR ($0D), LF ($0A), and Terminator ($24)
  $48; $65; $6C; $6C; $6F; $2C; $20; $57; $6F; $72; $6C; $64; $21; $0D; $0A; $24;
}

# |================================================|
# | Procedures                                     |
# |================================================|

procedure mov_ah_imm8(u8) {
  $B4; @arg(0);
}

procedure mov_ax_imm16(u16) {
  $B8;
  @and(@arg(0), $FF);
  @and(@shr(@arg(0), 8), $FF);
}

procedure mov_dx_imm16(u16) {
  $BA;
  @and(@arg(0), $FF);
  @and(@shr(@arg(0), 8), $FF);
}

procedure int_21h() {
  $CD; $21;
}
