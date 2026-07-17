import argparse
import filecmp
import os
import subprocess
import sys


def parse_arguments():
    parser = argparse.ArgumentParser(description="Test runner for the assembler")

    parser.add_argument(
        "assembler_path",
        help="Path to the assembler executable",
    )

    parser.add_argument(
        "test_mode",
        choices=["success", "failure"],
        help="Expected test outcome",
    )

    parser.add_argument(
        "source_file",
        help="Input assembly source file",
    )

    parser.add_argument(
        "output_file",
        help="Path for the generated output file",
    )

    parser.add_argument(
        "expect_file",
        nargs="?",
        default=None,
        help="Path to the expected binary output file",
    )

    return parser.parse_args()


def handle_success_mode(args, process_result):
    if process_result.returncode != 0:
        print(f"Assembler failed to compile: {args.source_file}")
        print(f"  Code: {process_result.returncode}")
        print(f"  Message: {process_result.stderr}")
        return 1

    if not os.path.exists(args.output_file):
        print(f"Output file not found: {args.output_file}")
        return 1

    if not args.expect_file or not os.path.exists(args.expect_file):
        print(f"Expected file not found or not provided: {args.expect_file}")
        return 1

    if filecmp.cmp(args.output_file, args.expect_file, shallow=False):
        print("Output file matches expected file")
        return 0
    else:
        print(f"Output ({args.output_file}) does not match ({args.expect_file})")
        return 1


def handle_failure_mode(process_result):
    if process_result.returncode == 0:
        print("Assembler succeeded unexpectedly")
        return 1

    print("Assembler failed as expected:")
    print(f"  Code: {process_result.returncode}")
    print(f"  Message: {process_result.stderr}")
    return 0


def main():
    args = parse_arguments()
    print(f"Testing [mode:{args.test_mode}]: {args.source_file}")

    try:
        process_result = subprocess.run(
            [args.assembler_path, "-i", args.source_file, "-o", args.output_file],
            capture_output=True,
        )
    except FileNotFoundError:
        print(f"Executable not found: {args.assembler_path}")
        return 1
    except Exception as exception:
        print(f"Failed to execute assembler: {exception}")
        return 1

    if args.test_mode == "success":
        return handle_success_mode(args, process_result)
    else:
        return handle_failure_mode(process_result)


if __name__ == "__main__":
    sys.exit(main())
