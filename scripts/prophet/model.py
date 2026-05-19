from enum import Enum
from pathlib import Path


class Placeholder(Enum):
    DRIVER = "%DRIVER"
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
        self.error_mode = False
        self.run_line: str = ""
        self.source_section: list[str] = []
        self.gold_ir_section: list[str] = []
        self.gold_symbol_table_section: list[str] = []
        self.gold_diagnostic_section: list[str] = []

    def set_run_line(self, run_line: str, driver_path: Path):
        if self.run_line:
            raise StageError("run_line has been already filled")
        self.run_line = run_line
        self.substitute_driver_placeholder(driver_path)
        self.substitute_expect_errors_placeholder()

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

    def substitute_driver_placeholder(self, driver_path: Path) -> None:
        self._substitute_placeholder(Placeholder.DRIVER, str(driver_path))

    def substitute_self_placeholder(self, self_path: Path) -> None:
        self_path = Path(str(self_path).replace(' ', '\\ '))
        self._substitute_placeholder(Placeholder.SELF, str(self_path))

    def substitute_expect_errors_placeholder(self):
        # Optional placeholder
        if Placeholder.ERROR_MODE.value  in self.run_line:
            self.error_mode = True
            self._substitute_placeholder(Placeholder.ERROR_MODE, Testfile.error_mode_flags)

    def _substitute_placeholder(self, placeholder: Placeholder, replacement: str) -> None:
        if not self.run_line:
            raise StageError("run_line is empty, cannot substitute placeholders")

        if placeholder.value not in self.run_line:
            raise PlaceholderError(
                f"Placeholder {placeholder.value} is missing in run_line: <<{self.run_line}>>")
        if self.run_line.count(placeholder.value) > 1:
            raise PlaceholderError(
                f"Placeholder {placeholder.value} exists more than once in run_line: <<{self.run_line}>>")

        self.run_line = self.run_line.replace(placeholder.value, replacement)
