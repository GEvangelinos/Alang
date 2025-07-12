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
#include "parser/ir.hpp"
#include "L3_ir_infra/expr_folder.hpp"
#include "L3_ir_infra/expr_maker.hpp"
#include "L3_ir_infra/quad_handler.hpp"

namespace Alpha
{
// TODO: maybe replace with the INIT pack.. or remove this class from initpack it contains duplciate fields
class SemanticSystemBridge
{
public:
    SemanticSystemBridge(ParseCtx *parse_ctx, ExprMaker *expr_maker, QuadHandler *quad_handler);

    const Expr *emit_quad_if_table_item(const Expr *expr);

private:
    ParseCtx *const parse_ctx_;
    ExprMaker *const expr_maker_;
    QuadHandler *const quad_handler_;
};

class Backpatcher; // Forward-Decl to avoid cyclic Includes

struct SemanticSystemServices
{
    ParseCtx *const parse_ctx;
    SymbolTable *const symbol_table;
    DiagnosticReporter *const dr;
    ExprMaker *const expr_maker;
    ExprFolder *const expr_folder;
    ExprSnitch *const expr_snitch;
    QuadHandler *const quad_handler;
    Backpatcher *const backpatcher;
    SemanticSystemBridge *const sd_bridge;
};
} // namespace Alpha


#endif //SEMANTIC_DRIVER_SERVICES_HPP
