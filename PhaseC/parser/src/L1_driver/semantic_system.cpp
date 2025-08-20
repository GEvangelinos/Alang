#include "L1_driver/semantic_system.hpp"

namespace alpha
{
SemanticSystem::SemanticSystem(
    const Options &options,
    ParseCtx *const parse_ctx,
    SymbolTable *const symbol_table,
    DiagnosticReporter *const dr)
    : status_gateway(this),

      // External resources, required to initialize class.
      parse_ctx_(utils::require_ptr(parse_ctx)),
      symbol_table_(utils::require_ptr(symbol_table)),
      dr_(utils::require_ptr(dr)),

      // Private resources, used by public submodules.
      expr_maker_(std::make_unique<ExprMaker>(parse_ctx_)),
      quad_handler_(std::make_unique<QuadHandler>()),
      expr_optimizer_(std::make_unique<ExprOptimizer>(
          get_expr_optimizer_options(options),
          expr_maker_.get()
      )),
      sd_bridge_(parse_ctx_, expr_maker_.get(), quad_handler_.get()),

      // public servicers, used by users of semantic driver.
      aggregate_builder(create_semantic_system_services()),
      assign_builder(get_assign_builder_options(options), create_semantic_system_services()),
      basic_builder(create_semantic_system_services()),
      block_manager(create_semantic_system_services()),
      call_builder(create_semantic_system_services()),
      const_builder(create_semantic_system_services()),
      control_flow_manager(create_semantic_system_services()),
      lvalue_resolver(create_semantic_system_services()),
      function_builder(create_semantic_system_services()),
      table_access_builder(create_semantic_system_services()) {}

SemanticSystemServices
SemanticSystem::create_semantic_system_services()
{
    return {
        .symbol_table = utils::require_ptr(symbol_table_),
        .parse_ctx = utils::require_ptr(parse_ctx_),
        .dr = utils::require_ptr(dr_),
        .expr_maker = utils::require_ptr(expr_maker_.get()),
        .expr_optimizer = utils::require_ptr(expr_optimizer_.get()),
        .quad_handler = utils::require_ptr(quad_handler_.get()),
        .ss_bridge = &sd_bridge_,
    };
}

AssignBuilder::Options
SemanticSystem::get_assign_builder_options(const Options &options)
{
    return {
        // constant propagation requires recording of constants inside Expr(essions)
        .record_constant_variables = options.propagate_constants
    };
}

ExprOptimizer::Options
SemanticSystem::get_expr_optimizer_options(const Options &options)
{
    return {
        .constant_propagation = options.propagate_constants,
        .expr_folding = options.expr_folding,
        .expr_trimming = options.expr_trimming,
    };
}

const Expr *
SemanticSystem::normalize_to_bool_expr(const Expr *const expr)
{
    DEBUG_SMART_ASSERT(!!expr);

    if (expr->type == Expr::Type::BOOL_EXPR)
        return expr;
    if (SemUtils::is_static_expr(expr))
        return SemUtils::as_bool(expr)
               ? expr_maker_->make_const_bool_expr(expr->loc, true)
               : expr_maker_->make_const_bool_expr(expr->loc, false);

    const BoolExpr *const bool_expr = expr_maker_->make_bool_expr(expr->loc);
    bool_expr->true_list.push_back(quad_handler_->next_quad_label());
    quad_handler_->emit_labelless(ir::Opcode::IF_EQ, nullptr, expr, &k_static_true_expr, expr->loc);
    bool_expr->false_list.push_back(quad_handler_->next_quad_label());
    quad_handler_->emit_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, expr->loc);

    return bool_expr;
}

void
SemanticSystem::reset_stmt_context() noexcept { parse_ctx_->name_generator.reset_temp_names(); }

void
SemanticSystem::finalize_bool_expr(const Expr *const expr)
{
    DEBUG_SMART_ASSERT(!!expr);
    if (expr->type != Expr::Type::BOOL_EXPR)
        return; // Nothing to backpatch if not bool_expr.

    const BoolExpr *const bool_expr = static_cast<const BoolExpr *>(expr);
    auto *const qh = quad_handler_.get(); // Short alias for readability.

    DEBUG_SMART_ASSERT(!!bool_expr->var_symbol);

    // true branch: patch to here and assign true
    qh->patch_list(bool_expr->true_list, qh->next_quad_label());
    qh->emit_next(ir::Opcode::ASSIGN, expr, &k_static_true_expr, nullptr, expr->loc);

    // Offset to land after the false branch
    constexpr LabelID past_false_branch_offset = 2; // Depends on how many emits occur after jump.
    qh->emit_next(ir::Opcode::JUMP, nullptr, nullptr, nullptr, expr->loc, past_false_branch_offset);

    // false branch: patch to here and assign false
    qh->patch_list(bool_expr->false_list, quad_handler_->next_quad_label());
    qh->emit_next(ir::Opcode::ASSIGN, expr, &k_static_false_expr, nullptr, expr->loc);
}

SourceLocation
SemanticSystem::get_loc_of_last_expr() const
{
    DEBUG_SMART_ASSERT(expr_maker_.get() != nullptr);
    if (expr_maker_->expr_sink_.empty())
        return k_no_loc;
    return expr_maker_->expr_sink_.back()->loc;
}

void
SemanticSystem::DriverLink::notify_hard_error() noexcept
{
    host_->ss_status_ = SemanticSystem::Status::ERROR;
}
} // namespace alpha
