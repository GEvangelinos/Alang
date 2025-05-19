#ifndef ALPHA_SEMANTIC_ACTION_FUNCS_HPP
#define ALPHA_SEMANTIC_ACTION_FUNCS_HPP
#include "core/alpha_error.hpp"            // for ErrorTracker
#include "core/alpha_location.hpp"         // for Location
#include "parser/alpha_parser_context.hpp" // for ParseCtx
#include "parser/alpha_symbol_table.hpp"   // for Symbol, SymbolTable
#include <string>                          // for string

#include <list> // for list, _List_const_iterator

#include "core/alpha_error.hpp"      // for ErrorTracker, Diagnostic
#include "core/alpha_konstants.hpp"  // for k_global_scope, k_public_...
#include "core/alpha_location.hpp"   // for Location
#include "core/alpha_types.hpp"      // for u32
#include "parser/_parser_common.hpp" // for Parameter
#include "parser/alpha_backpatcher.hpp"
#include "parser/alpha_parser_context.hpp" // for ParseCtx
#include "utils/format_adapter.hpp"        // for format, FMT
#include "utils/misc.hpp"                  // for DEBUG_ALWAYS_INLINE
#include "utils/smart_assert.h"            // for DEBUG_SMART_ASSERT
#include "parser/_parser_type_aliases.hpp"
#include "_parser_common.hpp"

namespace Alpha::SemanticFunctions
{
        [[nodiscard]] inline ExprList *make_expr_list()
        {
                return new std::vector<Expr *>();
        }

        [[nodiscard]] inline ExprList *make_expr_list(Expr *expr)
        {
                auto elist = make_expr_list();
                elist->push_back(expr);
                return elist;
        }

        [[nodiscard]] inline ExprList *extend_expr_list(Expr *expr, ExprList *exprListTail)
        {
                DEBUG_SMART_ASSERT(!!expr, !!exprListTail);
                exprListTail->push_back(expr);
                return exprListTail;
        }

        [[nodiscard]] inline Expr *make_const_nil(ParseCtx &parse_ctx, Location nil_location)
        {
                return parse_ctx.expr_handler.make_expr_const_nil(nil_location);
        }

        [[nodiscard]] inline Expr *make_const_true(ParseCtx &parse_ctx, Location true_location)
        {
                return parse_ctx.expr_handler.make_expr_const_bool(true, true_location);
        }

        [[nodiscard]] inline Expr *make_const_false(ParseCtx &parse_ctx, Location false_location)
        {
                return parse_ctx.expr_handler.make_expr_const_bool(false, false_location);
        }

        [[nodiscard]] inline Expr *
        make_const_int(ParseCtx &parse_ctx, decltype(Expr::const_int) int_value, Location int_location)
        {
                return parse_ctx.expr_handler.make_expr_const_int(int_value, int_location);
        }

        [[nodiscard]] inline Expr *
        make_const_real(ParseCtx &parse_ctx, decltype(Expr::const_real) real_value, Location real_location)
        {
                return parse_ctx.expr_handler.make_expr_const_real(real_value, real_location);
        }

        [[nodiscard]] inline Expr *make_const_string(
            ParseCtx &parse_ctx,
            const char *str_value,
            Location str_location)
        {
                return parse_ctx.expr_handler.make_expr_const_string(str_value, str_location);
        }

        [[nodiscard]] inline Expr *make_call(
            SymbolTable &st,
            ParseCtx &parse_ctx,
            Expr *lvalue,
            ExprList *elist,
            Location call_location)
        {
                Expr *func_expr = parse_ctx.expr_handler.emit_quad_if_table_item(st, lvalue);
                for (Expr *e : *elist)
                        parse_ctx.quad_handler.emit_quad(
                            IOPCode::PARAM,
                            e,
                            nullptr,
                            nullptr,
                            e->location //
                        );

                DEBUG_SMART_ASSERT(!!func_expr->symbol);

                parse_ctx.quad_handler.emit_quad(
                    IOPCode::CALL,
                    func_expr,
                    nullptr,
                    nullptr,
                    call_location);

                Expr *getretval_expr = parse_ctx.expr_handler.make_expr_variable(parse_ctx.new_temp(st), k_no_location);

                parse_ctx.quad_handler.emit_quad(
                    IOPCode::GETRETVAL,
                    nullptr,
                    nullptr,
                    getretval_expr,
                    k_no_location); // We could pass call_location, but we follow strict policy: temps have no location

                return getretval_expr;
        }

        [[nodiscard]] inline BlockLocation make_block_location(Location begin, Location end) noexcept
        {
                return {
                    .begin = begin,
                    .end = end,
                };
        }
} // namespace Alpha::Sem::Fns

#endif // ALPHA_SEMANTIC_ACTION_FUNCS_HPP