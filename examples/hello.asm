# ==========================================
# Configuration
# ==========================================

constant Chip8 {
  ProgramStart = 512;
}

constant Register {
  V0 = 0; # X Coordinate
  V1 = 1; # Y Coordinate
}

# ==========================================
# Binary
# ==========================================

binary {
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

  # --- Infinite Halt Loop ---
  <LOOP;
  do_jump_to_label(>LOOP);

  # --- Font Data ---
  <LETTER_N; do_emit_bytes(144, 208, 176, 144, 144); # *..*.... | **.*.... | *.*..... | *..*.... | *..*....
  <LETTER_A; do_emit_bytes( 96, 144, 224, 144, 144); # .**..... | *..*.... | ***..... | *..*.... | *..*....
  <LETTER_D; do_emit_bytes(224, 144, 144, 144, 224); # ***..... | *..*.... | *..*.... | *..*.... | ***.....
  <LETTER_I; do_emit_bytes(224,  64,  64,  64, 224); # ***..... | .*...... | .*...... | .*...... | ***.....
  <LETTER_R; do_emit_bytes(224, 144, 224, 160, 144); # ***..... | *..*.... | ***..... | *.*..... | *..*....
}

# ==========================================
# Helpers
# ==========================================

procedure do_emit_bytes(u8, u8, u8, u8, u8) {
  @at(0); @at(1); @at(2); @at(3); @at(4);
}

procedure do_jump_to_label(u16) {
  @bit_or(16, @bit_and(@bit_shr(@add(Chip8.ProgramStart, @at(0)), 8), 15));
  @bit_and(@add(Chip8.ProgramStart, @at(0)), 255);
}

procedure do_load_index_label(u16) {
  @bit_or(160, @bit_and(@bit_shr(@add(Chip8.ProgramStart, @at(0)), 8), 15));
  @bit_and(@add(Chip8.ProgramStart, @at(0)), 255);
}

# ==========================================
# Opcodes
# ==========================================

procedure op_clear_screen() {
  0;
  224;
}

procedure op_load_register_byte(u8, u8) {
  @bit_or(96, @bit_and(@at(0), 15));
  @at(1);
}

procedure op_draw_sprite(u8, u8, u8) {
  @bit_or(208, @bit_and(@at(0), 15));
  @bit_or(@bit_shl(@bit_and(@at(1), 15), 4), @bit_and(@at(2), 15));
}
