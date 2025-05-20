
%%
const:
  NIL    { $const = sb.make_const_nil(@NIL); }
| TRUE   { $const = sb.make_const_true(@TRUE); }
| FALSE  { $const = sb.make_const_false(@FALSE); }
| INT    { $const = sb.make_const_int($INT, @INT); }
| REAL   { $const = sb.make_const_real($REAL, @REAL); }
| STRING { $const = sb.make_const_string($STRING, @STRING); delete[] $STRING; $STRING = nullptr; }
;

// TODO. STOP LEXER FROM COMPYING THE STRING.. IT USESLESS and MAKES US NEED EXTRA Deaclocation bookeeping 

ifStmt:
  IF LEFT_PAREN expr RIGHT_PAREN stmt %prec THEN
| IF LEFT_PAREN expr RIGHT_PAREN stmt ELSE stmt
;

%%
