import argparse
import os
import sys
from typing import NamedTuple
from fsm_parsers import Diagnostic, DiagnosticFSM, LineTracker
from cpp_generator import CppGenerator
from pathlib import Path


class CliArgs(NamedTuple):
    spec_filepath: str
    out_include_dir: str
    out_src_dir: str
    file_prefix: str
    stamp_filepath: str


def assert_is_yaml_file(input_file: str):
    if not input_file.endswith(".yaml"):
        raise ValueError(f"`{input_file}` is not a yaml file. Expected a file ending with .yaml")
    if not os.path.isfile(input_file):
        raise ValueError(f"`{input_file}` is not a file.")
    if not os.access(input_file, os.R_OK):
        raise ValueError(f"`{input_file}` is not a readable file.")


def parse_cli_args() -> CliArgs:
    parser = argparse.ArgumentParser(description="Generate diagnostic source and header files.")
    parser.add_argument("--spec-filepath", required=True, help="Path to the diagnostics YAML specification file")
    parser.add_argument("--out-include-dir", required=True, help="Path to directory for generated header files")
    parser.add_argument("--out-src-dir", required=True, help="Path to directory for generated source files")
    parser.add_argument("--file-prefix", required=True, help="Prefix for all generated filenames")
    parser.add_argument("--stamp-filepath", required=True, help="Path to stamp file used by CMake to track rebuilds")
    args = parser.parse_args()

    assert_is_yaml_file(args.spec_filepath)

    return CliArgs(
        spec_filepath=args.spec_filepath,
        out_include_dir=args.out_include_dir,
        out_src_dir=args.out_src_dir,
        file_prefix=args.file_prefix,
        stamp_filepath=args.stamp_filepath
    )


def load_diagnostics_file(input_filename: str) -> list[str]:
    with open(input_filename, 'r') as fin:
        return fin.readlines()


def load_diagnostics(yaml_lines: list[str]) -> list[Diagnostic]:
    line_tracker = LineTracker(yaml_lines)
    diagnostics = []
    while not line_tracker.at_end():
        line_tracker.skip_empty_lines()
        diagnostics.append(DiagnosticFSM().parse_diagnostic_fsm(line_tracker))
    return diagnostics


def renew_stamp(cli_args: CliArgs) -> None:
    Path(cli_args.stamp_filepath).touch()


def main():
    try:
        cli_args = parse_cli_args()
        yaml_lines = load_diagnostics_file(cli_args.spec_filepath)
        diagnostics = load_diagnostics(yaml_lines)
        cpp_generator = CppGenerator(cli_args, diagnostics)
        cpp_generator.generate_all_files()
        renew_stamp(cli_args)
    except(RuntimeError, ValueError) as e:
        print(e)
        sys.exit(1)


if __name__ == "__main__":
    main()
