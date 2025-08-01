import argparse

from cpp_generator import CPPGenerator
from yaml_parser import YamlParser
from pathlib import Path
from typing import NamedTuple


class StartupArguments(NamedTuple):
    input_filepath: Path
    output_filepath: Path


def _parse_startup_arguments() -> StartupArguments:
    parser = argparse.ArgumentParser(
        description="Generates IOPCodes and all relevant subsystems for IR generation")
    parser.add_argument("--input", required=True, help="Path to the input IOPCode_spec.yaml file")
    parser.add_argument("--output", required=True,
                        help="Path to the output IOPCode C++ header file")

    args = parser.parse_args()

    return StartupArguments(
        input_filepath=Path(args.input),
        output_filepath=Path(args.output),
    )


def main():
    startup_args = _parse_startup_arguments()
    yaml_parser = YamlParser(startup_args.input_filepath, "AVAILABLE_OPTIMIZATIONS", "IOPCODES")
    iopcode_dict, available_optimization_set = yaml_parser.run()
    cpp_generator = CPPGenerator(
        startup_args.output_filepath, iopcode_dict, available_optimization_set)
    cpp_generator.run()


if __name__ == "__main__":
    main()
