#ifndef INTERNAL_ALPHA_PARSER_LOCATIONS_H
#define INTERNAL_ALPHA_PARSER_LOCATIONS_H

#include <stdint.h> /* <cstdint> requires std:: prefix (according to standard C++). */
#include <string>
#include "parser/alpha_symbol_table.hpp"
#include "core/alpha_error_tracker.hpp"
#include "core/alpha_location.hpp"
#include "scanner/alpha_scanner_context.hpp"
#include "core/alpha_shared_interface.hpp"

extern ALPHA_YYLEX_SIGNATURE;

#define YYLLOC_DEFAULT(Current, Rhs, N)                                         \
        do                                                                      \
        {                                                                       \
                if (N)                                                          \
                {                                                               \
                        (Current).first_index_ = YYRHSLOC(Rhs, 1).first_index_; \
                        (Current).last_index_ = YYRHSLOC(Rhs, N).last_index_;   \
                }                                                               \
                else                                                            \
                {                                                               \
                        (Current).first_index_ = YYRHSLOC(Rhs, 0).last_index_;  \
                        (Current).last_index_ = YYRHSLOC(Rhs, 0).last_index_;   \
                }                                                               \
        } while (0)

static void alpha_yyerror(Alpha::ScnrCTX &scnr_ctx,
                          Alpha::PrsrCTX &prsr_ctx,
                          Alpha::SymbolTable &symbol_table,
                          Alpha::ErrorTracker &error_tracker,
                          const std::string &error_message)
{
#ifdef DEBUG_MODE
        (void)scnr_ctx;
        (void)prsr_ctx;
        (void)symbol_table;
        (void)error_tracker;
        (void)error_message;
#endif // DEBUG_MODE

        /* TODO: what else does this function do ? */
}

#endif /* INTERNAL_ALPHA_PARSER_LOCATIONS_H */