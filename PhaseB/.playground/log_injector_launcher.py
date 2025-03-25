import argparse
import sys
import os
from typing import NamedTuple, TextIO
from parser_context import ParserContext
from fsm_for_lhs_nonterminal import parse_lhs_nonterminal
from fsm_for_rhs_productions import parse_rhs_productions


class StartupArguments(NamedTuple):
        input_filename: str
        output_filename: str
        log_function_name: str


def validate_input_file(file_path: str) -> None:
        if not os.path.isfile(file_path):
                raise FileNotFoundError(f"Input file {os.path.basename(file_path)} is not a file")
        if not os.access(file_path, os.R_OK):
                raise PermissionError(f"Input file {os.path.basename(file_path)} is not readable")


def parse_startup_arguments() -> StartupArguments:
        parser = argparse.ArgumentParser(description="Inject logging hooks into grammar files.")
        parser.add_argument("--input", "-i", required=True, help="Path to the input grammar file")
        parser.add_argument("--output", "-o", required=True, help="Path to the output grammar file")
        parser.add_argument("--log-fn", "-l", required=True, help="Name of the logging function to inject")

        args = parser.parse_args()

        return StartupArguments(
                input_filename=args.input,
                output_filename=args.output,
                log_function_name=args.log_fn,
        )


def write_parsed_stream_to_file(ctx: ParserContext, fout: TextIO) -> None:
        ctx.injectedCharStream.rewind()
        while not ctx.injectedCharStream.eof():
                fout.write(ctx.injectedCharStream.next())


def copy_section_before_grammar(ctx: ParserContext, fin: TextIO) -> None:
        for line in fin:
                ctx.increment_line_counter()
                if "%%" in line and line.strip() != "%%":
                        raise ValueError(f"Line {ctx.line_counter}: Delimiter %% separating prologue and grammar rules"
                                         f" is not in a standalone line, please fix")
                ctx.injectedCharStream.extend(line)
                if "%%" in line:
                        break
        else:
                raise ValueError(f"Line {ctx.line_counter}: Delimiter %% before grammar rules is missing")


def grammar_section_parser(ctx: ParserContext) -> None:
        while not ctx.charStream.eof():
                ctx.lhs_nonterminal = parse_lhs_nonterminal(ctx)
                if ctx.lhs_nonterminal is None:
                        return
                parse_rhs_productions(ctx)


def process_section_inside_grammar(ctx: ParserContext, fin: TextIO) -> None:
        temp_line_counter = ctx.line_counter
        for line in fin:
                temp_line_counter += 1
                if "%%" in line and line.strip() != "%%":
                        raise ValueError(f"Line {temp_line_counter}: Delimiter %% separating epilogue and grammar rules"
                                         f" is not in a standalone line, please fix")
                if "%%" in line:
                        break
                ctx.charStream.extend(line)
        else:
                raise ValueError(f"Line {ctx.line_counter}: Delimiter %% after grammar rules is missing, please fix")
        ctx.increment_line_counter()  # Required as grammar_section_manager() will start from next line.
        grammar_section_parser(ctx)
        ctx.injectedCharStream.extend(line)  # Append line with %% delimiter, as it wasn't appended on stream.


def copy_section_after_grammar(ctx: ParserContext, fin: TextIO) -> None:
        for line in fin:
                ctx.increment_line_counter()
                ctx.injectedCharStream.extend(line)


def process_grammar_file(ctx: ParserContext, fin: TextIO) -> None:
        try:
                copy_section_before_grammar(ctx, fin)
                process_section_inside_grammar(ctx, fin)
                copy_section_after_grammar(ctx, fin)
        except ValueError as e:
                print(f"Caught ValueError: {e}")
        except AssertionError as e:
                print(f"Caught AssertionError: {e}")


def run_parser_pipeline(startup_arguments: StartupArguments) -> None:
        ctx = ParserContext(startup_arguments.log_function_name)

        with open(startup_arguments.input_filename, 'r') as fin:
                process_grammar_file(ctx, fin)

        with open(startup_arguments.output_filename, 'w') as fout:
                write_parsed_stream_to_file(ctx, fout)


def main() -> int:
        try:
                startup_arguments = parse_startup_arguments()
                validate_input_file(startup_arguments.input_filename)
                run_parser_pipeline(startup_arguments)
                return 0
        except ValueError as e:
                print(f"Caught ValueError: {e}")
        except AssertionError as e:
                print(f"Caught AssertionError: {e}")
        except FileNotFoundError as e:
                print(f"Caught FileNotFoundError: {e}")
        except PermissionError as e:
                print(f"Caught PermissionError: {e}")
        return 1


if __name__ == "__main__":
        sys.exit(main())
