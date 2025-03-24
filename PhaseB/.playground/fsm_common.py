from fsm_context import ParseContext

__all__ = ['advance_and_track_line']


def advance_and_track_line(ctx: ParseContext) -> None:
        ch = ctx.charStream.peek()
        if ch == '\n':
                ctx.line_count += 1
        ctx.injectedCharStream.extend(ch)
        ctx.charStream.next()
