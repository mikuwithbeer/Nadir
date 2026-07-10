constant UB {
  This = @here();
}

binary $10 {
  UB.This; # $0
  @here(); # $11
  $12; $1;
  fun($A);
}

procedure fun(u8) {
  @here();
  @arg(0);
  @here();
}
