constant Reg {
  A = 0;
  B = 1;
}

procedure load_abs(u8, u16) {
  @or($10, @and(@arg(0), $0F));
  @and(@shr(@arg(1), 8), $FF);
  @and(@arg(1), $FF);
}

procedure store_abs(u8, u16) {
  @or($20, @and(@arg(0), $0F));
  @and(@shr(@arg(1), 8), $FF);
  @and(@arg(1), $FF);
}

binary $0100 {
  load_abs(Reg.A, >DATA_SOURCE);
  store_abs(Reg.A, >DATA_DESTINATION);

  <DATA_SOURCE;      $42; # $0106
  <DATA_DESTINATION; $00; # $0107
}
