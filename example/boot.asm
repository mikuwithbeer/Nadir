# |=================================================================|
# | Hello there! Welcome to a beautifully simple boot sector.       |
# |                                                                 |
# | This file is meant to be a minimal example. Please feel free to |
# | learn from it, tinker with it, and make it your own! :)         |
# |=================================================================|

# |================================================|
# | Constants                                      |
# |================================================|

constant BIOS {
  VideoMode = $00;
  Teletype = $0E;
}

constant Graphics {
  VGA = $13;
  Foreground = $0D;
}

constant Register8 {
  AL = 0;
  BL = 3;
  AH = 4;
}

constant Register16 {
  AX = 0;
  SI = 6;
}

constant Segment {
  DS = 3;
}

constant Condition {
  Equal = 4;
}

# |================================================|
# | Entry Point                                    |
# |================================================|

binary $7C00 {
  # --- Data Segment ---
  xor_r16_r16(Register16.AX, Register16.AX);
  mov_sreg_r16(Segment.DS, Register16.AX);

  # --- Video Mode ---
  mov_r8_imm8(Register8.AH, BIOS.VideoMode);
  mov_r8_imm8(Register8.AL, Graphics.VGA);
  int_imm8($10);

  # --- Print ---
  mov_r16_imm16(Register16.SI, >STRING);
  mov_r8_imm8(Register8.AH, BIOS.Teletype);
  mov_r8_imm8(Register8.BL, Graphics.Foreground);

  <PRINT_LOOP;
    lodsb();
    test_r8_r8(Register8.AL, Register8.AL);
    jcc_rel16(Condition.Equal, >HALT);
    int_imm8($10);
    jmp_rel16(>PRINT_LOOP);

  # --- Halt ---
  <HALT;
    jmp_rel16(>HALT);

  # --- String ---
  <STRING;
    $48; $65; $6C; $6C; $6F; $21; $20;
    $54; $68; $69; $73; $20;
    $77; $61; $73; $20;
    $62; $75; $69; $6C; $74; $20;
    $77; $69; $74; $68; $20;
    $4E; $61; $64; $69; $72; $2E;
    $0D; $0A;
    $00;

  # --- Padding and Signature ---
  until $00 510;
  $55; $AA;
}

# |================================================|
# | Procedures                                     |
# |================================================|

procedure xor_r16_r16(u8, u8) {
  $31;
  @add($C0, @add(@shl(@arg(1), 3), @arg(0)));
}

procedure mov_sreg_r16(u8, u8) {
  $8E;
  @add($C0, @add(@shl(@arg(0), 3), @arg(1)));
}

procedure mov_r16_imm16(u8, u16) {
  @add($B8, @arg(0));
  @and(@arg(1), $FF);
  @and(@shr(@arg(1), 8), $FF);
}

procedure mov_r8_imm8(u8, u8) {
  @add($B0, @arg(0));
  @arg(1);
}

procedure test_r8_r8(u8, u8) {
  $84;
  @add($C0, @add(@shl(@arg(1), 3), @arg(0)));
}

procedure int_imm8(u8) {
  $CD;
  @arg(0);
}

procedure jcc_rel16(u8, u16) {
  $0F;
  @add($80, @arg(0));
  @and(@sub(@arg(1), @add(@here(), 2)), $FF);
  @and(@shr(@sub(@arg(1), @add(@here(), 2)), 8), $FF);
}

procedure jmp_rel16(u16) {
  $E9;
  @and(@sub(@arg(0), @add(@here(), 2)), $FF);
  @and(@shr(@sub(@arg(0), @add(@here(), 2)), 8), $FF);
}

procedure lodsb() {
  $AC;
}
