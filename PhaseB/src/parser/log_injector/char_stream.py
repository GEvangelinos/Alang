class CharStream:
        def __init__(self):
                self.__character_list: list[str] = []
                self.__size: int = 0
                self.__index: int = 0

        @property
        def size(self) -> int:
                return self.__size

        @property
        def index(self) -> int:
                return self.__index

        def peek(self) -> str | None:
                if self.__index < self.__size:
                        return self.__character_list[self.__index]
                return None

        def peek_next(self, forward_offset:int = None) -> str | None:
                if forward_offset is None:
                        forward_offset = 1
                if self.__index + forward_offset < self.__size:
                        return self.__character_list[self.__index + forward_offset]
                return None

        def next(self) -> str | None:
                if self.__index < self.__size:
                        ch = self.__character_list[self.__index]
                        self.__index += 1
                        return ch
                return None

        def eof(self) -> bool:
                return self.__index >= self.__size

        def extend(self, data: str | list[str]) -> None:
                try:
                        self.__character_list.extend(data)
                        self.__size += len(data)
                except TypeError:
                        raise TypeError("append() expects a str or a list[str]")

        def rewind(self) -> None:
                self.__index = 0

        def clear(self) -> None:
                self.__character_list.clear()
                self.__size = 0
                self.__index = 0

        # Warning: O(n) complexity
        def is_char1_before_char2(self, char1: str, char2: str) -> bool:
                for i in range(self.__index, self.__size):
                        if self.__character_list[i] == char1:
                                return True
                        if self.__character_list[i] == char2:
                                return False
                return False
        

