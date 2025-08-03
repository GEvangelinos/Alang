import os.path
from functools import partial
from typing import TextIO

from yaml_parser import IROpInfo
from pathlib import Path


class CPPGenerator:
    alpha_ir_ops = "ALPHA_IR_OPS"

    def __init__(self,
                 out_header_filename: Path,
                 irop_dict: dict[str, IROpInfo],
                 available_optimization_set: set[str]):
        self._irop_dict = irop_dict
        self._out_header_filepath = out_header_filename
        self._available_optimization_set = available_optimization_set

    @property
    def out_header_filename(self):
        return os.path.basename(self._out_header_filepath)

    @property
    def include_guard(self):
        return self.out_header_filename.replace(".", "_").upper()

    def write_ir_opening(self, fout: TextIO, include_guard: str):
        fout.write(
            f"#ifndef {include_guard}\n"
            f"#define {include_guard}\n"
        )

    def write_ir_closing(self, fout: TextIO, include_guard: str):
        fout.write(
            f"#endif // {include_guard}\n"
        )

    def write_project_namespace_opening(self, fout: TextIO, project_namespace: str):
        fout.write(
            f"namespace {project_namespace}\n"
            f"{{\n"
        )

    def write_project_namespace_closure(self, fout: TextIO, project_namespace: str):
        fout.write(
            f"}} // namespace {project_namespace}\n"
        )

    def write_ir_opcode_x_macro(self, fout: TextIO, x_macro_name: str):
        irop_name_list = list(self._irop_dict.keys())

        fout.write(f"#define {x_macro_name} \\\n")
        for i in range(len(irop_name_list)):
            irop_name = irop_name_list[i]
            is_last_enum = i + 1 == len(irop_name_list)
            suffix = "\\" if not is_last_enum else ""
            fout.write(f'\tX({irop_name}) {suffix}\n')

    def write_ir_opcodes(self, fout: TextIO, x_macro_name: str):
        fout.write(
            f"enum class Op\n"
            f"{{\n"
            f"\t#define X(irop) irop,\n"
            f"\t{x_macro_name}\n"
            f"\t#undef  X\n"
            f"}};\n"
        )

    def write_ir_info_traits_definitions(self, fout: TextIO, namespace: str):
        fout.write(
            f"// Follows definitions of all generic Traits of IOPCode\n"
            f"namespace {namespace}\n"
            f"{{\n"
            f"template<ir::Op irop> inline constexpr bool uses_label = false;\n"
            f"template<ir::Op irop> inline constexpr unsigned char arg_count = 0;\n"
            f"}} // namespace {namespace}\n"
        )

    def write_ir_optim_trait_definitions(self, fout: TextIO, namespace: str):
        fout.write(
            f"// Follows definitions of all Optimization (Optims) Traits of IOPCode\n"
            f"namespace {namespace}\n"
            f"{{\n"
        )
        for optimization in self._available_optimization_set:
            fout.write(
                f"\n"
                f"template<ir::Op irop> inline constexpr bool can_{optimization} = false;\n"
            )
        fout.write(
            f"}} // namespace {namespace}\n"
        )

    def write_ir_info_trait_specializations(self, fout: TextIO, namespace):
        fout.write(
            f"// Follows definitions of all generic Traits of IOPCode\n"
            f"namespace {namespace}\n"
            f"{{\n"
        )
        for irop_name, irop_info in self._irop_dict.items():
            fout.write(
                f"template<> inline constexpr unsigned char arg_count<ir::Op::{irop_name}> = {irop_info.arg_count};\n"
            )
            if irop_info.uses_label:
                fout.write(
                    f"template<> inline constexpr bool uses_label<ir::Op::{irop_name}> = true;\n"
                )
        fout.write(
            f"}} // namespace {namespace}\n"
        )

    def write_ir_optim_trait_specializations(self, fout: TextIO, namespace: str):
        fout.write(
            f"// Follows definitions of all generic Traits of IOPCode\n"
            f"namespace {namespace}\n"
            f"{{\n"
        )
        for irop_name, irop_info in self._irop_dict.items():
            if irop_info.optimizations is None:
                continue
            for optimization in self._available_optimization_set:
                if optimization in irop_info.optimizations:
                    fout.write(
                        f"template<> inline constexpr bool can_{optimization}<ir::Op::{irop_name}> = true;\n"
                    )
        fout.write(
            f"\n"
            f"}} // namespace {namespace}\n"
        )

    def run(self):

        with open(self._out_header_filepath, "w") as fout:
            calls = [
                partial(self.write_ir_opening, fout, self.include_guard),
                partial(self.write_ir_opcode_x_macro, fout, CPPGenerator.alpha_ir_ops),
                partial(self.write_project_namespace_opening, fout, "alpha::ir"),
                partial(self.write_ir_opcodes, fout, CPPGenerator.alpha_ir_ops),
                partial(self.write_ir_info_traits_definitions, fout, "info_traits"),
                partial(self.write_ir_optim_trait_definitions, fout, "opt_traits"),
                partial(self.write_ir_info_trait_specializations, fout, "info_traits"),
                partial(self.write_ir_optim_trait_specializations, fout, "opt_traits"),
                partial(self.write_project_namespace_closure, fout, "alpha::ir"),
                partial(self.write_ir_closing, fout, self.include_guard),
            ]

            for call in calls:
                call()
                fout.write("\n")
