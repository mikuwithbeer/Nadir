constant Limit {
  Minimum = 1_0;
  Maximum = 100;
  CanPass = $42;
}

procedure checker(u8) {
  # if (arg > Minimum AND arg < Maximum) OR (arg == CanPass) arg
  # else 255
  @if(
    @lor(
      @land(@gt(@arg(0), Limit.Minimum), @lt(@arg(0), Limit.Maximum)),
      @eq(@arg(0), Limit.CanPass)
    ),
    @arg(0),
    $FF
  );
}

binary $8080 {
  checker(5);
  checker(50);
  checker(150);
  checker($42);
}