import argparse
import os
from typing import NamedTuple


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

    return StartupArguments(
        input_filename=args.input
    )


def yaml_document_parser():
    pass


def load_diagnostics_file(startup_args: StartupArguments):
    yaml_filename = startup_args.input_filename

    with open()
