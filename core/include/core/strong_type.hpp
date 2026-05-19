#ifndef STRONG_TYPE_HPP
#define STRONG_TYPE_HPP

#include <compare>
#include <concepts>
#include <limits>

namespace alpha
{
template
<
    typename DerivedType,
    std::integral UnderlyingType_,
    UnderlyingType_ none_value = std::numeric_limits<UnderlyingType_>::max()
>
struct StrongType
{
    // We add the using, so we cant export the UnderlyingType
    using UnderlyingType = UnderlyingType_;
    UnderlyingType value;

    [[nodiscard]] static constexpr DerivedType none() { return {}; }

    constexpr StrongType() : value(none_value) {}

    explicit constexpr StrongType(const UnderlyingType value) noexcept : value(value) {}

    constexpr StrongType(const StrongType&) noexcept = default;
    constexpr StrongType& operator=(const StrongType&) noexcept = default;
    [[nodiscard]] constexpr auto operator<=>(const StrongType& rhs) const noexcept = default;

    // Logic helper for better ergonomics!
    [[nodiscard]] constexpr bool is_none() const noexcept { return value == none_value; }

    constexpr DerivedType& operator+=(const DerivedType rhs) noexcept
    {
        value += rhs.value;
        return static_cast<DerivedType&>(*this);
    }

    constexpr DerivedType& operator-=(const DerivedType rhs) noexcept
    {
        value -= rhs.value;
        return static_cast<DerivedType&>(*this);
    }

    [[nodiscard]] constexpr DerivedType operator+(const DerivedType rhs) const noexcept
    {
        return DerivedType{value + rhs.value};
    }

    [[nodiscard]] constexpr DerivedType operator-(const DerivedType rhs) const noexcept
    {
        return DerivedType{value - rhs.value};
    }

    constexpr DerivedType& operator++() noexcept
    {
        ++value;
        return static_cast<DerivedType&>(*this);
    }

    constexpr DerivedType operator++(int) noexcept
    {
        auto tmp = static_cast<DerivedType&>(*this);
        ++static_cast<DerivedType&>(*this);
        return tmp;
    }

    constexpr DerivedType& operator--() noexcept
    {
        --value;
        return static_cast<DerivedType&>(*this);
    }

    constexpr DerivedType operator--(int) noexcept
    {
        auto tmp = static_cast<DerivedType&>(*this);
        --static_cast<DerivedType&>(*this);
        return tmp;
    }
};
} // namespace alpha
#endif // STRONG_TYPE_HPP
