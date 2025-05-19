#ifndef ALPHA_SEMANTIC_BUILDER_HPP
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

namespace Alpha
{
        class SemanticBuilder
        {
        public:
                SemanticBuilder(ParseCtx *const parse_ctx) : parse_ctx_(parse_ctx) {}

                [[nodiscard]] ExprList *make_empty_expr_list();
                [[nodiscard]] ExprList *extend_expr_list(Expr *expr, ExprList *elist_tail);
                [[nodiscard]] Expr *make_const_nil(Location nil_loc);
                [[nodiscard]] Expr *make_const_true(Location true_loc);
                [[nodiscard]] Expr *make_const_false(Location false_loc);
                [[nodiscard]] Expr *make_const_int(i64 int_value, Location int_loc);
                [[nodiscard]] Expr *make_const_real(f64 real_value, Location real_loc);
                [[nodiscard]] Expr *make_const_string(const char *str_value, Location str_loc);
                [[nodiscard]] Expr *make_call(Expr *lvalue, ExprList *elist, Location call_loc);

                [[nodiscard]] static BlockLocation
                make_block_location(Location begin, Location end) noexcept;

        private:
                ParseCtx *const parse_ctx_;
        };

        inline ExprList *
        SemanticBuilder::make_empty_expr_list()
        {
                return new std::vector<Expr *>();
        }

        inline ExprList *
        SemanticBuilder::extend_expr_list(Expr *expr, ExprList *elist_tail)
        {
                DEBUG_SMART_ASSERT(!!expr, !!elist_tail);
                elist_tail->push_back(expr);
                return elist_tail;
        }

        inline Expr *
        SemanticBuilder::make_const_nil(Location nil_loc)
        {
                return parse_ctx_->expr_handler.make_expr_const_nil(nil_loc);
        }

        inline Expr *
        SemanticBuilder::make_const_true(Location true_loc)
        {
                return parse_ctx_->expr_handler.make_expr_const_bool(true, true_loc);
        }

        inline Expr *
        SemanticBuilder::make_const_false(Location false_loc)
        {
                return parse_ctx_->expr_handler.make_expr_const_bool(false, false_loc);
        }

        inline Expr *
        SemanticBuilder::make_const_int(i64 int_value, Location int_loc)
        {
                return parse_ctx_->expr_handler.make_expr_const_int(int_value, int_loc);
        }

        inline Expr *
        SemanticBuilder::make_const_real(f64 real_value, Location real_loc)
        {
                return parse_ctx_->expr_handler.make_expr_const_real(real_value, real_loc);
        }

        inline Expr *
        SemanticBuilder::make_const_string(const char *str_value, Location str_loc)
        {
                return parse_ctx_->expr_handler.make_expr_const_string(str_value, str_loc);
        }

        inline Expr *
        SemanticBuilder::make_call(Expr *lvalue, ExprList *elist, Location call_loc)
        {

                Expr *func_expr = parse_ctx_->expr_handler.emit_quad_if_table_item(lvalue);
                for (Expr *e : *elist)
                        parse_ctx_->quad_handler.emit_quad(
                            IOPCode::PARAM,
                            e,
                            nullptr,
                            nullptr,
                            e->location //
                        );

                DEBUG_SMART_ASSERT(!!func_expr->symbol);

                parse_ctx_->quad_handler.emit_quad(
                    IOPCode::CALL,
                    func_expr,
                    nullptr,
                    nullptr,
                    call_loc);

                Expr *getretval_expr = parse_ctx_->expr_handler.make_expr_variable(
                    parse_ctx_->new_temp(),
                    k_no_location //
                );

                parse_ctx_->quad_handler.emit_quad(
                    IOPCode::GETRETVAL,
                    nullptr,
                    nullptr,
                    getretval_expr,
                    k_no_location); // We could pass call_location, but we follow strict policy: temps have no location

                return getretval_expr;
        }

        inline BlockLocation
        SemanticBuilder::make_block_location(Location begin, Location end) noexcept
        {
                return {
                    .begin = begin,
                    .end = end,
                };
        }
} // namespace Alpha::Sem::Fns

#endif // ALPHA_SEMANTIC_ACTION_FUNCS_HPP