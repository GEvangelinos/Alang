__all__ = ['grammar_section_manager']

from fsm_context import ParseContext, GrammarContext
from fsm_for_lhs_nonterminal import parse_lhs_nonterminal
from fsm_for_rhs_productions import parse_rhs_productions

def grammar_section_manager(ctx: ParseContext) -> None:
        gctx = GrammarContext(logging_function_name=ctx.logging_function_name, hook_name=ctx.hook_name)
        while not ctx.charStream.eof():
                gctx.lhs_nonterminal = parse_lhs_nonterminal(ctx)
                if gctx.lhs_nonterminal is None:
                        return None
                parse_rhs_productions(ctx=ctx, gctx=gctx)
