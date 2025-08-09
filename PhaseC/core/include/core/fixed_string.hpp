#ifndef FIXED_STRING_HPP
#define FIXED_STRING_HPP
#include <iosfwd>
#include <type_traits>

template<std::size_t N>
struct FixedString
{
    char data[N]{};

    consteval FixedString() = default;

    // Do NOT make this constructor explicit. We specifically require implicit conversion property.
    consteval FixedString(const char (&str)[N])
    {
        for (decltype(N) i = 0; i < N; ++i)
            data[i] = str[i];
    }

    consteval FixedString(const FixedString &other)
    {
        for (decltype(N) i = 0; i < N; ++i)
            data[i] = other[i];
    }

    template<typename Int>
    consteval char operator[](Int idx) const
    {
        static_assert(std::is_integral_v<Int>, "Argument `idx` must be integral.");
        if (idx >= N)
            throw "Index out of bounds";
        return data[idx];
    }

    template<typename Int>
    consteval char &operator[](Int idx)
    {
        static_assert(std::is_integral_v<Int>, "Argument `idx` must be integral.");
        if (idx >= N)
            throw "Index out of bounds";
        return data[idx];
    }

    template<decltype(N) M>
    consteval bool operator==(const FixedString<M> &fs) const
    {
        if (N != M) return false;
        for (decltype(N) i = 0; i < N; ++i)
            if (data[i] != fs[i])
                return false;
        return true;
    }

    template<decltype(N) M>
    consteval bool operator==(const char (&array_str)[M]) const
    {
        return operator==(FixedString<M>(array_str));
    }

    template<decltype(N) M>
    consteval bool starts_with(const FixedString<M> &sub_fs) const
    {
        if (M > N) return false;                // `substr` doesn't even fit inside the string.
        for (decltype(N) i = 0; i < M - 1; ++i) // M-1: ignore `substr`'s trailing '\0'.
            if (data[i] != sub_fs[i])
                return false;
        return true;
    }

    template<decltype(N) M>
    consteval bool starts_with(const char (&substr)[M]) const
    {
        return starts_with(FixedString<M>(substr));
    }

    // `prefix` is unused as a value; it's only passed so we can deduce M,
    // the compile-time length of the prefix (including the null terminator).
    template<decltype(N) M>
    consteval auto without_prefix([[maybe_unused]] const char (&prefix)[M])
    const
    {
        constexpr decltype(N) prefix_len = M - 1; // exclude null terminator
        constexpr decltype(N) remaining = N - prefix_len;
        FixedString<remaining> result{};

        for (std::size_t i = 0; i < remaining; ++i)
            result.data[i] = data[prefix_len + i];

        return result;
    }

    friend std::ostream &operator <<(std::ostream &os, const FixedString &rhs)
    {
        return os << rhs.data;
    }
};

#endif // FIXED_STRING_HPP
