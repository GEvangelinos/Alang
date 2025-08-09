from typing import NamedTuple
from pathlib import Path

class ConfigArguments(NamedTuple):
    in_ir_spec_filepath: Path
    out_opcode_header_filepath: Path
    out_opcode_source_filepath: Path
    out_opcode_info_traits_header_filepath: Path
    out_opcode_opt_traits_header_filepath: Path

