import enum
from fsm_context import ParseContext
from fsm_common import advance_and_track_line

__all__ = ['parse_lhs_nonterminal']


class _States(enum.Enum):
        SKIP_LEADING_WHITESPACE = enum.auto()
        READ_IDENTIFIER = enum.auto()
        READ_COLON = enum.auto()
        EXPORT_LHS_NONTERMINAL = enum.auto()  # Final state




def _handle_skip_leading_whitespace(ctx: ParseContext) -> _States:
        ch = ctx.charStream.peek()

        if ch.isspace():
                advance_and_track_line(ctx=ctx)
                return _States.SKIP_LEADING_WHITESPACE
        return _States.READ_IDENTIFIER


def _handle_read_identifier(lhs_nonterminal: list[str], ctx: ParseContext) -> _States:
        ch = ctx.charStream.peek()

        if ch == ':' and len(lhs_nonterminal) == 0:
                raise ValueError(f"Line {ctx.line_count}: LHS_NONTERMINAL empty, ':' met before name of nonterminal.")
        if ch == ':':
                advance_and_track_line(ctx=ctx)
                return _States.EXPORT_LHS_NONTERMINAL
        if ch.isspace():
                return _States.READ_COLON
        if len(lhs_nonterminal) == 0 and ch.isdigit():
                raise ValueError(f"Line {ctx.line_count}: LHS_NONTERMINAL cannot begin with digit '{ch}'.")
        if ch.isalnum() or ch == '_':
                lhs_nonterminal.append(ch)
                advance_and_track_line(ctx=ctx)
                return _States.READ_IDENTIFIER
        else:
                raise ValueError(f"Line {ctx.line_count}: LHS_NONTERMINAL cannot contain character '{ch}'.")


def _handle_read_colon(lhs_nonterminal: list[str], ctx: ParseContext) -> _States:
        ch = ctx.charStream.peek()
        if ch.isspace():
                advance_and_track_line(ctx=ctx)
                return _States.READ_COLON
        elif ch == ':':
                advance_and_track_line(ctx=ctx)
                return _States.EXPORT_LHS_NONTERMINAL

        raise ValueError(f"Line {ctx.line_count}:"
                         f" LHS_NONTERMINAL expected ':' after '{''.join(lhs_nonterminal)}'.")


def parse_lhs_nonterminal(ctx: ParseContext) -> str | None:
        lhs_nonterminal: list[str] = []
        state = _States.SKIP_LEADING_WHITESPACE

        while not ctx.charStream.eof():
                if state == _States.SKIP_LEADING_WHITESPACE:
                        state = _handle_skip_leading_whitespace(ctx=ctx)
                elif state == _States.READ_IDENTIFIER:
                        state = _handle_read_identifier(lhs_nonterminal=lhs_nonterminal, ctx=ctx)
                elif state == _States.READ_COLON:
                        state = _handle_read_colon(lhs_nonterminal=lhs_nonterminal, ctx=ctx)
                elif state == _States.EXPORT_LHS_NONTERMINAL:
                        return ''.join(lhs_nonterminal)
        if lhs_nonterminal:
                raise ValueError(f"Line {ctx.line_count}: Incomplete LHS_NONTERMINAL, ':' not found, EOF reached.")
        return None
