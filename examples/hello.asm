# A CHIP-8 program that:
#
# * Clears the display
# * Draws the word "NADIR" using custom 5-byte sprites
# * Enters an infinite loop to keep the image visible
#
# This file also serves as a minimal example :)

# |================================================|
# | Constants                                      |
# |================================================|

constant Register {
  V0 = 0; # X Coordinate
  V1 = 1; # Y Coordinate
}

# |================================================|
# | Program                                        |
# |================================================|

binary $200 {
  op_clear_screen();
  op_load_register_byte(Register.V1, 12);

  # --- Draw 'N' ---
  op_load_register_byte(Register.V0, 6);
  do_load_index_label(>LETTER_N);
  op_draw_sprite(Register.V0, Register.V1, 5);

  # --- Draw 'A' ---
  op_load_register_byte(Register.V0, 12);
  do_load_index_label(>LETTER_A);
  op_draw_sprite(Register.V0, Register.V1, 5);

  # --- Draw 'D' ---
  op_load_register_byte(Register.V0, 18);
  do_load_index_label(>LETTER_D);
  op_draw_sprite(Register.V0, Register.V1, 5);

  # --- Draw 'I' ---
  op_load_register_byte(Register.V0, 24);
  do_load_index_label(>LETTER_I);
  op_draw_sprite(Register.V0, Register.V1, 5);

  # --- Draw 'R' ---
  op_load_register_byte(Register.V0, 30);
  do_load_index_label(>LETTER_R);
  op_draw_sprite(Register.V0, Register.V1, 5);

  # --- Infinite Loop ---
  <LOOP;
  do_jump_to_label(>LOOP);

  # --- Font Data ---
  <LETTER_N; do_emit_bytes($90, $D0, $B0, $90, $90);
  <LETTER_A; do_emit_bytes($60, $90, $E0, $90, $90);
  <LETTER_D; do_emit_bytes($E0, $90, $90, $90, $E0);
  <LETTER_I; do_emit_bytes($E0, $40, $40, $40, $E0);
  <LETTER_R; do_emit_bytes($E0, $90, $E0, $A0, $90);
}

# |================================================|
# | Helpers                                        |
# |================================================|

procedure do_emit_bytes(u8, u8, u8, u8, u8) {
  @at(0); @at(1); @at(2); @at(3); @at(4);
}

procedure do_jump_to_label(u16) {
  @bit_or($10, @bit_and(@bit_shr(@at(0), 8), $0F));
  @bit_and(@at(0), $FF);
}

procedure do_load_index_label(u16) {
  @bit_or($A0, @bit_and(@bit_shr(@at(0), 8), $0F));
  @bit_and(@at(0), $FF);
}

# |================================================|
# | Opcodes                                        |
# |================================================|

procedure op_clear_screen() {
  $00; $E0;
}

procedure op_load_register_byte(u8, u8) {
  @bit_or($60, @bit_and(@at(0), $0F));
  @at(1);
}

procedure op_draw_sprite(u8, u8, u8) {
  @bit_or($D0, @bit_and(@at(0), $0F));
  @bit_or(@bit_shl(@bit_and(@at(1), $0F), 4), @bit_and(@at(2), $0F));
}
