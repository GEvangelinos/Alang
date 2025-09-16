#include <L2_semantic_subsystems/expr_builders.hpp>
#include <diagnostics/diagnostic_reporter.gen.hpp>
#include "parser/internal_typedefs.hpp"

#ifdef CYA_MODE
#define FORCE_ASSIGNMENT_TEMPS 1
#else
#define FORCE_ASSIGNMENT_TEMPS 0
#endif

namespace
{
using namespace alpha;

[[nodiscard]] bool validate_lvalue(
    DiagnosticReporter *const dr,
    const Expr *const expr,
    const SourceLocation full_expr_loc,
    const char *op_name,
    const char *op_symbol,
    const char *lvalue_subject
)
{
    DEBUG_SMART_ASSERT(!!dr, !!expr, !!op_name, !!op_symbol, !!lvalue_subject);
    DEBUG_SMART_ASSERT(!!*op_name, !!*op_symbol, !!*lvalue_subject);
    if (expr->is_lvalue_type() && expr->is_rvalue_casted())
    {
        dr->report_operator_on_lvalue_casted_to_rvalue(
            op_name, op_symbol, full_expr_loc, expr->loc);
        return false;
    }
    if (!expr->is_lvalue())
    {
        dr->report_operator_requires_lvalue(
            op_name, op_symbol, lvalue_subject, expr->type, full_expr_loc, expr->loc);
        return false;
    }
    return true;
}

[[nodiscard]] bool validate_direct_lvalue(
    DiagnosticReporter *const dr,
    const Expr *const expr,
    const SourceLocation full_expr_loc,
    const char *op_name,
    const char *op_symbol,
    const char *lvalue_subject)

{
    DEBUG_SMART_ASSERT(!!expr);
    DEBUG_SMART_ASSERT(
        expr->type != Expr::Type::TABLE_ITEM &&
        "validate_direct_lvalue: TABLE_ITEM is not a direct lvalue (use table-item path)"
    );

    if (!validate_lvalue(dr, expr, full_expr_loc, op_name, op_symbol, lvalue_subject))
        return false;
    if (expr->has_temp_symbol())
    {
        dr->report_operator_requires_non_temp_lvalue(
            op_name, op_symbol, lvalue_subject, expr->type, full_expr_loc, expr->loc);
        return false;
    }
    return true;
}
} // namespace

namespace alpha
{
AggregateBuilder::AggregateBuilder(const SemanticSystemServices &ss_services)
    :DISPATCH_TARGET(ss_services) {}

AggregateBuilder::Restricted::Restricted(const SemanticSystemServices &ss_services)
    : SemanticSubsystem(ss_services) {}

AssignBuilder::AssignBuilder(
    AssignBuilder::Options &&options,
    const SemanticSystemServices &ss_services)
    : DISPATCH_TARGET(std::move(options), ss_services) {}

AssignBuilder::Restricted::Restricted(
    AssignBuilder::Options &&options,
    const SemanticSystemServices &ss_services)
    : SemanticSubsystem(ss_services),
      options_(options) {}

BasicBuilder::BasicBuilder(
    BasicBuilder::Options &&options,
    const SemanticSystemServices &ss_services)
    : DISPATCH_TARGET(std::move(options), ss_services) {}

BasicBuilder::Restricted::Restricted(
    BasicBuilder::Options &&options,
    const SemanticSystemServices &ss_services)
    : SemanticSubsystem(ss_services), options_(options) {}

CallBuilder::CallBuilder(const SemanticSystemServices &ss_services)
    : DISPATCH_TARGET(ss_services) {}

CallBuilder::Restricted::Restricted(const SemanticSystemServices &ss_services)
    : SemanticSubsystem(ss_services),
      method_call_draft_(std::string(), k_no_loc) {}

ConstBuilder::ConstBuilder(const SemanticSystemServices &ss_services)
    : DISPATCH_TARGET(ss_services) {}

ConstBuilder::Restricted::Restricted(const SemanticSystemServices &ss_services)
    : SemanticSubsystem(ss_services) {}

FunctionBuilder::FunctionBuilder(const SemanticSystemServices &ss_services)
    : DISPATCH_TARGET(ss_services) {}

FunctionBuilder::Restricted::Restricted(const SemanticSystemServices &ss_services)
    : SemanticSubsystem(ss_services),
      function_draft_(std::string()) {}

TableAccessBuilder::TableAccessBuilder(const SemanticSystemServices &ss_services)
    : DISPATCH_TARGET(ss_services) {}

TableAccessBuilder::Restricted::Restricted(const SemanticSystemServices &ss_services)
    : SemanticSubsystem(ss_services) {}

ExprList *
AggregateBuilder::Restricted::extend_expr_list(
    ExprList *const elist,
    const Expr *const next)
{
    DEBUG_SMART_ASSERT(!!elist, !!next);
    const Expr *const materialized_next_expr = ss_bridge_->materialize_if_table_item(next);
    ss_bridge_->finalize_bool_expr(materialized_next_expr);
    #ifndef CYA_MODE
    if (!draft_.table_literal_stack.empty() &&
        parse_ctx_->temp_ctx_handler.current_critical_region().has_value() &&
        parse_ctx_->temp_ctx_handler.current_critical_region().value() ==
        TempCtxHandler::CriticalRegion::TABLE)
    {
        auto &top_elist_ctx = draft_.table_literal_stack.top();
        const Expr *const index_expr = expr_maker_->make_const_int_expr(
            next->loc,
            top_elist_ctx.current_list_index++
        );
        quad_handler_->emit_next(
            ir::Opcode::TABLESETELEM,
            top_elist_ctx.current_table_expr,
            index_expr,
            materialized_next_expr,
            materialized_next_expr->loc
        );
        parse_ctx_->temp_ctx_handler.reset_to_checkpoint();
    }
    else if (parse_ctx_->temp_ctx_handler.current_critical_region().has_value() &&
             parse_ctx_->temp_ctx_handler.current_critical_region().value() ==
             TempCtxHandler::CriticalRegion::FORLOOP_CLAUSE)
    {
        parse_ctx_->temp_ctx_handler.reset_to_checkpoint();
    }
    #endif
    elist->push_back(materialized_next_expr);
    return elist;
}

DictList *
AggregateBuilder::Restricted::extend_dict_list(
    DictList *const dlist,

    const ExprPair *const next_pair
)
{
    DEBUG_SMART_ASSERT(!!dlist, !!next_pair);
    #ifndef CYA_MODE
    if (!draft_.table_literal_stack.empty())
    {
        const auto [key, value] = *next_pair;
        const SourceLocation pair_loc = merge(key->loc, value->loc);
        quad_handler_->emit_next(
            ir::Opcode::TABLESETELEM,
            draft_.table_literal_stack.top().current_table_expr,
            key,
            value,
            pair_loc
        );
        reset_temps_if_temp_operand(key, value);
    }
    #endif
    dlist->push_back(next_pair);
    return dlist;
}

// Passed by reference to nullify after deletion -- avoids leaving a dangling pointer.
void
AggregateBuilder::Restricted::delete_dict_list(DictList *dlist)
{
    // Note: Do NOT delete the expressions in ExprPair -- those are handler by ExprMaker.
    for (const ExprPair *pair: *dlist)
        delete pair; // Shallow delete, it does NOT delete the expressions it's holding.
    delete dlist;
}

void
CallBuilder::Restricted::begin_call()
{
    #ifndef CYA_MODE
    parse_ctx_->temp_ctx_handler.enter_critical_region(TempCtxHandler::CriticalRegion::CALL);
    parse_ctx_->temp_ctx_handler.push_checkpoint_barrier();
    #endif
    parse_ctx_->call_ctx_handler.enter_call();
}

void
CallBuilder::Restricted::end_call()
{
    parse_ctx_->call_ctx_handler.exit_call();
    #ifndef CYA_MODE
    parse_ctx_->temp_ctx_handler.pop_checkpoint_barrier();
    DEBUG_SMART_ASSERT(parse_ctx_->temp_ctx_handler.current_critical_region().has_value());
    DEBUG_SMART_ASSERT(
        parse_ctx_->temp_ctx_handler.current_critical_region().value()== TempCtxHandler::
        CriticalRegion::CALL);
    parse_ctx_->temp_ctx_handler.exit_critical_region();
    #endif
}

void
AggregateBuilder::Restricted::initiate_table_literal(const SourceLocation table_list_loc)
{
    #ifndef CYA_MODE
    const NewTableExpr *const new_table_expr = expr_maker_->make_new_table_expr(table_list_loc);
    parse_ctx_->temp_ctx_handler.push_checkpoint();
    parse_ctx_->temp_ctx_handler.enter_critical_region(TempCtxHandler::CriticalRegion::TABLE);
    quad_handler_->emit_next(
        ir::Opcode::TABLECREATE, new_table_expr, nullptr, nullptr, table_list_loc);
    draft_.table_literal_stack.emplace(new_table_expr);
    #endif
}

const Expr *AggregateBuilder::Restricted::extract_table_literal_consuming(ExprList *elist)
{
    return extract_table_literal_consuming_impl(
        elist, &AggregateBuilder::Restricted::delete_expr_list);
}

const Expr *AggregateBuilder::Restricted::extract_table_literal_consuming(DictList *dlist)
{
    DEBUG_SMART_ASSERT(
        draft_.table_literal_stack.top().current_list_index == 0 &&
        "In dictionary construction list_index should be not used, it should remain 0"
    );
    return extract_table_literal_consuming_impl(
        dlist, &AggregateBuilder::Restricted::delete_dict_list);
}

template<typename ListT>
const Expr *AggregateBuilder::Restricted::extract_table_literal_consuming_impl(
    ListT *list,
    void (*deleter)(ListT *))
{
    DEBUG_SMART_ASSERT(!draft_.table_literal_stack.empty());

    const Expr *const retval = draft_.table_literal_stack.top().current_table_expr;
    draft_.table_literal_stack.pop();
    parse_ctx_->temp_ctx_handler.reset_to_checkpoint();
    parse_ctx_->temp_ctx_handler.pop_checkpoint();

    DEBUG_SMART_ASSERT(parse_ctx_->temp_ctx_handler.current_critical_region().has_value());
    DEBUG_SMART_ASSERT(
        parse_ctx_->temp_ctx_handler.current_critical_region().value() ==
        TempCtxHandler::CriticalRegion::TABLE
    );
    parse_ctx_->temp_ctx_handler.exit_critical_region();

    deleter(list);
    return retval;
}

#ifdef CYA_MODE
const Expr *
AggregateBuilder::Restricted::build_table_list_consuming(
    ExprList *elist,
    const SourceLocation table_list_loc)
{
    DEBUG_SMART_ASSERT(!!elist);
    auto *const qh = quad_handler_; // Short alias for readability.

    const NewTableExpr *const new_table_expr = expr_maker_->make_new_table_expr(table_list_loc);
    qh->emit_next(ir::Opcode::TABLECREATE, new_table_expr, nullptr, nullptr, table_list_loc);

    // Emit exprlist's items.
    u32 list_index = 0;
    for (auto expr_it = elist->cbegin(); expr_it != elist->cend(); ++expr_it)
    {
        const Expr *const list_item = *expr_it;
        const SourceLocation list_item_loc = list_item->loc;
        const Expr *const idx_expr = expr_maker_->make_const_int_expr(list_item_loc, list_index++);
        qh->emit_next(ir::Opcode::TABLESETELEM, new_table_expr, idx_expr, list_item, list_item_loc);
    }

    // Delete elist after use — it must not be used again
    AggregateBuilder::Restricted::delete_expr_list(elist);

    return new_table_expr;
}

const Expr *
AggregateBuilder::Restricted::build_table_dict_consuming(
    DictList *dlist,
    const SourceLocation table_dict_loc)
{
    DEBUG_SMART_ASSERT(!!dlist);
    auto *const qh = quad_handler_; // Short alias for readability.

    const Expr *const new_table_expr = expr_maker_->make_new_table_expr(table_dict_loc);
    qh->emit_next(ir::Opcode::TABLECREATE, new_table_expr, nullptr, nullptr, table_dict_loc);

    // Emit dict's items.
    for (auto it = dlist->cbegin(); it != dlist->cend(); ++it)
    {
        const Expr *const key = (*it)->first;
        const Expr *const value = (*it)->second;
        const SourceLocation pair_loc = merge(key->loc, value->loc);
        qh->emit_next(ir::Opcode::TABLESETELEM, new_table_expr, key, value, pair_loc);
    }

    // Delete elist after use — it must not be used again
    delete_dict_list(dlist);

    return new_table_expr;
}
#endif

const Expr *
AssignBuilder::Restricted::build_assignment(
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    if (!validate_assignment(lhs, result_loc))
        return nullptr;

    const Expr *const materialized_rhs = ss_bridge_->materialize_if_table_item(rhs);
    ss_bridge_->finalize_bool_expr(materialized_rhs);

    return AssignBuilder::Restricted::is_direct_target_expr(lhs)
           ? handle_direct_assignment(lhs, materialized_rhs, result_loc)
           : handle_table_item_assignment(lhs, materialized_rhs, result_loc);
}

const Expr *
AssignBuilder::Restricted::build_pre_inc(const Expr *const expr, const SourceLocation result_loc)
{
    return build_inc_dec<OpVariant::PRE, IncPolicy>(expr, result_loc);
}

const Expr *
AssignBuilder::Restricted::build_post_inc(const Expr *const expr, const SourceLocation result_loc)
{
    return build_inc_dec<OpVariant::POST, IncPolicy>(expr, result_loc);
}

const Expr *
AssignBuilder::Restricted::build_pre_dec(const Expr *const expr, const SourceLocation result_loc)
{
    return build_inc_dec<OpVariant::PRE, DecPolicy>(expr, result_loc);
}

const Expr *
AssignBuilder::Restricted::build_post_dec(const Expr *const expr, const SourceLocation result_loc)
{
    return build_inc_dec<OpVariant::POST, DecPolicy>(expr, result_loc);
}

bool
AssignBuilder::Restricted::is_direct_target_expr(const Expr *expr)
{
    return expr->type != Expr::Type::TABLE_ITEM;
}

bool
AssignBuilder::Restricted::validate_assignment(
    const Expr *const lhs,
    const SourceLocation assign_loc)
{
    DEBUG_SMART_ASSERT(!!lhs);
    if (lhs->type == Expr::Type::LIBRARY_FUNCTION)
    {
        const auto *const func_symbol = static_cast<const LibFuncExpr *>(lhs)->func_symbol;
        dr_->report_assign_to_libfunc(func_symbol->name, assign_loc);
        return false;
    }
    if (lhs->type == Expr::Type::PROGRAM_FUNCTION)
    {
        const auto *const func_symbol = static_cast<const ProgFuncExpr *>(lhs)->func_symbol;
        dr_->report_assign_to_func(func_symbol->name, assign_loc, func_symbol->loc);
        return false;
    }

    const auto validator = AssignBuilder::Restricted::is_direct_target_expr(lhs)
                           ? &validate_direct_lvalue
                           : &validate_lvalue;
    return validator(dr_, lhs, assign_loc, "assignment", "=", "left operand");
}

// TODO: do we propagate assignment of assignment like x = y = z = 5? If NOT
// We might need to let Expr::Type::ASSIGN_EXPR
bool
AssignBuilder::Restricted::try_record_const_expr(const Expr *const lvalue, const Expr *const rvalue)
{
    DEBUG_SMART_ASSERT(!!lvalue, !!rvalue);
    DEBUG_SMART_ASSERT(
        !options_.record_constant_variables &&
        "Recording values of constant variables is OFF, shouldn't be called"
    );
    if (lvalue->type != Expr::Type::VARIABLE)
        return false;

    const auto *const var_symbol = static_cast<const VariableExpr *>(lvalue)->var_symbol;
    if (!rvalue->is_const())
    {
        SymbolTable::clear_const_expr(var_symbol);
        return false;
    }
    SymbolTable::attach_const_expr(var_symbol, static_cast<const ConstExpr *>(rvalue));
    return true;
}

inline const Expr *
AssignBuilder::Restricted::handle_direct_assignment(
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    if (options_.record_constant_variables)
        if (try_record_const_expr(lhs, rhs))
            return lhs; // Now lvalue's symbol carries rvalue.

    quad_handler_->emit_next(ir::Opcode::ASSIGN, lhs, rhs, nullptr, result_loc);

    // In calls, force each (x=val) to yield its arg value; avoid C’s unspecified arg order
    if (FORCE_ASSIGNMENT_TEMPS || parse_ctx_->call_ctx_handler.is_in_call())
    {
        const Expr *const temp = expr_maker_->make_assign_expr(result_loc, parse_ctx_->new_temp());
        quad_handler_->emit_next(ir::Opcode::ASSIGN, temp, lhs, nullptr, result_loc);
        return temp;
    }
    return lhs;
}

const Expr *
AssignBuilder::Restricted::handle_table_item_assignment(
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    DEBUG_SMART_ASSERT(lhs->type == Expr::Type::TABLE_ITEM);

    const auto *const ti = static_cast<const TableItemExpr *>(lhs);
    quad_handler_->emit_next(ir::Opcode::TABLESETELEM, ti, ti->index, rhs, result_loc);

    // We resurface the assigned element of table to allow chained assignment. Ex.: a = b.c = d;
    const Expr *temp_var = ss_bridge_->materialize_if_table_item(lhs); // !CERTAIN EMIT!

    DEBUG_SMART_ASSERT(temp_var->type == Expr::Type::VARIABLE);
    const VarSymbol *temp_symbol = static_cast<const VariableExpr *>(temp_var)->var_symbol;
    return expr_maker_->make_assign_expr(result_loc, temp_symbol);
}

template<AssignBuilder::Restricted::OpVariant op_variant, typename Policy>
const Expr *
AssignBuilder::Restricted::build_inc_dec(const Expr *const expr, const SourceLocation result_loc)
{
    static_assert(std::is_same_v<Policy, IncPolicy> || std::is_same_v<Policy, DecPolicy>);

    const auto validator = AssignBuilder::Restricted::is_direct_target_expr(expr)
                           ? &validate_direct_lvalue
                           : &validate_lvalue;
    if (!validator(dr_, expr, result_loc, Policy::op_name, Policy::op_symbol, "operand"))
        return nullptr;
    if constexpr (op_variant == OpVariant::PRE)
        return handle_pre_inc_dec<Policy>(expr, result_loc);
    else if constexpr (op_variant == OpVariant::POST)
        return handle_post_inc_dec<Policy>(expr, result_loc);
    else
        static_assert([]() { return false; }(), "build_inc_dec(): Unknown OpVariant");
}

template<typename Policy>
const Expr *
AssignBuilder::Restricted::handle_pre_inc_dec(
    const Expr *const expr,
    const SourceLocation result_loc)
{
    static_assert(std::is_same_v<Policy, IncPolicy> || std::is_same_v<Policy, DecPolicy>);
    DEBUG_SMART_ASSERT(!!expr);
    auto *const qh = quad_handler_; // Short alias for readability.

    if (expr->type == Expr::Type::TABLE_ITEM)
    {
        const auto *const ti_lvalue = static_cast<const TableItemExpr *>(expr);
        const Expr *const result = ss_bridge_->materialize_if_table_item(ti_lvalue); // EMITS!
        qh->emit_next(Policy::opc, result, result, &k_static_int_1_expr, result_loc);
        qh->emit_next(ir::Opcode::TABLESETELEM, ti_lvalue, ti_lvalue->index, result, result_loc);
        return DEBUG_REQUIRE_PTR(result);
    }
    qh->emit_next(Policy::opc, expr, expr, &k_static_int_1_expr, result_loc);
    if (FORCE_ASSIGNMENT_TEMPS || parse_ctx_->call_ctx_handler.is_in_call())
    {
        const Expr *const result = expr_maker_->make_arithmetic_expr(result_loc);
        qh->emit_next(ir::Opcode::ASSIGN, result, expr, nullptr, result_loc);
        return DEBUG_REQUIRE_PTR(result);
    }
    return DEBUG_REQUIRE_PTR(expr);
}

template<typename Policy>
const Expr *
AssignBuilder::Restricted::handle_post_inc_dec(const Expr *lvalue, const SourceLocation result_loc)
{
    static_assert(std::is_same_v<Policy, IncPolicy> || std::is_same_v<Policy, DecPolicy>);
    auto *const qh = quad_handler_; // Short alias for readability.

    const Expr *result = expr_maker_->make_variable_expr(result_loc, parse_ctx_->new_temp());
    if (lvalue->type == Expr::Type::TABLE_ITEM)
    {
        const auto *const ti_lvalue = static_cast<const TableItemExpr *>(lvalue);
        const Expr *ti = ss_bridge_->materialize_if_table_item(lvalue);
        qh->emit_next(ir::Opcode::ASSIGN, result, ti, nullptr, result_loc);
        qh->emit_next(Policy::opc, ti, ti, &k_static_int_1_expr, result_loc);
        qh->emit_next(ir::Opcode::TABLESETELEM, ti_lvalue, ti_lvalue->index, ti, result_loc);
    }
    else
    {
        qh->emit_next(ir::Opcode::ASSIGN, result, lvalue, nullptr, result_loc);
        qh->emit_next(Policy::opc, lvalue, lvalue, &k_static_int_1_expr, result_loc);
    }
    return result;
}

const Expr *
BasicBuilder::Restricted::prepare_logical_operand_expr(const Expr *expr)
{
    expr = ss_bridge_->materialize_if_table_item(expr);
    return normalize_to_bool_expr(expr);
}

void
BasicBuilder::Restricted::mark_short_circuit_jump_point()
{
    short_circuit_jump_stack_.push(quad_handler_->next_quad_label());
}

const Expr *
BasicBuilder::Restricted::build_uminus(
    const Expr *expr,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!expr);

    expr = ss_bridge_->materialize_if_table_item(expr);

    if (!validate_arithmetic_expr(ir::Opcode::UMINUS, expr, OperandSide::UNARY))
        goto skip_opt;
    if (const auto optimized = expr_optimizer_->try_optimize<ir::Opcode::UMINUS>(result_loc, expr))
        return optimized;

skip_opt:
    reset_temps_if_temp_operand(expr);
    const ArithmeticExpr *const arithmetic_expr = expr_maker_->make_arithmetic_expr(result_loc);
    quad_handler_->emit_next(ir::Opcode::UMINUS, arithmetic_expr, expr, nullptr, result_loc);
    return arithmetic_expr;
}

const Expr *
BasicBuilder::Restricted::build_arithmetic(
    const ir::Opcode opc,
    const Expr *lhs,
    const Expr *rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);

    lhs = ss_bridge_->materialize_if_table_item(lhs);
    rhs = ss_bridge_->materialize_if_table_item(rhs);

    // Always build IR. On validation errors we report error diagnostics and never export bad IR.
    const bool valid_lhs = validate_arithmetic_expr(opc, lhs, OperandSide::LEFT);
    const bool valid_rhs = validate_arithmetic_expr(opc, rhs, OperandSide::RIGHT);
    if (!(valid_lhs && valid_rhs))
        goto skip_opt;
    // Warn on CT div-by-zero, skip folding; VM must still handle division-by-zero at runtime.
    if (!validate_possible_division(opc, rhs, result_loc))
        goto skip_opt;

    if (const auto optimized = this->try_optimize_arithmetic_expr(opc, lhs, rhs, result_loc))
        return optimized;

skip_opt:
    reset_temps_if_temp_operand(lhs, rhs);
    const ArithmeticExpr *const arithmetic_expr = expr_maker_->make_arithmetic_expr(result_loc);
    quad_handler_->emit_next(opc, arithmetic_expr, lhs, rhs, result_loc);
    return arithmetic_expr;
}

const Expr *
BasicBuilder::Restricted::build_relational(
    const ir::Opcode opc,
    const Expr *lhs,
    const Expr *rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);

    lhs = ss_bridge_->materialize_if_table_item(lhs);
    rhs = ss_bridge_->materialize_if_table_item(rhs);

    // In case of == and != the expr need to be finalized.
    ss_bridge_->finalize_bool_expr(lhs);
    ss_bridge_->finalize_bool_expr(rhs);

    // Always build IR. On validation errors we report error diagnostics and never export bad IR.
    const bool valid_lhs = validate_relational_expr(opc, lhs, OperandSide::LEFT);
    const bool valid_rhs = validate_relational_expr(opc, rhs, OperandSide::RIGHT);
    if (!(valid_lhs && valid_rhs))
        goto skip_opt;

    if (const auto optimized = this->try_optimize_relational_expr(opc, lhs, rhs, result_loc))
        return optimized;

skip_opt:
    const BoolExpr *result_expr = expr_maker_->make_bool_expr(result_loc);
    result_expr->true_list.push_back(quad_handler_->next_quad_label());
    quad_handler_->emit_labelless(opc, nullptr, lhs, rhs, result_loc);
    result_expr->false_list.push_back(quad_handler_->next_quad_label());
    quad_handler_->emit_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, result_loc);
    return result_expr;
}

const Expr *
BasicBuilder::Restricted::build_logical_not(const Expr *const expr, const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!expr);
    DEBUG_SMART_ASSERT(expr->is_bool_or_const_bool());

    if (const auto optimized = expr_optimizer_->try_optimize<ir::Opcode::NOT>(result_loc, expr))
        return optimized;
    // Sanity check, CONST_BOOL must be consumed by the optimizer.
    DEBUG_SMART_ASSERT(expr->is_bool_or_const_bool());

    reset_temps_if_temp_operand(expr);
    const BoolExpr *const bool_result_expr = expr_maker_->make_bool_expr(result_loc);
    bool_result_expr->true_list = static_cast<const BoolExpr *>(expr)->false_list;
    bool_result_expr->false_list = static_cast<const BoolExpr *>(expr)->true_list;
    return bool_result_expr;
}

const Expr *
BasicBuilder::Restricted::build_logical_and(
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    DEBUG_SMART_ASSERT(lhs->is_bool_or_const_bool(), rhs->is_bool_or_const_bool());

    if (const auto optimized = expr_optimizer_->try_optimize<ir::Opcode::AND>(result_loc, lhs, rhs))
        return optimized;

    reset_temps_if_temp_operand(lhs, rhs);
    return build_short_circuit_bool_expr<AndShortCircuitPolicy>(lhs, rhs, result_loc);
}

const Expr *
BasicBuilder::Restricted::build_logical_or(
    const Expr *lhs,
    const Expr *rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    DEBUG_SMART_ASSERT(lhs->is_bool_or_const_bool(), rhs->is_bool_or_const_bool());

    if (const auto optimized = expr_optimizer_->try_optimize<ir::Opcode::OR>(result_loc, lhs, rhs))
        return optimized;

    reset_temps_if_temp_operand(lhs, rhs);
    return build_short_circuit_bool_expr<OrShortCircuitPolicy>(lhs, rhs, result_loc);
}

template<typename Policy>
const Expr *
BasicBuilder::Restricted::build_short_circuit_bool_expr(
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation result_loc)
{
    static_assert(
        std::is_same_v<Policy, OrShortCircuitPolicy> ||
        std::is_same_v<Policy, AndShortCircuitPolicy>,
        "Unknown backpatching policy"
    );

    DEBUG_SMART_ASSERT(lhs->type == Expr::Type::BOOL_EXPR && rhs->type == Expr::Type::BOOL_EXPR);
    const BoolExpr *const lhs_bool = static_cast<const BoolExpr *>(lhs);
    const BoolExpr *const rhs_bool = static_cast<const BoolExpr *>(rhs);
    const BoolExpr *const bool_result_expr = expr_maker_->make_bool_expr(result_loc);

    // Patching left side.
    DEBUG_SMART_ASSERT(!short_circuit_jump_stack_.empty());
    for (const LabelID quad_label: Policy::backpatch_list(lhs_bool))
        quad_handler_->patch_quad(quad_label, short_circuit_jump_stack_.top());
    short_circuit_jump_stack_.pop();
    Policy::backpatch_list(lhs_bool).clear();

    // Merging right side.
    auto &lhs_merge = Policy::merge_lhs_list(lhs_bool);            //lhs_bool->false_list
    auto &rhs_merge = Policy::merge_rhs_list(rhs_bool);            //rhs_bool->false_list
    auto &result_merge = Policy::merge_lhs_list(bool_result_expr); //lhs_bool->false_list

    // We could use merge_rhs too
    result_merge.reserve(lhs_merge.size() + rhs_merge.size());
    result_merge.insert(result_merge.end(), lhs_merge.begin(), lhs_merge.end());
    result_merge.insert(result_merge.end(), rhs_merge.begin(), rhs_merge.end());

    Policy::assign_list(bool_result_expr) = Policy::assign_list(rhs_bool);
    return bool_result_expr;
}

const Expr *
BasicBuilder::Restricted::normalize_to_bool_expr(const Expr *const expr)
{
    DEBUG_SMART_ASSERT(!!expr);
    auto *const qh = quad_handler_; // Short alias for readability.

    if (expr->type == Expr::Type::BOOL_EXPR)
        return expr;

    if (options_.fold_static_bools)
        if (expr->is_static())
            return SemUtils::as_bool(expr)
                   ? expr_maker_->make_const_bool_expr(expr->loc, true)
                   : expr_maker_->make_const_bool_expr(expr->loc, false);

    const BoolExpr *const bool_expr = expr_maker_->make_bool_expr(expr->loc);
    bool_expr->true_list.push_back(qh->next_quad_label());
    qh->emit_labelless(ir::Opcode::IF_EQ, nullptr, expr, &k_static_true_expr, expr->loc);
    bool_expr->false_list.push_back(qh->next_quad_label());
    qh->emit_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, expr->loc);

    return bool_expr;
}

bool
BasicBuilder::Restricted::validate_arithmetic_expr(
    const ir::Opcode opc,
    const Expr *expr,
    const OperandSide op_side)
{
    DEBUG_SMART_ASSERT(!!expr);
    if (expr->is_arithmetic_convertible())
        return true;

    if (SemUtils::is_binary_arithmetic_opcode(opc))
        dr_->report_nonarith_arith_op_operand(
            op_side, SemUtils::arith_op_str(opc), expr->type, expr->loc);
    else if (opc == ir::Opcode::UMINUS)
        dr_->report_nonarith_uminus_operand(expr->type, expr->loc);
    else
        throw std::logic_error(ATTACH_CONTEXT(
            "Expected arithmetic ir::Opcode (bin arith or uminus)"));
    return false;
}

bool
BasicBuilder::Restricted::validate_relational_expr(
    const ir::Opcode opc,
    const Expr *const expr,
    const OperandSide op_side)
{
    DEBUG_SMART_ASSERT(!!expr,);
    // In Alpha everything is convertible to bool.
    // And operators == and != convert their operands to bool.
    if (SemUtils::is_relational_equality_iropcode(opc))
        return true;
    // If here relational operator is:  < <= > >=
    if (expr->is_arithmetic_convertible())
        return true;
    dr_->report_nonarith_rel_op_operand(op_side, SemUtils::relop_str(opc), expr->type, expr->loc);
    return false;
}

bool
BasicBuilder::Restricted::validate_possible_division(
    const ir::Opcode opc,
    const Expr *const rhs,
    const SourceLocation division_loc)
{
    if (opc != ir::Opcode::DIV && opc != ir::Opcode::MOD)
        return true;
    if (!rhs->is_const_0())
        return true;
    dr_->report_division_by_zero(division_loc);
    return false;
}

const Expr *
BasicBuilder::Restricted::try_optimize_arithmetic_expr(
    const ir::Opcode opc,
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation result_loc)
{
    using Op = ir::Opcode;
    switch (opc)
    {
    case Op::ADD: return expr_optimizer_->try_optimize<Op::ADD>(result_loc, lhs, rhs);
    case Op::SUB: return expr_optimizer_->try_optimize<Op::SUB>(result_loc, lhs, rhs);
    case Op::MUL: return expr_optimizer_->try_optimize<Op::MUL>(result_loc, lhs, rhs);
    case Op::DIV: return expr_optimizer_->try_optimize<Op::DIV>(result_loc, lhs, rhs);
    case Op::MOD: return expr_optimizer_->try_optimize<Op::MOD>(result_loc, lhs, rhs);
    default: [[unlikely]] UNREACHABLE(FMT::format("Unexpected opcode: {}", static_cast<int>(opc)));
    }
}

const Expr *
BasicBuilder::Restricted::try_optimize_relational_expr(
    const ir::Opcode opc,
    const Expr *const lhs,
    const Expr *const rhs,
    const SourceLocation result_loc)
{
    using Op = ir::Opcode;
    switch (opc)
    {
    case Op::IF_EQ: return expr_optimizer_->try_optimize<Op::IF_EQ>(result_loc, lhs, rhs);
    case Op::IF_NEQ: return expr_optimizer_->try_optimize<Op::IF_NEQ>(result_loc, lhs, rhs);
    case Op::IF_LT: return expr_optimizer_->try_optimize<Op::IF_LT>(result_loc, lhs, rhs);
    case Op::IF_LTE: return expr_optimizer_->try_optimize<Op::IF_LTE>(result_loc, lhs, rhs);
    case Op::IF_GT: return expr_optimizer_->try_optimize<Op::IF_GT>(result_loc, lhs, rhs);
    case Op::IF_GTE: return expr_optimizer_->try_optimize<Op::IF_GTE>(result_loc, lhs, rhs);
    default: [[unlikely]] UNREACHABLE(FMT::format("Unexpected opcode: {}", static_cast<int>(opc)));
    }
}

void
BasicBuilder::Restricted::warn_if_lossy_conversion_int_to_float(
    const AlphaInt value,
    const SourceLocation conversion_loc)
{
    if (!support::is_lossless_int_to_float<AlphaFloat>(value))
        dr_->report_implicit_int_to_float_loss(conversion_loc);
}

void
CallBuilder::Restricted::update_method_call_draft(
    const char *const id,
    const SourceLocation id_loc)
{
    method_call_draft_.id = id;
    method_call_draft_.id_loc = id_loc;
}

void
CallBuilder::Restricted::check_for_argument_mismatch(
    const Expr *const callable_lvalue,
    const ExprList *param_list,
    const SourceLocation call_loc
)
{
    if (callable_lvalue->type != Expr::Type::PROGRAM_FUNCTION)
        return;
    const auto func_symbol = static_cast<const ProgFuncExpr *>(callable_lvalue)->func_symbol;
    if (func_symbol->parameter_list.size() == param_list->size())
        return;
    dr_->report_call_argument_mismatch(
        func_symbol->name,
        func_symbol->parameter_list.size(),
        param_list->size(),
        call_loc,
        func_symbol->loc
    );
}

const Expr *
CallBuilder::Restricted::build_call_consuming(
    const Expr *const callable_lvalue,
    ExprList *arg_list,
    const SourceLocation call_loc,
    const Expr *const method)
{
    DEBUG_SMART_ASSERT(!!callable_lvalue, !!arg_list);

    check_for_argument_mismatch(callable_lvalue, arg_list, call_loc);
    const Expr *func_expr = ss_bridge_->materialize_if_table_item(callable_lvalue);

    for (auto it = arg_list->crbegin(); it != arg_list->crend(); ++it)
        quad_handler_->emit_next(ir::Opcode::PARAM, nullptr, *it, nullptr, (*it)->loc);
    if (method)
        quad_handler_->emit_next(ir::Opcode::PARAM, nullptr, method, nullptr, method->loc);

    quad_handler_->emit_next(ir::Opcode::CALL, nullptr, func_expr, nullptr, call_loc);
    reset_temps_if_temp_operand(func_expr);
    const Expr *getretval_expr = expr_maker_->make_variable_expr(call_loc, parse_ctx_->new_temp());
    quad_handler_->emit_next(ir::Opcode::GETRETVAL, getretval_expr, nullptr, nullptr, call_loc);

    CallBuilder::Restricted::delete_expr_list(arg_list);
    return getretval_expr;
}

const Expr *
CallBuilder::Restricted::build_method_call_consuming(
    const Expr *const callable_lvalue, ExprList *arg_list, const SourceLocation call_loc)
{
    const Expr *lvalue = ss_bridge_->materialize_if_table_item(callable_lvalue);
    const Expr *const lvalue_copy = lvalue;

    const Expr *const method_index = expr_maker_->make_const_string_expr(
        method_call_draft_.id_loc, method_call_draft_.id.c_str());
    const Expr *const hosting_var = expr_maker_->make_table_item_expr(
        call_loc, lvalue, method_index);

    lvalue = ss_bridge_->materialize_if_table_item(hosting_var);
    return build_call_consuming(lvalue, arg_list, call_loc, DEBUG_REQUIRE_PTR(lvalue_copy));
}

const Expr *
CallBuilder::Restricted::build_iife_call_consuming(
    const FuncSymbol *const func_symbol, ExprList *arg_list, const SourceLocation call_loc)
{
    DEBUG_SMART_ASSERT(!!func_symbol);
    const auto *const prog_func_expr = expr_maker_->make_prog_func_expr(call_loc, func_symbol);
    return build_call_consuming(prog_func_expr, arg_list, call_loc);
}

void CallBuilder::Restricted::delete_expr_list(ExprList *param_list) { delete param_list; }

const Expr *
ConstBuilder::Restricted::build_true_expr(const SourceLocation loc)
{
    return expr_maker_->make_const_bool_expr(loc, true);
}

const Expr *
ConstBuilder::Restricted::build_false_expr(const SourceLocation loc)
{
    return expr_maker_->make_const_bool_expr(loc, false);
}

const Expr *
ConstBuilder::Restricted::build_int_expr(const AlphaInt value, const SourceLocation loc)
{
    return expr_maker_->make_const_int_expr(loc, value);
}

const Expr *
ConstBuilder::Restricted::build_float_expr(const AlphaFloat value, const SourceLocation loc)
{
    return expr_maker_->make_const_float_expr(loc, value);
}

const Expr *
ConstBuilder::Restricted::build_string_expr(const char *const value, const SourceLocation loc)
{
    return expr_maker_->make_const_string_expr(loc, value);
}

const Expr *
ConstBuilder::Restricted::build_nil_expr(const SourceLocation loc)
{
    return expr_maker_->make_nil_expr(loc);
}

void
FunctionBuilder::Restricted::update_function_draft()
{
    update_function_draft(parse_ctx_->anonymous_generator.new_anonymous());
}

void
FunctionBuilder::Restricted::update_function_draft(const std::string &id)
{
    function_draft_.id = id;

    // We probably enter next space before function_entry early on here, for formal arguments.
    parse_ctx_->space_handler.enter_space();
}

void
FunctionBuilder::Restricted::collect_function_parameter(
    const std::string &id,
    const SourceLocation id_loc) { function_draft_.parameter_list.emplace_back(id, id_loc); }

void
FunctionBuilder::Restricted::register_function_parameters()
{
    constexpr auto space = VarSymbol::Space::FORMAL_ARGUMENT;
    DEBUG_SMART_ASSERT(parse_ctx_->space_handler.space() == VarSymbol::Space::FORMAL_ARGUMENT);

    for (const Parameter &p: function_draft_.parameter_list)
        if (validate_formal_param_name(p))
            symbol_table_->insert_variable(
                p.name,
                parse_ctx_->scope_handler.scope(),
                VarSymbol::Type::FORMAL_ARGUMENT,
                space,
                parse_ctx_->space_handler.next_offset(),
                p.loc
            );
}

bool
FunctionBuilder::Restricted::validate_funcdef_name(
    const std::string &func_name,
    const SourceLocation funcname_loc)
{
    if (symbol_table_->is_libfunc_name(func_name))
    {
        dr_->report_redefinition_of_libfunc(func_name, funcname_loc);
        return false;
    }

    const auto curr_scope = parse_ctx_->scope_handler.scope();
    if (const Symbol *const found_symbol = symbol_table_->lookup_local(func_name, curr_scope))
    {
        if (found_symbol->is_function())
        {
            dr_->report_redefinition_of_func(func_name, funcname_loc, found_symbol->loc);
            return false;
        }
        if (found_symbol->is_variable())
        {
            dr_->report_redefinition_of_var_as_func(func_name, funcname_loc, found_symbol->loc);
            return false;
        }
    }
    return true;
}

bool
FunctionBuilder::Restricted::validate_formal_param_name(const Parameter &param)
{
    // Library‐function conflict
    if (symbol_table_->is_libfunc_name(param.name))
    {
        dr_->report_libfunc_redefined_as_formal_parameter(param.name, param.loc);
        return false;
    }

    const auto curr_scope = parse_ctx_->scope_handler.scope();
    if (const Symbol *const formal_symbol = symbol_table_->lookup_local(param.name, curr_scope))
    {
        // Parameter should produce name conflicts only with themselves.
        DEBUG_SMART_ASSERT(
            !!dynamic_cast<const VarSymbol *>(formal_symbol), // non-nullptr == valid conversion
            formal_symbol->is_variable(),
            formal_symbol->type == VarSymbol::Type::FORMAL_ARGUMENT
        );
        dr_->report_redefinition_of_formal_parameter(param.name, param.loc, formal_symbol->loc);
        return false;
    }
    return true;
}

const Expr *
FunctionBuilder::Restricted::forward_program_function(
    const FuncSymbol *const func_symbol,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!func_symbol);
    return expr_maker_->make_prog_func_expr(result_loc, func_symbol);
}

/// Handles a function signature’s prefix + argument list.
///
/// If a name conflict is detected, we still need to call
/// enter_function() (to keep our frame‐stack balanced), but
/// we must *NOT* back-patch the local-variable count,
/// or we’ll end up polluting the original function’s frame with
/// local_variable_count from the redefinition. TODO: DO WE POLLUTE CURRENTLY?
const FuncSymbol *
FunctionBuilder::Restricted::build_program_function_entry(const SourceLocation func_signature_loc)
{
    const bool validated_funcname = validate_funcdef_name(function_draft_.id, func_signature_loc);
    const u32 skip_func_jump_label = quad_handler_->next_quad_label();
    quad_handler_->emit_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, func_signature_loc);
    const FuncSymbol *func_symbol = nullptr;
    if (validated_funcname)
    {
        func_symbol = symbol_table_->insert_function(
            function_draft_.id,
            parse_ctx_->scope_handler.scope(),
            next_function_address_++,
            function_draft_.parameter_list,
            func_signature_loc
        );

        quad_handler_->emit_next(
            ir::Opcode::FUNCSTART,
            nullptr,
            expr_maker_->make_prog_func_expr(func_signature_loc, func_symbol),
            nullptr,
            func_signature_loc
        );
    }
    DEBUG_SMART_ASSERT(support::logical_xnor(validated_funcname, !!func_symbol)); // Sanity check

    parse_ctx_->func_ctx_handler.enter_function(
        function_draft_.id, func_signature_loc, func_symbol, skip_func_jump_label);
    register_function_parameters();
    function_draft_.reset(); // Mandatory to support nested functions in the upcoming func-block.
    parse_ctx_->space_handler.enter_space(); // New var space -- must be after param registration.

    return func_symbol;
}

const FuncSymbol *
FunctionBuilder::Restricted::build_program_function_exit(const BlockSourceLocation block_loc)
{
    quad_handler_->patch_list(
        parse_ctx_->func_ctx_handler.return_list(), quad_handler_->next_quad_label());

    const auto fbi = parse_ctx_->func_ctx_handler.exit_function();
    if (!!fbi.func_symbol)
    {
        fbi.func_symbol->stackframe_slot_count = fbi.local_var_count;

        quad_handler_->emit_next(
            ir::Opcode::FUNCEND,
            nullptr,
            expr_maker_->make_prog_func_expr(block_loc.end, fbi.func_symbol),
            nullptr,
            block_loc.end);
    }
    quad_handler_->patch_quad(fbi.funcdef_skip_jump, quad_handler_->next_quad_label());
    parse_ctx_->space_handler.exit_space();

    return fbi.func_symbol;
}

const Expr *
TableAccessBuilder::Restricted::build_member_access(
    const Expr *const base,
    const char *const member_id,
    const SourceLocation member_id_loc,
    const SourceLocation access_loc)
{
    DEBUG_SMART_ASSERT(!!base, !!member_id);
    if (!validate_lvalue(dr_, base, access_loc, "member access", ".", "base expression"))
        return nullptr;
    const Expr *const materialized_lvalue = ss_bridge_->materialize_if_table_item(base);
    const Expr *const index = expr_maker_->make_const_string_expr(member_id_loc, member_id);
    return expr_maker_->make_table_item_expr(access_loc, materialized_lvalue, index);
}

const Expr *
TableAccessBuilder::Restricted::build_subscript_access(
    const Expr *const base,
    const Expr *const subscript,
    const SourceLocation access_loc)
{
    DEBUG_SMART_ASSERT(!!base, !!subscript);

    if (!validate_lvalue(dr_, base, access_loc, "subscript", "[]", "base expression"))
        return nullptr;
    const Expr *const materialized_lvalue = ss_bridge_->materialize_if_table_item(base);
    const Expr *const materialized_index = ss_bridge_->materialize_if_table_item(subscript);
    ss_bridge_->finalize_bool_expr(materialized_index);
    return expr_maker_->make_table_item_expr(access_loc, materialized_lvalue, materialized_index);
}
} // namespace alpha
