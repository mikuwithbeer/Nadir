constant FNV {
  Offset = $811C9DC5; # 2166136261 (FNV 32-bit offset basis)
  Prime  = $01000193; # 16777619   (FNV 32-bit prime)
}

constant DJB2 {
  Seed = 5381;
  Mult = 33;
}

# Calculates a 32-bit FNV-1a hash for 2 characters and emits as Big-Endian.
procedure emit_fnv1a_2chars_be(u8, u8) {
  @cast(@shr(@mul(@xor(@mul(@xor(FNV.Offset, @arg(0)), FNV.Prime), @arg(1)), FNV.Prime), 24), u8);
  @cast(@shr(@mul(@xor(@mul(@xor(FNV.Offset, @arg(0)), FNV.Prime), @arg(1)), FNV.Prime), 16), u8);
  @cast(@shr(@mul(@xor(@mul(@xor(FNV.Offset, @arg(0)), FNV.Prime), @arg(1)), FNV.Prime), 8),  u8);
  @cast(@mul(@xor(@mul(@xor(FNV.Offset, @arg(0)), FNV.Prime), @arg(1)), FNV.Prime),           u8);
}

# Calculates a 32-bit DJB2 hash for 3 characters and emits as Little-Endian.
procedure emit_djb2_3chars_le(u8, u8, u8) {
  @cast(@add(@mul(@add(@mul(@add(@mul(DJB2.Seed, DJB2.Mult), @arg(0)), DJB2.Mult), @arg(1)), DJB2.Mult), @arg(2)),           u8);
  @cast(@shr(@add(@mul(@add(@mul(@add(@mul(DJB2.Seed, DJB2.Mult), @arg(0)), DJB2.Mult), @arg(1)), DJB2.Mult), @arg(2)), 8),  u8);
  @cast(@shr(@add(@mul(@add(@mul(@add(@mul(DJB2.Seed, DJB2.Mult), @arg(0)), DJB2.Mult), @arg(1)), DJB2.Mult), @arg(2)), 16), u8);
  @cast(@shr(@add(@mul(@add(@mul(@add(@mul(DJB2.Seed, DJB2.Mult), @arg(0)), DJB2.Mult), @arg(1)), DJB2.Mult), @arg(2)), 24), u8);
}

binary 0 {
  # Hash the string "HI" using FNV-1a
  emit_fnv1a_2chars_be($48, $49);

  # Hash the string "CAT" using DJB2
  emit_djb2_3chars_le($43, $41, $54);
}
