import enum
from char_stream import CharStream
from parse_context import ParseContext

__all__ = ['fsm_for_rule_header']

_latest_constructed_line: list[str] = []


class States(enum.Enum):
        SKIP_LEADING_WHITESPACE = enum.auto()
        READ_IDENTIFIER = enum.auto()
        READ_COLON = enum.auto()
        EXPORT_HEADER_RULE = enum.auto()  # Final state


def handle_skip_leading_whitespace(ch: str, cs: CharStream) -> States:
        if ch.isspace():
                cs.next()
                return States.SKIP_LEADING_WHITESPACE
        return States.READ_IDENTIFIER


def handle_read_identifier(ch: str, rule_header: list[str], cs: CharStream, line_count: int) -> States:
        if not ch.isalnum() and ch != '_':
                raise ValueError(
                        f"Line {line_count}: RULE_HEADER_NAME cannot contain character '{ch}'.")
        if len(rule_header) == 0 and ch.isdigit():
                raise ValueError(f"Line {line_count}: RULE_HEADER_NAME cannot begin with digit '{ch}'.")
        if len(rule_header) == 0 and ch == ':':
                raise ValueError(f"Line {line_count}: "
                                 f"RULE_HEADER_NAME empty, ':' met before rule name.")
        if ch.isalnum() or ch == '_':
                rule_header.append(ch)
                cs.next()
                return States.READ_IDENTIFIER
        elif ch == ':':
                cs.next()
                return States.EXPORT_HEADER_RULE
        elif ch.isspace():
                return States.READ_COLON

        raise AssertionError(f"Line {line_count}: Unhandled character '{ch}' "
                             f"in READ_IDENTIFIER state — logic error or missing transition.")


def handle_read_colon(ch: str, rule_header: list[str], cs: CharStream, line_count: int) -> States:
        if ch.isspace():
                cs.next()
                return States.READ_COLON
        elif ch == ':':
                cs.next()
                return States.EXPORT_HEADER_RULE

        raise ValueError(f"Line {line_count}:"
                         f" RULE_HEADER_NAME ':' expected after '{''.join(rule_header)}'.")


def fsm_for_rule_header(cs: CharStream, ctx: ParseContext) -> str:
        rule_header: list[str] = []
        state = States.SKIP_LEADING_WHITESPACE

        while not cs.eof():
                ch = cs.peek()

                if state == States.SKIP_LEADING_WHITESPACE:
                        state = handle_skip_leading_whitespace(ch=ch, cs=cs)
                elif state == States.READ_IDENTIFIER:
                        state = handle_read_identifier(ch=ch, rule_header=rule_header, cs=cs, line_count=ctx.line_count)
                elif state == States.READ_COLON:
                        state = handle_read_colon(ch=ch, rule_header=rule_header, cs=cs, line_count=ctx.line_count)
                elif state == States.EXPORT_HEADER_RULE:
                        return ''.join(rule_header)

        raise ValueError(f"Line {ctx.line_count}: Incomplete rule header. ':' not found before EOF.")
