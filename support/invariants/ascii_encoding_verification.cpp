/**
 * @file verify_ascii_encoding.cpp
 * @brief HARD-CODED FORMAL VERIFICATION OF THE ASCII CHARACTER ENCODING SET.
 * !!! DO NOT REMOVE THIS FILE !!!
 * This check is mission-critical because the ScannerAutomaton uses aggressive
 * ASCII bitfield tricks, destructive folds (c | 0x20), and pointer-arithmetic
 * hacks for performance. These hacks rely on the specific algebraic properties
 * and contiguity of the ASCII table. If this file fails to compile, the
 * environment is non-ASCII, and the scanner's logic is fundamentally broken.
 */

// --- Non-Printable Control Characters ---
static_assert('\0' == 0, "ASCII 00: NUL");
static_assert('\1' == 1, "ASCII 01: SOH");
static_assert('\2' == 2, "ASCII 02: STX");
static_assert('\3' == 3, "ASCII 03: ETX");
static_assert('\4' == 4, "ASCII 04: EOT");
static_assert('\5' == 5, "ASCII 05: ENQ");
static_assert('\6' == 6, "ASCII 06: ACK");
static_assert('\a' == 7, "ASCII 07: BEL (Alert)");
static_assert('\b' == 8, "ASCII 08: BS  (Backspace)");
static_assert('\t' == 9, "ASCII 09: TAB (Horizontal Tab)");
static_assert('\n' == 10, "ASCII 10: LF  (Line Feed)");
static_assert('\v' == 11, "ASCII 11: VT  (Vertical Tab)");
static_assert('\f' == 12, "ASCII 12: FF  (Form Feed)");
static_assert('\r' == 13, "ASCII 13: CR  (Carriage Return)");
static_assert('\16' == 14, "ASCII 14: SO");
static_assert('\17' == 15, "ASCII 15: SI");
static_assert('\20' == 16, "ASCII 16: DLE");
static_assert('\21' == 17, "ASCII 17: DC1");
static_assert('\22' == 18, "ASCII 18: DC2");
static_assert('\23' == 19, "ASCII 19: DC3");
static_assert('\24' == 20, "ASCII 20: DC4");
static_assert('\25' == 21, "ASCII 21: NAK");
static_assert('\26' == 22, "ASCII 22: SYN");
static_assert('\27' == 23, "ASCII 23: ETB");
static_assert('\30' == 24, "ASCII 24: CAN");
static_assert('\31' == 25, "ASCII 25: EM");
static_assert('\32' == 26, "ASCII 26: SUB");
static_assert('\33' == 27, "ASCII 27: ESC");
static_assert('\34' == 28, "ASCII 28: FS");
static_assert('\35' == 29, "ASCII 29: GS");
static_assert('\36' == 30, "ASCII 30: RS");
static_assert('\37' == 31, "ASCII 31: US");

// --- Printable Characters & Symbols ---
static_assert(' ' == 32, "ASCII 32: Space");
static_assert('!' == 33, "ASCII 33: !");
static_assert('\"' == 34, "ASCII 34: \"");
static_assert('#' == 35, "ASCII 35: #");
static_assert('$' == 36, "ASCII 36: $");
static_assert('%' == 37, "ASCII 37: %");
static_assert('&' == 38, "ASCII 38: &");
static_assert('\'' == 39, "ASCII 39: '");
static_assert('(' == 40, "ASCII 40: (");
static_assert(')' == 41, "ASCII 41: )");
static_assert('*' == 42, "ASCII 42: *");
static_assert('+' == 43, "ASCII 43: +");
static_assert(',' == 44, "ASCII 44: ,");
static_assert('-' == 45, "ASCII 45: -");
static_assert('.' == 46, "ASCII 46: .");
static_assert('/' == 47, "ASCII 47: /");

// --- Digits ---
static_assert('0' == 48, "ASCII 48: 0");
static_assert('1' == 49, "ASCII 49: 1");
static_assert('2' == 50, "ASCII 50: 2");
static_assert('3' == 51, "ASCII 51: 3");
static_assert('4' == 52, "ASCII 52: 4");
static_assert('5' == 53, "ASCII 53: 5");
static_assert('6' == 54, "ASCII 54: 6");
static_assert('7' == 55, "ASCII 55: 7");
static_assert('8' == 56, "ASCII 56: 8");
static_assert('9' == 57, "ASCII 57: 9");

// --- Symbols ---
static_assert(':' == 58, "ASCII 58: :");
static_assert(';' == 59, "ASCII 59: ;");
static_assert('<' == 60, "ASCII 60: <");
static_assert('=' == 61, "ASCII 61: =");
static_assert('>' == 62, "ASCII 62: >");
static_assert('?' == 63, "ASCII 63: ?");
static_assert('@' == 64, "ASCII 64: @");

// --- Uppercase Alphabet ---
static_assert('A' == 65, "ASCII 65: A");
static_assert('B' == 66, "ASCII 66: B");
static_assert('C' == 67, "ASCII 67: C");
static_assert('D' == 68, "ASCII 68: D");
static_assert('E' == 69, "ASCII 69: E");
static_assert('F' == 70, "ASCII 70: F");
static_assert('G' == 71, "ASCII 71: G");
static_assert('H' == 72, "ASCII 72: H");
static_assert('I' == 73, "ASCII 73: I");
static_assert('J' == 74, "ASCII 74: J");
static_assert('K' == 75, "ASCII 75: K");
static_assert('L' == 76, "ASCII 76: L");
static_assert('M' == 77, "ASCII 77: M");
static_assert('N' == 78, "ASCII 78: N");
static_assert('O' == 79, "ASCII 79: O");
static_assert('P' == 80, "ASCII 80: P");
static_assert('Q' == 81, "ASCII 81: Q");
static_assert('R' == 82, "ASCII 82: R");
static_assert('S' == 83, "ASCII 83: S");
static_assert('T' == 84, "ASCII 84: T");
static_assert('U' == 85, "ASCII 85: U");
static_assert('V' == 86, "ASCII 86: V");
static_assert('W' == 87, "ASCII 87: W");
static_assert('X' == 88, "ASCII 88: X");
static_assert('Y' == 89, "ASCII 89: Y");
static_assert('Z' == 90, "ASCII 90: Z");

// --- Symbols ---
static_assert('[' == 91, "ASCII 91: [");
static_assert('\\' == 92, "ASCII 92: \\");
static_assert(']' == 93, "ASCII 93: ]");
static_assert('^' == 94, "ASCII 94: ^");
static_assert('_' == 95, "ASCII 95: _");
static_assert('`' == 96, "ASCII 96: `");

// --- Lowercase Alphabet ---
static_assert('a' == 97, "ASCII 97: a");
static_assert('b' == 98, "ASCII 98: b");
static_assert('c' == 99, "ASCII 99: c");
static_assert('d' == 100, "ASCII 100: d");
static_assert('e' == 101, "ASCII 101: e");
static_assert('f' == 102, "ASCII 102: f");
static_assert('g' == 103, "ASCII 103: g");
static_assert('h' == 104, "ASCII 104: h");
static_assert('i' == 105, "ASCII 105: i");
static_assert('j' == 106, "ASCII 106: j");
static_assert('k' == 107, "ASCII 107: k");
static_assert('l' == 108, "ASCII 108: l");
static_assert('m' == 109, "ASCII 109: m");
static_assert('n' == 110, "ASCII 110: n");
static_assert('o' == 111, "ASCII 111: o");
static_assert('p' == 112, "ASCII 112: p");
static_assert('q' == 113, "ASCII 113: q");
static_assert('r' == 114, "ASCII 114: r");
static_assert('s' == 115, "ASCII 115: s");
static_assert('t' == 116, "ASCII 116: t");
static_assert('u' == 117, "ASCII 117: u");
static_assert('v' == 118, "ASCII 118: v");
static_assert('w' == 119, "ASCII 119: w");
static_assert('x' == 120, "ASCII 120: x");
static_assert('y' == 121, "ASCII 121: y");
static_assert('z' == 122, "ASCII 122: z");

// --- Final Symbols ---
static_assert('{' == 123, "ASCII 123: {");
static_assert('|' == 124, "ASCII 124: |");
static_assert('}' == 125, "ASCII 125: }");
static_assert('~' == 126, "ASCII 126: ~");
static_assert('\177' == 127, "ASCII 127: DEL");

// -----------------------------------------------------------------------------
// Linker Anchor:
// -----------------------------------------------------------------------------
// This dummy function ensures the Translation Unit is not considered
// empty by the compiler/linker. Without a visible symbol, some toolchains
// may optimize out the object file entirely during incremental builds,
// skipping the static_assert checks above.
// -----------------------------------------------------------------------------
extern "C" void _force_compilation_unit_verify_ascii() {}
