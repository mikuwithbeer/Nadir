procedure halt() {
  $FF; $FF;
}

binary 0 {
  halt();
  wrap($42);
  halt();
}

procedure wrap(u8) {
  @arg(0);
}
