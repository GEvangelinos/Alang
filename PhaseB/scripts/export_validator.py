import argparse
import os
from typing import NoReturn
import csv
import sys

COLOR_BLACK = "\033[90m"
COLOR_RED = "\033[91m"
COLOR_GREEN = "\033[92m"
COLOR_YELLOW = "\033[93m"
COLOR_BLUE = "\033[94m"
COLOR_MAGENTA = "\033[95m"
COLOR_CYAN = "\033[96m"
COLOR_WHITE = "\033[97m"

STYLE_BOLD = "\033[1m"
STYLE_UNDERLINE = "\033[4m"

SGR_RESET = "\033[0m"


def _validate_input_file(file_path: str) -> None:
        if not os.path.isfile(file_path):
                raise FileNotFoundError(f"Input file {os.path.basename(file_path)} is not a file")
        if not os.access(file_path, os.R_OK):
                raise PermissionError(f"Input file {os.path.basename(file_path)} is not readable")


def _parser_startup_arguments() -> argparse.Namespace:
        parser = argparse.ArgumentParser(
                description="Compare an exported symbol table against a golden reference to detect mismatches.")
        parser.add_argument(
                "--golden-file",
                required=True,
                help="Path to the golden (reference) symbol table file (CSV format).")
        parser.add_argument(
                "--export-file",
                required=True,
                help="Path to the exported symbol table file (CSV format).")
        return parser.parse_args()


def _load_csv_file(filename: str) -> list[list[str]]:
        with open(filename, newline='', encoding='utf-8') as csv_file:
                reader = csv.reader(csv_file, delimiter=',', skipinitialspace=True)
                return list(reader)


def main():
        try:
                args = _parser_startup_arguments()
                _validate_input_file(args.golden_file)
                _validate_input_file(args.export_file)
                golden = _load_csv_file(args.golden_file)
                export = _load_csv_file(args.export_file)
                
                basename = os.path.basename(args.golden_file)

                print(f"Running test '{basename}': ", end="")
                if (golden == export):
                        print(f"{COLOR_GREEN}TEST_PASSED{SGR_RESET}", flush=True)
                else:
                        print(f"{COLOR_RED}TEST_FAILED{SGR_RESET}", flush=True)
        except FileNotFoundError as e:
                print(f"{COLOR_RED}FileNotFoundError: {SGR_RESET}{e}", file=sys.stderr)
        except Exception as e:
                print(f"{COLOR_RED}Exception: {SGR_RESET}{e}", file=sys.stderr)

        return 0



if __name__ == "__main__":
        main()
