# |=================================================================|
# | Hello there! Welcome to a beautifully simple WASM source.       |
# |                                                                 |
# | This source code defines four functions: add, sub, mul, div.    |
# | You can compile and use these functions anywhere you want!      |
# |                                                                 |
# | This file is meant to be a minimal example. Please feel free to |
# | learn from it, tinker with it, and make it your own! :)         |
# |=================================================================|

# |================================================|
# | Constants                                      |
# |================================================|

constant WASM {
  Type = $01;
  Function = $03;
  Export = $07;
  Code = $0A;
}

constant Instruction {
  LocalGet = $20;
  I32Add = $6A;
  I32Sub = $6B;
  I32Mul = $6C;
  I32Div = $6D;
  End = $0B;
}

# |================================================|
# | Entry Point                                    |
# |================================================|

binary 0 {
  # --- WASM Header ---
  write_wasm_header();

  # --- Type Section ---
  write_section_header(WASM.Type, >TYPE_SECTION_BEGIN, >TYPE_SECTION_END);
  <TYPE_SECTION_BEGIN;
    write_vector_length(1);
    write_function_type_i32_i32_to_i32();
  <TYPE_SECTION_END;

  # --- Function Section ---
  write_section_header(WASM.Function, >FUNCTION_SECTION_BEGIN, >FUNCTION_SECTION_END);
  <FUNCTION_SECTION_BEGIN;
    write_vector_length(4);
    write_type_index(0);
    write_type_index(0);
    write_type_index(0);
    write_type_index(0);
  <FUNCTION_SECTION_END;

  # --- Export Section ---
  write_section_header(WASM.Export, >EXPORT_SECTION_BEGIN, >EXPORT_SECTION_END);
  <EXPORT_SECTION_BEGIN;
    write_vector_length(4);
    write_function_export_three_character_name($61, $64, $64, 0);
    write_function_export_three_character_name($73, $75, $62, 1);
    write_function_export_three_character_name($6D, $75, $6C, 2);
    write_function_export_three_character_name($64, $69, $76, 3);
  <EXPORT_SECTION_END;

  # --- Code Section ---
  write_section_header(WASM.Code, >CODE_SECTION_BEGIN, >CODE_SECTION_END);
  <CODE_SECTION_BEGIN;
    write_vector_length(4);

    # --- Function Add ---
    <FUNCTION_ADD_START;
      write_function_body_size_and_local_count(>FUNCTION_ADD_START, >FUNCTION_ADD_END, 0);
      write_local_get_instruction(0);
      write_local_get_instruction(1);
      write_i32_add_instruction();
      write_end_instruction();
    <FUNCTION_ADD_END;

    # --- Function Sub ---
    <FUNCTION_SUB_START;
      write_function_body_size_and_local_count(>FUNCTION_SUB_START, >FUNCTION_SUB_END, 0);
      write_local_get_instruction(0);
      write_local_get_instruction(1);
      write_i32_sub_instruction();
      write_end_instruction();
    <FUNCTION_SUB_END;

    # --- Function Mul ---
    <FUNCTION_MUL_START;
      write_function_body_size_and_local_count(>FUNCTION_MUL_START, >FUNCTION_MUL_END, 0);
      write_local_get_instruction(0);
      write_local_get_instruction(1);
      write_i32_mul_instruction();
      write_end_instruction();
    <FUNCTION_MUL_END;

    # --- Function Div ---
    <FUNCTION_DIV_START;
      write_function_body_size_and_local_count(>FUNCTION_DIV_START, >FUNCTION_DIV_END, 0);
      write_local_get_instruction(0);
      write_local_get_instruction(1);
      write_i32_div_instruction();
      write_end_instruction();
    <FUNCTION_DIV_END;
  <CODE_SECTION_END;
}

# |================================================|
# | Procedures                                     |
# |================================================|

procedure write_wasm_header() {
  $00; $61; $73; $6D; # \0asm
  $01; $00; $00; $00; # Version 1
}

procedure write_section_header(u8, u16, u16) {
  @arg(0);
  @sub(@arg(2), @arg(1));
}

procedure write_vector_length(u8) {
  @assert(@lt(@arg(0), 128), @arg(0));
}

procedure write_function_type_i32_i32_to_i32() {
  $60;      # Function
  $02;      # Parameter Count
  $7F; $7F; # i32, i32
  $01;      # Result Count
  $7F;      # i32
}

procedure write_type_index(u8) {
  @assert(@lt(@arg(0), 128), @arg(0));
}

procedure write_function_export_three_character_name(u8, u8, u8, u8) {
  $03;
  @arg(0); @arg(1); @arg(2);
  $00;
  @assert(@lt(@arg(3), 128), @arg(3));
}

procedure write_function_body_size_and_local_count(u16, u16, u8) {
  @sub(@sub(@arg(1), @arg(0)), 1);
  @assert(@lt(@arg(2), 128), @arg(2));
}

procedure write_local_get_instruction(u8) {
  Instruction.LocalGet;
  @assert(@lt(@arg(0), 128), @arg(0));
}

procedure write_i32_add_instruction() { Instruction.I32Add; }
procedure write_i32_sub_instruction() { Instruction.I32Sub; }
procedure write_i32_mul_instruction() { Instruction.I32Mul; }
procedure write_i32_div_instruction() { Instruction.I32Div; }

procedure write_end_instruction() { Instruction.End; }
