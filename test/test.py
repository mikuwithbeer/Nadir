import os
import subprocess
import sys


def main():
    if len(sys.argv) < 5:
        print("Invalid number of arguments")
        sys.exit(1)

    assembler_path = sys.argv[1]

    test_mode = sys.argv[2].lower()

    source_file = sys.argv[3]
    output_file = sys.argv[4]
    expect_file = sys.argv[5] if len(sys.argv) > 5 else None

    print(f"Testing [mode:{test_mode}]: {source_file}")

    try:
        process_result = subprocess.run(
            [assembler_path, "-i", source_file, "-o", output_file],
            capture_output=True,
            text=True,
        )
    except Exception as exception:
        print(f"Failed to execute assembler: {exception}")
        sys.exit(1)

    if test_mode == "success":
        if process_result.returncode != 0:
            print(f"Assembler failed to compile: {source_file}")
            print(f"  Code: {process_result.returncode}")
            print(f"  Message: {process_result.stderr}")
            sys.exit(1)

        if not os.path.exists(output_file):
            print(f"Output file not found: {output_file}")
            sys.exit(1)

        if not expect_file or not os.path.exists(expect_file):
            print(f"Expected file not found: {expect_file}")
            sys.exit(1)

        with (
            open(output_file, "rb") as file_left,
            open(expect_file, "rb") as file_right,
        ):
            left = file_left.read()
            right = file_right.read()

            if left == right:
                print("Output file matches expected file")
                sys.exit(0)
            else:
                print("Output file does not match expected file:")
                print("  Left: ", left)
                print("  Right: ", right)
                sys.exit(1)
    elif test_mode == "failure":
        if process_result.returncode == 0:
            print("Assembler succeeded unexpectedly")
            sys.exit(1)
        else:
            print("Assembler failed as expected:")
            print(f"  Code: {process_result.returncode}")
            print(f"  Message: {process_result.stderr}")
            sys.exit(0)
    else:
        print(f"Invalid test mode: {test_mode}")
        sys.exit(1)


if __name__ == "__main__":
    main()
