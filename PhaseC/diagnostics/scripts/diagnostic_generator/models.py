from enum import Enum, auto



class DiagnosticEntry:
    def __init__(self, message: str, args: list[str], location: str):
        self.message = message
        self.args = args
        self.location = location


class Diagnostic:
    class Type(Enum):
        WARNING = auto()
        ERROR = auto()
        FATAL = auto()

        def __str__(self):
            return f"{self.name}"

    def __init__(
        self,
        name: str,
        type_: Type,
        primary: DiagnosticEntry,
        notes: list[DiagnosticEntry]
    ):
        self.name = name
        self.type = type_
        self.primary = primary
        self.notes = notes

    @staticmethod
    def to_type(type_: str):
        match type_:
            case "WARNING":
                return Diagnostic.Type.WARNING
            case "ERROR":
                return Diagnostic.Type.ERROR
            case "FATAL":
                return Diagnostic.Type.FATAL
            case _:
                raise ValueError("Unknown diagnostic type")
