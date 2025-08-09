#include "L1_driver/semantic_system.hpp"

namespace alpha
{
SemanticSystem::SemanticSystem(
    const Options &options,
    ParseCtx *const parse_ctx,
    SymbolTable *const symbol_table,
    DiagnosticEngine *const diagnostic_engine)
    : ss_gateway_(ss_status_),
      // External resources, required to initialize class.
      parse_ctx_(Utils::require_ptr(parse_ctx)),
      symbol_table_(Utils::require_ptr(symbol_table)),
      diagnostic_engine_(Utils::require_ptr(diagnostic_engine)),

      // private resources, used by public submodules.
      expr_maker_(std::make_unique<ExprMaker>(parse_ctx_)),
      quad_handler_(std::make_unique<QuadHandler>()),
      expr_optimizer_(std::make_unique<ExprOptimizer>(
          get_expr_optimizer_options(options),
          expr_maker_.get()
      )),
      sd_bridge_(parse_ctx_, expr_maker_.get(), quad_handler_.get()),

      // public servicers, used by users of semantic driver.
      aggregate_builder(export_semantic_system_services()),
      assign_builder(get_assign_builder_options(options), export_semantic_system_services()),
      basic_builder(export_semantic_system_services()),
      block_manager(export_semantic_system_services()),
      const_builder(export_semantic_system_services()),
      loop_manager(export_semantic_system_services()),
      lvalue_resolver(export_semantic_system_services()) {}

SemanticSystemServices
SemanticSystem::export_semantic_system_services()
{
    return SemanticSystemServices{
        .parse_ctx = REQUIRE_PTR(parse_ctx_),
        .symbol_table = REQUIRE_PTR(symbol_table_),
        .dr = &REQUIRE_PTR(diagnostic_engine_)->dr,
        .expr_maker = REQUIRE_PTR(expr_maker_.get()),
        .expr_optimizer = REQUIRE_PTR(expr_optimizer_.get()),
        .quad_handler = REQUIRE_PTR(quad_handler_.get()),
        .ss_bridge = &sd_bridge_,
    };
}

AssignBuilder::Options
SemanticSystem::get_assign_builder_options(const Options &options)
{
    return {
        // constant propagation requires constant recording
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
SemanticSystem::convert_to_bool_expr(const Expr *const expr)
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
SemanticSystem::finalize_bool_expr(const Expr *const expr)
{
    DEBUG_SMART_ASSERT(!!expr);
    if (expr->type != Expr::Type::BOOL_EXPR)
        return; // Nothing to backpatch if not bool_expr.

    const BoolExpr *const bool_expr = static_cast<const BoolExpr *>(expr);
    auto *const qh = quad_handler_.get(); // Short alias to improve readability and reduce verbosity

    DEBUG_SMART_ASSERT(!!bool_expr->var_symbol);

    qh->patch_list(bool_expr->true_list, qh->next_quad_label());
    qh->emit_next(ir::Opcode::ASSIGN, expr, &k_static_true_expr, nullptr, expr->loc);
    qh->emit_next(ir::Opcode::JUMP, nullptr, nullptr, nullptr, expr->loc, 2);
    qh->patch_list(bool_expr->false_list, quad_handler_->next_quad_label());
    qh->emit_next(ir::Opcode::ASSIGN, expr, &k_static_false_expr, nullptr, expr->loc);
}
} // namespace alpha
