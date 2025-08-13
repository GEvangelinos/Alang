#ifndef OLD_SEMANTIC_BUILDER_HPP
#define OLD_SEMANTIC_BUILDER_HPP
namespace Alpha
{
class SemanticBuilder
{
public:
    [[nodiscard]] Expr *make_call(Expr *lvalue, ExprList *&elist, Location call_loc);
    [[nodiscard]] Expr *make_normal_call(Expr *&lvalue, ExprList *&elist, Location call_loc);
    [[nodiscard]] Expr *make_method_call(Expr *&lvalue, ExprList *&elist, Location call_loc);
    [[nodiscard]] Expr *make_iife_call(const Function *func_symbol, ExprList *&elist,
                                       Location call_loc);
}; // class SemanticBuilder


} // namespace alpha

#endif // ALPHA_SEMANTIC_BUILDER_HPP

#endif //OLD_SEMANTIC_BUILDER_HPP
