
def is_in_double_quotes(string:str) -> bool:
        string = string.strip()
        return string.startswith('"') and string.endswith('"')

def strip_double_quotes(string:str) -> str:
        return string.removeprefix('"').removesuffix('"')

def is_in_brackets(string:str) -> bool:
        string = string.strip()
        return string.startswith('[') and string.endswith(']')

def strip_brackets(string:str) -> str:
        return string.removeprefix('[').removesuffix(']')

