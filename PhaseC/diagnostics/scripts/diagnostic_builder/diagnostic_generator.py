import argparse
import os
from typing import NamedTuple
from diagnostic_fsm_parsers import DiagnosticFSM


class StartupArguments(NamedTuple):
    input_filename: str


def assert_is_yaml_file(input_file: str):
    if not input_file.endswith(".yaml"):
        raise ValueError(f"`{input_file}` is not a yaml file. Expected a file ending with .yaml")
    if not os.path.isfile(input_file):
        raise ValueError(f"`{input_file}` is not a file.")
    if not os.access(input_file, os.R_OK):
        raise ValueError(f"`{input_file}` is not a readable file.")


def parse_startup_arguments() -> StartupArguments:
    parser = argparse.ArgumentParser(description="Inject logging hooks into grammar files.")
    parser.add_argument("--input", required=True, help="Path to the input diagnostics .yaml file")
    args = parser.parse_args()

    assert_is_yaml_file(args.input)

    return StartupArguments(input_filename=args.input)


def load_diagnostics_file(input_filename) -> list[str]:
    with open(input_filename, 'r') as fin:
        return fin.readlines()


def load_diagnostics(yaml_lines: list[str]):
    fsm = DiagnosticFSM()
    fsm.parse_diagnostic_fsm(yaml_lines, 1)


def main():
    startup_args = parse_startup_arguments()
    yaml_lines = load_diagnostics_file(startup_args.input_filename)
    load_diagnostics(yaml_lines)

if __name__ == "__main__":
    main()
