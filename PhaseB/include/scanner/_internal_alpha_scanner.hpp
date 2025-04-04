#ifndef ALPHA_SCANNER_INTERNALS_HPP
#define ALPHA_SCANNER_INTERNALS_HPP

#include "core/alpha_shared_interface.hpp"

#ifdef YY_DECL
#undef YY_DECL
#endif /* YY_DECL */

#define YY_DECL ALPHA_YYLEX_SIGNATURE

#define YY_USER_ACTION                                                        \
        do                                                                    \
        {                                                                     \
                alpha_yylloc.first_line_ = scanner_context.line_;             \
                alpha_yylloc.first_column_ = scanner_context.column_;         \
                alpha_yylloc.first_index_ = scanner_context.index_;           \
                                                                              \
                alpha_yylloc.last_line_ = scanner_context.line_;              \
                alpha_yylloc.last_column_ = scanner_context.column_ + yyleng; \
                alpha_yylloc.last_index_ = scanner_context.index_ + yyleng;   \
                                                                              \
                scanner_context.column_ += yyleng;                            \
                scanner_context.index_ += yyleng;                             \
        } while (0); /* Semi-Colon is not placed by flex, we place it manually. */

#endif /* ALPHA_SCANNER_INTERNALS_HPP */