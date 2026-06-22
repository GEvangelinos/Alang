import shutil

from model import Testfile
from pathlib import Path
import os
import subprocess
import shlex
from _colours import *

_SYMTABLE_CSV_EXT = ".st.csv"
_IR_CSV_EXT = ".ir.csv"
_DIAGNOSTICS_CSV_EXT = ".diag.csv"
_VM_OUT_EXT = ".vmout.txt"
_VM_ERR_EXT = ".vmerr.txt"
_GOLD_PREFIX = "GOLD_"

_EXIT_SUCCESS_RETURNCODE = 0
_EXIT_FAILURE_RETURNCODE = 1


class TestfileExecutor:
    workdir_path = Path(os.getcwd())
    run_valgrind = False
    _valgrind_error_exitcode = 1

    def __init__(self, testfile: Testfile):
        self.testfile: Testfile = testfile
        self.test_dirpath = Path(TestfileExecutor.workdir_path) / Path(testfile.name).stem

        self.gold_symbol_table_filename = _GOLD_PREFIX + testfile.name + _SYMTABLE_CSV_EXT
        self.gold_ir_filename = _GOLD_PREFIX + testfile.name + _IR_CSV_EXT
        self.gold_diagnostics_filename = _GOLD_PREFIX + testfile.name + _DIAGNOSTICS_CSV_EXT

        self.gold_vm_out_filename = _GOLD_PREFIX + testfile.name + _VM_OUT_EXT
        self.gold_vm_err_filename = _GOLD_PREFIX + testfile.name + _VM_ERR_EXT

        self.out_symbol_table_filename = testfile.name + _SYMTABLE_CSV_EXT
        self.out_ir_filename = testfile.name + _IR_CSV_EXT
        self.out_diagnostics_filename = testfile.name + _DIAGNOSTICS_CSV_EXT
        self.out_vmout_filename = testfile.name + _VM_OUT_EXT
        self.out_vmerr_filename = testfile.name + _VM_ERR_EXT

        self._status_line: list[str] = []

    def run(self):
        self.prepare_test_dir()
        os.chdir(self.test_dirpath)
        self.prepare_test_samples()
        self.testfile.compiler_run_line = self.testfile.substitute_self_placeholder(self.testfile.compiler_run_line, self.test_dirpath / self.testfile.name)

        if self.execute_compiler_run_line() == _EXIT_SUCCESS_RETURNCODE:
            self.flatten_exports()
            self.validate_compile_side_testfile()

        if self.testfile.missing_vm_runline:
            self._status_line.append(f"{COLOR_YELLOW}(Missing VM run-command){SGR_RESET}")
            return
        self.testfile.vm_run_line = self.testfile.substitute_self_abc_placeholder(self.testfile.vm_run_line, self.test_dirpath / self.testfile.name)
        self.testfile.vm_run_line += f" --out-file {self.testfile.name}.vmout.txt "
        self.testfile.vm_run_line += f" --err-file {self.testfile.name}.vmerr.txt "
        if self.execute_vm_run_line() == _EXIT_SUCCESS_RETURNCODE:
            self.validate_vm_side_testfile()

        os.chdir(Path(TestfileExecutor.workdir_path))

    @property
    def status_line(self) -> str:
        assert self._status_line
        return "".join(self._status_line)

    def prepare_test_dir(self):
        # Usually test dirs exist from old runs, so we delete dir and its contents and remake.
        if os.path.exists(self.test_dirpath):
            shutil.rmtree(self.test_dirpath)
        os.makedirs(self.test_dirpath, exist_ok=True)

    def prepare_test_samples(self):
        # Compiler:
        with open(self.testfile.name, 'w') as fout:
            fout.write("\n".join(self.testfile.source_section))
        with open(self.gold_diagnostics_filename, 'w') as fout:
            fout.write("\n".join(self.testfile.gold_diagnostic_section))

        if self.testfile.cmp_error_mode:
            return

        with open(self.gold_ir_filename, 'w') as fout:
            fout.write("\n".join(self.testfile.gold_ir_section))
        with open(self.gold_symbol_table_filename, 'w') as fout:
            fout.write("\n".join(self.testfile.gold_symbol_table_section))

        # VirtualMachine:
        with open(self.gold_vm_out_filename, 'w') as fout:
            fout.write("\n".join(self.testfile.gold_vm_out_section))
        with open(self.gold_vm_err_filename, 'w') as fout:
            fout.write("\n".join(self.testfile.gold_vm_err_section))


    def execute_compiler_run_line(self) -> int:
        completed_subprocess = subprocess.run(
            shlex.split(self.testfile.compiler_run_line),
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,       # capture stderr
            text=True                     # decode bytes -> str automatically
        )
        self._status_line.append(f"--Testing: {self.testfile.name:<60} ")
        if completed_subprocess.returncode != _EXIT_SUCCESS_RETURNCODE:
            self._status_line.append(
                f"{COLOR_RED}Failure, test produced errors.{SGR_RESET}")
            if completed_subprocess.stderr:
                self._status_line.append("\n\n--- STDERR-BEGIN ---\n")
                self._status_line.append(completed_subprocess.stderr)
                self._status_line.append("\n--- STDERR-END   ---\n\n")
        return completed_subprocess.returncode

    def execute_vm_run_line(self) -> int:
        completed_subprocess = subprocess.run(
            shlex.split(self.testfile.vm_run_line),
            text=True                     # decode bytes -> str automatically
        )
        # if completed_subprocess.returncode != _EXIT_SUCCESS_RETURNCODE:
        #     self._status_line.append(
        #         f"{COLOR_RED}Failure, test produced errors.{SGR_RESET}")
        #     if completed_subprocess.stderr:
        #         self._status_line.append("\n\n--- STDERR-BEGIN ---\n")
        #         self._status_line.append(completed_subprocess.stderr)
        #         self._status_line.append("\n--- STDERR-END   ---\n\n")
        return completed_subprocess.returncode

    def flatten_exports(self):
        dest_path = self.test_dirpath
        assert dest_path.is_dir()

        def recurse():
            dir_entries = [Path(f) for f in os.listdir()]
            current_dirpath = Path(os.getcwd())
            for entry in dir_entries:
                entry_fullpath = entry.resolve()
                if entry.is_file() and entry_fullpath.parent != dest_path:
                    if not (dest_path / entry_fullpath.name).exists():
                        shutil.move(entry_fullpath, dest_path)
                if entry.is_dir():
                    os.chdir(entry_fullpath)
                    recurse()
                    os.chdir(current_dirpath)
            if current_dirpath != dest_path:
                shutil.rmtree(current_dirpath)

        recurse()
        os.chdir(dest_path)

    @staticmethod
    def load_csv(path: Path) -> list[list[str]]:
        import csv
        """
        Return CSV rows, skipping blank lines.
        """
        with path.open(newline="", encoding="utf-8") as fh:
            reader = csv.reader(fh, delimiter=",", skipinitialspace=True)
            rows: list[list[str]] = []
            for row in reader:
                row = [cell.strip() for cell in row]
                if any(row):
                    rows.append(row)
            return rows

    @staticmethod
    def cmp_file_lines(golden: Path, out: Path) -> tuple[int, str]:
        """
        Compare two CSVs.
        Returns (return_code, extra_msg).
            0 → match, 1 → differ, 2 → I/O error
        """
        try:
            with open(golden, 'r') as f1, open(out, 'r') as f2:
                golden_lines = [line.rstrip() for line in f1]
                export_lines = [line.rstrip() for line in f2]

            if golden_lines == export_lines:
                return 0, ""
            # Optional: Find the first line that differs for the extra_msg
            for i, (l1, l2) in enumerate(zip(golden_lines, export_lines)):
                if l1 != l2:
                    return 1, f"Mismatch at line {i+1}: '{l1.strip()}' vs '{l2.strip()}'"
            return 1, "Files have different line counts"

            ###  DEPRECATED WAY OF DOING THE COMPARISON (CSV Readers ignore quotes)  ###
            # golden_rows = TestfileExecutor.load_csv(golden)
            # export_rows = TestfileExecutor.load_csv(out)
            # return (0, "") if golden_rows == export_rows else (1, "")
        except FileNotFoundError as e:
            return 2, f"{COLOR_BLACK}FileNotFound: {e}{SGR_RESET}"
        except PermissionError as e:
            return 2, f"{COLOR_RED}PermissionError: {e}{SGR_RESET}"
        except Exception as e:
            return 2, f"{COLOR_RED}Unhandled: {e}{SGR_RESET}"

    @staticmethod
    def pretty_status(message: str, return_code: int) -> str:
        if return_code == 0:
            return f"{message}{COLOR_GREEN}{"PASS" : <5}{SGR_RESET}"
        elif return_code == 1:
            return f"{message}{COLOR_RED}{"FAIL" : <5}{SGR_RESET}"
        else:
            return f"{COLOR_RED}{message}{SGR_RESET}"

    def validate_vm_side_testfile(self):
        # VirtualMachine:
        self._status_line.append("VM: [ ")

        td = self.test_dirpath
        ret, msg = TestfileExecutor.cmp_file_lines(td / self.gold_vm_out_filename, td / self.out_vmout_filename)
        self._status_line.append(TestfileExecutor.pretty_status(f"Out:" + msg, ret))

        ret, msg = TestfileExecutor.cmp_file_lines(td / self.gold_vm_err_filename, td / self.out_vmerr_filename)
        self._status_line.append(TestfileExecutor.pretty_status(f"Err:" + msg, ret))

        if TestfileExecutor.run_valgrind:
            ret = self.run_valgrind_tests(self.testfile.vm_run_line)
            self._status_line.append(TestfileExecutor.pretty_status(f"Memcheck:", ret))

        self._status_line.append("] ")

    def validate_compile_side_testfile(self):
        os.chdir(self.test_dirpath)

        assert os.path.exists(self.gold_diagnostics_filename)
        assert os.path.exists(self.out_diagnostics_filename)
        if not self.testfile.cmp_error_mode:
            assert os.path.exists(self.gold_ir_filename)
            assert os.path.exists(self.gold_symbol_table_filename)
            assert os.path.exists(self.out_ir_filename)
            assert os.path.exists(self.out_symbol_table_filename)

        td = self.test_dirpath

        # Compiler:
        self._status_line.append("CMP: [ ")

        if not self.testfile.cmp_error_mode:
            ret, msg = TestfileExecutor.cmp_file_lines(td / self.gold_ir_filename, td / self.out_ir_filename)
            self._status_line.append(TestfileExecutor.pretty_status(f"Ir:" + msg, ret))

            ret, msg = TestfileExecutor.cmp_file_lines(td / self.gold_symbol_table_filename, td / self.out_symbol_table_filename)
            self._status_line.append(TestfileExecutor.pretty_status(f"Symtable:" + msg, ret))
        else:
            # extra 22 spaces to align "Diagnostics:"  field with working tests
            self._status_line.append(' ' * 22)

        ret, msg = TestfileExecutor.cmp_file_lines(td / self.gold_diagnostics_filename, td / self.out_diagnostics_filename)
        self._status_line.append(TestfileExecutor.pretty_status(f"Diagnostics:" + msg, ret))

        if TestfileExecutor.run_valgrind:
            ret = self.run_valgrind_tests(self.testfile.compiler_run_line)
            self._status_line.append(TestfileExecutor.pretty_status(f"Memcheck:", ret))

        self._status_line.append("] ")

    def run_valgrind_tests(self, run_line) -> int:
        valgrind_args = [
            "valgrind",
            "--leak-check=full",
            "--track-origins=yes",
            "--show-leak-kinds=all",
            "--errors-for-leak-kinds=all",
            f"--error-exitcode={TestfileExecutor._valgrind_error_exitcode}",
            "--quiet",
        ]
        exe_args = shlex.split(run_line)

        completed_process = subprocess.run(
            valgrind_args + exe_args, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
        )

        return completed_process.returncode
