#include "L2_semantic_subsystems/expr_builders.hpp"

#include <ranges>
#include <diagnostics/diagnostic_reporter.gen.hpp>

#include "expr_type_traits.hpp"
#include "L2_semantic_subsystems/core/expr_normalizer.hpp"
#include "parser/internal_typedefs.hpp"

namespace
{
using namespace alpha;

[[nodiscard]] bool validate_lvalue(
    DiagnosticReporter* const dr,
    const Expr* const expr,
    const SourceLocation full_expr_loc,
    const char* op_name,
    const char* op_symbol,
    const char* lvalue_subject)
{
    DEBUG_SMART_ASSERT(!!dr, !!expr, !!op_name, !!op_symbol, !!lvalue_subject);
    DEBUG_SMART_ASSERT(!!*op_name, !!*op_symbol, !!*lvalue_subject);
    if (expr->is_lvalue_type() && expr->is_rvalue_casted())
    {
        const SourceLocation lhs_cast_loc{expr->loc.begin, SrcBuffIdx{expr->loc.begin.value + 1}};
        const SourceLocation rhs_cast_loc{SrcBuffIdx{expr->loc.end.value - 1}, expr->loc.end};
        dr->report_operator_on_lvalue_casted_to_rvalue(
            op_name, op_symbol, full_expr_loc, lhs_cast_loc, rhs_cast_loc);
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
    DiagnosticReporter* const dr,
    const Expr* const expr,
    const SourceLocation full_expr_loc,
    const char* op_name,
    const char* op_symbol,
    const char* lvalue_subject)

{
    DEBUG_SMART_ASSERT(!!expr);
    DEBUG_SMART_ASSERT(
        expr->type != Expr::Type::TABLE_ITEM &&
        "validate_direct_lvalue: TABLE_ITEM is not a direct lvalue (use table-item path)"
    );

    if (expr->has_active_temp())
    {
        dr->report_operator_requires_non_temp_lvalue(
            op_name, op_symbol, lvalue_subject, expr->type, full_expr_loc, expr->loc);
        return false;
    }
    if (!validate_lvalue(dr, expr, full_expr_loc, op_name, op_symbol, lvalue_subject))
        return false;
    return true;
}
} // namespace

namespace alpha
{
AssignBuilder::AssignBuilder(
    AssignBuilder::Options&& options,
    const SemanticSystemServices& ss_services)
    : DISPATCH_TARGET(std::move(options), ss_services) {}

AssignBuilder::Restricted::Restricted(
    AssignBuilder::Options&& options,
    const SemanticSystemServices& ss_services)
    : SemanticSubsystem(ss_services),
      options_(std::move(options)) {}

BasicBuilder::BasicBuilder(
    BasicBuilder::Options&& options,
    const SemanticSystemServices& ss_services)
    : DISPATCH_TARGET(std::move(options), ss_services) {}

BasicBuilder::Restricted::Restricted(
    BasicBuilder::Options&& options,
    const SemanticSystemServices& ss_services)
    : SemanticSubsystem(ss_services), options_(std::move(options)) {}

CallBuilder::CallBuilder(const SemanticSystemServices& ss_services)
    : DISPATCH_TARGET(ss_services) {}

CallBuilder::Restricted::Restricted(const SemanticSystemServices& ss_services)
    : SemanticSubsystem(ss_services) {}

CallBuilder::Restricted::CallInfo::CallInfo()
    : pending_method_info(std::nullopt),
      arguments() {}

CallBuilder::Restricted::CallInfo::CallInfo(MethodInfo method_info)
    : pending_method_info(method_info),
      arguments() {}

ConstBuilder::ConstBuilder(const SemanticSystemServices& ss_services)
    : DISPATCH_TARGET(ss_services) {}

ConstBuilder::Restricted::Restricted(const SemanticSystemServices& ss_services)
    : SemanticSubsystem(ss_services) {}

FunctionBuilder::FunctionBuilder(const SemanticSystemServices& ss_services)
    : DISPATCH_TARGET(ss_services) {}

FunctionBuilder::Restricted::Restricted(const SemanticSystemServices& ss_services)
    : SemanticSubsystem(ss_services) {}

TableAccessBuilder::TableAccessBuilder(const SemanticSystemServices& ss_services)
    : DISPATCH_TARGET(ss_services) {}

TableAccessBuilder::Restricted::Restricted(const SemanticSystemServices& ss_services)
    : SemanticSubsystem(ss_services) {}

AggregateBuilder::AggregateBuilder(const SemanticSystemServices& ss_services)
    :DISPATCH_TARGET(ss_services) {}

AggregateBuilder::Restricted::Restricted(const SemanticSystemServices& ss_services)
    : SemanticSubsystem(ss_services) {}

AggregateBuilder::Restricted::TableLiteralInfo::TableLiteralInfo(
    const NewTableExpr* const new_table_expr,
    const LabelID host_quad_label)
    : host_expr(DEBUG_REQUIRE_PTR(new_table_expr)),
      host_quad_label(host_quad_label) {}

// We must create the table literal at the start in order to emit TABLESETELEM instructions in
// place, which minimizes the lifetime of temporary variables. However, at creation time we do not
// yet know the full span of the source location that the table literal covers.
// As a result, location is patched once the table literal is closed.
void
AggregateBuilder::Restricted::init_table_literal()
{
    parse_ctx_->elist_ctx_handler.enter_region(ElistCtxHandler::Region::TABLE);
    const NewTableExpr* const new_table = expr_maker_->make_new_table_expr(SourceLocation::none());

    // Store quad label, so I can loc-patch quad's location
    draft_.table_literal_stack.emplace(new_table, quad_handler_->next_quad_label());

    quad_yielder_->yield_next(
        ir::Opcode::TABLECREATE,
        new_table,
        nullptr,
        nullptr,
        SourceLocation::none()
    );
}

const Expr*
AggregateBuilder::Restricted::finalize_table_literal(const SourceLocation table_loc)
{
    auto& tls = draft_.table_literal_stack; // Short alias for readability.
    DEBUG_SMART_ASSERT(!tls.empty() && "No active table literal to finalize");

    // TODO: provide a locpatch_expr method of expr_maker.. instead of cloining
    // Clone the table expression with its finalized source location for accurate diagnostics
    const Expr* const table_literal =
        expr_maker_->clone_with_updated_location(table_loc, tls.top().host_expr);

    // Backpatch the TABLECREATE quad with the correct source location
    quad_handler_->locPatch_tablecreate(tls.top().host_quad_label, table_loc);

    tls.pop();
    parse_ctx_->elist_ctx_handler.exit_region(DEBUG(ElistCtxHandler::Region::TABLE));
    return table_literal;
}

void AggregateBuilder::Restricted::commit_dict_element(
    const Expr* key,
    const Expr* value,
    const SourceLocation dict_elem_loc)
{
    DEBUG_SMART_ASSERT(!!key, !!value);

    key = expr_optimizer_->try_propagate_const(key);
    key = expr_normalizer_->materialize_if_table_item(key);
    expr_normalizer_->resolve_bool_short_circuit(key);
    value = expr_optimizer_->try_propagate_const(value);
    value = expr_normalizer_->materialize_if_table_item(value);
    expr_normalizer_->resolve_bool_short_circuit(value);

    DEBUG_SMART_ASSERT(
        !draft_.table_literal_stack.empty() &&
        "If we collect a dict_entry, then we must be inside a table_literal"
    );

    const auto& top_table = draft_.table_literal_stack.top();

    quad_yielder_->yield_next(
        ir::Opcode::TABLESETELEM,
        top_table.host_expr,
        key,
        value,
        dict_elem_loc
    );
}

// All state/services are intentionally nested inside `Restricted`.
// This prevents `friend classes from accessing them directly.
// As a side effect, even outer methods (like this one) must go through
// `restricted()` to reach private data, since that's the only legal path.
void
AggregateBuilder::commit_list_element(const Expr* list_elem)
{
    DEBUG_SMART_ASSERT(!!list_elem);
    auto& r = restricted(); // Local alias for clarity

    DEBUG_SMART_ASSERT(
        r.parse_ctx_->elist_ctx_handler.region().has_value() &&
        r.parse_ctx_->elist_ctx_handler.region().value() == ElistCtxHandler::Region::TABLE
    );

    list_elem = r.expr_optimizer_->try_propagate_const(list_elem);
    list_elem = r.expr_normalizer_->materialize_if_table_item(list_elem);
    r.expr_normalizer_->resolve_bool_short_circuit(list_elem);

    DEBUG_SMART_ASSERT(!r.draft_.table_literal_stack.empty());
    auto& top_table = r.draft_.table_literal_stack.top();

    const Expr* const index_expr = r.expr_maker_->make_const_int_expr(
        list_elem->loc,
        top_table.list_index++
    );
    r.quad_yielder_->yield_next(
        ir::Opcode::TABLESETELEM,
        top_table.host_expr,
        index_expr,
        list_elem,
        list_elem->loc
    );
}

// TODO: do we propagate assignment of assignment like x = y = z = 5? If NOT
// We might need to let Expr::Type::ASSIGN_EXPR
bool
AssignBuilder::Restricted::try_record_const_expr(const Expr* const lvalue, const Expr* const rvalue)
{
    DEBUG_SMART_ASSERT(
        !!lvalue, !!rvalue,
        options_.record_constant_variables &&
        "Recording values of constant variables is OFF, shouldn't be called"
    );
    if (lvalue->type != Expr::Type::VARIABLE)
        return false;

    const auto* const var_symbol = static_cast<const VariableExpr*>(lvalue)->var_symbol;
    if (!rvalue->is_const())
    {
        SymbolTable::detach_const_expr(var_symbol);
        return false;
    }
    SymbolTable::attach_const_expr(var_symbol, static_cast<const ConstExpr*>(rvalue));
    return true;
}

const Expr*
AssignBuilder::Restricted::handle_direct_assignment(
    const Expr* const lhs,
    const Expr* rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    DEBUG_SMART_ASSERT(lhs->type != Expr::Type::TABLE_ITEM && "goto handle_table_item_assignment");

    rhs = expr_optimizer_->try_propagate_const(rhs);
    rhs = expr_normalizer_->materialize_if_table_item(rhs);
    expr_normalizer_->resolve_bool_short_circuit(rhs);

    if (options_.record_constant_variables)  // Returning rhs, instead of lhs has 2 benefits:
        if (try_record_const_expr(lhs, rhs)) // 1) Removes the need for const extraction.
            return rhs;                      // 2) No temp required for: <<x = [{c=10 : c= 5}];>>

    quad_yielder_->yield_next(ir::Opcode::ASSIGN, lhs, rhs, nullptr, result_loc);

    if (!assignment_requires_temp())
        return lhs;

    const auto result_factory = [result_loc, this]()
    {
        return expr_maker_->make_assign_expr(result_loc, parse_ctx_->new_temp());
    };
    return quad_yielder_->yield_next(ir::Opcode::ASSIGN, result_factory, lhs, nullptr, result_loc);
}

const Expr*
AssignBuilder::Restricted::handle_table_item_assignment(
    const Expr* const lhs,
    const Expr* rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    DEBUG_SMART_ASSERT(lhs->type == Expr::Type::TABLE_ITEM);

    const auto* const ti = static_cast<const TableItemExpr*>(lhs);
    rhs = expr_optimizer_->try_propagate_const(rhs);
    rhs = expr_normalizer_->materialize_if_table_item(rhs);
    expr_normalizer_->resolve_bool_short_circuit(rhs);

    quad_yielder_->yield_next(ir::Opcode::TABLESETELEM, ti, ti->index, rhs, result_loc);

    // We resurface the assigned element of table to allow chained assignment. Ex.: a = b.c = d;
    // This is also useful for call(t[i]=1, t[i]=2, t[i]=3);
    const Expr* const materialized = expr_normalizer_->materialize_if_table_item(ti);
    DEBUG_SMART_ASSERT(materialized->has_var_symbol());

    // We semantically transform materialized to an ASSIGN expression
    return expr_maker_->make_assign_expr(
        materialized->loc,
        static_cast<const ExprWVarSymbol*>(materialized)->var_symbol
    );
}

const Expr*
AssignBuilder::Restricted::build_assignment(
    const Expr* const lhs,
    const Expr* const rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    if (!validate_assignment(lhs, result_loc))
        return nullptr;

    return AssignBuilder::Restricted::is_direct_target_expr(lhs)
           ? handle_direct_assignment(lhs, rhs, result_loc)
           : handle_table_item_assignment(lhs, rhs, result_loc);
}

const Expr*
AssignBuilder::Restricted::build_pre_inc(const Expr* const expr, const SourceLocation result_loc)
{
    return build_inc_dec<OpVariant::PRE, IncPolicy>(expr, result_loc);
}

const Expr*
AssignBuilder::Restricted::build_post_inc(const Expr* const expr, const SourceLocation result_loc)
{
    return build_inc_dec<OpVariant::POST, IncPolicy>(expr, result_loc);
}

const Expr*
AssignBuilder::Restricted::build_pre_dec(const Expr* const expr, const SourceLocation result_loc)
{
    return build_inc_dec<OpVariant::PRE, DecPolicy>(expr, result_loc);
}

const Expr*
AssignBuilder::Restricted::build_post_dec(const Expr* const expr, const SourceLocation result_loc)
{
    return build_inc_dec<OpVariant::POST, DecPolicy>(expr, result_loc);
}

bool
AssignBuilder::Restricted::is_direct_target_expr(const Expr* expr) noexcept
{
    return expr->type != Expr::Type::TABLE_ITEM;
}

bool
AssignBuilder::Restricted::assignment_requires_temp() const
{
    return parse_ctx_->call_ctx_handler.is_in_call() ||
           parse_ctx_->table_ctx_handler.is_in_dict_entry();
}

bool
AssignBuilder::Restricted::validate_assignment(
    const Expr* const lhs,
    const SourceLocation assign_loc)
{
    DEBUG_SMART_ASSERT(!!lhs);
    if (lhs->type == Expr::Type::LIBRARY_FUNCTION)
    {
        const auto* const func_symbol = static_cast<const LibFuncExpr*>(lhs)->libfunc_symbol;
        dr_->report_assign_to_libfunc(func_symbol->name, assign_loc);
        return false;
    }
    if (lhs->type == Expr::Type::PROGRAM_FUNCTION)
    {
        const auto* const func_symbol = static_cast<const ProgFuncExpr*>(lhs)->progfunc_symbol;
        dr_->report_assign_to_func(func_symbol->name, assign_loc, func_symbol->loc);
        return false;
    }

    const auto validator = AssignBuilder::Restricted::is_direct_target_expr(lhs)
                           ? &validate_direct_lvalue
                           : &validate_lvalue;
    return validator(dr_, lhs, assign_loc, "assignment", "=", "left operand");
}

// TODO: DRY!
template <typename Policy>
const Expr*
AssignBuilder::Restricted::handle_pre_inc_dec(
    const Expr* const lvalue,
    const SourceLocation result_loc)
{
    static_assert(std::is_same_v<Policy, IncPolicy> || std::is_same_v<Policy, DecPolicy>);
    DEBUG_SMART_ASSERT(!!lvalue);
    auto* qy = quad_yielder_;

    if (lvalue->type == Expr::Type::TABLE_ITEM)
    {
        const auto* const ti_host = static_cast<const TableItemExpr*>(lvalue);
        // External materialization is required, as ti is also used on quad's result field.
        const Expr* const ti = expr_normalizer_->materialize_if_table_item(ti_host);
        DEBUG_SMART_ASSERT(ti_host->index->type != Expr::Type::TABLE_ITEM && "Materialize index!");
        qy->yield_next(Policy::opc, ti, ti, &k_static_int_1_expr, result_loc);
        qy->yield_next(ir::Opcode::TABLESETELEM, ti_host, ti_host->index, ti, result_loc);

        if (!assignment_requires_temp())
        {
            DEBUG_SMART_ASSERT(ti->has_var_symbol());
            const auto ti_symbol = static_cast<const ExprWVarSymbol*>(ti)->var_symbol;
            return expr_maker_->make_arithmetic_expr(result_loc, ti_symbol);
        }

        // NOTE: This will emit a self-assignment (e.g. assign $N, $N) purely to preserve a live temp handle.
        // While we could implement a special-case "rebind" API to transfer ownership of the temp,
        // it's better to keep IR emission explicit and let a later copy-propagation pass remove the redundancy.
        const auto result_factory =
            [result_loc, this]() { return expr_maker_->make_arithmetic_expr(result_loc); };
        return qy->yield_next(ir::Opcode::ASSIGN, result_factory, ti, nullptr, result_loc);
    }
    else
    {
        qy->yield_next(Policy::opc, lvalue, lvalue, &k_static_int_1_expr, result_loc);
        if (!assignment_requires_temp())
        {
            DEBUG_SMART_ASSERT(lvalue->has_var_symbol());
            const auto lvalue_symbol = static_cast<const ExprWVarSymbol*>(lvalue)->var_symbol;
            return expr_maker_->make_arithmetic_expr(result_loc, lvalue_symbol);
        }

        // TODO: should re use result hook in assignment temps?
        const auto result_factory =
            [result_loc, this]() { return expr_maker_->make_arithmetic_expr(result_loc); };
        return qy->yield_next(ir::Opcode::ASSIGN, result_factory, lvalue, nullptr, result_loc);
    }
}

template <typename Policy>
const Expr*
AssignBuilder::Restricted::handle_post_inc_dec(
    const Expr* const lvalue,
    const SourceLocation result_loc)
{
    static_assert(std::is_same_v<Policy, IncPolicy> || std::is_same_v<Policy, DecPolicy>);
    auto* qy = quad_yielder_;

    const auto temp_factory = [result_loc, this]()
    {
        return expr_maker_->make_arithmetic_expr(result_loc, parse_ctx_->new_temp());
    };

    const Expr* result = nullptr;
    if (lvalue->type == Expr::Type::TABLE_ITEM)
    {
        const auto* const ti_host = static_cast<const TableItemExpr*>(lvalue);
        const Expr* const ti = expr_normalizer_->materialize_if_table_item(lvalue);
        DEBUG_SMART_ASSERT(ti->has_active_temp());
        // We externally materialize as we need `ti` in more than a single yield.
        // IMPORTANT: For table items we must allocate the result temp before `yield_next` call.
        // Deferring (by passing `temp_factory` as a callable, like in the else branch) would
        // release the table item’s active temp before ASSIGN and then re-acquire the same slot,
        // yielding a self-assignment (`assign $n $n`).
        result = temp_factory();
        qy->yield_next(ir::Opcode::ASSIGN, result, ti, nullptr, result_loc);
        qy->yield_next(Policy::opc, ti, ti, &k_static_int_1_expr, result_loc);
        qy->yield_next(ir::Opcode::TABLESETELEM, ti_host, ti_host->index, ti, result_loc);
    }
    else
    {
        DEBUG_SMART_ASSERT(!lvalue->has_active_temp() &&
            "if lvalue has temp we need to set result before yield_next call");
        result = qy->yield_next(ir::Opcode::ASSIGN, temp_factory, lvalue, nullptr, result_loc);
        qy->yield_next(Policy::opc, lvalue, lvalue, &k_static_int_1_expr, result_loc);
    }
    return DEBUG_REQUIRE_PTR(result);
}

template <AssignBuilder::Restricted::OpVariant op_variant, typename Policy>
const Expr*
AssignBuilder::Restricted::build_inc_dec(const Expr* const expr, const SourceLocation result_loc)
{
    static_assert(std::is_same_v<Policy, IncPolicy> || std::is_same_v<Policy, DecPolicy>);

    const auto validator = AssignBuilder::Restricted::is_direct_target_expr(expr)
                           ? &validate_direct_lvalue
                           : &validate_lvalue;

    if (!validator(dr_, expr, result_loc, Policy::op_name, Policy::op_symbol, "operand"))
        return nullptr;

    const Expr* result = nullptr;
    if constexpr (op_variant == OpVariant::PRE)
        result = handle_pre_inc_dec<Policy>(expr, result_loc);
    else if constexpr (op_variant == OpVariant::POST)
        result = handle_post_inc_dec<Policy>(expr, result_loc);
    else
        static_assert(always_false_v<void>, "build_inc_dec(): Unknown OpVariant");

    DEBUG_SMART_ASSERT(
        support::require_ptr(result)->type == Expr::Type::ARITHMETIC &&
        "increment/decrement is an arithmetic expression"
    );
    return result;
}

const Expr*
BasicBuilder::Restricted::prepare_logical_operand_expr(const Expr* expr)
{
    expr = expr_normalizer_->materialize_if_table_item(expr);
    expr = expr_optimizer_->try_propagate_const(expr);
    expr = normalize_to_bool_expr(expr);

    // TODO: test if releasing temp handle causes problems.
    // by so far because we use backpatching i cant find a scenario it causes problems...
    // Test further
    quad_yielder_->release_temp_handle_if_active(expr);
    return expr;
}

void
BasicBuilder::Restricted::mark_short_circuit_jump_point()
{
    short_circuit_jump_stack_.push(quad_handler_->next_quad_label());
}

const Expr*
BasicBuilder::Restricted::build_uminus(
    const Expr* expr,
    const SourceLocation uminus_loc,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!expr);
    if (!validate_arithmetic_expr(OperandSide::UNARY, ir::Opcode::UMINUS, expr, uminus_loc))
        goto skip_opt;
    if (const auto optimized = expr_optimizer_->try_optimize<ir::Opcode::UMINUS>(result_loc, expr))
        return optimized;

skip_opt:
    const auto result_factory =
        [result_loc, this]() { return expr_maker_->make_arithmetic_expr(result_loc); };
    expr = expr_normalizer_->materialize_if_table_item(expr);
    return quad_yielder_->yield_next(ir::Opcode::UMINUS, result_factory, expr, nullptr, result_loc);
}

const Expr*
BasicBuilder::Restricted::build_arithmetic(
    const ir::Opcode opc,
    const Expr* lhs,
    const Expr* rhs,
    const SourceLocation arith_op_loc,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    // Always build IR. On validation errors we report error diagnostics and never export bad IR.
    if (!validate_arithmetic_operation(opc, lhs, rhs, arith_op_loc))
        goto skip_opt;

    // We pre-propagate const (so we can catch div by zero)
    rhs = expr_optimizer_->try_propagate_const(rhs);

    // Warn on CT div-by-zero, skip folding; VM must still handle division-by-zero at runtime.
    if (!validate_possible_division(opc, rhs, result_loc))
        goto skip_opt;
    if (const auto optimized = this->try_optimize_arithmetic_expr(opc, lhs, rhs, result_loc))
        return optimized;

skip_opt:
    const auto result_factory =
        [result_loc, this]() { return expr_maker_->make_arithmetic_expr(result_loc); };
    lhs = expr_normalizer_->materialize_if_table_item(lhs);
    rhs = expr_normalizer_->materialize_if_table_item(rhs);
    return quad_yielder_->yield_next(opc, result_factory, lhs, rhs, result_loc);
}

const Expr*
BasicBuilder::Restricted::build_relational(
    const ir::Opcode opc,
    const Expr* lhs,
    const Expr* rhs,
    const SourceLocation operator_loc,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);

    lhs = expr_normalizer_->materialize_if_table_item(lhs);
    rhs = expr_normalizer_->materialize_if_table_item(rhs);

    // In case of == and != exprs need to be finalized.
    expr_normalizer_->resolve_bool_short_circuit(lhs);
    expr_normalizer_->resolve_bool_short_circuit(rhs);

    // Always build IR. On validation errors we report error diagnostics and never export bad IR.
    if (!validate_relational_operation(opc, lhs, rhs, operator_loc))
        goto skip_opt;

    if (const auto optimized = this->try_optimize_relational_expr(opc, lhs, rhs, result_loc))
        return optimized;

skip_opt:
    auto hook = [result_loc, this]()
    {
        const BoolExpr* result_expr = expr_maker_->make_bool_expr(result_loc);
        result_expr->true_list.push_back(quad_handler_->next_quad_label());
        result_expr->false_list.push_back(quad_handler_->next_quad_label() + 1); // +1 for jump quad
        return result_expr;
    };

    const auto hook_result = quad_yielder_->yield_returning_hook_result(
        opc, nullptr, lhs, rhs, result_loc, k_no_label, hook
    );
    quad_yielder_->yield_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, result_loc);
    return hook_result;
}

const Expr*
BasicBuilder::Restricted::build_logical_not(const Expr* expr, const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!expr);
    DEBUG_SMART_ASSERT(expr->is_bool_or_const_bool());

    if (const auto optimized = expr_optimizer_->try_optimize<ir::Opcode::NOT>(result_loc, expr))
        return optimized;
    // Sanity check, CONST_BOOL must be consumed by the optimizer.
    DEBUG_SMART_ASSERT(expr->type == Expr::Type::BOOL);

    static_cast<const BoolExpr*>(expr)->invert();
    return expr;
}

const Expr*
BasicBuilder::Restricted::build_logical_and(
    const Expr* lhs,
    const Expr* rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    DEBUG_SMART_ASSERT(lhs->is_bool_or_const_bool(), rhs->is_bool_or_const_bool());

    if (const auto optimized = expr_optimizer_->try_optimize<ir::Opcode::AND>(result_loc, lhs, rhs))
        return optimized;

    // TODO: do we need to materialize? or resolve bool?
    quad_yielder_->release_temp_handle_if_active(rhs);
    quad_yielder_->release_temp_handle_if_active(lhs);
    return build_short_circuit_bool_expr<AndShortCircuitPolicy>(lhs, rhs, result_loc);
}

const Expr*
BasicBuilder::Restricted::build_logical_or(
    const Expr* lhs,
    const Expr* rhs,
    const SourceLocation result_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs);
    DEBUG_SMART_ASSERT(lhs->is_bool_or_const_bool(), rhs->is_bool_or_const_bool());

    if (const auto optimized = expr_optimizer_->try_optimize<ir::Opcode::OR>(result_loc, lhs, rhs))
        return optimized;

    // TODO: do we need to materialize? or resolve bool?
    quad_yielder_->release_temp_handle_if_active(rhs);
    quad_yielder_->release_temp_handle_if_active(lhs);
    return build_short_circuit_bool_expr<OrShortCircuitPolicy>(lhs, rhs, result_loc);
}

template <typename Policy>
const Expr*
BasicBuilder::Restricted::build_short_circuit_bool_expr(
    const Expr* const lhs,
    const Expr* const rhs,
    const SourceLocation result_loc)
{
    static_assert(
        std::is_same_v<Policy, OrShortCircuitPolicy> ||
        std::is_same_v<Policy, AndShortCircuitPolicy>,
        "Unknown backpatching policy"
    );

    DEBUG_SMART_ASSERT(lhs->type == Expr::Type::BOOL && rhs->type == Expr::Type::BOOL);
    const BoolExpr* const lhs_bool = static_cast<const BoolExpr*>(lhs);
    const BoolExpr* const rhs_bool = static_cast<const BoolExpr*>(rhs);
    const BoolExpr* const bool_result_expr = expr_maker_->make_bool_expr(result_loc);

    // Patching left side.
    DEBUG_SMART_ASSERT(!short_circuit_jump_stack_.empty());
    for (const LabelID quad_label : Policy::backpatch_list(lhs_bool))
        quad_handler_->labelPatch_quad(quad_label, short_circuit_jump_stack_.top());
    short_circuit_jump_stack_.pop();
    Policy::backpatch_list(lhs_bool).clear();

    // Merging right side.
    auto& lhs_merge = Policy::merge_lhs_list(lhs_bool);            // lhs_bool->false_list
    auto& rhs_merge = Policy::merge_rhs_list(rhs_bool);            // rhs_bool->false_list
    auto& result_merge = Policy::merge_lhs_list(bool_result_expr); // lhs_bool->false_list

    // We could use merge_rhs too
    result_merge.reserve(lhs_merge.size() + rhs_merge.size());
    result_merge.insert(result_merge.end(), lhs_merge.begin(), lhs_merge.end());
    result_merge.insert(result_merge.end(), rhs_merge.begin(), rhs_merge.end());

    Policy::assign_list(bool_result_expr) = Policy::assign_list(rhs_bool);
    return bool_result_expr;
}

const Expr*
BasicBuilder::Restricted::normalize_to_bool_expr(const Expr* const expr)
{
    DEBUG_SMART_ASSERT(!!expr);

    if (expr->type == Expr::Type::BOOL)
        return expr;

    if (options_.fold_static_bools)
        if (expr->is_static())
            return SemUtils::as_bool(expr)
                   ? expr_maker_->make_const_bool_expr(expr->loc, true)
                   : expr_maker_->make_const_bool_expr(expr->loc, false);

    const auto hook = [expr, this]()
    {
        const BoolExpr* const bool_expr = expr_maker_->make_bool_expr(expr->loc);
        bool_expr->true_list.push_back(quad_handler_->next_quad_label());
        bool_expr->false_list.push_back(quad_handler_->next_quad_label() + 1);
        return bool_expr;
    };

    const Expr* const bool_expr = quad_yielder_->yield_returning_hook_result(
        ir::Opcode::IF_EQ, nullptr, expr, &k_static_true_expr, expr->loc, k_no_label, hook
    );
    quad_yielder_->yield_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, expr->loc);
    return bool_expr;
}

bool
BasicBuilder::Restricted::validate_arithmetic_expr(
    const OperandSide op_side,
    const ir::Opcode opc,
    const Expr* expr,
    const SourceLocation arith_op_loc)
{
    DEBUG_SMART_ASSERT(!!expr);
    if (expr->is_arithmetic_convertible())
        return true;

    if (SemUtils::is_binary_arithmetic_opcode(opc))
        dr_->report_nonarith_arith_op_operand(
            op_side, SemUtils::arith_op_str(opc), expr->type, arith_op_loc, expr->loc
        );
    else if (opc == ir::Opcode::UMINUS)
        dr_->report_nonarith_uminus_operand(expr->type, arith_op_loc, expr->loc);
    else
        throw std::logic_error(ATTACH_CONTEXT(
            "Expected arithmetic ir::Opcode (bin arith or uminus)"
        ));
    return false;
}

bool
BasicBuilder::Restricted::validate_arithmetic_operation(
    const ir::Opcode opc,
    const Expr* const lhs,
    const Expr* const rhs,
    const SourceLocation arith_op_loc)
{
    const bool valid_lhs = validate_arithmetic_expr(OperandSide::LEFT, opc, lhs, arith_op_loc);
    const bool valid_rhs = validate_arithmetic_expr(OperandSide::RIGHT, opc, rhs, arith_op_loc);
    return valid_lhs && valid_rhs;
}

bool
BasicBuilder::Restricted::validate_relational_operation(
    const ir::Opcode opc,
    const Expr* const lhs,
    const Expr* const rhs,
    const SourceLocation operator_loc)
{
    DEBUG_SMART_ASSERT(!!lhs, !!rhs, SemUtils::is_relational_iropcode(opc));

    const auto validate_numeric = [opc, lhs, rhs, operator_loc, this]() -> bool
    {
        DEBUG_SMART_ASSERT(SemUtils::is_relational_numeric_iropcode(opc));
        if (lhs->is_arithmetic_convertible() && rhs->is_arithmetic_convertible())
            return true;
        const auto opc_str = SemUtils::relop_str(opc);
        if (!lhs->is_arithmetic_convertible())
            dr_->report_nonarith_rel_op_operand(
                OperandSide::LEFT, opc_str, lhs->type, operator_loc, lhs->loc
            );
        if (!rhs->is_arithmetic_convertible())
            dr_->report_nonarith_rel_op_operand(
                OperandSide::RIGHT, opc_str, rhs->type, operator_loc, rhs->loc
            );
        return false;
    };

    const auto validate_equality = [opc, lhs, rhs, operator_loc, this]() -> bool
    {
        using ET = Expr::Type;
        DEBUG_SMART_ASSERT(SemUtils::is_relational_equality_iropcode(opc));

        // Pass rhs->type through specific masks/filters to detect invalid comparisons.
        const bool passes_mask = [lhs, rhs]()
        {
            namespace ETCM = expr_traits::cmp_bitmasks;
            const auto rhs_bitmask = ETCM::to_bitmask(rhs->type);
            switch (lhs->type)
            {
            case ET::ARITHMETIC:
            case ET::CONST_FLOAT:
            case ET::CONST_INT: return static_cast<bool>(ETCM::arithmetic & rhs_bitmask);
            case ET::CONST_STRING: return static_cast<bool>(ETCM::string & rhs_bitmask);
            case ET::LIBRARY_FUNCTION: return static_cast<bool>(ETCM::libfunc & rhs_bitmask);
            case ET::PROGRAM_FUNCTION: return static_cast<bool>(ETCM::progfunc & rhs_bitmask);
            case ET::CONST_NIL:
            case ET::NEW_TABLE: return static_cast<bool>(ETCM::aggregate & rhs_bitmask);
            default: return true;
            }
        }();
        if (passes_mask)
            return true;

        const auto opc_str = SemUtils::relop_str(opc);
        dr_->report_equality_rel_op_operand_mismatch(
            opc_str, lhs->type, rhs->type, operator_loc, lhs->loc, rhs->loc
        );
        return false;
    };

    if (SemUtils::is_relational_numeric_iropcode(opc))
        return validate_numeric();
    if (SemUtils::is_relational_equality_iropcode(opc))
        return validate_equality();
    throw std::logic_error(ATTACH_CONTEXT("Unknown relational OPCode"));
}

bool
BasicBuilder::Restricted::validate_possible_division(
    const ir::Opcode opc,
    const Expr* const rhs,
    const SourceLocation division_loc)
{
    if (opc != ir::Opcode::DIV && opc != ir::Opcode::MOD)
        return true;
    if (!rhs->is_const_0())
        return true;
    dr_->report_division_by_zero(division_loc);
    return false;
}

const Expr*
BasicBuilder::Restricted::try_optimize_arithmetic_expr(
    const ir::Opcode opc,
    const Expr*& lhs,
    const Expr*& rhs,
    const SourceLocation result_loc)
{
    #define HANDLE_ARITHMETIC(OPC) \
        case ir::Opcode::OPC:  return expr_optimizer_->try_optimize<ir::Opcode::OPC>(result_loc, lhs, rhs)
    switch (opc)
    {
    HANDLE_ARITHMETIC(ADD);
    HANDLE_ARITHMETIC(SUB);
    HANDLE_ARITHMETIC(MUL);
    HANDLE_ARITHMETIC(DIV);
    HANDLE_ARITHMETIC(MOD);
    default: [[unlikely]] UNREACHABLE(FMT::format("Unexpected opcode: {}", static_cast<int>(opc)));
    }
    #undef  HANDLE_ARITHMETIC
}

const Expr*
BasicBuilder::Restricted::try_optimize_relational_expr(
    const ir::Opcode opc,
    const Expr*& lhs,
    const Expr*& rhs,
    const SourceLocation result_loc)
{
    #define HANDLE_RELATIONAL(OPC) \
        case ir::Opcode::OPC:  return expr_optimizer_->try_optimize<ir::Opcode::OPC>(result_loc, lhs, rhs)
    switch (opc)
    {
    HANDLE_RELATIONAL(IF_EQ);
    HANDLE_RELATIONAL(IF_NEQ);
    HANDLE_RELATIONAL(IF_LT);
    HANDLE_RELATIONAL(IF_LTE);
    HANDLE_RELATIONAL(IF_GT);
    HANDLE_RELATIONAL(IF_GTE);
    default: [[unlikely]] UNREACHABLE(FMT::format("Unexpected opcode: {}", static_cast<int>(opc)));
    }
    #undef  HANDLE_RELATIONAL
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
    const StringSpan method_id,
    const SourceLocation method_id_loc)
{
    DEBUG(
        if (!parse_ctx_->hard_error_occurred()) SMART_ASSERT(
            !draft_.immediate_method_info.has_value() &&
            "When call is initiated, the method-info draft should be cleared()."
        );
    )
    draft_.immediate_method_info.emplace(MethodInfo{.id = method_id, .id_loc = method_id_loc});
}

void
CallBuilder::Restricted::init_call()
{
    parse_ctx_->elist_ctx_handler.enter_region(ElistCtxHandler::Region::CALL);
    parse_ctx_->call_ctx_handler.enter_call();

    if (draft_.immediate_method_info.has_value())
    {
        draft_.call_info_stack.emplace(*draft_.immediate_method_info);
        draft_.immediate_method_info.reset();
    }
    else
        draft_.call_info_stack.emplace();
}

void
CallBuilder::Restricted::finalize_call()
{
    parse_ctx_->call_ctx_handler.exit_call();
    draft_.call_info_stack.pop();
    parse_ctx_->elist_ctx_handler.exit_region(DEBUG(ElistCtxHandler::Region::CALL));
}

void
CallBuilder::Restricted::check_for_argument_mismatch(
    const Expr* const callable_lvalue,
    const CallInfo::ArgStack& arg_stack,
    const SourceLocation call_loc
)
{
    // If not a program function, we can't know expected arguments. As we can make a call with any lvalue.
    if (callable_lvalue->type != Expr::Type::PROGRAM_FUNCTION)
        return;

    DEBUG_SMART_ASSERT(callable_lvalue->type == Expr::Type::PROGRAM_FUNCTION);
    const auto func_symbol = static_cast<const ProgFuncExpr*>(callable_lvalue)->progfunc_symbol;
    if (func_symbol->parameter_list.size() == arg_stack.size())
        return;
    dr_->report_call_argument_mismatch(
        func_symbol->name,
        func_symbol->parameter_list.size(),
        arg_stack.size(),
        call_loc,
        func_symbol->loc
    );
}

const Expr*
CallBuilder::Restricted::build_call_consuming(
    const Expr* const callable,
    const SourceLocation call_loc,
    const ConstStringExpr* const method_name)
{
    DEBUG_SMART_ASSERT(!!callable);
    DEBUG_SMART_ASSERT(callable->is_callable());
    DEBUG_SMART_ASSERT(!draft_.call_info_stack.empty());
    auto* qy = quad_yielder_; // Short alias for readability.

    auto& call_args = draft_.call_info_stack.top().arguments;
    check_for_argument_mismatch(callable, call_args, call_loc);
    while (!call_args.empty())
    {
        const Expr* const arg = call_args.top();
        qy->yield_next(ir::Opcode::PARAM, nullptr, arg, nullptr, arg->loc);
        call_args.pop();
    }

    const Expr* call_target = callable;
    if (method_name)
    {
        // Materialize host (handles cases like a[b]..c; host being a[b])
        const Expr* const method_keeper = expr_normalizer_->materialize_if_table_item(callable);
        qy->yield_next(ir::Opcode::PARAM, nullptr, method_keeper, nullptr, method_keeper->loc);
        // Extract method into a call_target.
        const SourceLocation method_get_loc = merge(method_keeper->loc, method_name->loc);
        call_target = expr_maker_->make_table_item_expr(method_get_loc, method_keeper, method_name);
    }
    const Expr* const callee = expr_normalizer_->materialize_if_table_item(call_target);
    DEBUG_SMART_ASSERT(callee->is_callable());
    qy->yield_next(ir::Opcode::CALL, nullptr, callee, nullptr, call_loc);

    // At these point we have used everything required to make a call.
    finalize_call();
    const auto* getretval_expr = expr_maker_->make_variable_expr(call_loc, parse_ctx_->new_temp());
    qy->yield_next(ir::Opcode::GETRETVAL, getretval_expr, nullptr, nullptr, call_loc);
    return getretval_expr;
}

const Expr*
CallBuilder::Restricted::build_method_call_consuming(
    const Expr* const method_host,
    const SourceLocation call_loc)
{
    DEBUG_SMART_ASSERT(!draft_.call_info_stack.empty());
    DEBUG_SMART_ASSERT(draft_.call_info_stack.top().pending_method_info.has_value());

    if (!validate_lvalue(dr_, method_host, call_loc, "method access", "..", "base expression"))
        return nullptr;

    const MethodInfo& mi = *draft_.call_info_stack.top().pending_method_info;
    // Turn method name into a string literal expression
    const auto* const method_name = expr_maker_->make_const_string_expr(mi.id_loc, mi.id);
    // Don't materialize callable_method, let build_call_consuming() do it.
    return build_call_consuming(method_host, call_loc, method_name);
}

const Expr*
CallBuilder::Restricted::build_iife_call_consuming(
    const ProgFuncSymbol* const func_symbol,
    const SourceLocation call_loc)
{
    DEBUG_SMART_ASSERT(!!func_symbol);
    const auto* const prog_func_expr = expr_maker_->make_prog_func_expr(call_loc, func_symbol);
    return build_call_consuming(prog_func_expr, call_loc);
}

void CallBuilder::commit_call_argument(const Expr* call_arg)
{
    DEBUG_SMART_ASSERT(!!call_arg);
    auto& r = restricted();
    DEBUG_SMART_ASSERT(
        r.parse_ctx_->elist_ctx_handler.region().has_value() &&
        r.parse_ctx_->elist_ctx_handler.region().value() == ElistCtxHandler::Region::CALL
    );

    // TODO: if we move the following 3 commands in build_call_consuming... does it change anything?
    // TODO: would we use less temps if we did so ?
    call_arg = r.expr_optimizer_->try_propagate_const(call_arg);
    call_arg = r.expr_normalizer_->materialize_if_table_item(call_arg);
    r.expr_normalizer_->resolve_bool_short_circuit(call_arg);

    DEBUG_SMART_ASSERT(!r.draft_.call_info_stack.empty());
    r.draft_.call_info_stack.top().arguments.push(call_arg);
}

const Expr*
ConstBuilder::Restricted::build_true_expr(const SourceLocation loc)
{
    return expr_maker_->make_const_bool_expr(loc, true);
}

const Expr*
ConstBuilder::Restricted::build_false_expr(const SourceLocation loc)
{
    return expr_maker_->make_const_bool_expr(loc, false);
}

const Expr*
ConstBuilder::Restricted::build_int_expr(const AlphaInt value, const SourceLocation loc)
{
    return expr_maker_->make_const_int_expr(loc, value);
}

const Expr*
ConstBuilder::Restricted::build_float_expr(const AlphaFloat value, const SourceLocation loc)
{
    return expr_maker_->make_const_float_expr(loc, value);
}

const Expr*
ConstBuilder::Restricted::build_string_expr(const StringSpan value, const SourceLocation loc)
{
    DEBUG_SMART_ASSERT(!value.empty());
    return expr_maker_->make_const_string_expr(loc, value);
}

const Expr*
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
FunctionBuilder::Restricted::update_function_draft(const StringSpan id)
{
    function_draft_.id = id;
    // We probably enter next space before function_entry early on here, for formal arguments.
    parse_ctx_->space_handler.enter_space();
}

void
FunctionBuilder::Restricted::collect_function_parameter(
    const StringSpan id,
    const SourceLocation id_loc)
{
    function_draft_.parameter_list.emplace_back(id, id_loc);
}

void
FunctionBuilder::Restricted::register_function_parameters()
{
    constexpr auto space = VarSymbol::Space::FORMAL_ARGUMENT;
    DEBUG_SMART_ASSERT(parse_ctx_->space_handler.space() == VarSymbol::Space::FORMAL_ARGUMENT);

    for (const Parameter& p : function_draft_.parameter_list)
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
    const StringSpan func_name,
    const SourceLocation funcname_loc)
{
    if (symbol_table_->is_libfunc_name(func_name))
    {
        dr_->report_redefinition_of_libfunc(func_name.to_string(), funcname_loc);
        return false;
    }

    const auto curr_scope = parse_ctx_->scope_handler.scope();
    if (const Symbol* const found_symbol = symbol_table_->lookup_local(func_name, curr_scope))
    {
        if (found_symbol->is_function())
        {
            dr_->report_redefinition_of_func(func_name.to_string(), funcname_loc, found_symbol->loc);
            return false;
        }
        if (found_symbol->is_variable())
        {
            dr_->report_redefinition_of_var_as_func(func_name.to_string(), funcname_loc, found_symbol->loc);
            return false;
        }
    }
    return true;
}

bool
FunctionBuilder::Restricted::validate_formal_param_name(const Parameter& param)
{
    // Library‐function conflict
    if (symbol_table_->is_libfunc_name(param.name))
    {
        dr_->report_libfunc_redefined_as_formal_parameter(param.name, param.loc);
        return false;
    }

    const auto curr_scope = parse_ctx_->scope_handler.scope();
    if (const Symbol* const formal_symbol = symbol_table_->lookup_local(param.name, curr_scope))
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

const Expr*
FunctionBuilder::Restricted::forward_program_function(
    const ProgFuncSymbol* const func_symbol,
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
const ProgFuncSymbol*
FunctionBuilder::Restricted::build_program_function_entry(const SourceLocation func_signature_loc)
{
    const bool validated_funcname = validate_funcdef_name(function_draft_.id, func_signature_loc);
    const u32 skip_func_jump_label = quad_handler_->next_quad_label();
    quad_yielder_->yield_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, func_signature_loc);
    const ProgFuncSymbol* func_symbol = nullptr;
    if (validated_funcname)
    {
        func_symbol = symbol_table_->insert_program_function(
            function_draft_.id,
            parse_ctx_->scope_handler.scope(),
            next_function_address_++,
            function_draft_.parameter_list,
            func_signature_loc
        );

        quad_yielder_->yield_next(
            ir::Opcode::FUNCSTART,
            nullptr,
            expr_maker_->make_prog_func_expr(func_signature_loc, func_symbol),
            nullptr,
            func_signature_loc
        );
    }
    // Sanity check
    DEBUG_SMART_ASSERT(!(validated_funcname ^ !!func_symbol));

    parse_ctx_->func_ctx_handler.enter_function(
        function_draft_.id, func_signature_loc, func_symbol, skip_func_jump_label);
    register_function_parameters();
    function_draft_.reset(); // Mandatory to support nested functions in the upcoming func-block.
    parse_ctx_->space_handler.enter_space(); // New var space -- must be after param registration.

    return func_symbol;
}

const ProgFuncSymbol*
FunctionBuilder::Restricted::build_program_function_exit(
    const BlockSourceLocation block_loc)
{
    quad_handler_->labelPatch_list(
        parse_ctx_->func_ctx_handler.return_list(),
        quad_handler_->next_quad_label()
    );

    const auto fbi = parse_ctx_->func_ctx_handler.exit_function();
    if (!!fbi.func_symbol)
    {
        fbi.func_symbol->stackframe_slot_count = fbi.local_var_count;

        quad_yielder_->yield_next(
            ir::Opcode::FUNCEND,
            nullptr,
            expr_maker_->make_prog_func_expr(block_loc.end_raw_loc, fbi.func_symbol),
            nullptr,
            block_loc.end_raw_loc);
    }
    quad_handler_->labelPatch_quad(fbi.funcdef_skip_jump, quad_handler_->next_quad_label());
    parse_ctx_->space_handler.exit_space();

    return fbi.func_symbol;
}

const Expr*
TableAccessBuilder::Restricted::build_member_access(
    const Expr* const base,
    const StringSpan member_id,
    const SourceLocation member_id_loc,
    const SourceLocation access_loc)
{
    DEBUG_SMART_ASSERT(!!base, !member_id.empty());
    if (!validate_lvalue(dr_, base, access_loc, "member access", ".", "base expression"))
        return nullptr;
    const Expr* const materialized_lvalue = expr_normalizer_->materialize_if_table_item(base);
    const Expr* const index = expr_maker_->make_const_string_expr(member_id_loc, member_id);
    return expr_maker_->make_table_item_expr(access_loc, materialized_lvalue, index);
}

const Expr*
TableAccessBuilder::Restricted::build_subscript_access(
    const Expr* base,
    const Expr* subscript,
    const SourceLocation access_loc)
{
    DEBUG_SMART_ASSERT(!!base, !!subscript);

    if (!validate_lvalue(dr_, base, access_loc, "subscript", "[]", "base expression"))
        return nullptr;

    base = expr_normalizer_->materialize_if_table_item(base);

    subscript = expr_optimizer_->try_propagate_const(subscript);
    subscript = expr_normalizer_->materialize_if_table_item(subscript);
    expr_normalizer_->resolve_bool_short_circuit(subscript);

    return expr_maker_->make_table_item_expr(access_loc, base, subscript);
}
} // namespace alpha
