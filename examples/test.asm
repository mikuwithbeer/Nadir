binary {
  move(Register.A, 10);
  move(Register.B, 20);
  <INFINITE;
  noop();
  jmp(>INFINITE);
  # move(Register.F, @add(@at(0), 1));
}

constant Register {
  Secret = 254;
}

procedure noop() {
  0;
  Register.Secret;
}

constant Register {
  A = 1;
  B = @add(Register.A, 1);
  C = @add(Register.B, 1);
  D = @add(Register.C, 1);
  E = @add(Register.D, 1);
  F = @add(Register.E, 1);
}

procedure move(u8, u8) {
  67;
  @at(0);
  @at(1);
}

procedure jmp(u16) {
  100;
  @bit_and(@at(0), 255);
  @bit_and(@bit_shr(@at(0), 8), 255);
}
