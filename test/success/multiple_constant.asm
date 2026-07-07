# This file is also uglified to test the lexer and parser.
# It should still compile successfully.

constant Register     {
  A=-0
  ;}


constant Register{
  B = @add(Register.A, 1);
}

binary   $200
{
  set_register(Register .
    A, 1_0_0_0);
  set_register  (Register.B,+2______000) ;
}

procedure set_register(u8 ,  u16
 ){
  $72;@arg(0);
  @and(@shr(@bswap(@arg(1
  ),16), 8), $FF);
  @cast(@bswap(@arg(  1), 16),
 u8)#;}
;
}
