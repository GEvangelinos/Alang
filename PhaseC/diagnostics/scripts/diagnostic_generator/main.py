import argparse
import os
from typing import NamedTuple
from fsm_parsers import Diagnostic, DiagnosticFSM, LineTracker
from cpp_generator import CppGenerator


class CliArgs(NamedTuple):
    input_filename: str
    out_include: str
    out_src: str
    outfile_prefix: str


def assert_is_yaml_file(input_file: str):
    if not input_file.endswith(".yaml"):
        raise ValueError(f"`{input_file}` is not a yaml file. Expected a file ending with .yaml")
    if not os.path.isfile(input_file):
        raise ValueError(f"`{input_file}` is not a file.")
    if not os.access(input_file, os.R_OK):
        raise ValueError(f"`{input_file}` is not a readable file.")


def parse_cli_args() -> CliArgs:
    parser = argparse.ArgumentParser(description="Inject logging hooks into grammar files.")
    parser.add_argument("--input", required=True, help="Path to the input diagnostics .yaml file")
    parser.add_argument("--out-include", required=True, help="Path to directory for generated header files")
    parser.add_argument("--out-src", required=True, help="Path to directory for generated source files")
    parser.add_argument("--outfile-prefix", required=True, help="Name prefix that will be prepended in generated files")
    args = parser.parse_args()

    assert_is_yaml_file(args.input)

    return CliArgs(
        input_filename=args.input,
        out_include=args.out_include,
        out_src=args.out_src,
        outfile_prefix=args.outfile_prefix
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


def main():
    try:
        cli_args = parse_cli_args()
        yaml_lines = load_diagnostics_file(cli_args.input_filename)
        diagnostics = load_diagnostics(yaml_lines)
        cpp_generator = CppGenerator(cli_args.outfile_prefix, diagnostics)
        cpp_generator.generate_all_files()

    except Exception as e:
        print(e)


if __name__ == "__main__":
    main()
