constant NotBroken {
  Value = @add(1, @sub(5, @mul(2, 3)));
}

binary 0 {
  NotBroken.Value;
}}
