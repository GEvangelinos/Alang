#ifndef EXPR_FOLDER_HPP
#define EXPR_FOLDER_HPP

#include <type_traits>
#include "expr_maker.hpp"
#include "../../../../build/AUTOGEN/parser/include/parser/ir_opcode_info_traits.hpp"
#include "core/source_location.hpp"
#include "parser/ir_opcode.hpp"
#include "parser/ir_opcode_info_traits.hpp"
#include "parser/ir_opcode_opt_traits.hpp"

namespace alpha
{
class ExprFolder : private Immobile
{
public:
    struct Options
    {
        bool fold_arithmetic;
        bool fold_relational;
        bool fold_logical;
    };

    ExprFolder(ExprFolder::Options &&options, ExprMaker *expr_maker);

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
    const Options options_;

    template<typename... Exprs>
    [[nodiscard]] bool should_fold_arithmetic(const Exprs &... exprs);
    template<typename... Εxprs>
    [[nodiscard]] bool should_fold_relational_numeric(const Εxprs &... exprs);
    template<typename... Εxprs>
    [[nodiscard]] bool should_fold_relational_equality(const Εxprs &... exprs);
    template<typename... Exprs>
    [[nodiscard]] bool should_fold_logical(const Exprs &... exprs);
    ExprMaker *const expr_maker_;
};

class ExprTrimmer : private Immobile
{
public:
    struct Options {};

    ExprTrimmer(ExprTrimmer &&options, ExprMaker *expr_maker);

    [[nodiscard]] const Expr *try_trim_relational_equality(
        ir::Opcode opc, const Expr *lhs, const Expr *rhs, SourceLocation result_loc);
    [[nodiscard]] const Expr *try_trim_binary_arithmetic(
        ir::Opcode opc, const Expr *lhs, const Expr *rhs, SourceLocation result_loc);

private:
    const Options options_;

    ExprMaker *const expr_maker_;
};

class ExprOptimizer : private Immobile
{
public:
    struct Options
    {
        bool constant_propagation;
        bool fold_arithmetic;
        bool fold_relational;
        bool fold_logical;
    };

    ExprOptimizer(ExprOptimizer::Options &&options, ExprMaker *expr_maker);

    template<ir::Opcode opc, typename... Exprs>
    [[nodiscard]] const Expr *try_optimize(SourceLocation result_loc, Exprs... exprs);

private:
    const Expr *try_propagate_const(const Expr *expr);

    template<ir::Opcode opc, typename... Exprs>
    [[nodiscard]] const Expr *try_fold_optimize(SourceLocation result_loc, const Exprs &... exprs);

    template<ir::Opcode opc, typename... Exprs>
    [[nodiscard]] const Expr *try_trim_optimize(SourceLocation result_loc, const Exprs &... exprs);

    const Options options_;
    ExprFolder expr_folder_;
    ExprTrimmer expr_trimmer_;
};

template<typename... Exprs>
bool ExprFolder::should_fold_arithmetic(const Exprs &... exprs)
{
    static_assert(sizeof...(Exprs) >= 1, "should_fold_arithmetic: expects at least 1 const Expr *");
    static_assert(sizeof...(Exprs) <= 2, "should_fold_arithmetic: expects at max 2 const Expr *");
    static_assert((std::is_same_v<Exprs, const Expr *> && ...),
                  "should_fold_arithmetic: expects all arguments to be const Expr *");

    // We fold the variadic exprs into a single `and` joined expression.
    return options_.fold_arithmetic && (SemUtils::is_const_arithmetic_expr(exprs) && ...);
}

template<typename... Exprs>
bool ExprFolder::should_fold_relational_numeric(const Exprs &... exprs)
{
    static_assert(sizeof...(Exprs) == 2,
                  "should_fold_relational_arithmetic: expects exactly 2 const Expr *");
    static_assert((std::is_same_v<Exprs, const Expr *> && ...),
                  "should_fold_relational_arithmetic: expects all arguments to be const Expr *");

    // We fold the variadic exprs into a single `and` joined expression.
    return options_.fold_relational && (SemUtils::is_const_arithmetic_expr(exprs) && ...);
}

template<typename... Exprs>
bool ExprFolder::should_fold_relational_equality(const Exprs &... exprs)
{
    static_assert(sizeof...(Exprs) == 2,
                  "should_fold_relational_equality: expects exactly 2 const Expr *");
    static_assert((std::is_same_v<Exprs, const Expr *> && ...),
                  "should_fold_relational_equality: expects all arguments to be const Expr *");

    // We fold the variadic exprs into a single `and` joined expression.
    return options_.fold_relational && (SemUtils::is_static_expr(exprs) && ...);
}

template<typename... Exprs>
bool
ExprFolder::should_fold_logical(const Exprs &... exprs)
{
    static_assert(sizeof...(Exprs) >= 1, "should_fold_logical: expects at least 1 const Expr *");
    static_assert(sizeof...(Exprs) <= 2, "should_fold_logical: expects at max 2 const Expr *");
    static_assert((std::is_same_v<Exprs, const Expr *> && ...),
                  "should_fold_logical: expects all arguments to be const Expr *");

    // We fold the variadic exprs into a single `or` joined expression.
    // Why `or` because in logical operators AND, OR, we can even do partial folding.
    // e.g.: true and var => var
    return options_.fold_logical && (SemUtils::is_const_bool_expr(exprs) || ...);
}

template<ir::Opcode opc, typename... Exprs>
const Expr *ExprOptimizer::try_optimize(const SourceLocation result_loc, Exprs... exprs)
{
    ((exprs = try_propagate_const(exprs)), ...);
    UNREACHABLE("REMOVE THIS LINE. But be sure when to do constant propagation valid is valid");
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
ExprOptimizer::try_trim_optimize(const SourceLocation result_loc, const Exprs &... exprs)
{
    static_assert((std::is_same_v<Exprs, const Expr *> && ...), "all args must be const Expr *");
    static_assert(ir::opt_traits::is_trimmable(opc), "`trimming` not supported for this ir::Opcode");
    static_assert(sizeof...(exprs) == ir::info_traits::operands(opc), "arg count mismatch");

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
        else if constexpr (SemUtils::is_binary_arithmetic_iropcode(opc))
            return expr_trimmer_.try_trim_binary_arithmetic(opc, lhs, rhs, result_loc);
        else
            return nullptr; // We don't have any trim optimizations yet.
    }
    else static_assert([] { return false; }(), "trimmable ir::Opcode not handled.");
}
} // namespace alpha

#endif //EXPR_FOLDER_HPP
