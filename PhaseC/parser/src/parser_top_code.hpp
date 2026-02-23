#ifndef PARSER_TOP_CODE_HPP
#define PARSER_TOP_CODE_HPP

// IWYU pragma: no_include <features.h>
// IWYU pragma: no_include <stdio.h>
// IWYU pragma: no_include <stdlib.h>
// IWYU pragma: no_include <string.h>
#include <parser/alpha_parser.gen.hpp>
#include <string>                       // for basic_string, string
#include "parser/trace_logger.hpp"      // for display_trace
#include "parser/parser_context.hpp"    // for ParseCtx
#include "scanner/alpha_yylex.hpp"
#include <L1_driver/semantic_system.hpp>
using Op = alpha::ir::Opcode;

#ifndef PARSER_STACK_CAPACITY
#define PARSER_STACK_CAPACITY 10000 // Bison's default is 200
#endif

#ifndef YYINITDEPTH
#define YYINITDEPTH PARSER_STACK_CAPACITY
#endif

using Op = alpha::ir::Opcode;

#endif // PARSER_TOP_CODE_HPP
