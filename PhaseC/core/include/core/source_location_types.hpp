#ifndef SOURCE_LOCATION_TYPES_HPP
#define SOURCE_LOCATION_TYPES_HPP

#include "core/numeric_types.hpp" // for word

namespace alpha
{
#ifdef DEFINE_SRC_IDX_TYPE
#error "Macro collision detected"
#endif
#define DEFINE_SRC_IDX_TYPE_STRUCT(StructName)                                               \
    struct StructName                                                                        \
    {                                                                                        \
        using UnderlyingType = Word;                                                         \
        static_assert(std::is_unsigned_v<UnderlyingType>);                                   \
                                                                                             \
        static constexpr UnderlyingType none = 0;                                            \
        UnderlyingType value;                                                                \
                                                                                             \
        constexpr StructName() : value(none) {}                                              \
        explicit constexpr StructName(const UnderlyingType value) noexcept : value(value) {} \
        constexpr StructName(const StructName &rhs) noexcept : value(rhs.value) {}           \
        [[nodiscard]] constexpr auto operator<=>(const StructName &rhs) const noexcept = default; \
        constexpr StructName &operator=(StructName rhs) noexcept;                            \
        constexpr StructName &operator+=(StructName rhs) noexcept;                           \
        constexpr StructName &operator-=(StructName rhs) noexcept;                           \
        [[nodiscard]] constexpr StructName operator+(StructName rhs) const noexcept;         \
        [[nodiscard]] constexpr StructName operator-(StructName rhs) const noexcept;         \
        constexpr StructName &operator++() noexcept;                                         \
        constexpr StructName operator++(int) noexcept;                                       \
        constexpr StructName &operator--() noexcept;                                         \
        constexpr StructName operator--(int) noexcept;                                       \
        [[nodiscard]] constexpr bool is_none() const noexcept { return value == none; }      \
    };                                                                                       \
    static_assert(sizeof(StructName) <= 8, "If false in overloaded operators I must reconsider pass rhs by ref (not by value)");

    DEFINE_SRC_IDX_TYPE_STRUCT(SrcLineIdx)
    DEFINE_SRC_IDX_TYPE_STRUCT(SrcColumnIdx)
    DEFINE_SRC_IDX_TYPE_STRUCT(SrcBuffIdx)
#undef DEFINE_SRC_IDX_TYPE_STRUCT

#ifdef DEFINE_SRC_IDX_TYPE_METHOD_IMPLS
#error "Macro collision detected"
#endif
#define DEFINE_SRC_IDX_TYPE_METHOD_IMPLS(StructName)                                \
    constexpr StructName &StructName::operator=(const StructName rhs) noexcept      \
    {                                                                               \
        value = rhs.value;                                                          \
        return *this;                                                               \
    }                                                                               \
                                                                                    \
    constexpr StructName &StructName::operator+=(const StructName rhs) noexcept     \
    {                                                                               \
        value += rhs.value;                                                         \
        return *this;                                                               \
    }                                                                               \
                                                                                    \
    constexpr StructName &StructName::operator-=(const StructName rhs) noexcept     \
    {                                                                               \
        value -= rhs.value;                                                         \
        return *this;                                                               \
    }                                                                               \
                                                                                    \
    constexpr StructName StructName::operator+(const StructName rhs) const noexcept \
    {                                                                               \
        return StructName{value + rhs.value};                                       \
    }                                                                               \
                                                                                    \
    constexpr StructName StructName::operator-(const StructName rhs) const noexcept \
    {                                                                               \
    return StructName{value - rhs.value};                                           \
    }                                                                               \
                                                                                    \
    constexpr StructName &StructName::operator++() noexcept                         \
    {                                                                               \
        ++value;                                                                    \
        return *this;                                                               \
    }                                                                               \
                                                                                    \
    constexpr StructName StructName::operator++(int) noexcept                       \
    {                                                                               \
        auto tmp = *this;                                                           \
        ++*this;                                                                    \
        return tmp;                                                                 \
    }                                                                               \
                                                                                    \
    constexpr StructName &StructName::operator--() noexcept                         \
    {                                                                               \
        --value;                                                                    \
        return *this;                                                               \
    }                                                                               \
                                                                                    \
    constexpr StructName StructName::operator--(int) noexcept                       \
    {                                                                               \
        auto tmp = *this;                                                           \
        --*this;                                                                    \
        return tmp;                                                                 \
    }

    DEFINE_SRC_IDX_TYPE_METHOD_IMPLS(SrcLineIdx)
    DEFINE_SRC_IDX_TYPE_METHOD_IMPLS(SrcColumnIdx)
    DEFINE_SRC_IDX_TYPE_METHOD_IMPLS(SrcBuffIdx)
#undef DEFINE_SRC_IDX_TYPE_METHOD_IMPLS
} // namespace alpha
#endif // SOURCE_LOCATION_TYPES_HPP
