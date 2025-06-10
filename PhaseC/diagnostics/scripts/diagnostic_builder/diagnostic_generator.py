import argparse
import os
from encodings.punycode import selective_find
from turtledemo.paint import switchupdown
from typing import NamedTuple
from enum import Enum, auto


class StartupArguments(NamedTuple):
    input_filename: str


class DiagnosticEntry:
    def __init__(self, message: str, args: list[str], location: str):
        self.message = message
        self.args = args
        self.location = location


class Diagnostic:
    class Type(Enum):
        WARNING = auto()
        ERROR = auto()
        FATAL = auto()

    @staticmethod
    def to_diag_type(diag_type: str):
        match diag_type:
            case "WARNING":
                return Diagnostic.Type.WARNING
            case "ERROR":
                return Diagnostic.Type.ERROR
            case "FATAL":
                return Diagnostic.Type.FATAL
            case _:
                raise ValueError("Unknown diagnostic type")

    def __init__(
        self,
        diag_name: str,
        diag_type: str,
        primary: DiagnosticEntry,
        notes: list[DiagnosticEntry]
    ):
        self.diag_name = diag_name
        self.diag_type = diag_type
        self.primary = primary
        self.notes = notes


def assert_is_yaml_file(input_file: str):
    if not input_file.endswith(".yaml"):
        raise ValueError(f"`{input_file}` is not a yaml file. Expected a file ending with .yaml")
    if not os.path.isfile(input_file):
        raise ValueError(f"`{input_file}` is not a file.")
    if not os.access(input_file, os.R_OK):
        raise ValueError(f"`{input_file}` is not a readable file.")


def parse_startup_arguments() -> StartupArguments:
    parser = argparse.ArgumentParser(description="Inject logging hooks into grammar files.")
    parser.add_argument("--input", required=True, help="Path to the input diagnostics .yaml file")
    args = parser.parse_args()

    assert_is_yaml_file(args.input)

    return StartupArguments(
        input_filename=args.input
    )


def load_diagnostics_file(startup_args: StartupArguments) -> list[str]:
    with open(startup_args.input_filename, 'r') as fin:
        return fin.readlines()


def generator_manager():
    pass


def parse_diagnostics_file():
    pass


def load_diagnostics(yaml_lines: list[str]):
    for line in yaml_lines:
        if line.isspace():
            continue
        # if line.startswith("- Diagnostic: "):


class PrimaryFSM:
    class State(Enum):
        EXPECT_MESSAGE = auto()
        EXPECT_ARGS = auto()
        EXPECT_LOCATION = auto()

    message: str
    args: list[str]
    location: str

    def __init__(self, current_line: int):
        self.state = self.State.EXPECT_MESSAGE
        self.current_line = current_line

    def parse_primary_fsm(self, line: str):
        if self.state == self.State.EXPECT_MESSAGE:
            if not line.startswith("    Message: "):
                raise RuntimeError(f"In `Primary`, line {self.current_line}: expected to start with `    Message: `")
            message_string = line.split(':', 1)[1].strip()  # We strip prefix and suffix spaces.
            if not (message_string.startswith('"') and message_string.endswith('"')):
                raise RuntimeError(f"In `Primary`, line {self.current_line}, message not enclosed in double quotes")
            self.message = message_string[1:-1]  # We strip message from double quotes.
            self.state = self.State.EXPECT_ARGS
            return
        if self.state == self.State.EXPECT_ARGS:
            if not line.startswith("      Args: "):
                raise RuntimeError(f"In `Primary`, line {self.current_line} expected to start with `      Args: `")
            arg_list_line = line.split(':', 1)[1].strip()  # We strip prefix and suffix spaces.
            if not (arg_list_line.startswith('[') and arg_list_line.endswith(']')):
                raise RuntimeError(f"In `Primary`, line {self.current_line} expected Args enclosed in [] (brackets)")
            self.args.extend([arg.strip() for arg in arg_list_line[1:-1].split(',')])
            self.state = self.State.EXPECT_LOCATION
            return
        if self.state == self.State.EXPECT_LOCATION:
            if not line.startswith("    Location: "):
                raise RuntimeError(f"In `Primary`, line {self.current_line} expected to start with `    Location: `")
            location_string = line.split(':', 1)[1].strip()  # We strip prefix and suffix spaces.
            if not (location_string.startswith('"') and location_string.endswith('"')):
                raise RuntimeError(f"In `Primary`, line {self.current_line}, location not enclosed in double quotes")



class DiagnosticFSM:
    class State(Enum):
        EXPECT_DIAGNOSTIC = auto()
        EXPECT_TYPE = auto()
        EXPECT_PRIMARY = auto()
        EXPECT_NOTES = auto()

    diag_name: str
    diag_type: Diagnostic.Type

    def __init__(self):
        self.state = self.State.EXPECT_DIAGNOSTIC
        self.current_line = 1

    def parse_diagnostic_fsm(self, line: str):
        if self.state == self.State.EXPECT_DIAGNOSTIC:
            if line.startswith("- Diagnostic: "):
                self.diag_name = line.split(':', 1)[1].strip()
                self.state = self.State.EXPECT_TYPE
                return
            raise RuntimeError(f"Expected line {self.current_line} to start with  `- Diagnostic: ` ")
        if self.state == self.State.EXPECT_TYPE:
            if line.startswith("  Type: "):
                self.diag_type = Diagnostic.to_diag_type(line.split(':', 1)[1].strip())
                self.state = self.State.EXPECT_PRIMARY
                return
            raise RuntimeError(f"Expected line {self.current_line} to start with `  Type: `")
        if self.state == self.State.EXPECT_PRIMARY:
            if line.startswith("  Primary:"):
                self.state =
            raise RuntimeError("")

        self.current_line += 1
