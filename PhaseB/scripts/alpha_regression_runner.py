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
import threading
import os
import shutil
import subprocess
import csv
import sys
from pathlib import Path
from _colours import *
from concurrent.futures import ThreadPoolExecutor

# ───────────────────────── constants & helpers ─────────────────────────
SYMBOL_TABLES_DIR = "SYMBOL_TABLE_EXPORTS"
COMPILE_ERRORS_DIR = "COMPILE_ERROR_EXPORTS"
GOLD_FILES_PREFIX = "GOLD_"
SYMTABLE_CSV = ".st.csv"
ERRORS_CSV = ".error.csv"
ASC_EXT = ".asc"
VALGRIND_ERROR_EXITCODE = 3
BAR_WIDTH = 40
print_lock = threading.Lock()


def parser_startup_arguments() -> argparse.Namespace:
    """
    Parse and validate command-line arguments.
    """
    parser = argparse.ArgumentParser(
        description="Run Alpha-compiler regression tests with optional Valgrind.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=(
            "Runs .asc test files (working + error) → compares produced exports "
            "against GOLDEN references and optionally runs Valgrind."
        ),
    )
    parser.add_argument("--alpha-compiler", required=True, type=Path,
                        help="Path to alpha_driver.out (executable).")
    parser.add_argument("--golden-symbol-tables-dir", required=True, type=Path,
                        help="Directory containing GOLD_* .st.csv files.")
    parser.add_argument("--golden-compile-errors-dir", required=True, type=Path,
                        help="Directory containing GOLD_* .error.csv files.")
    parser.add_argument("--working-dir", required=True, type=Path,
                        help="Directory with working .asc tests.")
    parser.add_argument("--error-dir", required=True, type=Path,
                        help="Directory with error .asc tests.")
    parser.add_argument("--memcheck", action="store_true",
                        help="Also run each test under Valgrind.")
    return parser.parse_args()


def _validate_input_file(file_path: Path) -> None:
    if not os.path.isfile(file_path):
        raise FileNotFoundError(os.path.basename(file_path))
    if not os.access(file_path, os.R_OK):
        raise PermissionError(os.path.basename(file_path))


def load_csv(path: Path) -> list[list[str]]:
    """
    Return CSV rows, skipping blank lines.
    """
    with path.open(newline="", encoding="utf-8") as fh:
        reader = csv.reader(fh, delimiter=",", skipinitialspace=True)
        return [row for row in reader if any(cell.strip() for cell in row)]


def compare_csv(golden: Path, export: Path) -> tuple[int, str]:
    """
    Compare two CSVs.  
    Returns (return_code, extra_msg).
        0 → match, 1 → differ, 2 → I/O error
    """
    try:
        golden_rows = load_csv(golden)
        export_rows = load_csv(export)
        return (0, "") if golden_rows == export_rows else (1, "")
    except FileNotFoundError as e:
        return (2, f"{COLOR_RED}FileNotFound: {e}{SGR_RESET}")
    except PermissionError as e:
        return (2, f"{COLOR_RED}PermissionError: {e}{SGR_RESET}")
    except Exception as e:
        return (2, f"{COLOR_RED}Unhandled: {e}{SGR_RESET}")


def load_asc_filepaths(dirname: str) -> list[str]:
    asc_files = []
    for filename in os.listdir(dirname):
        filepath = os.path.join(dirname, filename)
        if filepath.endswith(ASC_EXT):
            asc_files.append(os.path.abspath(filepath))
    return asc_files


def ensure_alpha_compiler_executable(alpha_compiler_path: Path) -> None:
    if not os.path.isfile(alpha_compiler_path):
        raise ValueError("Alpha Compiler executable file not found.")
    if not os.access(alpha_compiler_path, os.X_OK):
        raise ValueError("Alpha Compiler file is not executable.")


def validate_symbol_table(golden_symbol_table_dir: Path, asc_filepath: Path) -> int:
    golden_filepath = os.path.join(
        golden_symbol_table_dir, f"{GOLD_FILES_PREFIX}{os.path.basename(asc_filepath)}{SYMTABLE_CSV}"
    )
    export_filepath = os.path.join(
        SYMBOL_TABLES_DIR, f"{os.path.basename(asc_filepath)}{SYMTABLE_CSV}"
    )
    return compare_csv(Path(golden_filepath), Path(export_filepath))


def validate_compile_errors(golden_compile_error_dir: Path, asc_filepath: Path) -> int:
    golden_filepath = os.path.join(
        golden_compile_error_dir, f"{GOLD_FILES_PREFIX}{os.path.basename(asc_filepath)}{ERRORS_CSV}"
    )
    export_filepath = os.path.join(
        COMPILE_ERRORS_DIR, f"{os.path.basename(asc_filepath)}{ERRORS_CSV}"
    )
    return compare_csv(Path(golden_filepath), Path(export_filepath))


def run_alpha_compiler(alpha_compiler_path: Path, asc_filepath: Path) -> int:
    ac_process_args = [
        str(alpha_compiler_path),
        "--export-symbol-table",
        "--export-compile-errors",
        "--no-show-errors",
        "--input-file",
        asc_filepath,
    ]
    completed_process = subprocess.run(ac_process_args)
    return completed_process.returncode


def run_valgrind_tests(
    alpha_compiler_path: Path, asc_filepath: Path, valgrind_error_exitcode: int
) -> int:
    valgrind_args = [
        "valgrind",
        "--leak-check=full",
        "--track-origins=yes",
        "--show-leak-kinds=all",
        "--errors-for-leak-kinds=all",
        f"--error-exitcode={valgrind_error_exitcode}",
        "--quiet",
    ]
    ac_args = [
        str(alpha_compiler_path),
        "--export-symbol-table",
        "--export-compile-errors",
        "--show-symbol-table",
        "--show-parser-trace",
        "--input-file",
        asc_filepath,
    ]

    completed_process = subprocess.run(
        valgrind_args + ac_args, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
    )

    return completed_process.returncode


def pretty_status(message: str, return_code: int) -> str:
    if return_code == 0:
        return f"{message}{COLOR_GREEN}{"PASS":<6}{SGR_RESET} "
    elif return_code == 1:
        return f"{message}{COLOR_RED}{"FAIL":<6}{SGR_RESET} "
    else:
        return f"{COLOR_RED}{message}{SGR_RESET}"


def run_test_file(args, asc_filepath) -> str:
    ac_retval = run_alpha_compiler(
        Path(args.alpha_compiler).resolve(), asc_filepath)
    result_line = []
    result_line.append(f"--Testing: {os.path.basename(asc_filepath):<30} ")
    result_line.append(pretty_status(f"AC_Exec:", ac_retval))
    if ac_retval != 0:
        return "".join(result_line)
    sym_retval, result_str = validate_symbol_table(
        args.golden_symbol_tables_dir, asc_filepath)
    result_line.append(pretty_status(
        f"Symtable:"+result_str, sym_retval))
    err_retval, result_str = validate_compile_errors(
        args.golden_compile_errors_dir, asc_filepath)
    result_line.append(pretty_status(
        f"CTErrors:"+result_str, err_retval))

    if args.memcheck:
        val_retval = run_valgrind_tests(
            Path(args.alpha_compiler).resolve(), asc_filepath, VALGRIND_ERROR_EXITCODE)
        result_line.append(pretty_status(f"MEMcheck:", val_retval))
    return "".join(result_line)


def print_progress_bar(completed: int, total: int, bar_width: int = 40):
    progress_ratio = completed / total
    filled = int(progress_ratio * bar_width)
    pointer = '>' if filled < bar_width else '='
    bar = '=' * (filled - 1 if filled > 0 else 0) + pointer
    spaces = ' ' * (bar_width - len(bar))
    percent = int(progress_ratio * 100)
    print(f"[{bar}{spaces}] {percent}% ({completed}/{total}) Using {os.cpu_count()} threads",
          end='\r', flush=True)


def run_test_files(args, asc_testfile_paths):
    asc_testfile_paths.sort()
    total_tests = len(asc_testfile_paths)
    test_completed = 0
    print_progress_bar(test_completed, total_tests)
    with ThreadPoolExecutor(max_workers=os.cpu_count()) as pool:
        futures = []
        for asc_testfile_path in asc_testfile_paths:
            future = pool.submit(run_test_file, args, asc_testfile_path)
            futures.append(future)

        for future in futures:
            result = future.result()
            with print_lock:
                test_completed += 1
                print()
                print_progress_bar(test_completed, total_tests)
                print("\033[2F\r")
                print(result)
        print_progress_bar(test_completed, total_tests)
        print()


def delete_export_dir(export_dir: Path) -> None:
    if export_dir.exists() and export_dir.is_dir():
        shutil.rmtree(export_dir)


def cleanup():
    delete_export_dir(Path(os.path.join(os.getcwd(), SYMBOL_TABLES_DIR)))
    delete_export_dir(Path(os.path.join(os.getcwd(), COMPILE_ERRORS_DIR)))


def main():
    args = parser_startup_arguments()
    ensure_alpha_compiler_executable(args.alpha_compiler)
    working_asc_files = load_asc_filepaths(args.working_dir)
    error_asc_files = load_asc_filepaths(args.error_dir)
    run_test_files(args, working_asc_files + error_asc_files)


if __name__ == "__main__":
    main()
