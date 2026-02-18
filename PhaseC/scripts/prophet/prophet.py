import argparse
import os
import shutil
from pathlib import Path

from regression_utils import visible_len
from _colours import *

from executor import TestfileExecutor
from parser import TestfileParser
from model import StageError
import time

PROPHET_BANNER = r"""
 █████   █████ █████ █████ ██████████   ███████████     █████████  
▒▒███   ▒▒███ ▒▒███ ▒▒███ ▒▒███▒▒▒▒███ ▒▒███▒▒▒▒▒███   ███▒▒▒▒▒███ 
 ▒███    ▒███  ▒▒███ ███   ▒███   ▒▒███ ▒███    ▒███  ▒███    ▒███ 
 ▒███████████   ▒▒█████    ▒███    ▒███ ▒██████████   ▒███████████ 
 ▒███▒▒▒▒▒███    ▒▒███     ▒███    ▒███ ▒███▒▒▒▒▒███  ▒███▒▒▒▒▒███ 
 ▒███    ▒███     ▒███     ▒███    ███  ▒███    ▒███  ▒███    ▒███ 
 █████   █████    █████    ██████████   █████   █████ █████   █████
▒▒▒▒▒   ▒▒▒▒▒    ▒▒▒▒▒    ▒▒▒▒▒▒▒▒▒▒   ▒▒▒▒▒   ▒▒▒▒▒ ▒▒▒▒▒   ▒▒▒▒▒ 
"""
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


def print_simple_progress_bar(completed: int, total: int, move_cursor_up: bool):
    # if regression time is negligible.. then we assume 5 second for whole animation...
    animation_slot_delay = 5 / _total_tests

    def get_funny_comment(percent: int) -> str:
        if percent < 10:
            return "Starting optimistic..."
        elif percent < 20:
            return "Already regretting this build..."
        elif percent < 30:
            return "Pretending the parser works..."
        elif percent < 40:
            return "Gaslighting the optimizer again..."
        elif percent < 50:
            return "Blaming it on undefined behavior..."
        elif percent < 60:
            return "Asserting confidence. Failing assert."
        elif percent < 70:
            return "Bribing the linker with coffee..."
        elif percent < 80:
            return "Counting warnings as features..."
        elif percent < 90:
            return "Sacrificing another test to the CI gods..."
        elif percent < 100:
            return "Almost done... compiling hope."
        return "Regression passed."

    terminal_columns = shutil.get_terminal_size().columns
    progress_ratio = completed / total
    percent = int(progress_ratio * 100)
    funny_comment = get_funny_comment(percent)
    status_text = f" {percent}% ({completed}/{total}) {funny_comment + " " * (42 - len(funny_comment))}"
    bar_width = terminal_columns - visible_len(status_text) - 2  # -2 for [] for kernel of bar

    filled = int(progress_ratio * bar_width)
    pointer = '>' if filled < bar_width else '='
    bar = '=' * (filled - 1 if filled > 0 else 0) + pointer
    spaces = ' ' * (bar_width - len(bar))
    print(
        f"[{bar}{spaces}]{status_text}",
        end='\n\r', flush=True
    )
    if move_cursor_up:
        print("\033[2F\r")
    # TODO: enable before committing :D
    # time.sleep(animation_slot_delay)


def gather_test_filepaths(dirname: str) -> list[Path]:
    asc_files: list[Path] = []
    for filename in os.listdir(dirname):
        filepath = os.path.join(dirname, filename)
        if filepath.endswith(ASC_EXT):
            asc_files.append(Path(os.path.abspath(filepath)))
    return asc_files


def run_testfiles(driver_path: Path, test_filepaths: list[Path]):
    test_filepaths.sort()
    print_simple_progress_bar(_completed_tests, _total_tests, move_cursor_up=True)
    #
    for test_filepath in test_filepaths:
        run_testfile(driver_path, test_filepath)
        os.chdir(_workdir_path)
    print(end="\n" * 2)  # Required to move cursor past script's output and progress bar


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
        status_line = executor.status_line
        terminal_columns = shutil.get_terminal_size().columns
        status_line = status_line + " " * (terminal_columns - visible_len(status_line))
        print(status_line)
        _completed_tests += 1
        print_simple_progress_bar(_completed_tests, _total_tests, move_cursor_up=True)
    except StageError as e:
        print(f"{COLOR_RED}Prophet: Testfile-error:{SGR_RESET} {e}")


def main():
    print(PROPHET_BANNER)
    # TODO: enable before committing :D
    # time.sleep(3) # Just to scary the reader :D
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
