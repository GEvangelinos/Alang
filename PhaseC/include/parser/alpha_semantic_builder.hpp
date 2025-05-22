#ifndef ALPHA_SEMANTIC_BUILDER_HPP
#define ALPHA_SEMANTIC_BUILDER_HPP
#include "core/alpha_error.hpp"            // for ErrorTracker
#include "core/alpha_location.hpp"         // for Location
#include "parser/alpha_parser_context.hpp" // for ParseCtx
#include "parser/alpha_symbol_table.hpp"   // for Symbol, SymbolTable
#include <string>                          // for string
#include <utility>

#include <list> // for list, _List_const_iterator

#include "_parser_common.hpp"
#include "core/alpha_error.hpp"      // for ErrorTracker, Diagnostic
#include "core/alpha_konstants.hpp"  // for k_global_scope, k_public_...
#include "core/alpha_location.hpp"   // for Location
#include "core/alpha_types.hpp"      // for u32
#include "parser/_parser_common.hpp" // for Parameter
#include "parser/_parser_type_aliases.hpp"
#include "parser/alpha_backpatcher.hpp"
#include "parser/alpha_parser_context.hpp" // for ParseCtx
#include "utils/format_adapter.hpp"        // for format, FMT
#include "utils/misc.hpp"                  // for DEBUG_ALWAYS_INLINE
#include "utils/smart_assert.h"            // for DEBUG_SMART_ASSERT
#include <stdexcept>

namespace // (Anonymous)
{

} // namespace

namespace Alpha
{
bool is_relational_iopcode(IOPCode iopc);
bool is_equality_iopcode(IOPCode iopc);
bool is_numeric_convertible_expr(const Expr *expr);
bool is_rvalue_expr(Expr::Type type);
constexpr const char *relational_iopc_to_string(IOPCode iopc);

struct SemanticOpts // TODO: rename?
{
        bool arithmetic_folding;
        bool realtional_folding;
        bool logical_foling;
};

class SemanticBuilder
{
public:
        const SemanticOpts sem_opts;

        SemanticBuilder(SemanticOpts sem_opts, ParseCtx &parse_ctx, SymbolTable &st,
                        ErrorTracker &et);

        [[nodiscard]] Expr *convert_to_boolean(Expr *expr, Location expr_loc);
        [[nodiscard]] Expr *make_arithmetic(IOPCode iopc, Expr *left, Expr *right,
                                            Location result_loc, Location left_loc,
                                            Location right_loc);
        [[nodiscard]] Expr *make_relational(IOPCode iopc, Expr *left, Expr *right,
                                            Location result_loc, Location left_loc,
                                            Location right_loc);
        [[nodiscard]] Expr *make_logical_or(Expr *left, Expr *right, Location result_loc,
                                            Location left_loc, Location right_loc);
        [[nodiscard]] Expr *make_logical_and(Expr *left, Expr *right, Location result_loc,
                                             Location left_loc, Location right_loc);
        [[nodiscard]] Expr *make_uminus(Expr *expr, Location term_loc, Location expr_loc);
        [[nodiscard]] Expr *make_logical_not(Expr *expr, Location result_loc);
        [[nodiscard]] ExprList *make_empty_expr_list();
        [[nodiscard]] ExprList *make_expr_list_with(Expr *expr, Location new_expr_loc);
        [[nodiscard]] ExprList *extend_expr_list_with(ExprList *expr_list, Expr *expr,
                                                      Location new_expr_loc);
        [[nodiscard]] DictList *make_dict_list_with(ExprPair *first_pair);
        [[nodiscard]] DictList *extend_dict_list_with(DictList *dict_list, ExprPair *new_pair);
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
        [[nodiscard]] Expr *make_table_item(Expr *&lvalue, const char *id, Location table_item_loc,
                                            Location id_loc);
        [[nodiscard]] Expr *make_table_item(Expr *&lvalue, Expr *expr, Location table_item_loc);
        [[nodiscard]] Expr *make_call(Expr *lvalue, ExprList *&elist, Location call_loc);
        [[nodiscard]] Expr *make_normal_call(Expr *&lvalue, ExprList *&elist, Location call_loc);
        [[nodiscard]] Expr *make_method_call(Expr *&lvalue, ExprList *&elist, Location call_loc);
        [[nodiscard]] Expr *make_iife_call(const Function *func_symbol, ExprList *&elist,
                                           Location call_loc);

        [[nodiscard]] static BlockLocation make_block_location(Location begin,
                                                               Location end) noexcept;

private:
        ParseCtx &parse_ctx_;
        [[maybe_unused]] SymbolTable &st_; // TODO: REMOVE IF UNUSED
        ErrorTracker &et_;

        void delete_expr_list(ExprList *&elist);
        void delete_dict_list(DictList *&dlist);
        void validate_lvalue_for_assignment(const Symbol *lvalue_symbol, Location assign_loc);
        [[nodiscard]] Expr *handle_table_item_assignment(Expr *lvalue, Expr *expr,
                                                         Location assign_loc);
        [[nodiscard]] Expr *handle_direct_assignment(Expr *lvalue, Expr *expr, Location assign_loc);

        static void update_expr_location(Expr *expr, Location new_expr_loc);
        [[nodiscard]] DictList *make_empty_dict_list();

        // TODO: ! this checker is the same in semantic MANAGER (merge the two..)
        // maybe split manager in background actions.
        // and builder in short actions that return stuff.. no to  much semantic checking..
        // a few emit , and few expr* create , and out the door ->>>
        void report_error_if_not_arithmetic(IOPCode iopc, const Expr *expr, Location expr_loc,
                                            OperandPosition side);
        void report_error_if_not_relational(IOPCode iopc, const Expr *expr, Location expr_loc,
                                            OperandPosition side);
        Expr *try_fold_arithmetic(IOPCode iopc, const Expr *l, const Expr *r, Location loc);
        static void report_non_arithmetic_operand(const IOPCode iopc, const Expr *expr,
                                                  const Location expr_loc,
                                                  const OperandPosition op_pos, ErrorTracker &et);
        void report_non_relational_operand(const IOPCode iopc, const Expr *expr,
                                           const Location expr_loc, const OperandPosition op_pos,
                                           ErrorTracker &et);
}; // class SemanticBuilder

inline SemanticBuilder::SemanticBuilder(SemanticOpts sem_opts, ParseCtx &parse_ctx, SymbolTable &st,
                                        ErrorTracker &et)
    : sem_opts(sem_opts), parse_ctx_(parse_ctx), st_(st), et_(et)
{}

inline Expr *SemanticBuilder::try_fold_arithmetic(const IOPCode iopc, const Expr *left,
                                                  const Expr *right, const Location loc)
{
        DEBUG_SMART_ASSERT(!!left, !!right, sem_opts.arithmetic_folding);
        using T = Expr::Type;
        auto &eh = parse_ctx_.expr_handler;
        const bool is_left_int = left->type == T::CONST_INT;
        const bool is_right_int = right->type == T::CONST_INT;
        const auto etype_to_str = [](const Expr *e) -> const char * {
                return e->type == Expr::Type::CONST_INT    ? "`int`"
                       : e->type == Expr::Type::CONST_REAL ? "`real`"
                                                           : "`unknown`";
        };

        if (is_left_int && is_right_int)
        {
                const i64 l = left->const_int, r = right->const_int;
                switch (iopc)
                {
                case IOPCode::ADD: return eh.make_expr_const_int(l + r, loc);
                case IOPCode::SUB: return eh.make_expr_const_int(l - r, loc);
                case IOPCode::MUL: return eh.make_expr_const_int(l * r, loc);
                case IOPCode::MOD: return eh.make_expr_const_int(l % r, loc);
                case IOPCode::DIV: return eh.make_expr_const_real(f64(l) / r, loc);
                default: return nullptr;
                }
        }

        // Convert to REAL (INT+REAL, REAL+INT, REAL+REAL) // No C++ safety checks
        const f64 l = is_left_int ? left->const_int : left->const_real;
        const f64 r = is_right_int ? right->const_int : right->const_real;
        switch (iopc)
        {
        case IOPCode::ADD: return eh.make_expr_const_real(l + r, loc);
        case IOPCode::SUB: return eh.make_expr_const_real(l - r, loc);
        case IOPCode::MUL: return eh.make_expr_const_real(l * r, loc);
        case IOPCode::DIV: return eh.make_expr_const_real(l / r, loc);
        case IOPCode::MOD: { // Required-block due to initialization inside case label.
                std::string error =
                    FMT::format("{} and {} constant operands are invalid to binary `operator%`",
                                etype_to_str(left), etype_to_str(right));
                et_.report_error(CTError::Type::SEMANTIC, error, loc);
                return nullptr;
        }
        [[unlikely]]
        default:
                throw std::logic_error(ATTACH_CONTEXT(
                    FMT::format("BUG:Unexpected IOPCode `{}` with operand types `{}` and `{}`",
                                to_string(iopc), etype_to_str(left), etype_to_str(right))));
        }
}

inline Expr *SemanticBuilder::make_uminus(Expr *expr, Location term_loc, Location expr_loc)
{
        DEBUG_SMART_ASSERT(!!expr);
        report_error_if_not_arithmetic(IOPCode::UMINUS, expr, expr_loc, OperandPosition::UNARY);

        auto &eh = parse_ctx_.expr_handler;
        auto &qh = parse_ctx_.quad_handler;

        switch (expr->type)
        {
        case Expr::Type::CONST_INT: return eh.make_expr_const_int(-expr->const_int, term_loc);
        case Expr::Type::CONST_REAL: return eh.make_expr_const_real(-expr->const_real, term_loc);
        default:
                Expr *arithm_expr = eh.make_expr_arithmetic(term_loc);
                qh.emit_quad(IOPCode::UMINUS, expr, nullptr, arithm_expr, term_loc);
                return arithm_expr;
        }
}

inline Expr *SemanticBuilder::convert_to_boolean(Expr *expr, Location expr_loc)
{
        DEBUG_SMART_ASSERT(!!expr);
        if (expr->type == Expr::Type::BOOLEAN_EXPR)
                return expr;

        auto &eh = parse_ctx_.expr_handler;
        auto &qh = parse_ctx_.quad_handler;

        Expr *bool_expr = eh.make_expr_boolean(expr_loc);
        Expr *true_expr = make_const_true(expr_loc); // TODO : Dude.. having so many make function
                                                     // is confusing and pointless;

        bool_expr->backpatch_info->true_list.push_back(qh.next_quad_label());
        qh.emit_quad_labelless(IOPCode::IF_EQ, expr, true_expr, nullptr, expr_loc);
        // TODO: this would be a good place to free initial expr* as its now
        // useless.. Also you could reuse old expr.. why make new all the time??
        // Like all expressions get a "face-lift"
        bool_expr->backpatch_info->false_list.push_back(qh.next_quad_label());
        qh.emit_quad_labelless(IOPCode::JUMP, nullptr, nullptr, nullptr, expr_loc);
        return bool_expr;
}

inline Expr *SemanticBuilder::make_arithmetic(IOPCode iopc, Expr *left, Expr *right,
                                              Location result_loc, Location left_loc,
                                              Location right_loc)
{
        DEBUG_SMART_ASSERT(!!left, !!right);
        report_error_if_not_arithmetic(iopc, left, left_loc, OperandPosition::LEFT);
        report_error_if_not_arithmetic(iopc, right, right_loc, OperandPosition::RIGHT);

        if (sem_opts.arithmetic_folding)
                if (Expr *folded = try_fold_arithmetic(iopc, left, right, result_loc))
                        return folded;

        Expr *result = parse_ctx_.expr_handler.make_expr_arithmetic(result_loc);
        parse_ctx_.quad_handler.emit_quad(iopc, left, right, result, result_loc);
        return result;
}

inline Expr *SemanticBuilder::make_relational(
    IOPCode iopc, Expr *left, Expr *right, Location result_loc,
    [[maybe_unused]] Location left_loc,  // TODO: If you dont do constant folding remove
    [[maybe_unused]] Location right_loc) // TODO: If you dont do constant folding remove
{
        // TODO: rename emit_quad to emit().
        DEBUG_SMART_ASSERT(!!left, !!right);
        auto &qh = parse_ctx_.quad_handler;
        auto &eh = parse_ctx_.expr_handler;

        report_error_if_not_relational(iopc, left, left_loc, OperandPosition::LEFT);
        report_error_if_not_relational(iopc, right, right_loc, OperandPosition::RIGHT);

        // TODO: If all cases where we need backpatch lists require us to push next label
        // and next label+1 we put that in the constructor. Or we make constructor ask for next
        // true and false list. Although I think its always and IOPCode (with label) and then a
        // jump.
        Expr *bool_result_expr = eh.make_expr_boolean(result_loc);

        bool_result_expr->backpatch_info->true_list.push_back(qh.next_quad_label());
        qh.emit_quad_labelless(iopc, left, right, nullptr, result_loc);
        bool_result_expr->backpatch_info->false_list.push_back(qh.next_quad_label());
        qh.emit_quad_labelless(IOPCode::JUMP, nullptr, nullptr, nullptr, result_loc);
        return bool_result_expr;
}

inline Expr *SemanticBuilder::make_logical_or(
    Expr *left, Expr *right, Location result_loc,
    [[maybe_unused]] Location left_loc,  // TODO: If you dont do constant folding remove
    [[maybe_unused]] Location right_loc) // TODO: If you dont do constant folding remove
{
        DEBUG_SMART_ASSERT(!!left, !!right);
        auto &qh = parse_ctx_.quad_handler;
        auto &eh = parse_ctx_.expr_handler;
        Expr *bool_result_expr = eh.make_expr_boolean(result_loc);

        // TODO: Semantic Manager has a patch function.. this fucks with DRY
        // So make a common backpatcher class or namespace that does this function for you
        // Maybe put it in parseCTX or in QUAD_HANDLER.
        for (u32 quad_label : left->backpatch_info->false_list)
        {
                qh.patch_quad(quad_label, parse_ctx_.cache.or_hook.next_quad_stack.top());
                parse_ctx_.cache.or_hook.next_quad_stack.pop();
        }
        left->backpatch_info->false_list.clear();

        // TODO: MAKE A CUSTOM MERGE FUNCTION this FUCKs with DRY, as in logical and we do
        // the same fucking thing... just in reverse... FOR FUCK SAKE
        left->backpatch_info->true_list.insert(left->backpatch_info->true_list.end(),
                                               right->backpatch_info->true_list.begin(),
                                               right->backpatch_info->true_list.end());
        bool_result_expr->backpatch_info = left->backpatch_info;
        bool_result_expr->backpatch_info->false_list = right->backpatch_info->false_list;

        return bool_result_expr;
}
inline Expr *SemanticBuilder::make_logical_and(
    Expr *left, Expr *right, Location result_loc,
    [[maybe_unused]] Location left_loc,  // TODO: If you dont do constant folding remove
    [[maybe_unused]] Location right_loc) // TODO: If you dont do constant folding remove
{
        auto &qh = parse_ctx_.quad_handler;
        auto &eh = parse_ctx_.expr_handler;
        Expr *bool_result_expr = eh.make_expr_boolean(result_loc);

        DEBUG_SMART_ASSERT(!!left->backpatch_info);

        // TODO: Semantic Manager has a patch function.. this fucks with DRY,
        // So make a common backpatcher class or namespace that does this function for you
        // Maybe put it in parseCTX or in QUAD_HANDLER.
        for (u32 quad_label : left->backpatch_info->true_list)
        {
                qh.patch_quad(quad_label, parse_ctx_.cache.and_hook.next_quad_stack.top());
                parse_ctx_.cache.and_hook.next_quad_stack.pop();
        }
        left->backpatch_info->true_list.clear();

        // TODO: MAKE A CUSTOM MERGE FUNCTION this FUCKs with DRY, as in logical or we do
        // the same fucking thing... just in reverse... FOR FUCK SAKE
        left->backpatch_info->false_list.insert(left->backpatch_info->false_list.end(),
                                                right->backpatch_info->false_list.begin(),
                                                right->backpatch_info->false_list.end());
        bool_result_expr->backpatch_info = left->backpatch_info;

        // TODO: FUCKING DO SOMETHING MORE EFFIECIENT that COPYING VECTOR. (maybe std::move or
        // std::swap. move sounds more effiecient!!)
        bool_result_expr->backpatch_info->true_list = right->backpatch_info->true_list;

        return bool_result_expr;
}

inline Expr *SemanticBuilder::make_logical_not(Expr *expr, Location result_loc)
{
        // TODO: Can we check if already boolexpr and reuse that? instead of making new?
        DEBUG_SMART_ASSERT(!!expr);
        auto &eh = parse_ctx_.expr_handler;
        auto &qh = parse_ctx_.quad_handler;

        Expr *bool_result_expr = eh.make_expr_boolean(result_loc);
        Expr *true_expr = make_const_true(result_loc);

        bool_result_expr->backpatch_info->true_list.push_back(qh.next_quad_label());
        qh.emit_quad_labelless(IOPCode::IF_EQ, expr, true_expr, nullptr, result_loc);
        bool_result_expr->backpatch_info->false_list.push_back(qh.next_quad_label());
        qh.emit_quad_labelless(IOPCode::JUMP, nullptr, nullptr, nullptr, result_loc);

        std::swap(bool_result_expr->backpatch_info->true_list,
                  bool_result_expr->backpatch_info->false_list);
        return bool_result_expr;
}

inline ExprList *SemanticBuilder::make_empty_expr_list()
{
        return new ExprList();
}

inline void SemanticBuilder::delete_expr_list(ExprList *&elist)
{
        delete elist;
        DEBUG_NULLIFY(elist);
}

inline void SemanticBuilder::delete_dict_list(DictList *&dlist)
{
        for (ExprPair *pair : *dlist)
                delete pair;
        delete dlist;
        DEBUG_NULLIFY(dlist);
}

inline ExprList *SemanticBuilder::make_expr_list_with(Expr *expr, const Location new_expr_loc)
{
        DEBUG_SMART_ASSERT(!!expr);
        ExprList *new_expr_list = make_empty_expr_list();
        return extend_expr_list_with(new_expr_list, expr, new_expr_loc);
}

inline ExprList *SemanticBuilder::extend_expr_list_with(ExprList *expr_list, Expr *expr,
                                                        const Location new_expr_loc)
{
        DEBUG_SMART_ASSERT(!!expr_list, !!expr);
        update_expr_location(expr, new_expr_loc);
        expr_list->push_back(expr);
        return expr_list;
}

inline Expr *SemanticBuilder::make_const_nil(Location nil_loc)
{
        return parse_ctx_.expr_handler.make_expr_const_nil(nil_loc);
}

inline Expr *SemanticBuilder::make_const_true(Location true_loc)
{
        return parse_ctx_.expr_handler.make_expr_const_bool(true, true_loc);
}

inline Expr *SemanticBuilder::make_const_false(Location false_loc)
{
        return parse_ctx_.expr_handler.make_expr_const_bool(false, false_loc);
}

inline Expr *SemanticBuilder::make_const_int(i64 int_value, Location int_loc)
{
        return parse_ctx_.expr_handler.make_expr_const_int(int_value, int_loc);
}

inline Expr *SemanticBuilder::make_const_real(f64 real_value, Location real_loc)
{
        return parse_ctx_.expr_handler.make_expr_const_real(real_value, real_loc);
}

inline Expr *SemanticBuilder::make_const_string(const char *str_value, Location str_loc)
{
        return parse_ctx_.expr_handler.make_expr_const_string(str_value, str_loc);
}

inline Expr *SemanticBuilder::make_call(Expr *lvalue, ExprList *&elist, Location call_loc)
{

        Expr *func_expr = parse_ctx_.expr_handler.emit_quad_if_table_item(lvalue);
        for (Expr *e : *elist)
                parse_ctx_.quad_handler.emit_quad(IOPCode::PARAM, e, nullptr, nullptr,
                                                  e->location //
                );

        DEBUG_SMART_ASSERT(!!func_expr->symbol);

        parse_ctx_.quad_handler.emit_quad(IOPCode::CALL, func_expr, nullptr, nullptr, call_loc);

        Expr *getretval_expr = parse_ctx_.expr_handler.make_expr_variable(parse_ctx_.new_temp(),
                                                                          k_no_location //
        );

        parse_ctx_.quad_handler.emit_quad(
            IOPCode::GETRETVAL, nullptr, nullptr, getretval_expr,
            k_no_location); // We could pass call_location, but we follow strict policy: temps have
                            // no location

        delete_expr_list(elist);
        return getretval_expr;
}

inline Expr *SemanticBuilder::resolve_lvalue_to_primary(Expr *lvalue)
{
        return parse_ctx_.expr_handler.emit_quad_if_table_item(lvalue);
}

inline Expr *SemanticBuilder::resolve_assign_expr(Expr *lvalue, Expr *expr,
                                                  const Location assign_loc)
{
        DEBUG_SMART_ASSERT(!!lvalue, !!expr);

        validate_lvalue_for_assignment(lvalue->symbol, assign_loc);
        if (lvalue->type == Expr::Type::TABLE_ITEM)
                return handle_table_item_assignment(lvalue, expr, assign_loc);
        return handle_direct_assignment(lvalue, expr, assign_loc);
}

inline Expr *SemanticBuilder::make_table_list(ExprList *&elist, Location table_list_loc)
{
        DEBUG_SMART_ASSERT(!!elist);
        Expr *new_table_expr = parse_ctx_.expr_handler.make_expr_new_table(table_list_loc);
        parse_ctx_.quad_handler.emit_quad(IOPCode::TABLECREATE, nullptr, nullptr, new_table_expr,
                                          table_list_loc);

        // Emit list's items.
        u32 list_index = 0;
        for (auto expr_it = elist->crbegin(); expr_it != elist->crend(); ++expr_it)
        {
                Expr *index_expr = parse_ctx_.expr_handler.make_expr_const_int(
                    list_index++,
                    (*expr_it)->location // //TODO: you could remove as index its unseen is source
                                         // code, and locations is ment to point to source code,
                                         // Except if we think of it, as "cause-of-existance"
                                         // Like I exist dude to this thing there...
                );
                parse_ctx_.quad_handler.emit_quad(IOPCode::TABLESETELEM, index_expr, *expr_it,
                                                  new_table_expr,
                                                  (*expr_it)->location //
                );
        }
        delete_expr_list(elist);
        return new_table_expr;
}

inline Expr *SemanticBuilder::make_table_dict(DictList *&dlist, Location table_dict_loc)
{
        Expr *new_table_expr = parse_ctx_.expr_handler.make_expr_new_table(table_dict_loc);
        parse_ctx_.quad_handler.emit_quad(IOPCode::TABLECREATE, nullptr, nullptr, new_table_expr,
                                          table_dict_loc //
        );

        for (auto it = dlist->crbegin(); it != dlist->crend(); ++it)
        {
                parse_ctx_.quad_handler.emit_quad(IOPCode::TABLESETELEM, (*it)->first,
                                                  (*it)->second, new_table_expr,
                                                  k_no_location //
                );
        }
        delete_dict_list(dlist);
        return new_table_expr;
}

inline Expr *SemanticBuilder::make_program_function(const Function *function_symbol)
{
        return parse_ctx_.expr_handler.make_expr_program_function(function_symbol);
}

inline ExprPair *SemanticBuilder::make_expr_pair(Expr *first, Expr *second)
{
        return new ExprPair(first, second);
}

inline DictList *SemanticBuilder::make_empty_dict_list()
{
        return new DictList{};
}

inline DictList *SemanticBuilder::make_dict_list_with(ExprPair *first_element)
{
        DEBUG_SMART_ASSERT(!!first_element);
        DictList *new_dict_list = make_empty_dict_list();
        new_dict_list->push_back(first_element);
        return new_dict_list;
}

inline DictList *SemanticBuilder::extend_dict_list_with(DictList *dict_list, ExprPair *new_pair)
{
        DEBUG_SMART_ASSERT(!!dict_list, !!new_pair);
        dict_list->push_back(new_pair);
        return dict_list;
}

inline Expr *SemanticBuilder::make_table_item(Expr *&lvalue, const char *id,
                                              Location table_item_loc, Location id_loc)
{
        return parse_ctx_.expr_handler.make_expr_table_item(lvalue, id, id_loc, table_item_loc);
}

inline Expr *SemanticBuilder::make_table_item(Expr *&lvalue, Expr *expr, Location table_item_loc)
{
        return parse_ctx_.expr_handler.make_expr_table_item(lvalue, expr, table_item_loc);
}

inline Expr *SemanticBuilder::make_normal_call(Expr *&lvalue, ExprList *&elist, Location call_loc)
{
        lvalue = parse_ctx_.expr_handler.emit_quad_if_table_item(lvalue);
        return make_call(lvalue, elist, call_loc);
}

inline Expr *SemanticBuilder::make_method_call(Expr *&lvalue, ExprList *&elist, Location call_loc)
{
        lvalue = parse_ctx_.expr_handler.emit_quad_if_table_item(lvalue);
        elist->push_back(lvalue);

        // TODO: Understand what this name should be... And also understand first emit_quad_if...
        Expr *temp_var = parse_ctx_.expr_handler.make_expr_table_item(
            lvalue, parse_ctx_.cache.method_call_id.id, parse_ctx_.cache.method_call_id.id_location,
            k_no_location //
        );

        lvalue = parse_ctx_.expr_handler.emit_quad_if_table_item(temp_var);
        return make_call(lvalue, elist, call_loc);
}

inline Expr *SemanticBuilder::make_iife_call(const Function *func_symbol, ExprList *&elist,
                                             const Location call_loc)
{
        Expr *func_expr = parse_ctx_.expr_handler.make_expr_program_function(func_symbol);
        return make_call(func_expr, elist, call_loc);
}

inline void SemanticBuilder::validate_lvalue_for_assignment(const Symbol *lvalue_symbol,
                                                            const Location assign_loc)
{
        DEBUG_SMART_ASSERT(!!lvalue_symbol);
        if (Symbol::is_modifiable_symbol(lvalue_symbol))
                return;
        if (lvalue_symbol->type == Symbol::Type::LIBRARY_FUNCTION)
        {
                std::string error =
                    FMT::format("assignment of library function `{}`", lvalue_symbol->name);
                et_.report_error(CTError::Type::SEMANTIC, error, assign_loc);
        }
        else if (lvalue_symbol->type == Symbol::Type::PROGRAM_FUNCTION)
        {
                std::string error = FMT::format("assignment of function `{}`", lvalue_symbol->name);
                std::string note = FMT::format("function {} declared here", lvalue_symbol->name);
                et_.report_error(CTError::Type::SEMANTIC, error, assign_loc, note,
                                 lvalue_symbol->location);
        }
}

inline BlockLocation SemanticBuilder::make_block_location(Location begin, Location end) noexcept
{
        return {
            .begin = begin,
            .end = end,
        };
}

inline Expr *SemanticBuilder::handle_table_item_assignment(Expr *lvalue, Expr *expr,
                                                           Location assign_loc)
{
        parse_ctx_.quad_handler.emit_quad(IOPCode::TABLESETELEM, lvalue, lvalue->index, expr,
                                          assign_loc);

        Expr *rvalue = parse_ctx_.expr_handler.emit_quad_if_table_item(lvalue);
        return parse_ctx_.expr_handler.make_expr_assign(rvalue, assign_loc);
}

inline Expr *SemanticBuilder::handle_direct_assignment(Expr *lvalue, Expr *expr,
                                                       Location assign_loc)
{

        parse_ctx_.quad_handler.emit_quad(
            IOPCode::ASSIGN, expr, nullptr, lvalue,
            assign_loc); // TODO (NOT IMPORTANT): location (can we construct it from expr (to catch
                         // whole assignment expression?))

        Expr *assignExpr = parse_ctx_.expr_handler.make_expr_assign(parse_ctx_.new_temp(),
                                                                    assign_loc //
        );

        parse_ctx_.quad_handler.emit_quad(IOPCode::ASSIGN, lvalue, nullptr, assignExpr,
                                          k_no_location);

        return assignExpr;
}

inline void SemanticBuilder::update_expr_location(Expr *expr, Location new_expr_loc)
{
        expr->location = new_expr_loc;
}

inline void SemanticBuilder::report_non_arithmetic_operand(const IOPCode iopc, const Expr *expr,
                                                           const Location expr_loc,
                                                           const OperandPosition op_pos,
                                                           ErrorTracker &et)
{
        std::string error;

        using OP = OperandPosition;
        if (op_pos == OP::UNARY)
                error = "operand of unary `-` is never arithmetic";
        else if (op_pos == OP::LEFT || op_pos == OP::RIGHT)
                error = FMT::format("`{}` operand of arithmetic operator `{}` is never arithmetic ",
                                    to_string(op_pos), relational_iopc_to_string(iopc));
        else [[unlikely]]
                throw std::logic_error(ATTACH_CONTEXT("Invalid arithmetic OperandPosition"));

        const std::string note =
            FMT::format("operand's expression type: `{}`", to_string(expr->type));

        et.report_error(CTError::Type::SEMANTIC, error, expr_loc, note, expr_loc);
}

inline void SemanticBuilder::report_non_relational_operand(const IOPCode iopc, const Expr *expr,
                                                           const Location expr_loc,
                                                           const OperandPosition op_pos,
                                                           ErrorTracker &et)
{
        using OP = OperandPosition;
        if (op_pos != OP::LEFT && op_pos != OP::RIGHT) [[unlikely]]
                throw std::logic_error(ATTACH_CONTEXT("Invalid relational OperandPosition "));

        const std::string error =
            FMT::format("`{}` operand of relational operator `{}` is never arithmetic",
                        to_string(op_pos), relational_iopc_to_string(iopc));
        const std::string note =
            FMT::format("operand's expression type: `{}`", to_string(expr->type));
        et.report_error(CTError::Type::SEMANTIC, error, expr_loc, note, expr_loc);
}

inline void SemanticBuilder::report_error_if_not_arithmetic(const IOPCode iopc, const Expr *expr,
                                                            const Location expr_loc,
                                                            const OperandPosition op_pos)
{
        DEBUG_SMART_ASSERT(!!expr);
        if (is_numeric_convertible_expr(expr))
                return;
        SemanticBuilder::report_non_arithmetic_operand(iopc, expr, expr_loc, op_pos, et_);
}

inline void SemanticBuilder::report_error_if_not_relational(const IOPCode iopc, const Expr *expr,
                                                            const Location expr_loc,
                                                            const OperandPosition op_pos)
{
        DEBUG_SMART_ASSERT(!!expr, is_relational_iopcode(iopc),
                           op_pos == OperandPosition::LEFT || op_pos == OperandPosition::RIGHT //
        );

        // In Alpha everything convertible to bool.
        // And operators == and != convert their operands to bool.
        if (is_equality_iopcode(iopc))
                return;
        // If here operator IOPCode is:  < <= > >=
        if (is_numeric_convertible_expr(expr))
                return;

        SemanticBuilder::report_non_relational_operand(iopc, expr, expr_loc, op_pos, et_);
}

inline bool is_relational_iopcode(const IOPCode iopc)
{
        switch (iopc)
        {
        case IOPCode::IF_EQ:
        case IOPCode::IF_NOTEQ:
        case IOPCode::IF_GREATER:
        case IOPCode::IF_GREATEREQ:
        case IOPCode::IF_LESS:
        case IOPCode::IF_LESSEQ: return true;
        default: return false;
        }
}

inline bool is_equality_iopcode(const Alpha::IOPCode iopc)
{
        return iopc == IOPCode::IF_EQ || iopc == IOPCode::IF_NOTEQ;
}

inline bool is_numeric_convertible_expr(const Alpha::Expr *const expr)
{
        using AET = Alpha::Expr::Type;
        switch (expr->type)
        {
        case AET::ARITHMETIC_EXPR:
        case AET::ASSIGN_EXPR:
        case AET::CONST_INT:
        case AET::CONST_REAL:
        case AET::TABLE_ITEM:
        case AET::VARIABLE: return true;
        default: return false;
        }
}

inline bool is_rvalue_expr(const Alpha::Expr::Type type)
{
        using AET = Alpha::Expr::Type;
        switch (type)
        {
        case AET::CONST_BOOL:
        case AET::CONST_INT:
        case AET::CONST_NIL:
        case AET::CONST_REAL:
        case AET::CONST_STRING:
        case AET::LIBRARY_FUNCTION:
        case AET::PROGRAM_FUNCTION: return true;
        default: return false;
        }
}

constexpr const char *relational_iopc_to_string(const Alpha::IOPCode iopc)
{
        DEBUG_SMART_ASSERT(is_relational_iopcode(iopc));
        switch (iopc)
        {
        case IOPCode::IF_LESS: return "<";
        case IOPCode::IF_GREATER: return ">";
        case IOPCode::IF_LESSEQ: return "<=";
        case IOPCode::IF_GREATEREQ: return ">=";
        case IOPCode::IF_EQ: return "==";
        case IOPCode::IF_NOTEQ: return "!=";
        default:
                throw std::logic_error(ATTACH_CONTEXT(
                    "Expected strictly an IOPCode corresponding to a relational operator"));
        }
}
} // namespace Alpha

#endif // ALPHA_SEMANTIC_BUILDER_HPP