from parser_context import ParserContext

def advance_and_track_line(ctx: ParserContext) -> None:
        ch = ctx.charStream.peek()
        if ch == '\n':
                ctx.increment_line_counter()
        ctx.injectedCharStream.extend(ch)
        ctx.charStream.next()
