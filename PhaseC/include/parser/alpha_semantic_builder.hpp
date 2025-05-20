#ifndef ALPHA_SEMANTIC_BUILDER_HPP
#define ALPHA_SEMANTIC_BUILDER_HPP
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
                SemanticBuilder(ParseCtx &parse_ctx, SymbolTable &st, ErrorTracker &et);

                [[nodiscard]] ExprList *make_empty_expr_list();
                [[nodiscard]] ExprList *make_expr_list_with(Expr *expr, Location new_expr_loc);
                [[nodiscard]] ExprList *extend_expr_list_with(
                    ExprList *expr_list,
                    Expr *expr,
                    Location new_expr_loc);
                [[nodiscard]] DictList *make_dict_list_with(ExprPair *first_pair);
                [[nodiscard]] DictList *extend_dict_list_with(
                    DictList *dict_list,
                    ExprPair *new_pair);
                [[nodiscard]] Expr *make_const_nil(Location nil_loc);
                [[nodiscard]] Expr *make_const_true(Location true_loc);
                [[nodiscard]] Expr *make_const_false(Location false_loc);
                [[nodiscard]] Expr *make_const_int(i64 int_value, Location int_loc);
                [[nodiscard]] Expr *make_const_real(f64 real_value, Location real_loc);
                [[nodiscard]] Expr *make_const_string(const char *str_value, Location str_loc);
                [[nodiscard]] Expr *resolve_lvalue_to_primary(Expr *lvalue);
                [[nodiscard]] Expr *resolve_assign_expr(Expr *lvalue, Expr *expr, Location assign_loc);
                [[nodiscard]] Expr *make_table_list(ExprList *&elist, Location table_list_loc);
                [[nodiscard]] Expr *make_table_dict(DictList *&dlist, Location table_dict_loc);
                [[nodiscard]] Expr *make_program_function(const Function *function_symbol);
                [[nodiscard]] ExprPair *make_expr_pair(Expr *first, Expr *second);
                [[nodiscard]] Expr *make_table_item(
                    Expr *&lvalue,
                    const char *id,
                    Location table_item_loc,
                    Location id_loc);
                [[nodiscard]] Expr *make_table_item(
                    Expr *&lvalue,
                    Expr *expr,
                    Location table_item_loc);
                [[nodiscard]] Expr *make_call(Expr *lvalue, ExprList *&elist, Location call_loc);
                [[nodiscard]] Expr *make_normal_call(
                    Expr *&lvalue,
                    ExprList *&elist,
                    Location call_loc);
                [[nodiscard]] Expr *make_method_call(
                    Expr *&lvalue,
                    ExprList *&elist,
                    Location call_loc);
                [[nodiscard]] Expr *make_iife_call(
                    const Function *func_symbol,
                    ExprList *&elist,
                    Location call_loc);

                [[nodiscard]] static BlockLocation
                make_block_location(Location begin, Location end) noexcept;

        private:
                ParseCtx &parse_ctx_;
                [[maybe_unused]] SymbolTable &st_; // TODO: REMOVE IF UNUSED
                ErrorTracker &et_;

                void delete_expr_list(ExprList *&elist);
                void delete_dict_list(DictList *&dlist);
                void validate_lvalue_for_assignment(const Symbol *lvalue_symbol, Location assign_loc);
                [[nodiscard]] Expr *
                handle_table_item_assignment(Expr *lvalue, Expr *expr, Location assign_loc);
                [[nodiscard]] Expr *
                handle_direct_assignment(Expr *lvalue, Expr *expr, Location assign_loc);

                static void update_expr_location(Expr *expr, Location new_expr_loc);
                [[nodiscard]] DictList *make_empty_dict_list();
        }; // class SemanticBuilder

        inline SemanticBuilder::SemanticBuilder(ParseCtx &parse_ctx, SymbolTable &st, ErrorTracker &et)
            : parse_ctx_(parse_ctx), st_(st), et_(et) {}

        inline ExprList *
        SemanticBuilder::make_empty_expr_list()
        {
                return new ExprList();
        }

        inline void
        SemanticBuilder::delete_expr_list(ExprList *&elist)
        {
                delete elist;
                DEBUG_NULLIFY(elist);
        }

        inline void
        SemanticBuilder::delete_dict_list(DictList *&dlist)
        {
                for (ExprPair *pair : *dlist)
                        delete pair;
                delete dlist;
                DEBUG_NULLIFY(dlist);
        }

        inline ExprList *
        SemanticBuilder::make_expr_list_with(Expr *expr, const Location new_expr_loc)
        {
                DEBUG_SMART_ASSERT(!!expr);
                ExprList *new_expr_list = make_empty_expr_list();
                return extend_expr_list_with(new_expr_list, expr, new_expr_loc);
        }

        inline ExprList *
        SemanticBuilder::extend_expr_list_with(
            ExprList *expr_list,
            Expr *expr,
            const Location new_expr_loc)
        {
                DEBUG_SMART_ASSERT(!!expr_list, !!expr);
                update_expr_location(expr, new_expr_loc);
                expr_list->push_back(expr);
                return expr_list;
        }

        inline Expr *
        SemanticBuilder::make_const_nil(Location nil_loc)
        {
                return parse_ctx_.expr_handler.make_expr_const_nil(nil_loc);
        }

        inline Expr *
        SemanticBuilder::make_const_true(Location true_loc)
        {
                return parse_ctx_.expr_handler.make_expr_const_bool(true, true_loc);
        }

        inline Expr *
        SemanticBuilder::make_const_false(Location false_loc)
        {
                return parse_ctx_.expr_handler.make_expr_const_bool(false, false_loc);
        }

        inline Expr *
        SemanticBuilder::make_const_int(i64 int_value, Location int_loc)
        {
                return parse_ctx_.expr_handler.make_expr_const_int(int_value, int_loc);
        }

        inline Expr *
        SemanticBuilder::make_const_real(f64 real_value, Location real_loc)
        {
                return parse_ctx_.expr_handler.make_expr_const_real(real_value, real_loc);
        }

        inline Expr *
        SemanticBuilder::make_const_string(const char *str_value, Location str_loc)
        {
                return parse_ctx_.expr_handler.make_expr_const_string(str_value, str_loc);
        }

        inline Expr *
        SemanticBuilder::make_call(Expr *lvalue, ExprList *&elist, Location call_loc)
        {

                Expr *func_expr = parse_ctx_.expr_handler.emit_quad_if_table_item(lvalue);
                for (Expr *e : *elist)
                        parse_ctx_.quad_handler.emit_quad(
                            IOPCode::PARAM,
                            e,
                            nullptr,
                            nullptr,
                            e->location //
                        );

                DEBUG_SMART_ASSERT(!!func_expr->symbol);

                parse_ctx_.quad_handler.emit_quad(
                    IOPCode::CALL,
                    func_expr,
                    nullptr,
                    nullptr,
                    call_loc);

                Expr *getretval_expr = parse_ctx_.expr_handler.make_expr_variable(
                    parse_ctx_.new_temp(),
                    k_no_location //
                );

                parse_ctx_.quad_handler.emit_quad(
                    IOPCode::GETRETVAL,
                    nullptr,
                    nullptr,
                    getretval_expr,
                    k_no_location); // We could pass call_location, but we follow strict policy: temps have no location

                delete_expr_list(elist);
                return getretval_expr;
        }

        inline Expr *
        SemanticBuilder::resolve_lvalue_to_primary(Expr *lvalue)
        {
                return parse_ctx_.expr_handler.emit_quad_if_table_item(lvalue);
        }

        inline Expr *
        SemanticBuilder::resolve_assign_expr(
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

        inline Expr *
        SemanticBuilder::make_table_list(ExprList *&elist, Location table_list_loc)
        {
                DEBUG_SMART_ASSERT(!!elist);
                Expr *new_table_expr = parse_ctx_.expr_handler.make_expr_new_table(table_list_loc);
                parse_ctx_.quad_handler.emit_quad(
                    IOPCode::TABLECREATE,
                    nullptr,
                    nullptr,
                    new_table_expr,
                    table_list_loc //
                );

                // Emit list's items.
                u32 list_index = 0;
                for (auto expr_it = elist->crbegin(); expr_it != elist->crend(); ++expr_it)
                {
                        Expr *index_expr = parse_ctx_.expr_handler.make_expr_const_int(
                            list_index++,
                            (*expr_it)->location // //TODO: you could remove as index its unseen is source code,
                                                 // and locations is ment to point to source code,
                                                 // Except if we think of it, as "cause-of-existance"
                                                 // Like I exist dude to this thing there...
                        );
                        parse_ctx_.quad_handler.emit_quad(
                            IOPCode::TABLESETELEM,
                            index_expr,
                            *expr_it,
                            new_table_expr,
                            (*expr_it)->location //
                        );
                }
                delete_expr_list(elist);
                return new_table_expr;
        }

        inline Expr *
        SemanticBuilder::make_table_dict(DictList *&dlist, Location table_dict_loc)
        {
                Expr *new_table_expr = parse_ctx_.expr_handler.make_expr_new_table(table_dict_loc);
                parse_ctx_.quad_handler.emit_quad(
                    IOPCode::TABLECREATE,
                    nullptr,
                    nullptr,
                    new_table_expr,
                    table_dict_loc //
                );

                for (auto it = dlist->crbegin(); it != dlist->crend(); ++it)
                {
                        parse_ctx_.quad_handler.emit_quad(
                            IOPCode::TABLESETELEM,
                            (*it)->first,
                            (*it)->second,
                            new_table_expr,
                            k_no_location //
                        );
                }
                delete_dict_list(dlist);
                return new_table_expr;
        }

        inline Expr *
        SemanticBuilder::make_program_function(const Function *function_symbol)
        {
                return parse_ctx_.expr_handler.make_expr_program_function(function_symbol);
        }

        inline ExprPair *
        SemanticBuilder::make_expr_pair(Expr *first, Expr *second)
        {
                return new ExprPair(first, second);
        }

        inline DictList *
        SemanticBuilder::make_empty_dict_list()
        {
                return new DictList{};
        }

        inline DictList *
        SemanticBuilder::make_dict_list_with(ExprPair *first_element)
        {
                DEBUG_SMART_ASSERT(!!first_element);
                DictList *new_dict_list = make_empty_dict_list();
                new_dict_list->push_back(first_element);
                return new_dict_list;
        }

        inline DictList *
        SemanticBuilder::extend_dict_list_with(DictList *dict_list, ExprPair *new_pair)
        {
                DEBUG_SMART_ASSERT(!!dict_list, !!new_pair);
                dict_list->push_back(new_pair);
                return dict_list;
        }

        inline Expr *
        SemanticBuilder::make_table_item(
            Expr *&lvalue,
            const char *id,
            Location table_item_loc,
            Location id_loc)
        {
                return parse_ctx_.expr_handler.make_expr_table_item(lvalue, id, id_loc, table_item_loc);
        }

        inline Expr *
        SemanticBuilder::make_table_item(
            Expr *&lvalue,
            Expr *expr,
            Location table_item_loc)
        {
                return parse_ctx_.expr_handler.make_expr_table_item(lvalue, expr, table_item_loc);
        }

        inline Expr *
        SemanticBuilder::make_normal_call(Expr *&lvalue, ExprList *&elist, Location call_loc)
        {
                lvalue = parse_ctx_.expr_handler.emit_quad_if_table_item(lvalue);
                return make_call(lvalue, elist, call_loc);
        }

        inline Expr *
        SemanticBuilder::make_method_call(
            Expr *&lvalue,
            ExprList *&elist,
            Location call_loc)
        {
                lvalue = parse_ctx_.expr_handler.emit_quad_if_table_item(lvalue);
                elist->push_back(lvalue);

                // TODO: Understand what this name should be... And also understand first emit_quad_if...
                Expr *temp_var = parse_ctx_.expr_handler.make_expr_table_item(
                    lvalue,
                    parse_ctx_.cache.method_call_id.id,
                    parse_ctx_.cache.method_call_id.id_location,
                    k_no_location //
                );

                lvalue = parse_ctx_.expr_handler.emit_quad_if_table_item(temp_var);
                return make_call(lvalue, elist, call_loc);
        }

        inline Expr *
        SemanticBuilder::make_iife_call(
            const Function *func_symbol,
            ExprList *&elist,
            const Location call_loc)
        {
                Expr *func_expr = parse_ctx_.expr_handler.make_expr_program_function(func_symbol);
                return make_call(func_expr, elist, call_loc);
        }

        inline void
        SemanticBuilder::validate_lvalue_for_assignment(
            const Symbol *lvalue_symbol,
            const Location assign_loc)
        {
                DEBUG_SMART_ASSERT(!!lvalue_symbol);
                if (Symbol::is_modifiable_symbol(lvalue_symbol))
                        return;
                if (lvalue_symbol->type == Symbol::Type::LIBRARY_FUNCTION)
                {
                        std::string error = FMT::format("assignment of library function `{}`", lvalue_symbol->name);
                        et_.report_error(CTError::Type::SEMANTIC, error, assign_loc);
                }
                else if (lvalue_symbol->type == Symbol::Type::PROGRAM_FUNCTION)
                {
                        std::string error = FMT::format("assignment of function `{}`", lvalue_symbol->name);
                        std::string note = FMT::format("function {} declared here", lvalue_symbol->name);
                        et_.report_error(CTError::Type::SEMANTIC, error, assign_loc, note, lvalue_symbol->location);
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
                parse_ctx_.quad_handler.emit_quad(
                    IOPCode::TABLESETELEM,
                    lvalue,
                    lvalue->index,
                    expr,
                    assign_loc);

                Expr *rvalue = parse_ctx_.expr_handler.emit_quad_if_table_item(lvalue);
                return parse_ctx_.expr_handler.make_expr_assign(rvalue, assign_loc);
        }

        inline Expr *
        SemanticBuilder::handle_direct_assignment(
            Expr *lvalue,
            Expr *expr,
            Location assign_loc)
        {

                parse_ctx_.quad_handler.emit_quad(
                    IOPCode::ASSIGN,
                    expr,
                    nullptr,
                    lvalue,
                    assign_loc); // TODO (NOT IMPORTANT): location (can we construct it from expr (to catch whole assignment expression?))

                Expr *assignExpr = parse_ctx_.expr_handler.make_expr_assign(
                    parse_ctx_.new_temp(),
                    assign_loc //
                );

                parse_ctx_.quad_handler.emit_quad(
                    IOPCode::ASSIGN,
                    lvalue,
                    nullptr,
                    assignExpr,
                    k_no_location);

                return assignExpr;
        }

        inline void
        SemanticBuilder::update_expr_location(Expr *expr, Location new_expr_loc)
        {
                expr->location = new_expr_loc;
        }
} // namespace Alpha

#endif // ALPHA_SEMANTIC_BUILDER_HPP