procedure pack_332(u8, u8, u8) {
  @or(
    @shl(@and(@arg(0), 7), 5), # mask 3 bits, shift to top
    @shl(@and(@arg(1), 7), 2), # mask 3 bits, shift to middle
    @and(@arg(2), 3)           # mask 2 bits, leave at bottom
  );
}

binary $0000 {
  # binary 101
  # binary 011
  # binary 10
  # 1010 1110 = $AE
  pack_332(5, 3, 2);
}
