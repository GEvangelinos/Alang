#ifndef EXPR_TYPE_TRAITS_HPP
#define EXPR_TYPE_TRAITS_HPP

#include <type_traits>
#include "core/ir/ir_expr.hpp"

namespace alpha::expr_traits::cmp_bitmasks
{
using ExprTypeMask = u16;

// -=========================  Expr::Type Mask Capacity Check =========================-
#if defined(EXPR_TYPE_COUNT) || defined(EXPR_MASK_BIT_COUNT)
#error "Macro collision detected"
#endif
#define EXPR_TYPE_COUNT      (static_cast<std::underlying_type_t<Expr::Type>>(Expr::Type::COUNT__))
#define EXPR_MASK_BIT_COUNT  (sizeof(ExprTypeMask) * 8)
static_assert(
    EXPR_TYPE_COUNT <= EXPR_MASK_BIT_COUNT,
    "ExprMaskUT is too small to represent all Expr::Type bit flags.\n"
    "Possible causes:\n"
    "  1. You assigned explicit large values to Expr::Type enumerators.\n"
    "  2. You added more Expr::Type entries than the mask can hold.\n"
    "     Fix: Use a larger integral type for ExprMaskUT (e.g. uint64_t).\n"
);
#undef EXPR_TYPE_COUNT
#undef EXPR_MASK_BIT_COUNT
// -===================================================================================-

constexpr ExprTypeMask to_bitmask(Expr::Type t)
{
    return 1ULL << static_cast<std::underlying_type_t<Expr::Type>>(t);
}

//                   ^^^ Local macro helper, #undefed at end of file. ^^^
// -================================ Expr::Type -> Bitmask ============================-
#ifdef TO_BITMASK
#error "Macro collission detected"
#endif
#define TO_BITMASK(expr_type) to_bitmask(Expr::Type::expr_type)

// -================================ Expr::Type -> Bitmask ============================-

#ifdef LVALUE_MASK
#error "Macro collision detected"
#endif
#define LVALUE_MASK          \
    TO_BITMASK(ASSIGN)       \
    | TO_BITMASK(TABLE_ITEM) \
    | TO_BITMASK(VARIABLE)

inline constexpr ExprTypeMask arithmetic =
    LVALUE_MASK
    | TO_BITMASK(ARITHMETIC)
    | TO_BITMASK(CONST_FLOAT)
    | TO_BITMASK(CONST_INT);

// In language Alpha everything can be compared with a bool (so we can ignore it, comment it out))
// inline constexpr ExprTypeMask boolean = ~static_cast<ExprTypeMask>(0);

inline constexpr ExprTypeMask string =
    LVALUE_MASK
    | TO_BITMASK(CONST_STRING);

inline constexpr ExprTypeMask aggregate =
    LVALUE_MASK
    | TO_BITMASK(CONST_NIL)
    | TO_BITMASK(NEW_TABLE);

inline constexpr ExprTypeMask libfunc =
    LVALUE_MASK
    | TO_BITMASK(LIBRARY_FUNCTION);

inline constexpr ExprTypeMask progfunc =
    LVALUE_MASK
    | TO_BITMASK(PROGRAM_FUNCTION);

#undef TO_BITMASK
#undef LVALUE_MASK
} // alpha::expr_traits::cmp_bitmasks

#endif // EXPR_TYPE_TRAITS_HPP
