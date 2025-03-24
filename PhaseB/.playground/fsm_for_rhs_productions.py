from char_stream import CharStream
from fsm_context import ParseContext, GrammarContext
from fsm_common import advance_and_track_line
import re

__all__ = ['parse_rhs_productions']

terminal_dict: dict[str, str] = {
        # Operator tokens
        "ASSIGN": "=",
        "PLUS": "+",
        "MINUS": "-",
        "MUL": "*",
        "DIV": "/",
        "MOD": "%",
        "EQ": "==",
        "NEQ": "!=",
        "PLUS_PLUS": "++",
        "MINUS_MINUS": "--",
        "GT": ">",
        "LT": "<",
        "GTE": ">=",
        "LTE": "<=",

        # Punctuation tokens
        # TODO: Write those too!

}


def _handled_whitespace(ch: str, current_token: list[str], gctx: GrammarContext) -> bool:
        if ch.isspace():
                if current_token:
                        gctx.production.append(''.join(current_token))
                        current_token.clear()
                return True
        return False


def _handled_quotes(ch: str, current_token: list[str], gctx: GrammarContext) -> bool:
        if ch == '\'':
                if gctx.in_quotes:
                        gctx.production.append(''.join(current_token))
                        current_token.clear()
                        gctx.in_quotes = False
                else:
                        if current_token:  # That is the case where we have something like this: expr')' with no space in between.
                                gctx.production.append(''.join(current_token))
                        gctx.in_quotes = True
                return True  # Enter quote mode
        elif gctx.in_quotes:
                current_token.append(ch)
                return True
        return False


def _handled_braces(ch: str, current_token: list[str], ctx: ParseContext, gctx: GrammarContext) -> bool:
        if ch == '{':
                if current_token:
                        gctx.production.append(''.join(current_token))
                        current_token.clear()
                ctx.brace_depth += 1
                gctx.in_block = True
                return True
        if ch == '}':
                if ctx.brace_depth <= 0:
                        raise ValueError(f"Line {ctx.line_count}: unmatched '}}'")
                ctx.brace_depth -= 1
                if ctx.brace_depth == 0:
                        gctx.in_block = False
                return True
        if gctx.in_block:
                # TODO: If you implement a hook-based injection logging system, you need to replace hook with log here.
                return True
        return False


def _filter_directive_from_production(unfiltered_production: str) -> str:
        return re.sub(r"\s*%\w+\s+\w+", '', unfiltered_production)


def convert_terminals_to_symbols(production_rule: list[str], terminal_map: dict[str, str]):
        pass  # TODO: implement


def _inject_log(ctx: ParseContext, gctx: GrammarContext):
        unfiltered_production = ' '.join(gctx.production)
        directive_filtered_production = _filter_directive_from_production(unfiltered_production)
        injection_string = f"{{{gctx.logging_function_name}(\"{gctx.lhs_nonterminal}\", \"{directive_filtered_production}\");}}\n"
        ctx.injectedCharStream.extend(injection_string)


def _handled_production_or_rule_end(ch: str, ctx: ParseContext, gctx: GrammarContext) -> bool:
        if ch not in ['|', ';']:
                return False

        _inject_log(ctx=ctx, gctx=gctx)

        gctx.production.clear()
        if ch == ';':
                gctx.lhs_nonterminal = ""
        return True


def parse_rhs_productions(ctx: ParseContext, gctx: GrammarContext) -> None:
        current_token: list[str] = []
        gctx.in_block = False
        gctx.in_quotes = False
        done = False
        while not done and not ctx.charStream.eof():
                ch = ctx.charStream.peek()
                if _handled_whitespace(ch=ch, current_token=current_token, gctx=gctx):
                        pass
                elif _handled_quotes(ch=ch, current_token=current_token, gctx=gctx):
                        pass
                elif _handled_braces(ch=ch, current_token=current_token, ctx=ctx, gctx=gctx):
                        pass
                elif _handled_production_or_rule_end(ch=ch, ctx=ctx, gctx=gctx):
                        if ch == ';':
                                done = True
                else:
                        current_token.append(ch)

                advance_and_track_line(ctx=ctx)
