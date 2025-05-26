#ifndef ALPHA_TYPES_HPP
#define ALPHA_TYPES_HPP

#include <cstdint>

namespace Alpha
{
        /* Fixed-width unsigned integers. */
        using U8 = std::uint8_t;
        using U16 = std::uint16_t;
        using U32 = std::uint32_t;
        using U64 = std::uint64_t;

        /* Fixed-width signed integers. */
        using I8 = std::int8_t;
        using I16 = std::int16_t;
        using I32 = std::int32_t;
        using I64 = std::int64_t;

        /* Fast minimum-width unsigned integers. */
        using UF8 = std::uint_fast8_t;
        using UF16 = std::uint_fast16_t;
        using UF32 = std::uint_fast32_t;
        using UF64 = std::uint_fast64_t;

        /* Fast minimum-width signed integers. */
        using IF8 = std::int_fast8_t;
        using IF16 = std::int_fast16_t;
        using IF32 = std::int_fast32_t;
        using if64 = std::int_fast64_t;

        /* Floating-point types. */
        using F32 = float;
        using F64 = double;

        static_assert(sizeof(F32) == 4, "Type `F32` is not 4 bytes on current system");
        static_assert(sizeof(F64) == 8, "Type `F64` is not 8 bytes on current system");
} // namespace Alpha
#endif // ALPHA_TYPES_HPP
