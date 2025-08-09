#ifndef EXPR_OPTIMIZER_HPP
#define EXPR_OPTIMIZER_HPP

#include <type_traits>
#include "expr_maker.hpp"
#include <parser/ir_opcode_info_traits.hpp>
#include "core/source_location.hpp"
#include "parser/ir_opcode.hpp"
#include <parser/ir_opcode_opt_traits.hpp>

namespace alpha
{
class ExprFolder : private Immobile
{
public:
    explicit ExprFolder(ExprMaker *expr_maker);

    [[nodiscard]] const Expr *try_fold_arithmetic_uminus(
        const Expr *expr, SourceLocation result_loc);
    [[nodiscard]] const Expr *try_fold_arithmetic_binary(
        ir::Opcode opc, const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
    [[nodiscard]] const Expr *try_fold_relational_numeric(
        ir::Opcode opc, const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
    [[nodiscard]] const Expr *try_fold_relational_equality(
        ir::Opcode opc, const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
    [[nodiscard]] const Expr *try_fold_logical_or(
        const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
    [[nodiscard]] const Expr *try_fold_logical_and(
        const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
    [[nodiscard]] const Expr *try_fold_logical_not(const Expr *expr, SourceLocation result_loc);

private:
    ExprMaker *const expr_maker_;

    [[nodiscard]] static bool should_fold_arithmetic(const Expr *expr);
    [[nodiscard]] static bool should_fold_arithmetic(const Expr *lhs, const Expr *rhs);
    [[nodiscard]] static bool should_fold_relational_numeric(const Expr *lhs, const Expr *rhs);
    [[nodiscard]] static bool should_fold_relational_equality(const Expr *lhs, const Expr *rhs);
    [[nodiscard]] static bool should_fold_logical(const Expr *expr);
    [[nodiscard]] static bool should_fold_logical(const Expr *lhs, const Expr *rhs);
};

class ExprTrimmer : private Immobile
{
public:
    explicit ExprTrimmer(ExprMaker *expr_maker);

    [[nodiscard]] const Expr *try_trim_binary_arithmetic(
        ir::Opcode opc, const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
    [[nodiscard]] const Expr *try_trim_relational_equality(
        ir::Opcode opc, const Expr *lhs, const Expr *rhs, SourceLocation result_loc);

private:
    ExprMaker *const expr_maker_;
};

class ExprOptimizer : private Immobile
{
public:
    struct Options
    {
        bool constant_propagation;
        bool expr_folding;
        bool expr_trimming;
    };

    ExprOptimizer(ExprOptimizer::Options &&options, ExprMaker *expr_maker);

    template<ir::Opcode opc, typename... Exprs>
    [[nodiscard]] const Expr *try_optimize(SourceLocation result_loc, Exprs... exprs);

private:
    const Options options_;
    ExprFolder expr_folder_;
    ExprTrimmer expr_trimmer_;

    [[nodiscard]] const Expr *try_propagate_const(const Expr *expr);

    template<ir::Opcode opc, typename... Exprs>
    [[nodiscard]] const Expr *try_fold_optimize(SourceLocation result_loc, const Exprs &... exprs);
    template<ir::Opcode opc, typename... Exprs>
    [[nodiscard]] const Expr *try_trim_optimize(SourceLocation result_loc, const Exprs &... exprs);
};

inline bool ExprFolder::should_fold_arithmetic(const Expr *const expr)
{
    return SemUtils::is_const_arithmetic_expr(expr);
}

inline bool ExprFolder::should_fold_arithmetic(const Expr *const lhs, const Expr *const rhs)
{
    return SemUtils::is_const_arithmetic_expr(lhs) && SemUtils::is_const_arithmetic_expr(rhs);
}

inline bool ExprFolder::should_fold_relational_numeric(const Expr *const lhs, const Expr *const rhs)
{
    return SemUtils::is_const_arithmetic_expr(lhs) && SemUtils::is_const_arithmetic_expr(rhs);
}

inline bool ExprFolder::should_fold_relational_equality(const Expr *lhs, const Expr *rhs)
{
    return SemUtils::is_static_expr(lhs) && SemUtils::is_static_expr(rhs);
}

inline bool ExprFolder::should_fold_logical(const Expr *expr)
{
    return SemUtils::is_const_bool_expr(expr);
}

inline bool ExprFolder::should_fold_logical(const Expr *lhs, const Expr *rhs)
{
    return SemUtils::is_const_bool_expr(lhs) && SemUtils::is_const_bool_expr(rhs);
}

template<ir::Opcode opc, typename... Exprs>
const Expr *ExprOptimizer::try_optimize(const SourceLocation result_loc, Exprs... exprs)
{
    static_assert((std::is_same_v<Exprs, const Expr *> && ...), "All args must be `const Expr *`");
    static_assert(sizeof...(exprs) > 0, "Received 0 `const Expr *` args");

    ((exprs = try_propagate_const(exprs)), ...);
    if constexpr (ir::opt_traits::is_foldable(opc))
        if (const Expr *folded = try_fold_optimize<opc>(result_loc, exprs...))
            return folded;
    if constexpr (ir::opt_traits::is_trimmable(opc))
        if (const Expr *trimmed = try_trim_optimize<opc>(result_loc, exprs...))
            return trimmed;
    return nullptr;
}

template<ir::Opcode opc, typename... Exprs>
const Expr *
ExprOptimizer::try_fold_optimize(const SourceLocation result_loc, const Exprs &... exprs)
{
    static_assert((std::is_same_v<Exprs, const Expr *> && ...), "all args must be const Expr *");
    static_assert(ir::opt_traits::is_foldable(opc), "`folding`  not supported for this ir::Opcode");
    static_assert(sizeof...(exprs) == ir::info_traits::operands(opc), "exprs-operands mismatch");

    auto expr_tuple = std::forward_as_tuple(exprs...);
    if constexpr (ir::info_traits::operands(opc) == 1)
    {
        auto &unary_expr = std::get<0>(expr_tuple);
        if constexpr (opc == ir::Opcode::UMINUS)
            return expr_folder_.try_fold_arithmetic_uminus(unary_expr, result_loc);
        else if constexpr (opc == ir::Opcode::NOT)
            return expr_folder_.try_fold_logical_not(unary_expr, result_loc);
        else
            static_assert([]() { return false; }(),
                          "try_fold_optimize: not sure how to optimize this unary ir::Opcode");
    }
    else if constexpr (ir::info_traits::operands(opc) == 2)
    {
        auto &lhs = std::get<0>(expr_tuple);
        auto &rhs = std::get<1>(expr_tuple);

        if constexpr (opc == ir::Opcode::ADD || opc == ir::Opcode::SUB ||
                      opc == ir::Opcode::MUL || opc == ir::Opcode::DIV || opc == ir::Opcode::MOD)
            return expr_folder_.try_fold_arithmetic_binary(opc, lhs, rhs, result_loc);
        else if constexpr (opc == ir::Opcode::AND)
            return expr_folder_.try_fold_logical_and(lhs, rhs, result_loc);
        else if constexpr (opc == ir::Opcode::OR)
            return expr_folder_.try_fold_logical_or(lhs, rhs, result_loc);
        else if constexpr (opc == ir::Opcode::IF_EQ || opc == ir::Opcode::IF_NEQ)
            return expr_folder_.try_fold_relational_equality(opc, lhs, rhs, result_loc);
        else if constexpr (opc == ir::Opcode::IF_LT || opc == ir::Opcode::IF_LTE ||
                           opc == ir::Opcode::IF_GT || opc == ir::Opcode::IF_GTE)
            return expr_folder_.try_fold_relational_numeric(opc, lhs, rhs, result_loc);
        else
            static_assert([]() { return false; }(),
                          "try_fold_optimize: not sure how to optimize binary ir::Opcode");
    }
    else static_assert([] { return false; }(), "foldable ir::Opcode not handled.");
}

template<ir::Opcode opc, typename... Exprs>
const Expr *
ExprOptimizer::try_trim_optimize(const SourceLocation result_loc, const Exprs &... exprs)
{
    static_assert((std::is_same_v<Exprs, const Expr *> && ...), "all args must be const Expr *");
    static_assert(ir::opt_traits::is_trimmable(opc),
                  "`trimming` not supported for this ir::Opcode");
    static_assert(sizeof...(exprs) == ir::info_traits::operands(opc), "exprs-operands mismatch");

    if constexpr (ir::info_traits::operands(opc) == 2)
    {
        // NOTE: Move expr_tuple outside this block if needed for other constexpr branches later.
        auto expr_tuple = std::forward_as_tuple(exprs...);
        auto &lhs = std::get<0>(expr_tuple);
        auto &rhs = std::get<1>(expr_tuple);

        if constexpr (opc == ir::Opcode::ASSIGN)
            return lhs == rhs ? lhs : nullptr; // expr = expr -> delete self-assignment (useless).
        if constexpr (opc == ir::Opcode::IF_EQ || opc == ir::Opcode::IF_NEQ)
            return expr_trimmer_.try_trim_relational_equality(opc, lhs, rhs, result_loc);
        else if constexpr (SemUtils::is_binary_arithmetic_opcode(opc))
            return expr_trimmer_.try_trim_binary_arithmetic(opc, lhs, rhs, result_loc);
        else
            return nullptr; // We don't have any trim optimizations yet.
    }
    else static_assert([] { return false; }(), "trimmable ir::Opcode not handled.");
}
} // namespace alpha

#endif // EXPR_OPTIMIZER_HPP
