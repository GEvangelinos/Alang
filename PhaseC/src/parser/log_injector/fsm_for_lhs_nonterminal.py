import enum

from parser_context import ParserContext
from fsm_common import advance_and_track_line
from raise_handler import raise_error


class _States(enum.Enum):
    SKIP_LEADING_WHITESPACE = enum.auto()
    READ_IDENTIFIER = enum.auto()
    READ_COLON = enum.auto()
    FOUND_BACKSLASH = enum.auto()
    FOUND_LEFT_BRACKET = enum.auto()
    EXPORT_LHS_NONTERMINAL = enum.auto()  # Final state
# 

# Does NOT return next state
def _handle_found_backslash(ctx: ParserContext) -> None:
    ch = ctx.charStream.peek()
    if ch != '/':
        raise_error(AssertionError,
                    f"In line {ctx.line_counter}:{ch} != '/', thus handler should not be called.")
    ch_next = ctx.charStream.peek_next()
    if ch_next is None:
        raise_error(
            EOFError, f"In line {ctx.line_counter}: reached EOF with stray '/'.")
    if ch == '/' and ch_next == '/':  # Handle line-comment
        while True:
            if ctx.charStream.peek() is None:
                raise_error(
                    EOFError, f"In line {ctx.line_counter}: reached EOF with stray '/'.")
            if ctx.charStream.peek() != '\n':
                advance_and_track_line(ctx)
    elif ch == '/' and ch_next == '*':  # Handle block-comment
        advance_and_track_line(ctx)  # Skip '/'
        advance_and_track_line(ctx)  # Skip '*'
        while True:
            if ctx.charStream.peek() is None:
                raise_error(
                    EOFError, f"In line {ctx.line_counter}: reached EOF with stray '/*'.")
            if ctx.charStream.peek() == '*' and ctx.charStream.peek_next() == '/':
                break
            advance_and_track_line(ctx)
        advance_and_track_line(ctx)  # Skip '*'
        advance_and_track_line(ctx)  # Skip '/'
    else:
        raise_error(
            ValueError, f"In line{ctx.line_counter}: {ch} followed after '/'.")


def _handle_found_left_bracket(lhs_nonterminal: list[str], ctx: ParserContext) -> _States:
    ch = ctx.charStream.peek()
    if ch != '[':
        raise_error(AssertionError,
                    f"In line {ctx.line_counter}:{ch} != '[', thus handler should not be called.")

    while True:
        ch = ctx.charStream.peek()
        if ch is None:
            raise_error(
                EOFError, f"In line {ctx.line_counter}: reached EOF with stray '['.")
        if ch == ':':
            raise_error(
                EOFError, f"In line {ctx.line_counter}: found ':' before closing bracketed name with ']'.")
        lhs_nonterminal.append(ch)
        advance_and_track_line(ctx)
        if ch == ']':
            return _States.READ_COLON


def _handle_skip_leading_whitespace(ctx: ParserContext) -> _States:
    ch = ctx.charStream.peek()
    if ch == '/':
        return _States.FOUND_BACKSLASH
    if ch.isspace():
        advance_and_track_line(ctx)
        return _States.SKIP_LEADING_WHITESPACE
    return _States.READ_IDENTIFIER


def _handle_read_identifier(lhs_nonterminal: list[str], ctx: ParserContext) -> _States:
    ch = ctx.charStream.peek()
    if ch == '/':
        return _States.FOUND_BACKSLASH
    if ch == '[':
        return _States.FOUND_LEFT_BRACKET
    if ch == ':' and len(lhs_nonterminal) == 0:
        raise_error(ValueError,
                    f"In Line {ctx.line_counter}: LHS_NONTERMINAL empty, ':' met before name of nonterminal.")
    if ch == ':':
        advance_and_track_line(ctx)
        return _States.EXPORT_LHS_NONTERMINAL
    if ch.isspace():
        return _States.READ_COLON
    if len(lhs_nonterminal) == 0 and ch.isdigit():
        raise ValueError(
            f"In line {ctx.line_counter}: LHS_NONTERMINAL cannot begin with digit '{ch}'.")
    if ch.isalnum() or ch == '_':
        lhs_nonterminal.append(ch)
        advance_and_track_line(ctx)
        return _States.READ_IDENTIFIER
    else:
        raise ValueError(
            f"In line {ctx.line_counter}: LHS_NONTERMINAL cannot contain character '{ch}'.")


def _handle_read_colon(lhs_nonterminal: list[str], ctx: ParserContext) -> _States:
    ch = ctx.charStream.peek()
    if ch == '/':
        return _States.FOUND_BACKSLASH
    if ch.isspace():
        advance_and_track_line(ctx)
        return _States.READ_COLON
    elif ch == ':':
        advance_and_track_line(ctx)
        return _States.EXPORT_LHS_NONTERMINAL

    raise ValueError(f"In line {ctx.line_counter}:"
                     f" LHS_NONTERMINAL expected ':' after '{''.join(lhs_nonterminal)}'.")


def parse_lhs_nonterminal(ctx: ParserContext) -> str | None:
    lhs_nonterminal: list[str] = []
    prev_state = None
    curr_state = _States.SKIP_LEADING_WHITESPACE  # Initial state

    while not ctx.charStream.eof():
        if curr_state is not _States.FOUND_BACKSLASH:
            prev_state = curr_state

        if curr_state is _States.FOUND_LEFT_BRACKET:
            curr_state = _handle_found_left_bracket(lhs_nonterminal, ctx)
        elif curr_state is _States.SKIP_LEADING_WHITESPACE:
            curr_state = _handle_skip_leading_whitespace(ctx)
        elif curr_state is _States.READ_IDENTIFIER:
            curr_state = _handle_read_identifier(lhs_nonterminal, ctx)
        elif curr_state is _States.READ_COLON:
            curr_state = _handle_read_colon(lhs_nonterminal, ctx)
        elif curr_state is _States.FOUND_BACKSLASH:
            _handle_found_backslash(ctx)
            curr_state = prev_state  # It continues from where it left off.
        elif curr_state is _States.EXPORT_LHS_NONTERMINAL:
            return ''.join(lhs_nonterminal)
    if lhs_nonterminal:
        raise ValueError(
            f"In line {ctx.line_counter}: Incomplete LHS_NONTERMINAL, ':' not found, EOF reached.")
    return None
