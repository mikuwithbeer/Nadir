binary 0 {
  1;
  <TEST;
  noop(>TEST);
  Example.U8_MAX;
  @add(1, 2, 3, 4);
  >LOL # This should fail
}

procedure noop(u8) { 0; @arg(0); }

constant Example { U8_MAX = $+FF; }
