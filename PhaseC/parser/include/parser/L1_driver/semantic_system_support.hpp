#ifndef SEMANTIC_SYSTEM_SUPPORT_HPP
#define SEMANTIC_SYSTEM_SUPPORT_HPP

namespace alpha
{
class SymbolTable;
class ParseCtx;
class DiagnosticReporter;
class ExprMaker;
class QuadHandler;
class QuadYielder;
class ExprNormalizer;
class ExprOptimizer;

struct SemanticSystemServices
{
    SymbolTable *const symbol_table = nullptr;
    ParseCtx *const parse_ctx = nullptr;
    DiagnosticReporter *const dr = nullptr;
    ExprMaker *const expr_maker = nullptr;
    QuadHandler *const quad_handler = nullptr;
    QuadYielder *const quad_yielder = nullptr;
    ExprNormalizer *const expr_normalizer = nullptr;
    ExprOptimizer *const expr_optimizer = nullptr;
};
} // namespace alpha
#endif // SEMANTIC_SYSTEM_SUPPORT_HPP
