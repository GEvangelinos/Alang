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
                [[nodiscard]] Expr *transform_lvalue_to_primary(Expr *lvalue);

                [[nodiscard]] static BlockLocation
                make_block_location(Location begin, Location end) noexcept;

        private:
                ParseCtx *const parse_ctx_;

                void validate_lvalue_for_assignment(const Symbol *lvalue_symbol, Location assign_loc);
                [[nodiscard]] Expr *
                handle_table_item_assignment(Expr *lvalue, Expr *expr, Location assign_loc);
                [[nodiscard]] Expr *
                handle_direct_assignment(Expr *lvalue, Expr *expr, Location assign_loc);
                [[nodiscard]] Expr *
                assignExpr__lvalue_assign_expr(Expr *lvalue, Expr *expr, Location assign_loc);
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

        inline Expr *
        SemanticBuilder::transform_lvalue_to_primary(Expr *lvalue)
        {
                return parse_ctx_->expr_handler.emit_quad_if_table_item(lvalue);
        }

        inline void
        SemanticBuilder::validate_lvalue_for_assignment(
            const Symbol *lvalue_symbol,
            const Location assign_loc)
        {
                DEBUG_SMART_ASSERT(!!lvalue_symbol);
                if (is_modifiable_symbol(lvalue_symbol))
                        return;
                if (lvalue_symbol->type == Symbol::Type::LIBRARY_FUNCTION)
                {
                        std::string error = FMT::format("assignment of library function `{}`", lvalue_symbol->name);
                        parse_ctx_->et->report_error(CTError::Type::SEMANTIC, error, assign_loc);
                }
                else if (lvalue_symbol->type == Symbol::Type::PROGRAM_FUNCTION)
                {
                        std::string error = FMT::format("assignment of function `{}`", lvalue_symbol->name);
                        std::string note = FMT::format("function {} declared here", lvalue_symbol->name);
                        parse_ctx_->et->report_error(CTError::Type::SEMANTIC, error, assign_loc, note, lvalue_symbol->location);
                }
        }

        inline BlockLocation
        SemanticBuilder::make_block_location(Location begin, Location end) noexcept
        {
                return {
                    .begin = begin,
                    .end = end,
                };
        }

        inline Expr *
        SemanticBuilder::handle_table_item_assignment(
            Expr *lvalue,
            Expr *expr,
            Location assign_loc)
        {
                parse_ctx_->quad_handler.emit_quad(
                    IOPCode::TABLESETELEM,
                    lvalue,
                    lvalue->index,
                    expr,
                    assign_loc);

                Expr *rvalue = parse_ctx_->expr_handler.emit_quad_if_table_item(lvalue);
                return parse_ctx_->expr_handler.make_expr_assign(rvalue, assign_loc);
        }

        inline Expr *
        SemanticBuilder::handle_direct_assignment(
            Expr *lvalue,
            Expr *expr,
            Location assign_loc)
        {

                parse_ctx_->quad_handler.emit_quad(
                    IOPCode::ASSIGN,
                    expr,
                    nullptr,
                    lvalue,
                    assign_loc); // TODO (NOT IMPORTANT): location (can we construct it from expr (to catch whole assignment expression?))

                Expr *assignExpr = parse_ctx_->expr_handler.make_expr_assign(
                    parse_ctx_->new_temp(),
                    assign_loc //
                );

                parse_ctx_->quad_handler.emit_quad(
                    IOPCode::ASSIGN,
                    lvalue,
                    nullptr,
                    assignExpr,
                    k_no_location);

                return assignExpr;
        }

        inline Expr *
        SemanticBuilder::assignExpr__lvalue_assign_expr(
            Expr *lvalue,
            Expr *expr,
            const Location assign_loc)
        {
                DEBUG_SMART_ASSERT(!!lvalue, !!expr);

                validate_lvalue_for_assignment(lvalue->symbol, assign_loc);
                if (lvalue->type == Expr::Type::TABLE_ITEM)
                        return handle_table_item_assignment(lvalue, expr, assign_loc);
                return handle_direct_assignment(lvalue, expr, assign_loc);
        }
} // namespace Alpha::Sem::Fns

#endif // ALPHA_SEMANTIC_ACTION_FUNCS_HPP