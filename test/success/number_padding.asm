constant Padding {
  Hex = $+00000000000000000001;
  Dec = -00000000000000000001;
}

binary 0 {
  @cast(Padding.Hex, u8);
  @cast(Padding.Dec, u8);
}
