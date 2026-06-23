from pathlib import Path
import re

from model import StageError, Testfile

COMMENT_TOKEN = "//"


class TestfileParser:
    COMPILER_RUN_MARKER = "__CMP_RUN__"
    VM_RUN_MARKER = "__VM_RUN__"
    BEGIN_SOURCE_MARKER = "__BEGIN_SOURCE__"
    END_SOURCE_MARKER = "__END_SOURCE__"
    BEGIN_IR_MARKER = "__BEGIN_IR__"
    END_IR_MARKER = "__END_IR__"
    BEGIN_SYMBOL_TABLE_MARKER = "__BEGIN_SYMBOL_TABLE__"
    END_SYMBOL_TABLE_MARKER = "__END_SYMBOL_TABLE__"
    BEGIN_DIAGNOSTICS_MARKER = "__BEGIN_DIAGNOSTICS__"
    END_DIAGNOSTICS_MARKER = "__END_DIAGNOSTICS__"
    BEGIN_VM_OUT_MARKER = "__BEGIN_VM_OUT__"
    END_VM_OUT_MARKER = "__END_VM_OUT__"
    BEGIN_VM_ERR_MARKER = "__BEGIN_VM_ERR__"
    END_VM_ERR_MARKER = "__END_VM_ERR__"

    SPACES = r"[ \t]*"

    COMPILER_RUN_REGEX = re.compile(rf"{COMMENT_TOKEN}{SPACES}{COMPILER_RUN_MARKER}{SPACES}:(.*)")
    VM_RUN_REGEX = re.compile(rf"{COMMENT_TOKEN}{SPACES}{VM_RUN_MARKER}{SPACES}:(.*)")
    BEGIN_SOURCE_RE = re.compile(rf"{COMMENT_TOKEN}{SPACES}{BEGIN_SOURCE_MARKER}{SPACES}")
    END_SOURCE_RE = re.compile(rf"{COMMENT_TOKEN}{SPACES}{END_SOURCE_MARKER}{SPACES}")
    BEGIN_IR_RE = re.compile(rf"{COMMENT_TOKEN}{SPACES}{BEGIN_IR_MARKER}{SPACES}")
    END_IR_RE = re.compile(rf"{COMMENT_TOKEN}{SPACES}{END_IR_MARKER}{SPACES}")
    BEGIN_SYMBOL_TABLE_RE = re.compile(rf"{COMMENT_TOKEN}{SPACES}{BEGIN_SYMBOL_TABLE_MARKER}{SPACES}")
    END_SYMBOL_TABLE_RE = re.compile(rf"{COMMENT_TOKEN}{SPACES}{END_SYMBOL_TABLE_MARKER}{SPACES}")
    BEGIN_DIAGNOSTICS_RE = re.compile(rf"{COMMENT_TOKEN}{SPACES}{BEGIN_DIAGNOSTICS_MARKER}{SPACES}")
    END_DIAGNOSTICS_RE = re.compile(rf"{COMMENT_TOKEN}{SPACES}{END_DIAGNOSTICS_MARKER}{SPACES}")
    BEGIN_VM_OUT_RE = re.compile(rf"{COMMENT_TOKEN}{SPACES}{BEGIN_VM_OUT_MARKER}{SPACES}")
    END_VM_OUT_RE = re.compile(rf"{COMMENT_TOKEN}{SPACES}{END_VM_OUT_MARKER}{SPACES}")
    BEGIN_VM_ERR_RE = re.compile(rf"{COMMENT_TOKEN}{SPACES}{BEGIN_VM_ERR_MARKER}{SPACES}")
    END_VM_ERR_RE = re.compile(rf"{COMMENT_TOKEN}{SPACES}{END_VM_ERR_MARKER}{SPACES}")

    def __init__(self, cmp_driver_path: Path, vm_driver_path: Path, testfile_path: Path):
        self.testfile_path = testfile_path
        self.cmp_driver_path = cmp_driver_path
        self.vm_driver_path = vm_driver_path
        self.testfile_lines = self.testfile_path.read_text().splitlines()

    def assemble_testfile(self) -> Testfile:
        testfile = Testfile(self.testfile_path.name)
        testfile.set_compiler_run_line(self.find_run_line(TestfileParser.COMPILER_RUN_REGEX, "cmp"), self.cmp_driver_path)

        vm_runline = self.find_run_line(TestfileParser.VM_RUN_REGEX, "vm", True)
        if not vm_runline:
            testfile.missing_vm_runline = True
        else:
            testfile.set_vm_run_line(vm_runline, self.vm_driver_path)

        testfile.set_source_code_section(self.find_source_section())


        if not testfile.cmp_error_mode and not testfile.skip_cmp_testing:
            testfile.set_gold_ir_section(self.find_gold_ir_section())
            testfile.set_gold_symbol_table_section(self.find_gold_symbol_table_section())
        if not testfile.cmp_error_mode and not testfile.missing_vm_runline:
            testfile.set_gold_vm_out_section(self.find_gold_vm_out_section())
            testfile.set_gold_vm_err_section(self.find_gold_vm_err_section())
        if not testfile.skip_cmp_testing:
            testfile.set_gold_diagnostic_section(self.find_gold_diagnostic_section())

        return testfile

    def find_run_line(self, runline_regex, str_ctx_desc, is_optional = False) -> str:
        for line in self.testfile_lines:
            run_match = runline_regex.fullmatch(line)
            if run_match:
                return run_match.group(1).strip()
        if is_optional:
            return ""
        raise StageError(f"Failed finding {str_ctx_desc} runline for {self.testfile_path}")

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

    def find_gold_vm_out_section(self) -> list[str]:
        section_name = "golden vm-out"
        golden_vm_out_section = self._find_section(
            TestfileParser.BEGIN_VM_OUT_RE,
            TestfileParser.END_VM_OUT_RE,
            section_name
        )
        return self._uncomment_section(golden_vm_out_section, section_name)

    def find_gold_vm_err_section(self) -> list[str]:
        section_name = "golden vm-err"
        golden_vm_err_section = self._find_section(
            TestfileParser.BEGIN_VM_ERR_RE,
            TestfileParser.END_VM_ERR_RE,
            section_name
        )
        return self._uncomment_section(golden_vm_err_section, section_name)

    def _find_section(self, begin_re: re.Pattern[str], end_re: re.Pattern[str], section_name: str) -> list[str]:
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

    def _uncomment_section(self, section: list[str], section_name: str) -> list[str]:
        cleaned_section: list[str] = []
        for line in section:
            cleaned = line.lstrip()
            if not cleaned.startswith(COMMENT_TOKEN):
                raise StageError(
                    f"{section_name} line does not start with comment token `{COMMENT_TOKEN}`: <<{line}>>"
                    f"\n\tfor {self.testfile_path}"
                )
            cleaned = cleaned.removeprefix(COMMENT_TOKEN)
            cleaned = cleaned.lstrip()
            cleaned_section.append(cleaned)
        return cleaned_section
