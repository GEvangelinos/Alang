class LineTracker:
    def __init__(self, lines: list[str]):
        self.lines = lines
        self._line_index = 0

    def lineno(self) -> int:
        return self._line_index + 1

    def line(self) -> str:
        return self.lines[self._line_index]

    def advance(self) -> None:
        self._line_index += 1

    def at_end(self) -> bool:
        return self._line_index >= len(self.lines)

    def skip_empty_lines(self) -> None:
        def is_skippable_line(line: str) -> bool:
            return line.lstrip().startswith("#") or not line.strip()

        while not self.at_end() and is_skippable_line(self.line()):
            self.advance()
