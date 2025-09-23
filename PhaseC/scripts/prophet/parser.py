from pathlib import Path
import re

from model import StageError, Testfile

COMMENT_TOKEN = "//"


class TestfileParser:
    RUN_MARKER = "__RUN__"
    BEGIN_SOURCE_MARKER = "__BEGIN_SOURCE__"
    END_SOURCE_MARKER = "__END_SOURCE__"
    BEGIN_IR_MARKER = "__BEGIN_IR__"
    END_IR_MARKER = "__END_IR__"
    BEGIN_SYMBOL_TABLE_MARKER = "__BEGIN_SYMBOL_TABLE__"
    END_SYMBOL_TABLE_MARKER = "__END_SYMBOL_TABLE__"
    BEGIN_DIAGNOSTICS_MARKER = "__BEGIN_DIAGNOSTICS__"
    END_DIAGNOSTICS_MARKER = "__END_DIAGNOSTICS__"

    SPACES = r"[ \t]*"

    RUN_REGEX = re.compile(rf"{COMMENT_TOKEN}{SPACES}{RUN_MARKER}{SPACES}:(.*)")
    BEGIN_SOURCE_RE = re.compile(rf"{COMMENT_TOKEN}{SPACES}{BEGIN_SOURCE_MARKER}{SPACES}")
    END_SOURCE_RE = re.compile(rf"{COMMENT_TOKEN}{SPACES}{END_SOURCE_MARKER}{SPACES}")
    BEGIN_IR_RE = re.compile(rf"{COMMENT_TOKEN}{SPACES}{BEGIN_IR_MARKER}{SPACES}")
    END_IR_RE = re.compile(rf"{COMMENT_TOKEN}{SPACES}{END_IR_MARKER}{SPACES}")
    BEGIN_SYMBOL_TABLE_RE = re.compile(
        rf"{COMMENT_TOKEN}{SPACES}{BEGIN_SYMBOL_TABLE_MARKER}{SPACES}")
    END_SYMBOL_TABLE_RE = re.compile(rf"{COMMENT_TOKEN}{SPACES}{END_SYMBOL_TABLE_MARKER}{SPACES}")
    BEGIN_DIAGNOSTICS_RE = re.compile(rf"{COMMENT_TOKEN}{SPACES}{BEGIN_DIAGNOSTICS_MARKER}{SPACES}")
    END_DIAGNOSTICS_RE = re.compile(rf"{COMMENT_TOKEN}{SPACES}{END_DIAGNOSTICS_MARKER}{SPACES}")

    def __init__(self, driver_path: Path, testfile_path: Path):
        self.testfile_path = testfile_path
        self.driver_path = driver_path
        self.testfile_lines = self.testfile_path.read_text().splitlines()

    def assemble_testfile(self) -> Testfile:
        testfile = Testfile(self.testfile_path.name)
        testfile.set_run_line(self.find_run_line(), self.driver_path)
        testfile.set_source_code_section(self.find_source_section())
        testfile.set_gold_ir_section(self.find_gold_ir_section())
        testfile.set_gold_symbol_table_section(self.find_gold_symbol_table_section())
        testfile.set_gold_diagnostic_section(self.find_gold_diagnostic_section())
        return testfile

    def find_run_line(self) -> str:
        for line in self.testfile_lines:
            run_match = TestfileParser.RUN_REGEX.fullmatch(line)
            if run_match:
                return run_match.group(1).strip()
        raise StageError(f"Failed finding run line for {self.testfile_path}")

    def find_source_section(self) -> list[str]:
        return self._find_section(
            TestfileParser.BEGIN_SOURCE_RE,
            TestfileParser.END_SOURCE_RE,
            "source"
        )

    def find_gold_ir_section(self) -> list[str]:
        section_name = "golden IR"
        golden_ir_section = self._find_section(
            TestfileParser.BEGIN_IR_RE,
            TestfileParser.END_IR_RE,
            section_name
        )
        return self._uncomment_section(golden_ir_section, section_name)

    def find_gold_symbol_table_section(self) -> list[str]:
        section_name = "golden symbol table"
        golden_symbol_table_section = self._find_section(
            TestfileParser.BEGIN_SYMBOL_TABLE_RE,
            TestfileParser.END_SYMBOL_TABLE_RE,
            section_name
        )
        return self._uncomment_section(golden_symbol_table_section, section_name)

    def find_gold_diagnostic_section(self) -> list[str]:
        section_name = "golden diagnostic"
        golden_diagnostic_section = self._find_section(
            TestfileParser.BEGIN_DIAGNOSTICS_RE,
            TestfileParser.END_DIAGNOSTICS_RE,
            section_name
        )
        return self._uncomment_section(golden_diagnostic_section, section_name)

    def _find_section(
            self, begin_re: re.Pattern[str], end_re: re.Pattern[str], section_name: str) \
            -> list[str]:
        section_lines: list[str] = []
        inside_section = False

        for line in self.testfile_lines:
            if inside_section:
                if end_re.fullmatch(line):
                    return section_lines
                section_lines.append(line)
            elif begin_re.fullmatch(line):
                inside_section = True

        point = "ending" if inside_section else "beginning"
        raise StageError(f"Failed finding {point} of {section_name} lines for {self.testfile_path}")

    @staticmethod
    def _uncomment_section(section: list[str], section_name: str) -> list[str]:
        cleaned_section: list[str] = []
        for line in section:
            cleaned = line.lstrip()
            if not cleaned.startswith(COMMENT_TOKEN):
                raise StageError(
                    f"{section_name} line does not start with comment token `{COMMENT_TOKEN}`: <<{line}>>")
            cleaned = cleaned.removeprefix(COMMENT_TOKEN)
            cleaned = cleaned.lstrip()
            cleaned_section.append(cleaned)
        return cleaned_section
