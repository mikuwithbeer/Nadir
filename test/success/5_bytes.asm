procedure increment(u8) { @add(@arg(0), 1); }

binary $0 {
  1;
  <TEST;
  increment(>TEST);
  Data.Three;
  @sub(@mul(Data.Three, 2), Data.Two);
  $5;
}
constant Data {
  Two = 2;
  Three = $03;
}
