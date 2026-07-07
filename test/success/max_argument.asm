procedure holy(u8, u8, u8, u8, u8, u8, u8, u8, u8, u8, u8, u8, u8, u8, u8, u8) {
  @arg(0); @arg(1); @arg(2); @arg(3); @arg(4); @arg(5); @arg(6); @arg(7);
  @arg(8); @arg(9); @arg(10); @arg(11); @arg(12); @arg(13); @arg(14); @arg(15);
}

binary 0 {
  holy(
    $00, $11, $22, $33, $44, $55, $66, $77,
    $88, $99, $AA, $BB, $CC, $DD, $EE, $FF
  );
}
