constant Magic {
  Metal  = $DCAC;
  Header = $DEADBEEF;
}

procedure emit_u16_le(u16) {
  @cast(@arg(0), u8);
  @cast(@shr(@arg(0), 8), u8);
}

procedure emit_u32_be(u32) {
  @cast(@shr(@arg(0), 24), u8);
  @cast(@shr(@arg(0), 16), u8);
  @cast(@shr(@arg(0), 8), u8);
  @cast(@arg(0), u8);
}

binary $1000 {
  emit_u16_le(Magic.Metal);
  emit_u32_be(Magic.Header);
}
