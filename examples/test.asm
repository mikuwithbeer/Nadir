constant Wow {
  A = $+FF;
  B = $-AbcDef;
  C = 10;
  D = -20;
}

procedure do(i32) {
    @cast(@abs(@at(0)), u8);
}

binary {
  do(Wow.A);
  do(Wow.B);
  do(Wow.C);
  do(Wow.D);
}