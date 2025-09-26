import argparse
import os
import shutil
import threading
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from _colours import *

from executor import TestfileExecutor
from parser import TestfileParser
from model import StageError

ASC_EXT = ".asc"

_total_tests = 0
_completed_tests = 0
_workdir_path = (Path(os.getcwd()) / Path("__WORK_DIR__")).resolve()


def parser_startup_arguments() -> argparse.Namespace:
    """
    Parse and validate command-line arguments for the Prophet regression runner.
    Prophet executes .asc golden test files:
      - Extracts SOURCE and expected outputs (IR, SYMBOL_TABLE, DIAGNOSTICS).
      - Runs the Alpha compiler with the embedded // RUN: directive.
      - Compares actual outputs against golden references.
    """
    import textwrap

    parser = argparse.ArgumentParser(
        prog="prophet",
        description="Prophet – Golden regression runner for the Alpha compiler.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=textwrap.dedent("""\
            Prophet test files (.asc) contain:
              - // RUN: command with %SELF and %DRIVER placeholders
              - // BEGIN_SOURCE ... // END_SOURCE
              - // BEGIN_IR ... // END_IR
              - // BEGIN_SYMBOL_TABLE ... // END_SYMBOL_TABLE
              - // BEGIN_DIAGNOSTICS ... // END_DIAGNOSTICS

            Prophet extracts the SOURCE block into a temp file,
            substitutes placeholders in the RUN command, executes it,
            and compares produced outputs against the embedded GOLDEN sections.
        """),
    )

    parser.add_argument(
        "--driver-path",
        required=True,
        help="Path to Alpha compiler driver."
    )

    parser.add_argument(
        "--gospel-dir",
        required=True,
        help="Path to Alpha compiler driver."
    )

    parser.add_argument(
        "--work-dir",
        required=True,
        help="Path to the dir where tests will be generated and tried."
    )

    parser.add_argument(
        "--valgrind",
        action="store_true",
        help="Run the compiler under Valgrind to check for leaks."
    )

    return parser.parse_args()


def ensure_driver_executable(alpha_compiler_path: Path) -> None:
    if not os.path.isfile(alpha_compiler_path):
        raise ValueError("Compiler executable file not found.")
    if not os.access(alpha_compiler_path, os.X_OK):
        raise ValueError("Compiler file is not executable.")


def print_progress_bar(completed: int, total: int, move_cursor_up: bool, bar_width: int = 40):
    progress_ratio = completed / total
    filled = int(progress_ratio * bar_width)
    pointer = '>' if filled < bar_width else '='
    bar = '=' * (filled - 1 if filled > 0 else 0) + pointer
    spaces = ' ' * (bar_width - len(bar))
    percent = int(progress_ratio * 100)
    print(
        f"[{bar}{spaces}] {percent}% ({completed}/{total})",
        end='\n\r', flush=True
    )
    if move_cursor_up:
        print("\033[2F\r")


def gather_test_filepaths(dirname: str) -> list[Path]:
    asc_files: list[Path] = []
    for filename in os.listdir(dirname):
        filepath = os.path.join(dirname, filename)
        if filepath.endswith(ASC_EXT):
            asc_files.append(Path(os.path.abspath(filepath)))
    return asc_files


def run_testfiles(driver_path: Path, test_filepaths: list[Path]):
    test_filepaths.sort()
    print_progress_bar(_completed_tests, _total_tests, move_cursor_up=True)
    #
    for test_filepath in test_filepaths:
        run_testfile(driver_path, test_filepath)
        os.chdir(_workdir_path)
    print(end="\n" * 2) # Required to move cursor past script's output and progress bar

def clean_work_dir():
    assert _workdir_path.exists() and _workdir_path.is_dir()
    shutil.rmtree(_workdir_path)

def run_testfile(driver_path: Path, testfile_path: Path) -> str:
    global _completed_tests
    try:
        parser = TestfileParser(driver_path.resolve(), testfile_path.resolve())
        testfile = parser.assemble_testfile()
        executor = TestfileExecutor(testfile)
        executor.run()
        print(executor.status_line)
        _completed_tests += 1
        print_progress_bar(_completed_tests, _total_tests, move_cursor_up=True)
    except StageError as e:
        print(f"{COLOR_RED}Prophet: Testfile-error:{SGR_RESET} {e}")



def main():
    try:
        global _total_tests
        global _workdir_path
        args = parser_startup_arguments()
        _workdir_path = Path(args.work_dir).resolve()
        TestfileExecutor.run_valgrind = args.valgrind
        TestfileExecutor.workdir_path = Path(args.work_dir).resolve()
        ensure_driver_executable(args.driver_path)
        test_filepaths = gather_test_filepaths(args.gospel_dir)
        _total_tests = len(test_filepaths)
        run_testfiles(Path(args.driver_path), test_filepaths)
    except Exception as e:
        print(f"{COLOR_RED}Prophet: Error:{SGR_RESET} {e}")


if __name__ == "__main__":
    main()
