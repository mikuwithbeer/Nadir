# |=================================================================|
# | Hello there! Welcome to a beautifully simple CHIP-8 program.    |
# |                                                                 |
# | This program does:                                              |
# | 1. Clear the display                                            |
# | 2. Draw the word "NADIR"                                        |
# | 3. Enter an infinite loop                                       |
# |                                                                 |
# | This file is meant to be a minimal example. Please feel free to |
# | learn from it, tinker with it, and make it your own! :)         |
# |=================================================================|

# |================================================|
# | Constants                                      |
# |================================================|

constant Register {
  V0 = 0; # X Coordinate
  V1 = 1; # Y Coordinate
}

constant Display {
  SpriteHeight = 5;
}

constant Display {
  PositionXN = 6;
  PositionYN = Display.PositionXN;

  PositionXA = @add(Display.PositionXN, 6);
  PositionYA = @mul(Display.PositionYN, 2);

  PositionXD = @add(Display.PositionXA, 6);
  PositionYD = @mul(Display.PositionYN, 3);

  PositionXI = @add(Display.PositionXD, 6);
  PositionYI = @sub(Display.PositionYD, 6);

  PositionXR = @add(Display.PositionXI, 6);
  PositionYR = @div(Display.PositionYD, 3);
}

# |================================================|
# | Entry Point                                    |
# |================================================|

binary $200 {
  clear_display();

  # --- Draw 'N' ---
  set_register(Register.V0, Display.PositionXN);
  set_register(Register.V1, Display.PositionYN);
  set_index(>SPRITE_N);
  draw(Register.V0, Register.V1, Display.SpriteHeight);

  # --- Draw 'A' ---
  set_register(Register.V0, Display.PositionXA);
  set_register(Register.V1, Display.PositionYA);
  set_index(>SPRITE_A);
  draw(Register.V0, Register.V1, Display.SpriteHeight);

  # --- Draw 'D' ---
  set_register(Register.V0, Display.PositionXD);
  set_register(Register.V1, Display.PositionYD);
  set_index(>SPRITE_D);
  draw(Register.V0, Register.V1, Display.SpriteHeight);

  # --- Draw 'I' ---
  set_register(Register.V0, Display.PositionXI);
  set_register(Register.V1, Display.PositionYI);
  set_index(>SPRITE_I);
  draw(Register.V0, Register.V1, Display.SpriteHeight);

  # --- Draw 'R' ---
  set_register(Register.V0, Display.PositionXR);
  set_register(Register.V1, Display.PositionYR);
  set_index(>SPRITE_R);
  draw(Register.V0, Register.V1, Display.SpriteHeight);

  # --- Infinite Loop ---
  <LOOP;
  jump(>LOOP);

  # --- Sprite Data ---
  <SPRITE_N; _emit_bytes($90, $D0, $B0, $90, $90);
  <SPRITE_A; _emit_bytes($60, $90, $E0, $90, $90);
  <SPRITE_D; _emit_bytes($E0, $90, $90, $90, $E0);
  <SPRITE_I; _emit_bytes($E0, $40, $40, $40, $E0);
  <SPRITE_R; _emit_bytes($E0, $90, $E0, $A0, $90);
}

# |================================================|
# | Helpers                                        |
# |================================================|

procedure _emit_bytes(u8, u8, u8, u8, u8) {
  @arg(0); @arg(1); @arg(2); @arg(3); @arg(4);
}

# |================================================|
# | Opcodes                                        |
# |================================================|

# 00E0: Clear the display
procedure clear_display() {
  $00; $E0;
}

# 1NNN: Jump to address NNN
procedure jump(u16) {
  @or($10, @and(@shr(@arg(0), 8), $0F));
  @and(@arg(0), $FF);
}

# 6XNN: Set register VX to value NN
procedure set_register(u8, u8) {
  @or($60, @and(@arg(0), $0F));
  @arg(1);
}

# ANNN: Set index register I to address NNN
procedure set_index(u16) {
  @or($A0, @and(@shr(@arg(0), 8), $0F));
  @and(@arg(0), $FF);
}

# DXYN: Draw sprite at VX, VY with height N
procedure draw(u8, u8, u8) {
  @or($D0, @and(@arg(0), $0F));
  @or(@shl(@and(@arg(1), $0F), 4), @and(@arg(2), $0F));
}
