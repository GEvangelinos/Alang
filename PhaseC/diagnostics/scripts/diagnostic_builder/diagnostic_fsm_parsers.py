import sys
from enum import Enum, auto
import re


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
        diag_type: Type,
        diag_primary: DiagnosticEntry,
        diag_notes: list[DiagnosticEntry]
    ):
        self.diag_name = diag_name
        self.diag_type = diag_type
        self.diag_primary = diag_primary
        self.diag_notes = diag_notes


class DiagnosticEntryFSM:
    class State(Enum):
        EXPECT_MESSAGE = auto()
        EXPECT_ARGS = auto()
        EXPECT_LOCATION = auto()
        DONE = auto()

    state: State
    yaml_lines: list[str]
    current_linenum: int

    message: str
    expected_args: int
    args: list[str]
    location: str

    @staticmethod
    def count_expected_args(message: str) -> int:
        # Matches unescaped FMT::format-style numbered placeholders like {0}, {1}, etc.,
        # but skips escaped ones like {{0}} or {{name}}
        placeholder_pattern = re.compile(r"(?<!\{)\{(\d+)\}(?!\})")
        placeholder_list = re.findall(placeholder_pattern, message)
        for placeholder in placeholder_list:
            print(f"Placeholder #{placeholder}\n")
        print("EXITING FINDME DELETE ME")
        sys.exit(1)

    def handle_expect_message(self, line: str) -> State:
        if not line.startswith("    Message: "):
            raise RuntimeError(f"In `Primary`, line {self.current_linenum}: expected to start with `    Message: `")
        message_string = line.split(':', 1)[1].strip()  # We strip prefix and suffix spaces.
        if not (message_string.startswith('"') and message_string.endswith('"')):
            raise RuntimeError(f"In `Primary`, line {self.current_linenum}, message not enclosed in double quotes")
        self.message = message_string[1:-1]  # We strip message from double quotes.
        self.expected_args = DiagnosticEntryFSM.count_expected_args(self.message)
        return self.State.EXPECT_ARGS

    def handle_expect_args(self, line: str) -> State:
        if self.expected_args != 0 and not line.startswith("      Args: "):
            raise RuntimeError(f"In `Primary`, line {self.current_linenum} expected to start with `      Args: `")
        arg_list_line = line.split(':', 1)[1].strip()  # We strip prefix and suffix spaces.
        if not (arg_list_line.startswith('[') and arg_list_line.endswith(']')):
            raise RuntimeError(f"In `Primary`, line {self.current_linenum} expected `Args` enclosed in [] (brackets)")
        arg_list_line = arg_list_line[1:-1]  # We strip enclosing brackets.
        for arg in arg_list_line.split(','):
            arg = arg.strip()  # We strip surrounding spaces.
            if not (arg.startswith('"') and arg.endswith('"')):
                arg = arg.removeprefix('"').removesuffix('"')
                raise RuntimeError(f"In `Primary`, line {self.current_linenum}, `{arg}` not enclosed in double quotes")
            arg = arg[1:-1]  # We strip arg from double quotes.
            self.args.append(arg)
        if self.expected_args != len(self.args):
            raise RuntimeError(f"In `Primary`, line {self.current_linenum}, "
                               f"message expects {self.expected_args}, but args were {len(self.args)}")
        return self.State.EXPECT_LOCATION

    def handle_expect_location(self, line: str) -> State:
        if not line.startswith("    Location: "):
            raise RuntimeError(f"In `Primary`, line {self.current_linenum} expected to start with `    Location: `")
        location_string = line.split(':', 1)[1].strip()  # We strip prefix and suffix spaces.
        if not (location_string.startswith('"') and location_string.endswith('"')):
            raise RuntimeError(f"In `Primary`, line {self.current_linenum}, location not enclosed in double quotes")
        self.location = location_string[1:-1]  # We strip arg from double quotes.
        return self.State.DONE

    def parse_primary_fsm(self, yaml_lines: list[str], current_linenum: int) -> DiagnosticEntry:
        self.yaml_lines = yaml_lines
        self.current_linenum = current_linenum
        self.args = list()

        self.state = self.State.EXPECT_MESSAGE  # Initial FSM state
        while not self.state.DONE:
            if self.current_linenum >= len(self.yaml_lines):
                raise RuntimeError(f"In line {self.current_linenum} EOF reached. But PrimaryFSM was not DONE")

            current_line = self.yaml_lines[current_linenum]
            if self.state == self.State.EXPECT_MESSAGE:
                self.state = self.handle_expect_message(current_line)
            elif self.state == self.State.EXPECT_ARGS:
                self.state = self.handle_expect_args(current_line)
            elif self.state == self.State.EXPECT_LOCATION:
                self.state = self.handle_expect_location(current_line)
            else:
                assert False, "Unknown PrimaryFSM.State"

            self.current_linenum += 1

        return DiagnosticEntry(message=self.message, args=self.args, location=self.location)


class DiagnosticFSM:
    class State(Enum):
        EXPECT_DIAGNOSTIC = auto()
        EXPECT_TYPE = auto()
        EXPECT_PRIMARY = auto()
        EXPECT_NOTES = auto()
        DONE = auto()

    state: State
    yaml_lines: list[str]
    current_linenum: int

    diag_name: str
    diag_type: Diagnostic.Type
    diag_primary: DiagnosticEntry
    diag_notes: list[DiagnosticEntry]

    # Initialize sub-FSMs
    def __init__(self):
        self.entry_fsm = DiagnosticEntryFSM()

    def handle_expect_diagnostic(self, line: str) -> State:
        if not line.startswith("- Diagnostic: "):
            raise RuntimeError(f"In line {self.current_linenum}, expected line to start with  `- Diagnostic: ` ")
        self.diag_name = line.split(':', 1)[1].strip()
        return self.State.EXPECT_TYPE

    def handle_expect_type(self, line: str) -> State:
        if not line.startswith("  Type: "):
            raise RuntimeError(f"In line {self.current_linenum}, expected line to start with `  Type: `")
        self.diag_type = Diagnostic.to_diag_type(line.split(':', 1)[1].strip())
        return self.State.EXPECT_PRIMARY

    def handle_expect_primary(self, line: str) -> State:
        if not line.startswith("  Primary:"):
            raise RuntimeError(f"In line {self.current_linenum}, expected line to start with `  Primary:`")
        self.diag_primary = self.entry_fsm.parse_primary_fsm(self.yaml_lines, self.current_linenum)
        return self.State.EXPECT_NOTES

    def handle_expect_notes(self, line: str) -> State:
        if not line.startswith("  Notes:"):
            raise RuntimeError(f"In line {self.current_linenum}, expected line to start with `  Notes:`")
        return self.State.DONE

    def parse_diagnostic_fsm(self, yaml_lines: list[str], current_linenum: int) -> Diagnostic:
        self.yaml_lines = yaml_lines
        self.current_linenum = current_linenum

        self.state = self.State.EXPECT_DIAGNOSTIC  # Initial FSM state
        while not self.State.DONE:
            if self.current_linenum >= len(self.yaml_lines):
                raise RuntimeError(f"In line {self.current_linenum} EOF reached. But PrimaryFSM was not DONE")

            current_line = self.yaml_lines[self.current_linenum]
            if self.state == self.State.EXPECT_DIAGNOSTIC:
                self.state = self.handle_expect_diagnostic(current_line)
            elif self.state == self.State.EXPECT_TYPE:
                self.state = self.handle_expect_type(current_line)
            elif self.state == self.State.EXPECT_PRIMARY:
                self.state = self.handle_expect_primary(current_line)
            else:
                assert False, "Unknown DiagnosticFSM.State"

            self.current_linenum += 1

        return Diagnostic(
            diag_name=self.diag_name,
            diag_type=self.diag_type,
            diag_primary=self.diag_primary,
            diag_notes=self.diag_notes
        )
