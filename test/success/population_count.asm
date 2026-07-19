constant TestValues {
  PopZero  = @popcnt($00);
  PopFour  = @popcnt($F0);
  PopSixteen = @popcnt($FFFF);
}

binary 0 {
  @assert(@eq(TestValues.PopZero, 0), $00);
  @assert(@eq(TestValues.PopFour, 4), $01);
  @assert(@eq(TestValues.PopSixteen, 16), $02);
}
