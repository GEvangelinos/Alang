#include <cmath>
#include <parser/ir_opcode.gen.hpp>
#include <parser/semantic_utils.hpp>
#include <../../include/parser/L2_semantic_subsystems/core/expr_optimizer.hpp>
#include "L1_driver/semantic_system.hpp"

namespace alpha
{
[[nodiscard]] static const Expr*
try_trim_add(ExprMaker* expr_maker, const Expr* lhs, const Expr* rhs, SourceLocation add_loc);
[[nodiscard]] static const Expr*
try_trim_sub(ExprMaker* expr_maker, const Expr* lhs, const Expr* rhs, SourceLocation sub_loc);
[[nodiscard]] static const Expr*
try_trim_mul(ExprMaker* expr_maker, const Expr* lhs, const Expr* rhs, SourceLocation mul_loc);
[[nodiscard]] static const Expr*
try_trim_div(ExprMaker* expr_maker, const Expr* lhs, const Expr* rhs, SourceLocation div_loc);
[[nodiscard]] static const Expr*
try_trim_mod(ExprMaker* expr_maker, const Expr* lhs, const Expr* rhs, SourceLocation mod_loc);

ExprOptimizer::ExprOptimizer(const settings::ExprOpts& expr_opts, ExprMaker* const expr_maker)
    : expr_opts_(expr_opts),
      expr_maker_(support::require_ptr(expr_maker)),
      expr_folder_(support::require_ptr(expr_maker)),
      expr_trimmer_(support::require_ptr(expr_maker)) {}

ExprFolder::ExprFolder(ExprMaker* const expr_maker)
    : expr_maker_(support::require_ptr(expr_maker)) {}

ExprTrimmer::ExprTrimmer(ExprMaker* const expr_maker)
    : expr_maker_(expr_maker) {}

const Expr*
ExprFolder::try_fold_arithmetic_uminus(const Expr* const expr, const SourceLocation result_loc)
{
    DMASSERT(!!expr);
    if (!ExprFolder::should_fold_arithmetic(expr))
        return nullptr;

    switch (expr->type)
    {
    case Expr::Type::CONST_INT:
        return expr_maker_->make_const_int_expr(
            result_loc, -static_cast<const ConstIntExpr*>(expr)->value);
    case Expr::Type::CONST_FLOAT:
        return expr_maker_->make_const_float_expr(
            result_loc, -static_cast<const ConstFloatExpr*>(expr)->value);
    default: [[unlikely]] throw std::logic_error(ATTACH_CONTEXT("Needed const numeric expr."));
    }
}

const Expr*
ExprFolder::try_fold_arithmetic_binary(
    const ir::Opcode opc,
    const Expr* const lhs,
    const Expr* const rhs,
    const SourceLocation result_loc)
{
    const auto fold_arith_op = [this, opc, result_loc](const auto l, const auto r) -> const Expr*
    {
        const auto make_const_num_expr = [this, result_loc](const auto num) -> const Expr*
        {
            if constexpr (std::is_integral_v<decltype(num)>)
                return expr_maker_->make_const_int_expr(result_loc, num);
            else if constexpr (std::is_floating_point_v<decltype(num)>)
                return expr_maker_->make_const_float_expr(result_loc, num);
            else
                static_assert(always_false_v<void>, "Unsupported numeric Alpha type");
        };

        switch (opc)
        {
        case ir::Opcode::ADD: return make_const_num_expr(l + r);
        case ir::Opcode::SUB: return make_const_num_expr(l - r);
        case ir::Opcode::MUL: return make_const_num_expr(l * r);
        case ir::Opcode::DIV: return make_const_num_expr(l / r);
        case ir::Opcode::MOD:
            if constexpr (std::is_integral_v<decltype(l)> && std::is_integral_v<decltype(r)>)
                return expr_maker_->make_const_int_expr(result_loc, l % r);
            else
                return expr_maker_->make_const_float_expr(result_loc, std::fmod(l, r));
        default: [[unlikely]] throw std::logic_error(ATTACH_CONTEXT("(Needed arithmetic IOPC"));
        }
    };

    DMASSERT(!!lhs, !!rhs, SemUtils::is_binary_arithmetic_opcode(opc));
    if (!ExprFolder::should_fold_arithmetic(lhs, rhs))
        return nullptr;

    return lhs->type == Expr::Type::CONST_INT && rhs->type == Expr::Type::CONST_INT
           ? fold_arith_op(static_cast<const ConstIntExpr*>(lhs)->value,
                           static_cast<const ConstIntExpr*>(rhs)->value)
           : fold_arith_op(SemUtils::extract_alpha_float(lhs), SemUtils::extract_alpha_float(rhs));
}

const Expr*
ExprFolder::try_fold_relational_numeric(
    const ir::Opcode opc,
    const Expr* const lhs,
    const Expr* const rhs,
    const SourceLocation result_loc)
{
    const auto fold_rel_num_op = [this, opc, result_loc](const auto l, const auto r) -> const Expr*
    {
        switch (opc)
        {
        case ir::Opcode::IF_GT:
            return expr_maker_->make_const_bool_expr(result_loc, l > r);
        case ir::Opcode::IF_GTE:
            return expr_maker_->make_const_bool_expr(result_loc, l >= r);
        case ir::Opcode::IF_LT:
            return expr_maker_->make_const_bool_expr(result_loc, l < r);
        case ir::Opcode::IF_LTE:
            return expr_maker_->make_const_bool_expr(result_loc, l <= r);
        default:
            throw std::logic_error(ATTACH_CONTEXT("Needed relational arithmetic IOPC"));
        }
    };

    DMASSERT(
        !!lhs, !!rhs,
        SemUtils::is_relational_numeric_iropcode(opc)
    );
    if (!ExprFolder::should_fold_relational_numeric(lhs, rhs))
        return nullptr;

    return lhs->type == Expr::Type::CONST_INT && rhs->type == Expr::Type::CONST_INT
           ? fold_rel_num_op(static_cast<const ConstIntExpr*>(lhs)->value,
                             static_cast<const ConstIntExpr*>(rhs)->value)
           : fold_rel_num_op(SemUtils::extract_alpha_float(lhs),
                             SemUtils::extract_alpha_float(rhs));
}

const Expr*
ExprFolder::try_fold_relational_equality(
    const ir::Opcode opc,
    const Expr* const lhs,
    const Expr* const rhs,
    const SourceLocation result_loc)

{
    DMASSERT(
        !!lhs, !!rhs,
        SemUtils::is_relational_equality_iropcode(opc)
    );
    if (!should_fold_relational_equality(lhs, rhs))
        return nullptr;

    const auto equality_check = [lhs, rhs]() -> bool
    {
        using ET = Expr::Type;
        if (lhs->type == ET::CONST_BOOL || rhs->type == ET::CONST_BOOL)
            return SemUtils::as_bool(lhs) == SemUtils::as_bool(rhs);
        if (lhs->is_const_arithmetic() && rhs->is_const_arithmetic())
            return SemUtils::extract_alpha_float(rhs) == SemUtils::extract_alpha_float(lhs);
        if (lhs->type == ET::CONST_NIL && rhs->type == ET::CONST_NIL)
            return true;
        if (lhs->type == ET::CONST_STRING && rhs->type == ET::CONST_STRING)
        {
            const StringSpan lhs_val = static_cast<const ConstStringExpr*>(lhs)->value;
            const StringSpan rhs_val = static_cast<const ConstStringExpr*>(rhs)->value;
            return std::string_view{lhs_val.data, lhs_val.size} ==
                   std::string_view{rhs_val.data, rhs_val.size};
        }
        if (lhs->type == ET::LIBRARY_FUNCTION && rhs->type == ET::LIBRARY_FUNCTION)
            return static_cast<const LibFuncExpr*>(lhs)->libfunc_symbol->name ==
                   static_cast<const LibFuncExpr*>(rhs)->libfunc_symbol->name;
        if (lhs->type == ET::PROGRAM_FUNCTION && rhs->type == ET::PROGRAM_FUNCTION)
            return static_cast<const ProgFuncExpr*>(lhs)->progfunc_symbol->address ==
                   static_cast<const ProgFuncExpr*>(rhs)->progfunc_symbol->address;
        UNREACHABLE("Some static combination is not handled");
    };

    if (opc == ir::Opcode::IF_EQ)
        return expr_maker_->make_const_bool_expr(result_loc, equality_check());
    if (opc == ir::Opcode::IF_NEQ)
        return expr_maker_->make_const_bool_expr(result_loc, !equality_check());
    throw std::logic_error(ATTACH_CONTEXT("Needed equality ir::Opcode"));
}

const Expr*
ExprFolder::try_fold_logical_or(
    const Expr* const lhs,
    const Expr* const rhs,
    const SourceLocation result_loc)
{
    DMASSERT(!!lhs, !!rhs);
    if (!ExprFolder::should_fold_logical(lhs, rhs))
        return nullptr;

    DMASSERT(lhs->type == Expr::Type::CONST_BOOL, rhs->type == Expr::Type::CONST_BOOL);
    const auto bool_lhs = static_cast<const ConstBoolExpr*>(lhs)->value;
    const auto bool_rhs = static_cast<const ConstBoolExpr*>(rhs)->value;

    return expr_maker_->make_const_bool_expr(result_loc, bool_lhs || bool_rhs);
}

const Expr*
ExprFolder::try_fold_logical_and(
    const Expr* const lhs,
    const Expr* const rhs,
    const SourceLocation result_loc)
{
    DMASSERT(!!lhs, !!rhs);
    if (!ExprFolder::should_fold_logical(lhs, rhs))
        return nullptr;

    DMASSERT(lhs->type == Expr::Type::CONST_BOOL, rhs->type == Expr::Type::CONST_BOOL);
    const auto bool_lhs = static_cast<const ConstBoolExpr*>(lhs)->value;
    const auto bool_rhs = static_cast<const ConstBoolExpr*>(rhs)->value;

    return expr_maker_->make_const_bool_expr(result_loc, bool_lhs && bool_rhs);
}

const Expr*
ExprFolder::try_fold_logical_not(
    const Expr* const expr,
    const SourceLocation result_loc)
{
    DMASSERT(!!expr);
    if (!ExprFolder::should_fold_logical(expr))
        return nullptr;

    return expr_maker_->make_const_bool_expr(
        result_loc, !static_cast<const ConstBoolExpr*>(expr)->value
    );
}

bool
ExprFolder::should_fold_arithmetic(const Expr* const expr)
{
    return DEBUG_REQUIRE_PTR(expr)->is_const_arithmetic();
}

bool
ExprFolder::should_fold_arithmetic(const Expr* const lhs, const Expr* const rhs)
{
    return DEBUG_REQUIRE_PTR(lhs)->is_const_arithmetic() &&
           DEBUG_REQUIRE_PTR(rhs)->is_const_arithmetic();
}

bool
ExprFolder::should_fold_relational_numeric(const Expr* const lhs, const Expr* const rhs)
{
    return DEBUG_REQUIRE_PTR(lhs)->is_const_arithmetic() &&
           DEBUG_REQUIRE_PTR(rhs)->is_const_arithmetic();
}

bool
ExprFolder::should_fold_relational_equality(const Expr* const lhs, const Expr* const rhs)
{
    return DEBUG_REQUIRE_PTR(lhs)->is_static() &&
           DEBUG_REQUIRE_PTR(rhs)->is_static();
}

bool
ExprFolder::should_fold_logical(const Expr* const expr)
{
    return DEBUG_REQUIRE_PTR(expr)->type == Expr::Type::CONST_BOOL;
}

bool
ExprFolder::should_fold_logical(const Expr* const lhs, const Expr* const rhs)
{
    return DEBUG_REQUIRE_PTR(lhs)->type == Expr::Type::CONST_BOOL &&
           DEBUG_REQUIRE_PTR(rhs)->type == Expr::Type::CONST_BOOL;
}

const Expr*
ExprTrimmer::try_trim_binary_arithmetic(
    const ir::Opcode opc,
    const Expr* const lhs,
    const Expr* const rhs,
    const SourceLocation result_loc)
{
    DMASSERT(!!lhs, !!rhs);
    switch (opc)
    {
    case ir::Opcode::ADD: return try_trim_add(expr_maker_, lhs, rhs, result_loc);
    case ir::Opcode::SUB: return try_trim_sub(expr_maker_, lhs, rhs, result_loc);
    case ir::Opcode::MUL: return try_trim_mul(expr_maker_, lhs, rhs, result_loc);
    case ir::Opcode::DIV: return try_trim_div(expr_maker_, lhs, rhs, result_loc);
    case ir::Opcode::MOD: return try_trim_mod(expr_maker_, lhs, rhs, result_loc);
        [[unlikely]] default: throw std::logic_error(
            ATTACH_CONTEXT("Expected a binary arithmetic ir::Opcode"));
    }
}

const Expr*
ExprTrimmer::try_trim_relational_equality(
    const ir::Opcode opc,
    const Expr* const lhs,
    const Expr* const rhs,
    const SourceLocation result_loc)
{
    if (opc == ir::Opcode::IF_EQ)
    {
        // 1 == var(true) -> var(true), 1 == var(false) -> var(false) => 1 == var -> var
        if (lhs->is_static() && SemUtils::as_bool(lhs) == true)
            return expr_maker_->clone_with_updated_location(result_loc, rhs);
        if (rhs->is_static() && SemUtils::as_bool(rhs))
            return expr_maker_->clone_with_updated_location(result_loc, lhs);
    }
    if (opc == ir::Opcode::IF_NEQ) // var != 0 -> var  ,  0 != var -> var
    {
        if (lhs->is_static() && SemUtils::as_bool(lhs) == false)
            return expr_maker_->clone_with_updated_location(result_loc, rhs);
        if (rhs->is_static() && SemUtils::as_bool(rhs) == false)
            return expr_maker_->clone_with_updated_location(result_loc, lhs);
    }
    return nullptr; // Trimming failed (most common scenario)
}

const Expr*
ExprTrimmer::try_trim_binary_logical(
    const ir::Opcode opc,
    const Expr* const lhs,
    const Expr* const rhs,
    const SourceLocation result_loc)
{
    if (opc == ir::Opcode::OR)
    {
        if (lhs->is_const_false()) // false OR var = var
            return expr_maker_->clone_with_updated_location(result_loc, rhs);
        if (rhs->is_const_false()) // var OR false = var
            return expr_maker_->clone_with_updated_location(result_loc, lhs);
        if (lhs->is_const_true() || rhs->is_const_true())
            return expr_maker_->make_const_bool_expr(result_loc, true);
    }
    if (opc == ir::Opcode::AND)
    {
        if (lhs->is_const_true()) // true AND var = var
            return expr_maker_->clone_with_updated_location(result_loc, rhs);
        if (rhs->is_const_true()) // var AND true = var
            return expr_maker_->clone_with_updated_location(result_loc, lhs);
        if (lhs->is_const_false() || rhs->is_const_false())
            return expr_maker_->make_const_bool_expr(result_loc, false);
    }
    return nullptr;
}

const Expr*
ExprOptimizer::try_propagate_const(const Expr* const expr)
{
    DMASSERT(!!expr);
    if (!expr_opts_.opt_const_propagation) [[unlikely]] // We optimize for fully optimized setups.
        return expr;
    if (expr->type != Expr::Type::VARIABLE)
        return expr;
    const VarSymbol* const var_symbol = static_cast<const VariableExpr*>(expr)->var_symbol;
    DMASSERT(!!var_symbol); // All VariableExpr must be tied to a Variable(Symbol);
    if (!var_symbol->has_const_value())
        return expr;

    // We need to update the location cause the point of use is different from point of const decl.
    return expr_maker_->clone_with_updated_location(expr->loc, var_symbol->get_const_expr());
}

const Expr*
try_trim_add(
    ExprMaker* const expr_maker,
    const Expr* const lhs,
    const Expr* const rhs,
    const SourceLocation add_loc)
{
    DMASSERT(!!expr_maker, !!lhs, !!rhs);
    // 0 + x -> x and x + 0 -> x
    if (lhs->is_const_0()) return expr_maker->clone_with_updated_location(add_loc, rhs);
    if (rhs->is_const_0()) return expr_maker->clone_with_updated_location(add_loc, lhs);
    return nullptr; // Trimming failed (most common scenario)
}

const Expr*
try_trim_sub(
    ExprMaker* const expr_maker,
    const Expr* const lhs,
    const Expr* const rhs,
    const SourceLocation sub_loc)
{
    DMASSERT(!!expr_maker, !!lhs, !!rhs);
    // x - 0 -> x
    if (rhs->is_const_0()) return expr_maker->clone_with_updated_location(sub_loc, lhs);
    return nullptr; // Trimming failed (most common scenario)
}

const Expr*
try_trim_mul(
    ExprMaker* const expr_maker,
    const Expr* const lhs,
    const Expr* const rhs,
    const SourceLocation mul_loc)
{
    DMASSERT(!!expr_maker, !!lhs, !!rhs);
    // x * 0 -> 0 and 0 * x -> 0
    if (lhs->is_const_0() || rhs->is_const_0())
        return expr_maker->make_const_int_expr(mul_loc, 0);

    // x * 1 -> x and 1 * x -> x
    if (lhs->is_const_1()) return expr_maker->clone_with_updated_location(mul_loc, rhs);
    if (rhs->is_const_1()) return expr_maker->clone_with_updated_location(mul_loc, lhs);
    return nullptr; // Trimming failed (most common scenario)
}

const Expr*
try_trim_div(
    ExprMaker* const expr_maker,
    const Expr* const lhs,
    const Expr* const rhs,
    const SourceLocation div_loc)
{
    DMASSERT(!!expr_maker, !!lhs, !!rhs);
    if (rhs->is_const_1()) return expr_maker->clone_with_updated_location(div_loc, lhs);
    return nullptr; // Trimming failed (most common scenario)
}

const Expr*
try_trim_mod(
    ExprMaker* const expr_maker,
    [[maybe_unused]] const Expr* const lhs,
    const Expr* const rhs,
    const SourceLocation mod_loc)
{
    DMASSERT(!!expr_maker, !!lhs, !!rhs);
    // x % 1 -> 0
    if (rhs->is_const_1()) return expr_maker->make_const_int_expr(mod_loc, 0);
    return nullptr; // Trimming failed (most common scenario)
}
} // namespace alpha
