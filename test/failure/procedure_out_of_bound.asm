procedure unsafe_access(u8) {
  @arg(0);
  @arg(1);
}

binary 0 {
  unsafe_access($FF);
}
