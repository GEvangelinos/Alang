from enum import Enum, auto
import re
import utils


class LineTracker:
    def __init__(self, lines: list[str]):
        self.lines = lines
        self._line_index = 0

    def linenum(self) -> int:
        return self._line_index + 1

    def line(self) -> str:
        return self.lines[self._line_index]

    def advance(self) -> None:
        self._line_index += 1

    def at_end(self) -> bool:
        return self._line_index >= len(self.lines)

    def skip_empty_lines(self) -> None:
        while not self.at_end():
            if not self.line() or self.line().isspace():
                self.advance()



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

        def __str__(self):
            return f"{self.name}"

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
        name: str,
        dtype: Type,
        primary: DiagnosticEntry,
        notes: list[DiagnosticEntry]
    ):
        self.name = name
        self.type = dtype
        self.primary = primary
        self.notes = notes


class DiagnosticEntryFSM:
    class State(Enum):
        EXPECT_MESSAGE = auto()
        EXPECT_ARGS = auto()
        EXPECT_LOCATION = auto()
        DONE = auto()

    state: State
    line_tracker: LineTracker

    message: str
    expected_args: int
    args: list[str]
    location: str

    # noinspection RegExpRedundantEscape
    @staticmethod
    def count_expected_args(message: str) -> int:
        # Matches unescaped FMT::format-style numbered placeholders like {0}, {1}, etc.,
        # but skips escaped ones like {{0}} or {{name}}
        placeholder_pattern = re.compile(r"(?<!\{)\{(\d+)\}(?!\})")  # We only put \d in capture group ( integer itself)
        indices = [int(index) for index in re.findall(placeholder_pattern, message)]
        return max(indices) + 1 if indices else 0  # We add +1 to convert max_index to expected args count.

    def handle_expect_message(self) -> State:
        linenum = self.line_tracker.linenum()
        line = self.line_tracker.line()
        if not line.startswith("    - Message: "):
            raise RuntimeError(f"In `Primary`, line {linenum}: expected to start with `    - Message: `")
        message_string = line.split(':', 1)[1].strip()  # We strip prefix and suffix spaces.
        if not utils.is_in_double_quotes(message_string):
            raise RuntimeError(f"In `Primary`, line {linenum}, message not enclosed in double quotes")
        self.message = message_string[1:-1]  # We strip message from double quotes.
        self.expected_args = DiagnosticEntryFSM.count_expected_args(self.message)
        self.line_tracker.advance()
        return self.State.EXPECT_ARGS

    def handle_expect_args(self) -> State:
        if self.expected_args == 0:
            return self.State.EXPECT_LOCATION

        linenum = self.line_tracker.linenum()
        line = self.line_tracker.line()
        if not line.startswith("      Args: "):
            raise RuntimeError(f"In `Primary`, line {linenum} expected to start with `      Args: `")
        arg_list_line = line.split(':', 1)[1].strip()  # We strip prefix and suffix spaces.
        if not utils.is_in_brackets(arg_list_line):
            raise RuntimeError(f"In `Primary`, line {linenum} expected `Args` enclosed in [] (brackets)")
        arg_list_line = utils.strip_brackets(arg_list_line)
        for arg in arg_list_line.split(','):
            arg = arg.strip()  # We strip surrounding spaces.
            if not utils.is_in_double_quotes(arg):
                raise RuntimeError(f"In `Primary`, line {linenum}, "
                                   f"`{utils.strip_double_quotes(arg)}` not enclosed in double quotes")
            arg = arg[1:-1]  # We strip arg from double quotes.
            self.args.append(arg)
        if self.expected_args != len(self.args):
            raise RuntimeError(f"In `Primary`, line {linenum}, "
                               f"message expects {self.expected_args}, but args were {len(self.args)}")
        self.line_tracker.advance()
        return self.State.EXPECT_LOCATION

    def handle_expect_location(self) -> State:
        linenum = self.line_tracker.linenum()
        line = self.line_tracker.line()
        if not line.startswith("      Location: "):
            raise RuntimeError(f"In `Primary`, line {linenum} expected to start with `      Location: `")
        location_string = line.split(':', 1)[1].strip()  # We strip prefix and suffix spaces.
        if not utils.is_in_double_quotes(location_string):
            raise RuntimeError(f"In `Primary`, line {linenum}, location not enclosed in double quotes")
        self.location = location_string[1:-1]  # We strip arg from double quotes.
        self.line_tracker.advance()
        return self.State.DONE

    def parse_entry_fsm(self, line_tracker: LineTracker) -> DiagnosticEntry:
        self.line_tracker = line_tracker
        self.args = list()

        self.state = self.State.EXPECT_MESSAGE  # Initial FSM state
        while self.state != self.state.DONE:
            if self.line_tracker.at_end():
                raise RuntimeError(f"In line {self.line_tracker.linenum()} EOF reached. But EntryFSM was not DONE")
            if self.line_tracker.line().isspace():  # Ignore and skip empty lines
                self.line_tracker.advance()
                continue

            if self.state == self.State.EXPECT_MESSAGE:
                self.state = self.handle_expect_message()
            elif self.state == self.State.EXPECT_ARGS:
                self.state = self.handle_expect_args()
            elif self.state == self.State.EXPECT_LOCATION:
                self.state = self.handle_expect_location()
            else:
                assert False, "Unknown PrimaryFSM.State"

        return DiagnosticEntry(message=self.message, args=self.args, location=self.location)


class DiagnosticFSM:
    class State(Enum):
        EXPECT_DIAGNOSTIC = auto()
        EXPECT_TYPE = auto()
        EXPECT_PRIMARY = auto()
        EXPECT_NOTES = auto()
        DONE = auto()

    state: State
    line_tracker: LineTracker

    diag_name: str
    diag_type: Diagnostic.Type
    diag_primary: DiagnosticEntry
    diag_notes: list[DiagnosticEntry]

    # Initialize sub-FSMs
    def __init__(self):
        self.entry_fsm = DiagnosticEntryFSM()

    def handle_expect_diagnostic(self) -> State:
        line = self.line_tracker.line()
        if not line.startswith("- Diagnostic: "):
            raise RuntimeError(f"In line {self.line_tracker.linenum()}, expected line to start with  `- Diagnostic: ` ")
        self.diag_name = line.split(':', 1)[1].strip()
        self.line_tracker.advance()
        return self.State.EXPECT_TYPE

    def handle_expect_type(self) -> State:
        linenum = self.line_tracker.linenum()
        line = self.line_tracker.line()
        if not line.startswith("  Type: "):
            raise RuntimeError(f"In line {linenum}, expected line to start with `  Type: `")
        line = line.split(':', 1)[1].strip()  # We extract type and we strip spaces around it.
        if not utils.is_in_double_quotes(line):
            raise RuntimeError(f"In line {linenum}, expected `Type` enclosed in double quotes")
        line = line[1:-1]
        try:
            self.diag_type = Diagnostic.to_diag_type(line)
        except ValueError as e:
            raise ValueError(f"In line {linenum}: {e}")
        self.line_tracker.advance()
        return self.State.EXPECT_PRIMARY

    def handle_expect_primary(self) -> State:
        if not self.line_tracker.line().startswith("  Primary:"):
            raise RuntimeError(f"In line {self.line_tracker.linenum()}, expected line to start with `  Primary:`")
        self.line_tracker.advance()  # We advance to "enter" primary
        self.diag_primary = self.entry_fsm.parse_entry_fsm(self.line_tracker)
        return self.State.EXPECT_NOTES

    def handle_expect_notes(self) -> State:
        if self.line_tracker.line().startswith("  Notes:"):
            self.line_tracker.advance()
            while not self.line_tracker.at_end() and self.line_tracker.line().isspace():  # Skip empty lines.
                self.line_tracker.advance()
            if self.line_tracker.at_end() or not self.line_tracker.line().startswith("    - Message: "):
                raise RuntimeError(f"In line {self.line_tracker.linenum()}, expected `Message` definition for `Notes`")
            while not self.line_tracker.at_end() and self.line_tracker.line().startswith("    - Message: "):
                self.diag_notes.append(self.entry_fsm.parse_entry_fsm(self.line_tracker))
        return self.State.DONE

    def parse_diagnostic_fsm(self, line_tracker: LineTracker) -> Diagnostic:
        self.line_tracker = line_tracker
        self.diag_notes = list()

        self.state = self.State.EXPECT_DIAGNOSTIC  # Initial FSM state
        while self.state != self.State.DONE:
            if self.line_tracker.at_end() and self.state == self.State.EXPECT_NOTES:
                break
            if self.line_tracker.at_end() and self.state != self.State.EXPECT_NOTES:
                raise RuntimeError(f"In line {self.line_tracker.linenum()} EOF reached. But DiagnosticFSM was not DONE")
            if self.line_tracker.line().isspace():  # Ignore and skip empty lines
                self.line_tracker.advance()
                continue

            if self.state == self.State.EXPECT_DIAGNOSTIC:
                self.state = self.handle_expect_diagnostic()
            elif self.state == self.State.EXPECT_TYPE:
                self.state = self.handle_expect_type()
            elif self.state == self.State.EXPECT_PRIMARY:
                self.state = self.handle_expect_primary()
            elif self.state == self.State.EXPECT_NOTES:
                self.state = self.handle_expect_notes()
            else:
                assert False, "Unknown DiagnosticFSM.State"

        return Diagnostic(
            name=self.diag_name,
            dtype=self.diag_type,
            primary=self.diag_primary,
            notes=self.diag_notes
        )
