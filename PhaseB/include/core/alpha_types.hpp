#ifndef ALPHA_TYPES_HPP
#define ALPHA_TYPES_HPP

#include <cstdint>

namespace Alpha
{
        /* Fixed-width unsigned integers. */
        using u8 = std::uint8_t;
        using u16 = std::uint16_t;
        using u32 = std::uint32_t;
        using u64 = std::uint64_t;

        /* Fixed-width signed integers. */
        using i8 = std::int8_t;
        using i16 = std::int16_t;
        using i32 = std::int32_t;
        using i64 = std::int64_t;

        /* Fast minimum-width unsigned integers. */
        using uf8 = std::uint_fast8_t;
        using uf16 = std::uint_fast16_t;
        using uf32 = std::uint_fast32_t;
        using uf64 = std::uint_fast64_t;

        /* Fast minimum-width signed integers. */
        using if8 = std::int_fast8_t;
        using if16 = std::int_fast16_t;
        using if32 = std::int_fast32_t;
        using if64 = std::int_fast64_t;

        /* Floating-point types. */
        using f32 = float;
        using f64 = double;

        static_assert(sizeof(f32) == 4, "Type `f32` is not 4 bytes on current system");
        static_assert(sizeof(f64) == 8, "Type `f64` is not 8 bytes on current system");
}

#endif // ALPHA_TYPES_HPP
