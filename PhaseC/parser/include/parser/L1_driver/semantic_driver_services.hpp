#ifndef SEMANTIC_DRIVER_SERVICES_HPP
#define SEMANTIC_DRIVER_SERVICES_HPP

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

namespace Alpha
{
class SemanticDriverServices
{
public:
    SemanticDriverServices(ParseCtx *parse_ctx, ExprMaker *expr_maker, QuadHandler *quad_handler);
    const Expr *emit_quad_if_table_item(const Expr *expr);

private:
    ParseCtx *const parse_ctx_;
    ExprMaker *const expr_maker_;
    QuadHandler *const quad_handler_;
};


#endif //SEMANTIC_DRIVER_SERVICES_HPP
