import re
import utils
from enum import Enum, auto
from line_tracker import LineTracker
from models import DiagnosticEntry, Diagnostic


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
            raise RuntimeError(f"In line {linenum}: expected to start with `    - Message: `")
        message_string = line.split(':', 1)[1].strip()  # We strip prefix and suffix spaces.
        if not utils.is_in_double_quotes(message_string):
            raise RuntimeError(f"In line {linenum}:, message not enclosed in double quotes")
        self.message = message_string[1:-1]  # We strip message from double quotes.
        self.expected_args = DiagnosticEntryFSM.count_expected_args(self.message)
        self.line_tracker.advance()
        return self.State.EXPECT_ARGS

    # noinspection RegExpRedundantEscape
    def handle_expect_args(self) -> State:
        if self.expected_args == 0:
            return self.State.EXPECT_LOCATION

        linenum = self.line_tracker.linenum()
        line = self.line_tracker.line()
        if not line.startswith("      Args: "):
            raise RuntimeError(f"In line {linenum}: expected to start with `      Args: `")
        arg_list_line = line.split(':', 1)[1].strip()  # We strip prefix and suffix spaces.
        if not utils.is_in_brackets(arg_list_line):
            raise RuntimeError(f"In line {linenum}: expected `Args` enclosed in [] (brackets)")
        arg_list_line = utils.strip_brackets(arg_list_line)

        for arg in arg_list_line.split(','):
            arg = arg.strip()  # We strip surrounding spaces.
            if not utils.is_valid_cpp_identifier(arg):
                raise RuntimeError(f"In line {linenum}: argument `{arg}` is not a valid C++ identifier")
            self.args.append(arg)

        if self.expected_args != len(self.args):
            raise RuntimeError(
                f"In line {linenum}: argument count mismatch "
                f"— expected {self.expected_args} values in `Args`, but got {len(self.args)}."
            )
        self.line_tracker.advance()
        return self.State.EXPECT_LOCATION

    def handle_expect_location(self) -> State:
        linenum = self.line_tracker.linenum()
        line = self.line_tracker.line()
        if not line.startswith("      Location: "):
            raise RuntimeError(f"In line {linenum}: expected to start with `      Location: `(Are placeholders index?)")
        self.location = line.split(':', 1)[1].strip()  # We strip prefix and suffix spaces.
        self.line_tracker.advance()
        return self.State.DONE

    def parse_entry_fsm(self, line_tracker: LineTracker) -> DiagnosticEntry:
        self.line_tracker = line_tracker
        self.args = list()

        self.state = self.State.EXPECT_MESSAGE  # Initial FSM state
        while self.state != self.state.DONE:
            self.line_tracker.skip_empty_lines()
            if self.line_tracker.at_end():
                raise RuntimeError(f"In line {self.line_tracker.linenum()}: EOF reached, but EntryFSM was not DONE")

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
            raise RuntimeError(
                f"In line {self.line_tracker.linenum()}:, expected line to start with  `- Diagnostic: ` ")
        self.diag_name = line.split(':', 1)[1].strip()
        self.line_tracker.advance()
        return self.State.EXPECT_TYPE

    def handle_expect_type(self) -> State:
        linenum = self.line_tracker.linenum()
        line = self.line_tracker.line()
        if not line.startswith("  Type: "):
            raise RuntimeError(f"In line {linenum}:, expected line to start with `  Type: `")
        line = line.split(':', 1)[1].strip()  # We extract type and we strip spaces around it.
        try:
            self.diag_type = Diagnostic.to_type(line)
        except ValueError as e:
            raise ValueError(f"In line {linenum}: {e}")
        self.line_tracker.advance()
        return self.State.EXPECT_PRIMARY

    def handle_expect_primary(self) -> State:
        if not self.line_tracker.line().startswith("  Primary:"):
            raise RuntimeError(f"In line {self.line_tracker.linenum()}:, expected line to start with `  Primary:`")
        self.line_tracker.advance()  # We advance to "enter" primary
        self.diag_primary = self.entry_fsm.parse_entry_fsm(self.line_tracker)
        return self.State.EXPECT_NOTES

    def handle_expect_notes(self) -> State:
        if self.line_tracker.line().startswith("  Notes:"):
            self.line_tracker.advance()
            self.line_tracker.skip_empty_lines()
            if self.line_tracker.at_end() or not self.line_tracker.line().startswith("    - Message: "):
                raise RuntimeError(f"In line {self.line_tracker.linenum()}:, expected `Message` definition for `Notes`")
            while not self.line_tracker.at_end() and self.line_tracker.line().startswith("    - Message: "):
                self.diag_notes.append(self.entry_fsm.parse_entry_fsm(self.line_tracker))
        return self.State.DONE

    def parse_diagnostic_fsm(self, line_tracker: LineTracker) -> Diagnostic:
        self.line_tracker = line_tracker
        self.diag_notes = list()

        self.state = self.State.EXPECT_DIAGNOSTIC  # Initial FSM state
        while self.state != self.State.DONE:
            self.line_tracker.skip_empty_lines()
            if self.line_tracker.at_end() and self.state == self.State.EXPECT_NOTES:
                break
            if self.line_tracker.at_end() and self.state != self.State.EXPECT_NOTES:
                raise RuntimeError(
                    f"In line {self.line_tracker.linenum()}: EOF reached, but DiagnosticFSM was not DONE")

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
            type_=self.diag_type,
            primary=self.diag_primary,
            notes=self.diag_notes
        )
