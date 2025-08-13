import argparse

from cpp_generator import CPPGenerator
from yaml_parser import YamlParser
from pathlib import Path
from typing import NamedTuple

from shared_config import ConfigArguments



def _parse_config_arguments() -> ConfigArguments:
    parser = argparse.ArgumentParser(
        description="Generates IOPCodes and all relevant subsystems for IR generation")
    parser.add_argument(
        "--in-ir-spec",
        required=True,
        help="Path to the input IOPCode_spec.yaml file"
    )
    parser.add_argument(
        "--out-iropcode-header",
        required=True,
        help="Path to the output header file containing the ir::Opcode enum class and more"
    )
    parser.add_argument(
        "--out-iropcode-source",
        required=True,
        help="Path to the output source file containing implementation code"
    )
    parser.add_argument(
        "--out-iropcode-info-traits-header",
        required=True,
        help="Path to the output header file containing info traits of ir::Opcode (e.g., operand count, use of label, etc.)"
    )
    parser.add_argument(
        "--out-iropcode-opt-traits-header",
        required=True,
        help="Path to the output header file containing optimization traits of ir::Opcode (e.g., fold, trim support)"
    )

    args = parser.parse_args()

    return ConfigArguments(
        in_ir_spec_filepath=Path(args.in_ir_spec),
        out_opcode_header_filepath=Path(args.out_iropcode_header),
        out_opcode_source_filepath=Path(args.out_iropcode_source),
        out_opcode_info_traits_header_filepath=Path(args.out_iropcode_info_traits_header),
        out_opcode_opt_traits_header_filepath=Path(args.out_iropcode_opt_traits_header),
    )


def main():
    try:
        config_args = _parse_config_arguments()
        yaml_parser = YamlParser(
            config_args,
            "IROPCODE_INFO_ENUMS",
            "IROPCODE_INFO",
            "IROPCODE_OPTIMIZATION_FLAGS",
            "IROPCODE_LIST"
        )
        yaml_parser_products = yaml_parser.run()

        cpp_generator = CPPGenerator(
            config_args,
            yaml_parser_products,
            "Opcode"
        )

        cpp_generator.run()
    except RuntimeError as e:
        print(e)



if __name__ == "__main__":
    main()
