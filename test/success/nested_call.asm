binary 0 {
  Depth.Level10;
  Depth.Complex;
}

constant Depth {
  Level10 = @add(1, @add(1, @add(1, @add(1, @add(1, @add(1, @add(1, @add(1, @add(1, @add(1, 1))))))))), 1);
  Complex = @or(@shl($01, @add(1, 1)), @and($FF, @sub(10, 5)));
}
