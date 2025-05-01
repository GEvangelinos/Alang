"""
alpha_regression_runner.py

Runs regression tests for the Alpha compiler by:
- Executing .asc test files (both working and error cases)
- Validating exported symbol tables and compile errors against predefined GOLDEN CSVs
- Optionally running the compiler under Valgrind to check for memory issues
- Reporting results in a concise, color-coded summary

This script is intended for use as a post-build sanity check, either manually or via CMake integration.
It uses `export_validator` as a library module for CSV comparison, ensuring consistency across test stages.

Exit codes from subprocesses are preserved to detect and report failures accurately.
"""

import argparse
import os
import shutil
import subprocess
import csv
import sys
from pathlib import Path
from _colours import *

VALGRIND_ERROR_EXITCODE = 3


def parser_startup_arguments() -> argparse.Namespace:
        parser = argparse.ArgumentParser(
                description="A tool that runs working and error tests and compares results with predefined GOLD outputs.",
                epilog="""
        alpha_regression_runner.py

        Runs regression tests for the Alpha compiler:
        - Executes .asc test files (working + error)
        - Compares symbol tables and compile errors to GOLDEN references
        - Optionally checks memory correctness via Valgrind
        - Reports results in structured, color-coded output

        Intended for use in CI pipelines or local validation during development.
        """,
                formatter_class=argparse.RawDescriptionHelpFormatter
        )
        parser.add_argument("--alpha-compiler", required=True, type=Path, help="Path to the alpha compiler.")
        parser.add_argument("--golden-symbol-tables-dir", required=True, type=Path,
                            help="Path to the directory with the GOLDEN symbol tables.")
        parser.add_argument("--golden-compile-errors-dir", required=True, type=Path,
                            help="Path to the directory with the GOLDEN errors.")
        parser.add_argument("--working-dir", type=Path, help="Path to the directory with working tests.")
        parser.add_argument("--error-dir", type=Path, help="Path to the directory with error tests.")
        parser.add_argument("--valgrind", action="store_true", help="Runs memory tests using valgrind.")
        return parser.parse_args()


def _validate_input_file(file_path: Path) -> None:
        if not os.path.isfile(file_path):
                raise FileNotFoundError(os.path.basename(file_path))
        if not os.access(file_path, os.R_OK):
                raise PermissionError(os.path.basename(file_path))


def _load_csv_file(filepath: Path) -> list[list[str]]:
        with open(filepath, newline='', encoding='utf-8') as csv_file:
                reader = csv.reader(csv_file, delimiter=',', skipinitialspace=True)
                return [row for row in reader if any(cell.strip() for cell in row)]


def export_validator(golden_file: Path, export_file: Path):
        try:
                _validate_input_file(golden_file)
                _validate_input_file(export_file)
                golden = _load_csv_file(golden_file)
                export = _load_csv_file(export_file)
                if golden == export:
                        return 0
                else:
                        return 1
        except FileNotFoundError as e:
                print(f"{COLOR_RED}FileNotFoundError: {e}{SGR_RESET} ", file=sys.stderr, end="", flush=True)
        except PermissionError as e:
                print(f"{COLOR_RED}PermissionError: {e}{SGR_RESET} ", file=sys.stderr, end="", flush=True)
        except Exception as e:
                print(f"{COLOR_RED}UnhandledException: {e}{SGR_RESET}{e} ", file=sys.stderr, end="", flush=True)
        return 2


def load_asc_filepaths(dirname: str) -> list[str]:
        asc_files = []
        for filename in os.listdir(dirname):
                filepath = os.path.join(dirname, filename)
                if filepath.endswith(".asc"):
                        asc_files.append(os.path.abspath(filepath))
        return asc_files


def validate_alpha_compiler_executable(alpha_compiler_path: Path) -> None:
        if not os.path.isfile(alpha_compiler_path):
                raise ValueError("Alpha Compiler executable file not found.")
        if not os.access(alpha_compiler_path, os.X_OK):
                raise ValueError("Alpha Compiler file is not executable.")


def validate_symbol_table(golden_symbol_table_dir: Path, asc_filepath: Path) -> int:
        golden_filepath = os.path.join(golden_symbol_table_dir, f"GOLD_{os.path.basename(asc_filepath)}.st.csv")
        export_filepath = os.path.join("SYMBOL_TABLE_EXPORTS", f"{os.path.basename(asc_filepath)}.st.csv")
        return export_validator(Path(golden_filepath), Path(export_filepath))


def validate_compile_errors(golden_compile_error_dir: Path, asc_filepath: Path) -> int:
        golden_filepath = os.path.join(golden_compile_error_dir, f"GOLD_{os.path.basename(asc_filepath)}.error.csv")
        export_filepath = os.path.join("COMPILE_ERROR_EXPORTS", f"{os.path.basename(asc_filepath)}.error.csv")
        return export_validator(Path(golden_filepath), Path(export_filepath))


def run_alpha_compiler(alpha_compiler_path: Path, asc_filepath: Path) -> int:
        ac_process_args = [
                str(alpha_compiler_path),
                "--export-symbol-table",
                "--export-compile-errors",
                "--no-show-errors",
                "--input-file", asc_filepath]
        completed_process = subprocess.run(ac_process_args)
        return completed_process.returncode


def run_valgrind_tests(alpha_compiler_path: Path, asc_filepath: Path, valgrind_error_exitcode: int) -> int:
        valgrind_args = [
                "valgrind",
                "--leak-check=full",
                "--track-origins=yes",
                "--show-leak-kinds=all",
                "--errors-for-leak-kinds=all",
                f"--error-exitcode={valgrind_error_exitcode}",
                "--quiet"]
        ac_args = [
                str(alpha_compiler_path),
                "--export-symbol-table",
                "--export-compile-errors",
                "--show-symbol-table",
                "--show-parser-trace",
                "--input-file",
                asc_filepath]

        completed_process = subprocess.run(
                valgrind_args + ac_args,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL)

        return completed_process.returncode


def pretty_print(message: str, return_code: int):
        if return_code == 0:
                print(f"{message}{COLOR_GREEN}{"PASS":<6}{SGR_RESET} ", end="", flush=True)
        elif return_code == 1:
                print(f"{message}{COLOR_RED}{"FAIL":<6}{SGR_RESET} ", end="", flush=True)
        else:
                pass
                # print(f"{message}{COLOR_RED}{"ERROR":<10}{SGR_RESET}", end="", flush=True)


def run_test_files(args, asc_filepaths):
        asc_filepaths.sort()
        total_tests = len(asc_filepaths)
        for test_index, asc_filepath in enumerate(asc_filepaths, 1):
                print()
                print()
                ## TODO DO A PROGRSS  bar with  =====
                ## TODO DO A PROGRSS  bar with  =====
                ## TODO DO A PROGRSS  bar with  =====
                ## TODO DO A PROGRSS  bar with  =====
                ## TODO DO A PROGRSS  bar with  =====
                ## TODO DO A PROGRSS  bar with  =====
                ## TODO DO A PROGRSS  bar with  =====
                ## TODO use a for loop to make the progress bar based on percentage !
                ## TODO use a for loop to make the progress bar based on percentage !
                ## TODO use a for loop to make the progress bar based on percentage !
                if test_index != total_tests:
                        print(f"Running test... {test_index}/{total_tests}", end='', flush=True)
                        print("\033[2F\r")
                else:
                        print(f"Running test... {test_index}/{total_tests}")  # final print with newline
                print(f"\033[2K-- Testing {os.path.basename(asc_filepath):<30} ", end="")
                ac_retval = run_alpha_compiler(Path(args.alpha_compiler).resolve(), asc_filepath)
                pretty_print(f"AC_EXECUTION:", ac_retval)
                if ac_retval != 0:
                        print()
                        continue
                sym_retval = validate_symbol_table(args.golden_symbol_tables_dir, asc_filepath)
                pretty_print(f"EXPECTED_SYMTABLE:", sym_retval)
                err_retval = validate_compile_errors(args.golden_compile_errors_dir, asc_filepath)
                pretty_print(f"EXPECTED_CT_ERRORS:", err_retval)

                if args.valgrind:
                        val_retval = run_valgrind_tests(Path(args.alpha_compiler).resolve(), asc_filepath,
                                                        VALGRIND_ERROR_EXITCODE)
                        pretty_print(f"VALGRIND:", val_retval)



def delete_export_dir(export_dir: Path) -> None:
        if export_dir.exists() and export_dir.is_dir():
                shutil.rmtree(export_dir)


def cleanup():
        delete_export_dir(Path(os.path.join(os.getcwd(), "SYMBOL_TABLE_EXPORTS")))
        delete_export_dir(Path(os.path.join(os.getcwd(), "COMPILE_ERROR_EXPORTS")))


def main():
        args = parser_startup_arguments()
        validate_alpha_compiler_executable(args.alpha_compiler)
        working_asc_files = load_asc_filepaths(args.working_dir)
        error_asc_files = load_asc_filepaths(args.error_dir)
        run_test_files(args, working_asc_files + error_asc_files)


if __name__ == '__main__':
        main()
