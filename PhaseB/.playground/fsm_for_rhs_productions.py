import re
from parser_context import ParserContext
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
        "PLUS_PLUS": "++",
        "MINUS_MINUS": "--",
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
        "COLON_BLOCK": "::",
        "DOT": ".",
        "DDOT": ".."
}


def _handled_whitespace(ch: str, current_token: list[str], ctx: ParserContext) -> bool:
        if ch.isspace():
                if current_token:
                        ctx.production.append(''.join(current_token))
                        current_token.clear()
                return True
        return False


def _handled_quotes(ch: str, current_token: list[str], ctx: ParserContext) -> bool:
        if ch == '\'':
                if ctx.in_quotes:
                        ctx.production.append(''.join(current_token))
                        current_token.clear()
                        ctx.in_quotes = False
                else:
                        if current_token:  # That is the case where we have something like this: expr')' with no space in between.
                                ctx.production.append(''.join(current_token))
                        ctx.in_quotes = True
                return True  # Enter quote mode
        elif ctx.in_quotes:
                current_token.append(ch)
                return True
        return False


def _handled_braces(ch: str, current_token: list[str], ctx: ParserContext) -> bool:
        if ch == '{':
                if current_token:
                        ctx.production.append(''.join(current_token))
                        current_token.clear()
                ctx.increment_brace_depth()
                ctx.in_block = True
                return True
        if ch == '}':
                if ctx.brace_depth <= 0:
                        raise ValueError(f"In line {ctx.line_counter}: unmatched '}}'")
                ctx.decrement_brace_depth()
                if ctx.brace_depth == 0:
                        ctx.in_block = False
                return True
        if ctx.in_block:
                return True
        return False


def _filter_directive_from_production(unfiltered_production: str) -> str:
        return re.sub(r"\s*%\w+\s+\w+", '', unfiltered_production)


def convert_terminals_to_symbols(production_rule: list[str]):
        for i in range(len(production_rule)):
                if production_rule[i] in terminal_dict.keys():
                        production_rule[i] = terminal_dict[production_rule[i]]


def _inject_log_call_in_rule(ctx: ParserContext) -> None:
        convert_terminals_to_symbols(ctx.production)
        unfiltered_production = ' '.join(ctx.production)
        directive_filtered_production = _filter_directive_from_production(unfiltered_production)
        injection_string = f"{{{ctx.logging_function_name}(\"{ctx.lhs_nonterminal}\", \"{directive_filtered_production}\");}}\n"
        ctx.injectedCharStream.extend(injection_string)


def _handled_production_or_rule_end(ch: str, ctx: ParserContext) -> bool:
        if ch not in ['|', ';']:
                return False

        _inject_log_call_in_rule(ctx)

        ctx.production.clear()
        if ch == ';':
                ctx.lhs_nonterminal = ""
        return True


def _handled_comment(ch: str, ctx: ParserContext) -> bool:
        if ctx.in_line_comment:  # Handle line-comment.
                if ch == '\n':  # Condition to exit line-comment.
                        ctx.in_line_comment = False
                        ctx.found_backslash = False
        elif ctx.in_block_comment:
                if ch == '*':
                        ctx.found_asterisk = True
                elif ch == '/' and ctx.found_asterisk:  # Condition to exit block-comment.
                        ctx.in_block_comment = False
                        ctx.found_backslash = False
                        ctx.found_asterisk = False
                else:
                        ctx.found_asterisk = False  # We could check before, but I chose to just false for optimization.
        elif ctx.found_backslash:
                if ch != '/' and ch != '*':
                        raise ValueError(f"In line {ctx.line_counter}: {ch} followed after '/'. ")
                if ch == '/':
                        ctx.in_line_comment = True
                if ch == '*':
                        ctx.in_block_comment = True
        elif ch == '/':
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
                elif _handled_production_or_rule_end(ch, ctx):
                        if ch == ';':
                                done = True
                else:
                        current_token.append(ch)

                advance_and_track_line(ctx)
