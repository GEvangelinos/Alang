#ifndef ALPHA_PARSER_SEMANTIC_MACROS_HPP
#define ALPHA_PARSER_SEMANTIC_MACROS_HPP

// #define MAKE_ARITHMETIC(SEMANTIC_BUILDER, IOPCODE) \
//         SEMANTIC_BUILDER.make_arithmetic(IOPCODE, $left, $right, @result, @left, @right);

// MACROS HERE !!
// | expr[left] PLUS expr[right] { $result = sb.make_arithmetic(AOP::ADD, $left, $right, @result, @left, @right); }
// | expr[left] MINUS expr[right] { $result = sb.make_arithmetic(AOP::SUB, $left, $right, @result, @left, @right); }
// | expr[left] MUL expr[right] { $result = sb.make_arithmetic(AOP::MUL, $left, $right, @result, @left, @right); }
// | expr[left] DIV expr[right] { $result = sb.make_arithmetic(AOP::DIV, $left, $right, @result, @left, @right); }
// | expr[left] MOD expr[right] { $result = sb.make_arithmetic(AOP::MOD, $left, $right, @result, @left, @right); }
// | expr[left] GT expr[right] { $result = sb.make_relational(AOP::IF_GREATER, $left, $right, @result, @left, @right); }
// | expr[left] GTE expr[right] { $result = sb.make_relational(AOP::IF_GREATEREQ, $left, $right, @result, @left, @right); }
// | expr[left] LT expr[right] { $result = sb.make_relational(AOP::IF_LESS, $left, $right, @result, @left, @right); }
// | expr[left] LTE expr[right] { $result = sb.make_relational(AOP::IF_LESSEQ, $left, $right, @result, @left, @right); }
// | expr[left] EQ expr[right] { $result = sb.make_relational(AOP::IF_EQ, $left, $right, @result, @left, @right); }
// | expr[left] NEQ expr[right] { $result = sb.make_relational(AOP::IF_NOTEQ, $left, $right, @result, @left, @right); }
// | expr[left] AND expr[right] { $result = sb.make_logical(AOP::AND, $left, $right, @result, @left, @right); }
// | expr[left] OR expr[right] { $result = sb.make_logical(AOP::OR, $left, $right, @result, @left, @right); }

#endif // ALPHA_PARSER_SEMANTIC_MACROS_HPP
