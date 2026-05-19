import re
import sys
from enum import Enum, auto
from typing import Optional

MacroName = str
MacroContent = list[str]


class MacroPreprocessor:
    _COMMENT_TOKEN = "#"
    _MACRO_TOKEN = "@macro"
    _USE_TOKEN = "@use"
    _MACRO_NAME_PATTERN = r"[a-zA-Z_][a-zA-Z0-9_]*"

    _MACRO_HEADER_PATTERN = re.compile(
        rf"^{_COMMENT_TOKEN}{_MACRO_TOKEN}\s*({_MACRO_NAME_PATTERN})\s*$"
    )

    _USE_PATTERN = re.compile(
        rf"^(\s*){_COMMENT_TOKEN}\s*{_USE_TOKEN}\s*({_MACRO_NAME_PATTERN})\s*$"
    )

    class _ScanStates(Enum):
        SCANNING = auto()
        SCANNED_MACRO_TOKEN = auto()
        SCANNED_USE_TOKEN = auto()

    def __init__(self, source_lines: list[str]):
        self._source_lines = source_lines
        self._line_idx = 0
        self._out_lines: list[str] = []
        self._scanned_macros: dict[MacroName, MacroContent] = {}
        self._scanner_state: MacroPreprocessor._ScanStates = self.__class__._ScanStates.SCANNING

    def run(self) -> list[str]:
        try:
            while self._line_idx < len(self._source_lines):
                line = self._source_lines[self._line_idx].rstrip()
                if self.__class__._USE_TOKEN in line:
                    self._handle_use_token()
                elif re.match(self.__class__._MACRO_HEADER_PATTERN, line):
                    self._handle_macro_token()
                else:
                    self._out_lines.append(line)
                self._line_idx += 1
        except Exception as e:
            print(f"Line: {self._line_idx} error occurred: {e}")
            sys.exit(1)

        return self._out_lines

    def _handle_macro_token(self):
        curr_line = self._source_lines[self._line_idx].rstrip()
        assert self.__class__._MACRO_TOKEN in curr_line

        header_match = re.fullmatch(self.__class__._MACRO_HEADER_PATTERN, curr_line)
        if not header_match:
            raise ValueError(
                f"Invalid {self.__class__._MACRO_HEADER_PATTERN} directive — expected "
                f"'{self.__class__._COMMENT_TOKEN}{self.__class__._MACRO_HEADER_PATTERN} <MACRO_NAME>'"
            )
        self._line_idx += 1  # Consume line of @macro token

        macro_name = header_match.group(1)
        macro_text: list[str] = []
        while True:
            if self._line_idx >= len(self._source_lines): # EOF reached
                self._line_idx -= 1 # Backtrack 1 line, in order to stay in valid line index range
                break
            curr_line = self._source_lines[self._line_idx].rstrip()
            if curr_line.lstrip().startswith(self.__class__._COMMENT_TOKEN):
                if not curr_line.startswith(self.__class__._COMMENT_TOKEN):
                    raise ValueError("Macro does not start at column 0, its indented.")
            if not curr_line.startswith(self.__class__._COMMENT_TOKEN):
                self._line_idx -= 1 # Backtrack 1 line
                break  # End of macro (comment section finished)

            if self.__class__._MACRO_TOKEN in curr_line: # New macro begins
                self._line_idx -= 1 # Backtrack 1 line
                break
            if self.__class__._USE_TOKEN in curr_line:
                self._handle_use_token(macro_text)
            else:
                curr_line = curr_line.removeprefix(self.__class__._COMMENT_TOKEN)
                macro_text.append(curr_line)
                self._line_idx += 1  # Move to next line.


        if not macro_text:
            raise ValueError(
                f"Macro '{macro_name}' has no content — expected at least one line "
                f"after its {self.__class__._MACRO_TOKEN} definition"
            )

        if macro_name in self._scanned_macros:
            raise ValueError(f"Redefinition of macro '{macro_name}'")

        self._scanned_macros[macro_name] = macro_text

    def _handle_use_token(self, target_buffer: Optional[list[str]] = None):
        curr_line = self._source_lines[self._line_idx].rstrip()
        assert self.__class__._USE_TOKEN in curr_line

        use_match = re.fullmatch(self.__class__._USE_PATTERN, curr_line)
        if not use_match:
            raise ValueError(
                f"Invalid {self.__class__._USE_TOKEN} directive — expected "
                f"'{self.__class__._USE_TOKEN} <MACRO_NAME>'"
            )

        indentation_spaces = use_match.group(1)
        macro_name = use_match.group(2)
        if macro_name not in self._scanned_macros:
            raise ValueError(f"Undefined macro '{macro_name}'")

        for macro_line in self._scanned_macros[macro_name]:
            indented_macro_line = indentation_spaces + macro_line
            if target_buffer is not None:
                target_buffer.append(indented_macro_line)
            else:
                self._out_lines.append(indented_macro_line)
