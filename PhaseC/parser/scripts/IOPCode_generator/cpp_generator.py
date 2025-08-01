import os.path
from typing import TextIO

from yaml_parser import IOPCodeInfo
from pathlib import Path


class CPPGenerator:

    def __init__(self,
                 output_filepath: Path,
                 iopcode_dict: dict[str, IOPCodeInfo],
                 available_optimization_set: set[str]):
        self._iopcode_dict = iopcode_dict
        self._output_filepath = output_filepath
        self._available_optimization_set = available_optimization_set

    @property
    def output_filename(self):
        return os.path.basename(self._output_filepath)

    @property
    def include_guard(self):
        return self.output_filename.replace(".", "_").upper()

    def write_iopcode_opening(self, fout: TextIO):
        fout.write(
            f"#ifndef {self.include_guard}\n"
            f"#define {self.include_guard}\n"
            f"\n"
        )

    def write_iopcode_x_macro(self, fout: TextIO):
        iopc_name_list = list(self._iopcode_dict.keys())

        fout.write(f"#define ALPHA_IOPCODES \\\n")
        for i in range(len(iopc_name_list)):
            iopc_name = iopc_name_list[i]
            is_last_enum = i + 1 == len(iopc_name_list)
            suffix = "\\" if is_last_enum else ""
            fout.write(f'\tX({iopc_name}) {suffix}\n')
        fout.write("\n")

    def write_iopcode_enum_class(self, fout: TextIO):
        fout.write(
            f"namespace Alpha::IOPC\n"
            f"{{\n"
            f"enum class Code\n"
            f"{{\n"
            f"\t#define X(iopcode) iopcode,\n"
            f"\tALPHA_IOPCODES\n"
            f"\t#undef  X\n"
            f"}};\n"
            f"}} // namespace Alpha::IOPC\n"
        )

    def write_general_trait_definitions(self, fout:TextIO):
         fout.write(
            f"\n"
            f"// Follows definitions of all generic Traits of IOPCode\n"
            f"namespace Alpha::IOPC::Traits"
            f"{{\n"
            f"template<IOPCode iopc> inline constexpr bool uses_label = false;\n"
            f"template<IOPCode iopc> inline constexpr unsigned char argument_count;\n"
            f"\n"
            f"}} // namespace Alpha::IOPC::Traits\n"
         )
    def write_optim_trait_definitions(self, fout:TextIO):
        f"\n"
        f"// Follows definitions of all Optimization (Optims) Traits of IOPCode"
        f"namespace Alpha::IOPC::Traits:Optims"
        f"{{"

        for optimization in self._available_optimization_set:
            fout.write(
                f"\n"
                f"template<IOPCode iopc> inline constexpr bool can_{optimization} = false;\n"
            )




    def generate_iopcodes(self, fout: TextIO):

        fout.write("\n")

        for iopc_name, iopc_info in self._iopcode_dict.items():
            fout.write(
                f"template<> inline constexpr unsigned char argument_count<IOPCode::{iopc_name} = {iopc_info.arg_count};\n"
            )

        fout.write("\n")

        for iopc_name, iopc_info in self._iopcode_dict.items():
            if iopc_info.uses_label:
                fout.write(
                    f"template<> inline constexpr bool uses_label<IOPCode::{iopc_name}> = true;\n"
                )

        fout.write("\n")

        for iopc_name, iopc_info in self._iopcode_dict.items():
            if iopc_info.optimizations is None:
                continue
            for optimization in self._available_optimization_set:
                if optimization in iopc_info.optimizations:
                    fout.write(
                        f"template<> inline constexpr bool can_{optimization}<IOPCode::{iopc_name}> = true;\n"
                    )
            fout.write("\n")

        fout.write(
            f"}} // namespace Alpha\n"
            f"#endif // {self.include_guard}\n"
        )

    def run(self):
        with open(self._output_filepath, "w") as fout:
            self.write_iopcode_opening(fout)
            self.write_iopcode_x_macro(fout)
            self.write_iopcode_enum_class(fout)
            self.write_general_trait_definitions(fout)
            self.write_optim_trait_definitions(fout)
