#ifndef STRONG_TYPE_HPP
#define STRONG_TYPE_HPP

#include <concepts>

namespace alpha
{
template <typename TypeTag, std::integral UnderlyingType_>
struct StrongType
{
    // We add the using, so we cant export the UnderlyingType
    using UnderlyingType = UnderlyingType_;

    [[nodiscard]] static constexpr StrongType none() { return {}; }
    UnderlyingType value;

    constexpr StrongType() : value(0) {}

    explicit constexpr StrongType(const UnderlyingType value) noexcept : value(value) {}

    constexpr StrongType(const StrongType&) noexcept = default;
    constexpr StrongType& operator=(const StrongType&) noexcept = default;
    [[nodiscard]] constexpr auto operator<=>(const StrongType& rhs) const noexcept = default;

    constexpr StrongType& operator+=(StrongType rhs) noexcept
    {
        value += rhs.value;
        return *this;
    }

    constexpr StrongType& operator-=(StrongType rhs) noexcept
    {
        value -= rhs.value;
        return *this;
    }

    [[nodiscard]] constexpr StrongType operator+(StrongType rhs) const noexcept
    {
        return StrongType{value + rhs.value};
    }

    [[nodiscard]] constexpr StrongType operator-(StrongType rhs) const noexcept
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
