#ifndef ENUM_OPERATORS_HPP
#define ENUM_OPERATORS_HPP
#include <type_traits>

template<typename Enum>
constexpr Enum operator |(const Enum lhs, const Enum rhs) noexcept
{
    static_assert(std::is_enum_v<Enum>);
    using U = std::underlying_type_t<Enum>;
    return static_cast<Enum>(static_cast<U>(lhs) | static_cast<U>(rhs));
}

template<typename Enum>
constexpr Enum operator &(const Enum lhs, const Enum rhs) noexcept
{
    static_assert(std::is_enum_v<Enum>);
    using U = std::underlying_type_t<Enum>;
    return static_cast<Enum>(static_cast<U>(lhs) & static_cast<U>(rhs));
}

template<typename Enum>
constexpr Enum operator ~(const Enum unary) noexcept
{
    static_assert(std::is_enum_v<Enum>);
    using U = std::underlying_type_t<Enum>;
    return static_cast<Enum>(~static_cast<U>(unary));
}

template<typename Enum>
constexpr bool any(Enum e) noexcept
{
    static_assert(std::is_enum_v<Enum>);
    using U = std::underlying_type_t<Enum>;
    return static_cast<U>(e) != 0;
}

#endif //ENUM_OPERATORS_HPP
