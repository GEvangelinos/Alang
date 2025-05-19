#ifndef ALPHA_SEMANTIC_ACTION_FUNCS_HPP
#define ALPHA_SEMANTIC_ACTION_FUNCS_HPP
#include <string>                          // for string
#include "core/alpha_error.hpp"            // for ErrorTracker
#include "core/alpha_location.hpp"         // for Location
#include "parser/alpha_parser_context.hpp" // for ParseCtx
#include "parser/alpha_symbol_table.hpp"   // for Symbol, SymbolTable

#include <list> // for list, _List_const_iterator

#include "core/alpha_error.hpp"            // for ErrorTracker, Diagnostic
#include "core/alpha_konstants.hpp"        // for k_global_scope, k_public_...
#include "core/alpha_location.hpp"         // for Location
#include "utils/misc.hpp"                  // for DEBUG_ALWAYS_INLINE
#include "core/alpha_types.hpp"            // for u32
#include "parser/_parser_common.hpp"       // for Parameter
#include "parser/alpha_parser_context.hpp" // for ParseCtx
#include "utils/format_adapter.hpp"        // for format, FMT
#include "utils/smart_assert.h"            // for DEBUG_SMART_ASSERT
#include "parser/alpha_backpatcher.hpp"

namespace Alpha::Sem::Fns
{
        inline std::vector<Expr *> *make_expr_list()
        {
                return new std::vector<Expr *>();
        }

        inline std::vector<Expr *> *make_expr_list(Expr *expr)
        {
                auto elist = make_expr_list();
                elist->push_back(expr);
                return elist;
        }

        inline Expr *make_const_nil(ParseCtx &parse_ctx)
        {
                return parse_ctx.expr_handler.make_expr_const_nil();
        }

        inline Expr *make_const_true(ParseCtx &parse_ctx)
        {
                return parse_ctx.expr_handler.make_expr_const_bool(true);
        }

        inline Expr *make_const_false(ParseCtx &parse_ctx)
        {
                return parse_ctx.expr_handler.make_expr_const_bool(false);
        }

        inline Expr *make_const_int(ParseCtx &parse_ctx, decltype(Expr::const_int) int_value)
        {
                return parse_ctx.expr_handler.make_expr_const_int(int_value);
        }

        inline Expr *make_const_real(ParseCtx &parse_ctx, decltype(Expr::const_real) real_value)
        {
                return parse_ctx.expr_handler.make_expr_const_real(real_value);
        }

        inline Expr *make_const_string(ParseCtx &parse_ctx, const char *str_value)
        {
                return parse_ctx.expr_handler.make_expr_const_string(str_value);
        }

        inline BlockLocation make_block_location(Location begin, Location end) noexcept
        {
                return {
                    .begin = begin,
                    .end = end,
                };
        }
}

#endif // ALPHA_SEMANTIC_ACTION_FUNCS_HPP