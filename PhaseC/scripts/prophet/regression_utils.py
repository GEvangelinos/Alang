from re import compile

def visible_len(s):
    ascii_sgr_pattern = compile(r'\033\[[0-?]*[ -/]*[@-~]')
    return len(ascii_sgr_pattern.sub('', s))
