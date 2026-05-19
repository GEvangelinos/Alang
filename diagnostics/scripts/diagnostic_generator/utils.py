from re import match

def is_in_double_quotes(string: str) -> bool:
    string = string.strip()
    return string.startswith('"') and string.endswith('"')


def strip_double_quotes(string: str) -> str:
    return string.removeprefix('"').removesuffix('"')


def is_in_brackets(string: str) -> bool:
    string = string.strip()
    return string.startswith('[') and string.endswith(']')


def strip_brackets(string: str) -> str:
    return string.removeprefix('[').removesuffix(']')


def is_valid_cpp_identifier(id: str) -> bool:
    return bool(match(r"^[a-zA-Z_]\w*$",id))

def temp_version(filename: str) -> str:
    return ".tmp_" + filename

