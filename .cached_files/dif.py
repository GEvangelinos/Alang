#!/usr/bin/env python3
import argparse
import csv
import re
from typing import List, Dict, Tuple, Optional

# ---------- Normalization helpers ----------

TEMP_CSV_RE = re.compile(r"^_t(\d+)$")
TEMP_PROF_RE = re.compile(r"^\^(\d+)$")
ANON_STUDENT_RE = re.compile(r"^@f(\d+)$")
ANON_PROF_RE = re.compile(r"^\$(\d+)$")

def is_number(s: str) -> bool:
    return bool(re.fullmatch(r"-?\d+(?:\.\d+)?", s))

def to_prof_temp(token: str) -> str:
    """_t7 -> ^7 ; leave others."""
    m = TEMP_CSV_RE.match(token)
    if m: return f"^{m.group(1)}"
    return token

def to_canonical_temp(token: str) -> str:
    """Normalize both ^7 and _t7 to ^7 for comparison."""
    m = TEMP_CSV_RE.match(token)
    if m: return f"^{m.group(1)}"
    m = TEMP_PROF_RE.match(token)
    if m: return f"^{m.group(1)}"
    return token

def to_prof_anon(token: str) -> str:
    """@f0 -> $0"""
    m = ANON_STUDENT_RE.match(token)
    if m: return f"${m.group(1)}"
    return token

def to_canonical_anon(token: str) -> str:
    """Normalize $0 and @f0 to $0 for comparison."""
    m = ANON_STUDENT_RE.match(token)
    if m: return f"${m.group(1)}"
    m = ANON_PROF_RE.match(token)
    if m: return f"${m.group(1)}"
    return token

def canonical_bool(token: str) -> str:
    """Map any of true/'true'/False/etc to lowercase bare 'true'/'false'."""
    t = token.strip().strip("'").strip('"').lower()
    if t in ("true", "false"):
        return t
    return token

def needs_quotes_for_key(token: str) -> bool:
    """
    Heuristic: table keys that are not numbers or temporaries/anon funcs
    should be quoted in professor's 1st form.
    """
    if is_number(token):
        return False
    if TEMP_CSV_RE.match(token) or TEMP_PROF_RE.match(token):
        return False
    if ANON_STUDENT_RE.match(token) or ANON_PROF_RE.match(token):
        return False
    # treat plain identifiers as needing quotes when used as table keys
    return True

def quote_string_if_needed(token: str, context: Optional[str] = None) -> str:
    """
    Produce a token as professor prints it:
    - booleans: 'true'/'false' (single quotes)
    - newline and other control chars: double-quoted escaped
    - table keys (context == 'table_key'): quote if heuristic says so
    - otherwise leave as-is
    """
    # booleans
    low = token.lower()
    if low in ("true", "false"):
        return f"'{low}'"

    # control chars (e.g., \n from CSV becomes actual newline char)
    if token == "\n":
        return r'"\n"'

    # explicit table-key quoting
    if context == "table_key" and needs_quotes_for_key(token):
        # avoid double quoting if already quoted
        t = token.strip('"').strip("'")
        return f'"{t}"'

    return token

# ---------- Opcode mapping (CSV -> Professor names) ----------

OPCODE_MAP = {
    "if_lt": "if_less",
    "if_gt": "if_greater",
    "if_lte": "if_lesseq",
    "if_gte": "if_greatereq",
    "if_neq": "if_noteq",
    # keep others as-is
}

def to_prof_opcode(op: str) -> str:
    return OPCODE_MAP.get(op.lower(), op.lower())

# ---------- Parsing professor (1st form) ----------

def parse_professor_lines(lines: List[str]) -> Dict[int, Dict]:
    """
    Parse professor's raw 1st-form lines:
      '13: jump 18 [line 10]'
    Returns: {quad_no: {"opcode": str, "tokens": [..], "lineno": int, "raw": str}}
    """
    out = {}
    for line in lines:
        s = line.strip()
        if not s: continue
        m = re.match(r"(\d+):\s+(\w+)\s*(.*?)\s*\[line\s+(\d+)\]\s*$", s)
        if not m:
            continue
        qno = int(m.group(1))
        opcode = m.group(2)
        rest = m.group(3).strip()
        lineno = int(m.group(4))

        # split tokens but preserve quoted chunks
        tokens = []
        buf = ""
        in_single = False
        in_double = False
        for ch in rest:
            if ch == "'" and not in_double:
                in_single = not in_single
                buf += ch
            elif ch == '"' and not in_single:
                in_double = not in_double
                buf += ch
            elif ch.isspace() and not in_single and not in_double:
                if buf:
                    tokens.append(buf)
                    buf = ""
            else:
                buf += ch
        if buf:
            tokens.append(buf)

        # drop IGNORE QUAD markers if present
        tokens = [t for t in tokens if t not in ("IGNORE", "QUAD")]

        out[qno] = {"opcode": opcode, "tokens": tokens, "lineno": lineno, "raw": s}
    return out

# ---------- Parsing & converting student's CSV (2nd -> 1st) ----------

def read_csv_rows(path: str) -> List[Dict[str, str]]:
    with open(path, newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))

def norm_csv_field(v: str) -> Optional[str]:
    if v is None: return None
    v = v.strip()
    if v == "" or v == "NA":
        return None
    return v

def csv_row_to_prof_tokens(row: Dict[str, str]) -> Tuple[str, List[str]]:
    """
    Convert a CSV row into (prof_opcode, prof_tokens_for_1st_form).
    This **does not** add [line N]; caller handles that.
    """
    op = to_prof_opcode(row["opcode"])
    # Collect raw fields (ignoring NA)
    result = norm_csv_field(row.get("result"))
    arg1 = norm_csv_field(row.get("arg1"))
    arg2 = norm_csv_field(row.get("arg2"))
    label = norm_csv_field(row.get("label"))

    # convert temps and anonymous names to professor style for printing
    def conv(tok: Optional[str]) -> Optional[str]:
        if tok is None: return None
        tok = to_prof_temp(tok)
        tok = to_prof_anon(tok)
        return tok

    result = conv(result)
    arg1 = conv(arg1)
    arg2 = conv(arg2)
    label = conv(label)

    tokens: List[str] = []

    if op == "tablesetelem":
        # CSV encodes as result=value, arg1=table, arg2=key
        # Professor prints: tablesetelem table key value
        table = arg1
        key = arg2
        val = result
        if table is not None:
            tokens.append(quote_string_if_needed(table))
        if key is not None:
            tokens.append(quote_string_if_needed(key, context="table_key"))
        if val is not None:
            # values can be booleans or strings; apply basic quoting
            val_print = quote_string_if_needed(val)
            tokens.append(val_print)
    elif op == "tablegetelem":
        # CSV: result, table, key -> Prof: result table key
        if result is not None:
            tokens.append(quote_string_if_needed(result))
        if arg1 is not None:
            tokens.append(quote_string_if_needed(arg1))
        if arg2 is not None:
            tokens.append(quote_string_if_needed(arg2, context="table_key"))
    else:
        # Generic: result, arg1, arg2, label (drop None)
        for idx, tok in enumerate((result, arg1, arg2, label)):
            if tok is None:
                continue
            ctx = None
            # Heuristic: if this is the third positional token of table ops, it may be a key,
            # but we already handled table* ops above. For general ops, only special-case booleans and \n.
            out_tok = quote_string_if_needed(tok, context=ctx)
            tokens.append(out_tok)

    return op, tokens

def csv_to_first_form_lines(csv_rows: List[Dict[str, str]]) -> Dict[int, str]:
    """
    Build {quad_no: 'N: opcode tokens... [line X]'} in professor's 1st-form style.
    """
    lines = {}
    for row in csv_rows:
        qno = int(row["quad_no"])
        first_lineno = int(row["first_lineno"])
        op, toks = csv_row_to_prof_tokens(row)

        # tweak booleans to professor's single-quoted form
        toks = [quote_string_if_needed(canonical_bool(t)) for t in toks]

        # finalize line
        body = " ".join(toks).strip()
        if body:
            s = f"{qno}: {op} {body} [line {first_lineno}]"
        else:
            s = f"{qno}: {op} [line {first_lineno}]"
        lines[qno] = s
    return lines

# ---------- Comparison (token-level, tolerant of quotes) ----------

def tokenize_prof_line_body(opcode: str, tokens: List[str]) -> List[str]:
    """
    Normalize tokens for fair comparison:
      - unify temps (^7/_t7 -> ^7)
      - unify anon funcs (@f0/$0 -> $0)
      - strip surrounding quotes
      - normalize booleans to bare 'true'/'false'
    """
    normd = []
    for t in tokens:
        # strip outer quotes
        t0 = t.strip()
        if (t0.startswith('"') and t0.endswith('"')) or (t0.startswith("'") and t0.endswith("'")):
            t0 = t0[1:-1]
        t0 = to_canonical_temp(t0)
        t0 = to_canonical_anon(t0)
        t0 = canonical_bool(t0)
        normd.append(t0)
    return normd

def compare_first_forms(prof_map: Dict[int, Dict], mine_map: Dict[int, str]) -> List[str]:
    """
    Compare professor's parsed map against our generated first-form lines.
    """
    diffs = []
    # Build tokenized version of my lines to compare
    my_parsed = parse_professor_lines([mine_map[k] for k in sorted(mine_map)])
    all_qnos = sorted(set(prof_map.keys()) | set(my_parsed.keys()))
    for q in all_qnos:
        p = prof_map.get(q)
        m = my_parsed.get(q)
        if p is None:
            diffs.append(f"Extra in mine: quad {q}\n  mine: {mine_map[q]}")
            continue
        if m is None:
            diffs.append(f"Missing in mine: quad {q}\n  prof: {p['raw']}")
            continue

        # opcode
        if p["opcode"].lower() != m["opcode"].lower():
            diffs.append(f"Opcode mismatch @ {q}\n  prof: {p['raw']}\n  mine: {m['raw']}")
            continue

        # tokens
        p_toks = tokenize_prof_line_body(p["opcode"], p["tokens"])
        m_toks = tokenize_prof_line_body(m["opcode"], m["tokens"])

        if p_toks != m_toks:
            diffs.append(
                f"Tokens mismatch @ {q}\n"
                f"  prof: {p['raw']}\n"
                f"  mine: {m['raw']}\n"
                f"  prof_norm_tokens: {p_toks}\n"
                f"  mine_norm_tokens: {m_toks}"
            )
            continue

        # line number (prof uses single [line N]; CSV has first/last; we printed first)
        if p["lineno"] != m["lineno"]:
            diffs.append(
                f"Line mismatch @ {q}\n"
                f"  prof: {p['raw']}\n"
                f"  mine:  {m['raw']}"
            )
    return diffs

# ---------- CLI ----------

def main():
    ap = argparse.ArgumentParser(description="Compare CSV quads (2nd form) to professor's 1st form.")
    ap.add_argument("--prof", required=True, help="Path to professor 1st-form file (raw quads).")
    ap.add_argument("--csv", required=True, help="Path to your CSV (2nd form).")
    ap.add_argument("--emit", help="Optional path to write the converted 1st-form output from your CSV.")
    args = ap.parse_args()

    # Read inputs
    with open(args.prof, encoding="utf-8") as f:
        prof_lines = [ln.rstrip("\n") for ln in f]

    csv_rows = read_csv_rows(args.csv)

    # Convert mine (CSV) -> first form
    mine_lines_map = csv_to_first_form_lines(csv_rows)
    mine_lines_out = [mine_lines_map[q] for q in sorted(mine_lines_map.keys())]

    if args.emit:
        with open(args.emit, "w", encoding="utf-8") as f:
            for ln in mine_lines_out:
                f.write(ln + "\n")

    # Parse professor 1st-form
    prof_map = parse_professor_lines(prof_lines)

    # Compare
    diffs = compare_first_forms(prof_map, mine_lines_map)

    print(f"# Converted my CSV -> 1st form ({len(mine_lines_out)} lines)")
    if args.emit:
        print(f"Wrote converted 1st form to: {args.emit}")
    print()
    if not diffs:
        print("✅ No differences found. (After normalization of temps, anon funcs, booleans, and quoting.)")
    else:
        print(f"❌ Found {len(diffs)} differences:\n")
        for d in diffs:
            print(d)
            print("-" * 80)

if __name__ == "__main__":
    main()
