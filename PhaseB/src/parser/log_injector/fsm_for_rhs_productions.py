import re
from parser_context import ParserContext
from char_stream import CharStream
from fsm_common import advance_and_track_line

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
        "INC": "++",
        "DEC": "--",
        "GT": ">",
        "LT": "<",
        "GTE": ">=",
        "LTE": "<=",
        # Punctuation tokens
        "LEFT_BRACE": "{",
        "RIGHT_BRACE": "}",
        "LEFT_BRACKET": "[",
        "RIGHT_BRACKET": "]",
        "LEFT_PAREN": "(",
        "RIGHT_PAREN": ")",
        "SEMICOLON": ";",
        "COMMA": ",",
        "COLON": ":",
        "GLOBAL": "::",
        "DOT": ".",
        "METHOD_CALL": "..",
}

_PRODUCTION_SEPARATOR = "|"
_RULE_END_MARKER = ";"


def is_in_midrule(cs: CharStream) -> bool:
        index = 1
        while True:
                future_char: str = cs.peek_next(index)
                index += 1
                if future_char == None:
                        False
                if future_char.isspace():
                        continue
                if future_char in [_PRODUCTION_SEPARATOR, _RULE_END_MARKER]:
                        return False
                return True


def _filter_directive_from_production(unfiltered_production: str) -> str:
        return re.sub(r"\s*%\w+\s+\w+", "", unfiltered_production)


def convert_terminals_to_symbols(production_rule: list[str]):
        for i in range(len(production_rule)):
                if production_rule[i] in terminal_dict.keys():
                        production_rule[i] = terminal_dict[production_rule[i]]


def _inject_log_in_codeblock(ctx: ParserContext):
        convert_terminals_to_symbols(ctx.production)
        unfiltered_production = " ".join(ctx.production)
        directive_filtered_production = _filter_directive_from_production(
                unfiltered_production
        )
        injection_string = f'{ctx.logging_function_name}("{ctx.lhs_nonterminal}", "{directive_filtered_production}");'
        ctx.injectedCharStream.extend(injection_string)
        ctx.injected = True


def _inject_log_at_prodcuction_end(ctx: ParserContext) -> None:
        convert_terminals_to_symbols(ctx.production)
        unfiltered_production = " ".join(ctx.production)
        directive_filtered_production = _filter_directive_from_production(
                unfiltered_production
        )
        injection_string = f'{{{ctx.logging_function_name}("{ctx.lhs_nonterminal}", "{directive_filtered_production}");}}\n'
        ctx.injectedCharStream.extend(injection_string)
        ctx.injected = True


def _handled_whitespace(ch: str, current_token: list[str], ctx: ParserContext) -> bool:
        if ch.isspace():
                if current_token:
                        ctx.production.append("".join(current_token))
                        current_token.clear()
                return True
        return False


def _handled_quotes(ch: str, current_token: list[str], ctx: ParserContext) -> bool:
        if ch == "'":
                if ctx.in_quotes:
                        ctx.production.append("".join(current_token))
                        current_token.clear()
                        ctx.in_quotes = False
                else:
                        # We met a new token (but there was no space in between, e.g: expr'%'expr)
                        if (
                                current_token
                        ):  # This case runs when we have something like this: expr')' with no space in between.
                                ctx.production.append("".join(current_token))
                                current_token.clear()
                        ctx.in_quotes = True
                return True  # Enter quote mode
        elif ctx.in_quotes:
                current_token.append(ch)
                return True
        return False


def _handled_braces(ch: str, current_token: list[str], ctx: ParserContext) -> bool:
        if ch == "{":
                if current_token:
                        ctx.production.append("".join(current_token))
                        current_token.clear()
                ctx.increment_brace_depth()
                ctx.in_block = True
                return True
        if ch == "}":
                if ctx.brace_depth <= 0:
                        raise ValueError(f"In line {ctx.line_counter}: unmatched '}}'")
                if not ctx.injected and not is_in_midrule(ctx.charStream):
                        _inject_log_in_codeblock(ctx)

                ctx.decrement_brace_depth()
                if ctx.brace_depth == 0:
                        ctx.in_block = False
                return True
        if ctx.in_block:
                return True
        return False


def _handled_production_or_rule_end(ch: str, current_token: list[str], ctx: ParserContext) -> bool:
        if ch not in [_PRODUCTION_SEPARATOR, _RULE_END_MARKER]:
                return False

        if current_token:
                ctx.production.append("".join(current_token))
                current_token.clear()

        if not ctx.injected:
                _inject_log_at_prodcuction_end(ctx)

        ctx.production.clear()
        if ch == _RULE_END_MARKER:
                ctx.lhs_nonterminal = ""

        ctx.injected = False
        return True


def _handled_comment(ch: str, ctx: ParserContext) -> bool:
        if ctx.in_line_comment:  # Handle line-comment.
                if ch == "\n":  # Condition to exit line-comment.
                        ctx.in_line_comment = False
                        ctx.found_backslash = False
        elif ctx.in_block_comment:
                if ch == "*":
                        ctx.found_asterisk = True
                elif ch == "/" and ctx.found_asterisk:  # Condition to exit block-comment.
                        ctx.in_block_comment = False
                        ctx.found_backslash = False
                        ctx.found_asterisk = False
                else:
                        ctx.found_asterisk = False  # We could check before, but I chose to just false for optimization.
        elif ctx.found_backslash:
                if ch != "/" and ch != "*":
                        raise ValueError(f"In line {ctx.line_counter}: {ch} followed after '/'. ")
                if ch == "/":
                        ctx.in_line_comment = True
                if ch == "*":
                        ctx.in_block_comment = True
        elif ch == "/":
                ctx.found_backslash = True
        else:
                return False
        return True


def parse_rhs_productions(ctx: ParserContext) -> None:
        current_token: list[str] = []
        ctx.lower_all_flags()
        done = False
        while not done and not ctx.charStream.eof():
                ch = ctx.charStream.peek()
                if _handled_quotes(ch, current_token, ctx):
                        pass
                elif _handled_comment(ch, ctx):
                        pass
                elif _handled_whitespace(ch, current_token, ctx):
                        pass
                elif _handled_braces(ch, current_token, ctx):
                        pass
                elif _handled_production_or_rule_end(ch, current_token, ctx):
                        if ch == _RULE_END_MARKER:
                                done = True
                else:
                        current_token.append(ch)

                advance_and_track_line(ctx)
