procedure distance(u16, u16) {
  @sub(@arg(0), @arg(1));
}

binary $0000 {
  <PAYLOAD_START;
  $AA; $BB; $CC; $DD;
  <PAYLOAD_END;
  distance(>PAYLOAD_END, >PAYLOAD_START);
}
