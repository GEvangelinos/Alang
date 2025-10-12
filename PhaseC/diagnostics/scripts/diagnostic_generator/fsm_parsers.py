import re

import utils
from enum import Enum, IntFlag, auto
from line_tracker import LineTracker
from models import DiagnosticIssue, Diagnostic, IssueEntry
from typing import Optional


class DiagnosticIssueFSM:
    class State(IntFlag):
        EXPECT_MESSAGE = auto()
        EXPECT_ARGS = auto()
        EXPECT_LOCATION = auto()
        TRY_SUGGESTION = auto()
        TRY_HIGHLIGHTS = auto()
        DONE = auto()

    REQUIRED_FIELD_MASK = State.EXPECT_MESSAGE | State.EXPECT_ARGS | State.EXPECT_LOCATION

    state: State
    line_tracker: LineTracker

    class Entry:
        def __init__(self):
            self.message: str = ""
            self.expected_args: int = 0
            self.args: list[str] = []
            self.location: str = ""

    def __init__(self):
        self.reset()

    def reset(self):
        self.main_entry: DiagnosticIssueFSM.Entry = self.Entry()
        self.suggestion: Optional[DiagnosticIssueFSM.Entry] = None
        self.highlights: list[DiagnosticIssueFSM.Entry] = []

        self.parsing_suggestion: bool = False
        self.parsing_highlights: bool = False

    # noinspection RegExpRedundantEscape
    @staticmethod
    def count_expected_args(message: str) -> int:
        # Matches unescaped FMT::format-style numbered placeholders like {0}, {1}, etc.,
        # but skips escaped ones like {{0}} or {{name}}

        # We only put \d in capture group ( integer itself)
        placeholder_pattern = re.compile(r"(?<!\{)\{(\d+)\}(?!\})")
        indices = [int(index) for index in re.findall(placeholder_pattern, message)]

        # We add +1 to convert max_index to expected args count.
        return max(indices) + 1 if indices else 0

    def handle_expect_message(self) -> State:
        lineno = self.line_tracker.lineno()
        line = self.line_tracker.line()

        expected_message_str = " " * 4 + "- Message:"
        if self.parsing_suggestion or self.parsing_highlights:
            expected_message_str = " " * 4 + expected_message_str

        if not line.startswith(expected_message_str):
            raise RuntimeError(f"In line {lineno}: expected to start with `{expected_message_str}`")
        message_string = line.split(':', 1)[1].strip()  # We strip prefix and suffix spaces.
        if not utils.is_in_double_quotes(message_string):
            raise RuntimeError(f"In line {lineno}:, message not enclosed in double quotes")

        message_string = message_string[1:-1]  # We strip message from double quotes.
        expected_args = DiagnosticIssueFSM.count_expected_args(message_string)
        if self.parsing_suggestion:
            assert self.suggestion is not None
            self.suggestion.message = message_string
            self.suggestion.expected_args = expected_args
        elif self.parsing_highlights:
            self.highlights.append(self.Entry())
            self.highlights[-1].message = message_string
            self.highlights[-1].expected_args = expected_args
        else:
            self.main_entry.message = message_string
            self.main_entry.expected_args = expected_args

        self.line_tracker.advance()
        return self.State.EXPECT_ARGS

    # noinspection RegExpRedundantEscape
    def handle_expect_args(self) -> State:
        if self.parsing_suggestion:
            assert self.suggestion is not None
            expected_args = self.suggestion.expected_args
        elif self.parsing_highlights:
            assert len(self.highlights) > 0
            expected_args = self.highlights[-1].expected_args
        else:
            expected_args = self.main_entry.expected_args

        if expected_args == 0:
            return self.State.EXPECT_LOCATION

        lineno = self.line_tracker.lineno()
        line = self.line_tracker.line().strip()
        if not line.startswith("Args:"):
            raise RuntimeError(f"In line {lineno}: expected to start with `Args:`")
        arg_list_line = line.split(':', 1)[1].strip()  # We strip prefix and suffix spaces.
        if not utils.is_in_brackets(arg_list_line):
            raise RuntimeError(f"In line {lineno}: expected `Args` enclosed in [] (brackets)")
        arg_list_line = utils.strip_brackets(arg_list_line)

        args_buffer = []
        for arg in arg_list_line.split(','):
            arg = arg.strip()  # We strip surrounding spaces.
            if not utils.is_valid_cpp_identifier(arg):
                raise RuntimeError(
                    f"In line {lineno}: argument `{arg}` is not a valid C++ identifier")
            args_buffer.append(arg)

        if expected_args != len(args_buffer):
            raise RuntimeError(
                f"In line {lineno}: argument count mismatch "
                f"— expected {expected_args} values in `Args`, but got {len(args_buffer)}."
            )
        if self.parsing_suggestion:
            self.suggestion.args = args_buffer
        elif self.parsing_highlights:
            self.highlights[-1].args = args_buffer
        else:
            self.main_entry.args = args_buffer
        self.line_tracker.advance()
        return self.State.EXPECT_LOCATION

    def handle_expect_location(self) -> State:
        lineno = self.line_tracker.lineno()
        line = self.line_tracker.line().strip()
        if not line.startswith("Location:"):
            raise RuntimeError(
                f"In line {lineno}: expected to start with `Location:`(Are placeholders index?)")

        location = line.split(':', 1)[1].strip()  # We strip prefix and suffix spaces.
        if self.parsing_suggestion:
            self.suggestion.location = location
        elif self.parsing_highlights:
            self.highlights[-1].location = location
        else:
            self.main_entry.location = location

        self.line_tracker.advance()
        if self.parsing_highlights:
            return self.State.TRY_HIGHLIGHTS
        if self.parsing_suggestion:
            return self.State.TRY_SUGGESTION
        return self.State.TRY_SUGGESTION

    def handle_try_suggestion(self) -> State:
        # Check if we were called after parsing a suggestion.
        if self.parsing_suggestion:
            self.parsing_suggestion = False
            return self.State.TRY_HIGHLIGHTS

        line = self.line_tracker.line().strip()
        if not line.startswith("Suggestion:"):
            return self.State.TRY_HIGHLIGHTS
        self.line_tracker.advance()
        self.parsing_suggestion = True
        self.suggestion = self.Entry()
        return self.State.EXPECT_MESSAGE

    def handle_try_highlights(self) -> State:

        if self.line_tracker.at_end():
            return self.State.DONE

        line = self.line_tracker.line()
        lineno = self.line_tracker.lineno()

        if self.parsing_highlights and line.startswith("Highlights:"):
            raise ValueError(f"In line {lineno}, Duplicate `Highlights:` label detected")

        if self.parsing_highlights and " " * 8 + "- Message:" in line:
            return self.State.EXPECT_MESSAGE

        if not self.parsing_highlights and line.strip().startswith("Highlights:"):
            self.line_tracker.advance()
            self.parsing_highlights = True
            return self.State.EXPECT_MESSAGE

        return self.State.DONE

    def parse_entry_fsm(self, line_tracker: LineTracker) -> DiagnosticIssue:
        self.line_tracker = line_tracker
        self.reset()
        self.state = self.State.EXPECT_MESSAGE  # Initial FSM state
        while self.state != self.State.DONE:
            self.line_tracker.skip_empty_lines()

            if self.line_tracker.at_end():
                if (type(self).REQUIRED_FIELD_MASK & self.state) or self.parsing_suggestion:
                    raise RuntimeError(
                        f"In line {self.line_tracker.lineno()}: "
                        f"EOF reached, but DiagnosticIssueFSM was not finished scanning required fields"
                    )
                else:
                    break

            if self.state == self.State.EXPECT_MESSAGE:
                self.state = self.handle_expect_message()
            elif self.state == self.State.EXPECT_ARGS:
                self.state = self.handle_expect_args()
            elif self.state == self.State.EXPECT_LOCATION:
                self.state = self.handle_expect_location()
            elif self.state == self.State.TRY_SUGGESTION:
                self.state = self.handle_try_suggestion()
            elif self.state == self.State.TRY_HIGHLIGHTS:
                self.state = self.handle_try_highlights()
            else:
                assert False, "Unknown PrimaryFSM.State"

        def make_issue_entry(issue: Optional[DiagnosticIssueFSM.Entry]) -> Optional[IssueEntry]:
            if issue is None:
                return None
            return IssueEntry(message=issue.message, args=issue.args, location=issue.location)

        self.parsing_suggestion = False
        self.parsing_highlights = False
        return DiagnosticIssue(
            main_issue=make_issue_entry(self.main_entry),
            suggestion=make_issue_entry(self.suggestion),
            highlights=[make_issue_entry(hl) for hl in self.highlights]
        )


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
    diag_primary: DiagnosticIssue
    diag_notes: list[DiagnosticIssue]

    # Initialize sub-FSMs
    def __init__(self):
        self.entry_fsm = DiagnosticIssueFSM()

    def handle_expect_diagnostic(self) -> State:
        line = self.line_tracker.line()
        if not line.startswith("- Diagnostic: "):
            raise RuntimeError(
                f"In line {self.line_tracker.lineno()}:, expected line to start with  `- Diagnostic: ` ")
        self.diag_name = line.split(':', 1)[1].strip()
        self.line_tracker.advance()
        return self.State.EXPECT_TYPE

    def handle_expect_type(self) -> State:
        linenum = self.line_tracker.lineno()
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
            raise RuntimeError(
                f"In line {self.line_tracker.lineno()}:, expected line to start with `  Primary:`")
        self.line_tracker.advance()  # We advance to "enter" primary
        self.diag_primary = self.entry_fsm.parse_entry_fsm(self.line_tracker)
        return self.State.EXPECT_NOTES

    def handle_expect_notes(self) -> State:
        if self.line_tracker.line().startswith("  Notes:"):
            self.line_tracker.advance()
            self.line_tracker.skip_empty_lines()
            if self.line_tracker.at_end() or not self.line_tracker.line().startswith(
                    "    - Message: "):
                raise RuntimeError(
                    f"In line {self.line_tracker.lineno()}:, expected `Message` definition for `Notes`")
            while not self.line_tracker.at_end() and self.line_tracker.line().startswith(
                    "    - Message: "):
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
                    f"In line {self.line_tracker.lineno()}: EOF reached, but DiagnosticFSM was not DONE")

            if self.state == self.State.EXPECT_DIAGNOSTIC:
                self.state = self.handle_expect_diagnostic()
            elif self.state == self.State.EXPECT_TYPE:
                self.state = self.handle_expect_type()
            elif self.state == self.State.EXPECT_PRIMARY:
                self.state = self.handle_expect_primary()
            elif self.state == self.State.EXPECT_NOTES:
                self.state = self.handle_expect_notes()
            else:
                assert False, "Unknown DiagnosticIssueFSM.State"

        return Diagnostic(
            name=self.diag_name,
            type_=self.diag_type,
            primary=self.diag_primary,
            notes=self.diag_notes
        )
