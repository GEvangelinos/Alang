#ifndef EXPR_NORMALIZER_HPP
#define EXPR_NORMALIZER_HPP

namespace alpha
{
class QuadInterceptor;
class QuadYielder;
struct Expr;
class ParseCtx;
class ExprMaker;
class QuadHandler;

class ExprNormalizer
{
public:
    ExprNormalizer(
        ParseCtx *parse_ctx,
        ExprMaker *expr_maker,
        QuadHandler *quad_handler,
        QuadInterceptor *quad_interceptor,
        QuadYielder *quad_yielder);

    const Expr *materialize_if_table_item(const Expr *expr);
    void resolve_bool_short_circuit(const Expr *expr);

private:
    ParseCtx *const parse_ctx_;
    ExprMaker *const expr_maker_;
    QuadHandler *const quad_handler_;
    QuadYielder *const quad_yielder_;
    QuadInterceptor *const quad_interceptor_;
};
} // namespace alpha
#endif //EXPR_NORMALIZER_HPP
