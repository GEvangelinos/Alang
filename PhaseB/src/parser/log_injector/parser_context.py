from char_stream import CharStream

class ParserContext:
        def __init__(self, logging_function_name: str) -> None:
                self.__logging_function_name = logging_function_name  # private
                self.__brace_depth: int = 0  # We just started reading .y file
                self.__line_counter: int = 0  # We just started reading .y file (maybe there is nothing).
                self.charStream: CharStream = CharStream()
                self.injectedCharStream: CharStream = CharStream()
                self.lhs_nonterminal: str = ""
                self.production: list[str] = []
                self.directive: list[str] = []
                self.in_quotes: bool = False
                self.in_block: bool = False
                self.in_line_comment: bool = False
                self.in_block_comment: bool = False
                self.found_backslash: bool = False
                self.found_asterisk: bool = False

        @property
        def logging_function_name(self) -> str:
                return self.__logging_function_name

        @property
        def brace_depth(self) -> int:
                return self.__brace_depth

        @property
        def line_counter(self) -> int:
                return self.__line_counter

        def increment_line_counter(self) -> None:
                self.__line_counter += 1
        
        def increment_brace_depth(self) -> None:
                self.__brace_depth += 1
        
        def decrement_brace_depth(self) -> None:
                self.__brace_depth -= 1

        def lower_all_flags(self) -> None:
                self.in_quotes = False
                self.in_block = False
                self.in_line_comment = False
                self.in_block_comment = False
                self.found_backslash = False
                self.found_asterisk = False



