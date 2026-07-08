binary 0 {
  @assert(1, 01);
  # x(9);
  x($AA);
}

procedure x(u8) {
  @assert(@gt(@arg(0), 10), @arg(0));
}
