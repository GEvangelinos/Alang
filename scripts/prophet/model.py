from enum import Enum
from pathlib import Path


class Placeholder(Enum):
    DRIVER = "%DRIVER"
    SKIP = "%SKIP"
    SELF = "%SELF"
    ERROR_MODE = "%ERROR_MODE"  # Optional placeholder


class StageError(RuntimeError):
    pass


class PlaceholderError(ValueError):
    pass


class Testfile:
    error_mode_flags = "--expect_errors --no_show_diagnostics"

    def __init__(self, filename: str):
        self.name: str = filename
        self.cmp_error_mode = False
        self.compiler_run_line: str = ""
        self.vm_run_line: str = ""
        self.source_section: list[str] = []
        self.gold_ir_section: list[str] = []
        self.gold_symbol_table_section: list[str] = []
        self.gold_vm_out_section: list[str] = []
        self.gold_vm_err_section: list[str] = []
        self.gold_diagnostic_section: list[str] = []
        self.missing_vm_runline = False
        self.skip_cmp_testing = False
        self.failed = False

    def set_compiler_run_line(self, run_line: str, driver_path: Path):
        if self.compiler_run_line:
            raise StageError("compiler run-line has been already filled")
        self.compiler_run_line = run_line
        self.compiler_run_line = self.substitute_driver_placeholder(self.compiler_run_line, driver_path)
        self.compiler_run_line = self.substitute_expect_errors_placeholder(self.compiler_run_line)
        self.compiler_run_line = self.handle_cmp_skip_flag(self.compiler_run_line)

    def set_vm_run_line(self, run_line: str, driver_path: Path):
        if self.vm_run_line:
            raise StageError("vm run-line has been already filled")
        self.vm_run_line = run_line
        self.vm_run_line = self.substitute_driver_placeholder(self.vm_run_line, driver_path)
        self.vm_run_line = self.substitute_expect_errors_placeholder(self.vm_run_line)

    def set_source_code_section(self, source_lnes: list[str]):
        if self.source_section:
            raise StageError("source_code_lines have been already filled")
        self.source_section = source_lnes

    def set_gold_ir_section(self, gold_ir_lines: list[str]):
        if self.gold_ir_section:
            raise StageError("gold_ir_lines have been already filled")
        self.gold_ir_section = gold_ir_lines

    def set_gold_symbol_table_section(self, gold_symbol_table_lines: list[str]):
        if self.gold_symbol_table_section:
            raise StageError("gold_symbol_table_lines have been already filled")
        self.gold_symbol_table_section = gold_symbol_table_lines

    def set_gold_diagnostic_section(self, gold_diagnostic_lines: list[str]):
        if self.gold_diagnostic_section:
            raise StageError("gold_diagnostic_lines have been already filled")
        self.gold_diagnostic_section = gold_diagnostic_lines

    def set_gold_vm_out_section(self, gold_vm_out_lines: list[str]):
        if self.gold_vm_out_section:
            raise StageError("gold_vm_out_lines have been already filled")
        self.gold_vm_out_section = gold_vm_out_lines

    def set_gold_vm_err_section(self, gold_vm_err_lines: list[str]):
        if self.gold_vm_err_section:
            raise StageError("gold_vm_err_lines have been already filled")
        self.gold_vm_err_section = gold_vm_err_lines

    def substitute_driver_placeholder(self, run_line:str, driver_path: Path) -> str:
        return self._substitute_placeholder(run_line, Placeholder.DRIVER, str(driver_path))
    
    def handle_cmp_skip_flag(self, run_line: str) -> str:
        if Placeholder.SKIP.value in run_line:
            self.skip_cmp_testing = True
            return self._substitute_placeholder(run_line, Placeholder.SKIP, " ")
        return run_line

    def substitute_self_placeholder(self, run_line:str, self_path: Path) -> str:
        self_path = Path(str(self_path).replace(' ', '\\ '))
        return self._substitute_placeholder(run_line, Placeholder.SELF, str(self_path))

    def substitute_self_abc_placeholder(self, run_line:str, self_path: Path) -> str:
        self_path = Path(str(self_path).replace(' ', '\\ '))
        return self._substitute_placeholder(run_line, Placeholder.SELF, str(self_path).removesuffix(".asc") + ".abc")

    def substitute_expect_errors_placeholder(self, run_line:str) -> str:
        # Optional placeholder
        if Placeholder.ERROR_MODE.value  in run_line:
            self.cmp_error_mode = True
            return self._substitute_placeholder(run_line, Placeholder.ERROR_MODE, Testfile.error_mode_flags)
        return run_line

    def _substitute_placeholder(self, run_line: str, placeholder: Placeholder, replacement: str) -> None:
        if not run_line:
            raise StageError("run_line is empty, cannot substitute placeholders")
        if placeholder.value not in run_line:
            raise PlaceholderError(
                f"Placeholder {placeholder.value} is missing in run_line: <<{run_line}>>")
        if run_line.count(placeholder.value) > 1:
            raise PlaceholderError(
                f"Placeholder {placeholder.value} exists more than once in run_line: <<{run_line}>>")
        return run_line.replace(placeholder.value, replacement)
