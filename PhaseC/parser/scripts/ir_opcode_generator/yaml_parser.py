import argparse
import os
import inspect
import re
from dataclasses import dataclass, field
from enum import Enum
from os import WCONTINUED
from pathlib import Path
from sys import exec_prefix
from typing import NamedTuple, TextIO, Set, Optional

from shared_config import ConfigArguments


@dataclass(frozen=True)
class InfoFieldTypes:
    name: str
    type: str


@dataclass(frozen=True)
class InfoField:
    name: str
    type: str
    value: str


@dataclass
class IROpcodeInfo:
    info_field_set: set[InfoField]
    optimizations: set[str]

@dataclass(frozen=True)
class YamlParserProducts:
    opcode_dict: dict[str, IROpcodeInfo]
    available_opts: set[str]
    available_info_set: set[InfoFieldTypes]




_CPP_ID = r"[_a-zA-Z][_a-zA-Z0-9]*"
_CPP_ID_LIST = rf"(?:\s*{_CPP_ID}\s*)(?:,\s*{_CPP_ID}\s*)*"

_line = 0


def attach_context(error_message: str) -> str:
    frame = inspect.currentframe().f_back
    filename = inspect.getfile(frame)
    lineno = frame.f_lineno
    return f"{filename}:{lineno}: {error_message}"


class YamlParser:
    INDENT = " " * 2
    OPTIMIZATIONS_FIELD_NAME = "optimizations"

    class IndentLevel(Enum):
        IROPCODE_INFO_LABEL = 0
        IROPCODE_INFO_FIELD = 1
        IROPCODE_OPTIMIZATION_FLAGS_LABEL = 0
        IROPCODE_LISTING_LABEL = 0
        IROPCODE_DECL = 1
        IROPCODE_DEF = 2

    def __init__(
            self,
            config_args: ConfigArguments,
            iropcode_info_label: str,
            iropcode_opt_flags_label: str,
            iropcode_listing_label: str,
    ):
        self._parse_lines_buffer: Optional[list[str]] = None
        self._current_line_idx: Optional[int] = None
        self._ir_spec_filename = None
        self._iropcode_info_label = iropcode_info_label
        self._iropcode_opt_flags_label = iropcode_opt_flags_label
        self._iropcode_listing_label = iropcode_listing_label
        self._available_optimization_set: set[str] = set()
        self._available_info_field_set: set[InfoFieldTypes] = set()
        self._irop_dict: dict[str, IROpcodeInfo] = dict()

        YamlParser._validate_input_file(config_args.in_ir_spec_filepath)
        self._load_file_in_parse_buffer(config_args.in_ir_spec_filepath)

    @staticmethod
    def _validate_input_file(filepath: Path) -> None:
        if not os.path.isfile(filepath):
            raise FileNotFoundError(attach_context(f"{os.path.basename(filepath)}: is not a file"))
        if not os.access(filepath, os.R_OK):
            raise PermissionError(attach_context(f"{os.path.basename(filepath)}: is not readable"))

    def _load_file_in_parse_buffer(self, filepath: Path):
        with open(filepath, "r") as fin:
            self._ir_spec_filename = os.path.basename(filepath)
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

    def _parse_iropcode_info_label_list(self):
        self._skip_non_code_lines()
        code_line = self._consume_current_line()

        if code_line is None:
            raise RuntimeError(attach_context(
                f"[{self._ir_spec_filename}]: EOF reached before finding field with IROPCODE's INFO_LIST"))

        expected_indent = YamlParser._expected_indent(YamlParser.IndentLevel.IROPCODE_INFO_LABEL)
        iropcode_info_label_pattern = rf"{self._iropcode_info_label}\s*:\s*(?:#.*)?\n?"

        if match := re.fullmatch(rf"{expected_indent}{iropcode_info_label_pattern}", code_line):
            return
        if re.match(f"{iropcode_info_label_pattern}", code_line):
            raise RuntimeError(attach_context(
                f"[{self._ir_spec_filename}:{self._current_line}]: "
                f"`{self._iropcode_info_label}` is wrongly indented."
                f"Expected {YamlParser.IndentLevel.IROPCODE_INFO_LABEL} indentation(s)"))
        raise RuntimeError(attach_context(
            f"[{self._ir_spec_filename}:{self._current_line}]: "
            f"Expected line with {self._iropcode_info_label}"))

    def _parse_legal_iropcode_info_fields(self):
        if self._current_line is None:
            raise RuntimeError(attach_context(
                f"[{self._ir_spec_filename}]: EOF reached whiles looking for available info fields of opcodes"))

        expected_indent = YamlParser._expected_indent(YamlParser.IndentLevel.IROPCODE_INFO_FIELD)
        info_field_pattern = rf"({_CPP_ID})\s*:\s*(int|bool|str)\s*(?:#.*)?\n?"
        while True:
            self._skip_non_code_lines()
            code_line = self._current_line
            if code_line is None:
                return
            if match := re.fullmatch(rf"{expected_indent}{info_field_pattern}", code_line):
                current_info_field = InfoFieldTypes(match.group(1), match.group(2))
                if current_info_field in self._available_info_field_set:
                    raise RuntimeError(attach_context(
                        f"[{self._ir_spec_filename}]: InfoField: {current_info_field}, is a duplicate"))
                self._available_info_field_set.add(current_info_field)
                self._consume_current_line()
                continue
            if re.fullmatch(rf"{info_field_pattern}", code_line):
                raise RuntimeError(attach_context(
                    f"[{self._ir_spec_filename}:{self._current_line}]: "
                    f"info-field is wrongly indented."
                    f"Expected {YamlParser.IndentLevel.IROPCODE_INFO_FIELD} indentation(s)"))
            break

    def _parse_optimizations_flags(self):
        """Parses a line like: `AVAILABLE_OPTIMIZATIONS: [opt1, opt2, opt3]`"""

        self._skip_non_code_lines()

        code_line = self._consume_current_line()

        if code_line is None:
            raise RuntimeError(attach_context(
                f"[{self._ir_spec_filename}]: EOF reached before finding field with available optimizations"))

        expected_indent = YamlParser._expected_indent(
            YamlParser.IndentLevel.IROPCODE_OPTIMIZATION_FLAGS_LABEL)

        opt_flags_pattern = rf"{self._iropcode_opt_flags_label}\s*:\s*\[({_CPP_ID_LIST})\]\s*(?:#.*)?\n?"

        if match := re.fullmatch(rf"{expected_indent}{opt_flags_pattern}", code_line):
            self._available_optimization_set = set(
                [opt.strip() for opt in match.group(1).split(",")])
            return
        if re.fullmatch(rf"{opt_flags_pattern}", code_line):
            raise RuntimeError(attach_context(
                f"[{self._ir_spec_filename}:{self._current_line}]: "
                f"`{self._iropcode_opt_flags_label}` is wrongly indented."
                f"Expected {YamlParser.IndentLevel.IROPCODE_OPTIMIZATION_FLAGS_LABEL} indentation(s)"))
        raise RuntimeError(attach_context(
            f"[{self._ir_spec_filename}:{self._current_line}]: "
            f"Expected line with {self._iropcode_opt_flags_label}"))

    def _parse_iropcode_listing_label(self):
        self._skip_non_code_lines()

        code_line = self._consume_current_line()

        if code_line is None:
            raise RuntimeError(attach_context(
                f"[{self._ir_spec_filename}]: EOF reached before finding field with IROps"))

        expected_indent = YamlParser._expected_indent(YamlParser.IndentLevel.IROPCODE_LISTING_LABEL)
        iropcode_listing_pattern = rf"{self._iropcode_listing_label}\s*:\s*(?:#.*)?\n?"

        if re.fullmatch(f"{expected_indent}{iropcode_listing_pattern}", code_line):
            return
        if re.match(f"{iropcode_listing_pattern}", code_line):
            raise RuntimeError(attach_context(
                f"[{self._ir_spec_filename}:{self._current_line}]: "
                f"`{self._iropcode_listing_label}` is wrongly indented."
                f"Expected {YamlParser.IndentLevel.IROPCODE_OPTIMIZATION_FLAGS_LABEL} indentation(s)"))
        raise RuntimeError(attach_context(
            f"[{self._ir_spec_filename}:{self._current_line}]: "
            f"Expected line with {self._irops_label}"))

    def _parse_iropcode_name(self):
        self._skip_non_code_lines()
        code_line = self._consume_current_line()
        if code_line is None:
            return None
        expected_indent = YamlParser._expected_indent(YamlParser.IndentLevel.IROPCODE_DECL)
        iropcode_name_pattern = rf"({_CPP_ID})\s*:\s*(?:#.*)?\n?"

        if name_match := re.fullmatch(f"{expected_indent}{iropcode_name_pattern}", code_line):
            iropcode_name = name_match.group(1)
            if iropcode_name in self._irop_dict:
                raise RuntimeError(attach_context(
                    f"[{self._ir_spec_filename}:{self._current_line}]: "
                    f"Iropocode `{iropcode_name} redefined"))
            return iropcode_name
        if name_match := re.match(f"{iropcode_name_pattern}", code_line):
            raise RuntimeError(attach_context(
                f"[{self._ir_spec_filename}:{self._current_line}]: "
                f"Iropcode `{name_match.group(1)}` is wrongly indented."
                f"Expected {YamlParser.IndentLevel.IROP_DECL} indentation(s)"))
        return None

    def _parse_irop_arg_count(self):
        self._skip_non_code_lines()
        code_line = self._consume_current_line()
        if code_line is None:
            return None
        expected_indent = YamlParser._expected_indent(YamlParser.IndentLevel.IROPCODE_DEF)
        iropcode_arg_count_pattern = rf"args\s*:\s*(\d+)\s*(?:#.*)?\n?"

        if arg_count_match := re.fullmatch(f"{expected_indent}{iropcode_arg_count_pattern}",
                                           code_line):
            return arg_count_match.group(1)
        if arg_count_match := re.match(f"{iropcode_arg_count_pattern}", code_line):
            raise RuntimeError(attach_context(
                f"[{self._ir_spec_filename}:{self._current_line}]: "
                f"Iropcode `args: {arg_count_match.group(1)}` is wrongly indented."
                f"Expected {YamlParser.IndentLevel.IOPC_DEF} indentation(s)"))
        return None

    def _parse_irop_jumps(self):
        self._skip_non_code_lines()
        code_line = self._current_line
        if code_line is None:
            return None
        expected_indent = YamlParser._expected_indent(YamlParser.IndentLevel.IROPCODE_DEF)
        iropcode_jumps_pattern = rf"jumps\s*:\s*(?i:(true|false))\s*(?:#.*)?\n?"
        if jumps_match := re.fullmatch(f"{expected_indent}{iropcode_jumps_pattern}", code_line):
            self._consume_current_line()
            return jumps_match.group(1).lower()
        if jumps_match := re.match(f"{iropcode_jumps_pattern}", code_line):
            raise RuntimeError(attach_context(
                f"[{self._ir_spec_filename}:{self._current_line}]: "
                f"Iropcode `label: {jumps_match.group(1)}` is wrongly indented."
                f"Expected {YamlParser.IndentLevel.IROP_DEF} indentation(s)"))
        return None

    def _parse_irop_optimizations(self):
        self._skip_non_code_lines()
        code_line = self._current_line
        if code_line is None:
            return None
        expected_indent = YamlParser._expected_indent(YamlParser.IndentLevel.IROPCODE_DEF)
        iropcode_opts_pattern = rf"optimizations\s*:\s*\[({_CPP_ID_LIST})\]\s*(?:#.*)?\n?"
        if label_match := re.fullmatch(f"{expected_indent}{iropcode_opts_pattern}", code_line):
            self._consume_current_line()
            opt_list = [opt.strip() for opt in label_match.group(1).split(",")]
            if len(opt_list) != len(set(opt_list)):
                raise RuntimeError(attach_context(
                    f"[{self._ir_spec_filename}:{self._current_line}]: "
                    f"Duplicates found in optimization list"))
            return set(opt_list)

        if label_match := re.match(f"{iropcode_opts_pattern}", code_line):
            raise RuntimeError(attach_context(
                f"[{self._ir_spec_filename}:{self._current_line}]: "
                f"IROp `optimizations: {label_match.group(1)}` is wrongly indented."
                f"Expected {YamlParser.IndentLevel.IROPCODE_DEF} indentation(s)"))
        return None

    def _parse_irop_info_fields(self) -> set[InfoField]:
        info_field_set: set[InfoField] = set()

        expected_indent = YamlParser._expected_indent(YamlParser.IndentLevel.IROPCODE_DEF)
        string_pattern = r'"(?:[^"\\]|\\.)*"'
        bool_pattern = r"\btrue\b|\bfalse\b"
        int_pattern = r"[-+]?\d+"
        info_field_name_pattern = rf"({_CPP_ID})"
        info_field_pattern = rf"{info_field_name_pattern}\s*:\s*(?:(?:{bool_pattern})|(?:{int_pattern})|(?:{string_pattern}))\s*(?:#.*)?\n?"
        while True:
            self._skip_non_code_lines()
            code_line = self._current_line
            if code_line is None:
                break
            if match := re.fullmatch(rf"^{expected_indent}{info_field_pattern}", code_line):
                if match.group(1) == YamlParser.OPTIMIZATIONS_FIELD_NAME:
                    break

                matched_field = next(
                    (info_field for info_field in self._available_info_field_set
                     if info_field.name == match.group(1)),
                    None
                )
                if matched_field is None:
                    raise RuntimeError(
                        f"Field: {match.group(1)} is not declared under {self._iropcode_info_label}"
                    )
                if matched_field.type == "bool":
                    info_field_content_pattern = rf"({bool_pattern})"
                elif matched_field.type == "int":
                    info_field_content_pattern = rf"({int_pattern})"
                elif matched_field.type == "str":
                    info_field_content_pattern = rf"({string_pattern})"
                else:
                    raise RuntimeError(attach_context(
                        f"[{self._ir_spec_filename}:{self._current_line}]: unknown info_field type"))
                fpattern = rf"{expected_indent}{info_field_name_pattern}\s*:\s*{info_field_content_pattern}\s*(?:#.*)?\n?"
                if match := re.fullmatch(fpattern, code_line):
                    info_field_set.add(InfoField(match.group(1), matched_field.type, match.group(2)))
                    self._consume_current_line()
                    continue
                else:
                    raise RuntimeError(attach_context(
                        f"[{self._ir_spec_filename}:{self._current_line}]: info-field mismatch"))
            elif match := re.fullmatch(rf"^\s*{info_field_pattern}", code_line):
                raise RuntimeError(attach_context(
                    f"[{self._ir_spec_filename}:{self._current_line}]: "
                    f"Iropcode `{match.group(1)}` is wrongly indented."
                    f"Expected {YamlParser.IndentLevel.IROPCODE_DEF} indentation(s)"))
            break
        return info_field_set

    def _parse_irops(self):
        while self._current_line:
            name = self._parse_iropcode_name()
            info_field_set = self._parse_irop_info_fields()
            optimizations = self._parse_irop_optimizations()
            self._irop_dict[name] = IROpcodeInfo(info_field_set, optimizations)

    def run(self) -> YamlParserProducts:
        """Executes the parser."""
        self._parse_iropcode_info_label_list()
        self._parse_legal_iropcode_info_fields()
        self._parse_optimizations_flags()
        self._parse_iropcode_listing_label()
        self._parse_irops()
        return YamlParserProducts(self._irop_dict, self._available_optimization_set, self._available_info_field_set)
