cmake_minimum_required(VERSION 4.0)

# --- Python Interpreter ---
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# --- Success Testing ---
file(GLOB SUCCESS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/success/*.asm")
foreach (SOURCE_PATH IN LISTS SUCCESS_SOURCES)
    get_filename_component(SOURCE_NAME ${SOURCE_PATH} NAME_WE)

    set(OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/expect/${SOURCE_NAME}.bin")
    set(EXPECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/expect/${SOURCE_NAME}")

    add_test(
        NAME "test_success_${SOURCE_NAME}"
        COMMAND Python3::Interpreter
        ${CMAKE_CURRENT_SOURCE_DIR}/test.py
        $<TARGET_FILE:Nadir>
        "success"
        ${SOURCE_PATH}
        ${OUTPUT_PATH}
        ${EXPECT_PATH}
    )
endforeach ()

# --- Failure Testing ---
file(GLOB FAILURE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/failure/*.asm")
foreach (SOURCE_PATH IN LISTS FAILURE_SOURCES)
    get_filename_component(SOURCE_NAME ${SOURCE_PATH} NAME_WE)

    set(OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/expect/${SOURCE_NAME}.bin")

    add_test(
        NAME "test_failure_${SOURCE_NAME}"
        COMMAND Python3::Interpreter
        ${CMAKE_CURRENT_SOURCE_DIR}/test.py
        $<TARGET_FILE:Nadir>
        "failure"
        ${SOURCE_PATH}
        ${OUTPUT_PATH}
    )
endforeach ()
