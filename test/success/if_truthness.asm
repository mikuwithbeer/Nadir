binary 0 {
  @if(1, $AA, $00);
  @if($FF, $BB, $00);
  @if(@sub(0, 1), $CC, $00);
  @if(0, $00, $DD);
  @if(@sub($55, $55), $00, $EE);
}
