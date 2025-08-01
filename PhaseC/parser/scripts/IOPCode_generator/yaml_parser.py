import argparse
import os
import inspect
import re
from dataclasses import dataclass, field
from enum import Enum
from pathlib import Path
from typing import NamedTuple, TextIO, Set, Optional


@dataclass
class IOPCodeInfo:
    arg_count: int = 0
    uses_label: bool = False
    optimizations: Set[str] = field(default_factory=set)


_CPP_ID = r"[_a-zA-Z][_a-zA-Z0-9]*"
_CPP_ID_LIST = rf"(?:\s*{_CPP_ID}\s*)(?:,\s*{_CPP_ID}\s*)*"

_line = 0


def attach_context(error_message: str) -> str:
    frame = inspect.currentframe()
    filename = inspect.getfile(frame)
    lineno = frame.f_lineno
    return f"{filename}:{lineno}: {error_message}"


class YamlParser:
    INDENT = " " * 2

    class IndentLevel(Enum):
        AVAILABLE_OPTIMIZATIONS = 0
        IOPCODES_DECL_LABEL = 0
        IOPC_DECL = 1
        IOPC_DEF = 2

    def __init__(self, input_filepath: Path, available_opts_label: str, iopcodes_label: str):
        self._parse_lines_buffer: Optional[list[str]] = None
        self._current_line_idx: Optional[int] = None
        self._input_filename = None
        self._available_opts_label = available_opts_label
        self._iopcodes_label = iopcodes_label
        self._available_optimization_set: set[str] = set()
        self._iopcode_dict: dict[str, IOPCodeInfo] = dict()

        YamlParser._validate_input_file(input_filepath)
        self._load_file_in_parse_buffer(input_filepath)

    @staticmethod
    def _validate_input_file(filepath: Path) -> None:
        if not os.path.isfile(filepath):
            raise FileNotFoundError(attach_context(f"{os.path.basename(filepath)}: is not a file"))
        if not os.access(filepath, os.R_OK):
            raise PermissionError(attach_context(f"{os.path.basename(filepath)}: is not readable"))

    def _load_file_in_parse_buffer(self, filepath: Path):
        with open(filepath, "r") as fin:
            self._input_filename = os.path.basename(filepath)
            self._parse_lines_buffer = fin.readlines()
            self._current_line_idx = 0

    @property
    def _lineno(self):
        if self._current_line_idx is None:
            return None
        return self._current_line_idx + 1

    @property
    def _current_line(self):
        if self._current_line_idx < len(self._parse_lines_buffer):
            return self._parse_lines_buffer[self._current_line_idx]
        return None  # EOF

    def _consume_current_line(self):
        if self._current_line is None:
            return None
        line_buffer = self._current_line
        self._current_line_idx += 1
        return line_buffer

    def _skip_non_code_lines(self):
        while self._current_line:
            if self._current_line.isspace():
                self._consume_current_line()
                continue
            if self._current_line.lstrip().startswith("#"):
                self._consume_current_line()
                continue
            return

    @staticmethod
    def _expected_indent(level: IndentLevel) -> str:
        return YamlParser.INDENT * level.value

    def _parse_available_optimizations(self):
        """Parses a line like: `AVAILABLE_OPTIMIZATIONS: [opt1, opt2, opt3]`"""

        self._skip_non_code_lines()

        code_line = self._consume_current_line()

        if code_line is None:
            raise RuntimeError(attach_context(
                f"[{self._input_filename}]: EOF reached before finding field with available optimizations"))

        expected_indent = YamlParser._expected_indent(
            YamlParser.IndentLevel.AVAILABLE_OPTIMIZATIONS)

        avail_opts_pattern = rf"{self._available_opts_label}\s*:\s*\[({_CPP_ID_LIST})\]\s*"

        if match := re.fullmatch(rf"{expected_indent}{avail_opts_pattern}", code_line):
            self._available_optimization_set = set(
                [opt.strip() for opt in match.group(1).split(",")])
            return
        if re.fullmatch(rf"{avail_opts_pattern}", code_line):
            raise RuntimeError(attach_context(
                f"[{self._input_filename}:{self._current_line}]: "
                f"`{self._available_opts_label}` is wrongly indented."
                f"Expected {YamlParser.IndentLevel.AVAILABLE_OPTIMIZATIONS} indentation(s)"))
        raise RuntimeError(attach_context(
            f"[{self._input_filename}:{self._current_line}]: "
            f"Expected line with {self._available_opts_label}"))

    def _parse_iopcodes_label(self):
        self._skip_non_code_lines()

        code_line = self._consume_current_line()

        if code_line is None:
            raise RuntimeError(attach_context(
                f"[{self._input_filename}]: EOF reached before finding field with IOPCodes"))

        expected_indent = YamlParser._expected_indent(YamlParser.IndentLevel.IOPCODES_DECL_LABEL)
        iopcodes_decl_pattern = rf"{self._iopcodes_label}\s*:\s*"

        if re.fullmatch(f"{expected_indent}{iopcodes_decl_pattern}", code_line):
            return
        if re.match(f"{iopcodes_decl_pattern}", code_line):
            raise RuntimeError(attach_context(
                f"[{self._input_filename}:{self._current_line}]: "
                f"`{self._iopcodes_label}` is wrongly indented."
                f"Expected {YamlParser.IndentLevel.AVAILABLE_OPTIMIZATIONS} indentation(s)"))
        raise RuntimeError(attach_context(
            f"[{self._input_filename}:{self._current_line}]: "
            f"Expected line with {self._iopcodes_label}"))

    def _parse_iopcode_name(self):
        self._skip_non_code_lines()
        code_line = self._consume_current_line()
        if code_line is None:
            return None
        expected_indent = YamlParser._expected_indent(YamlParser.IndentLevel.IOPC_DECL)
        iopcode_name_pattern = rf"({_CPP_ID})\s*:\s*"

        if name_match := re.fullmatch(f"{expected_indent}{iopcode_name_pattern}", code_line):
            iopcode_name = name_match.group(1)
            if iopcode_name in self._iopcode_dict:
                raise RuntimeError(attach_context(
                    f"[{self._input_filename}:{self._current_line}]: "
                    f"IOPCode `{iopcode_name} redefined"))
            return iopcode_name
        if name_match := re.match(f"{iopcode_name_pattern}", code_line):
            raise RuntimeError(attach_context(
                f"[{self._input_filename}:{self._current_line}]: "
                f"IOPCode `{name_match.group(1)}` is wrongly indented."
                f"Expected {YamlParser.IndentLevel.IOPC_DECL} indentation(s)"))
        return None

    def _parse_iopcode_arg_count(self):
        self._skip_non_code_lines()
        code_line = self._consume_current_line()
        if code_line is None:
            return None
        expected_indent = YamlParser._expected_indent(YamlParser.IndentLevel.IOPC_DEF)
        iopcode_arg_count_pattern = rf"args\s*:\s*(\d+)\s*"

        if arg_count_match := re.fullmatch(f"{expected_indent}{iopcode_arg_count_pattern}",
                                           code_line):
            return arg_count_match.group(1)
        if arg_count_match := re.match(f"{iopcode_arg_count_pattern}", code_line):
            raise RuntimeError(attach_context(
                f"[{self._input_filename}:{self._current_line}]: "
                f"IOPCode `args: {arg_count_match.group(1)}` is wrongly indented."
                f"Expected {YamlParser.IndentLevel.IOPC_DEF} indentation(s)"))
        return None

    def _parse_iopcode_label(self):
        self._skip_non_code_lines()
        code_line = self._current_line
        if code_line is None:
            return None
        expected_indent = YamlParser._expected_indent(YamlParser.IndentLevel.IOPC_DEF)
        iopcode_label_pattern = rf"label\s*:\s*(?i:(true|false))\s*"
        if arg_count_match := re.fullmatch(f"{expected_indent}{iopcode_label_pattern}", code_line):
            self._consume_current_line()
            return arg_count_match.group(1).lower()
        if arg_count_match := re.match(f"{iopcode_label_pattern}", code_line):
            raise RuntimeError(attach_context(
                f"[{self._input_filename}:{self._current_line}]: "
                f"IOPCode `label: {arg_count_match.group(1)}` is wrongly indented."
                f"Expected {YamlParser.IndentLevel.IOPC_DEF} indentation(s)"))
        return None

    def _parse_iopcode_optimizations(self):
        self._skip_non_code_lines()
        code_line = self._current_line
        if code_line is None:
            return None
        expected_indent = YamlParser._expected_indent(YamlParser.IndentLevel.IOPC_DEF)
        iopcode_opts_pattern = rf"optimizations\s*:\s*\[({_CPP_ID_LIST})\]\s*"
        if label_match := re.fullmatch(f"{expected_indent}{iopcode_opts_pattern}", code_line):
            self._consume_current_line()
            opt_list = [opt.strip() for opt in label_match.group(1).split(",")]
            if len(opt_list) != len(set(opt_list)):
                raise RuntimeError(attach_context(
                    f"[{self._input_filename}:{self._current_line}]: "
                    f"Duplicates found in optimization list"))
            return set(opt_list)

        if label_match := re.match(f"{iopcode_opts_pattern}", code_line):
            raise RuntimeError(attach_context(
                f"[{self._input_filename}:{self._current_line}]: "
                f"IOPCode `optimizations: {label_match.group(1)}` is wrongly indented."
                f"Expected {YamlParser.IndentLevel.IOPC_DEF} indentation(s)"))
        return None

    def _parse_iopcodes(self):
        while self._current_line:
            name = self._parse_iopcode_name()
            arg_count = self._parse_iopcode_arg_count()
            label = True if self._parse_iopcode_label() is not None else False
            optimizations = self._parse_iopcode_optimizations()
            self._iopcode_dict[name] = IOPCodeInfo(arg_count, label, optimizations)

    def run(self) -> tuple[dict[str, IOPCodeInfo], set[str]]:
        """Executes the parser."""
        self._parse_available_optimizations()
        self._parse_iopcodes_label()
        self._parse_iopcodes()
        return self._iopcode_dict, self._available_optimization_set
