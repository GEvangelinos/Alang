#ifndef EXPR_NORMALIZER_HPP
#define EXPR_NORMALIZER_HPP

namespace alpha
{
struct Expr;
class ParseCtx;
class ExprMaker;
class QuadEmitter;

class ExprNormalizer
{
public:
    ExprNormalizer(ParseCtx *parse_ctx, ExprMaker *expr_maker, QuadEmitter *quad_emitter);

    const Expr *materialize_if_table_item(const Expr *expr);
    void resolve_bool_short_circuit(const Expr *expr);

private:
    ParseCtx *const parse_ctx_;
    ExprMaker *const expr_maker_;
    QuadEmitter *const quad_emitter_;
};
} // namespace alpha
#endif //EXPR_NORMALIZER_HPP
