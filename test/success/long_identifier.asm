constant Reg {
  V = $01;
  V_ = $02;
  V__ = $03;
  VERY_LONG_IDENTIFIER_THAT_STRETCHES_THE_BUFFER_LIMIT_JUST = $14;
}

procedure ThatsAnother__________LongProcedureName(u8, u8, u8, u8) {
  @arg(0);
  @arg(1);
  @arg(2);
  @arg(3);
}

binary 0 {
  ThatsAnother__________LongProcedureName(
    Reg.V,
    Reg.V_,
    Reg.V__,
    Reg.VERY_LONG_IDENTIFIER_THAT_STRETCHES_THE_BUFFER_LIMIT_JUST
  );
}
