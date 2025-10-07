#ifndef EXPR_NORMALIZER_HPP
#define EXPR_NORMALIZER_HPP

namespace alpha
{
class QuadYielder;
struct Expr;
class ParseCtx;
class ExprMaker;
class QuadEmitter;

class ExprNormalizer
{
public:
    ExprNormalizer(
        ParseCtx *parse_ctx,
        ExprMaker *expr_maker,
        QuadEmitter *quad_emitter,
        QuadYielder *quad_yielder);

    const Expr *materialize_if_table_item(const Expr *expr);
    void resolve_bool_short_circuit(const Expr *expr);

private:
    ParseCtx *const parse_ctx_;
    ExprMaker *const expr_maker_;
    QuadEmitter *const quad_emitter_;
    QuadYielder *const quad_yielder_;
};
} // namespace alpha
#endif //EXPR_NORMALIZER_HPP
