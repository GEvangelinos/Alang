#ifndef SEMANTIC_DRIVER_HPP
#define SEMANTIC_DRIVER_HPP

#include "parser/parser_context.hpp"
#include "parser/symbol_table.hpp"
#include "core/basics.hpp"
#include "diagnostics/diagnostic_engine.hpp"
#include "L2_builders/expr_builders_RENAME_CONTEXT_CHANGED.hpp"
#include "L2_builders/lvalue_resolver.hpp"
#include "L3_ir_infra/expr_folder.hpp"
#include "L3_ir_infra/expr_maker.hpp"
#include "L3_ir_infra/expr_snitch.hpp"
#include "L3_ir_infra/quad_handler.hpp"

namespace Alpha
{
// Order of initialization is intentional.
// As the subsystems of semantic driver utilize its internal state,
// for their own initialization.
class SemanticDriver : private Immobile
{
public:
    struct Options
    {
        const bool fold_arithmetic;
        const bool fold_relational;
        const bool fold_logical;
    };

    SemanticDriver(
        Options options,
        ParseCtx *parse_ctx,
        SymbolTable *symbol_table,
        DiagnosticEngine *diagnostic_engine);

    // TODO make a function that user calls before destructor call that basically extracts all this
    // alpha drivers would want (like the generated quads).
    [[nodiscard]] const std::vector<Quad> &retrieve_quads() const noexcept
    {
        return quad_handler_->quads();
    }

private:
    // Must be initialized first -- used by subsystems during their construction.
    // Defaulted to nullptr to trigger safe asserts if construction order is violated.
    ParseCtx *const parse_ctx_ = nullptr;
    SymbolTable *const symbol_table_ = nullptr;
    DiagnosticEngine *const diagnostic_engine_ = nullptr;


    // -- Layer 3 subsystems --
    std::unique_ptr<ExprSnitch> expr_snitch_;
    std::unique_ptr<ExprMaker> expr_maker_;
    std::unique_ptr<ExprFolder> expr_folder_;
    std::unique_ptr<QuadHandler> quad_handler_;

    SemanticDriverBridge sd_bridge_;

    DriverInitPack make_builder_init_pack();

    static BasicBuilder::Options get_basic_builder_options(const Options &options);

    friend class SemanticDriverBridge;

public:
    // --Layer 2 subsystems --
    ConstBuilder const_builder;
    BasicBuilder basic_builder;
    AssignBuilder assign_builder;
    LvalueResolver lvalue_resolver;

    void mark_short_circuit_jump_point();
    const Expr *convert_to_bool_expr(const Expr *expr);
};

inline void
SemanticDriver::mark_short_circuit_jump_point()
{
    parse_ctx_->cache.short_circuit_jump_stack.push(quad_handler_->next_quad_label());
}

inline const Expr *
SemanticDriver::convert_to_bool_expr(const Expr *const expr)
{
    DEBUG_SMART_ASSERT(!!expr);
    if (expr->type == Expr::Type::BOOL_EXPR)
        return expr;
    if (SemUtils::is_static_expr(expr))
        return SemUtils::as_bool(expr) ? expr_maker_->premade_true : expr_maker_->premade_false;

    BoolExpr *const bool_expr = expr_maker_->make_bool_expr(expr->loc);
    bool_expr->true_list.push_back(quad_handler_->next_quad_label());
    quad_handler_->emit_labelless_quad(
        IOPCode::IF_EQ, expr, expr_maker_->premade_true, nullptr, expr->loc);
    bool_expr->false_list.push_back(quad_handler_->next_quad_label());
    quad_handler_->emit_labelless_quad(IOPCode::JUMP, nullptr, nullptr, nullptr, expr->loc);
    return bool_expr;
}
} // namespace Alpha
#endif // SEMANTIC_DRIVER_HPP
