import argparse

from cpp_generator import CPPGenerator
from yaml_parser import YamlParser
from pathlib import Path
from typing import NamedTuple


class StartupArguments(NamedTuple):
    ir_spec_filepath: Path
    out_header_filepath: Path


def _parse_startup_arguments() -> StartupArguments:
    parser = argparse.ArgumentParser(
        description="Generates IOPCodes and all relevant subsystems for IR generation")
    parser.add_argument("--ir-spec", required=True, help="Path to the input IOPCode_spec.yaml file")
    parser.add_argument("--out-header", required=True,
                        help="Path to the output IOPCode C++ header file")

    args = parser.parse_args()

    return StartupArguments(
        ir_spec_filepath=Path(args.ir_spec),
        out_header_filepath=Path(args.out_header),
    )


def main():
    startup_args = _parse_startup_arguments()
    yaml_parser = YamlParser(startup_args.ir_spec_filepath, "AVAILABLE_OPTIMIZATIONS", "IR_OPS")
    iopcode_dict, available_optimization_set = yaml_parser.run()
    cpp_generator = CPPGenerator(
        startup_args.out_header_filepath, iopcode_dict, available_optimization_set)
    cpp_generator.run()


if __name__ == "__main__":
    main()
