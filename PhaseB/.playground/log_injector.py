import sys
import os
import enum
from typing import NamedTuple, TextIO
import re


class StartupArguments(NamedTuple):
        input_file_name: str
        output_file_name: str
        logging_function_name: str
        hook_name: str


class ParserState(enum.Enum):
        # Valid FSM states:
        BEFORE_GRAMMAR_SECTION = enum.auto()
        INSIDE_GRAMMAR_SECTION = enum.auto()
        AFTER_GRAMMAR_SECTION = enum.auto()
        PARSING_RULE_BODY = enum.auto()
        FINISHED_LOG_INSERTION = enum.auto()

        # Invalid FSM states:
        ERROR_GRAMMAR_DELIMITER_MISSING = enum.auto()
        ERROR_GRAMMAR_DELIMITER_NOT_STANDALONE = enum.auto()


class ParseContext:
        def __init__(self, logging_function_name: str, hook_name: str) -> None:
                self._logging_function_name = logging_function_name  # private
                self._hook_name = hook_name  # private
                self.brace_depth: int = 0  # We just started reading .y file
                self.parsed_lines: list[str] = []
                self.current_state = ParserState.BEFORE_GRAMMAR_SECTION  # We just started, we are certainly outside

        @property
        def logging_function_name(self) -> str:
                return self._logging_function_name

        @property
        def hook_name(self) -> str:
                return self._hook_name


def are_valid_startup_arguments() -> bool:
        if len(sys.argv) < 5:
                print(f"Usage of {os.path.basename(__file__)}: <input_file> <output_file> <log_function_name> <hook_name>",
                      file=sys.stderr)
                return False
        if not os.path.isfile(sys.argv[1]):
                print(f"Input file {sys.argv[1]} is not a file", file=sys.stderr)
                return False
        if not os.access(sys.argv[1], os.R_OK):
                print(f"Input file {sys.argv[1]} is not readable", file=sys.stderr)
        print("Startup arguments validated")
        return True


def load_startup_arguments() -> StartupArguments:
        if not are_valid_startup_arguments():
                sys.exit(1)

        return StartupArguments(
                input_file_name=sys.argv[1],
                output_file_name=sys.argv[2],
                logging_function_name=sys.argv[3],
                hook_name=sys.argv[4]
        )


def execute_state_before_grammar_section(ctx: ParseContext, fin: TextIO) -> ParserState:
        define_pattern = re.compile(rf"^\s*#define\s+{re.escape(ctx.hook_name)}\b")
        marker_before_grammar_rules = re.compile(r"^\s*%%")

        for line in fin:
                if define_pattern.match(line):  # Exclude the definition of the hook
                        continue
                if "%%" in line and line.strip() != "%%":
                        return ParserState.ERROR_GRAMMAR_DELIMITER_NOT_STANDALONE
                ctx.parsed_lines.append(line)
                if marker_before_grammar_rules.match(line):
                        break
        else:
                return ParserState.ERROR_GRAMMAR_DELIMITER_MISSING
        return ParserState.INSIDE_GRAMMAR_SECTION


def execute_state_inside_grammar_section(ctx: ParseContext, fin: TextIO) -> ParserState:
        header_rule_pattern = re.compile(r'^(\s*)([a-zA-Z_][a-zA-Z0-9_]*)(\s*):')

        current_rule_header: str = ""
        for line in fin:
                header_match = header_rule_pattern.match(line)
                if header_match:
                        current_rule_header = header_match.group(2) # Second group, enclosed in () contains the rule name.






def parser_fsm(ctx: ParseContext, fin: TextIO) -> None:
        while True:
                match ctx.current_state:
                        case ParserState.BEFORE_GRAMMAR_SECTION:
                                ctx.current_state = execute_state_before_grammar_section(ctx, fin)
                        case ParserState.INSIDE_GRAMMAR_SECTION:
                                return
                        case ParserState.AFTER_GRAMMAR_SECTION:
                                return
                        case ParserState.PARSING_RULE_BODY:
                                return
                        case ParserState.FINISHED_LOG_INSERTION:
                                print("Log insertion completed")
                                return
                        case ParserState.ERROR_GRAMMAR_DELIMITER_NOT_STANDALONE:
                                print("Delimiter %% separating grammar rules is not in a standalone line, please fix",
                                      file=sys.stderr)
                                return
                        case ParserState.ERROR_GRAMMAR_DELIMITER_MISSING:
                                print("Delimiter %% separating grammar rules is not found", file=sys.stderr)
                                return
                        case _:
                                print("Parsing FSM entered unknown state", file=sys.stderr)
                                sys.exit(2)


def parser_manager(startup_arguments: StartupArguments) -> None:
        ctx = ParseContext(startup_arguments.logging_function_name, startup_arguments.hook_name)

        with open(startup_arguments.input_file_name, 'r') as fin:
                parser_fsm(ctx=ctx, fin=fin)

        if ctx.current_state != ParserState.FINISHED_LOG_INSERTION:
                sys.exit(1)

        # If control reaches here, it means injections are completed, and we can write the modified file.
        with open(startup_arguments.output_file_name, 'w') as fout:
                for line in ctx.parsed_lines:
                        fout.write(line)


def main() -> int:
        startup_arguments = load_startup_arguments()
        parser_manager(startup_arguments)
        return 0


if __name__ == "__main__":
        sys.exit(main())
