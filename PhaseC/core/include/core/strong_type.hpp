#ifndef STRONG_TYPE_HPP
#define STRONG_TYPE_HPP

#include <compare>
#include <concepts>
#include <limits>

namespace alpha
{
template
<
    typename TypeTag,
    std::integral UnderlyingType_,
    UnderlyingType_ none_value = std::numeric_limits<UnderlyingType_>::max()
>
struct StrongType
{
    // We add the using, so we cant export the UnderlyingType
    using UnderlyingType = UnderlyingType_;
    UnderlyingType value;

    [[nodiscard]] static constexpr StrongType none() { return {}; }

    constexpr StrongType() : value(none_value) {}

    explicit constexpr StrongType(const UnderlyingType value) noexcept : value(value) {}

    constexpr StrongType(const StrongType&) noexcept = default;
    constexpr StrongType& operator=(const StrongType&) noexcept = default;
    [[nodiscard]] constexpr auto operator<=>(const StrongType& rhs) const noexcept = default;

    // Logic helper for better ergonomics!
    [[nodiscard]] constexpr bool is_none() const noexcept { return value == none_value; }

    constexpr StrongType& operator+=(const StrongType rhs) noexcept
    {
        value += rhs.value;
        return *this;
    }

    constexpr StrongType& operator-=(const StrongType rhs) noexcept
    {
        value -= rhs.value;
        return *this;
    }

    [[nodiscard]] constexpr StrongType operator+(const StrongType rhs) const noexcept
    {
        return StrongType{value + rhs.value};
    }

    [[nodiscard]] constexpr StrongType operator-(const StrongType rhs) const noexcept
    {
        return StrongType{value - rhs.value};
    }

    constexpr StrongType& operator++() noexcept
    {
        ++value;
        return *this;
    }

    constexpr StrongType operator++(int) noexcept
    {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    constexpr StrongType& operator--() noexcept
    {
        --value;
        return *this;
    }

    constexpr StrongType operator--(int) noexcept
    {
        auto tmp = *this;
        --*this;
        return tmp;
    }
};
} // namespace alpha
#endif // STRONG_TYPE_HPP
