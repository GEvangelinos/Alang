#ifndef INTERNAL_ALPHA_PARSER_LOCATIONS_H
#define INTERNAL_ALPHA_PARSER_LOCATIONS_H

#include <stdint.h> /* <cstdint> requires std:: prefix (according to standard C++). */
#include <string>
#include "core/symbolTable.hpp"
#include "core/errorTracker.hpp"
#include "core/alphaDefs.hpp"

int alpha_yylex(Alpha::SymbolTable &symbol_table, Alpha::InputBufferContext &input_buffer_context);

#define YYLLOC_DEFAULT(Current, Rhs, N)                                         \
        do                                                                      \
        {                                                                       \
                if (N)                                                          \
                {                                                               \
                        (Current).first_line = YYRHSLOC(Rhs, 1).first_line;     \
                        (Current).first_column = YYRHSLOC(Rhs, 1).first_column; \
                        (Current).first_index = YYRHSLOC(Rhs, 1).first_index;   \
                        (Current).last_line = YYRHSLOC(Rhs, N).last_line;       \
                        (Current).last_column = YYRHSLOC(Rhs, N).last_column;   \
                        (Current).last_index = YYRHSLOC(Rhs, N).last_index;     \
                }                                                               \
                else                                                            \
                {                                                               \
                        (Current).first_line = (Current).last_line =            \
                            YYRHSLOC(Rhs, 0).last_line;                         \
                        (Current).first_column = (Current).last_column =        \
                            YYRHSLOC(Rhs, 0).last_column;                       \
                        (Current).first_index = (Current).last_index =          \
                            YYRHSLOC(Rhs, 0).last_index;                        \
                }                                                               \
        } while (0)


// FIXME: If bison files are not singular... This will cause linking problems. (So far we are good...)
void alpha_yyerror(Alpha::SymbolTable &symbol_table,
                   Alpha::InputBufferContext &input_buffer_context,
                   std::string errorMessage)
{
        symbol_table.errorTracker.registerCompileTimeError(new Alpha::SyntaxError(input_buffer_context.line, 0, errorMessage));
        /* TODO: what else does this function do ? */
}

#endif /* INTERNAL_ALPHA_PARSER_LOCATIONS_H */