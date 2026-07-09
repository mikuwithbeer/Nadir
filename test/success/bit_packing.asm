constant Test {
  Red   = 5; # 101
  Green = 3; # 011
  Blue  = 2; # 10

  # $AE (1010 1110)
  Data = @insert(@insert(@insert(0, Test.Blue, 0, 2), Test.Green, 2, 3), Test.Red, 5, 3);
}

binary 0 {
  Test.Data; # $AE

  @mask(4); # $0F (0000 1111) - Low nibble mask
  @mask(7); # $7F (0111 1111) - Standard ASCII mask
  @mask(8); # $FF (1111 1111) - Full byte mask

  @extract(Test.Data, 5, 3); # $05
  @extract(Test.Data, 2, 3); # $03
  @extract(Test.Data, 0, 2); # $02
}
