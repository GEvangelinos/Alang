import re
import sys

def parse_symbol_line(line):
    match = re.match(r'"(.+)" \[([^\]]+)\] \(line (\d+)\) \(scope (\d+)\)', line.strip())
    if match:
        symbol, sym_type, line_num, scope = match.groups()
        sym_type_clean = sym_type.replace(" ", "_").upper()
        return symbol, sym_type_clean, line_num, scope
    return None

def main():
    input_lines = sys.stdin.read().splitlines()
    print("symbol,type,line,scope")
    for line in input_lines:
        # Skip section headers like: "--------------------     Scope #0     --------------------"
        if re.match(r'-+\s+Scope #\d+\s+-+', line.strip()):
            continue
        parsed = parse_symbol_line(line)
        if parsed:
            print(','.join(parsed))

if __name__ == "__main__":
    main()

