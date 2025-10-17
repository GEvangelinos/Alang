#ifndef SCANNER_PROLOGUE_HPP
#define SCANNER_PROLOGUE_HPP

#include "core//source_location_types.hpp"
#include "support/debug_tools.hpp"

#ifdef YY_USER_ACTION
#error "Macro collision detected"
#endif
#define YY_USER_ACTION                                                                                            \
    do                                                                                                            \
    {                                                                                                             \
        yylloc->begin = lexer_ctx.source_index;                                                                         \
        const auto end_result = lexer_ctx.source_index.value + yyleng;                                                  \
        DEBUG_SMART_ASSERT(alpha::support::is_in_numeric_range<alpha::SrcBufferIdx::UnderlyingType>(end_result)); \
        yylloc->end = alpha::SrcBufferIdx{static_cast<alpha::SrcBufferIdx::UnderlyingType>(end_result)};          \
        lexer_ctx.source_index.value = end_result;                                                                      \
    } while (0); /* Semi-Colon is not placed by flex, we place it manually. */

#ifdef EMIT
#error "Macro collision detected"
#endif
#define EMIT(TOK_ID)                                                                \
    do                                                                              \
    {                                                                               \
        lexer_ctx.register_token(alpha::TokenInfo{.id = (TOK_ID), .loc = *yylloc}); \
        return TOK_ID;                                                              \
    } while (0)
#endif // SCANNER_PROLOGUE_HPP
