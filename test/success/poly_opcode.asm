constant Type {
  Register = 1;
  Memory   = 2;
}

procedure smart_load(u8, u8) {
  @if(@eq(@arg(0), Type.Register),
    @or($10, @and(@arg(1), $0F)),
    @or($20, @and(@arg(1), $0F))
  );
}

binary $+0 {
  smart_load(Type.Register, 5);
  smart_load(Type.Memory, 5);
}
