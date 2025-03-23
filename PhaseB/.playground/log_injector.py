import sys
import os
import re
from typing import NamedTuple, TextIO
from parse_context import ParseContext, ParserState


class StartupArguments(NamedTuple):
        input_file_name: str
        output_file_name: str
        logging_function_name: str
        hook_name: str


def are_valid_startup_arguments() -> bool:
        if len(sys.argv) < 5:
                print(f"Usage of {os.path.basename(__file__)}: "
                      f"<input_file> <output_file> <log_function_name> <hook_name>", file=sys.stderr)
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

        for line in fin:
                ctx.line_count += 1
                if define_pattern.match(line):  # Exclude the definition of the hook
                        continue
                if "%%" in line and line.strip() != "%%":
                        raise ValueError(f"Line {ctx.line_count}: Delimiter %% separating prologue and grammar rules"
                                         f" is not in a standalone line, please fix")
                ctx.parsed_lines.append(line)
                break
        else:
                raise ValueError(f"Line {ctx.line_count}: Delimiter %% before grammar rules is missing")
        return ParserState.INSIDE_GRAMMAR_SECTION


def execute_state_inside_grammar_section(ctx: ParseContext, fin: TextIO) -> ParserState:
        char_stream: list[str] = []
        for line in fin:
                if "%%" in line and line.strip() != "%%":
                        raise ValueError(f"Line {ctx.line_count}: Delimiter %% separating epilogue and grammar rules"
                                         f" is not in a standalone line, please fix")
                if "%%" in line:
                        break
                char_stream.extend(line)
        else:
                raise ValueError(f"Line {ctx.line_count}: Delimiter %% after grammar rules is missing, please fix")

        # If control reaches here, we exited for-loop with break.
        # We need to run the FSMs for parsing the grammar rules.
        # Basically we need to call the manager for grammar section.

        # When control reaches here, it means we have parsed all the grammar rules.
        ctx.parsed_lines.append(line) # TODO: We append this line after we append the INJECTED GRAMMAR RULES
        return ParserState.AFTER_GRAMMAR_SECTION


def parser_fsm(ctx: ParseContext, fin: TextIO) -> None:
        while True:
                match ctx.current_state:
                        case ParserState.BEFORE_GRAMMAR_SECTION:
                                ctx.current_state = execute_state_before_grammar_section(ctx, fin)
                        case ParserState.INSIDE_GRAMMAR_SECTION:
                                return
                        case ParserState.AFTER_GRAMMAR_SECTION:
                                return
                        case ParserState.FINISHED_LOG_INSERTION:
                                print("Log insertion completed")
                                return
                        case _:
                                raise AssertionError(f"parser_fsm() enter unknown state, this should never happen")


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
