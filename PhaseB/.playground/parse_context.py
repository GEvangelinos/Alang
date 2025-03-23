# parse_context.py
import enum


class ParserState(enum.Enum):
        # Valid FSM states:
        BEFORE_GRAMMAR_SECTION = enum.auto()
        INSIDE_GRAMMAR_SECTION = enum.auto()
        AFTER_GRAMMAR_SECTION = enum.auto()

        # Valid exit FSM state:
        FINISHED_LOG_INSERTION = enum.auto()


class ParseContext:
        def __init__(self, logging_function_name: str, hook_name: str) -> None:
                self._logging_function_name = logging_function_name  # private
                self._hook_name = hook_name  # private
                self.brace_depth: int = 0  # We just started reading .y file
                self.line_count: int = 0  # We just started reading .y file (maybe there is nothing).
                self.column_count: int = 0  # We just started reading .y file (maybe there is nothing).
                self.parsed_lines: list[str] = []
                self.current_state = ParserState.BEFORE_GRAMMAR_SECTION  # We just started, we are certainly outside

        @property
        def logging_function_name(self) -> str:
                return self._logging_function_name

        @property
        def hook_name(self) -> str:
                return self._hook_name
