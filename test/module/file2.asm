constant Example {
  Value = 1;
  Other = $FF;
}

procedure yayyest() {
  @sub(@add(Example.Value, Example.Other), 10);
}
