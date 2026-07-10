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
  Wait = $86;
}

constant Graphics {
  VGA = $13;
}

constant Register8 {
  AL = 0;
  BL = 3;
  AH = 4;
}

constant Register16 {
  AX = 0;
  CX = 1;
  DX = 2;
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

  # --- Initialize Color ---
  mov_r8_imm8(Register8.BL, $01);

  # --- Main Loop ---
  <MAIN_LOOP;
    mov_r16_imm16(Register16.SI, >STRING);

    <PRINT_LOOP;
      lodsb();
      test_r8_r8(Register8.AL, Register8.AL);
      jcc_rel16(Condition.Equal, >WAIT_STATE);

      mov_r8_imm8(Register8.AH, BIOS.Teletype);
      int_imm8($10);
      jmp_rel16(>PRINT_LOOP);

    <WAIT_STATE;
      mov_r8_imm8(Register8.AH, BIOS.Wait);
      mov_r16_imm16(Register16.CX, $000F);
      mov_r16_imm16(Register16.DX, $4240);
      int_imm8($15);
      
      inc_r8(Register8.BL);
      jmp_rel16(>MAIN_LOOP);

  # --- String ---
  <STRING; $48; $65; $6C; $6C; $6F; $21; $20; $54;
           $68; $69; $73; $20; $77; $61; $73; $20;
           $62; $75; $69; $6C; $74; $20; $77; $69;
           $74; $68; $20; $4E; $61; $64; $69; $72;
           $2E; $0D; $0A; $00;

  # --- Padding and Signature ---
  until $00 510;
  $55; $AA;
}

# |================================================|
# | Procedures                                     |
# |================================================|

procedure xor_r16_r16(u8, u8) {
  $31;
  @insert(@insert($C0, @arg(1), 3, 3), @arg(0), 0, 3);
}

procedure mov_sreg_r16(u8, u8) {
  $8E;
  @insert(@insert($C0, @arg(0), 3, 3), @arg(1), 0, 3);
}

procedure mov_r16_imm16(u8, u16) {
  @add($B8, @arg(0));
  @extract(@arg(1), 0, 8);
  @extract(@arg(1), 8, 8);
}

procedure mov_r8_imm8(u8, u8) {
  @add($B0, @arg(0));
  @arg(1);
}

procedure test_r8_r8(u8, u8) {
  $84;
  @insert(@insert($C0, @arg(1), 3, 3), @arg(0), 0, 3);
}

procedure inc_r8(u8) {
  $FE;
  @insert($C0, @arg(0), 0, 3);
}

procedure int_imm8(u8) {
  $CD;
  @arg(0);
}

procedure jcc_rel16(u8, u16) {
  $0F;
  @add($80, @arg(0));
  @extract(@sub(@arg(1), @add(@here(), 2)), 0, 8);
  @extract(@sub(@arg(1), @add(@here(), 2)), 8, 8);
}

procedure jmp_rel16(u16) {
  $E9;
  @extract(@sub(@arg(0), @add(@here(), 2)), 0, 8);
  @extract(@sub(@arg(0), @add(@here(), 2)), 8, 8);
}

procedure lodsb() {
  $AC;
}
