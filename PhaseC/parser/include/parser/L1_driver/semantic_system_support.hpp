/// ─────────────────────────────────────────────────────────────────────────────
/// SemanticDriverServices:
/// A deliberately minimal interface exposing selected semantic functionality from
/// SemanticDriver to layer-2 builder classes.
///
/// RATIONALE:
/// SemanticDriver owns all core semantic subsystems (ExprMaker, QuadHandler, etc.),
/// but giving builders direct access would break encapsulation:
///   - Builders could access unrelated subsystems or coordination logic.
///   - Would require friend declarations or overly broad interfaces.
/// This class avoids that by acting as a semantic "arm":
///   - Grants builders access to *only* permitted utilities.
///   - Encapsulates only what’s necessary for local semantic actions.
///   - Maintains separation between builders and the driver internals.
///
/// IMPORTANT:
///   - Preserves clean layering (L1 driver → L2 builders).
///   - Builders stay isolated and unaware of each other.
///   - Driver remains the sole owner of coordination logic.
///   - No runtime cost: all access is static and direct.
/// In short: This class exists to enforce boundaries.
/// Only expand it with intent — never collapse it back into the driver.
/// ─────────────────────────────────────────────────────────────────────────────

#ifndef SEMANTIC_DRIVER_SERVICES_HPP
#define SEMANTIC_DRIVER_SERVICES_HPP

#include "parser/parser_context.hpp"
#include "L3_ir_infra/expr_optimizer.hpp"
#include "L3_ir_infra/expr_maker.hpp"
#include "L3_ir_infra/quad_handler.hpp"

namespace alpha
{
class SemanticSystemBridge
{
public:
    SemanticSystemBridge(ParseCtx *parse_ctx, ExprMaker *expr_maker, QuadHandler *quad_handler);

    [[nodiscard]] const Expr *materialize_lvalue_base(const Expr *lvalue);

    [[deprecated("Use materialize_lvalue_base()")]]
    const Expr* emit_if_table_item(const Expr *) = delete;

private:
    ParseCtx *const parse_ctx_ = nullptr;
    ExprMaker *const expr_maker_ = nullptr;
    QuadHandler *const quad_handler_ = nullptr;
};

struct SemanticSystemServices
{
    ParseCtx *const parse_ctx = nullptr;
    SymbolTable *const symbol_table = nullptr;
    DiagnosticReporter *const dr = nullptr;
    ExprMaker *const expr_maker = nullptr;
    ExprOptimizer *const expr_optimizer = nullptr;
    QuadHandler *const quad_handler = nullptr;
    SemanticSystemBridge *const ss_bridge = nullptr;
};
} // namespace alpha

#endif //SEMANTIC_DRIVER_SERVICES_HPP
